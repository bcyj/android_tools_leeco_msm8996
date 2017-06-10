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


/** stats_destroy:
 *    @mod_ctrl: pointer to instance private data
 *
 * Destroy all open submodule and and free private resources.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int stats_destroy (void *mod_ctrl)
{
  isp_stats_mod_t *stats = mod_ctrl;
  int rc = 0;
  int i;

  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (stats->stats_max_mask == 0)
      break;

    if (stats->stats_max_mask & (1 << i)) {
      rc = stats->stats_ops[i]->destroy(stats->stats_ops[i]->ctrl);
      if (rc < 0) {
        CDBG_ERROR("%s: cannot destroy stats %d\n", __func__, i);
        return rc;
      }
    }
  }
  free(stats);
  return 0;
}

/** stats_enable_substats:
 *    @mod_ctrl: pointer to instance private data
 *    @enb: input data
 *    @in_params_size: size of input data
 *
 * Calls enable for each open submodule.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int stats_enable_substats (void *mod_ctrl, void *enb,
  uint32_t in_params_size)
{
  isp_stats_mod_t *stats = mod_ctrl;
  isp_mod_set_enable_t *enable = (isp_mod_set_enable_t *)enb;
  int rc = 0;
  uint32_t i = 0;

  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (((1 << i) & stats->stats_max_mask) && stats->stats_ops[i]) {
      rc = stats->stats_ops[i]->set_params(stats->stats_ops[i]->ctrl,
        ISP_STATS_SET_ENABLE, enable, in_params_size);
      if (rc < 0) {
        CDBG_ERROR("%s: stats %d enable failed\n", __func__, i);
        return rc;
      }
    }
  }
  return rc;
}

/** stats_config_update_substats:
 *    @mod_ctrl: pointer to instance private data
 *    @in_params: input data
 *    @in_params_size: size of input data
 *
 * Calls enable for each open submodule.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
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
    }
  }

  return rc;
}

/** stats_enable_trigger
 *  @mod_ctrl: pointer to the instance private data
 *  @in_params: input data
 *  @in_params_size: size of input data
 *
 *  Calls trigger enable for BG submodule
 *
 *  Return 0 on Success.
 *
 **/
static int stats_enable_trigger(void *mod_ctrl, void *in_params,
  uint32_t in_params_size)
{
  isp_stats_mod_t *stats = mod_ctrl;
  int rc = 0;

  rc =  stats->stats_ops[MSM_ISP_STATS_BG]->set_params(
    stats->stats_ops[MSM_ISP_STATS_BG]->ctrl,
    ISP_STATS_SET_TRIGGER_ENABLE, in_params, in_params_size);
  if (rc < 0) {
    CDBG_ERROR("%s: Stats  enable trigger update failed\n", __func__);
  }
  return rc;
}

/** stats_config_substats:
 *    @mod_ctrl: pointer to instance private data
 *    @in_params: input data
 *    @in_params_size: size of input data
 *
 * Calls config for each open submodule.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int stats_config_substats(void *mod_ctrl, void *in_params,
  uint32_t in_params_size)
{
  isp_stats_mod_t *stats = mod_ctrl;
  int rc = 0;
  uint32_t i = 0;

  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (((1 << i) & stats->stats_max_mask) && stats->stats_ops[i]) {
      rc = stats->stats_ops[i]->set_params(stats->stats_ops[i]->ctrl,
        ISP_STATS_SET_CONFIG, in_params, in_params_size);
      if (rc < 0) {
        CDBG_ERROR("%s: stats %d config failed\n", __func__, i);
        return rc;
      }
    }
  }
  return rc;
}

/** isp_stats_stats_fullsize_config
 *  @stats: pointer to instance private data
 *  @in_params: Pointer to the input params
 *  @size: Size of input data size
 *
 *  Return 0 on Success
 *
 **/
static int isp_set_stats_fullsize_config(isp_stats_mod_t *stats, void* in_params,
  uint32_t size)
{
  int rc = 0;
  rc = stats->stats_ops[MSM_ISP_STATS_BG]->set_params(stats->stats_ops[MSM_ISP_STATS_BG]->ctrl,
    ISP_HW_MOD_SET_STATS_FULLSIZE_CFG, in_params, size);
  if (rc < 0) {
    CDBG_ERROR("%s: Stats Trigger update failed\n", __func__);
  }
  return rc;
}

/** stats_set_params:
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
static int stats_set_params(void *mod_ctrl, uint32_t params_id, void *in_params,
  uint32_t in_params_size)
{
  isp_stats_mod_t *stats = mod_ctrl;
  int rc = 0;

  switch (params_id) {
  case ISP_HW_MOD_SET_MOD_ENABLE:
    rc = stats_enable_substats(stats, in_params, in_params_size);
    break;
  case ISP_HW_MOD_SET_MOD_CONFIG:
    rc = stats_config_substats(stats, in_params, in_params_size);
    break;
  case ISP_HW_MOD_SET_TRIGGER_ENABLE:
    rc = stats_enable_trigger(stats, in_params, in_params_size);
    break;
  case ISP_HW_MOD_SET_CONFIG_UPDATE:
    rc = stats_config_update_substats(stats, in_params, in_params_size);
    break;
  case ISP_HW_MOD_SET_ZOOM_RATIO:
    break;
  case ISP_HW_MOD_SET_STATS_FULLSIZE_CFG:
    rc = isp_set_stats_fullsize_config(stats, in_params, in_params_size);
    break;
  default:
    rc = -EAGAIN; /* nop */
    break;
  }
  return rc;
}

/** stats_get_cs_rs_config:
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

static int stats_get_cs_rs_config(
  isp_stats_mod_t *stats,
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
static int stats_get_params(void *mod_ctrl, uint32_t params_id, void *in_params,
  uint32_t in_params_size, void *out_params, uint32_t out_params_size)
{
  isp_stats_mod_t *stats = mod_ctrl;
  int rc = 0;

  switch ((isp_hw_mod_get_param_id_t)params_id) {
  case ISP_HW_MOD_GET_MOD_ENABLE:
    break;
  case ISP_PIX_GET_SCALER_OUTPUT:
    break;
  case ISP_STATS_GET_STREAM_HANDLE:
    break;
  case ISP_HW_MOD_GET_CS_RS_CONFIG:
    rc = stats_get_cs_rs_config(stats, (isp_cs_rs_config_t *)out_params);
    break;
  default:
    rc = -EAGAIN;
    break;
  }
  return rc;
}

/** stats_action_hw_update:
 *    @stats: pointer to instance private data
 *    @cfg: configuration data
 *    @stats_mask: configuration mask
 *
 * Call action for each open submodule. It will configure hardware.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int stats_action_hw_update(isp_stats_mod_t *stats, uint8_t cfg,
  uint32_t stats_mask)
{
  int i, j;
  int rc = 0;
  uint32_t cmd = ISP_STATS_ACTION_HW_CFG_UPDATE;

  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (stats_mask & (1 << i)) {
      rc = stats->stats_ops[i]->action(stats->stats_ops[i]->ctrl, cmd, NULL, 0);
      if (rc < 0) {
        CDBG_ERROR("%s: rc = %d, stats type = %d\n", __func__, rc, i);
        return rc;
      }
    }
  }
  return rc;
}

/** stats_action_buf_config:
 *    @stats: pointer to instance private data
 *    @cfg: configuration data
 *    @stats_mask: configuration mask
 *
 * Call action for each open submodule. It will config of unconfig buffers.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int stats_action_buf_config(isp_stats_mod_t *stats, uint8_t cfg,
  uint32_t stats_mask)
{
  int i, j;
  int rc = 0;
  uint32_t cmd = ISP_STATS_ACTION_STREAM_BUF_CFG;

  if (!cfg)
    cmd = ISP_STATS_ACTION_STREAM_BUF_UNCFG;
  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (stats_mask & (1 << i)) {
      rc = stats->stats_ops[i]->action(stats->stats_ops[i]->ctrl, cmd, NULL, 0);
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
 *    @mod_ctrl: pointer to instance private data
 *    @action_code: action id
 *    @action_data: action data
 *    @action_data_size: action data size
 *
 * Handle all actions.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int stats_action(void *mod_ctrl, uint32_t action_code, void *action_data,
  uint32_t action_data_size)
{
  int rc = 0;
  isp_stats_mod_t *stats = mod_ctrl;
  uint32_t stats_mask = stats->stats_max_mask;

  switch (action_code) {
  case ISP_HW_MOD_ACTION_HW_UPDATE:
    stats_action_hw_update(stats, 1, stats_mask);
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
    rc = isp_stats_start_streams(stats, stats_mask);
    break;
  case ISP_HW_MOD_ACTION_STREAMOFF:
    stats_mask = *((uint32_t *)action_data);
    rc = isp_stats_stop_streams(stats, stats_mask);
    break;
  case ISP_HW_MOD_ACTION_STATS_PARSE:
    rc = isp_stats_parse(stats, (isp_pipeline_stats_parse_t *)action_data);
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
 *    @stats: pointer to instance private data
 *    @stats_type: determinate which stat module should be open.
 *
 * Handle all actions.
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int stats_open_sub_module(isp_stats_mod_t *stats,
  enum msm_isp_stats_type stats_type)
{
  switch (stats_type) {
  case MSM_ISP_STATS_BG:
    stats->stats_ops[stats_type] = bg_stats32_open(stats, stats_type);
    if (stats->stats_ops[stats_type] == NULL) {
      CDBG_ERROR("%s: cannot open bg stats\n", __func__);
      return -1;
    }
    break;
  case MSM_ISP_STATS_BF:
    if (GET_ISP_MAIN_VERSION(stats->isp_version) == ISP_VERSION_32 &&
        GET_ISP_SUB_VERSION(stats->isp_version) == ISP_REVISION_V3) {
      stats->stats_ops[stats_type] = bf_stats33_open(stats, stats_type);
    } else {
      stats->stats_ops[stats_type] = bf_stats32_open(stats, stats_type);
    }
    if (stats->stats_ops[stats_type] == NULL) {
      CDBG_ERROR("%s: cannot open bf stats\n", __func__);
      return -1;
    }
    break;
  case MSM_ISP_STATS_AWB:
    stats->stats_ops[stats_type] = awb_stats32_open(stats, stats_type);
    if (stats->stats_ops[stats_type] == NULL) {
      CDBG_ERROR("%s: cannot open awb stats\n", __func__);
      return -1;
    }
    break;
  case MSM_ISP_STATS_RS:
    stats->stats_ops[stats_type] = rs_stats32_open(stats, stats_type);
    if (stats->stats_ops[stats_type] == NULL) {
      CDBG_ERROR("%s: cannot open rs stats\n", __func__);
      return -1;
    }
    break;
  case MSM_ISP_STATS_CS:
    stats->stats_ops[stats_type] = cs_stats32_open(stats, stats_type);
    if (stats->stats_ops[stats_type] == NULL) {
      CDBG_ERROR("%s: cannot open cs stats\n", __func__);
      return -1;
    }
    break;
  case MSM_ISP_STATS_BHIST:
    stats->stats_ops[stats_type] = bhist_stats32_open(stats, stats_type);
    if (stats->stats_ops[stats_type] == NULL) {
      CDBG_ERROR("%s: cannot open bhist stats\n", __func__);
      return -1;
    }
    break;
  case MSM_ISP_STATS_IHIST:
    stats->stats_ops[stats_type] = ihist_stats32_open(stats, stats_type);
    if (stats->stats_ops[stats_type] == NULL) {
      CDBG_ERROR("%s: cannot open ihist stats\n", __func__);
      return -1;
    }
    break;
  default:
    stats->stats_ops[stats_type] = NULL;
    return -1;
    break;
  }

  return 0;
}

/** stats_init:
 *    @mod_ctrl: pointer to instance private data
 *    @in_params: input data
 *    @notify_ops: notify
 *
 * Open and initialize all required submodules
 *
 * This function executes in ISP thread context
 *
 * Return 0 on success.
 **/
static int stats_init(void *mod_ctrl, void *in_params,
  isp_notify_ops_t *notify_ops)
{
  isp_stats_mod_t *stats = mod_ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;
  int rc = 0;
  int i;

  stats->fd = init_params->fd;
  stats->notify_ops = notify_ops;
  stats->stats_max_mask = init_params->max_stats_mask;
  stats->stats_cfg_mask = init_params->max_stats_mask;
  stats->stats_enable_mask = init_params->max_stats_mask;
  stats->buf_mgr = init_params->buf_mgr;
  stats->dev_idx = init_params->dev_idx;

  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (stats->stats_max_mask == 0)
      break;

    if (stats->stats_max_mask & (1 << i)) {
      rc = stats_open_sub_module(stats, (enum msm_isp_stats_type)i);
      if (rc < 0) {
        CDBG_ERROR("%s: cannot open stats %d\n", __func__, i);
        return rc;
      }

      rc = stats->stats_ops[i]->init(stats->stats_ops[i]->ctrl, in_params,
        notify_ops);
      if (rc < 0) {
        CDBG_ERROR("%s: cannot init stats %d\n", __func__, i);
        return rc;
      }
    }
  }
  return 0;
}

/** stats32_open:
 *    @version: version of
 *
 * Allocate instance private data for module.
 *
 * This function executes in ISP thread context
 *
 * Return pointer to struct which contain module operations.
 **/
isp_ops_t *stats32_open(uint32_t version)
{
  isp_stats_mod_t *stats = malloc(sizeof(isp_stats_mod_t));

  if (!stats) {
    CDBG_ERROR("%s: no mem", __func__);
    return NULL;
  }

  memset(stats, 0, sizeof(isp_stats_mod_t));
  stats->ops.ctrl = (void *)stats;
  stats->isp_version = version;
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

