/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "cs_stats.h"
#include "isp_log.h"

#ifdef CS_STATS_DEBUG
#undef ISP_DBG
#define ISP_DBG ALOGE
#endif

#undef CDBG_ERROR
#define CDBG_ERROR ALOGE

#define CS_MAX_V_REGIONS 1
#define CS_MAX_H_REGIONS 1344

/** cs_cmd_debug:
 *    @pcmd: Pointer to the reg_cmd struct that needs to be dumped
 *
 *  Print the value of the parameters in CFG cmd
 *
 *
 * Return void
 **/
static void cs_cmd_debug(ISP_StatsCs_CfgType *pcmd)
{
  ISP_DBG(ISP_MOD_STATS, "%s: CS Stats Configurations\n", __func__);
  ISP_DBG(ISP_MOD_STATS, "%s: rgnHNum = %d \n", __func__, pcmd->rgnHNum);
  ISP_DBG(ISP_MOD_STATS, "%s: rgnVNum = %d \n", __func__, pcmd->rgnVNum);
  ISP_DBG(ISP_MOD_STATS, "%s: rgnWidth = %d \n", __func__, pcmd->rgnWidth);
  ISP_DBG(ISP_MOD_STATS, "%s: rgnHeight = %d \n", __func__, pcmd->rgnHeight);
  ISP_DBG(ISP_MOD_STATS, "%s: rgnHOffset = %d \n", __func__, pcmd->rgnHOffset);
  ISP_DBG(ISP_MOD_STATS, "%s: rgnVOffset = %d \n", __func__, pcmd->rgnVOffset);
  ISP_DBG(ISP_MOD_STATS, "%s: shiftBits = %d \n", __func__, pcmd->shiftBits);
}

/** cs_stats_get_h_num_adjustment:
 *
 *
 *  Programmed rgnHNum has a requirement to fulfill: when it is divided by 8,
 *  the remainder needs to lie within 4 to 7 inclusively. In dual-vfe mode,
 *  we need to adjust it so that when it's divided by 16, the remainder lies
 *  within 9 and 15 so that the remainder is big enough to be shared by both vfe's.
 *  This function returns the adjustment needed on the hNum to fulfill the requirement.
 *
 *
 * Return 0 on Success
 **/
static inline uint32_t cs_stats_get_h_num_adjustment(uint32_t h_num,
  uint32_t divider, uint32_t min_remainder)
{
  uint32_t remainder = h_num % divider;
  if (remainder < min_remainder)
    return remainder + 1;
  return 0;
}

/** cs_stats_config:
 *    @entry: Pointer to entry struct of cs stats
 *    @pix_settings: Pointer to the pipeline settings
 *    @in_param_size: Size of pix_settings
 *
 *  Configure the entry and reg_cmd for cs_stats using values passed in pix
 *  settings
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int cs_stats_config(isp_stats_entry_t *entry,
  isp_hw_pix_setting_params_t *pix_settings,
  uint32_t in_param_size)
{
  uint32_t demosaic_output_width, demosaic_output_height, rgn_width, rgn_height;
  uint32_t rgn_h_num, rgn_v_num;
  cs_stat_config_type_t *cfg = entry->private;
  ISP_StatsCs_CfgType *pcmd = entry->reg_cmd;
  cs_stat_config_type_t *priv_cfg = entry->private;

  if (!entry->enable) {
    ISP_DBG(ISP_MOD_STATS, "%s: not enabled", __func__);
    return 0;
  }

  entry->session_id = pix_settings->outputs->stream_param.session_id;
  entry->is_ispif_split = pix_settings->camif_cfg.ispif_out_info.is_split;
  if (pix_settings->camif_cfg.ispif_out_info.is_split) {
    if (!(pix_settings->outputs[0].isp_out_info.stripe_id == 0))
      entry->buf_offset = ISP_STATS_CS_BUF_SIZE;
  } else {
    entry->buf_offset = 0;
  }

  entry->ion_fd = pix_settings->ion_fd;
  entry->hfr_mode = pix_settings->camif_cfg.hfr_mode;
  entry->comp_flag = 1;

  demosaic_output_width = pix_settings->demosaic_output.last_pixel -
      pix_settings->demosaic_output.first_pixel + 1;
  demosaic_output_height = pix_settings->demosaic_output.last_line -
      pix_settings->demosaic_output.first_line + 1;

  rgn_h_num = CS_MAX_H_REGIONS;
  rgn_v_num = CS_MAX_V_REGIONS;

  /*config region size, check hw limitation*/
  rgn_width = (demosaic_output_width + rgn_h_num - 1) / rgn_h_num;
  rgn_height = demosaic_output_height / rgn_v_num;

  /*1. config rgn height and width
   2. check hw limitation*/
  rgn_width = MAX(2, rgn_width);
  rgn_width = MIN(4, rgn_width);
  pcmd->rgnWidth = rgn_width - 1;
  pcmd->rgnHeight = rgn_height - 1;

  /* 1.config region num
     2.modify according to final rgn_width &height
     3.check hw limitation*/
  rgn_h_num = demosaic_output_width / rgn_width;
  rgn_v_num = demosaic_output_height / rgn_height;
  rgn_h_num = MIN(CS_MAX_H_REGIONS, rgn_h_num);
  rgn_v_num = MIN(CS_MAX_V_REGIONS, rgn_v_num);
  pcmd->rgnHNum = rgn_h_num - 1;
  pcmd->rgnVNum = rgn_v_num - 1;

  /* Adjust rgnHNum to satisfy the hardware limitation: the programmed rgnHNum
      when divided by 8, needs to have a remainder within 4 and 7 inclusively. In dual-vfe
      mode, we need to adjust it so that when it's divided by 16, the remainder lies within
      9 and 15 so that the remainder is enough to be shared by both vfe's */
  entry->is_ispif_split = pix_settings->camif_cfg.ispif_out_info.is_split;
  pcmd->rgnHNum -= (entry->is_ispif_split) ?
    cs_stats_get_h_num_adjustment(pcmd->rgnHNum, 16, 9) :
    cs_stats_get_h_num_adjustment(pcmd->rgnHNum, 8, 4);
  rgn_h_num = pcmd->rgnHNum + 1;
  /*config offset: cs_rgn_offset + (cs_rgn_num + 1) *
    (rs_rgn_width + 1) <= image_width*/
  pcmd->rgnHOffset = (demosaic_output_width % (rgn_width * rgn_h_num)) / 2;
  pcmd->rgnVOffset = (demosaic_output_height % (rgn_height * rgn_v_num)) / 2;

  pcmd->shiftBits =
    isp_util_calculate_shift_bits(rgn_height);
  priv_cfg->shift_bits = pcmd->shiftBits;

  if (entry->is_ispif_split) {
    uint32_t end_point_stats_bound;
    uint32_t overlap = pix_settings->camif_cfg.ispif_out_info.overlap;
    isp_out_info_t *isp_out = &pix_settings->outputs[0].isp_out_info;
    /* Its possible that end point of stats boundary is falling completely within the left strip
       so the calculation for the no_regions_left will depend on the end of the stats boundary. */
    end_point_stats_bound = pcmd->rgnHOffset + (pcmd->rgnWidth + 1) * (pcmd->rgnHNum + 1);
    if (end_point_stats_bound > isp_out->right_stripe_offset + overlap)
      end_point_stats_bound = isp_out->right_stripe_offset + overlap;
    entry->buf_offset = isp_out->stripe_id * ISP_STATS_CS_BUF_SIZE;
    entry->num_left_rgns = (end_point_stats_bound > pcmd->rgnHOffset) ?
      (end_point_stats_bound - pcmd->rgnHOffset) / (pcmd->rgnWidth + 1) : 0;
    /* adjust (num_left_rgns-1) according to the mod 8 remainder within 4 to 7 requirement */
    if (entry->num_left_rgns) {
      uint32_t target_remainder = pcmd->rgnHNum % 16 / 2;
      uint32_t current_remainder = (entry->num_left_rgns - 1) % 8;
      entry->num_left_rgns -= (current_remainder >= target_remainder) ?
        current_remainder - target_remainder : current_remainder + 8 - target_remainder;
    }
    entry->num_right_rgns = (pcmd->rgnHNum + 1) - entry->num_left_rgns;
    if (isp_out->stripe_id == ISP_STRIPE_LEFT) {
      /* Make sure if number of region is zero for each side, we don't actually program 0 region */
      pcmd->rgnHNum = (entry->num_left_rgns > 0) ? entry->num_left_rgns - 1 : 4;
      pcmd->rgnHOffset = (entry->num_left_rgns > 0) ? pcmd->rgnHOffset : 2; // Default offset for CS stats
    } else { /* ISP_STRIPE_RIGHT */
      pcmd->rgnHNum = (entry->num_right_rgns > 0) ? entry->num_right_rgns - 1 : 4;
      pcmd->rgnHOffset = pcmd->rgnHOffset +
        (entry->num_left_rgns * (pcmd->rgnWidth + 1)) - isp_out->right_stripe_offset;
      pcmd->rgnHOffset = (entry->num_right_rgns > 0) ? pcmd->rgnHOffset : 2; // Default offset for CS stats
    }
  }

  entry->hw_update_pending = 1;
  return 0;
}

/** cs_stats_enable:
 *    @entry: Pointer to entry struct
 *    @in_params: input params with enable value
 *
 *  Enable the cs stats module
 *
 *
 * Return 0 on Success, negative on ERROR
 **/
static int cs_stats_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  ISP_DBG(ISP_MOD_STATS, "%s: enable = %d\n", __func__, in_params->enable);
  entry->enable = in_params->enable;
  entry->is_first = 1;
  return 0;
}

/** cs_stats_trigger_enable:
 *    @entry: Pointer to entry struct of cs stats
 *    @in_params: Input params
 *
 *  Set the trigger enable
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int cs_stats_trigger_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  ISP_DBG(ISP_MOD_STATS, "%s: trigger_enable = %d\n", __func__, in_params->enable);
  entry->trigger_enable = in_params->enable;
  return 0;
}

/** cs_stats_set_params:
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
static int cs_stats_set_params (void *ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;
  switch (param_id) {
  case ISP_STATS_SET_ENABLE:
      rc = cs_stats_enable(entry, (isp_mod_set_enable_t *)in_params);
    break;
  case ISP_STATS_SET_CONFIG:
    rc = cs_stats_config(entry,
         (isp_hw_pix_setting_params_t *)in_params, in_param_size);
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
static int cs_stats_get_params (void *ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size,
                     void *out_params, uint32_t out_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;

  ISP_DBG(ISP_MOD_STATS, "%s: param id = %d\n", __func__, param_id);
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

  case ISP_STATS_GET_CS_CONFIG: {
    uint32_t *val = out_params;
    *val = CS_MAX_H_REGIONS;
  }
    break;

  default:
    break;
  }
  return rc;
}

/** cs_stats_do_hw_update:
 *    @entry: Pointer to entry struct for cs stats
 *
 *  Method called at SOF, writes the value in reg_cmd to HW register
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int cs_stats_do_hw_update(isp_stats_entry_t *entry)
{
  int rc = 0;
  ISP_StatsCs_CfgType *pcmd = entry->reg_cmd;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];


  ISP_DBG(ISP_MOD_STATS, "%s: hw_update_pending = %d\n", __func__, entry->hw_update_pending);
  if (entry->hw_update_pending) {
    cs_cmd_debug(pcmd);
    cfg_cmd.cfg_data = (void *)entry->reg_cmd;
    cfg_cmd.cmd_len = sizeof(ISP_StatsCs_CfgType);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = CS_STATS_OFF;
    reg_cfg_cmd[0].u.rw_info.len = CS_STATS_LEN * sizeof(uint32_t);

    rc = ioctl(entry->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }

    entry->hw_update_pending = 0;
  }

  return rc;
}

/** cs_stats_parse:
 *    @entry: Pointer to entry struct for bf stats
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
static int cs_stats_parse(isp_stats_entry_t *entry,
                   void *raw_buf,
                   q3a_cs_stats_t *cs_stats,
                   uint32_t h_rgns_start,
                   uint32_t h_rgns_end,
                   uint32_t h_rgns_total,
                   uint32_t v_rgns_total)
{
  uint32_t  i, j;
  uint32_t *CSum;
  uint16_t *current_region;
  ISP_StatsCs_CfgType *pcmd = entry->reg_cmd;
  uint32_t shiftBits = pcmd->shiftBits;

  current_region = (uint16_t *)raw_buf;
  CSum = cs_stats->col_sum;
  cs_stats->num_col_sum += (h_rgns_end - h_rgns_start + 1) * v_rgns_total;
  ISP_DBG(ISP_MOD_STATS, "%s: num = %d, shiftBits = %d\n",
    __func__, cs_stats->num_col_sum, shiftBits);
  for (j = 0; j < v_rgns_total; j++) {
    CSum = CSum + h_rgns_start;
    for (i = h_rgns_start; i <= h_rgns_end; i++)
      *CSum++ = (*current_region++) << shiftBits;
    CSum = CSum + h_rgns_total - h_rgns_end - 1;
  }
  return 0;
}

/** cs_stats_action:
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
static int cs_stats_action (void *ctrl, uint32_t action_code,
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
    ISP_StatsCs_CfgType *pcmd = entry->reg_cmd;
    int buf_idx = action_data->raw_stats_event->
                  u.stats.stats_buf_idxs[MSM_ISP_STATS_CS];
    void *raw_buf = isp_get_buf_addr(entry->buf_mgr,
      entry->buf_handle, buf_idx);
    if(!raw_buf){
      CDBG_ERROR("%s: isp_get_buf_addr failed!\n", __func__);
      return -1;
    }

    if (entry->is_first == 1) {
       ISP_DBG(ISP_MOD_STATS, "%s: drop first stats\n", __func__);
       entry->is_first = 0;
       isp_stats_enqueue_buf(entry, buf_idx);
       return rc;
    }

    q3a_cs_stats_t *cs_stats = entry->parsed_stats_buf;
    isp_stats_event->stats_mask |= (1 << MSM_ISP_STATS_CS);
    memset(cs_stats, 0, sizeof(q3a_cs_stats_t));
    if (!entry->is_ispif_split)
      rc = cs_stats_parse(entry, raw_buf, cs_stats,
        0,
        pcmd->rgnHNum,
        pcmd->rgnHNum + 1,
        pcmd->rgnVNum + 1);
    else { /* ISP is outputting two split halves */
      /* Parse for left stripe */
      if (entry->num_left_rgns)
        rc = cs_stats_parse(entry, raw_buf, cs_stats,
          0,
          entry->num_left_rgns - 1,
          entry->num_left_rgns + entry->num_right_rgns,
          pcmd->rgnVNum + 1);
      /* Parse for right stripe */
      if (entry->num_right_rgns)
        rc = cs_stats_parse(entry, (uint8_t*)raw_buf + entry->buf_len / 2, cs_stats,
          entry->num_left_rgns,
          entry->num_left_rgns + entry->num_right_rgns - 1,
          entry->num_left_rgns + entry->num_right_rgns,
          pcmd->rgnVNum + 1);
    }
    if (entry->num_bufs !=0) {
       rc = isp_stats_enqueue_buf(entry, buf_idx);
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
 *    @ctrl: Pointer to entry (ctrl) struct
 *    @in_params: input params
 *    @notify_ops: notify_ops function pointer
 *
 *  Set initial values in the cs stats entry
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int cs_stats_init(void *ctrl, void *in_params, isp_notify_ops_t *notify_ops)
{
  isp_stats_entry_t *entry = ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  /* max size for dual vfe, pass the 2nd buf offset by the size*/
  entry->buf_len = ISP_STATS_CS_BUF_SIZE * 2;

  entry->stats_type = MSM_ISP_STATS_CS;
  entry->fd = init_params->fd;
  entry->notify_ops = notify_ops;
  entry->dev_idx = init_params->dev_idx;
  entry->buf_mgr = init_params->buf_mgr;
  isp_stats_reset(entry);
  return 0;
}

/** cs_stats_destroy:
 *    @ctrl: Pointer to the entry struct for cs stats
 *
 *  Free the memory for the cs stats module
 *
 *
 * Return 0 on Success, negative on ERROR
 **/
static int cs_stats_destroy (void *ctrl)
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

/** cs_stats_open:
 *    @stats: Pointer to the ISP stats module
 *    @stats_type: enum value that gives what stats this is
 *
 *  Allocate and Initialize memory for cs stats module
 *
 *
 *  Return pointer to isp ops. NULL if ERROR
 **/
isp_ops_t *cs_stats_open(isp_stats_mod_t *stats,
                                     enum msm_isp_stats_type stats_type)
{
  int rc = 0;
  isp_stats_entry_t *entry = NULL;
  ISP_StatsCs_CfgType *cmd = NULL;
  cs_stat_config_type_t *cfg = NULL;

  ISP_DBG(ISP_MOD_STATS, "%s: E.\n", __func__);

  entry = malloc(sizeof(isp_stats_entry_t));
  if (!entry) {
    CDBG_ERROR("%s: no mem for aec\n",  __func__);
      return NULL;
  }

  cmd = malloc(sizeof(ISP_StatsCs_CfgType));
  if (!cmd) {
    CDBG_ERROR("%s: no mem\n",  __func__);
      free(entry);
      return NULL;
  }

  cfg = malloc(sizeof(cs_stat_config_type_t));
  if (!cfg) {
    CDBG_ERROR("%s: no mem\n",  __func__);
      free(entry);
      free(cmd);
      return NULL;
  }
  memset(entry, 0, sizeof(isp_stats_entry_t));
  memset(cmd, 0, sizeof(*cmd));
  memset(cfg, 0, sizeof(*cfg));

  entry->len_parsed_stats_buf = sizeof(q3a_cs_stats_t);
  entry->parsed_stats_buf = malloc(entry->len_parsed_stats_buf);
  if (entry->parsed_stats_buf == NULL) {
    CDBG_ERROR("%s: no mem\n",  __func__);
    free(cfg);
    free(cmd);
    free(entry);
    return NULL;
  }

  entry->reg_cmd = cmd;
  entry->private = (void *)cfg;
  entry->ops.ctrl = (void *)entry;
  entry->ops.init = cs_stats_init;
  /* destroy the module object */
  entry->ops.destroy = cs_stats_destroy;
  /* set parameter */
  entry->ops.set_params = cs_stats_set_params;
  /* get parameter */
  entry->ops.get_params = cs_stats_get_params;
  entry->ops.action = cs_stats_action;
  return &entry->ops;
}


