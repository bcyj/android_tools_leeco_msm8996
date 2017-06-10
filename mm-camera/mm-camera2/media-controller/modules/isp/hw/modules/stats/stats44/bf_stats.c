/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "bf_stats.h"
#include "isp_log.h"

#ifdef BF_STATS_DEBUG
#undef ISP_DBG
#define ISP_DBG ALOGE
#endif

#undef CDBG_ERROR
#define CDBG_ERROR ALOGE

#ifdef BF_DUMP
#undef CDBG_DUMP
#define CDBG_DUMP ALOGE
#endif

/** bf_stats_debug:
 *    @pcmd: Pointer to the reg_cmd struct that needs to be dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 *
 * Return void
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

/** bf_stats_check_stream_path:
 *    @pix_settings: Pointer to Pipeline settings
 *    @stream_id: input stream id
 *
 *  Find the ISP PIX PATH for stream given by input stream id
 *
 *
 * Return integer giving PIX PATH for stream
 **/
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
 *    @entry: Pointer to entry struct of bf stats
 *    @pix_settings: Pointer to the pipeline settings
 *    @in_param_size: Size of pix_settings
 *
 *  Configure the entry and reg_cmd for bf_stats using values passed in pix
 *  settings
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int bf_stats_config(isp_stats_entry_t *entry,
  isp_hw_pix_setting_params_t *pix_settings,
  uint32_t in_param_size)
{
  int rc = 0;
  ISP_StatsBf_CfgCmdType *pcmd = entry->reg_cmd;
  af_config_t *af_config = &pix_settings->stats_cfg.af_config;
  isp_pix_camif_cfg_t *camif_cfg = &pix_settings->camif_cfg;
  isp_pixel_window_info_t scaler_output[ISP_PIX_PATH_MAX];
  isp_pixel_line_info_t fov_output[ISP_PIX_PATH_MAX];
  uint32_t af_rgn_width, af_rgn_height;
  uint32_t path_idx;
  int i;

  if (!entry->enable) {
    ISP_DBG(ISP_MOD_STATS, "%s: BF not enabled", __func__);
    return 0;
  }

  entry->session_id = pix_settings->outputs->stream_param.session_id;
  entry->is_ispif_split = pix_settings->camif_cfg.ispif_out_info.is_split;
  if (pix_settings->camif_cfg.ispif_out_info.is_split) {
     if (pix_settings->outputs[0].isp_out_info.stripe_id == 0)
       entry->buf_offset = 0;
     else
       entry->buf_offset = ISP_STATS_BF_BUF_SIZE;
  }

  entry->ion_fd = pix_settings->ion_fd;
  entry->hfr_mode = pix_settings->camif_cfg.hfr_mode;
  entry->comp_flag = 0;

  if (pix_settings->stats_cfg.af_config.grid_info.h_num ==0 ||
    pix_settings->stats_cfg.af_config.grid_info.v_num == 0){
    CDBG_ERROR("%s: Invalid BF h_num & v_num from 3A, h_num = %d, v_num = %d\n",
      __func__, pix_settings->stats_cfg.af_config.grid_info.h_num,
      pix_settings->stats_cfg.af_config.grid_info.v_num);
    return -1;
  }

  af_rgn_width =
    af_config->roi.width / af_config->grid_info.h_num;
  af_rgn_height =
    af_config->roi.height / af_config->grid_info.v_num;

  /*min of rgn_v_offset = 2, min of rgn_h_offset = 8
    rgn_v_offset = 2 + roi_top,  rgn_h_offset = 8 + roi_left*/
  pcmd->rgnHOffset = FLOOR2(8 + af_config->roi.left);
  pcmd->rgnVOffset = FLOOR2(2 + af_config->roi.top);
  pcmd->rgnWidth   = FLOOR2(af_rgn_width) - 1;
  pcmd->rgnHeight  = FLOOR2(af_rgn_height) - 1;
  pcmd->rgnHNum    = af_config->grid_info.h_num - 1;
  pcmd->rgnVNum    = af_config->grid_info.v_num - 1;
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

/* update afConfig to reflect the new config    *
   It will be sent to 3A in STATS_NOTIFY event  */
  af_config->roi.left = pcmd->rgnHOffset;
  af_config->roi.top = pcmd->rgnVOffset;
  af_config->roi.width = (pcmd->rgnWidth + 1) * af_config->grid_info.h_num;
  af_config->roi.height = (pcmd->rgnHeight + 1) * af_config->grid_info.v_num;

  if (entry->is_ispif_split) {
    uint32_t end_point_stats_bound;
    uint32_t overlap = pix_settings->camif_cfg.ispif_out_info.overlap;
    isp_out_info_t *isp_out = &pix_settings->outputs[0].isp_out_info;
    /* Its possible that end point of stats boundary is falling completely within the left stripe
       so the calculation for the no_regions_left will depend on the end of the stats boundary. */
    end_point_stats_bound = pcmd->rgnHOffset + (pcmd->rgnWidth + 1) * (pcmd->rgnHNum + 1);
    if (end_point_stats_bound > isp_out->right_stripe_offset + overlap)
      end_point_stats_bound = isp_out->right_stripe_offset + overlap;
    entry->buf_offset = isp_out->stripe_id * ISP_STATS_BF_BUF_SIZE;
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
      pcmd->rgnHOffset = (entry->num_left_rgns > 0) ? pcmd->rgnHOffset : 8; // Default offset for BF stats
    } else { /* ISP_STRIPE_RIGHT */
      pcmd->rgnHNum = (entry->num_right_rgns > 0) ? entry->num_right_rgns - 1 : 1;
      pcmd->rgnHOffset = pcmd->rgnHOffset +
        (entry->num_left_rgns * (pcmd->rgnWidth + 1)) - isp_out->right_stripe_offset;
      pcmd->rgnHOffset = (entry->num_right_rgns > 0) ? pcmd->rgnHOffset : 8; // Default offset for BF stats
    }
  }
  uint32_t camif_window_w_t = camif_cfg->sensor_out_info.request_crop.last_pixel -
                     camif_cfg->sensor_out_info.request_crop.first_pixel + 1;

  /* rgnHOffset needs to be at least 8 and at most camif_window_width - 2 */
  if (pcmd->rgnHOffset < 8 ||
      pcmd->rgnHOffset >= camif_window_w_t - 2) {
    CDBG_ERROR("%s: Unsupported BF stats region config: invalid offset: %d\n", __func__, pcmd->rgnHOffset);
    return -1;
  }
  entry->hw_update_pending = 1;
  return rc;
}

/** bf_stats_enable:
 *    @entry: Pointer to entry struct
 *    @in_params: input params with enable value
 *
 *  Enable the bf stats module
 *
 *
 * Return 0 on Success, negative on ERROR
 **/
static int bf_stats_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  entry->enable = in_params->enable;
  entry->is_first = 1;
  return 0;
}

/** bf_stats_trigger_enable:
 *    @entry: Pointer to entry struct of bf stats
 *    @in_params: Input params
 *
 *  Set the trigger enable
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int bf_stats_trigger_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  entry->trigger_enable = in_params->enable;
  return 0;
}

/** bf_stats_set_params:
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
static int bf_stats_set_params (void *ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;
  switch (param_id) {
  case ISP_STATS_SET_ENABLE:
      rc = bf_stats_enable(entry, (isp_mod_set_enable_t *)in_params);
    break;
    case ISP_STATS_SET_CONFIG:
      rc = bf_stats_config(entry,
           (isp_hw_pix_setting_params_t *)in_params, in_param_size);
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
static int bf_stats_get_params (void *ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size,
                     void *out_params, uint32_t out_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;

  ISP_DBG(ISP_MOD_STATS, "%s: param_id = %d\n", __func__, param_id);
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

/** bf_stats_do_hw_update:
 *    @entry: Pointer to entry struct for bf stats
 *
 *  Method called at SOF, writes the value in reg_cmd to HW register
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int bf_stats_do_hw_update(isp_stats_entry_t *entry)
{
  int rc = 0;
  ISP_StatsBf_CfgCmdType *pcmd = entry->reg_cmd;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  ISP_DBG(ISP_MOD_STATS, "%s: E, hw_update = %d\n", __func__, entry->hw_update_pending);
  vfe_bf_stats_debug(pcmd);
  if (entry->hw_update_pending) {
    cfg_cmd.cfg_data = (void *)entry->reg_cmd;
    cfg_cmd.cmd_len = sizeof(ISP_StatsBf_CfgCmdType);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = BF_STATS_OFF;
    reg_cfg_cmd[0].u.rw_info.len = BF_STATS_LEN * sizeof(uint32_t);

    rc = ioctl(entry->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }

    entry->hw_update_pending = 0;
  }

  return rc;
}

/** bf_stats_dump_parsed_stats:
 *    @bf_stats: bf_stats struct that is to be printed
 *    @frame_id: Frame id of current stats
 *
 * Print value of current parsed bf stats
 *
 *
 * Return void
 **/
static void bf_stats_dump_parsed_stats(q3a_bf_stats_t *bf_stats, uint32_t frame_id)
{
  uint32_t i, j, idx;
  uint32_t bf_r_sum_zero_cnt = 0;
  uint32_t bf_b_sum_zero_cnt = 0;
  uint32_t bf_gr_sum_zero_cnt = 0;
  uint32_t bf_gb_sum_zero_cnt = 0;
  uint32_t bf_r_sharp_zero_cnt = 0;
  uint32_t bf_b_sharp_zero_cnt = 0;
  uint32_t bf_gr_sharp_zero_cnt = 0;
  uint32_t bf_gb_sharp_zero_cnt = 0;
  uint32_t bf_r_num_zero_cnt = 0;
  uint32_t bf_b_num_zero_cnt = 0;
  uint32_t bf_gr_num_zero_cnt = 0;
  uint32_t bf_gb_num_zero_cnt = 0;
  uint32_t bf_r_max_fv_zero_cnt = 0;
  uint32_t bf_b_max_fv_zero_cnt = 0;
  uint32_t bf_gr_max_fv_zero_cnt = 0;
  uint32_t bf_gb_max_fv_zero_cnt = 0;

  CDBG_DUMP("%s: E - buf_ptr = %p, frame_id = 0x%x, rgn_h_num = %d, rgn_v_num = %d, use_max_fv = %d\n",
    __func__, bf_stats, frame_id, bf_stats->bf_region_h_num,
    bf_stats->bf_region_v_num, bf_stats->use_max_fv);

  for (i = 0; i < bf_stats->bf_region_v_num; i++) {
    for (j = 0; j < bf_stats->bf_region_h_num; j++) {
      idx = i *  bf_stats->bf_region_h_num + j;
      if (idx >= MAX_BF_STATS_NUM)
        goto end;
        /*CDBG_DUMP("%s: gr_sharp = 0x%x\n", __func__, bf_stats->bf_gr_sharp[idx]);*/
      if (0 ==  bf_stats->bf_r_sum[idx])
        bf_r_sum_zero_cnt++;
      if (0 ==  bf_stats->bf_gr_sum[idx])
        bf_gr_sum_zero_cnt++;
      if (0 ==  bf_stats->bf_gb_sum[idx])
        bf_gb_sum_zero_cnt++;
      if (0 ==  bf_stats->bf_r_sharp[idx])
        bf_r_sharp_zero_cnt++;
      if (0 ==  bf_stats->bf_b_sharp[idx])
        bf_b_sharp_zero_cnt++;
      if (0 ==  bf_stats->bf_gr_sharp[idx])
        bf_gr_sharp_zero_cnt++;
      if (0 ==  bf_stats->bf_gb_sharp[idx])
        bf_gb_sharp_zero_cnt++;
      if (0 ==  bf_stats->bf_b_num[idx])
        bf_b_num_zero_cnt++;
      if (0 ==  bf_stats->bf_gr_num[idx])
        bf_gr_num_zero_cnt++;
      if (0 ==  bf_stats->bf_gb_num[idx])
        bf_gb_num_zero_cnt++;
      if (0 ==  bf_stats->bf_r_max_fv[idx])
        bf_r_max_fv_zero_cnt++;
      if (0 ==  bf_stats->bf_b_max_fv[idx])
        bf_b_max_fv_zero_cnt++;
      if (0 ==  bf_stats->bf_gr_max_fv[idx])
        bf_gr_max_fv_zero_cnt++;
      if (0 ==  bf_stats->bf_gb_max_fv[idx])
        bf_gb_max_fv_zero_cnt++;
    }
  }

end:
  CDBG_DUMP("%s: X frame_id = 0x%x, bf_stats_dump: rgn_h_num = %d, rgn_v_num = %d, use_max_fv = %d\n"
    "r_sum_zero_cnt     = %d, gr_sum_zero_cnt    = %d, gb_sum_zero_cnt   = %d, r_sharp_zero_cnt  = %d,\n"
    "b_sharp_zero_cnt   = %d, gr_sharp_zero_cntt = %d, gb_sharp_zero_cnt = %d, b_num_zero_cnt    = %d,\n"
    "gr_num_zero_cnt    = %d, gb_num_zero_cnt    = %d, r_max_fv_zero_cnt = %d, b_max_fv_zero_cnt = %d,\n"
    "gr_max_fv_zero_cnt = %d, gb_max_fv_zero_cnt = %d", __func__, frame_id,
    bf_stats->bf_region_h_num,
    bf_stats->bf_region_v_num,
    bf_stats->use_max_fv,
    bf_r_sum_zero_cnt,
    bf_gr_sum_zero_cnt,
    bf_gb_sum_zero_cnt,
    bf_r_sharp_zero_cnt,
    bf_b_sharp_zero_cnt,
    bf_gr_sharp_zero_cnt,
    bf_gb_sharp_zero_cnt,
    bf_b_num_zero_cnt,
    bf_gr_num_zero_cnt,
    bf_gb_num_zero_cnt,
    bf_r_max_fv_zero_cnt,
    bf_b_max_fv_zero_cnt,
    bf_gr_max_fv_zero_cnt,
    bf_gb_max_fv_zero_cnt);
  return;
}

/** bf_stats_parse:
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
static int bf_stats_parse(isp_stats_entry_t *entry,
                   void *raw_buf,
                   q3a_bf_stats_t *bf_stats,
                   uint32_t h_rgns_start,
                   uint32_t h_rgns_end,
                   uint32_t h_rgns_total,
                   uint32_t v_rgns_total)
{
  int window;
  uint32_t i, j;
  uint32_t *Sr,*Sb, *Sgr, *Sgb;
  uint32_t *r_sh, *b_sh, *gr_sh, *gb_sh;
  uint32_t *r_num, *b_num, *gr_num, *gb_num;
  uint32_t *r_fv_max, *b_fv_max, *gr_fv_max, *gb_fv_max;
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
  r_fv_max = bf_stats->bf_r_max_fv;
  b_fv_max = bf_stats->bf_b_max_fv;
  gr_fv_max = bf_stats->bf_gr_max_fv;
  gb_fv_max = bf_stats->bf_gb_max_fv;

  current_region = raw_buf;
  bf_stats->use_max_fv = 1;
  bf_stats->bf_region_h_num += (h_rgns_end - h_rgns_start + 1);
  bf_stats->bf_region_v_num = v_rgns_total;

  for (j = 0; j < v_rgns_total; j++) {
    for (i = 0; i < h_rgns_total; i++) {
      if (i >= h_rgns_start && i <= h_rgns_end) {
        /*parse AF stats */
        *Sr = ((*(current_region)) & 0x00FFFFFF);
        current_region++;
        *Sb = ((*(current_region)) & 0x00FFFFFF);
        current_region++;
        *Sgr = ((*(current_region)) & 0x00FFFFFF);
        current_region++;
        *Sgb = ((*(current_region)) & 0x00FFFFFF);
        current_region++;
        *r_sh = *current_region;
        current_region++;
        *b_sh = *current_region;
        current_region++;
        *gr_sh = *current_region;
        current_region++;
        *gb_sh = *current_region;
        current_region++;
        *r_num = ((*(current_region)) & 0x0000FFFF);
        *b_num = ((*(current_region)) & 0xFFFF0000) >> 16;
        current_region++;
        *gr_num = ((*(current_region)) & 0x0000FFFF);
        *gb_num = ((*(current_region)) & 0xFFFF0000) >> 16;
        current_region++;
        *r_fv_max = ((*current_region) & 0x00FFFFFF);
        current_region++;
        *b_fv_max = ((*current_region) & 0x00FFFFFF);
        current_region++;
        *gr_fv_max = ((*current_region) & 0x00FFFFFF);
        current_region++;
        *gb_fv_max = ((*current_region) & 0x00FFFFFF);
        current_region++;
      }
      Sr++;
      Sb++;
      Sgr++;
      Sgb++;
      r_sh++;
      b_sh++;
      gr_sh++;
      gb_sh++;
      r_num++;
      b_num++;
      gr_num++;
      gb_num++;
      r_fv_max++;
      b_fv_max++;
      gr_fv_max++;
      gb_fv_max++;
    }
  }
  return 0;
}

/** bf_stats_reset:
 *    @entry: Pointer to entry struct for bf stats
 *
 * Reset the entry struct
 *
 *
 * Return void
 **/
static void bf_reset(isp_stats_entry_t *entry)
{
  ISP_StatsBf_CfgCmdType *pcmd = entry->reg_cmd;

  isp_stats_reset(entry);
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
}

/** bf_stats_action:
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
static int bf_stats_action (void *ctrl, uint32_t action_code,
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
    bf_reset(entry);
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
    ISP_StatsBf_CfgCmdType *pcmd = entry->reg_cmd;

    int buf_idx = action_data->raw_stats_event->
                  u.stats.stats_buf_idxs[MSM_ISP_STATS_BF];
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

    q3a_bf_stats_t *bf_stats = entry->parsed_stats_buf;
    isp_stats_event->stats_mask |= (1 << MSM_ISP_STATS_BF);
    memset(bf_stats, 0, sizeof(q3a_bf_stats_t));

    if (!entry->is_ispif_split)
      rc = bf_stats_parse(entry, raw_buf, bf_stats,
        0,
        pcmd->rgnHNum,
        pcmd->rgnHNum + 1,
        pcmd->rgnVNum + 1);
    else { /* ISP is outputting two split halves */
      /* Parse for left stripe */
      if (entry->num_left_rgns)
        rc = bf_stats_parse(entry, raw_buf, bf_stats,
          0,
          entry->num_left_rgns - 1,
          entry->num_left_rgns + entry->num_right_rgns,
          pcmd->rgnVNum + 1);
      /* Parse for right stripe */
      if (entry->num_right_rgns)
        rc = bf_stats_parse(entry, (uint8_t*)raw_buf + entry->buf_len / 2, bf_stats,
          entry->num_left_rgns,
          entry->num_left_rgns + entry->num_right_rgns - 1,
          entry->num_left_rgns + entry->num_right_rgns,
          pcmd->rgnVNum + 1);
    }
    if (entry->num_bufs !=0) {
      int ret;
      ret = isp_stats_enqueue_buf(entry, buf_idx);
      if (ret < 0) {
        CDBG_ERROR("%s: error enqueue_buf, type = %d, buf_idx = %d, rc = %d\n",
          __func__, entry->stats_type, buf_idx, ret);
      }
    }
    if (rc == 0) {
      stats_data[MSM_ISP_STATS_BF].stats_buf = bf_stats;
      stats_data[MSM_ISP_STATS_BF].stats_type = MSM_ISP_STATS_BF;
      stats_data[MSM_ISP_STATS_BF].buf_size = sizeof(q3a_bf_stats_t);
      stats_data[MSM_ISP_STATS_BF].used_size = sizeof(q3a_bf_stats_t);
#ifdef BF_DUMP
	  bf_stats_dump_parsed_stats(bf_stats, isp_stats_event->frame_id);
#endif

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
 *    @ctrl: Pointer to entry (ctrl) struct
 *    @in_params: input params
 *    @notify_ops: notify_ops function pointer
 *
 *  Set initial values in the bf stats entry
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int bf_stats_init(void *ctrl, void *in_params, isp_notify_ops_t *notify_ops)
{
  isp_stats_entry_t *entry = ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;
  ISP_StatsBf_CfgCmdType *pcmd = entry->reg_cmd;

  entry->fd = init_params->fd;
  entry->stats_type = MSM_ISP_STATS_BF;
  /* max size for dual vfe, pass the 2nd buf offset by the size*/
  entry->buf_len = ISP_STATS_BF_BUF_SIZE * 2;

  entry->notify_ops = notify_ops;
  entry->dev_idx = init_params->dev_idx;
  entry->buf_mgr = init_params->buf_mgr;

  bf_reset(entry);
  return 0;
}

/** bf_stats_destroy:
 *    @ctrl: Pointer to the entry struct for bf stats
 *
 *  Free the memory for the bf stats module
 *
 *
 * Return 0 on Success, negative on ERROR
 **/
static int bf_stats_destroy (void *ctrl)
{
  isp_stats_entry_t *entry = ctrl;
  free(entry->reg_cmd);
  if (entry->parsed_stats_buf)
    free(entry->parsed_stats_buf);
  free(entry);
  return 0;
}

/** bf_stats44_open:
 *    @stats: Pointer to the ISP stats module
 *    @stats_type: enum value that gives what stats this is
 *
 *  Allocate and Initialize memory for bf stats module
 *
 *
 *  Return pointer to isp ops. NULL if ERROR
 **/
isp_ops_t *bf_stats44_open(isp_stats_mod_t *stats,
                                     enum msm_isp_stats_type stats_type)
{
  int rc = 0;
  isp_stats_entry_t *entry = NULL;
  ISP_StatsBf_CfgCmdType *cmd = NULL;
  ISP_DBG(ISP_MOD_STATS, "%s: E\n", __func__);

  entry = malloc(sizeof(isp_stats_entry_t));
  if (!entry) {
    CDBG_ERROR("%s: no mem for aec\n",  __func__);
    return NULL;
  }
  cmd = malloc(sizeof(ISP_StatsBf_CfgCmdType));
  if (!cmd) {
    CDBG_ERROR("%s: no mem\n",  __func__);
    free(entry);
    return NULL;
  }

  memset(entry, 0, sizeof(isp_stats_entry_t));
  memset(cmd, 0, sizeof(*cmd));
  entry->len_parsed_stats_buf = sizeof(q3a_bf_stats_t);
  entry->parsed_stats_buf = malloc(entry->len_parsed_stats_buf);
  if (entry->parsed_stats_buf== NULL) {
    CDBG_ERROR("%s: no mem\n",  __func__);
    free(cmd);
    free(entry);
    return NULL;
  }

  entry->reg_cmd = cmd;
  entry->ops.ctrl = (void *)entry;
  entry->ops.init = bf_stats_init;
  /* destroy the module object */
  entry->ops.destroy = bf_stats_destroy;
  /* set parameter */
  entry->ops.set_params = bf_stats_set_params;
  /* get parameter */
  entry->ops.get_params = bf_stats_get_params;
  entry->ops.action = bf_stats_action;
  return &entry->ops;
}


