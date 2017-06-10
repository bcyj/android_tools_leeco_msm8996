/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include <math.h>
#include "../isp_stats.h"
#include "aec_stats.h"
#include "af_stats.h"
#include "awb_stats.h"
#include "be_stats.h"
#include "bg_stats.h"
#include "bf_stats.h"
#include "cs_stats.h"
#include "rs_stats.h"
#include "bhist_stats.h"
#include "ihist_stats.h"
#include "isp_log.h"

#ifdef STATS40_INTF_DEBUG
#undef ISP_DBG
#define ISP_DBG ALOGE
#endif

#undef CDBG_ERROR
#define CDBG_ERROR ALOGE

/** stats_destroy:
 *    @mod_ctrl: Pointer to stats40 module
 *
 *  Destroy all stats 40 sub modules
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int stats_destroy (void *mod_ctrl)
{
  isp_stats_mod_t *stats = mod_ctrl;
  uint32_t i = 0;
  int rc = 0;

  /* loop sub modules to destroy al stats modules */
  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
     if (((1 << i) & stats->stats_max_mask) && stats->stats_ops[i]) {
       rc = stats->stats_ops[i]->destroy(stats->stats_ops[i]->ctrl);
       if (rc < 0) {
         CDBG_ERROR("%s: stats %d enable failed\n", __func__, i);
         return rc;
       }
    }
  }
  free(stats);
  return rc;
}
/** stats_enable_substats:
 *    @mod_ctrl: Pointer to stats40 module
 *    @enb: enable flag
 *    @in_params_size:
 *
 *  Enable all sub stats modules
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int stats_enable_substats (void *mod_ctrl, void *enb,
  uint32_t in_params_size)
{
  isp_stats_mod_t *stats = mod_ctrl;
  isp_mod_set_enable_t *enable = (isp_mod_set_enable_t *) enb;
  uint32_t fast_aec_mask;
  int rc = 0;
  uint32_t i = 0;

  if (enable->fast_aec_mode)
    fast_aec_mask = (1<<MSM_ISP_STATS_BG) | (1<<MSM_ISP_STATS_BHIST);
  else
    fast_aec_mask = 0xFFFFFFFF;

  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
     if (((1 << i) & stats->stats_max_mask & fast_aec_mask) && stats->stats_ops[i]) {
       rc = stats->stats_ops[i]->set_params(stats->stats_ops[i]->ctrl, ISP_STATS_SET_ENABLE,
         enable, in_params_size);
       if (rc < 0) {
         CDBG_ERROR("%s: stats %d enable failed\n", __func__, i);
         return rc;
       }
    }
  }
  return rc;
}

/** stats_config_update_substats:
 *    @mod_ctrl: pointer to stats40 module
 *    @in_params: pipeline settings params
 *    @in_params_size:
 *
 *  New stats config received. Update the sub stats modules
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int stats_config_update_substats(void *mod_ctrl, void *in_params,
  uint32_t in_params_size)
{
  isp_stats_mod_t *stats = mod_ctrl;
  int rc = 0;
  int i;
  isp_hw_pix_setting_params_t *pix_settings =
    (isp_hw_pix_setting_params_t *)in_params;
  uint32_t stats_mask;

  if (sizeof(isp_hw_pix_setting_params_t) != in_params_size) {
    CDBG_ERROR("%s: size mismatch! Stats Config Update Failed\n", __func__);
    return -1;
  }

  stats_mask = pix_settings->stats_cfg.stats_mask;

  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (((1 << i) & stats_mask) &&
    ((1 << i) & stats->stats_max_mask) && stats->stats_ops[i]) {
      rc = stats->stats_ops[i]->set_params(
        stats->stats_ops[i]->ctrl, ISP_STATS_SET_CONFIG,
        in_params, in_params_size);
      if (rc < 0) {
        CDBG_ERROR("%s: stats BF config failed\n", __func__);
        return rc;
      }

      rc = stats->stats_ops[i]->action(
        stats->stats_ops[i]->ctrl,
        ISP_STATS_ACTION_HW_CFG_UPDATE, NULL, 0);
      if (rc < 0) {
        CDBG_ERROR("%s: stats BF hw update failed, rc = %d\n", __func__, rc);
        return rc;
      }
    }
  }

  return rc;
}

/** stats_config_substats:
 *    @mod_ctrl: pointer to stats40 module
 *    @in_params: pipeline settings params
 *    @in_params_size:
 *
 *  Config the sub stats modules
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int stats_config_substats(void *mod_ctrl, void *in_params,
  uint32_t in_params_size)
{
  isp_stats_mod_t *stats = mod_ctrl;
  int rc = 0;
  uint32_t i = 0;

  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
     if (((1 << i) & stats->stats_max_mask) && stats->stats_ops[i]) {
       rc = stats->stats_ops[i]->set_params(stats->stats_ops[i]->ctrl, ISP_STATS_SET_CONFIG,
         in_params, in_params_size);
       if (rc < 0) {
         CDBG_ERROR("%s: stats %d config failed\n", __func__, i);
         return rc;
       }
     }
  }

  return rc;
}

/** stats_set_params:
 *    @mod_ctrl: Pointer to stats40 module
 *    @param_id: event id indicating what value is set
 *    @in_params: input event params
 *    @in_param_size: size of input params
 *
 *  Set value for parameter given by param id on sub stats modules and pass
 *  input params
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int stats_set_params (void *mod_ctrl, uint32_t params_id,
  void *in_params, uint32_t in_params_size)
{
  isp_stats_mod_t *stats = mod_ctrl;
  int rc = 0;

  ISP_DBG(ISP_MOD_STATS, "%s: E, params_id = %d\n", __func__, params_id);
  switch (params_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE:
    rc = stats_enable_substats(stats, in_params, in_params_size);
    break;
  case ISP_HW_MOD_SET_MOD_CONFIG:
    rc = stats_config_substats(stats, in_params, in_params_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_ENABLE:
    break;
  case ISP_HW_MOD_SET_CONFIG_UPDATE:
    rc = stats_config_update_substats(stats, in_params, in_params_size);
    break;
  case ISP_HW_MOD_SET_ZOOM_RATIO:
    break;
  default:
    rc = -EAGAIN; /* nop */
    break;
  }
  ISP_DBG(ISP_MOD_STATS, "%s: perer X rc = %d\n", __func__, rc);
  return rc;
}

/** stats_get_cs_rs_config:
 *    @stats: pointer to stats module
 *    @cs_rs_config: output cs_rs config
 *
 *  Get CS & RS Config from module
 *
 *
 *  Return 0 on SUccess, negative on ERROR
 **/
static int stats_get_cs_rs_config(isp_stats_mod_t *stats,
  isp_cs_rs_config_t *cs_rs_config)
{
  int rc = 0;
  uint32_t val;
  enum msm_isp_stats_type stats_type = MSM_ISP_STATS_RS;

  if (stats->stats_ops[stats_type]) {
    rc = stats->stats_ops[stats_type]->get_params(
      stats->stats_ops[stats_type]->ctrl,
      ISP_STATS_GET_RS_CONFIG,
      NULL, 0, &val, sizeof(val));
    if (rc < 0) {
      CDBG_ERROR("%s: ISP_STATS_GET_RS_CONFIG failed\n", __func__);
      return rc;
    }
    cs_rs_config->raw_num = val;
  }

  stats_type = MSM_ISP_STATS_CS;
  if (stats->stats_ops[stats_type]) {
    rc = stats->stats_ops[stats_type]->get_params(
      stats->stats_ops[stats_type]->ctrl,
      ISP_STATS_GET_CS_CONFIG,
      NULL, 0, &val, sizeof(val));
    if (rc < 0) {
      CDBG_ERROR("%s: ISP_STATS_GET_RS_CONFIG failed\n", __func__);
      return rc;
    }
    cs_rs_config->col_num = val;
  }

  return rc;
}

/** stats_get_params:
 *    @mod_ctrl: Pointer to stats40 module
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
static int stats_get_params (void *mod_ctrl, uint32_t params_id,
  void *in_params, uint32_t in_params_size, void *out_params,
  uint32_t out_params_size)
{
  isp_stats_mod_t *stats = mod_ctrl;
  int rc = 0;
  switch ((isp_hw_mod_get_param_id_t)params_id) {
  case ISP_HW_MOD_GET_CS_RS_CONFIG:
    rc = stats_get_cs_rs_config(stats, (isp_cs_rs_config_t *)out_params);
    break;

  default:
    rc = 0;
    break;
  }

  return rc;
}

/** stats_action_hw_update:
 *    @stats: pointer to stat40 module
 *    @stats_mask: mask for different sub stats modules
 *
 *  Perform HW update on different sub stats modules
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int stats_action_hw_update(isp_stats_mod_t *stats, uint32_t stats_mask)
{
  int i, j;
  int rc = 0;
  uint32_t cmd = ISP_STATS_ACTION_HW_CFG_UPDATE;

  ISP_DBG(ISP_MOD_STATS, "%s: E,  stats_mask = 0x%x\n", __func__, stats_mask);
  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (stats_mask & (1 << i)) {
      ISP_DBG(ISP_MOD_STATS, "%s: match i = %d\n", __func__, i);
      rc = stats->stats_ops[i]->action(stats->stats_ops[i]->ctrl,
        cmd, NULL, 0);
      if (rc < 0) {
        CDBG_ERROR("%s: rc = %d, stats type = %d\n", __func__, rc, i);
        return rc;
      }
    }
  }

  return rc;
}

/** stats_action_buf_config:
 *    @stats: pointer to stats module
 *    @cfg: flag for cfg/uncfg
 *    @stats_mask:
 *
 *  Config the BUF on sub stats modules
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int stats_action_buf_config(isp_stats_mod_t *stats, uint8_t cfg,
  uint32_t stats_mask)
{
  int i, j;
  int rc = 0;
  uint32_t cmd = ISP_STATS_ACTION_STREAM_BUF_CFG;

  ALOGE("%s: cfg = %d, stats_mask = 0x%x\n", __func__, cfg, stats_mask);
  if (!cfg)
    cmd = ISP_STATS_ACTION_STREAM_BUF_UNCFG;
  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (stats_mask & (1 << i)) {
      rc = stats->stats_ops[i]->action(stats->stats_ops[i]->ctrl,
        cmd, NULL, 0);
      if (rc < 0) {
        CDBG_ERROR("%s: rc = %d, stats type = %d\n", __func__, rc, i);
        goto end;
      }
    }
  }
end:
  return rc;
}

/** stats_action:
 *    @mod_ctrl: Pointer to stats40 module
 *    @action_code:  indicates what action is required
 *    @action_data: input param
 *    @action_data_size: size of input param
 *
 *  Do the required actions for event given by action code
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int stats_action(void *mod_ctrl, uint32_t action_code, void *action_data,
  uint32_t action_data_size)
{
  int rc = 0;
  isp_stats_mod_t *stats = mod_ctrl;
  uint32_t stats_mask = stats->stats_max_mask;
  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE:
    stats_action_hw_update(stats, stats_mask);
    break;
  case ISP_HW_MOD_ACTION_BUF_CFG:
    stats_mask = *((uint32_t *)action_data);
    rc = stats_action_buf_config(stats, 1, stats_mask);
    break;
  case ISP_HW_MOD_ACTION_BUF_UNCFG:
    stats_mask = *((uint32_t *)action_data);
    rc = stats_action_buf_config(stats, 0, stats_mask);
    break;
  case ISP_HW_MOD_ACTION_STREAMON:
    stats_mask = *((uint32_t *)action_data);
    ALOGE("%s: stats mask = 0x%x\n", __func__, stats_mask);
    rc = isp_stats_start_streams(stats, stats_mask);
    break;
  case ISP_HW_MOD_ACTION_STREAMOFF:
    stats_mask = *((uint32_t *)action_data);
    rc = isp_stats_stop_streams(stats, stats_mask);
    break;
  case ISP_HW_MOD_ACTION_STATS_PARSE:
    rc = isp_stats_parse(stats, (isp_pipeline_stats_parse_t *)action_data);
    break;
  case ISP_HW_MOD_ACTION_RESET:
    rc = isp_stats_do_reset(stats);
    break;
  default:
    /* no op */
    rc = -EAGAIN;
    CDBG_HIGH("%s: action code = %d is not supported. nop",
              __func__, action_code);
    break;
  }
  return rc;
}

/** stats_open_sub_module:
 *    @stats: pointer to stats40 module
 *    @stats_type: mask indicating one of the sub stats
 *
 *  Open corresponding stats_type module
 *
 *
 *  Return 0 on Success, negative on ERROR
 **/
static int stats_open_sub_module(isp_stats_mod_t *stats,
  enum msm_isp_stats_type stats_type)
{
  switch (stats_type) {
  case MSM_ISP_STATS_AEC:
    stats->stats_ops[stats_type] = aec_stats44_open(stats, stats_type);
    if (stats->stats_ops[stats_type] == NULL) {
      CDBG_ERROR("%s: cannot open aec stats\n", __func__);
      return -1;
    }
    break;
  case MSM_ISP_STATS_AF:
    stats->stats_ops[stats_type] = af_stats44_open(stats, stats_type);
    if (stats->stats_ops[stats_type] == NULL) {
      CDBG_ERROR("%s: cannot open af stats\n", __func__);
      return -1;
    }
    break;
  case MSM_ISP_STATS_AWB:
    stats->stats_ops[stats_type] = awb_stats44_open(stats, stats_type);
    if (stats->stats_ops[stats_type] == NULL) {
     CDBG_ERROR("%s: cannot open awb stats\n", __func__);
     return -1;
    }
    break;
  case MSM_ISP_STATS_RS:
    stats->stats_ops[stats_type] = rs_stats44_open(stats, stats_type);
    if (stats->stats_ops[stats_type] == NULL) {
      CDBG_ERROR("%s: cannot open rs stats\n", __func__);
      return -1;
    }
    break;
  case MSM_ISP_STATS_CS:
    stats->stats_ops[stats_type] = cs_stats44_open(stats, stats_type);
    if (stats->stats_ops[stats_type] == NULL) {
      CDBG_ERROR("%s: cannot open cs stats\n", __func__);
      return -1;
    }
    break;
  case MSM_ISP_STATS_IHIST:
    stats->stats_ops[stats_type] = ihist_stats44_open(stats, stats_type);
    if (stats->stats_ops[stats_type] == NULL) {
      CDBG_ERROR("%s: cannot open ihist stats\n", __func__);
      return -1;
    }
    break;
  case MSM_ISP_STATS_SKIN:
                break;
  case MSM_ISP_STATS_BG:
    stats->stats_ops[stats_type] = bg_stats44_open(stats, stats_type);
    if (stats->stats_ops[stats_type] == NULL) {
      CDBG_ERROR("%s: cannot open bg stats\n", __func__);
      return -1;
    }
    break;
  case MSM_ISP_STATS_BF:
    stats->stats_ops[stats_type] = bf_stats44_open(stats, stats_type);
    if (stats->stats_ops[stats_type] == NULL) {
      CDBG_ERROR("%s: cannot open bf stats\n", __func__);
      return -1;
    }
    break;
  case MSM_ISP_STATS_BE:
    stats->stats_ops[stats_type] = be_stats44_open(stats, stats_type);
    if (stats->stats_ops[stats_type] == NULL) {
      CDBG_ERROR("%s: cannot open be stats\n", __func__);
      return -1;
    }
    break;
  case MSM_ISP_STATS_BHIST:
    stats->stats_ops[stats_type] = bhist_stats44_open(stats, stats_type);
    if (stats->stats_ops[stats_type] == NULL) {
      CDBG_ERROR("%s: cannot open bhist stats\n", __func__);
      return -1;
    }
    break;
  default:
    break;
  }

  return 0;
}

/** stats_init:
 *    @mod_ctrl: Pointer to stats40 module
 *    @in_params: input params
 *    @notify_ops: notify_ops function pointer
 *
 *  Initialize the stats40 module & sub stats modules
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int stats_init (void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  int rc = 0;
  int i;
  isp_stats_mod_t *stats = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  stats->fd = init_params->fd;
  stats->notify_ops = notify_ops;
  stats->stats_max_mask = init_params->max_stats_mask;
  stats->stats_cfg_mask = init_params->max_stats_mask; /* TODO: may only use one mask*/
  stats->stats_enable_mask = init_params->max_stats_mask;
  stats->buf_mgr = init_params->buf_mgr;
  stats->dev_idx = init_params->dev_idx;

  ISP_DBG(ISP_MOD_STATS, "%s: stats_max_mask = %x\n", __func__, stats->stats_max_mask);
  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (stats->stats_max_mask == 0){
      ISP_DBG(ISP_MOD_STATS, "%s: sub-stats mask = %x\n", __func__, stats->stats_max_mask);
      break;
    }

    if (stats->stats_max_mask & (1 << i)) {
      rc = stats_open_sub_module(stats, (enum msm_isp_stats_type)i);
      if (rc < 0) {
        CDBG_ERROR("%s: cannot open stats %d\n", __func__, i);
        return rc;
      }

      rc = stats->stats_ops[i]->init(stats->stats_ops[i]->ctrl, in_params, notify_ops);
      if (rc < 0) {
        CDBG_ERROR("%s: cannot init stats %d\n", __func__, i);
        return rc;
      }
    }
  }
  /* open and init each stats entries */
  return rc;
}

/** stats44_open:
 *    @version:
 *
 *  Allocate and set basic params for stats40 module
 *
 *
 *  Return pointer to isp ops. NULL if ERROR
 **/
isp_ops_t *stats44_open(uint32_t version)
{
  isp_stats_mod_t *stats = malloc(sizeof(isp_stats_mod_t));

  ISP_DBG(ISP_MOD_STATS, "%s: open STATS40 module\n", __func__);
  if (!stats) {
    /* no memory */
    CDBG_ERROR("%s: no mem",  __func__);
    return NULL;
  }
  memset(stats,  0,  sizeof(isp_stats_mod_t));
  stats->ops.ctrl = (void *)stats;
  stats->ops.init = stats_init;
  /* destroy the module object */
  stats->ops.destroy = stats_destroy;
  /* set parameter */
  stats->ops.set_params = stats_set_params;
  /* get parameter */
  stats->ops.get_params = stats_get_params;
  stats->ops.action = stats_action;
  return &stats->ops;
}

