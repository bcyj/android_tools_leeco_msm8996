/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "bg_stats.h"
#include "isp_log.h"

#ifdef BG_STATS_DEBUG
#undef ISP_DBG
#define ISP_DBG ALOGE
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE
#endif

/** bg_stats_debug:
 *    @pcmd: Pointer to the reg_cmd struct that needs to be dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 *
 * Return void
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
 *    @entry: Pointer to entry struct of bg stats
 *    @pix_settings: Pointer to the pipeline settings
 *    @in_param_size: Size of pix_settings
 *
 *  Configure the entry and reg_cmd for bg_stats using values passed in pix
 *  settings
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int bg_stats_config(isp_stats_entry_t *entry,
  isp_hw_pix_setting_params_t *pix_settings,
  uint32_t in_param_size)
{
  ISP_StatsBg_CfgCmdType *pcmd = entry->reg_cmd;
  aec_bg_config_t *bg_config = &pix_settings->stats_cfg.aec_config.bg_config;
  uint32_t bg_rgn_width, bg_rgn_height;

  ISP_DBG(ISP_MOD_STATS, "%s: config BG, enable %d\n", __func__, entry->enable);
  if (!entry->enable) {
    ISP_DBG(ISP_MOD_STATS, "%s: BG not enabled", __func__);
    return 0;
  }

  entry->session_id = pix_settings->outputs->stream_param.session_id;
  entry->is_ispif_split = pix_settings->camif_cfg.ispif_out_info.is_split;
  if (pix_settings->camif_cfg.ispif_out_info.is_split) {
     if (pix_settings->outputs[0].isp_out_info.stripe_id == 0)
       entry->buf_offset = 0;
     else
       entry->buf_offset = ISP_STATS_BG_BUF_SIZE;
  }

  entry->ion_fd = pix_settings->ion_fd;
  entry->hfr_mode = pix_settings->camif_cfg.hfr_mode;
  entry->comp_flag = 1;

  if (bg_config->grid_info.h_num == 0 || bg_config->grid_info.v_num == 0) {
    CDBG_ERROR("%s: ERROR Received stats config with h_num=%d v_num=%d\n",
      __func__, bg_config->grid_info.h_num, bg_config->grid_info.v_num);
    return -1;
  }
  bg_rgn_width = bg_config->roi.width /
                   bg_config->grid_info.h_num;
  bg_rgn_height = bg_config->roi.height /
                   bg_config->grid_info.v_num;

  pix_settings->saved_zoom_roi.rgnHOffset = pcmd->rgnHOffset =
    FLOOR2(bg_config->roi.left);
  pix_settings->saved_zoom_roi.rgnVOffset = pcmd->rgnVOffset =
    FLOOR2(bg_config->roi.top);
  pix_settings->saved_zoom_roi.rgnWidth = pcmd->rgnWidth =
    FLOOR2(bg_rgn_width) - 1;
  pix_settings->saved_zoom_roi.rgnHeight = pcmd->rgnHeight =
    FLOOR2(bg_rgn_height) - 1;
  pix_settings->saved_zoom_roi.rgnHNum =  pcmd->rgnHNum =
    bg_config->grid_info.h_num - 1;
  pix_settings->saved_zoom_roi.rgnVNum = pcmd->rgnVNum =
    bg_config->grid_info.v_num - 1;
  pix_settings->saved_zoom_roi.rMax =  pcmd->rMax =
    bg_config->r_Max;
  pix_settings->saved_zoom_roi.grMax = pcmd->grMax =
    bg_config->gr_Max;
  pix_settings->saved_zoom_roi.bMax = pcmd->bMax =
    bg_config->b_Max;
  pix_settings->saved_zoom_roi.gbMax = pcmd->gbMax =
    bg_config->gb_Max;

  /* update aecConfig to reflect the new config    *
     It will be sent to 3A in STATS_NOTIFY event  */
  bg_config->roi.left = pcmd->rgnHOffset;
  bg_config->roi.top = pcmd->rgnVOffset;
  bg_config->roi.width = (pcmd->rgnWidth + 1) * bg_config->grid_info.h_num;
  bg_config->roi.height = (pcmd->rgnHeight + 1) * bg_config->grid_info.v_num;

  if (entry->is_ispif_split) {
    uint32_t end_point_stats_bound;
    uint32_t overlap = pix_settings->camif_cfg.ispif_out_info.overlap;
    isp_out_info_t *isp_out = &pix_settings->outputs[0].isp_out_info;
    /* Its possible that end point of stats boundary is falling completely within the left strip
       so the calculation for the no_regions_left will depend on the end of the stats boundary. */
    end_point_stats_bound = pcmd->rgnHOffset + (pcmd->rgnWidth + 1) * (pcmd->rgnHNum + 1);
    if (end_point_stats_bound > isp_out->right_stripe_offset + overlap)
      end_point_stats_bound = isp_out->right_stripe_offset + overlap;
    entry->buf_offset = isp_out->stripe_id * ISP_STATS_BG_BUF_SIZE;
    entry->num_left_rgns = (end_point_stats_bound > pcmd->rgnHOffset) ?
      (end_point_stats_bound - pcmd->rgnHOffset) / (pcmd->rgnWidth + 1) : 0;
    entry->num_right_rgns = (pcmd->rgnHNum + 1) - entry->num_left_rgns;
    /* Check if the stat region satisfies the crucial assumption: there is
       at least one grid line within the overlap area if the whole region spans across
       both stripes */
    if (pcmd->rgnHOffset + (entry->num_left_rgns * (pcmd->rgnWidth + 1)) < isp_out->right_stripe_offset &&
        entry->num_right_rgns > 0) {
      CDBG_ERROR("%s: Unable to support such stats region in dual-ISP mode\n", __func__);
      return -1;
    }
    if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
      /* Make sure if number of region is zero for each side, we don't actually program 0 region */
      pcmd->rgnHNum = (entry->num_left_rgns > 0) ? entry->num_left_rgns - 1 : 1;
      pcmd->rgnHOffset = (entry->num_left_rgns > 0) ? pcmd->rgnHOffset : 2; // Default offset for BG stats
    } else { /* ISP_STRIPE_RIGHT */
      pcmd->rgnHNum = (entry->num_right_rgns > 0) ? entry->num_right_rgns - 1 : 1;
      pcmd->rgnHOffset = pcmd->rgnHOffset +
        (entry->num_left_rgns * (pcmd->rgnWidth + 1)) - isp_out->right_stripe_offset;
      pcmd->rgnHOffset = (entry->num_right_rgns > 0) ? pcmd->rgnHOffset : 2; // Default offset for BG stats
    }
  }

  bg_stats_debug(pcmd);
  entry->hw_update_pending = 1;
  return 0;
}

/** bg_stats_enable:
 *    @entry: Pointer to entry struct
 *    @in_params: input params with enable value
 *
 *  Enable the bg stats module
 *
 *
 * Return 0 on Success, negative on ERROR
 **/
static int bg_stats_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  entry->enable = in_params->enable;
  entry->is_first = 1;
  return 0;
}

/** bg_stats_trigger_enable:
 *    @entry: Pointer to entry struct of bg stats
 *    @in_params: Input params
 *
 *  Set the trigger enable
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int bg_stats_trigger_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  entry->trigger_enable = in_params->enable;
  return 0;
}

/** bg_stats_set_params:
 *    @ctrl: Pointer to entry (ctrl) struct
 *    @param_id: event id indicating what value is set
 *    @in_params: input event params
 *    @in_param_size: size of input params
 *
 *  Set value for parameter given by param id and pass input params
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int bg_stats_set_params (void *ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_STATS_SET_ENABLE:
    rc = bg_stats_enable(entry, (isp_mod_set_enable_t *)in_params);
    break;
  case ISP_STATS_SET_CONFIG:
    rc = bg_stats_config(entry,
    (isp_hw_pix_setting_params_t *)in_params, in_param_size);
    break;
  case ISP_STATS_SET_TRIGGER_ENABLE:
    rc = bg_stats_trigger_enable(entry, (isp_mod_set_enable_t *)in_params);
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

/** bg_stats_get_params:
 *    @ctrl: Pointer to entry (ctrl) struct
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
static int bg_stats_get_params (void *ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size,
                     void *out_params, uint32_t out_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_STATS_GET_ENABLE:
    break;
  case ISP_STATS_GET_STREAM_HANDLE:{
    uint32_t *handle = (uint32_t *) (out_params);
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
 *    @entry: Pointer to entry struct for bg stats
 *
 *  Method called at SOF, writes the value in reg_cmd to HW register
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int bg_stats_do_hw_update(isp_stats_entry_t *entry)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  if (entry->hw_update_pending) {
    cfg_cmd.cfg_data = (void *)entry->reg_cmd;
    cfg_cmd.cmd_len = sizeof(ISP_StatsBg_CfgCmdType);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = BG_STATS_OFF;
    reg_cfg_cmd[0].u.rw_info.len = BG_STATS_LEN * sizeof(uint32_t);

    rc = ioctl(entry->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }
    entry->hw_update_pending = 0;
    entry->skip_stats = 1;
  }

  return rc;
}

/** bg_stats_parse:
 *    @entry: Pointer to entry struct for bg stats
 *    @raw_buf: Pointer to raw buf containing the stats recevied from HW
 *    @h_rgns_start: start value of H Region for this ISP HW Stripe
 *    @h_rgns_end: end value of H Region for this ISP HW Stripe
 *    @h_rgns_total: Total num of H Regions including both Stripes
 *    @v_rgns_total: Total num of V Regions
 *
 *  Parse the different stats parameters received from raw_buf to their
 *  corresponding struct
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int bg_stats_parse(isp_stats_entry_t *entry,
                   void *raw_buf,
                   q3a_bg_stats_t *bg_stats,
                   uint32_t h_rgns_start,
                   uint32_t h_rgns_end,
                   uint32_t h_rgns_total,
                   uint32_t v_rgns_total)
{
  uint32_t *SY,*Sr, *Sb, *Sgr, *Sgb;
  uint32_t *r_num, *b_num, *gr_num, *gb_num;
  uint32_t *current_region;
  uint32_t  i, j, x, y;

  ISP_DBG(ISP_MOD_STATS, "%s: E\n", __func__);
  Sr     = bg_stats->bg_r_sum;
  Sb     = bg_stats->bg_b_sum;
  Sgr    = bg_stats->bg_gr_sum;
  Sgb    = bg_stats->bg_gb_sum;
  r_num  = bg_stats->bg_r_num;
  b_num  = bg_stats->bg_b_num;
  gr_num = bg_stats->bg_gr_num;
  gb_num = bg_stats->bg_gb_num;

  current_region = (uint32_t*)raw_buf;
  bg_stats->bg_region_h_num += (h_rgns_end - h_rgns_start + 1);
  bg_stats->bg_region_v_num = v_rgns_total;
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
  for (j = 0; j < v_rgns_total; j++) {
    for (i = 0; i < h_rgns_total; i++) {
      if (i >= h_rgns_start && i <= h_rgns_end) {
        /* 23 bits sum of r, b, gr, gb. */
        *Sr = ((*(current_region)) & 0x007FFFFF);
        current_region ++;
        *Sb = ((*(current_region)) & 0x007FFFFF);
        current_region ++;
        *Sgr = ((*(current_region)) & 0x007FFFFF);
        current_region ++;
        *Sgb = ((*(current_region)) & 0x007FFFFF);
        current_region ++;
        /*15 bit pixel count used for r_sum, b_sum, gr_sum and gb_sum*/
        *r_num = ((*(current_region)) & 0x00007FFF);
        *b_num = ((*(current_region)) & 0x7FFF0000) >> 16;
        current_region++;
        *gr_num = ((*(current_region)) & 0x00007FFF);
        *gb_num = ((*(current_region)) & 0x7FFF0000) >> 16;
        current_region ++;
      }
      Sr++;
      Sb++;
      Sgr++;
      Sgb++;
      r_num++;
      b_num++;
      gr_num++;
      gb_num++;
    }
  }
  /* convert bayer r,g,b stat into Ysum to make
   * it work on current 3a version
   * that uses 16x16 Ysum is done in 3A code
   **/
  ISP_DBG(ISP_MOD_STATS, "%s: X\n", __func__);
  return 0;
}

/** bg_stats_reset:
 *    @entry: Pointer to entry struct for bg stats
 *
 * Reset the entry struct
 *
 *
 * Return void
 **/
static void bg_reset(isp_stats_entry_t *entry)
{
  ISP_StatsBg_CfgCmdType *pcmd = entry->reg_cmd;

  isp_stats_reset(entry);
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
}

/** bg_stats_action:
 *    @ctrl: Pointer to entry (ctrl) struct
 *    @action_code:  indicates what action is required
 *    @data: input param
 *    @data_size: size of input param
 *
 *  Do the required actions for event given by action code
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int bg_stats_action (void *ctrl, uint32_t action_code,
                 void *data, uint32_t data_size)
{
  int rc = 0;
  isp_stats_entry_t *entry = ctrl;

  ISP_DBG(ISP_MOD_STATS, "%s: action code = %d\n", __func__, action_code);
  switch ((isp_stats_action_code_t)action_code) {
  case ISP_STATS_ACTION_STREAM_START:
    break;
  case ISP_STATS_ACTION_STREAM_STOP:
    break;
  case ISP_STATS_ACTION_RESET:
    bg_reset(entry);
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
    ISP_StatsBg_CfgCmdType *pcmd = entry->reg_cmd;

    int buf_idx = action_data->raw_stats_event->
                  u.stats.stats_buf_idxs[MSM_ISP_STATS_BG];
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
    isp_stats_event->stats_mask |= (1 << MSM_ISP_STATS_BG);
    memset(bg_stats, 0, sizeof(q3a_bg_stats_t));
    if (!entry->is_ispif_split)
      rc = bg_stats_parse(entry, raw_buf, bg_stats,
        0,
        pcmd->rgnHNum,
        pcmd->rgnHNum + 1,
        pcmd->rgnVNum + 1);
    else { /* ISP is outputting two split halves */
      /* Parse for left stripe */
      if (entry->num_left_rgns)
        rc = bg_stats_parse(entry, raw_buf, bg_stats,
          0,
          entry->num_left_rgns - 1,
          entry->num_left_rgns + entry->num_right_rgns,
          pcmd->rgnVNum + 1);
      /* Parse for right stripe */
      if (entry->num_right_rgns)
        rc = bg_stats_parse(entry, (uint8_t*)raw_buf + entry->buf_len / 2, bg_stats,
          entry->num_left_rgns,
          entry->num_left_rgns + entry->num_right_rgns - 1,
          entry->num_left_rgns + entry->num_right_rgns,
          pcmd->rgnVNum + 1);
    }
    if (entry->num_bufs !=0) {
       rc = isp_stats_enqueue_buf(entry, buf_idx);
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

/** bg_stats_init:
 *    @ctrl: Pointer to entry (ctrl) struct
 *    @in_params: input params
 *    @notify_ops: notify_ops function pointer
 *
 *  Set initial values in the bg stats entry
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int bg_stats_init (void *ctrl, void *in_params, isp_notify_ops_t *notify_ops)
{
  isp_stats_entry_t *entry = ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;
  ISP_StatsBg_CfgCmdType *pcmd = entry->reg_cmd;

  /* max size for dual vfe, pass the 2nd buf offset by the size*/
  entry->buf_len = ISP_STATS_BG_BUF_SIZE * 2;

  entry->stats_type = MSM_ISP_STATS_BG;
  entry->fd = init_params->fd;
  entry->notify_ops = notify_ops;
  entry->dev_idx = init_params->dev_idx;
  entry->buf_mgr = init_params->buf_mgr;
  entry->skip_stats = 0;

  bg_reset(entry);
  return 0;
}

/** bg_stats_destroy:
 *    @ctrl: Pointer to the entry struct for bg stats
 *
 *  Free the memory for the bg stats module
 *
 *
 * Return 0 on Success, negative on ERROR
 **/
static int bg_stats_destroy (void *ctrl)
{
  isp_stats_entry_t *entry = ctrl;
  free(entry->reg_cmd);
  if (entry->parsed_stats_buf)
    free(entry->parsed_stats_buf);
  free(entry);
  return 0;
}

/** bg_stats44_open:
 *    @stats: Pointer to the ISP stats module
 *    @stats_type: enum value that gives what stats this is
 *
 *  Allocate and Initialize memory for bg stats module
 *
 *
 *  Return pointer to isp ops. NULL if ERROR
 **/
isp_ops_t *bg_stats44_open(isp_stats_mod_t *stats,
                                     enum msm_isp_stats_type stats_type)
{
  int rc = 0;
  isp_stats_entry_t *entry = NULL;
  ISP_StatsBg_CfgCmdType *cmd = NULL;
  ISP_DBG(ISP_MOD_STATS, "%s: open BG stats\n", __func__);

  entry = malloc(sizeof(isp_stats_entry_t));
  if (!entry) {
    CDBG_ERROR("%s: no mem for bg\n",  __func__);
    return NULL;
  }
  cmd = malloc(sizeof(ISP_StatsBg_CfgCmdType));
  if (!cmd) {
    CDBG_ERROR("%s: no mem\n",  __func__);
    free(entry);
    return NULL;
  }
  memset(entry, 0, sizeof(isp_stats_entry_t));
  memset(cmd, 0, sizeof(*cmd));
  entry->len_parsed_stats_buf = sizeof(q3a_bg_stats_t);
  entry->parsed_stats_buf = malloc(entry->len_parsed_stats_buf);
  if (entry->parsed_stats_buf== NULL) {
    CDBG_ERROR("%s: no mem\n",  __func__);
    free(cmd);
    free(entry);
    return NULL;
  }
  entry->reg_cmd = cmd;
  entry->ops.ctrl = (void *)entry;
  entry->ops.init = bg_stats_init;
  /* destroy the module object */
  entry->ops.destroy = bg_stats_destroy;
  /* set parameter */
  entry->ops.set_params = bg_stats_set_params;
  /* get parameter */
  entry->ops.get_params = bg_stats_get_params;
  entry->ops.action = bg_stats_action;
  return &entry->ops;
}
