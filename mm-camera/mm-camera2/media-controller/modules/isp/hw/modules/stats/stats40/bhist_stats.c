/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "bhist_stats.h"
#include "isp_log.h"

#ifdef BHIST_STATS_DEBUG
#undef ISP_DBG
#define ISP_DBG ALOGE
#endif

#undef CDBG_ERROR
#define CDBG_ERROR ALOGE

#define BHIST_RGN_WIDTH 2
#define BHIST_RGN_HEIGHT 2

/** bhist_stats_debug:
 *    @pcmd: Pointer to the reg_cmd struct that needs to be dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 *
 * Return void
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
 *    @entry: Pointer to entry struct of bhist stats
 *    @pix_settings: Pointer to the pipeline settings
 *    @in_param_size: Size of pix_settings
 *
 *  Configure the entry and reg_cmd for bhist_stats using values passed in pix
 *  settings
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int bhist_stats_config(isp_stats_entry_t *entry,
  isp_hw_pix_setting_params_t *pix_settings,
  uint32_t in_param_size)
{
  ISP_StatsBhist_CfgCmdType *pcmd = entry->reg_cmd;
  aec_bhist_config_t *bhist_config = &pix_settings->stats_cfg.aec_config.bhist_config;

  if (!entry->enable) {
    ISP_DBG(ISP_MOD_STATS, "%s: Bhist stats not enabled", __func__);
    return 0;
  }

  entry->session_id = pix_settings->outputs->stream_param.session_id;
  entry->is_ispif_split = pix_settings->camif_cfg.ispif_out_info.is_split;
  if (pix_settings->camif_cfg.ispif_out_info.is_split) {
    if (!(pix_settings->outputs[0].isp_out_info.stripe_id == 0))
      entry->buf_offset = ISP_STATS_BHIST_BUF_SIZE;
  } else {
    entry->buf_offset = 0;
  }

  entry->ion_fd = pix_settings->ion_fd;
  entry->hfr_mode = pix_settings->camif_cfg.hfr_mode;
  entry->comp_flag = 1;

  pcmd->rgnHOffset = FLOOR2(bhist_config->roi.left % BHIST_RGN_WIDTH);
  pcmd->rgnVOffset = FLOOR2(bhist_config->roi.top % BHIST_RGN_HEIGHT);
  pcmd->rgnHNum    = FLOOR2(bhist_config->roi.width / BHIST_RGN_WIDTH) - 1;
  pcmd->rgnVNum    = FLOOR2(bhist_config->roi.height / BHIST_RGN_HEIGHT) - 1;
  bhist_stats_debug(pcmd);

  /* update aecConfig to reflect the new config    *
     It will be sent to 3A in STATS_NOTIFY event  */
  bhist_config->roi.left = pcmd->rgnHOffset;
  bhist_config->roi.top = pcmd->rgnVOffset;
  bhist_config->grid_info.h_num = pcmd->rgnHNum;
  bhist_config->grid_info.v_num = pcmd->rgnVNum;

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
    entry->buf_offset = isp_out->stripe_id * ISP_STATS_BHIST_BUF_SIZE;
    entry->num_left_rgns = (end_point_stats_bound > pcmd->rgnHOffset) ?
      (end_point_stats_bound - pcmd->rgnHOffset) / 2 : 0;
    entry->num_right_rgns = (pcmd->rgnHNum + 1) - entry->num_left_rgns;
    if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
      /* Make sure if number of region is zero for each side, we don't actually program 0 region */
      pcmd->rgnHNum = (entry->num_left_rgns > 0) ? entry->num_left_rgns - 1 : 1;
      pcmd->rgnHOffset = (entry->num_left_rgns > 0) ? pcmd->rgnHOffset : 2; // Default offset for BHIST STATS
    } else { /* ISP_STRIPE_RIGHT */
      pcmd->rgnHNum = (entry->num_right_rgns > 0) ? entry->num_right_rgns - 1 : 1;
      pcmd->rgnHOffset = pcmd->rgnHOffset + (entry->num_left_rgns * 2) -
        isp_out->right_stripe_offset;
      pcmd->rgnHOffset = (entry->num_right_rgns > 0) ? pcmd->rgnHOffset : 2; // Default offset for BHIST STATS
    }
  }

  entry->hw_update_pending = 1;
  return 0;
}

/** bhist_stats_enable:
 *    @entry: Pointer to entry struct
 *    @in_params: input params with enable value
 *
 *  Enable the bhist stats module
 *
 *
 * Return 0 on Success, negative on ERROR
 **/
static int bhist_stats_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  ISP_DBG(ISP_MOD_STATS, "%s: enable = %d\n", __func__, in_params->enable);
  entry->enable = in_params->enable;
  entry->is_first = 1;
  return 0;
}

/** bhist_stats_trigger_enable:
 *    @entry: Pointer to entry struct of bhist stats
 *    @in_params: Input params
 *
 *  Set the trigger enable
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int bhist_stats_trigger_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  ISP_DBG(ISP_MOD_STATS, "%s: trigger_enable = %d\n", __func__, in_params->enable);
  entry->trigger_enable = in_params->enable;
  return 0;
}

/** bhist_stats_set_params:
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
static int bhist_stats_set_params (void *ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;

  switch (param_id) {
  case ISP_STATS_SET_ENABLE:
    rc = bhist_stats_enable(entry, (isp_mod_set_enable_t *)in_params);
    break;
  case ISP_STATS_SET_CONFIG:
    rc = bhist_stats_config(entry,
     (isp_hw_pix_setting_params_t *)in_params, in_param_size);
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
static int bhist_stats_get_params (void *ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size,
                     void *out_params, uint32_t out_param_size)
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

/** bhist_stats_do_hw_update:
 *    @entry: Pointer to entry struct for bhist stats
 *
 *  Method called at SOF, writes the value in reg_cmd to HW register
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int bhist_stats_do_hw_update(isp_stats_entry_t *entry)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  if (entry->hw_update_pending) {
    cfg_cmd.cfg_data = (void *)entry->reg_cmd;
    cfg_cmd.cmd_len = sizeof(ISP_StatsBhist_CfgCmdType);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = BHIST_STATS_OFF;
    reg_cfg_cmd[0].u.rw_info.len = BHIST_STATS_LEN * sizeof(uint32_t);

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

/** bhist_stats_parse:
 *    @entry: Pointer to entry struct for bhist stats
 *    @raw_buf: Pointer to raw buf containing the stats recevied from HW
 *    @bhist_stats: BHist Stats struct that needs to be filled with parsed stats
 *
 *  Parse the different stats parameters received from raw_buf to their
 *  corresponding struct
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int bhist_stats_parse(isp_stats_entry_t *entry,
  void *raw_buf,
  q3a_bhist_stats_t *bhist_stats)
{
  uint32_t * Srh,*Sbh,*Sgrh,*Sgbh;
  uint32_t *current_region;
  uint32_t  i;

  Srh = bhist_stats->bayer_r_hist;
  Sbh = bhist_stats->bayer_b_hist;
  Sgrh = bhist_stats->bayer_gr_hist;
  Sgbh = bhist_stats->bayer_gb_hist;
  current_region = raw_buf;
  for (i = 0; i < MAX_HIST_STATS_NUM; i++) { //0 to 255, total 256 bins
    *Srh += ((*(current_region)) & 0x007FFFFF);
    Srh++; current_region++;
    *Sbh += ((*(current_region)) & 0x007FFFFF);
    Sbh++; current_region++;
    *Sgrh += ((*(current_region)) & 0x007FFFFF);
    Sgrh++; current_region++;
    *Sgbh += ((*(current_region)) & 0x007FFFFF);
    Sgbh++; current_region++;
  }
  return 0;
}

/** bhist_stats_action:
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
static int bhist_stats_action (void *ctrl, uint32_t action_code,
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
    rc = bhist_stats_do_hw_update(entry);
    break;
  case ISP_STATS_ACTION_STREAM_BUF_CFG:
    rc = isp_stats_config_stats_stream(entry, ISP_STATS_BHIST_BUF_NUM);
    break;
  case ISP_STATS_ACTION_STREAM_BUF_UNCFG:
    rc = isp_stats_unconfig_stats_stream(entry);
    break;
  case ISP_STATS_ACTION_STATS_PARSE:
    {
      isp_pipeline_stats_parse_t *action_data = data;
      mct_event_stats_isp_t *isp_stats_event = action_data->parsed_stats_event;
      mct_event_stats_isp_data_t *stats_data = isp_stats_event->stats_data;
      int buf_idx = action_data->raw_stats_event->
        u.stats.stats_buf_idxs[MSM_ISP_STATS_BHIST];
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
      memset(bhist_stats, 0, sizeof(q3a_bhist_stats_t));
      if (!entry->is_ispif_split)
        rc = bhist_stats_parse(entry, raw_buf, bhist_stats);
      else { /* ISP is outputting two split halves */
        /* Parse for left stripe */
        if (entry->num_left_rgns)
          rc = bhist_stats_parse(entry, raw_buf, bhist_stats);
        /* Parse for right stripe */
        if (entry->num_right_rgns)
          rc = bhist_stats_parse(entry, (uint8_t*)raw_buf + entry->buf_len / 2, bhist_stats);
      }
      if (entry->num_bufs !=0) {
         rc = isp_stats_enqueue_buf(entry, buf_idx);
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
 *    @ctrl: Pointer to entry (ctrl) struct
 *    @in_params: input params
 *    @notify_ops: notify_ops function pointer
 *
 *  Set initial values in the bhist stats entry
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int bhist_stats_init (void *ctrl, void *in_params, isp_notify_ops_t *notify_ops)
{
  isp_stats_entry_t *entry = ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;
  ISP_StatsBhist_CfgCmdType *pcmd = entry->reg_cmd;

  /* max size for dual vfe, pass the 2nd buf offset by the size*/
  entry->buf_len = ISP_STATS_BHIST_BUF_SIZE * 2;

  entry->stats_type = MSM_ISP_STATS_BHIST;
  entry->fd = init_params->fd;
  entry->notify_ops = notify_ops;
  entry->dev_idx = init_params->dev_idx;
  entry->buf_mgr = init_params->buf_mgr;
  entry->skip_stats = 0;

  isp_stats_reset(entry);
  return 0;
}

/** bhist_stats_destroy:
 *    @ctrl: Pointer to the entry struct for bhist stats
 *
 *  Free the memory for the bhist stats module
 *
 *
 * Return 0 on Success, negative on ERROR
 **/
static int bhist_stats_destroy (void *ctrl)
{
  isp_stats_entry_t *entry = ctrl;
  free(entry->reg_cmd);
  if (entry->parsed_stats_buf)
    free(entry->parsed_stats_buf);
  free(entry);
  return 0;
}

/** bhist_stats_open:
 *    @stats: Pointer to the ISP stats module
 *    @stats_type: enum value that gives what stats this is
 *
 *  Allocate and Initialize memory for bhist stats module
 *
 *
 *  Return pointer to isp ops. NULL if ERROR
 **/
isp_ops_t *bhist_stats_open(isp_stats_mod_t *stats,
                                     enum msm_isp_stats_type stats_type)
{
  int rc = 0;
  isp_stats_entry_t *entry = NULL;
  ISP_StatsBhist_CfgCmdType *cmd = NULL;

  ISP_DBG(ISP_MOD_STATS, "%s: E\n", __func__);
  entry = malloc(sizeof(isp_stats_entry_t));
  if (!entry) {
    CDBG_ERROR("%s: no mem for aec\n",  __func__);
      return NULL;
  }

  cmd = malloc(sizeof(ISP_StatsBhist_CfgCmdType));
  if (!cmd) {
    CDBG_ERROR("%s: no mem\n",  __func__);
      free(entry);
      return NULL;
  }
  memset(entry, 0, sizeof(isp_stats_entry_t));
  memset(cmd, 0, sizeof(*cmd));
  entry->len_parsed_stats_buf = sizeof(q3a_bhist_stats_t);
  entry->parsed_stats_buf = malloc(entry->len_parsed_stats_buf);
  if (entry->parsed_stats_buf == NULL) {
    CDBG_ERROR("%s: no mem\n",  __func__);
    free(cmd);
    free(entry);
    return NULL;
  }
  entry->reg_cmd = cmd;
  entry->ops.ctrl = (void *)entry;
  entry->ops.init = bhist_stats_init;
  /* destroy the module object */
  entry->ops.destroy = bhist_stats_destroy;
  /* set parameter */
  entry->ops.set_params = bhist_stats_set_params;
  /* get parameter */
  entry->ops.get_params = bhist_stats_get_params;
  entry->ops.action = bhist_stats_action;
  return &entry->ops;
}


