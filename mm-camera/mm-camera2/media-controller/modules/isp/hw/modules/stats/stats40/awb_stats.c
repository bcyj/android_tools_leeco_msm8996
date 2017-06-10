/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include <stddef.h>
#include "awb_stats.h"
#include "isp_log.h"

#define AWB_SHIFT_BITS(n) ({ \
  uint32_t s_bits; \
  s_bits = CEIL_LOG2(n); \
  s_bits = (s_bits > 8) ? (s_bits-8) : 0; \
  s_bits;})

#ifdef AWB_STATS_DEBUG
#undef ISP_DBG
#define ISP_DBG ALOGE
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE
#endif

/* Stats AWB Config Command */
/* 1296 * 972 camif size.  16x16 regions. */
const struct ISP_StatsAwb_CfgCmdType ISP_DefaultStatsAwb_ConfigCmd = {
  /* reserved */
  0,   /* rgnHOffset */
  0,   /* rgnVOffset */
  1,   /* shiftBits */
  79,  /* rgnWidth  */
  59,  /* rgnHeight */
  /* reserved */
  15,  /*  rgnHNum */
  15,  /*  rgnVNum */
  241, /*  yMax    */
  10,  /*  yMin    */
  /* reserved */
  114,  /* c1  */
  /* reserved */
  136, /* c2  */
  /* reserved */
  -34, /* c3  */
  /* reserved */
  257, /* c4  */
  /* reserved */
  2, /* m1 */
  -16, /* m2 */
  16, /* m3 */
  -16, /* m4 */
  61,   /* t1 */
  32,   /* t2 */
  33,   /* t3 */
  64,   /* t6 */
  130,  /* t4 */
  /* reserved */
  157,  /* mg */
  /* reserved */
  64,  /* t5  */
};

/** isp_awb_stats_get_shiftbits:
 *    @entry: Pointer to entry struct for awb
 *    @p_shiftbits: pointer to Integer for output shiftbit
 *
 *   Get the shiftbits from reg_cmd in entry
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int isp_awb_stats_get_shiftbits(isp_stats_entry_t *entry,
  uint32_t *p_shiftbits)
{
  ISP_StatsAwb_CfgCmdType *cmd = entry->reg_cmd;
  *p_shiftbits = cmd->shiftBits;
  return 0;
}

/** awb_stats_enable:
 *    @entry: Pointer to entry struct
 *    @in_params: input params with enable value
 *
 *  Enable the awb stats module
 *
 *
 * Return 0 on Success, negative on ERROR
 **/
static int awb_stats_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  ISP_DBG(ISP_MOD_STATS, "%s: enable = %d\n", __func__, in_params->enable);
  entry->enable = in_params->enable;
  return 0;
}

/** awb_stats_trigger_enable:
 *    @entry: Pointer to entry struct of awb stats
 *    @in_params: Input params
 *
 *  Set the trigger enable
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int awb_stats_triger_enable(isp_stats_entry_t *entry,
  isp_mod_set_enable_t *in_params)
{
  ISP_DBG(ISP_MOD_STATS, "%s: trigger_enable = %d\n", __func__, in_params->enable);
  entry->trigger_enable = in_params->enable;
  return 0;
}

/** awb_stats_debug:
 *    @pcmd: Pointer to the reg_cmd struct that needs to be dumped
 *
 *  Print the value of the parameters in reg_cmd
 *
 *
 * Return void
 **/
static void awb_stats_debug(ISP_StatsAwb_CfgCmdType *pcmd)
{
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig shiftBits %d\n", pcmd->shiftBits);
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig rgnWidth  %d\n", pcmd->rgnWidth);
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig rgnHeight %d\n", pcmd->rgnHeight);
#ifndef VFE_31
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig rgnHOffset  %d\n", pcmd->rgnHOffset);
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig rgnVOffset %d\n", pcmd->rgnVOffset);
#endif
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig rgnHNum   %d\n", pcmd->rgnHNum);
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig rgnVNum   %d\n", pcmd->rgnVNum);
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig yMax      %d\n", pcmd->yMax);
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig yMin      %d\n", pcmd->yMin);

  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig t1 %d\n", pcmd->t1);
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig t2 %d\n", pcmd->t2);
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig t3 %d\n", pcmd->t3);
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig t4 %d\n", pcmd->t4);
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig mg %d\n", pcmd->mg);
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig t5 %d\n", pcmd->t5);
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig t6 %d\n", pcmd->t6);

  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig m1 %d\n", pcmd->m1);
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig m2 %d\n", pcmd->m2);
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig m3 %d\n", pcmd->m3);
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig m4 %d\n", pcmd->m4);

  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig c1 %d\n", pcmd->c1);
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig c2 %d\n", pcmd->c2);
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig c3 %d\n", pcmd->c3);
  ISP_DBG(ISP_MOD_STATS, "AWB statsconfig c4 %d\n", pcmd->c4);
}

/** awb_stats_config:
 *    @entry: Pointer to entry struct of awb stats
 *    @pix_settings: Pointer to the pipeline settings
 *    @in_param_size: Size of pix_settings
 *
 *  Configure the entry and reg_cmd for awb_stats using values passed in pix
 *  settings
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int awb_stats_config(isp_stats_entry_t *entry,
  isp_hw_pix_setting_params_t *pix_settings,
  uint32_t in_param_size)
{
  ISP_StatsAwb_CfgCmdType *pcmd = entry->reg_cmd;
  uint32_t pix_per_region;
  int rc = 0;
  uint32_t width, height;

  if (!entry->enable) {
    ISP_DBG(ISP_MOD_STATS, "%s: AWB stats not enabled", __func__);
    return 0;
  }

  /* AWB stats not support dual vfe yet*/
  entry->buf_offset = 0;

  entry->session_id = pix_settings->outputs->stream_param.session_id;
  entry->ion_fd = pix_settings->ion_fd;
  entry->hfr_mode = pix_settings->camif_cfg.hfr_mode;
  entry->comp_flag = 1;
  width= pix_settings->demosaic_output.last_pixel -
      pix_settings->demosaic_output.first_pixel + 1;
  pcmd->rgnWidth = ((width/(pcmd->rgnHNum + 1)) - 1);

  height = pix_settings->demosaic_output.last_line -
      pix_settings->demosaic_output.first_line + 1;
  pcmd->rgnHeight = ((height/(pcmd->rgnVNum + 1)) - 1);
  pix_per_region = (pcmd->rgnWidth+1) * (pcmd->rgnHeight+1);
  ISP_DBG(ISP_MOD_STATS, "%s: pix_per_region %d %dx%d", __func__, pix_per_region,
    pcmd->rgnWidth, pcmd->rgnHeight);
  pcmd->shiftBits = 0;
#if 0 /* TODO */
  pcmd->yMax = params->awb_params.bounding_box.y_max;
  pcmd->yMin = params->awb_params.bounding_box.y_min;
  pcmd->t1 = params->awb_params.exterme_col_param.t1;
  pcmd->t2 = params->awb_params.exterme_col_param.t2;
  pcmd->t3 = params->awb_params.exterme_col_param.t3;
  pcmd->t4 = params->awb_params.exterme_col_param.t4;
  pcmd->t5 = params->awb_params.exterme_col_param.t5;
  pcmd->t6 = params->awb_params.exterme_col_param.t6;
  pcmd->mg = params->awb_params.exterme_col_param.mg;

  pcmd->c1 = params->awb_params.bounding_box.c1;
  pcmd->c2 = params->awb_params.bounding_box.c2;
  pcmd->c3 = params->awb_params.bounding_box.c3;
  pcmd->c4 = params->awb_params.bounding_box.c4;
  pcmd->m1 = params->awb_params.bounding_box.m1;
  pcmd->m2 = params->awb_params.bounding_box.m2;
  pcmd->m3 = params->awb_params.bounding_box.m3;
  pcmd->m4 = params->awb_params.bounding_box.m4;
#endif
  awb_stats_debug(pcmd);
    entry->hw_update_pending = 1;
  return 0;
}

/** awb_stats_trigger_update:
 *    @entry: Pointer to entry struct
 *    @pix_settings: Pointer to Pipeline Settings
 *    @in_param_size: Size of input pipeline settings
 *
 *  Perform trigger update and calculate new value for parameters in reg_cmd
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int awb_stats_triger_update(isp_stats_entry_t *entry,
  isp_hw_pix_setting_params_t *pix_settings,
  uint32_t in_param_size)
{
  ISP_StatsAwb_CfgCmdType *pcmd = entry->reg_cmd;
  if (entry->trigger_enable == 0)
    return 0;
  // TODO if (pcmd->yMin != params->awb_params.bounding_box.y_min)

  return awb_stats_config(entry, pix_settings, in_param_size);
}

/** awb_stats_set_params:
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
static int awb_stats_set_params (void *ctrl, uint32_t param_id,
                     void *in_params, uint32_t in_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;

  switch (param_id) {
    case ISP_STATS_SET_ENABLE:
      rc = awb_stats_enable(entry, (isp_mod_set_enable_t *)in_params);
    break;
    case ISP_STATS_SET_CONFIG:
      rc = awb_stats_config(entry,
           (isp_hw_pix_setting_params_t *)in_params, in_param_size);
    break;
    case ISP_STATS_SET_TRIGGER_ENABLE:
      rc = awb_stats_triger_enable(entry, (isp_mod_set_enable_t *)in_params);
      break;
    case ISP_STATS_SET_TRIGGER_UPDATE:
      rc = awb_stats_triger_update(entry,
           (isp_hw_pix_setting_params_t *)in_params, in_param_size);
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

/** awb_stats_get_params:
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
static int awb_stats_get_params (void *ctrl, uint32_t param_id,
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
  case ISP_STATS_GET_STREAM_HANDLE: {
    uint32_t *handle = (uint32_t *) (out_params);
    *handle = entry->stream_handle;
  }
    break;
  default:
    break;
  }

  return rc;
}

/** awb_stats_do_hw_update:
 *    @entry: Pointer to entry struct for awb stats
 *
 *  Method called at SOF, writes the value in reg_cmd to HW register
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int awb_stats_do_hw_update(isp_stats_entry_t *entry)
{
  int rc = 0;
  struct msm_vfe_cfg_cmd2 cfg_cmd;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd[1];

  if (entry->hw_update_pending) {
    cfg_cmd.cfg_data = (void *)entry->reg_cmd;
    cfg_cmd.cmd_len = sizeof(ISP_StatsAwb_CfgCmdType);
    cfg_cmd.cfg_cmd = (void *) reg_cfg_cmd;
    cfg_cmd.num_cfg = 1;

    reg_cfg_cmd[0].u.rw_info.cmd_data_offset = 0;
    reg_cfg_cmd[0].cmd_type = VFE_WRITE;
    reg_cfg_cmd[0].u.rw_info.reg_offset = AWB_STATS_OFF;
    reg_cfg_cmd[0].u.rw_info.len = AWB_STATS_LEN * sizeof(uint32_t);

    rc = ioctl(entry->fd, VIDIOC_MSM_VFE_REG_CFG, &cfg_cmd);
    if (rc < 0){
      CDBG_ERROR("%s: HW update error, rc = %d", __func__, rc);
      return rc;
    }
    entry->hw_update_pending = 0;
  }

  return rc;
}

/** awb_stats_parse:
 *    @entry: Pointer to entry struct for awb stats
 *    @raw_buf: Pointer to raw buf containing the stats recevied from HW
 *    @awb_stats: Pointer to output awb_stats struct that needs to be filled
 *
 *  Parse the different stats parameters received from raw_buf to their
 *  corresponding struct
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int awb_stats_parse(isp_stats_entry_t *entry,
                   void *raw_buf,
                   q3a_awb_stats_t *awb_stats)
{
  int32_t  numRegions, i;
  uint32_t *current_region;
  uint32_t high_shift_bits;
  uint8_t inputNumReg;
  unsigned long *SCb, *SCr, *SY1, *NSCb;
  int rc = 0;

  ISP_DBG(ISP_MOD_STATS, "%s: E\n", __func__);
  SCb = awb_stats->SCb;
  SCr = awb_stats->SCr;
  SY1 = awb_stats->SY1;
  NSCb = awb_stats->NSCb;
  awb_stats->wb_region_h_num = 16;
  awb_stats->wb_region_v_num = 16;
  numRegions = awb_stats->wb_region_h_num * awb_stats->wb_region_v_num;

  /* Translate packed 4 32 bit word per region struct comming from the VFE
   * into more usable struct for microprocessor algorithms,
   * vfeStatDspOutput - up to 4k output of DSP from VFE block for AEC and
     AWB control */

  /* copy pointer to VFE stat 2 output region, plus 1 for header */
  current_region = raw_buf;

  for (i = 0; i < numRegions; i++) {
    /* Either 64 or 256 regions processed here */
    /* 16 bits sum of Y. */
    *SY1 = ((*(current_region)) & 0x01FFFFFF);
    SY1++;
    current_region ++;  /* each step is a 32 bit words or 32 bytes long */
                        /* which is 2 of 16 bit words */
    *SCb = ((*(current_region)) & 0x01FFFFFF);
    SCb++;
    current_region ++;
    *SCr = ((*(current_region)) & 0x01FFFFFF);
    SCr++;
    current_region ++;
    /* NSCb counter should not left shifted by bitshift */
    *NSCb = ((*(current_region)) & 0x0001FFFF);
    NSCb++;
    current_region ++;
  }
  awb_stats->awb_extra_stats.GLB_Y = *(current_region);
  current_region ++;
  awb_stats->awb_extra_stats.GLB_Cb = *(current_region);
  current_region ++;
  awb_stats->awb_extra_stats.GLB_Cr = *(current_region);
  current_region ++;
  awb_stats->awb_extra_stats.GLB_N = ((*(current_region)) & 0x1FFFFFF);
  current_region ++;
  awb_stats->awb_extra_stats.Green_r = *(current_region);
  current_region ++;
  awb_stats->awb_extra_stats.Green_g = *(current_region);
  current_region ++;
  awb_stats->awb_extra_stats.Green_b = *(current_region);
  current_region ++;
  awb_stats->awb_extra_stats.Green_N = ((*current_region) & 0x1FFFFFF);
  current_region ++;
  awb_stats->awb_extra_stats.ExtBlue_r = *(current_region);
  current_region ++;
  awb_stats->awb_extra_stats.ExtBlue_g = *(current_region);
  current_region ++;
  awb_stats->awb_extra_stats.ExtBlue_b = *(current_region);
  current_region ++;
  awb_stats->awb_extra_stats.ExtBlue_N = ((*current_region) & 0x1FFFFFF);
  current_region ++;
  awb_stats->awb_extra_stats.ExtRed_r = *(current_region);
  current_region ++;
  awb_stats->awb_extra_stats.ExtRed_g = *(current_region);
  current_region ++;
  awb_stats->awb_extra_stats.ExtRed_b = *(current_region);
  current_region ++;
  awb_stats->awb_extra_stats.ExtRed_N = ((*current_region) & 0x1FFFFFF);
  return 0;
}

/** awb_stats_action:
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
static int awb_stats_action (void *ctrl, uint32_t action_code,
                 void *data, uint32_t data_size)
{
  int rc = 0;
  isp_stats_entry_t *entry = ctrl;

  switch ((isp_stats_action_code_t)action_code) {
  case ISP_STATS_ACTION_STREAM_START:
    break;
  case ISP_STATS_ACTION_STREAM_STOP:
    break;
  case ISP_STATS_ACTION_RESET:
    isp_stats_reset(entry);
    break;
  case ISP_STATS_ACTION_HW_CFG_UPDATE:
    rc = awb_stats_do_hw_update(entry);
    break;
  case ISP_STATS_ACTION_STATS_PARSE: {
    isp_pipeline_stats_parse_t *action_data = data;
    mct_event_stats_isp_t *isp_stats_event = action_data->parsed_stats_event;
    mct_event_stats_isp_data_t *stats_data = isp_stats_event->stats_data;
    int buf_idx = action_data->raw_stats_event->
                  u.stats.stats_buf_idxs[MSM_ISP_STATS_AWB];
    void *raw_buf = isp_get_buf_addr(entry->buf_mgr,
      entry->buf_handle, buf_idx);
    if(!raw_buf){
      CDBG_ERROR("%s: isp_get_buf_addr failed!\n", __func__);
      return -1;
    }
    q3a_awb_stats_t *awb_stats = entry->parsed_stats_buf;
    isp_stats_event->stats_mask |= (1 << MSM_ISP_STATS_AWB);
    rc = awb_stats_parse(entry, raw_buf, awb_stats);
    if (entry->num_bufs !=0) {
       rc = isp_stats_enqueue_buf(entry, buf_idx);
    }
    if (rc == 0) {
      stats_data[MSM_ISP_STATS_AWB].stats_buf = awb_stats;
      stats_data[MSM_ISP_STATS_AWB].stats_type = MSM_ISP_STATS_AWB;
      stats_data[MSM_ISP_STATS_AWB].buf_size = sizeof(q3a_awb_stats_t);
      stats_data[MSM_ISP_STATS_AWB].used_size = sizeof(q3a_awb_stats_t);
    } else {
      stats_data[MSM_ISP_STATS_AWB].stats_buf = NULL;
      stats_data[MSM_ISP_STATS_AWB].buf_size = 0;
      stats_data[MSM_ISP_STATS_AWB].used_size = 0;
    }
    break;
  }
  case ISP_STATS_ACTION_STREAM_BUF_CFG:
    rc = isp_stats_config_stats_stream(entry, ISP_STATS_AWB_BUF_NUM);
    break;
  case ISP_STATS_ACTION_STREAM_BUF_UNCFG:
    rc = isp_stats_unconfig_stats_stream(entry);
    break;
  default:
    break;
  }
  return rc;
}

/** awb_stats_init:
 *    @ctrl: Pointer to entry (ctrl) struct
 *    @in_params: input params
 *    @notify_ops: notify_ops function pointer
 *
 *  Set initial values in the awb stats entry
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int awb_stats_init (void *ctrl, void *in_params, isp_notify_ops_t *notify_ops)
{
  isp_stats_entry_t *entry = ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;
  ISP_StatsAwb_CfgCmdType *cmd = entry->reg_cmd;

  /* max size for dual vfe, pass the 2nd buf offset by the size*/
  entry->buf_len = ISP_STATS_AWB_BUF_SIZE *2;

  entry->stats_type = MSM_ISP_STATS_AWB;
  entry->fd = init_params->fd;
  entry->notify_ops = notify_ops;
  entry->dev_idx = init_params->dev_idx;
  entry->buf_mgr = init_params->buf_mgr;
  *cmd = ISP_DefaultStatsAwb_ConfigCmd;
  isp_stats_reset(entry);
  return 0;
}

/** awb_stats_destroy:
 *    @ctrl: Pointer to the entry struct for awb stats
 *
 *  Free the memory for the awb stats module
 *
 *
 * Return 0 on Success, negative on ERROR
 **/
static int awb_stats_destroy (void *ctrl)
{
  isp_stats_entry_t *entry = ctrl;
  free(entry->reg_cmd);
  if (entry->parsed_stats_buf)
    free(entry->parsed_stats_buf);
  if (entry->private)
    free(entry->private);
  free(entry);
  return 0;
}

/** awb_stats_open:
 *    @stats: Pointer to the ISP stats module
 *    @stats_type: enum value that gives what stats this is
 *
 *  Allocate and Initialize memory for awb stats module
 *
 *
 *  Return pointer to isp ops. NULL if ERROR
 **/
isp_ops_t *awb_stats_open(isp_stats_mod_t *stats,
  enum msm_isp_stats_type stats_type)
{
  int rc = 0;
  isp_stats_entry_t *entry = NULL;
  ISP_StatsAwb_CfgCmdType *cmd = NULL;
  uint32_t *acked_ymin = NULL;

  ISP_DBG(ISP_MOD_STATS, "%s: E\n", __func__);

  entry = malloc(sizeof(*entry));
  if (!entry) {
    CDBG_ERROR("%s: no mem for aec\n", __func__);
    return NULL;
  }
  memset(entry, 0, sizeof(*entry));

  cmd = malloc(sizeof(*cmd));
  if (!cmd) {
    CDBG_ERROR("%s: no mem\n", __func__);
    free(entry);
    return NULL;
  }
  memset(cmd, 0, sizeof(*cmd));

  acked_ymin = malloc(sizeof(uint32_t));
  if (!acked_ymin) {
    CDBG_ERROR("%s: no mem\n", __func__);
    free(cmd);
    free(entry);
    return NULL;
  }
  *acked_ymin = 0;

  memset(entry, 0, sizeof(isp_stats_entry_t));
  memset(cmd, 0, sizeof(*cmd));
  entry->len_parsed_stats_buf = sizeof(q3a_awb_stats_t);
  entry->parsed_stats_buf = malloc(entry->len_parsed_stats_buf);
  if (entry->parsed_stats_buf== NULL) {
    CDBG_ERROR("%s: no mem\n",  __func__);
    free(acked_ymin);
    free(cmd);
    free(entry);
    return NULL;
  }

  ISP_DBG(ISP_MOD_STATS, "%s: open AWB stats.\n", __func__);
  entry->private = (void *)acked_ymin;
  entry->reg_cmd = cmd;
  entry->ops.ctrl = (void *)entry;
  entry->ops.init = awb_stats_init;
  /* destroy the module object */
  entry->ops.destroy = awb_stats_destroy;
  /* set parameter */
  entry->ops.set_params = awb_stats_set_params;
  /* get parameter */
  entry->ops.get_params = awb_stats_get_params;
  entry->ops.action = awb_stats_action;
  return &entry->ops;
}


