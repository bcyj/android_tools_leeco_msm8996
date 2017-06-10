/*============================================================================
Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/media.h>
#include <media/msmb_isp.h>
#include "isp_log.h"

#include "isp_stats.h"
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#if 0
#undef ISP_DBG
#define ISP_DBG ALOGE
#endif

#undef CDBG_ERROR
#define CDBG_ERROR ALOGE

/*===========================================================================
 * FUNCTION    - isp_stats_get_stream_id -
 *
 * DESCRIPTION:
 *==========================================================================*/
static uint32_t isp_stats_get_stream_id(isp_stats_entry_t *entry)
{
  uint32_t stream_id = entry->stats_type |
    ISP_NATIVE_BUF_BIT | ISP_STATS_STREAM_BIT;

  return stream_id;
}

/*===========================================================================
 * FUNCTION    - isp_stats_alloc_native_buf -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int isp_stats_alloc_native_buf(int ion_fd, isp_frame_buffer_t *bufs, int num_bufs,
  cam_frame_len_offset_t *len_offset)
{
  int i, j, rc = 0;
  int cached = 1;

  ISP_DBG(ISP_MOD_STATS, "%s: num_buf = %d\n", __func__, num_bufs);

  for (i = 0; i < num_bufs; i++) {
    rc = isp_init_native_buffer(&bufs[i], i, ion_fd, len_offset, cached);
    if (rc < 0)
      goto error;
  }
  return 0;

error:
  for (j = 0; j < i; j++)
    isp_deinit_native_buffer(&bufs[j], ion_fd);
  return -1;
}

/*===========================================================================
 * FUNCTION    - isp_stats_release_native_buf -
 *
 * DESCRIPTION:
 *==========================================================================*/
static void isp_stats_release_native_buf(int ion_fd, int num_bufs, isp_frame_buffer_t *bufs)
{
  int i;
  for (i = 0; i < num_bufs; i++)
    isp_deinit_native_buffer(&bufs[i], ion_fd);
}

/*===========================================================================
 * FUNCTION    - isp_stats_enqueue_buf -
 *
 * DESCRIPTION:
 *==========================================================================*/
int isp_stats_enqueue_buf(isp_stats_entry_t *entry, int buf_idx)
{
  int rc = 0;
  /* this fuc only called during parsing so dirty bit always 1*/
  uint32_t dirty_buf = 1;

  rc = isp_queue_buf(entry->buf_mgr, entry->buf_handle,
    buf_idx, dirty_buf, entry->fd);

  if (rc < 0)
    CDBG_ERROR("%s: isp_queue_buf error = %d\n", __func__, rc);
  return rc;
}

/*===========================================================================
 * FUNCTION    - isp_stats_reg_buf -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int isp_stats_reg_buf(isp_stats_entry_t *entry)
{
  int rc = 0;
  isp_buf_request_t buf_request;
  uint32_t stats_stream_id = isp_stats_get_stream_id(entry);


  memset(&buf_request, 0, sizeof(buf_request));

  if (entry->buf_mgr == NULL) {
    CDBG_ERROR("%s: no buf_mgr ptr\n", __func__);
    return -1;
  }

  if (entry->num_bufs > ISP_STATS_MAX_BUFS) {
    CDBG_ERROR("%s: num_buf %d > max number %d\n",
       __func__, entry->num_bufs, ISP_STATS_MAX_BUFS);
    return -1;
  }

  buf_request.buf_handle = 0;
  buf_request.session_id = entry->session_id;
  buf_request.stream_id = isp_stats_get_stream_id(entry);
  buf_request.use_native_buf = 1;
  /*For deferred buffer allocation, HAL buffers are updated after streamon
  For non HAL buffers current and total number of buffers is same*/
  buf_request.total_num_buf = entry->num_bufs;
  buf_request.current_num_buf = entry->num_bufs;
  buf_request.buf_info.num_planes = 1;
  buf_request.buf_info.mp[0].len = entry->buf_len;
  buf_request.buf_info.mp[0].offset = 0;
  buf_request.buf_info.frame_len = entry->buf_len;
  buf_request.img_buf_list = NULL;
  buf_request.cached = 1;

  if (entry->is_ispif_split)
    buf_request.buf_type = ISP_SHARE_BUF;
  else
    buf_request.buf_type = ISP_PRIVATE_BUF;

  pthread_mutex_lock(&((isp_buf_mgr_t *)entry->buf_mgr)->req_mutex);
  entry->buf_handle = isp_find_matched_bufq_handle(entry->buf_mgr,
    entry->session_id, stats_stream_id);
  if (entry->buf_handle == 0) {
    rc = isp_request_buf(entry->buf_mgr, &buf_request);
    if (rc < 0) {
      CDBG_ERROR("%s: isp_request_buf error= %d\n", __func__, rc);
      pthread_mutex_unlock(&((isp_buf_mgr_t *)entry->buf_mgr)->req_mutex);
      return -1;
    }
    entry->buf_handle = buf_request.buf_handle;
  }
  pthread_mutex_unlock(&((isp_buf_mgr_t *)entry->buf_mgr)->req_mutex);

  rc = isp_register_buf(entry->buf_mgr, entry->buf_handle, entry->fd);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_register_buf error= %d\n", __func__, rc);
    pthread_mutex_lock(&((isp_buf_mgr_t *)entry->buf_mgr)->req_mutex);
    isp_release_buf(entry->buf_mgr, entry->buf_handle);
    pthread_mutex_unlock(&((isp_buf_mgr_t *)entry->buf_mgr)->req_mutex);
    entry->buf_handle= 0;
    return -1;
  }

  return rc;
}

/*===========================================================================
 * FUNCTION    - isp_stats_unreg_buf -
 *
 * DESCRIPTION:
 *==========================================================================*/
static int isp_stats_unreg_buf(isp_stats_entry_t *entry)
{
  int rc = 0;

  if (entry->buf_handle == 0) {
    CDBG_ERROR("%s: error, buf_handle == 0\n", __func__);
    return -1;
  }
  rc = isp_unregister_buf(entry->buf_mgr, entry->buf_handle, entry->fd);
  if (rc < 0)
    CDBG_ERROR("%s: isp_register_buf error= %d\n", __func__, rc);

  pthread_mutex_lock(&((isp_buf_mgr_t *)entry->buf_mgr)->req_mutex);
  if (rc == 0)
    isp_release_buf(entry->buf_mgr, entry->buf_handle);
  pthread_mutex_unlock(&((isp_buf_mgr_t *)entry->buf_mgr)->req_mutex);
  entry->buf_handle = 0;
  entry->num_bufs = 0;

  return rc;
}

/*===========================================================================
 * FUNCTION    - isp_stats_config_stream -
 *
 * DESCRIPTION:
 *==========================================================================*/
int isp_stats_config_stats_stream(isp_stats_entry_t *entry, int num_bufs)
{
  int rc = 0;
  struct msm_vfe_stats_stream_request_cmd req_cmd;

  entry->num_bufs = num_bufs;
  memset(&req_cmd, 0, sizeof(req_cmd));
  req_cmd.session_id = entry->session_id;
  req_cmd.stream_id = isp_stats_get_stream_id(entry);
  req_cmd.stats_type = entry->stats_type;
  req_cmd.buffer_offset = entry->buf_offset;
  req_cmd.composite_flag = entry->comp_flag;

  switch (entry->hfr_mode) {
  case CAM_HFR_MODE_OFF:
    req_cmd.framedrop_pattern = NO_SKIP;
    break;
  case CAM_HFR_MODE_60FPS:
    req_cmd.framedrop_pattern = EVERY_2FRAME;
    break;
  case CAM_HFR_MODE_90FPS:
    req_cmd.framedrop_pattern = EVERY_3FRAME;
    break;
  case CAM_HFR_MODE_120FPS:
    req_cmd.framedrop_pattern = EVERY_4FRAME;
    break;
  case CAM_HFR_MODE_150FPS:
    req_cmd.framedrop_pattern = EVERY_5FRAME;
    break;
  default:
    req_cmd.framedrop_pattern = NO_SKIP;
  }

  rc = ioctl(entry->fd, VIDIOC_MSM_ISP_REQUEST_STATS_STREAM, &req_cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: cannot request stream for stats 0x%x\n",
      __func__, entry->stats_type);
    return rc;
  }

  entry->stream_handle = req_cmd.stream_handle;
  rc = isp_stats_reg_buf(entry);
  if (rc < 0) {
     ISP_DBG(ISP_MOD_STATS, "%s: isp request buffer failed, rc = %d\n", __func__, rc);
    struct msm_vfe_stats_stream_release_cmd rel_cmd;
    rel_cmd.stream_handle = entry->stream_handle;
    ioctl(entry->fd, VIDIOC_MSM_ISP_RELEASE_STATS_STREAM, &rel_cmd);
  }

  return rc;
}

/*===========================================================================
 * FUNCTION    - isp_stats_unconfig_stream -
 *
 * DESCRIPTION:
 *==========================================================================*/
int isp_stats_unconfig_stats_stream(isp_stats_entry_t *entry)
{
  int rc = 0;
  struct msm_vfe_stats_stream_release_cmd rel_cmd;
  rel_cmd.stream_handle = entry->stream_handle;

  isp_stats_unreg_buf(entry);
  ISP_DBG(ISP_MOD_STATS, "%s: entry->fd = %d, rel_cmd = %p\n", __func__, entry->fd, &rel_cmd);
  rc = ioctl(entry->fd, VIDIOC_MSM_ISP_RELEASE_STATS_STREAM, &rel_cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: cannot release stream for stats 0x%x\n",
      __func__, entry->stats_type);
  }

  entry->stream_handle = 0;
  return rc;
}

/*===========================================================================
 * FUNCTION    - isp_stats_start_streams -
 *
 * DESCRIPTION:
 *==========================================================================*/
int isp_stats_start_streams(
  isp_stats_mod_t *mod,
  uint32_t stats_mask)
{
  int rc = 0, i;
  struct msm_vfe_stats_stream_cfg_cmd cmd;
  uint32_t handle;
  memset(&cmd, 0, sizeof(cmd));
  pthread_mutex_init(&mod->parse_stats_mutex, NULL);

  cmd.enable = 1;
  cmd.stats_burst_len = mod->stats_burst_len;
  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (stats_mask & (1 << i)) {
      mod->stats_ops[i]->get_params(mod->stats_ops[i]->ctrl,
        ISP_STATS_GET_STREAM_HANDLE, NULL, 0, &handle, sizeof(handle));
      cmd.stream_handle[cmd.num_streams++] = handle;
    }
  }

  rc = ioctl(mod->fd, VIDIOC_MSM_ISP_CFG_STATS_STREAM, &cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: cannot start, stats mask = 0x%x\n",
    __func__, stats_mask);
  }

  return rc;
}

/*===========================================================================
 * FUNCTION    - isp_stats_stop_streams -
 *
 * DESCRIPTION:
 *==========================================================================*/
int isp_stats_stop_streams(
  isp_stats_mod_t *mod,
  uint32_t stats_mask)
{
  int rc = 0, i;
  struct msm_vfe_stats_stream_cfg_cmd cmd;
  uint32_t handle;

  memset(&cmd, 0, sizeof(cmd));

  cmd.enable = 0;
  /* Wait for stats parsing to complete */
  pthread_mutex_lock(&mod->parse_stats_mutex);
  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (stats_mask & (1 << i)) {
      mod->stats_ops[i]->get_params(mod->stats_ops[i]->ctrl,
        ISP_STATS_GET_STREAM_HANDLE, NULL, 0, &handle, sizeof(handle));
      cmd.stream_handle[cmd.num_streams++] = handle;
      /* send stop action to each stats in
       * case they need to do local cleanup */
      mod->stats_ops[i]->action(mod->stats_ops[i]->ctrl,
        ISP_STATS_ACTION_STREAM_STOP, NULL, 0);
    }
  }

  rc = ioctl(mod->fd, VIDIOC_MSM_ISP_CFG_STATS_STREAM, &cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: cannot stop, stats mask = 0x%x\n",
    __func__, stats_mask);
  }

  pthread_mutex_unlock(&mod->parse_stats_mutex);
  pthread_mutex_destroy(&mod->parse_stats_mutex);
  return rc;
}

int isp_stats_do_reset(isp_stats_mod_t *mod)
{
  int i;
  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
  if (mod->stats_ops[i] && mod->stats_ops[i]->action)
      mod->stats_ops[i]->action(mod->stats_ops[i]->ctrl,
        ISP_STATS_ACTION_RESET, NULL, 0);
  }
  return 0;
}
int isp_stats_parse(isp_stats_mod_t *mod,
  isp_pipeline_stats_parse_t *action_data)
{
  struct msm_isp_event_data *raw_event = action_data->raw_stats_event;
  uint32_t stats_mask = raw_event->u.stats.stats_mask;
  int rc = 0, i;

  action_data->parsed_stats_event->frame_id = raw_event->frame_id;
  action_data->parsed_stats_event->timestamp = raw_event->timestamp;

  pthread_mutex_lock(&mod->parse_stats_mutex);
  for (i = 0; i < MSM_ISP_STATS_MAX; i++) {
    if (!(stats_mask & (1 << i)))
      continue;
    rc = mod->stats_ops[i]->action(
           mod->stats_ops[i]->ctrl, ISP_STATS_ACTION_STATS_PARSE,
           action_data, sizeof(isp_pipeline_stats_parse_t));
    if (rc < 0) {
      CDBG_ERROR("%s: stats (%d) parsing error = %d\n", __func__, i, rc);
      pthread_mutex_unlock(&mod->parse_stats_mutex);
      return rc;
    }
  }
  pthread_mutex_unlock(&mod->parse_stats_mutex);
  return rc;
}

void isp_stats_reset(isp_stats_entry_t *entry)
{
  entry->hw_update_pending = 0;
  entry->trigger_enable = 0; /* enable trigger update feature flag */
  entry->enable = 0;         /* enable flag from PIX */
  entry->is_used = 0;        /* is this entry used or not */
  entry->state = ISP_STATS_STREAM_STATE_INITTED;
  entry->stream_handle = 0;
  entry->session_id = 0;
  entry->is_first = 1;
  entry->hfr_mode = CAM_HFR_MODE_OFF;
  entry->buf_handle = 0;
  entry->is_ispif_split = 0;
  entry->num_left_rgns = 0;
  entry->num_right_rgns = 0;
}

