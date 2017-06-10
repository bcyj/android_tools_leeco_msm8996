/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "be_stats.h"
#include "isp_log.h"

#ifdef BE_STATS_DEBUG
#undef ISP_DBG
#define ISP_DBG ALOGE
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE
#endif

/** be_stats_debug:
 *    @pcmd: Pointer to the reg_cmd struct that needs to be dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 *
 * Return void
 **/
static void be_stats_debug(ISP_StatsBe_CfgCmdType *pcmd)
{
  ISP_DBG(ISP_MOD_STATS, "%s:Bayer Exposure Stats Configurations\n", __func__);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnHOffset %d\n", __func__, pcmd->rgnHOffset);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnVOffset %d\n", __func__, pcmd->rgnVOffset);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnWidth   %d\n", __func__, pcmd->rgnWidth);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnHeight  %d\n", __func__, pcmd->rgnHeight);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnHNum    %d\n", __func__, pcmd->rgnHNum);
  ISP_DBG(ISP_MOD_STATS, "%s:rgnVNum    %d\n", __func__, pcmd->rgnVNum);
  ISP_DBG(ISP_MOD_STATS, "%s:r_max      %d\n", __func__, pcmd->rMax);
  ISP_DBG(ISP_MOD_STATS, "%s:gr_max     %d\n", __func__, pcmd->grMax);
  ISP_DBG(ISP_MOD_STATS, "%s:b_max      %d\n", __func__, pcmd->bMax);
  ISP_DBG(ISP_MOD_STATS, "%s:gb_max     %d\n", __func__, pcmd->gbMax);
}

/** be_stats_config:
 *    @entry: Pointer to entry struct of be stats
 *    @pix_settings: Pointer to the pipeline settings
 *    @in_param_size: Size of pix_settings
 *
 *  Configure the entry and reg_cmd for be_stats using values passed in pix
 *  settings
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int be_stats_config(isp_stats_entry_t *entry,
  isp_hw_pix_setting_params_t *pix_settings,
  uint32_t in_param_size)
{
  ISP_StatsBe_CfgCmdType *pcmd = entry->reg_cmd;
  isp_pix_camif_cfg_t *camif_cfg = &pix_settings->camif_cfg;
  uint32_t camif_window_w_t, camif_window_h_t;

  ISP_DBG(ISP_MOD_STATS, "%s: config BE, enable %d\n", __func__, entry->enable);
  if (!entry->enable) {
    ISP_DBG(ISP_MOD_STATS, "%s: BE not enabled", __func__);
    return 0;
  }
  ISP_DBG(ISP_MOD_STATS, "%s: ion_fd = %d\n", __func__, pix_settings->ion_fd);

  entry->session_id = pix_settings->outputs->stream_param.session_id;
  entry->is_ispif_split = pix_settings->camif_cfg.ispif_out_info.is_split;
  if (pix_settings->camif_cfg.ispif_out_info.is_split) {
     if (pix_settings->outputs[0].isp_out_info.stripe_id == 0)
       entry->buf_offset = 0;
     else
       entry->buf_offset = ISP_STATS_BE_BUF_SIZE;
  }

  entry->ion_fd = pix_settings->ion_fd;
  camif_window_w_t = camif_cfg->sensor_out_info.request_crop.last_pixel -
    camif_cfg->sensor_out_info.request_crop.first_pixel + 1;
  camif_window_h_t = camif_cfg->sensor_out_info.request_crop.last_line -
    camif_cfg->sensor_out_info.request_crop.first_line + 1;

  pcmd->rgnHOffset = FLOOR2(camif_window_w_t%32);
  pcmd->rgnVOffset = FLOOR2(camif_window_h_t%24);
  pcmd->rgnWidth   = FLOOR2(camif_window_w_t/32) - 1;
  pcmd->rgnHeight  = FLOOR2(camif_window_h_t/24) - 1;
  pcmd->rgnHNum    = 31;
  pcmd->rgnVNum    = 23;
  pcmd->rMax       = 255 - 16;
  pcmd->grMax      = 255 - 16;
  pcmd->bMax       = 255 - 16;
  pcmd->gbMax      = 255 - 16;

  entry->is_ispif_split = pix_settings->camif_cfg.ispif_out_info.is_split;
  if (entry->is_ispif_split) {
    uint32_t end_point_stats_bound;
    uint32_t overlap = pix_settings->camif_cfg.ispif_out_info.overlap;
    isp_out_info_t *isp_out = &pix_settings->outputs[0].isp_out_info;
    /* Its possible that end point of stats boundary is falling completely within the left strip
       so the calculation for the no_regions_left will depend on the end of the stats boundary. */
    end_point_stats_bound = pcmd->rgnHOffset + (pcmd->rgnWidth + 1) * (pcmd->rgnHNum + 1);
    if (end_point_stats_bound > isp_out->right_stripe_offset + overlap)
      end_point_stats_bound = isp_out->right_stripe_offset + overlap;
    entry->buf_offset = isp_out->stripe_id * ISP_STATS_BE_BUF_SIZE;
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
      pcmd->rgnHOffset = (entry->num_left_rgns > 0) ? pcmd->rgnHOffset : 2; // Default offset for BE stats
    } else { /* ISP_STRIPE_RIGHT */
      pcmd->rgnHNum = (entry->num_right_rgns > 0) ? entry->num_right_rgns - 1 : 1;
      pcmd->rgnHOffset = pcmd->rgnHOffset +
        (entry->num_left_rgns * (pcmd->rgnWidth + 1)) - isp_out->right_stripe_offset;
      pcmd->rgnHOffset = (entry->num_right_rgns > 0) ? pcmd->rgnHOffset : 2; // Default offset for BE stats
    }
  }

  be_stats_debug(pcmd);
    entry->hw_update_pending = 1;
  return 0;
}

/** be_stats_enable:
 *    @entry: Pointer to entry struct
 *    @in_params: input params with enable value
 *
 *  Enable the be stats module
 *
 *
 * Return 0 on Success, negative on ERROR
 **/
static int be_stats_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  entry->enable = in_params->enable;
  return 0;
}

/** be_stats_trigger_enable:
 *    @entry: Pointer to entry struct of be stats
 *    @in_params: Input params
 *
 *  Set the trigger enable
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int be_stats_triger_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  entry->trigger_enable = in_params->enable;
  return 0;
}

/** be_stats_set_params:
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
static int be_stats_set_params (void *ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_STATS_SET_ENABLE:
    rc = be_stats_enable(entry, (isp_mod_set_enable_t *)in_params);
    break;
  case ISP_STATS_SET_CONFIG:
    rc = be_stats_config(entry,
      (isp_hw_pix_setting_params_t *)in_params, in_param_size);
    break;
  case ISP_STATS_SET_TRIGGER_ENABLE:
    rc = be_stats_triger_enable(entry, (isp_mod_set_enable_t *)in_params);
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

/** be_stats_get_params:
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
static int be_stats_get_params (void *ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size,
                     void *out_params, uint32_t out_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;

  ISP_DBG(ISP_MOD_STATS, "%s: param_id = %d\n", __func__, param_id);
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

/** be_stats_do_hw_update:
 *    @entry: Pointer to entry struct for be stats
 *
 *  Method called at SOF, writes the value in reg_cmd to HW register
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int be_stats_do_hw_update(isp_stats_entry_t *entry)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  if (entry->hw_update_pending) {
    cfg_cmd.cfg_data = (void *)entry->reg_cmd;
    cfg_cmd.cmd_len = sizeof(ISP_StatsBe_CfgCmdType);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = BE_STATS_OFF;
    reg_cfg_cmd[0].u.rw_info.len = BE_STATS_LEN * sizeof(uint32_t);

    rc = ioctl(entry->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }

    entry->hw_update_pending = 0;
  }

  return rc;
}

/** be_stats_parse:
 *    @entry: Pointer to entry struct for be stats
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
static int be_stats_parse(isp_stats_entry_t *entry,
                   void *raw_buf,
                   q3a_be_stats_t *be_stats,
                   uint32_t h_rgns_start,
                   uint32_t h_rgns_end,
                   uint32_t h_rgns_total,
                   uint32_t v_rgns_total)
{
  uint32_t *SY,*Sr, *Sb, *Sgr, *Sgb;
  uint32_t *r_num, *b_num, *gr_num, *gb_num;
  uint32_t *current_region;
  uint32_t  i, j, x, y;

  Sr     = be_stats->be_r_sum;
  Sb     = be_stats->be_b_sum;
  Sgr    = be_stats->be_gr_sum;
  Sgb    = be_stats->be_gb_sum;
  r_num  = be_stats->be_r_num;
  b_num  = be_stats->be_b_num;
  gr_num = be_stats->be_gr_num;
  gb_num = be_stats->be_gb_num;

  current_region = (uint32_t*)raw_buf;
  be_stats->be_region_h_num += (h_rgns_end - h_rgns_start + 1);
  be_stats->be_region_v_num = v_rgns_total;
  /* BE Stats expect:
   * 1 - 24bit out of 32bit r_sum
   * 2 - 24bit out of 32bit b_sum
   * 3 - 24bit out of 32bit gr_sum
   * 4 - 24bit out of 32bit gb_sum
   * 5 - 16bit out of 32bit USL bnum, 16bit out of 32bit LSL rnum
   * 6 - 16bit out of 32bit USL gbnum, 16bit out of 32bit LSL grnum
   * Expect buf_size = 32*24*6
  */
  for (j = 0; j < v_rgns_total; j++) {
    for (i = 0; i < h_rgns_total; i++) {
      if (i >= h_rgns_start && i <= h_rgns_end) {
        /* 24 bits sum of r, b, gr, gb. */
        *Sr = ((*(current_region)) & 0x00FFFFFF);
        current_region ++;
        *Sb = ((*(current_region)) & 0x00FFFFFF);
        current_region ++;
        *Sgr = ((*(current_region)) & 0x00FFFFFF);
        current_region ++;
        *Sgb = ((*(current_region)) & 0x00FFFFFF);
        current_region ++;
        /*16 bit pixel count used for r_sum, b_sum, gr_sum and gb_sum*/
        *r_num = ((*(current_region)) & 0x0000FFFF);
        *b_num = ((*(current_region)) & 0xFFFF0000) >> 16;
        current_region++;
        *gr_num = ((*(current_region)) & 0x0000FFFF);
        *gb_num = ((*(current_region)) & 0xFFFF0000) >> 16;
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
   * it work on current 3a version that uses
   * 16x16 Ysum is done in 3A code */
  return 0;
}

/** be_stats_reset:
 *    @entry: Pointer to entry struct for be stats
 *
 * Reset the entry struct
 *
 *
 * Return void
 **/
static void be_reset(isp_stats_entry_t *entry)
{
  ISP_StatsBe_CfgCmdType *pcmd = entry->reg_cmd;

  isp_stats_reset(entry);
  pcmd->rgnHOffset = 0;
  pcmd->rgnVOffset = 0;
  pcmd->rgnWidth   = 0;
  pcmd->rgnHeight  = 0;
  pcmd->rgnHNum    = 31;
  pcmd->rgnVNum    = 23;
  pcmd->rMax      = 254 - 16;
  pcmd->grMax     = 254 - 16;
  pcmd->bMax      = 254 - 16;
  pcmd->gbMax     = 254 - 16;
}

/** be_stats_action:
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
static int be_stats_action (void *ctrl, uint32_t action_code,
                 void *data, uint32_t data_size)
{
  int rc = 0;
  isp_stats_entry_t *entry = ctrl;

  ISP_DBG(ISP_MOD_STATS, "%s: action code = %d\n", __func__, action_code);
  switch (action_code) {
  case ISP_STATS_ACTION_STREAM_START:
    break;
  case ISP_STATS_ACTION_STREAM_STOP:
    break;
  case ISP_STATS_ACTION_RESET:
    be_reset(entry);
    break;
  case ISP_STATS_ACTION_HW_CFG_UPDATE:
    rc = be_stats_do_hw_update(entry);
    break;
  case ISP_STATS_ACTION_STREAM_BUF_CFG:
    rc = isp_stats_config_stats_stream(entry, ISP_STATS_BE_BUF_NUM);
    break;
  case ISP_STATS_ACTION_STREAM_BUF_UNCFG:
    rc = isp_stats_unconfig_stats_stream(entry);
    break;
  case ISP_STATS_ACTION_STATS_PARSE: {
    isp_pipeline_stats_parse_t *action_data = data;
    mct_event_stats_isp_t *isp_stats_event = action_data->parsed_stats_event;
    mct_event_stats_isp_data_t *stats_data = isp_stats_event->stats_data;
    ISP_StatsBe_CfgCmdType *pcmd = entry->reg_cmd;
    int buf_idx = action_data->raw_stats_event->
                  u.stats.stats_buf_idxs[MSM_ISP_STATS_BE];
    void *raw_buf = isp_get_buf_addr(entry->buf_mgr,
      entry->buf_handle, buf_idx);
    if(!raw_buf){
      CDBG_ERROR("%s: isp_get_buf_addr failed!\n", __func__);
      return -1;
    }
    q3a_be_stats_t *be_stats = entry->parsed_stats_buf;
    isp_stats_event->stats_mask |= (1 << MSM_ISP_STATS_BE);
    memset(be_stats, 0, sizeof(q3a_be_stats_t));
    if (!entry->is_ispif_split)
      rc = be_stats_parse(entry, raw_buf, be_stats,
        0,
        pcmd->rgnHNum,
        pcmd->rgnHNum + 1,
        pcmd->rgnVNum + 1);
    else { /* ISP is outputting two split halves */
      /* Parse for left stripe */
      if (entry->num_left_rgns)
        rc = be_stats_parse(entry, raw_buf, be_stats,
          0,
          entry->num_left_rgns - 1,
          entry->num_left_rgns + entry->num_right_rgns,
          pcmd->rgnVNum + 1);
      /* Parse for right stripe */
      if (entry->num_right_rgns)
        rc = be_stats_parse(entry, (uint8_t*)raw_buf + entry->buf_len / 2, be_stats,
          entry->num_left_rgns,
          entry->num_left_rgns + entry->num_right_rgns - 1,
          entry->num_left_rgns + entry->num_right_rgns,
          pcmd->rgnVNum + 1);
    }
    if (entry->num_bufs !=0) {
       rc = isp_stats_enqueue_buf(entry, buf_idx);
    }
    if (rc == 0) {
      stats_data[MSM_ISP_STATS_BE].stats_buf = be_stats;
      stats_data[MSM_ISP_STATS_BE].stats_type = MSM_ISP_STATS_BE;
      stats_data[MSM_ISP_STATS_BE].buf_size = sizeof(q3a_be_stats_t);
      stats_data[MSM_ISP_STATS_BE].used_size = sizeof(q3a_be_stats_t);
    } else {
      stats_data[MSM_ISP_STATS_BE].stats_buf = NULL;
      stats_data[MSM_ISP_STATS_BE].buf_size = 0;
      stats_data[MSM_ISP_STATS_BE].used_size = 0;
    }
    break;
  }
  default:
    break;
  }
  return rc;
}

/** be_stats_init:
 *    @ctrl: Pointer to entry (ctrl) struct
 *    @in_params: input params
 *    @notify_ops: notify_ops function pointer
 *
 *  Set initial values in the be stats entry
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int be_stats_init (void *ctrl, void *in_params, isp_notify_ops_t *notify_ops)
{
  isp_stats_entry_t *entry = ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;
  ISP_StatsBe_CfgCmdType *pcmd = entry->reg_cmd;

  entry->buf_len = ISP_STATS_BE_BUF_SIZE;
  entry->fd = init_params->fd;

  entry->dev_idx = init_params->dev_idx;
  entry->buf_mgr = init_params->buf_mgr;

  /* max size for dual vfe, pass the 2nd buf offset by the size*/
  entry->buf_len = ISP_STATS_BE_BUF_SIZE * 2;

  entry->stats_type = MSM_ISP_STATS_BE;
  entry->notify_ops = notify_ops;
  be_reset(entry);
  return 0;
}

/** be_stats_destroy:
 *    @ctrl: Pointer to the entry struct for be stats
 *
 *  Free the memory for the be stats module
 *
 *
 * Return 0 on Success, negative on ERROR
 **/
static int be_stats_destroy (void *ctrl)
{
  isp_stats_entry_t *entry = ctrl;
  free(entry->reg_cmd);
  if (entry->parsed_stats_buf)
    free(entry->parsed_stats_buf);
  free(entry);
  return 0;
}

/** be_stats44_open:
 *    @stats: Pointer to the ISP stats module
 *    @stats_type: enum value that gives what stats this is
 *
 *  Allocate and Initialize memory for be stats module
 *
 *
 *  Return pointer to isp ops. NULL if ERROR
 **/
isp_ops_t *be_stats44_open(isp_stats_mod_t *stats,
                         enum msm_isp_stats_type stats_type)
{
  int rc = 0;
  isp_stats_entry_t *entry = NULL;
  ISP_StatsBe_CfgCmdType *cmd = NULL;
ISP_DBG(ISP_MOD_STATS, "%s: E\n", __func__);
  entry = malloc(sizeof(isp_stats_entry_t));
  if (!entry) {
    CDBG_ERROR("%s: no mem for aec\n",  __func__);
    return NULL;
  }
  cmd = malloc(sizeof(ISP_StatsBe_CfgCmdType));
  if (!cmd) {
    CDBG_ERROR("%s: no mem\n",  __func__);
    free(entry);
    return NULL;
  }
  memset(entry, 0, sizeof(isp_stats_entry_t));
  memset(cmd, 0, sizeof(*cmd));
  entry->len_parsed_stats_buf = sizeof(q3a_be_stats_t);
  entry->parsed_stats_buf = malloc(entry->len_parsed_stats_buf);
  if (entry->parsed_stats_buf== NULL) {
    CDBG_ERROR("%s: no mem\n",  __func__);
    free(cmd);
    free(entry);
    return NULL;
  }
  entry->reg_cmd = cmd;
  entry->ops.ctrl = (void *)entry;
  entry->ops.init = be_stats_init;
  /* destroy the module object */
  entry->ops.destroy = be_stats_destroy;
  /* set parameter */
  entry->ops.set_params = be_stats_set_params;
  /* get parameter */
  entry->ops.get_params = be_stats_get_params;
  entry->ops.action = be_stats_action;
  return &entry->ops;
}

