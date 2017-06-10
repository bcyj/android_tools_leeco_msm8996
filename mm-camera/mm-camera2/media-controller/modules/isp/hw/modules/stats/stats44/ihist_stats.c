/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "ihist_stats.h"
#include "isp_log.h"

#ifdef IHIST_STATS_DEBUG
#undef ISP_DBG
#define ISP_DBG ALOGE
#endif

#undef CDBG_ERROR
#define CDBG_ERROR ALOGE

/** ihist_stats_get_shiftbits:
 *    @entry: Pointer to entry struct for awb
 *
 *   Get the shiftbits from CFG cmd in entry
 *
 *
 *  Return integer containing shiftbits
 **/
static uint32_t ihist_stats_get_shiftbits(ISP_StatsIhist_CfgType *pcmd)
{
  return pcmd->shiftBits;
}

/** ihist_stats_config:
 *    @entry: Pointer to entry struct of ihist stats
 *    @pix_settings: Pointer to the pipeline settings
 *    @in_param_size: Size of pix_settings
 *
 *  Configure the entry and reg_cmd for ihist_stats using values passed in pix
 *  settings
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int ihist_stats_config(isp_stats_entry_t *entry,
  isp_hw_pix_setting_params_t *pix_settings,
  uint32_t in_param_size)
{
  ISP_StatsIhist_CfgType *pcmd = entry->reg_cmd;
  uint32_t window_w_t, window_h_t, total_pixels;
  isp_pix_camif_cfg_t *camif_cfg = &pix_settings->camif_cfg;
  int32_t shift_bits;

  if (!entry->enable) {
    ISP_DBG(ISP_MOD_STATS, "%s: ihist not enabled", __func__);
    return 0;
  }

  entry->session_id = pix_settings->outputs->stream_param.session_id;
  entry->is_ispif_split = pix_settings->camif_cfg.ispif_out_info.is_split;
  if (pix_settings->camif_cfg.ispif_out_info.is_split) {
     if (pix_settings->outputs[0].isp_out_info.stripe_id == 0)
       entry->buf_offset = 0;
     else
       entry->buf_offset = ISP_STATS_IHIST_BUF_SIZE;
  }

  entry->ion_fd = pix_settings->ion_fd;
  entry->hfr_mode = pix_settings->camif_cfg.hfr_mode;
  entry->comp_flag = 1;

  if (pix_settings->outputs[0].stream_param.width > 0) {
    window_w_t = camif_cfg->sensor_out_info.request_crop.last_pixel -
                   camif_cfg->sensor_out_info.request_crop.first_pixel + 1;
    window_h_t = camif_cfg->sensor_out_info.request_crop.last_line -
                     camif_cfg->sensor_out_info.request_crop.first_line + 1;
  } else {
     CDBG_ERROR("%s: error, width = 0\n", __func__);
     return -1;
  }
  pcmd->channelSelect = 0;
  pcmd->rgnHNum =
    FLOOR16(window_w_t/2)-1;

  pcmd->rgnVNum =
    FLOOR16(window_h_t/2)-1;
  pcmd->rgnHOffset = 0;
  pcmd->rgnVOffset = 0;
  /* calculate shift bits */
  total_pixels = (float)((pcmd->rgnHNum + 1) * (pcmd->rgnVNum + 1)) / 2.0;
  shift_bits = CEIL_LOG2(total_pixels);
  shift_bits -= 16;
  shift_bits = MAX(0, shift_bits);
  shift_bits = MIN(4, shift_bits);
  ISP_DBG(ISP_MOD_STATS, "%s: tot %d shift %d", __func__, total_pixels, shift_bits);

  pcmd->shiftBits = shift_bits;
  pcmd->siteSelect = 0;

  entry->is_ispif_split = pix_settings->camif_cfg.ispif_out_info.is_split;
  if (entry->is_ispif_split) {
    uint32_t end_point_stats_bound;
    uint32_t overlap = pix_settings->camif_cfg.ispif_out_info.overlap;
    isp_out_info_t *isp_out = &pix_settings->outputs[0].isp_out_info;
    /* Its possible that end point of stats boundary is falling completely within the left strip
       so the calculation for the no_regions_left will depend on the end of the stats boundary. */
    end_point_stats_bound = pcmd->rgnHOffset + 2 * (pcmd->rgnHNum + 1);
    if (end_point_stats_bound > isp_out->right_stripe_offset + overlap)
      end_point_stats_bound = isp_out->right_stripe_offset + overlap;
    entry->buf_offset = isp_out->stripe_id * ISP_STATS_IHIST_BUF_SIZE;
    entry->num_left_rgns = (end_point_stats_bound > pcmd->rgnHOffset) ?
      (end_point_stats_bound - pcmd->rgnHOffset) / 2 : 0;
    entry->num_right_rgns = (pcmd->rgnHNum + 1) - entry->num_left_rgns;
    if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
      /* Make sure if number of region is zero for each side, we don't actually program 0 region */
      pcmd->rgnHNum = (entry->num_left_rgns > 0) ? entry->num_left_rgns - 1 : 1;
      pcmd->rgnHOffset = (entry->num_left_rgns > 0) ? pcmd->rgnHOffset : 2; // Default offset for IHIST stats
    } else { /* ISP_STRIPE_RIGHT */
      pcmd->rgnHNum = (entry->num_right_rgns > 0) ? entry->num_right_rgns - 1 : 1;
      pcmd->rgnHOffset = pcmd->rgnHOffset + (entry->num_left_rgns * 2) -
        isp_out->right_stripe_offset;
      pcmd->rgnHOffset = (entry->num_right_rgns > 0) ? pcmd->rgnHOffset : 2; // Default offset for IHIST stats
    }
  }

  ISP_DBG(ISP_MOD_STATS, "IHIST statsconfig shiftBits %d\n",
    pcmd->shiftBits);
  ISP_DBG(ISP_MOD_STATS, "IHIST statsconfig channelSelect  %d\n",
    pcmd->channelSelect);
  ISP_DBG(ISP_MOD_STATS, "IHIST statsconfig siteSelect %d\n",
    pcmd->siteSelect);
  ISP_DBG(ISP_MOD_STATS, "IHIST statsconfig rgnHNum   %d\n",
    pcmd->rgnHNum);
  ISP_DBG(ISP_MOD_STATS, "IHIST statsconfig rgnVNum   %d\n",
    pcmd->rgnVNum);
  entry->hw_update_pending = 1;
  return 0;
}

/** ihist_stats_enable:
 *    @entry: Pointer to entry struct
 *    @in_params: input params with enable value
 *
 *  Enable the ihist stats module
 *
 *
 * Return 0 on Success, negative on ERROR
 **/
static int ihist_stats_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  ISP_DBG(ISP_MOD_STATS, "%s: enable = %d\n", __func__, in_params->enable);
  entry->enable = in_params->enable;
  /* after enable the first stats is corrupted */
  entry->is_first = 1;
  return 0;
}

/** ihist_stats_trigger_enable:
 *    @entry: Pointer to entry struct of ihist stats
 *    @in_params: Input params
 *
 *  Set the trigger enable
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int ihist_stats_trigger_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  ISP_DBG(ISP_MOD_STATS, "%s: trigger_enable = %d\n", __func__, in_params->enable);
  entry->trigger_enable = in_params->enable;
  return 0;
}

/** ihist_stats_set_params:
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
static int ihist_stats_set_params (void *ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;
  switch (param_id) {
  case ISP_STATS_SET_ENABLE:
    rc = ihist_stats_enable(entry, (isp_mod_set_enable_t *)in_params);
    break;
  case ISP_STATS_SET_CONFIG:
    rc = ihist_stats_config(entry,
     (isp_hw_pix_setting_params_t *)in_params, in_param_size);
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
static int ihist_stats_get_params (void *ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size,
                     void *out_params, uint32_t out_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;

  ISP_DBG(ISP_MOD_STATS, "%s: param is = %d\n", __func__, param_id);
  switch (param_id) {
  case ISP_STATS_GET_ENABLE:
    break;
  case ISP_STATS_GET_STREAM_STATE:
    break;
  case ISP_STATS_GET_PARSED_STATS:
    break;
  case ISP_STATS_GET_STREAM_HANDLE:{
    uint32_t *handle = (uint32_t *) (out_params);
    *handle = entry->stream_handle;
    break;
  }
  default:
    break;
  }
  return rc;
}

/** ihist_stats_do_hw_update:
 *    @entry: Pointer to entry struct for ihist stats
 *
 *  Method called at SOF, writes the value in reg_cmd to HW register
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int ihist_stats_do_hw_update(isp_stats_entry_t *entry)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  if (entry->hw_update_pending) {
    cfg_cmd.cfg_data = (void *)entry->reg_cmd;
    cfg_cmd.cmd_len = sizeof(ISP_StatsIhist_CfgType);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = IHIST_STATS_OFF;
    reg_cfg_cmd[0].u.rw_info.len = IHIST_STATS_LEN * sizeof(uint32_t);

    rc = ioctl(entry->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }

    entry->hw_update_pending = 0;
  }

  return rc;
}

/** ihist_stats_parse:
 *    @entry: Pointer to entry struct for ihist stats
 *    @raw_buf: Pointer to raw buf containing the stats recevied from HW
 *    @ihist_stats: Ihist Stats struct that needs to be filled with parsed stats
 *
 *  Parse the different stats parameters received from raw_buf to their
 *  corresponding struct
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int ihist_stats_parse(isp_stats_entry_t *entry,
                   void *raw_buf,
                   q3a_ihist_stats_t *ihist_stats)
{
  int i;
  uint16_t *hist_statsBuffer = NULL;

  ISP_DBG(ISP_MOD_STATS, "%s: E\n", __func__);
  hist_statsBuffer = (uint16_t *)raw_buf;
  for (i= 0; i< 256; i++) {
    ihist_stats->histogram[i] += *hist_statsBuffer;
    hist_statsBuffer++;
  }

  return 0;
}

/** ihist_stats_action:
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
static int ihist_stats_action (void *ctrl, uint32_t action_code,
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
    isp_stats_reset(entry);
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
    memset(ihist_stats, 0, sizeof(q3a_ihist_stats_t));
    if (!entry->is_ispif_split)
      rc = ihist_stats_parse(entry, raw_buf, ihist_stats);
    else { /* ISP is outputting two split halves */
      /* Parse for left stripe */
      if (entry->num_left_rgns)
        rc = ihist_stats_parse(entry, raw_buf, ihist_stats);
      /* Parse for right stripe */
      if (entry->num_right_rgns)
        rc = ihist_stats_parse(entry, (uint8_t*)raw_buf + entry->buf_len / 2, ihist_stats);
    }
    if (entry->num_bufs !=0) {
       rc = isp_stats_enqueue_buf(entry, buf_idx);
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
 *    @ctrl: Pointer to entry (ctrl) struct
 *    @in_params: input params
 *    @notify_ops: notify_ops function pointer
 *
 *  Set initial values in the ihist stats entry
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int ihist_stats_init (void *ctrl, void *in_params, isp_notify_ops_t *notify_ops)
{
  isp_stats_entry_t *entry = ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  /* max size for dual vfe, pass the 2nd buf offset by the size*/
  entry->buf_len = ISP_STATS_IHIST_BUF_SIZE * 2;

  entry->stats_type = MSM_ISP_STATS_IHIST;

  entry->fd = init_params->fd;
  entry->dev_idx = init_params->dev_idx;
  entry->buf_mgr = init_params->buf_mgr;
  entry->notify_ops = notify_ops;
  isp_stats_reset(entry);
  return 0;
}

/** ihist_stats_destroy:
 *    @ctrl: Pointer to the entry struct for ihist stats
 *
 *  Free the memory for the ihist stats module
 *
 *
 * Return 0 on Success, negative on ERROR
 **/
static int ihist_stats_destroy (void *ctrl)
{
  isp_stats_entry_t *entry = ctrl;

  if (entry) {
    free(entry->parsed_stats_buf);
    entry->parsed_stats_buf = NULL;

    free(entry->reg_cmd);
    entry->reg_cmd = NULL;

    free(entry);
  }

  return 0;
}

/** ihist_stats44_open:
 *    @stats: Pointer to the ISP stats module
 *    @stats_type: enum value that gives what stats this is
 *
 *  Allocate and Initialize memory for ihist stats module
 *
 *
 *  Return pointer to isp ops. NULL if ERROR
 **/
isp_ops_t *ihist_stats44_open(isp_stats_mod_t *stats,
                                     enum msm_isp_stats_type stats_type)
{
  int rc = 0;
  isp_stats_entry_t *entry = NULL;
  ISP_StatsIhist_CfgType *cmd = NULL;
  ISP_DBG(ISP_MOD_STATS, "%s: E\n", __func__);
  entry = malloc(sizeof(isp_stats_entry_t));
  if (!entry) {
    CDBG_ERROR("%s: no mem for aec\n",  __func__);
      return NULL;
  }
  cmd = malloc(sizeof(ISP_StatsIhist_CfgType));
  if (!cmd) {
    CDBG_ERROR("%s: no mem\n",  __func__);
      free(entry);
      return NULL;
  }
  memset(entry, 0, sizeof(isp_stats_entry_t));
  memset(cmd, 0, sizeof(*cmd));
  entry->len_parsed_stats_buf = sizeof(q3a_ihist_stats_t);
  entry->parsed_stats_buf = malloc(entry->len_parsed_stats_buf);
  if (entry->parsed_stats_buf== NULL) {
    CDBG_ERROR("%s: no mem\n",  __func__);
    free(cmd);
    free(entry);
    return NULL;
  }
  entry->reg_cmd = cmd;
  entry->ops.ctrl = (void *)entry;
  entry->ops.init = ihist_stats_init;
  /* destroy the module object */
  entry->ops.destroy = ihist_stats_destroy;
  /* set parameter */
  entry->ops.set_params = ihist_stats_set_params;
  /* get parameter */
  entry->ops.get_params = ihist_stats_get_params;
  entry->ops.action = ihist_stats_action;
  return &entry->ops;
}


