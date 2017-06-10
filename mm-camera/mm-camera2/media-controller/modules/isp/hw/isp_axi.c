/*============================================================================
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <stdlib.h>
#include <sys/ioctl.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/videodev2.h>

#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_axi.h"
#include "isp_axi_util.h"
#include "isp_hw.h"
#include "isp.h"
#include "isp_log.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#ifdef ENABLE_ISP_AXI_LOGGING
  #undef CDBG
  #define CDBG ALOGE
#endif

static int isp_axi_create_stream(isp_axi_t *axi, start_stop_stream_t *params);
static int isp_axi_release_stream(isp_axi_t *axi, start_stop_stream_t *params);

/** isp_axi_stream_config:
 *
 *    @axi:
 *    @in_params:
 *    @in_params_size:
 *
 **/
static int isp_axi_stream_config(isp_axi_t *axi,
  isp_hwif_output_cfg_t *in_params, uint32_t in_params_size)
{
  int i, rc = 0;
  isp_axi_stream_t *stream = NULL;
  start_stop_stream_t create_stream_param;
  uint8_t buf_divert;
  uint32_t stream_id;

  ISP_DBG(ISP_MOD_COM,"%s: E", __func__);

  for (i = 0; i < ISP_AXI_STREAM_MAX; i++) {
    /* for first time config we reserve the stream */
    if (axi->streams[i].state == ISP_AXI_STREAM_STATE_INVALID) {
      if (stream == NULL)
        stream= &axi->streams[i];
    }

    if (axi->streams[i].state != ISP_AXI_STREAM_STATE_INVALID &&
        axi->streams[i].cfg.stream_param.stream_id == in_params->stream_param.stream_id &&
        axi->streams[i].cfg.stream_param.session_id == in_params->stream_param.session_id) {
      stream = &axi->streams[i];
      break;
    }
  }

  if (stream == NULL) {
    CDBG_ERROR("%s: no more stream slots\n", __func__);
    return -1;
  }

  stream->cfg = *in_params;
  stream->state = ISP_AXI_STREAM_STATE_CFG;
  create_stream_param.num_streams = 1;
  create_stream_param.session_id = stream->cfg.stream_param.session_id;
  stream_id = stream->cfg.stream_param.stream_id;
  create_stream_param.stream_ids = &stream_id;

  if (stream->cfg.need_divert) {
    buf_divert = 1;
  } else {
    buf_divert = 0;
  }

  if (!stream->axi_stream_handle) {
    /* have not create the kernel axi stream yet. Create it */
    rc = isp_axi_create_stream(axi, &create_stream_param);
    if (rc < 0)
      CDBG_ERROR("%s: isp_axi_wm_cfg error = %d\n", __func__, rc);
  }

  ISP_DBG(ISP_MOD_COM,"%s: X, rc = %d, session_id = %d, stream_id = %d",
    __func__, rc, stream->cfg.stream_param.session_id,
    stream->cfg.stream_param.stream_id);

  return rc;
}

/** isp_dump_axi_plane_config:
 *
 *    @stream:
 *    @axi_plane_cfg:
 *    @cam_format:
 *
 **/
static void isp_dump_axi_plane_config(isp_axi_stream_t *stream,
  struct msm_vfe_axi_plane_cfg *axi_plane_cfg, cam_format_t cam_format)
{
  uint32_t i;
  uint32_t plane_num =0;
  uint32_t hw_stream_id;

  if (stream == NULL)
    return;

  if (stream->cfg.use_native_buf) {
    hw_stream_id = stream->cfg.stream_param.stream_id |
      ISP_NATIVE_BUF_BIT;
  } else {
    hw_stream_id = stream->cfg.stream_param.stream_id;
  }

  ISP_DBG(ISP_MOD_COM,"%s:=== AXI DUMP: session_id %d, stream_id %x, hw_stream_id %x ====\n",
    __func__, stream->cfg.stream_param.session_id, stream->cfg.stream_param.stream_id, hw_stream_id);

  switch (cam_format) {
  case CAM_FORMAT_YUV_420_NV12_VENUS:
  case CAM_FORMAT_YUV_420_NV12:
  case CAM_FORMAT_YUV_420_NV21:
  case CAM_FORMAT_YUV_420_NV21_ADRENO:
  case CAM_FORMAT_YUV_422_NV16:
  case CAM_FORMAT_YUV_422_NV61: {
    /* two planes */
    plane_num = 2;
  }
    break;

  case CAM_FORMAT_YUV_420_YV12: {
    /* 3 planes */
    plane_num = 3;
  }
    break;

  default: {
    /*single plane*/
    plane_num = 1;
  }
    break;
  }

  for (i = 0; i < plane_num; i++) {
    ISP_DBG(ISP_MOD_COM,"%s: plane[%d]: plane_fmt %d, address_offset %x\n", __func__, i,
       axi_plane_cfg[i].output_plane_format, /*Y/Cb/Cr/CbCr*/
       axi_plane_cfg[i].plane_addr_offset);
    ISP_DBG(ISP_MOD_COM,"%s: plane[%d]: width = %d\n", __func__, i,
      axi_plane_cfg[i].output_width);
    ISP_DBG(ISP_MOD_COM,"%s: plane[%d]: height = %d\n", __func__, i,
      axi_plane_cfg[i].output_height);
    ISP_DBG(ISP_MOD_COM,"%s: plane[%d]: stride = %d\n", __func__, i,
      axi_plane_cfg[i].output_stride);
    ISP_DBG(ISP_MOD_COM,"%s: plane[%d]: scanlines = %d\n", __func__, i,
      axi_plane_cfg[i].output_scan_lines);
   }

  return;
}

/** isp_axi_stream_unconfig:
 *
 *    @axi:
 *    @in_params:
 *    @in_params_size:
 *
 **/
static int isp_axi_stream_unconfig(isp_axi_t *axi,
  isp_hw_stream_uncfg_t *in_params, uint32_t in_params_size)
{
  int i, rc = 0;
  isp_axi_stream_t *stream = NULL;
  start_stop_stream_t release_stream_param;
  uint32_t stream_id;

  if (in_params_size != sizeof(isp_hw_stream_uncfg_t)) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -1;
  }

  for (i = 0; i < in_params->num_streams; i++) {
    stream= isp_axi_util_find_stream(axi, in_params->session_id, in_params->stream_ids[i]);
    if (stream) {
      /* if axi stream exists in kernel release it */
      if (stream->axi_stream_handle) {
        release_stream_param.num_streams = 1;
        release_stream_param.session_id = stream->cfg.stream_param.session_id;
        stream_id = stream->cfg.stream_param.stream_id;
        release_stream_param.stream_ids = &stream_id;

        rc = isp_axi_release_stream(axi, &release_stream_param);
        if (rc < 0) {
          CDBG_ERROR("%s: release stream error, sessid = %d, streamid = %d, rc = %d\n",
            __func__, stream->cfg.stream_param.session_id, stream_id, rc);
          return rc;
        }
      }

      memset(stream, 0, sizeof(isp_axi_stream_t));
      stream->state = ISP_AXI_STREAM_STATE_INVALID;
    }
  }

  return rc;
}

/** isp_axi_stream_set_skip_pattern:
 *
 *    @axi:
 *    @skip_pattern:
 *    @in_params_size:
 *
 **/
static int isp_axi_stream_set_skip_pattern(isp_axi_t *axi,
  isp_param_frame_skip_pattern_t *skip_pattern, uint32_t in_params_size)
{
  int rc = 0;
  isp_axi_stream_t *stream = NULL;
  struct msm_vfe_axi_stream_update_cmd *update_cmd;

  update_cmd = &axi->update_cmd;
  stream = isp_axi_util_find_stream(axi, skip_pattern->session_id,
             skip_pattern->stream_id);

  if (!stream) {
    CDBG_ERROR("%s: cannot find stream, session_id = %d, stream_id = %d\n",
      __func__, skip_pattern->session_id, skip_pattern->stream_id);
    /* Skip can be applied to either preview or video stream in camcorder mode */
    stream = isp_axi_util_find_active_video_stream(axi, skip_pattern->session_id);
    if (!stream) {
      CDBG_ERROR("%s: cannot find VIDEO stream, session_id = %d, stream_id = %d\n",
        __func__, skip_pattern->session_id, skip_pattern->stream_id);
      return 0;
    }
  }

  update_cmd->num_streams = 1;
  update_cmd->update_info[0].skip_pattern = skip_pattern->pattern;
  update_cmd->update_info[0].stream_handle = stream->axi_stream_handle;
  update_cmd->update_type = UPDATE_STREAM_FRAMEDROP_PATTERN;

  rc = ioctl(axi->fd, VIDIOC_MSM_ISP_UPDATE_STREAM, update_cmd);
  if (rc < 0)
    CDBG_ERROR("%s: MSM_ISP_UPDATE_STREAM error= %d\n", __func__, rc);

  return rc;
}

/** isp_axi_stream_set_stream_update:
 *
 *    @axi:
 *    @stream_cfg:
 *    @in_params_size:
 *
 **/
static int isp_axi_stream_set_stream_update(isp_axi_t *axi,
  isp_hwif_output_cfg_t *stream_cfg, uint32_t in_params_size)
{
  int rc = 0;
  int i;
  isp_axi_stream_t *stream = NULL;
  struct msm_vfe_axi_stream_update_cmd *update_cmd;
  uint32_t num_streams = 0;
  update_cmd = &axi->update_cmd;

  /*PACK different stream cfg toaxi  update command*/
  for (i = 0; i < ISP_PIX_PATH_MAX; i++) {
    stream = isp_axi_util_find_stream(axi,
               stream_cfg[i].stream_param.session_id,
               stream_cfg[i].stream_param.stream_id);
    if (!stream) {
      CDBG_ERROR("%s: cannot find stream, session_id = %d, stream_id = %d\n",
        __func__, stream_cfg[i].stream_param.session_id,
        stream_cfg[i].stream_param.stream_id);
      continue;
    }

    stream->cfg.need_uv_subsample = stream_cfg[i].need_uv_subsample;
    update_cmd->update_type = UPDATE_STREAM_AXI_CONFIG;
    isp_axi_util_fill_plane_info(axi, update_cmd->update_info[i].plane_cfg, stream);
    update_cmd->update_info[num_streams].stream_handle = stream->axi_stream_handle;
    update_cmd->update_info[num_streams].output_format =
        isp_axi_util_cam_fmt_to_v4l2_fmt(stream_cfg[i].stream_param.fmt,
          stream_cfg[i].need_uv_subsample);
    num_streams++;
  }

  update_cmd->num_streams = num_streams;

  if (num_streams)
    axi->hw_update_pending = TRUE;

  return rc;
}

/** isp_axi_set_params:
 *
 *    @ctrl:
 *    @params_id:
 *    @in_params_size:
 *
 **/
int isp_axi_set_params(void *ctrl, uint32_t params_id, void *in_params,
  uint32_t in_params_size)
{
  isp_axi_t *axi = ctrl;
  int rc = 0;

  switch (params_id) {
  case ISP_AXI_SET_STREAM_CFG: {
    rc = isp_axi_stream_config(axi,
           (isp_hwif_output_cfg_t *)in_params, in_params_size);
  }
    break;

  case ISP_AXI_SET_STREAM_UNCFG: {
    rc = isp_axi_stream_unconfig(axi,
           (isp_hw_stream_uncfg_t *)in_params, in_params_size);
  }
    break;

  case ISP_AXI_SET_PARAM_FRAME_SKIP: {
    rc = isp_axi_stream_set_skip_pattern(axi,
           (isp_param_frame_skip_pattern_t *)in_params, in_params_size);
  }
    break;

  case ISP_AXI_SET_STREAM_UPDATE: {
    rc = isp_axi_stream_set_stream_update(axi,
           (isp_hwif_output_cfg_t *)in_params, in_params_size);
  }
     break;

  default: {
  }
    break;
  }

  return rc;
}

/** isp_axi_get_params:
 *
 *    @ctrl:
 *    @params_id:
 *    @in_params:
 *    @in_params_size:
 *    @out_params:
 *    @out_params_size:
 *
 **/
int isp_axi_get_params(void *ctrl, uint32_t params_id, void *in_params,
  uint32_t in_params_size, void *out_params, uint32_t out_params_size)
{
  isp_axi_t *axi = ctrl;
  int rc = 0;

  return rc;
}

/** isp_axi_release_stream:
 *
 *    @axi:
 *    @params:
 *
 **/
static int isp_axi_release_stream(isp_axi_t *axi, start_stop_stream_t *params)
{
  int rc = 0;
  int i;
  isp_axi_stream_t *stream;
  struct msm_vfe_axi_stream_release_cmd *cmd;

  cmd = &axi->work_struct.u.stream_release_cmd;

  for (i = 0; i < params->num_streams; i++) {
    stream = isp_axi_util_find_stream(axi, params->session_id,
               params->stream_ids[i]);
    if (stream == NULL) {
      CDBG_ERROR("%s: cannot find the stream\n", __func__);
      rc = -100;
      goto end;
    }

    memset(&axi->work_struct, 0, sizeof(axi->work_struct));
    cmd->stream_handle = stream->axi_stream_handle;

    rc = ioctl(axi->fd, VIDIOC_MSM_ISP_RELEASE_STREAM, cmd);
    if (rc < 0) {
      CDBG_ERROR("%s: ISP_RELEASE_STREAM error= %d\n", __func__, rc);
      goto end;
    }

    stream->axi_stream_handle = 0;
  }

end:
  return rc;
}

/** isp_axi_create_stream:
 *
 *    @axi:
 *    @params:
 *
 **/
static int isp_axi_create_stream(isp_axi_t *axi, start_stop_stream_t *params)
{
  int rc = 0;
  int i;
  isp_axi_stream_t *stream;
  struct msm_vfe_axi_stream_request_cmd *request_cfg;

  request_cfg = &axi->work_struct.u.stream_request_cmd;

  for (i = 0; i < params->num_streams; i++) {
    stream = isp_axi_util_find_stream(axi, params->session_id,
               params->stream_ids[i]);

    if (stream == NULL) {
      CDBG_ERROR("%s: cannot find the stream\n", __func__);
      rc = -100;
      goto error;
    }

    memset(&axi->work_struct, 0, sizeof(axi->work_struct));
    request_cfg->session_id = stream->cfg.stream_param.session_id;
    request_cfg->vt_enable = stream->cfg.vt_enable;

    if (stream->cfg.use_native_buf) {
      request_cfg->stream_id = stream->cfg.stream_param.stream_id |
        ISP_NATIVE_BUF_BIT;
    } else {
      request_cfg->stream_id = stream->cfg.stream_param.stream_id;
    }

    request_cfg->output_format =
      isp_axi_util_cam_fmt_to_v4l2_fmt(stream->cfg.stream_param.fmt,
        stream->cfg.need_uv_subsample);

    if (request_cfg->output_format == 0) {
      /* invalid format */
      CDBG_ERROR("%s: invalid cam_format %d received, session_id = %d, stream_id = %d",
        __func__, stream->cfg.stream_param.fmt,
        stream->cfg.stream_param.session_id,
        stream->cfg.stream_param.stream_id);
      return -100;
    }

    request_cfg->stream_src = stream->cfg.axi_path; /*CAMIF/IDEAL/RDIs*/
    isp_axi_util_fill_plane_info(axi, request_cfg->plane_cfg, stream);
    request_cfg->hfr_mode = 0; /* TODO: need fix */
    request_cfg->frame_base = stream->cfg.stream_param.frame_base;
    request_cfg->frame_skip_pattern = stream->cfg.frame_skip_pattern;
    request_cfg->burst_count = stream->cfg.stream_param.num_burst;
    request_cfg->init_frame_drop = stream->cfg.stream_param.sensor_skip_cnt;
    request_cfg->axi_stream_handle = 0; /*Return values*/
    request_cfg->buf_divert = stream->cfg.need_divert;
    request_cfg->burst_len = stream->cfg.burst_len;

    isp_dump_axi_plane_config(stream, &request_cfg->plane_cfg[0],
      stream->cfg.stream_param.fmt);

    rc = ioctl(axi->fd, VIDIOC_MSM_ISP_REQUEST_STREAM, request_cfg);
    if (rc < 0) {
      CDBG_ERROR("%s: ISP_REQUEST_STREAM error= %d, session_id = %d, stream_id = %d\n",
        __func__, rc, stream->cfg.stream_param.session_id,
        stream->cfg.stream_param.stream_id);
      goto error;
    }

    /* save the handle */
    stream->axi_stream_handle = request_cfg->axi_stream_handle;
    ISP_DBG(ISP_MOD_COM,"%s: axi_stream: stream id = %d, handle = %x\n", __func__,
      stream->cfg.stream_param.stream_id, stream->axi_stream_handle);
  }

  return 0;
error:
  /* TODO: need to remove already configured streams */
  return rc;
}

/** isp_axi_init_native_buf:
 *
 *    @axi:
 *    @stream:
 *
 **/
static int isp_axi_init_native_buf(isp_axi_t *axi, isp_axi_stream_t *stream)
{
  int rc = 0;

  return rc;
}

/** isp_axi_deinit_native_buf:
 *
 *    @axi:
 *    @stream:
 *
 **/
static int isp_axi_deinit_native_buf(isp_axi_t *axi, isp_axi_stream_t *stream)
{
  int rc = 0;

  return rc;
}

/** isp_axi_deinit_native_buf:
 *
 *    @axi:
 *    @stream:
 *    @buf_idx:
 *    @dirty_buf:
 *
 **/
static int isp_axi_queue_buf(isp_axi_t *axi, isp_axi_stream_t *stream,
  int buf_idx, uint32_t dirty_buf)
{
  int rc = 0;

  rc = isp_queue_buf(axi->buf_mgr, stream->buf_handle, buf_idx, dirty_buf,
         axi->fd);
  if (rc < 0)
    CDBG_ERROR("%s: isp_queue_buf error = %d\n", __func__, rc);

  return rc;
}

/** isp_axi_reg_buf:
 *
 *    @axi:
 *    @params:
 *
 **/
static int isp_axi_reg_buf(isp_axi_t *axi, start_stop_stream_t *params)
{
  int rc = 0;
  int i, k;
  isp_axi_stream_t *stream;
  uint32_t request_stream_id;

  for (i = 0; i < params->num_streams; i++) {
    stream = isp_axi_util_find_stream(axi, params->session_id,
               params->stream_ids[i]);

    if (stream == NULL) {
      CDBG_ERROR("%s: cannot find the stream\n", __func__);
      return -1;
    }

    /* If stream is in desired state, do not configure it */
    if (!(stream->state == ISP_AXI_STREAM_STATE_CFG))
      continue;

    if (stream->cfg.use_native_buf) {
      request_stream_id =
        stream->cfg.stream_param.stream_id | ISP_NATIVE_BUF_BIT;
    } else {
      request_stream_id = stream->cfg.stream_param.stream_id;
    }

    pthread_mutex_lock(&((isp_buf_mgr_t *)axi->buf_mgr)->req_mutex);

    stream->buf_handle = isp_find_matched_bufq_handle(axi->buf_mgr,
                           stream->cfg.stream_param.session_id, request_stream_id);

    pthread_mutex_unlock(&((isp_buf_mgr_t *)axi->buf_mgr)->req_mutex);

    if (stream->buf_handle == 0) {
      CDBG_ERROR("%s: cannot find buf handle, sessid = %d, straemid = %d\n",
        __func__, stream->cfg.stream_param.session_id,
        stream->cfg.stream_param.stream_id);
      return -1;
    }

    rc = isp_register_buf(axi->buf_mgr, stream->buf_handle, axi->fd);
    if (rc < 0) {
      CDBG_ERROR("%s: isp_register_buf error, sessid = %d, straemid = %d\n",
        __func__, stream->cfg.stream_param.session_id,
        stream->cfg.stream_param.stream_id);
      return rc;
    }
  }

  return 0;
}

/** isp_axi_unreg_buf:
 *
 *    @axi:
 *    @params:
 *
 **/
static int isp_axi_unreg_buf(isp_axi_t *axi, start_stop_stream_t *params)
{
  int rc = 0;
  int i, k;
  isp_axi_stream_t *stream;
  struct msm_isp_buf_request *cmd;

  cmd = &axi->work_struct.u.buf_request_cmd;

  for (i = 0; i < params->num_streams; i++) {
    stream = isp_axi_util_find_stream(axi, params->session_id,
               params->stream_ids[i]);
    if (stream == NULL) {
      CDBG_ERROR("%s: cannot find the stream\n", __func__);
      continue;
    }

    /* If stream is in desired state, do not configure it */
    if (!(stream->state == ISP_AXI_STREAM_STATE_CFG))
      continue;

    if (stream->buf_handle == 0) {
      CDBG_ERROR("%s: cannot find buf handle, sessid = %d, straemid = %d\n",
        __func__, stream->cfg.stream_param.session_id,
        stream->cfg.stream_param.stream_id);
      return -1;
    }

    rc = isp_unregister_buf(axi->buf_mgr, stream->buf_handle, axi->fd);
    if (rc < 0) {
      CDBG_ERROR("%s: isp_unregister_buf error, sessid = %d, straemid = %d\n",
        __func__, stream->cfg.stream_param.session_id,
        stream->cfg.stream_param.stream_id);
      return -1;
    }

    CDBG_ERROR("%s: session_id = %d, stream_id = %d, buf_handle = 0x%x\n",
      __func__, stream->cfg.stream_param.session_id,
      stream->cfg.stream_param.stream_id, stream->buf_handle);

    stream->buf_handle = 0;
  }

  return rc;
}

/** isp_axi_streamon:
 *
 *    @axi:
 *    @params:
 *    @start:
 *
 **/
static int isp_axi_streamon(isp_axi_t *axi, start_stop_stream_t *params,
  boolean start)
{
  int rc = 0;
  int i,j;
  isp_axi_stream_t *stream;
  struct msm_vfe_axi_stream_cfg_cmd *cmd;
  struct msm_vfe_axi_src_state src_state;

  ISP_DBG(ISP_MOD_COM,"%s: E, start_flag = %d, sessionid = %d", __func__, start,
    params->session_id);


  cmd = &axi->work_struct.u.stream_start_stop_cmd;
  memset(&axi->work_struct, 0, sizeof(axi->work_struct));
  memset(&src_state, 0, sizeof(src_state));


  j = 0;
  for (i = 0; i < params->num_streams; i++) {

    stream = isp_axi_util_find_stream(axi, params->session_id,
      params->stream_ids[i]);

    if (stream == NULL) {
      CDBG_ERROR("%s: cannot find the stream %d\n", __func__,
        params->stream_ids[i]);
      rc = -100;
      goto end;
    }

    /* If stream is in desired state, do not configure it */
    if (!(((stream->state == ISP_AXI_STREAM_STATE_CFG) && (start == TRUE)) ||
        ((stream->state == ISP_AXI_STREAM_STATE_ACTIVE) && (start == FALSE))))
      continue;

    if (start == TRUE) {
      stream->state = ISP_AXI_STREAM_STATE_ACTIVE;
    } else {
      stream->state = ISP_AXI_STREAM_STATE_CFG;
    }

    cmd->stream_handle[j] = stream->axi_stream_handle;

    if (stream->cfg.need_divert) {
      stream->divert_event_id = ISP_EVENT_BUF_DIVERT +
        (cmd->stream_handle[j] & 0xFF);
      rc = isp_axi_util_subscribe_v4l2_event(axi, stream->divert_event_id,
             start);
      if (rc < 0) {
        CDBG_ERROR("%s: cannot subscribe divert event\n", __func__);
        goto end;
      }
    }
    j++;
  }

  /* No stream for starting/stopping */
  if(j == 0)
    return(0);

  cmd->num_streams = j;

  if (start == TRUE)
    cmd->cmd = START_STREAM;
  else {
    cmd->cmd = STOP_IMMEDIATELY;
    if (!params->stop_immediately)
      cmd->cmd = STOP_STREAM;
  }

  rc = ioctl(axi->fd, VIDIOC_MSM_ISP_CFG_STREAM, cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: ISP_CFG_STREAM error = %d, start_straem = %d\n",
               __func__, rc, start);
    stream->state = ISP_AXI_STREAM_STATE_INVALID;
  }

  if (start == TRUE) {
    if (params->num_streams && params->frame_id != 0) {
      if ((stream->cfg.isp_output_interface == ISP_INTF_RDI0
        || stream->cfg.isp_output_interface == ISP_INTF_RDI1
        || stream->cfg.isp_output_interface == ISP_INTF_RDI2)) {
        src_state.input_src = (enum msm_vfe_input_src)stream->cfg.isp_output_interface;
        params->frame_id += 1;
        src_state.src_frame_id = params->frame_id;
        rc = ioctl(axi->fd, VIDIOC_MSM_ISP_SET_SRC_STATE, &src_state);
        if (rc < 0) {
         CDBG_ERROR("%s: VIDIOC_MSM_ISP_SET_SRC_STATE error = %d, start_straem = %d\n",
             __func__, rc, start);
         stream->state = ISP_AXI_STREAM_STATE_INVALID;
        }
      }
    }
  }

end:
  return rc;
}

/** isp_axi_start_stream:
 *
 *    @axi:
 *    @action_data:
 *    @action_data_size:
 *
 **/
static int isp_axi_start_stream(isp_axi_t *axi,
  start_stop_stream_t *action_data, uint32_t action_data_size)
{
  int rc = 0, rc_unreg = 0;
  boolean start = TRUE;

  if (sizeof(start_stop_stream_t) != action_data_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -100;
  }

  rc = isp_axi_reg_buf(axi, action_data);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_axi_reg_buf error = %d\n", __func__, rc);
    goto error;
  }

  rc = isp_axi_streamon(axi, action_data, start);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_axi_start_stop error = %d\n", __func__, rc);
    goto unreg_buf;
  }

  return rc;

unreg_buf:
  rc_unreg = isp_axi_unreg_buf(axi, action_data);
  if (rc_unreg) {
    CDBG_ERROR("%s: rc_unreg = %d\n", __func__, rc_unreg);
  }
error:
  return rc;
}

/** isp_axi_stop_stream:
 *
 *    @axi:
 *    @action_data:
 *    @action_data_size:
 *
 **/
static int isp_axi_stop_stream(isp_axi_t *axi,
  start_stop_stream_t *action_data, uint32_t action_data_size)
{
  int rc = 0;
  boolean start = FALSE;

  if (sizeof(start_stop_stream_t) != action_data_size) {
    CDBG_ERROR("%s: size mismatch\n", __func__);
    return -100;
  }

  rc = isp_axi_streamon(axi, action_data, start);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_axi_start_stop error = %d\n", __func__, rc);
    /* This error cannot be recovered. WE halt AXI,
     * reset VFE, free memory, send error to media bus */
    /* TODO */
  }
  rc = isp_axi_unreg_buf(axi, action_data);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_axi_wm_uncfg error = %d\n", __func__, rc);
  }

  return rc;
}

/** isp_axi_divert_ack:
 *
 *    @axi:
 *    @ack:
 *    @data_size:
 *
 **/
static int isp_axi_divert_ack(isp_axi_t *axi, isp_hw_buf_divert_ack_t *ack,
  uint32_t data_size)
{
  int rc = 0;
  isp_axi_stream_t *stream;

  stream = isp_axi_util_find_stream(axi, ack->session_id, ack->stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: not find stream, sesid = %d, streamid = %d, nop\n",
      __func__, ack->session_id, ack->stream_id);
    goto end;
  }

  rc = isp_axi_queue_buf(axi, stream, ack->buf_idx, ack->is_buf_dirty);
  if (rc < 0)
    CDBG_ERROR("%s: ISP_ENQUEUE_BUF error = %d\n", __func__, rc);

end:
  return rc;

}

/** isp_axi_do_hw_update:
 *
 *    @axi:
 *
 **/
static int isp_axi_do_hw_update(isp_axi_t *axi)
{
  int rc = 0;
  struct msm_vfe_axi_stream_update_cmd *update_cmd = &axi->update_cmd;

  if (axi->hw_update_pending) {
    rc = ioctl(axi->fd, VIDIOC_MSM_ISP_UPDATE_STREAM, update_cmd);
    if (rc < 0)
      CDBG_ERROR("%s: MSM_ISP_UPDATE_STREAM error= %d\n", __func__, rc);

    axi->hw_update_pending = FALSE;
  }

  return rc;
}

/** isp_axi_action:
 *
 *    @axi:
 *
 **/
int isp_axi_action(void *ctrl, uint32_t action_code, void *action_data,
  uint32_t action_data_size)
{
  isp_axi_t *axi = ctrl;
  int rc = 0;

  switch (action_code) {
  case ISP_AXI_ACTION_CODE_STREAM_START: {
    rc = isp_axi_start_stream(axi, action_data, action_data_size);
  }
    break;

  case ISP_AXI_ACTION_CODE_STREAM_STOP: {
    rc = isp_axi_stop_stream(axi, action_data, action_data_size);
  }
    break;

  case ISP_AXI_ACTION_CODE_STREAM_DIVERT_ACK: {
    rc = isp_axi_divert_ack(axi, action_data, action_data_size);
  }
    break;

  case ISP_AXI_ACTION_CODE_HW_UPDATE: {
    rc = isp_axi_do_hw_update(axi);
  }
    break;
  case ISP_AXI_ACTION_CODE_HALT: {
    rc = isp_axi_halt(axi);
  }
    break;
  case ISP_AXI_ACTION_CODE_RESET: {
    rc = isp_axi_reset(axi, action_data, action_data_size);
  }
    break;
  case ISP_AXI_ACTION_CODE_RESTART: {
    rc = isp_axi_restart(axi);
  }
    break;

  default: {
  }
    break;
  }
  return rc;
}

/** isp_axi_destroy:
 *
 *    @ctrl:
 *
 **/
int isp_axi_destroy(void *ctrl)
{
  isp_axi_t *axi = ctrl;

  /* TODO: if AXI running halt axi */
  free(axi);

  return 0;
}

/** isp_axi_init:
 *
 *    @ctrl: axi ptr
 *    @in_params: int num of axi outputs
 *    @parent:
 *
 **/
int isp_axi_init(void *ctrl, void *in_params, void *parent)
{
  isp_axi_t *axi = ctrl;
  int i, rc = 0;

  axi->parent = parent;

  return rc;
}

/** isp_hw_create_axi:
 *
 *    @fd:
 *    @isp_version:
 *    @dev_idx:
 *    @buf_mgr:
 *
 **/
void *isp_hw_create_axi(int fd, uint32_t isp_version, int dev_idx,
  void *buf_mgr)
{
  int i;
  int rc = 0;

  isp_axi_t *axi = NULL;

  axi = malloc(sizeof(isp_axi_t));
  if (!axi) {
    /* no mem */
    ISP_DBG(ISP_MOD_COM,"%s: error, no mem", __func__);
    return NULL;
  }

  memset(axi, 0, sizeof(isp_axi_t));
  axi->fd = fd;
  axi->isp_version = isp_version;
  axi->parent = NULL;
  axi->dev_idx = dev_idx;
  axi->buf_mgr = buf_mgr;

  return axi;
}

int isp_axi_halt(isp_axi_t *axi)
{
  int rc = 0;
  struct msm_vfe_axi_halt_cmd *halt_cmd;

  halt_cmd = &axi->work_struct.u.halt_cmd;
  memset(halt_cmd, 0, sizeof(struct msm_vfe_axi_halt_cmd));
  halt_cmd->stop_camif = 1;
  halt_cmd->overflow_detected = 1;

  rc = ioctl(axi->fd, VIDIOC_MSM_ISP_AXI_HALT, halt_cmd);
  if (rc < 0) {
    CDBG_ERROR("%s ioctl VIDIOC_MSM_ISP_AXI_HALT failed \n", __func__);
  }

  return rc;
}

int isp_axi_reset(isp_axi_t *axi, void *action_data,
  uint32_t action_data_size)
{
  int rc = 0;
  struct msm_vfe_axi_reset_cmd *reset_cmd;
  uint32_t *frame_id = NULL;

  if (!action_data) {
    CDBG_ERROR("%s Error! Invalid arguments \n", __func__);
    return -1;
  }
  if (action_data_size != sizeof(uint32_t)) {
    CDBG_ERROR("%s Error! Size mismatch \n", __func__);
    return -1;
  }

  frame_id = (uint32_t *)action_data;
  reset_cmd = &axi->work_struct.u.reset_cmd;
  memset(reset_cmd, 0, sizeof(struct msm_vfe_axi_reset_cmd));
  reset_cmd->blocking = 1;
  reset_cmd->frame_id = (unsigned long)(*frame_id);
  rc = ioctl(axi->fd, VIDIOC_MSM_ISP_AXI_RESET, reset_cmd);
  if (rc < 0) {
    CDBG_ERROR("%s ioctl VIDIOC_MSM_ISP_AXI_RESET failed \n", __func__);
  }
  return rc;
}

int isp_axi_restart(isp_axi_t *axi)
{
  int rc = 0;
  struct msm_vfe_axi_restart_cmd *restart_cmd;

  restart_cmd = &axi->work_struct.u.restart_cmd;
  memset(restart_cmd, 0, sizeof(struct msm_vfe_axi_restart_cmd));
  restart_cmd->enable_camif = 1;

  rc = ioctl(axi->fd, VIDIOC_MSM_ISP_AXI_RESTART, restart_cmd);
  if (rc < 0) {
    CDBG_ERROR("%s ioctl VIDIOC_MSM_ISP_AXI_RESTART failed \n", __func__);
  }
  return rc;
}
