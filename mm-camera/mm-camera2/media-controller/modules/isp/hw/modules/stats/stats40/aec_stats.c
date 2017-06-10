/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <unistd.h>
#include "aec_stats.h"

/** aec_stats_set_params:
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
static int aec_stats_set_params (void *ctrl, uint32_t param_id,
  void *in_params, uint32_t in_param_size)
{
  isp_stats_entry_t *entry = ctrl;
  int rc = 0;
  switch (param_id) {
  case ISP_STATS_SET_ENABLE:
    break;
  case ISP_STATS_SET_CONFIG:
    break;
  case ISP_STATS_SET_TRIGGER_ENABLE:
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

/** aec_stats_get_params:
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
static int aec_stats_get_params (void *ctrl, uint32_t param_id, void *in_params,
  uint32_t in_param_size, void *out_params, uint32_t out_param_size)
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
  default:
    break;
  }
  return rc;
}

/** aec_stats_action:
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
static int aec_stats_action (void *ctrl, uint32_t action_code,
  void *data, uint32_t data_size)
{
  int rc = 0;
  isp_stats_entry_t *entry = ctrl;
  switch (action_code) {
  case ISP_STATS_ACTION_STREAM_START:
    break;
  case ISP_STATS_ACTION_STREAM_STOP:
    break;
  case ISP_STATS_ACTION_HW_CFG_UPDATE:
    break;
  case ISP_STATS_ACTION_RESET:
    isp_stats_reset(entry);
    break;
  default:
    break;
  }
  return rc;
}

/** aec_stats_init:
 *    @ctrl: Pointer to entry (ctrl) struct
 *    @in_params: input params
 *    @notify_ops: notify_ops function pointer
 *
 *  Set initial values in the aec stats entry
 *
 *
 *  Return 0 for Success, negative if ERROR
 **/
static int aec_stats_init (void *ctrl, void *in_params, isp_notify_ops_t *notify_ops)
{
  isp_stats_entry_t *entry = ctrl;
  isp_hw_mod_init_params_t *init_params = in_params;

  entry->fd = init_params->fd;
  entry->notify_ops = notify_ops;
  entry->dev_idx = init_params->dev_idx;
  entry->buf_mgr = init_params->buf_mgr;
  isp_stats_reset(entry);
  return 0;
}

/** aec_stats_destroy:
 *    @ctrl: Pointer to the entry struct for aec stats
 *
 *  Free the memory for the aec stats module
 *
 *
 * Return 0 on Success, negative on ERROR
 **/
static int aec_stats_destroy (void *ctrl)
{
  isp_stats_entry_t *entry = ctrl;
  free(entry->reg_cmd);
  free(entry);
  return 0;
}

/** aec_stats_open:
 *    @stats: Pointer to the ISP stats module
 *    @stats_type: enum value that gives what stats this is
 *
 *  Allocate and Initialize memory for aec stats module
 *
 *
 *  Return pointer to isp ops. NULL if ERROR
 **/
isp_ops_t *aec_stats_open(isp_stats_mod_t *stats,
  enum msm_isp_stats_type stats_type)
{
  int rc = 0;
  isp_stats_entry_t *entry = NULL;
  ISP_StatsAe_CfgCmdType *cmd = NULL;

  entry = malloc(sizeof(isp_stats_entry_t));
  if (!entry) {
    CDBG_ERROR("%s: no mem for aec\n",  __func__);
    return NULL;
  }
  cmd = malloc(sizeof(ISP_StatsAe_CfgCmdType));
  if (!cmd) {
    CDBG_ERROR("%s: no mem\n",  __func__);
    free(entry);
    return NULL;
  }
  memset(entry, 0, sizeof(isp_stats_entry_t));
  memset(cmd, 0, sizeof(*cmd));
  entry->reg_cmd = cmd;
  entry->ops.ctrl = (void *)entry;
  entry->ops.init = aec_stats_init;
  /* destroy the module object */
  entry->ops.destroy = aec_stats_destroy;
  /* set parameter */
  entry->ops.set_params = aec_stats_set_params;
  /* get parameter */
  entry->ops.get_params = aec_stats_get_params;
  entry->ops.action = aec_stats_action;
  return &entry->ops;
}
