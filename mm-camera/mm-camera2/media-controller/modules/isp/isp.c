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
#include <linux/media.h>

#include "camera_dbg.h"
#include "cam_types.h"
#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "isp_def.h"
#include "isp_ops.h"
#include "isp_hw.h"
#include "isp.h"
#include "isp_util.h"
#include "isp_buf_mgr.h"
#include "isp_resource_mgr.h"
#include "q3a_stats.h"
#include "isp_log.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#if ISP_DEBUG
#undef CDBG
#define CDBG ALOGE
#endif
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE
#undef CDBG_HIGH
#define CDBG_HIGH ALOGE

#define PAD_TO_SIZE(size, padding)  ((size + padding - 1) & ~(padding - 1))

static int isp_add_meta_channel(isp_t *isp, isp_session_t *session,
  isp_stream_t *stream);
static int isp_proc_streamon(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, mct_event_t *event,
  isp_session_t *session);
static int isp_proc_streamoff(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, mct_event_t *event,
  isp_session_t *session);

/** prepare_isp_default_params
 *    @session: session in which to save defaults
 *
 *  Set the default values for ISP parametes so they always be valid.
 *
 *  Returns nothing
 **/
static void prepare_isp_default_params(isp_session_t *session)
{
  ISP_DBG(ISP_MOD_COM,"%s: E, session %p", __func__, (void *)session);
  session->saved_params.effect = ISP_DEFAULT_EFFECT;
  session->saved_params.contrast = ISP_DEFAULT_CONTRAST;
  session->saved_params.bestshot = ISP_DEFAULT_BESTSHOT;
  session->saved_params.sce_factor = ISP_DEFAULT_BESTSHOT;
  session->saved_params.saturation = ISP_DEFAULT_SATURATION;
  session->saved_params.sharpness = ISP_DEFAULT_SHARPNESS;
  session->saved_params.vhdr_enable = 0;
  session->saved_params.vt_enable = 0;
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
}

/** isp_start_session
 *    @isp: isp instance
 *    @session_id: session id to be started
 *
 *  Starts new isp session, new instance of ion driver, zoom
 *  session, buffer manager, async task thread.
 *
 *  Returns o for success and negative error for failure
 **/
int isp_start_session(isp_t *isp, uint32_t session_id)
{
  int i;
  isp_session_t *session = NULL;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d", __func__, (void *)isp, session_id);
  for (i = 0; i < ISP_MAX_SESSIONS; i++) {
    if (isp->data.sessions[i].isp_data == NULL) {
      /* save the 1st unused session ptr */
      session = &isp->data.sessions[i];
      memset(session, 0, sizeof(isp_session_t));
      session->ion_fd = isp_open_ion();
      if (session->ion_fd < 0) {
        CDBG_ERROR("%s: Ion device open failed\n", __func__);
        return -1;
      }

      /* open tintless session */
      session->tintless_session = isp_tintless_open_session(isp->data.tintless,
        session->session_id);
      if (session->tintless_session == NULL) {
        CDBG_ERROR("%s: cannot open tintless session\n", __func__);
      }

      /* open zoom session */
      session->zoom_session = isp_zoom_open_session(
        isp->data.zoom, session->session_id);
      if (session->zoom_session == NULL) {
        CDBG_ERROR("%s: cannot open zoom session\n", __func__);
        return -1;
      }
      /* assign default zooom value */
      session->zoom_val = isp_zoom_get_min_zoom(isp->data.zoom);
      session->isp_data = &isp->data;
      session->session_id = session_id;
      session->session_idx = i;

      /* Disabling UV sampling by default */
      session->uv_subsample_mode = ISP_UV_SUBSAMPLE_OFF;
      prepare_isp_default_params(session);
      session->hfr_param.hfr_mode = CAM_HFR_MODE_OFF;
      isp_open_buf_mgr(&isp->data.buf_mgr);
      increase_isp_session();
      if (isp_thread_async_task_start(isp, session)) {
        CDBG_ERROR("%s cannot start async task\n", __func__);
        isp_stop_session(isp, session_id);
        return -1;
      }
      session->is_session_active = TRUE;
      ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
      return 0;
    }
  }
  return -1;
}

/** isp_stop_session
 *    @isp: isp instance
 *    @session_id: session id to be stopped
 *
 *  Stop isp session, close ion driver fd, zoom session,
 *  buffer manager, async task thread and destroy vfes.
 *
 *  Returns o for success and negative error for failure
 **/
int isp_stop_session(isp_t *isp, uint32_t session_id)
{
  int i;
  isp_session_t *session = NULL;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d", __func__, (void *)isp, session_id);
  for (i = 0; i < ISP_MAX_SESSIONS; i++) {
    if (isp->data.sessions[i].isp_data &&
        isp->data.sessions[i].session_id == session_id) {
      session = &isp->data.sessions[i];

      if (session->vfe_ids & (1 << VFE0)) {
        isp_util_destroy_hw(isp, VFE0, 1);
      }
      if (session->vfe_ids & (1 << VFE1)) {
        isp_util_destroy_hw(isp, VFE1, 1);
      }

      isp_close_ion(session->ion_fd);

      if (isp->data.tintless->tintless_data.is_supported)
        isp_tintless_close_session(session->tintless_session);

      isp_zoom_close_session(session->zoom_session);
      isp_thread_async_task_stop(isp, session);
      memset(session, 0, sizeof(isp_session_t));
      isp_close_buf_mgr(&isp->data.buf_mgr);
      decrease_isp_session_cnt();
      ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
      return 0;
    }
  }
  return -1;
}

/** set_all_saved_params
 *    @isp : isp instance
 *    @isp_sink_port: sink port instance
 *    @session_id : current session id
 *    @stream_id : stream id of params
 *
 *  Sets all previously saved parameters to HW.
 *
 *  Returns 0 for success and negative error for failure
 **/
static int set_all_saved_params(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id)
{

  int rc = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return rc;
  }
  ISP_DBG(ISP_MOD_COM,"%s: contrast = %d, satuation = %d, "
    "special effect = %d, bestshot = %d, sce_factor = %d\n",
    __func__,
    session->saved_params.contrast, session->saved_params.saturation,
    session->saved_params.effect, session->saved_params.bestshot,
    session->saved_params.sce_factor);

  rc = isp_util_set_contrast(isp, isp_sink_port,
    session_id, stream_id, &session->saved_params.contrast);
  if (rc) {
    CDBG_ERROR("%s: isp_util_set_contrast error= %d\n", __func__, rc);
    goto END;
  }

  rc = isp_util_set_effect(isp, isp_sink_port,
    session_id, stream_id, &session->saved_params.effect);
  if (rc) {
    CDBG_ERROR("%s: isp_util_set_effect error= %d\n", __func__, rc);
    goto END;
  }

  rc = isp_util_set_saturation(isp, isp_sink_port,
    session_id, stream_id, &session->saved_params.saturation, TRUE);
  if (rc) {
    CDBG_ERROR("%s: isp_util_set_saturation error= %d\n", __func__, rc);
    goto END;
  }

  if (session->saved_params.bestshot != ISP_DEFAULT_BESTSHOT) {
    rc = isp_util_set_bestshot(isp, isp_sink_port,
      session_id, stream_id, &session->saved_params.bestshot);
    if (rc) {
      CDBG_ERROR("%s: isp_util_set_bestshot error= %d\n", __func__, rc);
      goto END;
    }
  }

  rc = isp_util_set_skin_color_enhance(isp, isp_sink_port,
    session_id, stream_id, &session->saved_params.sce_factor);
  if (rc) {
    CDBG_ERROR("%s: isp_util_set_skin_color_enhance error= %d\n", __func__, rc);
    goto END;
  }

  rc = isp_util_set_sharpness(isp, isp_sink_port,
    session_id, stream_id, &session->saved_params.sharpness);
  if (rc) {
    CDBG_ERROR("%s: isp_util_set_skin_color_enhance error= %d\n", __func__, rc);
    goto END;
  }


  /* set tintless data info */
  rc = isp_util_set_tintless(isp, session->session_id,
    &(isp->data.tintless->tintless_data));
  if (rc < 0) {
    CDBG_ERROR("%s: Cannot set tintless info\n", __func__);
  }

  rc = isp_util_set_frame_skip(isp, session->session_id, stream_id,
    (int32_t *)&session->saved_params.frame_skip.pattern);
  if (rc < 0) {
    CDBG_ERROR("%s: Cannot set frameskip info\n", __func__);
  }

  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
END:
  return rc;
}

/** isp_update_buf_info
 *    @isp: isp instance
 *    @session_id: session id
 *    @stream_id: stream id for which buffers will be updated
 *    @event: Contains information of buffer to be appended
 *
 *  This method is used to dynamically update hal buffers for a
 *  stream already running. We receive new buffer info from MCT,
 *  we update internal ISP buffer array and then enqueue to
 *  kernel without stopping the stream.
 *
 *  Returns 0 for success and negative error for failure
 **/
int isp_update_buf_info(isp_t *isp, uint32_t session_id,
  uint32_t stream_id, mct_event_t *event)
{
  int i, rc = 0;
  isp_session_t *session = NULL;
  isp_stream_t *stream = NULL;

  ISP_DBG(ISP_MOD_COM,"%s: E, session_id = %d, stream_id = %d\n", __func__, session_id,
    stream_id);
  session = isp_util_find_session(isp, session_id);
  if (!session) {
    CDBG_ERROR("%s: error, NULL session for session_id = %d\n", __func__,
      session_id);
    rc = -1;
    goto end;
  }

  stream = isp_util_find_stream_in_session(session, stream_id);
  if (!stream) {
    CDBG_ERROR("%s: cannot find stream (streamid = %d)\n",
               __func__, stream_id);
    rc = -1;
    goto end;
  }

  mct_stream_map_buf_t *buf_holder =
    (mct_stream_map_buf_t *)event->u.ctrl_event.control_event_data;
  ISP_DBG(ISP_MOD_COM,"%s:buf_holder %p\n", __func__, buf_holder);

  /* MCT has updated HAL buffers, we update the internal isp_map_buf struct
     and then enqueue bufs to kernel*/

  if (!isp_util_update_hal_image_buf_to_channel(session, stream)) {
    rc = isp_ch_util_reg_buf_list_update(isp, session, stream_id, buf_holder);
    if (rc < 0) {
      CDBG_ERROR("%s: isp_ch_util_reg_buf_list_update failed\n", __func__);
      goto end;
    }
  }

end:
  ISP_DBG(ISP_MOD_COM,"%s: X, rc = %d\n", __func__, rc);
  return rc;
}

/** isp_streamon
 *    @isp : isp instance
 *    @isp_sink_port: sink port instance
 *    @session_id : current session id
 *    @stream_id : stream id of params
 *    @event : mct event object
 *
 *  Starts isp streaming post request to async thread.
 *
 *  Returns 0 for success and negative error for failure
 **/
int isp_streamon(isp_t *isp, isp_port_t *isp_sink_port, uint32_t session_id,
  uint32_t stream_id, mct_event_t *event, boolean flash_streamon)
{
  int i, rc = 0;
  isp_session_t *session = NULL;
  isp_async_cmd_t *cmd;
  int cur_active_stream_cnt = 0;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d, stream_id %d", __func__, (void *)isp,
    session_id, stream_id);

  session = isp_util_find_session(isp, session_id);
  if (!session) {
    CDBG_ERROR("%s: error, NULL session for session_id = %d\n",
      __func__, session_id);
    return -1;
  }

  if (flash_streamon) {
    session->flash_streamon = flash_streamon;
  }

  CDBG_HIGH("%s: E, session_id = %d, stream_id = %d, active_count = %d flash=%s\n",
    __func__, session->session_id, stream_id, session->active_count,
    flash_streamon == TRUE ? "TRUE" : "FALSE");
  cur_active_stream_cnt = session->active_count;

  cmd = malloc(sizeof(isp_async_cmd_t));
  if (!cmd) {
    CDBG_ERROR("%s:error,  no memory for streamon command, session_id = %d\n",
      __func__, session_id);
    return -1;
  }
  memset(cmd, 0, sizeof(isp_async_cmd_t));
  cmd->cmd_id = ISP_ASYNC_COMMAND_STREAMON;
  cmd->streamon.isp = (void *)isp;
  cmd->streamon.isp_sink_port = (void *)isp_sink_port;
  cmd->streamon.session_id = session_id;
  cmd->streamon.stream_id = stream_id;
  cmd->streamon.event = event;
  cmd->streamon.session = (void *)session;
  cmd->streamon.sync_cmd = TRUE;
  pthread_mutex_lock(&session->async_task.sync_mutex);

  rc = isp_enqueue_async_command(isp, session, &cmd);
  if (cmd)
    free(cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: error, isp_enqueue_async_command, session_id = %d\n",
      __func__, session_id);
    goto end;
  }
  sem_wait(&session->async_task.sync_sem);
  rc = session->async_task.sync_ret;

end:
  pthread_mutex_unlock(&session->async_task.sync_mutex);
  if (!cur_active_stream_cnt && session->active_count > 0) {
    /* now we need to enable the in_service flag for buffered params */
    boolean has_params = FALSE;
    pthread_mutex_lock(
      &isp->data.session_critical_section[session->session_idx]);
    has_params = session->buffered_hw_params.new_params.has_params;
    session->buffered_hw_params.in_service = TRUE;
    if (has_params)
      session->buffered_hw_params.hw_update_pending = TRUE;
    pthread_mutex_unlock(
      &isp->data.session_critical_section[session->session_idx]);
    if (has_params) {
      int ret = 0;
      ret = isp_util_send_buffered_hw_params_to_hw(isp, session);
      if (ret < 0 && rc >= 0) {
        CDBG_ERROR("%s: Error = %d\n", __func__, ret);
        rc = ret;
      }
    }
  }

  CDBG_HIGH("%s: X, session_id = %d, rc = %d\n",
    __func__, session->session_id, rc);
  return rc;
}

/** isp_create_channels_for_streamon
 *    @isp : isp instance
 *    @session : session instance
 *
 *  Starts isp streamingusing async thread.
 *
 *  Returns 0 for success and negative error for failure
 **/
static int isp_create_channels_for_streamon(isp_t *isp, isp_session_t *session)
{
  int rc = 0;
  int i;
  isp_stream_t *stream = NULL;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p", __func__, (void *)isp);
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    stream = &session->streams[i];
    if (stream->state != ISP_STREAM_STATE_USER_CFG) {
      continue;
    }
    rc = isp_ch_util_sync_stream_cfg_to_channel(isp, session, stream);
    if (rc < 0) {
      CDBG_ERROR("%s: error, cannot create channel, identity = 0x%X\n",
        __func__, stream->stream_info.identity);
      return rc;
    }
    rc = isp_add_meta_channel(isp, session, stream);
    if (rc < 0) {
      CDBG_ERROR("%s: error, cannot add meta channel, identity = 0x%X\n",
        __func__, stream->stream_info.identity);
      return rc;
    }
  }
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return 0;
}

/** isp_proc_streamon
 *    @isp : isp instance
 *    @isp_sink_port: sink port instance
 *    @session_id : session id to be started
 *    @stream_id : stream id of params
 *    @event : mct event object
 *    @session : session instance
 *
 *  Processing streamon command, create channel, register
 *  buffers, config pipeline and prepare hw for streaming
 *
 *  Returns 0 for success and negative error for failure
 **/
static int isp_proc_streamon(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, mct_event_t *event,
  isp_session_t *session)
{
  int i, rc = 0, isp_id;
  isp_stream_t *stream;
  int num_streams;
  uint32_t user_stream_ids[ISP_MAX_STREAMS];
  mct_stream_info_t *stream_info = NULL;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  if (NULL == session) {
    CDBG_ERROR("%s: Session could not be found! \n", __func__);
    return -1;
  }
  stream = isp_util_find_stream_in_session(session, stream_id);
  if (!stream) {
    CDBG_ERROR("%s: cannot find stream (streamid = %d)\n",
      __func__, stream_id);
    return -1;
  }

  stream_info = (mct_stream_info_t *)event->u.ctrl_event.control_event_data;
  CDBG_HIGH("%s: E, session_id = %d, stream_id = %d, stream_type = %d\n",
    __func__, session_id, stream_id, stream_info->stream_type);

  /* MCT does not pass the HAL buffer list pointer when link the stream.
   * Here we save the HAL buffer list pointer for buffer mapping purpose */
  stream->stream_info = *stream_info;
  isp_util_dump_stream_planes(stream);
  /*isp_util_update_hal_image_buf_to_channel(session, stream);*/

  /* get user streams by HAL bundling mask*/
  num_streams = isp_util_get_user_streams(session, stream_id, user_stream_ids);
  if (num_streams == 0) {
    ISP_DBG(ISP_MOD_COM,"%s: no user streams, num_stream = %d\n", __func__, num_streams);
    return 0;
  }

  /* we are streamon the first stream. Now we need to create the
   * link between hal streams and ISP hw streams.
   * For streams using ISP pipeline, all streams need to be
   * preconfigured so that we can create the channels now.*/
  if (!session->active_count) {
    for (isp_id = 0; isp_id < VFE_MAX; isp_id++) {
       session->sof_id[isp_id] = 0;
    }
    rc = isp_create_channels_for_streamon(isp, session);
    if (rc < 0) {
      CDBG_ERROR("%s: error, cannot create channels for session_id = %d\n",
        __func__, session->session_id);
    }
  } else {
    if (!isp_util_stream_use_pipeline(isp, stream)) {
      rc = isp_ch_util_sync_stream_cfg_to_channel(isp, session, stream);
      if (rc < 0) {
        CDBG_ERROR("%s: error, error add channel, "
          "identity = 0x%x, active_count = %d\n",
          __func__, session->session_id, session->active_count);
        return rc;
      }
    }
    isp_util_update_hal_image_buf_to_channel(session, stream);
  }

  /* update HFR param with new stream id */
  if (session->hfr_param.hfr_mode != CAM_HFR_MODE_OFF) {
    session->hfr_param.stream_id = stream_id;
  }

  /* select pipeline channels for hw stream*/
  if (session->use_pipeline && !session->active_count) {
    /* In case of stream restart with DIS enabled, initialize DIS stream id
       with correct value. */
    if(session->dis_param.dis_enable && (session->dis_param.stream_id == 0))
      session->dis_param.stream_id = user_stream_ids[0];

    rc = isp_util_select_pipeline_streams(isp, session);
    if (rc < 0) {
      CDBG_ERROR("%s: select_pipeline_stream error = %d\n",
        __func__, rc);
      return rc;
    }
  }
  /* check for yuv sensors to select nativbuf*/
  isp_ch_util_config_for_yuv_sensor(isp, session);

  /* compute stripe info for dual isp case*/
  rc = isp_util_compute_stripe_info(isp, session, stream);
  if (rc < 0) {
    CDBG_ERROR("%s: failed to compute stripe info for dual ISP mode. rc = %d\n",
      __func__, rc);
    return rc;
  }

  /* please note that,
   * isp_util_request_image_buf need to be before isp_util_config_for_streamon.
   * In isp_util_request_image_buf,
   * we do adjustment for meta data channel's buffer info for axi config. */

  /*request buffer: hal buffer or native buffer*/
  rc = isp_util_request_image_buf(isp, session, num_streams, user_stream_ids);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_util_request_image_buf error = %d \n", __func__, rc);
    return rc;
  }

  if (session->active_count == 0) {
    /* Before we call config for streamon, we make a cached copy of
       saved params. Now even if saved copy changes in between streamon
       of other VFE, it won't impact it */
    pthread_mutex_lock(
      &isp->data.session_critical_section[session->session_idx]);
    session->pending_update_params.hw_update_params = session->saved_params;
    pthread_mutex_unlock(
      &isp->data.session_critical_section[session->session_idx]);
  }

  rc = isp_util_config_for_streamon(isp, session);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_util_config_for_streamon error!"
      "sessid = %d, rc = %d\n", __func__, session->session_id, rc);
    return rc;
  }

  /* after config for streamon,
     change the channel state to be ISP_CHANNEL_STATE_HW_CFG*/
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    /* already configured hw */
    if (session->channel[i].state != ISP_CHANNEL_STATE_USER_CFG)
     continue;

    session->channel[i].state = ISP_CHANNEL_STATE_HW_CFG;
  }

  /* Save params before streamon
   * so that params can be applied to the first frame. */
  if (session->active_count == 0)
    rc = set_all_saved_params(isp, isp_sink_port, session_id, stream_id);
  if (rc < 0) {
    CDBG_ERROR("%s: set_all_saved_params error! sessid = %d, rc = %d\n",
       __func__, session_id, rc);
    return rc;
  }

  /*after deciding the isp output dimension, send it out*/
  isp_util_send_hw_stream_output_dim_downstream(isp,
    session, num_streams, user_stream_ids);

  /* Provide information to stats module on BF filter size */
  isp_util_set_stats_bf_filter_size(isp, session_id, stream_id);

  /*Isp Streamon: after all the hw stream config and channel are prepared*/
  rc = isp_util_streamon(isp, session,
    num_streams, user_stream_ids);
  if (rc < 0) {
    CDBG_ERROR("%s: streamon error! sessid = %d, rc = %d\n",
       __func__, session->session_id, rc);
    return rc;
  }

  /*initial config*/
  if (session->active_count == 0) {
    /*ZOOM*/
    isp_util_do_zoom_at_streamon(isp, session);
    isp_util_send_initial_zoom_crop_to_3a(isp, session->session_id,
      num_streams, user_stream_ids);
    isp_util_broadcast_pproc_zoom_crop (isp, session->session_id,
      num_streams, user_stream_ids, 0, NULL);

    /* we can query stats module here to get the
     * initial stats config for cold start */
    memset(&session->pending_update_params.hw_fetch_pending[0],
      0, sizeof(boolean) * VFE_MAX);
  }

  /* update user stream state */
  for (i = 0; i < num_streams; i++) {
    stream = isp_util_find_stream_in_session(session, user_stream_ids[i]);
    if (!stream) {
      CDBG_ERROR("%s: cannot find stream (streamid = %d)\n",
        __func__, user_stream_ids[i]);
      return -1;
    }

    /* if stream is already started - avoid increasing active count*/
    if(stream->state == ISP_STREAM_STATE_ACTIVE) {
      CDBG_ERROR("%s: received streamon more than once for stream %d",
        __func__, stream->stream_id);
      continue;
    }
    stream->state = ISP_STREAM_STATE_ACTIVE;
    pthread_mutex_lock(
      &isp->data.session_critical_section[session->session_idx]);
    session->active_count++;
    pthread_mutex_unlock(
      &isp->data.session_critical_section[session->session_idx]);
  }
  ISP_DBG(ISP_MOD_COM,"%s:X, rc = %d", __func__, rc);
  return rc;
}

/** isp_streamoff
 *    @isp : isp instance
 *    @isp_sink_port: sink port instance
 *    @session_id : session id to be started
 *    @stream_id : stream id of params
 *    @event : mct event object
 *
 *  Stops isp streaming post request to async thread.
 *
 *  Returns 0 for success and negative error for failure
 **/
int isp_streamoff(isp_t *isp, isp_port_t *isp_sink_port, uint32_t session_id,
  uint32_t stream_id, mct_event_t *event)
{
  int i, rc = 0;
  isp_session_t *session = NULL;
  isp_async_cmd_t *cmd;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);

  session = isp_util_find_session(isp, session_id);
  if (!session) {
    CDBG_ERROR("%s: error, NULL session for session_id = %d\n",
      __func__, session_id);
    return -1;
  }
  CDBG_HIGH("%s: E, session_id = %d, stream_id = %d, active_count = %d\n",
    __func__, session->session_id, stream_id, session->active_count);

  cmd = malloc(sizeof(isp_async_cmd_t));
  if (!cmd) {
    CDBG_ERROR("%s:error,  no memory for streamon command, session_id = %d\n",
      __func__, session_id);
    return -1;
  }
  memset(cmd, 0, sizeof(isp_async_cmd_t));
  cmd->cmd_id = ISP_ASYNC_COMMAND_STRAEMOFF;
  cmd->streamon.isp = (void *)isp;
  cmd->streamon.isp_sink_port = (void *)isp_sink_port;
  cmd->streamon.session_id = session_id;
  cmd->streamon.stream_id = stream_id;
  cmd->streamon.event = event;
  cmd->streamon.session = (void *)session;
  cmd->streamon.sync_cmd = TRUE;
  pthread_mutex_lock(&session->async_task.sync_mutex);
  rc = isp_enqueue_async_command(isp, session, &cmd);
  if (cmd)
    free(cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: error, isp_enqueue_async_command, session_id = %d\n",
      __func__, session_id);
    goto end;
  }
  sem_wait(&session->async_task.sync_sem);
  rc = session->async_task.sync_ret;
end:
  pthread_mutex_unlock(&session->async_task.sync_mutex);

  if (!session->active_count) {
    /* now we need to disable the in_service flag for buffered params */
    pthread_mutex_lock(
      &isp->data.session_critical_section[session->session_idx]);
    session->buffered_hw_params.in_service = FALSE;
    session->buffered_hw_params.hw_update_pending = FALSE;
    pthread_mutex_unlock(
      &isp->data.session_critical_section[session->session_idx]);
  }

  CDBG_HIGH("%s: X, session_id = %d, rc = %d\n",
    __func__, session->session_id, rc);
  return rc;
}

/** isp_proc_streamon
 *    @isp : isp instance
 *    @isp_sink_port: sink port instance
 *    @session_id : session id to be started
 *    @stream_id : stream id of params
 *    @event : mct event object
 *    @session : session instance
 *
 *  This method processes streamoff command, releases channel,
 *  release buffers, unconfig pipeline and stop hw streaming
 *
 *  Returns 0 for success and negative error for failure
 **/
static int isp_proc_streamoff(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, mct_event_t *event,
  isp_session_t *session)
{
  int rc = 0;
  int i, isp_id;
  isp_stream_t *stream = NULL;
  int num_streams;
  uint32_t stream_ids[ISP_MAX_STREAMS];
  boolean stop_immediately = TRUE;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  if (!session) {
    CDBG_ERROR("%s: session is not existing. sessionid = %d\n",
      __func__, session_id);
    return -1;
  }
  stream = isp_util_find_stream_in_session(session, stream_id);
  if (!stream) {
    CDBG_ERROR("%s: cannot find stream (streamid = %d)\n",
      __func__, stream_id);
    return -1;
  }
  CDBG_HIGH("%s: E,session_id = %d, stream_id= %d, stream_type= %d meta %d\n",
     __func__, session_id, stream_id, stream->stream_info.stream_type,
    stream->meta_info.is_valid);
  memset(&stream->meta_info, 0, sizeof(sensor_meta_t));

  if (session->hal_bundling_mask != 0 &&
      (session->hal_bundling_mask & (1 << stream->stream_idx))) {
    if (session->streamoff_bundling_mask == 0) {
      /* first bundle streamoff */
      session->streamoff_bundling_mask |= (1 << stream->stream_idx);
      rc = isp_util_get_stream_ids_by_mask(session, session->hal_bundling_mask,
             &num_streams, stream_ids);
    } else {
      session->streamoff_bundling_mask |= (1 << stream->stream_idx);
      goto end;
    }
  } else {
    stream_ids[0] = stream_id;
    num_streams = 1;
  }
  if (isp_get_number_of_active_sessions() > 1) {
    CDBG_HIGH("%s: There are other active sessions\n", __func__);
    stop_immediately = FALSE;
  }
  rc = isp_util_streamoff(isp, session, num_streams, stream_ids,
    stop_immediately);
  if (rc < 0) {
    CDBG_ERROR("%s: error, isp_util_streamon, sessid = %d, rc = %d\n",
      __func__, session->session_id, rc);
    return rc;
  }

  if (rc >= 0) {
    isp_stream_t *stream;

    for (i = 0; i < num_streams; i++) {
      stream = isp_util_find_stream_in_session(session, stream_ids[i]);
      if (stream == NULL) {
        CDBG_ERROR("%s: cannot find stream (session_id = %d, straem_id = %d",
          __func__, session->session_id, stream_ids[i]);
        continue;
      }
      /* if stream is already stopped or there are no active streams - no op */
      if((stream->state == ISP_STREAM_STATE_HW_CFG) ||
         (session->active_count <= 0)) {
        CDBG_ERROR("%s: received streamoff more than once for stream %d",
          __func__, stream->stream_id);
        continue;
      }
      stream->state = ISP_STREAM_STATE_HW_CFG;
      pthread_mutex_lock(
        &isp->data.session_critical_section[session->session_idx]);
      session->active_count--;

      pthread_mutex_unlock(
        &isp->data.session_critical_section[session->session_idx]);
    }
  } else {
    /* stop error. Cannot recover */
    /* TODO: reset VFE and send notification to media bus */
    CDBG_ERROR("%s: cannot stop streams. rc = %d\n", __func__, rc);
  }

  isp_util_release_image_buf(isp, session, num_streams, stream_ids);
end:
  if (session->hal_bundling_mask &&
      session->streamoff_bundling_mask == session->hal_bundling_mask) {
    session->hal_bundling_mask = 0;
    session->streamoff_bundling_mask = 0;
    session->streamon_bundling_mask = 0;
  }
  if (session->active_count == 0) {
    /* reset the sof frame id to zero.*/
    session->sof_frame_id = 0;
    isp_ch_util_all_streams_off(isp, session);
    /* after all streams are off. WE reset straem state to user_cfg. */
    for (i = 0; i < ISP_MAX_STREAMS; i++) {
      if (session->streams[i].stream_id > 0) {
        session->streams[i].state = ISP_STREAM_STATE_USER_CFG;
        /* for each stream we need to unconfig channels to hw */
        isp_util_unconfig_stream(isp, session, &session->streams[i]);
        /* when all stream are off and we removed the link between hal
         * streams to isp hw streams, we do not really need to keep the link.
         i.e., the life span of channel is ended. */
        isp_ch_util_del_channel_by_mask(session,
          session->streams[i].channel_idx_mask);
        /* reset channel idx mask to zero */
        session->streams[i].channel_idx_mask = 0;
      }
    }
    memset(&session->pending_update_params.hw_fetch_pending[0],
      0, sizeof(boolean) * VFE_MAX);
    /* reset use_pipeline to FALSE. It will be set to proper value in
     * isp_sink_port_stream_config()
     */
    session->use_pipeline = FALSE;
  }

  return rc;
}

/** isp_set_awb_trigger_update
 *    @isp : isp instance
 *    @isp_sink_port: sink port instance
 *    @session_id : session id to be started
 *    @stream_id : stream id of params
 *    @data : stats data to update
 *
 *  This method triggers new auto white balance configuration to
 *  hardware
 *
 *  Returns 0 for success and negative error for failure
 **/
int isp_set_awb_trigger_update(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *data)
{
  int rc = 0;
  int isp_id = 0;
  awb_update_t *p_awb_update;
  boolean use_stored_stats;

  ISP_DBG(ISP_MOD_COM,"%s:E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  stats_update_t *stats_data = (stats_update_t *)data;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  if (!session) {
    CDBG_ERROR("%s: session is not existing. sessionid = %d\n",
      __func__, session_id);
    return -1;
  }

  /* If one VFE is done hw update and other is not done yet and there is
     possibility that awb params will get updated due to this trigger update.
     This will result in split screen. To AVOID changing of params in between
     2 VFE hw updates of same frame, we just save params here and not call
     trigger update immediately. Actual trigger update will happen in SOF*/

  pthread_mutex_lock(
    &isp->data.session_critical_section[session->session_idx]);

  ISP_DBG(ISP_MOD_COM,"%s:%d] sessionid = %d bracket state %d",
    __func__, __LINE__, session_id, session->bracket_info.state);
  use_stored_stats =
    (session->bracket_info.state == MCT_BRACKET_CTRL_RESTORE_LOCK_3A);
  p_awb_update = (use_stored_stats) ?
    &session->saved_params.awb_stored_stats :
    &stats_data->awb_update;

  if ((stats_data->flag & STATS_UPDATE_AWB)) {
    session->saved_params.awb_stats_update = *p_awb_update;
    session->saved_params.stats_flag |= STATS_UPDATE_AWB;
    ISP_DBG(ISP_MOD_COM,"%s: session_id = %d, color_temp = %d gain %f %f %f",
      __func__, session->session_id, p_awb_update->color_temp,
      p_awb_update->gain.r_gain,
      p_awb_update->gain.g_gain,
      p_awb_update->gain.b_gain);
  }
  pthread_mutex_unlock(
    &isp->data.session_critical_section[session->session_idx]);

  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return rc;
}

/** isp_proc_async_command
 *    @isp : isp instance
 *    @session : session instance
 *    @cmd : stream id of params
 *
 *  This method is to process important commands in async thread
 *  context viz Streamon/off, set hw params, uv subsample to
 *  avoid conccurent execution of them.
 *
 *  Returns 0 for success and negative error for failure
 **/
int isp_proc_async_command(isp_t *isp, isp_session_t *session,
  isp_async_cmd_t *cmd)
{
  int rc = 0;
  int i;
  ISP_DBG(ISP_MOD_COM,"%s: E, session_id = %d, async_cmd_id = %d\n",
    __func__, session->session_id, cmd->cmd_id);

  switch (cmd->cmd_id) {
  case ISP_ASYNC_COMMAND_SET_HW_PARAM: {
    pthread_mutex_lock(
      &isp->data.session_critical_section[session->session_idx]);
    session->async_task.wait_hw_update_done = TRUE;
    pthread_mutex_unlock(
      &isp->data.session_critical_section[session->session_idx]);
    sem_wait(&session->async_task.hw_wait_sem);
    ISP_DBG(ISP_MOD_COM,"%s: set_hw_param woke up by SOF, session_id = %d\n",
        __func__, session->session_id);
    rc = isp_proc_set_hw_params(isp, session);
  }
    break;

  case ISP_ASYNC_COMMAND_UV_SUBSAMPLE: {
    pthread_mutex_lock(
      &isp->data.session_critical_section[session->session_idx]);
    /* if isp has not started drop the request */
    if (session->active_count == 0 ||
        session->uv_subsample_ctrl.switch_state == ISP_UV_SWITCH_STATE_IDLE) {
      session->uv_subsample_ctrl.switch_state = ISP_UV_SWITCH_STATE_IDLE;
      pthread_mutex_unlock(
        &isp->data.session_critical_section[session->session_idx]);
      break;
    }

    /* if any burst stream is active, drop the request */
    for (i = 0; i < ISP_MAX_STREAMS; i++) {
      if (session->streams[i].state == ISP_STREAM_STATE_ACTIVE &&
          session->streams[i].stream_info.streaming_mode ==
            CAM_STREAMING_MODE_BURST) {
        session->uv_subsample_ctrl.switch_state = ISP_UV_SWITCH_STATE_IDLE;
        pthread_mutex_unlock(
          &isp->data.session_critical_section[session->session_idx]);
        return 0;
      }
    }

    /* Removed set uv subsampling from here. Here we just update save param
     * copy. update flag is to indicate whether uv sampling enable/disable
     * needs to be done. In corner case, uv subsampling could start on 2nd VFE
     * but first VFE may not have started with uv sampling yet. This may create
     * split/green patch on half screen. Now saving the uv params and
     * triggering them at SOF will protect us. */
    session->saved_params.uv_subsample_update = TRUE;
    session->saved_params.uv_subsample_enable = cmd->uv_subsample_enable;

    /* we need to apply the chroma sub-sampling at frame boundary
     * so that wait for the hw update done here */
    session->async_task.wait_hw_update_done = TRUE;
    /* UV sub-sampling switching in progress. nop  */
    session->uv_subsample_ctrl.switch_state = ISP_UV_SWITCH_STATE_IN_PROGRESS;

    pthread_mutex_unlock(
      &isp->data.session_critical_section[session->session_idx]);

    /*wait for next SOF: means CDS trigger update(AXI/SDCALER)started then wake up here*/
    sem_wait(&session->async_task.hw_wait_sem);

    /*after CDS trigger update, wait for hw cds hw update*/
    pthread_mutex_lock(
      &isp->data.session_critical_section[session->session_idx]);
    session->async_task.wait_hw_update_done = TRUE;
    /* update the current enable now that it's successful */
    session->uv_subsample_ctrl.curr_enable = cmd->uv_subsample_enable;
    pthread_mutex_unlock(
      &isp->data.session_critical_section[session->session_idx]);
    sem_wait(&session->async_task.hw_wait_sem);

    /* chroma sub-sample is done */
    pthread_mutex_lock(
      &isp->data.session_critical_section[session->session_idx]);
    session->uv_subsample_ctrl.switch_state = ISP_UV_SWITCH_STATE_FINISHING;
    session->uv_subsample_ctrl.min_wait_cnt = 2;;
    pthread_mutex_unlock(
      &isp->data.session_critical_section[session->session_idx]);

    CDBG_HIGH("%s: after ISP_UV_SWITCH_STATE_FINISHING session = %d\n",
      __func__, session->session_id);
  }
    break;

  case ISP_ASYNC_COMMAND_STREAMON: {
    isp_session_cmd_streamon_t *streamon = &cmd->streamon;
    if (streamon->sync_cmd) {
      session->async_task.sync_ret = isp_proc_streamon((isp_t *)streamon->isp,
        (isp_port_t *)streamon->isp_sink_port, streamon->session_id,
        streamon->stream_id, streamon->event,
        (isp_session_t *)streamon->session);
      ISP_DBG(ISP_MOD_COM,"%s: streamon, unblock MCT thread, session_id = %d, ret = %d\n",
        __func__, session->session_id, session->async_task.sync_ret);
      sem_post(&session->async_task.sync_sem);
    } else {
      rc = isp_proc_streamon((isp_t *)streamon->isp,
        (isp_port_t *)streamon->isp_sink_port, streamon->session_id,
        streamon->stream_id, streamon->event,
        (isp_session_t *)streamon->session);
    }
  }
    break;

  case ISP_ASYNC_COMMAND_STRAEMOFF: {
    isp_session_cmd_streamoff_t *streamoff = &cmd->streamoff;
    if (streamoff->sync_cmd) {
      session->async_task.sync_ret = isp_proc_streamoff((isp_t *)streamoff->isp,
        (isp_port_t *)streamoff->isp_sink_port, streamoff->session_id,
        streamoff->stream_id, streamoff->event,
        (isp_session_t *)streamoff->session);

    /* if no more active stream no need for chroma sub-sampling  */
    pthread_mutex_lock(
      &isp->data.session_critical_section[session->session_idx]);
    if (session->active_count == 0) {
      /*For hystersis we keep track of subsampling is enable or not
      Hence removing session->uv_subsample_ctrl.curr_enable = 0;*/
      session->uv_subsample_ctrl.switch_state = ISP_UV_SWITCH_STATE_IDLE;
    }
    pthread_mutex_unlock(
      &isp->data.session_critical_section[session->session_idx]);
    ISP_DBG(ISP_MOD_COM,"%s: streamoff, unblock MCT thread, session_id = %d, ret = %d\n",
      __func__, session->session_id, session->async_task.sync_ret);
    sem_post(&session->async_task.sync_sem);
  } else {
    isp_proc_streamoff((isp_t *)streamoff->isp,
      (isp_port_t *)streamoff->isp_sink_port, streamoff->session_id,
      streamoff->stream_id, streamoff->event,
      (isp_session_t *)streamoff->session);
    }
  }
    break;

  case ISP_ASYNC_COMMAND_WM_BUS_OVERFLOW_RECOVERY: {
    isp_session_cmd_wm_bus_overflow_t *wm_recovery = &cmd->wm_recovery;
    isp_hw_t *isp_hw =  wm_recovery->isp_hw;
    isp_hw_session_t *hw_session = wm_recovery->session;
    rc = isp_hw->notify_ops->notify((void *)isp_hw->notify_ops->parent,
           isp_hw->init_params.dev_idx,ISP_HW_NOTIFY_ISPIF_TO_RESET,
           &hw_session->session_id, sizeof(uint32_t));
      if (rc < 0) {
        CDBG_ERROR("%s Error doing ISPIF RESET \n", __func__);
        return -1;
      }
  }
    break;

  default: {
    CDBG_ERROR("%s: not supported, session_id = %d, async_cmd_id = %d\n",
      __func__, session->session_id, cmd->cmd_id);
  }
    break;
  }

  ISP_DBG(ISP_MOD_COM,"%s: X, session_id = %d, async_cmd_id = %d\n",
    __func__, session->session_id, cmd->cmd_id);

  return rc;
}

/** isp_enqueue_async_command
*    @isp: isp ctrl instance
*    @stats_data: isp port object
*
*  TODO
*
*  Returns 0 for success and negative error for failure
**/
int isp_enqueue_async_command(isp_t *isp, isp_session_t *session,
  isp_async_cmd_t **cmd)
{
  uint32_t idx = session->session_idx;
  isp_async_cmd_t *holder = NULL;

  if (!cmd || !(*cmd)) {
    CDBG_ERROR("%s: error, null session comamnd received, session_id = %d\n",
      __func__, session->session_id);
    return -1;
  }
  holder = *cmd;
  *cmd = NULL;

  if (!session->use_pipeline && (holder->cmd_id ==
      ISP_ASYNC_COMMAND_UV_SUBSAMPLE ||
      holder->cmd_id == ISP_ASYNC_COMMAND_SET_HW_PARAM)) {
    /* if the session does not use pixel interface zoom param has no meaning.
     * so drop it here to make the code more defensive.
     * Suggest the app not to send pixel interface based params to
     * RDI session.
     */
    free(holder);
    CDBG_HIGH("%s:%d WARNING, cmd_id = %d, Incorrect RDI param received\n",
      __func__, __LINE__, holder->cmd_id);
    return 0;
  }

  pthread_mutex_lock(&session->async_task.task_q_mutex);
  mct_queue_push_tail(&session->async_task.task_q, (void *)holder);
  pthread_mutex_unlock(&session->async_task.task_q_mutex);
  sem_post(&session->async_task.task_q_sem);
  return 0;
}

/** isp_set_uv_subsample
*    @isp: isp ctrl instance
*    @stats_data: isp port object
*
*  TODO
*
*  Returns 0 for success and negative error for failure
**/
int isp_set_uv_subsample(isp_t *isp, uint32_t session_id,
  stats_update_t *stats_data)
{
  int rc = 0;
  int i = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  if (!session) {
    CDBG_ERROR("%s: session is not existing. sessionid = %d\n",
      __func__, session_id);
    return -1;
  }

  switch (session->uv_subsample_mode) {
  case ISP_UV_SUBSAMPLE_OFF:
    if (1 == session->uv_subsample_ctrl.curr_enable)
      rc = isp_util_send_uv_subsample_cmd(isp, session, 0);
    break;
  case ISP_UV_SUBSAMPLE_ON:
    if (0 == session->uv_subsample_ctrl.curr_enable)
      rc = isp_util_send_uv_subsample_cmd(isp, session, 1);
    break;
  case ISP_UV_SUBSAMPLE_AUTO:
    /* Do nothing if stats data is not available or aec is in fast mode */
    if (!stats_data || session->isp_fast_aec_mode)
      return 0;

    pthread_mutex_lock(
      &isp->data.session_critical_section[session->session_idx]);

    /* if no active stream reset UV subsampling */
    if (session->active_count == 0) {
      /*For hystersis we keep track of subsampling is enable or not
      Hence removing session->uv_subsample_ctrl.curr_enable = 0;*/
      session->uv_subsample_ctrl.switch_state = ISP_UV_SWITCH_STATE_IDLE;
      session->saved_params.uv_subsample_update = FALSE;
      pthread_mutex_unlock(
        &isp->data.session_critical_section[session->session_idx]);
      return 0;
    }
    if(session->uv_subsample_ctrl.switch_state != ISP_UV_SWITCH_STATE_IDLE) {
      /* UV sub-sampling switching in progress. nop  */
      pthread_mutex_unlock(
        &isp->data.session_critical_section[session->session_idx]);
      return 0;
    }

    pthread_mutex_unlock(
      &isp->data.session_critical_section[session->session_idx]);

    /*Read 2 UVsubsample trigger points from Chromatix header say A & B
      Lux values: Min-----A-----B-----Max
      UV subsampling is tunred ON if lux idx goes beyond value B
      But UV subsampling will be tuned OFF if lux idx comes
      below value A and NOT immediately if lux idx is below value B.
      */
    if(!session->uv_subsample_ctrl.trigger_B ||
       !session->uv_subsample_ctrl.trigger_A)
      return 0;

      if (!session->uv_subsample_ctrl.curr_enable) {
      if (stats_data->aec_update.lux_idx > session->uv_subsample_ctrl.trigger_B) {
        /*send async cmd for doing UV sampling*/
        rc = isp_util_send_uv_subsample_cmd(isp, session, 1);
        if (rc < 0) {
          CDBG_ERROR("%s: failed to send cmd for uv_subsampling ON rc = %d",
            __func__, rc);
        }
      }
    } else {
      if (stats_data->aec_update.lux_idx < session->uv_subsample_ctrl.trigger_A) {
        /*send async cmd for to turn OFF UV sampling*/
        rc = isp_util_send_uv_subsample_cmd(isp, session, 0);
        if (rc < 0) {
          CDBG_ERROR("%s: failed to send cmd for uv_subsampling OFF rc = %d",
            __func__, rc);
        }
      }
    }
    break;
  default:
    CDBG_ERROR("%s:%d] Invalid subsample mode",__func__, __LINE__);
    rc = -1;
    break;
  }

  return rc;
}

/** isp_send_rolloff_to_sensor
*    @isp: isp ctrl instance
*    @mct_port: isp port object
*
*  TODO
*
*  Returns 0 for success and negative error for failure
**/
int isp_send_rolloff_to_sensor(isp_t *isp, uint32_t session_id,
  uint32_t stream_id, mct_port_t *mct_port)
{
  int rc;
  isp_session_t *session;
  isp_stream_t *stream;
  mct_event_t mct_event;
  mct_event_stats_isp_rolloff_t rolloff_table;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
       session_id, stream_id);
  session = isp_util_find_session(isp, session_id);
  if (!session) {
    CDBG_ERROR("%s: error: Cannot find session:%d\n",
      __func__, session_id);
    return -1;
  }

  /* if VHDR is off do not send rolloff*/
  if(!session->saved_params.vhdr_enable)
    return 0;

  rc = isp_util_get_rolloff_table(isp, session_id, stream_id, &rolloff_table);
  if (rc < 0) {
    CDBG_ERROR("%s: error: Cannot get rolloff table!\n",
      __func__);
    return rc;
  }

  stream = isp_util_find_stream_in_session(session, stream_id);
  if (!stream) {
    CDBG_ERROR("%s: error: Cannot find stream:%d\n",
      __func__, stream_id);
    return -1;
  }

  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.u.module_event.type = MCT_EVENT_MODULE_SENSOR_ROLLOFF;
  mct_event.u.module_event.module_event_data = (void *)&rolloff_table;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.identity = pack_identity(session_id, stream_id);
  mct_event.direction = MCT_EVENT_UPSTREAM;
  /* broadcast sof upstream */
  mct_port_send_event_to_peer(stream->sink_port, &mct_event);
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return rc;
}

/** isp_save_aec_param
*    @isp: isp ctrl instance
*    @isp_sink_port: isp port object
*    @data:
*
*  TODO
*
*  Returns 0 for success and negative error for failure
**/
int isp_save_aec_param(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *data)
{
  int rc = 0;
  int isp_id = 0;
  q3q_flash_sensitivity_t *flash_sensitivity = NULL;
  isp_flash_params_t      *flash_params = NULL;

  aec_update_t *p_aec_update;
  boolean use_stored_stats;
  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  stats_update_t *stats_data = (stats_update_t *)data;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  if (!session) {
    CDBG_ERROR("%s: session is not existing. sessionid = %d\n",
      __func__, session_id);
    return -1;
  }
  /* If one VFE is done hw update and other is not done yet and there
     is possibility that aec params will get updated due to this
     trigger update. This will result in split screen. To AVOID changing
     of params in between 2 VFE hw updates of same frame, we just save
     params here and not call trigger update immediately.
     Actual trigger update will happen in SOF */
  pthread_mutex_lock(
    &isp->data.session_critical_section[session->session_idx]);

  ISP_DBG(ISP_MOD_COM,"%s:%d] sessionid = %d bracket state %d",
    __func__, __LINE__, session_id, session->bracket_info.state);
  use_stored_stats =
    (session->bracket_info.state == MCT_BRACKET_CTRL_RESTORE_LOCK_3A);
  p_aec_update = (use_stored_stats) ?
    &session->saved_params.aec_stored_stats :
    &stats_data->aec_update;

  if ((stats_data->flag & STATS_UPDATE_AEC)) {
    session->saved_params.stats_flag |= STATS_UPDATE_AEC;
    if ((stats_data->aec_update.est_state == AEC_EST_START) ||
      (stats_data->aec_update.est_state == AEC_EST_DONE) ||
      (stats_data->aec_update.est_state == AEC_EST_DONE_FOR_AF) ||
      (stats_data->aec_update.est_state == AEC_EST_DONE_SKIP)) {
      flash_params = &session->saved_params.flash_params;
      flash_params->flash_type = CAMERA_FLASH_LED;
      flash_sensitivity =
        (q3q_flash_sensitivity_t *)stats_data->aec_update.flash_sensitivity;
      if (flash_sensitivity) {
        flash_params->sensitivity_led_off = flash_sensitivity->off;
        flash_params->sensitivity_led_low = flash_sensitivity->low;
        flash_params->sensitivity_led_hi = flash_sensitivity->high;
      }
    }
    session->saved_params.aec_stats_update = *p_aec_update;
    ISP_DBG(ISP_MOD_COM,"%s:%d] gain %f lux %f",
      __func__, __LINE__,
      p_aec_update->lux_idx,
      p_aec_update->real_gain);
  }
  pthread_mutex_unlock(
    &isp->data.session_critical_section[session->session_idx]);
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return rc;
}

/** isp_save_asd_param
*    @isp: isp ctrl instance
*    @isp_sink_port: isp port object
*    @data:
*
*  TODO
*
*  Returns 0 for success and negative error for failure
**/
int isp_save_asd_param(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *data)
{
  int rc = 0;
  int isp_id = 0;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  stats_update_t *stats_data = (stats_update_t *)data;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  if (!session) {
    CDBG_ERROR("%s: session is not existing. sessionid = %d\n",
      __func__, session_id);
    return -1;
  }
  rc = isp_ch_util_set_param(isp, session, stream_id,
    ISP_HW_SET_PARAM_SAVE_ASD_PARAMS, (void *)stats_data,
    sizeof(stats_update_t));
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return rc;
}

/** isp_get_la_gamma_tbl
*    @isp: isp ctrl instance
*    @isp_sink_port: isp port object
*    @data:
*
*  TODO
*
*  Returns 0 for success and negative error for failure
**/
int isp_get_la_gamma_tbl(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *data)
{
  int rc = 0;
  int isp_id = 0;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  mct_isp_table_t *isp_tbls = (mct_isp_table_t *)data;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  if (!session) {
    CDBG_ERROR("%s: session is not existing. sessionid = %d\n",
      __func__, session_id);
    return -1;
  }

  rc = isp_ch_util_get_param(isp, session, stream_id,
    ISP_HW_GET_PARAM_LA_GAMMA_TBLS, NULL, 0, (void *)isp_tbls,
    sizeof(mct_isp_table_t));
  return rc;

}

/** isp_set_gain_from_sensor_req
*    @isp: isp ctrl instance
*    @isp_sink_port: isp port object
*    @session id: session id
*    @stream id: stream id
*    @data:
*
*  Updates gain based on sensor request
*
*/
int isp_set_gain_from_sensor_req(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *data)
{
  stats_get_data_t *stats_get = (stats_get_data_t*)data;
  isp_session_t *session = isp_util_find_session(isp, session_id);

  if (!session) {
    CDBG_ERROR("%s: session is not existing. sessionid = %d\n",
      __func__, session_id);
    return -1;
  }

  pthread_mutex_lock(
    &isp->data.session_critical_section[session->session_idx]);

  if (stats_get->flag & STATS_UPDATE_AEC) {
    session->saved_params.stats_flag |= STATS_UPDATE_AEC;
    session->saved_params.aec_stats_update.real_gain =
      stats_get->aec_get.real_gain[0];
    session->saved_params.aec_stats_update.linecount =
      stats_get->aec_get.linecount[0];
  }

  pthread_mutex_unlock(
    &isp->data.session_critical_section[session->session_idx]);

  return 1;
}


/** isp_set_aec_trigger_update
*    @isp: isp ctrl instance
*    @isp_sink_port: isp port object
*    @data:
*
*  TODO
*
*  Returns 0 for success and negative error for failure
**/
int isp_set_aec_trigger_update(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *data)
{
  int rc = 0;
  int isp_id = 0;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  float *dig_gain = (float *)data;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  if (!session) {
    CDBG_ERROR("%s: session is not existing. sessionid = %d\n",
      __func__, session_id);
    return -1;
  }
  /* If one VFE is done hw update and other is not done yet and there is
     possibility that aec params will get updated due to this trigger update.
     This will result in split screen. To AVOID changing of params in between
     2 VFE hw updates of same frame, we just save params here and not
     call trigger update immediately.
     Actual trigger update will happen in SOF */

   pthread_mutex_lock(
      &isp->data.session_critical_section[session->session_idx]);

   if(session->saved_params.dig_gain > 32.0){
     CDBG_ERROR("%s : Digital Gain invalid! %f", __func__,
       session->saved_params.dig_gain);
   } else {
     session->saved_params.dig_gain = *dig_gain;
   }
   pthread_mutex_unlock(
      &isp->data.session_critical_section[session->session_idx]);

  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return rc;
}

/** isp_set_stats_config_update
*    @isp: isp ctrl instance
*    @isp_sink_port: isp port object
*    @data:
*
*  TODO
*
*  Returns 0 for success and negative error for failure
**/
int isp_set_stats_config_update(isp_t *isp,
  isp_port_t *isp_sink_port, uint32_t session_id,
  uint32_t stream_id, void *data)
{
  int rc = 0;
  int isp_id = 0;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  stats_config_t *stats_data = (stats_config_t *)data;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  if (!session) {
    CDBG_ERROR("%s: session is not existing. sessionid = %d\n",
      __func__, session_id);
    return -1;
  }

  isp_util_dump_stats_config(stats_data);
  rc = isp_ch_util_set_param(isp, session, stream_id,
         ISP_HW_SET_PARAM_STATS_CFG_UPDATE, (void *)stats_data,
         sizeof(stats_config_t));
  return rc;
}

/** isp_set_sensor_lens_position_trigger_update
*    @isp: isp ctrl instance
*    @isp_sink_port: isp port object
*    @data:
*
*  TODO
*
*  Returns 0 for success and negative error for failure
**/
int isp_set_sensor_lens_position_trigger_update(isp_t *isp,
  isp_port_t *isp_sink_port, uint32_t session_id, uint32_t stream_id,
  void *data)
{
  int rc = 0;
  int isp_id = 0;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  isp_session_t *session = isp_util_find_session(isp, session_id);
  if (session == NULL) {
    CDBG_ERROR("%s: cannot find session %d\n",
      __func__, session_id);
    return -1;
  }

  rc = isp_ch_util_set_param(isp, session, stream_id,
         ISP_HW_SET_PARAM_SENSOR_LENS_POSITION_TRIGGER_UPDATE, data,
         sizeof(lens_position_update_isp_t));
  return rc;
}

int isp_set_flash_mode(
  isp_t *isp,
  isp_port_t *isp_sink_port,
  uint32_t session_id,
  uint32_t stream_id,
  void *data)
{
  int rc = 0;
  int isp_id = 0;
  cam_flash_mode_t *flash_mode = (cam_flash_mode_t *)data;

  isp_session_t *session = isp_util_find_session(isp, session_id);
  if (session == NULL) {
    CDBG_ERROR("%s: cannot find session %d\n",
               __func__, session_id);
    return -1;
  }

  rc = isp_ch_util_set_param(isp, session, stream_id,
         ISP_HW_SET_FLASH_MODE, data, sizeof(cam_flash_mode_t));

  session->saved_params.flash_mode = *flash_mode;
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return rc;
}

/** isp_set_af_rolloff_params
*    @isp: isp ctrl instance
*    @isp_sink_port: isp port object
*    @data:
*
*  TODO
*
*  Returns 0 for success and negative error for failure
**/
int isp_set_af_rolloff_params(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *data)
{
  int rc = 0;
  int isp_id = 0;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  isp_session_t *session = isp_util_find_session(isp, session_id);
  if (session == NULL) {
    CDBG_ERROR("%s: cannot find session %d\n",
      __func__, session_id);
    return -1;
  }

  ISP_DBG(ISP_MOD_COM,"%s: Received Rolloff Macro tables\n", __func__);
  rc = isp_ch_util_set_param(isp, session, stream_id,
    ISP_HW_SET_PARAM_AF_ROLLOFF_PARAMS, data, sizeof(af_rolloff_info_t));
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return rc;
}

/** isp_set_fast_aec_mode
*    @isp: isp ctrl instance
*    @session_id: session
*    @stream_id: stream
*    @data: fast aec on/off
*
*  This method is to set/reset fast aec.
*
*  Returns 0 for success and negative error for failure
**/
int isp_set_fast_aec_mode(isp_t *isp, uint32_t session_id, uint32_t stream_id,
  void *data)
{
  int rc = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  mct_fast_aec_mode_t *fast_aec_mode = data;
  if (session == NULL) {
    CDBG_ERROR("%s: cannot find session %d\n",
               __func__, session_id);
    return -1;
  }

  session->isp_fast_aec_mode = fast_aec_mode->enable;
  if (fast_aec_mode->enable)
    session->hal_bundling_mask = 0;
  return rc;
}

/** isp_set_chromatix
*    @isp: isp ctrl instance
*    @isp_sink_port: isp port object
*    @chromatix: pointing to chromatix header data
*
*  This method updates the chromatix pointer in session and
*  passes it same down to all modules.
*
*  Returns 0 for success and negative error for failure
**/
int isp_set_chromatix(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *chromatix)
{
  int rc = 0;
  modulesChromatix_t *chromatix_param = chromatix;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  isp_session_t *session = isp_util_find_session(isp, session_id);
  if (session == NULL) {
    CDBG_ERROR("%s: cannot find session %d\n", __func__, session_id);
    rc = -1;
    goto end;
  }
  memset(&session->chromatix, 0, sizeof(session->chromatix));
  session->chromatix.chromatixPtr = chromatix_param->chromatixPtr;
  session->chromatix.chromatixComPtr = chromatix_param->chromatixComPtr;
  if (session->active_count > 0) {
    rc = isp_ch_util_set_param(isp, session, stream_id,
      ISP_HW_SET_PARAM_CHROMATIX, (void *)&session->chromatix,
      sizeof(session->chromatix));
      if (rc < 0) {
      CDBG_ERROR("%s: ERROR in setting chromatix ptr", __func__);
      goto end;
    }
  }
  rc = isp_ch_util_get_param(isp, session, stream_id,
         ISP_HW_GET_PARAM_CDS_TRIGER_VAL, &session->chromatix,
         sizeof(modulesChromatix_t), (void *)&session->uv_subsample_ctrl,
         sizeof(session->uv_subsample_ctrl));
  if (rc < 0) {
    CDBG_ERROR("%s: ERROR in getting trigger points", __func__);
    goto end;
  }

  if (isp->data.tintless->tintless_data.is_supported &&
      isp->data.tintless->tintless_data.is_enabled) {
    /* config tintless with chromatix */
    rc = isp_tintless_chroma_config(session->tintless_session,
           session->chromatix.chromatixPtr);
    if (rc < 0) {
      CDBG_ERROR("%s: tintless chroma config rc: %d\n", __func__, rc);
    }
  }

end:
  ISP_DBG(ISP_MOD_COM,"%s: X, rc = %d", __func__, rc);
  return rc;
}

/** isp_set_reload_chromatix
*    @isp: isp ctrl instance
*    @isp_sink_port: isp port object
*    @chromatix: pointing to chromatix header data
*
*  This method updates the chromatix pointer in session and
*  passes it same down to all modules.
*
*  Returns 0 for success and negative error for failure
**/
int isp_set_reload_chromatix(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *chromatix)
{
  int rc = 0;
  modulesChromatix_t *chromatix_param = chromatix;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  isp_session_t *session = isp_util_find_session(isp, session_id);
  if (session == NULL) {
    CDBG_ERROR("%s: cannot find session %d\n", __func__, session_id);
    rc = -1;
    goto end;
  }
  session->chromatix.chromatixPtr = chromatix_param->chromatixPtr;
  session->chromatix.chromatixComPtr = chromatix_param->chromatixComPtr;
  if (session->active_count > 0) {
    rc = isp_ch_util_set_param(isp, session, stream_id,
      ISP_HW_SET_PARAM_RELOAD_CHROMATIX, (void *)&session->chromatix,
      sizeof(session->chromatix));
      if (rc < 0) {
      CDBG_ERROR("%s: ERROR in setting chromatix ptr", __func__);
      goto end;
    }
  }
  rc = isp_ch_util_get_param(isp, session, stream_id,
         ISP_HW_GET_PARAM_CDS_TRIGER_VAL, NULL, 0,
         (void *)&session->uv_subsample_ctrl,
         sizeof(session->uv_subsample_ctrl));
  if (rc < 0) {
    CDBG_ERROR("%s: ERROR in getting trigger points", __func__);
    goto end;
  }
end:
  ISP_DBG(ISP_MOD_COM,"%s: X, rc = %d", __func__, rc);
  return rc;
}

/** isp_buf_divert_ack
*    @isp: isp ctrl instance
*    @isp_src_port: isp port object
*    @divert_ack:
*
*  TODO
*
*  Returns 0 for success and negative error for failure
**/
int isp_buf_divert_ack(isp_t *isp, isp_port_t *isp_src_port,
  uint32_t session_id, uint32_t stream_id, isp_buf_divert_ack_t *divert_ack)
{
  int i, rc = 0;
  isp_stream_t *stream;
  isp_session_t *session;
  isp_hw_buf_divert_ack_t ack;
  struct msm_isp_event_data buf_event;
  isp_hw_t *isp_hw;
  uint32_t isp_id;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  session = isp_util_find_session(isp, session_id);
  if (session == NULL) {
    CDBG_ERROR("%s: cannot find session %d\n",
      __func__, session_id);
    return -1;
  }
  if (session->vfe_ids & (1 << VFE0))
    isp_id = VFE0;
  else if (session->vfe_ids & (1 << VFE1))
    isp_id = VFE1;
  else {
    CDBG_ERROR("%s: no ISP is created yet\n", __func__);
    return -1;
  }
  isp_hw = (isp_hw_t *)isp->data.hw[isp_id].hw_ops->ctrl;
  if (divert_ack->channel_id > 0) {
    if (divert_ack->is_skip_pproc == FALSE) {
      /* direct do the enqueue buf */
      CDBG("%s: LPM disable\n", __func__);
      rc = isp_ch_util_divert_ack(isp, session, divert_ack);
    } else {
      /* do a buf done since CPP didnt process the buf */
      CDBG("%s: LPM enable\n", __func__);
      memset(&buf_event, 0, sizeof(buf_event));
      buf_event.input_intf = divert_ack->input_intf;
      buf_event.frame_id = divert_ack->frame_id;
      buf_event.timestamp = divert_ack->timestamp;
      buf_event.u.buf_done.session_id = session_id;
      buf_event.u.buf_done.stream_id = stream_id;
      buf_event.u.buf_done.output_format =
         divert_ack->output_format;
      buf_event.u.buf_done.buf_idx =
         divert_ack->buf_idx;
      buf_event.u.buf_done.handle =
         divert_ack->handle;

      rc = ioctl(isp_hw->fd, VIDIOC_MSM_ISP_BUF_DONE, &buf_event);
      if (rc < 0) {
        CDBG_ERROR("%s: VIDIOC_MSM_ISP_BUF_DONE ERROR = %d\n", __func__, rc);
        return rc;
      }
    }
    return rc;
  }
  stream = isp_util_find_stream_in_session(session, stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: cannot find stream %d\n",
      __func__, stream_id);
    return -1;
  }

  /* need to del this legacy code after PP making the channel_id change */
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (stream->channel_idx_mask & (1 << i)) {
      divert_ack->channel_id = session->channel[i].channel_id;
      rc = isp_ch_util_divert_ack(isp, session, divert_ack);
      ISP_DBG(ISP_MOD_COM,"%s: X, rc %d", __func__, rc);
      return rc;
    }
  }
  return -1;
}

/** isp_set_divert_to_3a
*    @isp: isp ctrl instance
*    @isp_src_port: isp port object
*
*  TODO
*
*  Returns 0 for success and negative error for failure
**/
int isp_set_divert_to_3a(isp_t *isp, isp_port_t *isp_src_port,
  uint32_t session_id, uint32_t stream_id)
{
  int rc = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream = NULL;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  if (session == NULL) {
    CDBG_ERROR("%s: cannot find session %d\n",
      __func__, session_id);
    return -1;
  }

  stream = isp_util_find_stream_in_session(session, stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: cannot find stream %d\n",
      __func__, stream_id);
    return -1;
  }
  stream->divert_to_3a = TRUE;
  ISP_DBG(ISP_MOD_COM,"%s: X, rc %d", __func__, rc);
  return rc;
}

/** isp_reserve_sink_port
*    @isp: isp ctrl instance
*    @isp_port: isp port object
*    @peer_port: peer port object
*
*  TODO
*
*  Returns 0 for success and negative error for failure
**/
int isp_reserve_sink_port(isp_t *isp, isp_port_t *reserved_sink_port,
  ispif_src_port_caps_t *ispif_src_cap, mct_stream_info_t *stream_info,
  unsigned int session_id, unsigned int stream_id)
{
  int rc = 0;
  int is_new_sink = 0;
  isp_stream_t *stream = NULL;
  isp_port_t *isp_sink_port;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  isp_sink_port = isp_util_find_sink_port(isp, ispif_src_cap);
  if (isp_sink_port != NULL && isp_sink_port != reserved_sink_port) {
    /* has another port matching the sensor cap. EAGAIN */
    CDBG_ERROR("%s#%d: XError has another port matching the sensor cap\n",
      __func__, __LINE__);
    return -EAGAIN;
  }

  if (isp_sink_port == NULL) {
    if (reserved_sink_port->state == ISP_PORT_STATE_CREATED) {
      isp_sink_port = reserved_sink_port;
      isp_sink_port->state = ISP_PORT_STATE_RESERVED;
      is_new_sink = 1;
    } else {
      CDBG_ERROR("%s#%d: X Error isp_sink_port is NULL\n", __func__, __LINE__);
      return -1;
    }
  }

  /* add stream */
  stream = isp_util_add_stream(isp, session_id, stream_id, stream_info);
  if (!stream) {
    CDBG_ERROR("%s: cannot find stream (sessid = %d, streamid = %d)\n",
      __func__, session_id, stream_id);
    /* no stream available */
    if (is_new_sink) {
      isp_sink_port->state = ISP_PORT_STATE_CREATED;
    }
    return -1;
  }

  if (is_new_sink) {
    /* save the cap */
    isp_sink_port->u.sink_port.caps = *ispif_src_cap;
    isp_sink_port->session_id = session_id;
  }

  rc = isp_util_add_stream_to_sink_port(isp, isp_sink_port, stream);
  if (rc < 0)
    goto error;

  /* here we do not reserve src port. It will be done when reserve src port */
  ISP_DBG(ISP_MOD_COM,"%s: X, rc %d\n", __func__, rc);
  return rc;
error:
  isp_unreserve_sink_port(isp, isp_sink_port, session_id, stream_id);
  CDBG_ERROR("%s: X, error rc %d\n", __func__, rc);
  return rc;
}

/** isp_unreserve_sink_port
*    @isp: isp ctrl instance
*    @isp_port: isp port object
*    @peer_port: peer port object
*
*  TODO
*
*  Returns 0 for success and negative error for failure
**/
int isp_unreserve_sink_port(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id)
{
  int i, rc = 0;
  isp_stream_t *stream = NULL;

  stream = isp_util_find_stream(isp, session_id, stream_id);
  if (!stream) {
    CDBG_ERROR("%s: cannot find stream (sessid = %d, streamid = %d)\n",
      __func__, session_id, stream_id);
    return -1;
  }

  isp_util_del_stream_from_sink_port(isp, isp_sink_port, stream);
  if (isp_sink_port->u.sink_port.num_streams == 0) {
    /* no more stream associated with it. release the port */
    isp_sink_port->state = ISP_PORT_STATE_CREATED;
    isp_sink_port->session_id = 0;
  }

  if (stream->link_cnt == 0) {
    ISP_DBG(ISP_MOD_COM,"%s: delete the stream (sessid = %d, streamid = %d)\n",
      __func__, stream->session_id, stream->stream_id);
    isp_util_del_stream(isp, stream);
  }
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return rc;
}

/** isp_reserve_src_port
*    @isp: isp ctrl instance
*    @isp_port: isp port object
*    @peer_port: peer port object
*
*  TODO
*
*  Returns 0 for success and negative error for failure
**/
int isp_reserve_src_port(isp_t *isp, isp_port_t *reserved_isp_src_port,
  mct_stream_info_t *stream_info, unsigned int session_id,
  unsigned int stream_id)
{
  int i, rc = 0;
  int is_new_sink = 0;
  isp_session_t *session = NULL;
  isp_stream_t *stream = NULL;
  isp_port_t *isp_src_port = NULL;
  isp_port_t *isp_sink_port = NULL;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  session = isp_util_find_session(isp, session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session %d\n", __func__, session_id);
    return -1;
  }

  stream = isp_util_find_stream(isp, session_id, stream_id);
  if (!stream) {
    CDBG_ERROR("%s: cannot find stream (sessid = %d, streamid = %d)\n",
      __func__, session_id, stream_id);
    return -1;
  }
  isp_sink_port = (isp_port_t *)stream->sink_port->port_private;

  /* ISP has two types of src ports. */
  isp_src_port = isp_util_get_matched_src_port(isp,
    reserved_isp_src_port->port->caps.port_caps_type, isp_sink_port, stream);

  if (isp_src_port != NULL && isp_src_port != reserved_isp_src_port) {
    /* the given src port cannot be used to reserve */
    CDBG_ERROR("%s: Error given src port cannot be used to reserve", __func__);
    return -EAGAIN;
  }

  if (isp_src_port == NULL) {
    if (reserved_isp_src_port->state == ISP_PORT_STATE_CREATED) {
      isp_src_port = reserved_isp_src_port;
      isp_src_port->u.src_port.caps = isp_sink_port->u.sink_port.caps;
      isp_src_port->state = ISP_PORT_STATE_RESERVED;
      isp_src_port->session_id = session_id;
      isp_src_port->u.src_port.streaming_mode =
        stream->stream_info.streaming_mode;
    } else {
      CDBG_ERROR("%s: Error isp_src_port\n", __func__);
      return -1;
    }
  }

  rc = isp_util_add_stream_to_src_port(isp, isp_src_port, stream);
  if (rc < 0) {
    CDBG_ERROR("%s: Error cannot add stream to src port\n", __func__);
    return rc;
  }

  if (reserved_isp_src_port->port->caps.port_caps_type == MCT_PORT_CAPS_FRAME) {
    stream->src_ports[ISP_SRC_PORT_DATA] = isp_src_port->port;
    isp_src_port->u.src_port.port_type = ISP_SRC_PORT_DATA;
    if (isp_src_port->u.src_port.num_streams == 1) {
      /* first time to add a stream to the src port */
      session->num_src_data_port++;
    }
  } else if (reserved_isp_src_port->port->caps.port_caps_type ==
             MCT_PORT_CAPS_STATS){
    stream->src_ports[ISP_SRC_PORT_3A] = isp_src_port->port;
    isp_src_port->u.src_port.port_type = ISP_SRC_PORT_3A;
  } else {
    CDBG_ERROR("%s: error, not supported port cap = %d\n",
      __func__, reserved_isp_src_port->port->caps.port_caps_type);
    rc = -1;
  }

  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return rc;
}

/** isp_unreserve_src_port
*    @isp: isp ctrl instance
*    @isp_port: isp port object
*    @peer_port: peer port object
*
*  TODO
*
*  Returns 0 for success and negative error for failure
**/
int isp_unreserve_src_port(isp_t *isp, isp_port_t *isp_port,
  unsigned int session_id, unsigned int stream_id)
{
  int rc = 0;
  isp_stream_t *stream = NULL;
  isp_port_t *isp_src_port = isp_port;
  isp_session_t *session;

  session = isp_util_find_session(isp, session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session %d\n", __func__, session_id);
    return -1;
  }
  stream = isp_util_find_stream_in_session(session, stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: stream not found, session_id = %d, stream_id = %d.\n",
      __func__, session_id, stream_id);
    return -1;
  }

  isp_util_del_stream_from_src_port(isp, isp_src_port, stream);
  if (isp_src_port->u.src_port.num_streams == 0) {
    if (isp_src_port->u.src_port.port_type == ISP_SRC_PORT_DATA &&
        session->num_src_data_port > 0) {
      session->num_src_data_port--;
    }
    /* no more stream associated with it. release the port */
    isp_src_port->state = ISP_PORT_STATE_CREATED;
    isp_src_port->session_id = 0;
    isp_src_port->port->peer = NULL;
    isp_src_port->u.src_port.streaming_mode = 0;
  }

  if (stream->link_cnt == 0) {
    ISP_DBG(ISP_MOD_COM,"%s: delete the stream (sessid = %d, streamid = %d)\n",
      __func__, stream->session_id, stream->stream_id);
    isp_util_del_stream(isp, stream);
  }
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return rc;
}

/** isp_unlink_sink_port
*    @isp: isp ctrl instance
*    @isp_sink_port: isp port object
*    @peer_port: peer port object
*
*  Unlinks sink port stream config
*
*  Returns 0 for success and negative error for failure
**/
int isp_unlink_sink_port(isp_t *isp, isp_port_t *isp_sink_port,
  mct_port_t *peer_port, uint32_t session_id, uint32_t stream_id)
{
  int rc = 0;
  isp_stream_t *stream = NULL;
  isp_session_t *session = NULL;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  session = isp_util_find_session(isp, session_id);
  if (session && session->active_count > 0) {
    /* we here when an ad hoc straem in unlinked while
     * other streams are still streaming. */
    CDBG_HIGH("%s: ad hoc sink port unlinked, session_id = %d",
      __func__, session_id);
    isp_util_unconfig_stream_by_sink_port(isp, session, isp_sink_port);
  }

  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return rc;
}

/** isp_link_sink_port
*    @isp: isp ctrl instance
*    @isp_sink_port: current senssion object
*    @peer_port: isp stream object
*
*  Store peer port information in isp port object
*
*  Returns 0 for success and negative error for failure
**/
int isp_link_sink_port(isp_t *isp, isp_port_t *isp_port, mct_port_t *peer_port,
  uint32_t session_id, uint32_t stream_id)
{
  int i, rc = 0;
  isp_stream_t *stream = NULL;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  stream = isp_util_find_stream(isp, session_id, stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s: cannot find stream, sessioN_id = %d, stream_id = %d\n",
      __func__, session_id, stream_id);
    return -1;
  }
  if (isp_util_is_stream_in_sink_port(isp, isp_port, stream) == FALSE) {
    /* error. stream is not in sink port. */
    CDBG_ERROR("%s: stream (session = %d, stream = %d) not in sink port\n",
      __func__, session_id, stream_id);
    return -1;
  }

  /* stream is linked with local sink. link is done */
  isp_port->port->peer = peer_port;
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return 0;
}

/** isp_link_src_port
*    @isp: isp ctrl instance
*    @isp_src_port: current senssion object
*    @peer_port: isp stream object
*
*  Store peer port information in isp port object
*
*  Returns 0 for success and negative error for failure
**/
int isp_link_src_port(isp_t *isp, isp_port_t *isp_port, mct_port_t *peer_port,
  uint32_t session_id, uint32_t stream_id)
{
  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  /* only need to save peer's port */
  isp_port->port->peer = peer_port;
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return 0;
}

/** isp_unlink_src_port
*    @isp: isp ctrl instance
*    @isp_src_port: current senssion object
*    @peer_port: isp stream object
*
*  TODO
*
*  Returns 0 for success and negative error for failure
**/
int isp_unlink_src_port(isp_t *isp, isp_port_t *isp_src_port,
  mct_port_t *peer_port, uint32_t session_id, uint32_t stream_id)
{
  int rc = 0;
  isp_stream_t *stream = NULL;
  isp_src_port_t *src_port = &isp_src_port->u.src_port;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  stream = isp_util_find_stream(isp, session_id, stream_id);
  if (stream == NULL) {
    CDBG_ERROR("%s:%d] stream not found. error\n", __func__, __LINE__);
    return -1;
  }
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return rc;
}

/** isp_meta_channel_config
*    @isp: isp ctrl instance
*    @session_id: current senssion object
*    @stream_id: isp stream object
*    @meta_info: sensor meta information
*
*  This method saves meta info for RDI stream
*
*  Returns 0 for success and negative error for failure
**/
int isp_meta_channel_config(isp_t *isp, uint32_t stream_id, uint32_t session_id,
  sensor_meta_t *meta_info)
{
  isp_session_t *session = NULL;
  isp_stream_t *stream = NULL;
  uint32_t meta_channel_id;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  session = isp_util_find_session(isp, session_id);
  if (session == NULL) {
    CDBG_ERROR("%s: no more session availabe, max = %d\n",
      __func__, ISP_MAX_SESSIONS);
    return -1;
  }
  stream = isp_util_find_stream_in_session(session, stream_id);
  if (!stream) {
    ISP_DBG(ISP_MOD_COM,"%s: stream ont exist, session_id = %d, stream_id = %d\n",
      __func__, session_id, stream_id);
    return -1;
  }
  stream->meta_info = *meta_info;
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return 0;
}

/** isp_add_meta_channel
*    @isp: isp ctrl instance
*    @session: current senssion object
*    @stream: isp stream object
*
*  Adds isp meta data channel on RDI path
*
*  Returns 0 for success and negative error for failure
**/
static int isp_add_meta_channel(isp_t *isp, isp_session_t *session,
  isp_stream_t *stream)
{
  int rc = 0;
  isp_channel_t *meta_channel;
  uint32_t meta_channel_id;
  sensor_meta_t *meta_info = NULL;
  mct_stream_info_t stream_info;

  ISP_DBG(ISP_MOD_COM,"%s: E", __func__);
  /* if we do not have meta data no op */
  if (stream->meta_info.is_valid == 0) {
    CDBG_HIGH("%s: no meta data, session_id = %d, stream_id = %d\n",
      __func__, stream->session_id, stream->stream_id);
    return 0;
  }

  meta_info = &stream->meta_info;
  meta_channel_id = stream->stream_id | ISP_META_CHANNEL_BIT;
  /*meta_channel_id = 6;*/
  /* it is good to delay adding meta channel till isp_streamon.
   * Will revisit later after making vhdr working. Now, if receive multiple
   * meta data configuration we assume that meta data information is the
   * same so that we only keep the first one.
   */
  meta_channel = isp_ch_util_find_channel_in_session(session, meta_channel_id);
  if (meta_channel) {
    CDBG_HIGH("%s: alraedy has meta cahnnel for session %d, nop\n",
      __func__, session->session_id);
    return 0;
  }

  /*TO CHECK:  fill in man made stream info, CHECK if anything missing*/
  stream_info.fmt = meta_info->fmt;
  stream_info.dim.height = meta_info->dim.height;
  stream_info.dim.width = meta_info->dim.width;
  stream_info.num_burst = 0;
  stream_info.stream_type = CAM_STREAM_TYPE_METADATA;
  stream_info.streaming_mode = stream->stream_info.streaming_mode;
  memset(&stream_info.buf_planes.plane_info, 0,
    sizeof(stream_info.buf_planes.plane_info));
  stream_info.buf_planes.plane_info.num_planes = 1;
  stream_info.buf_planes.plane_info.mp[0].width = meta_info->dim.width;
  stream_info.buf_planes.plane_info.mp[0].height = meta_info->dim.height;
  stream_info.buf_planes.plane_info.mp[0].scanline =
    PAD_TO_SIZE(meta_info->dim.height, CAM_PAD_TO_2);
  stream_info.buf_planes.plane_info.mp[0].stride =
    PAD_TO_SIZE(meta_info->dim.width, CAM_PAD_TO_16);
  if (meta_info->fmt == CAM_FORMAT_META_RAW_10BIT) {
    stream_info.buf_planes.plane_info.mp[0].stride =
    PAD_TO_SIZE(meta_info->dim.width*5/4, CAM_PAD_TO_16);
  }
  stream_info.buf_planes.plane_info.mp[0].len =
    stream_info.buf_planes.plane_info.mp[0].stride *
    stream_info.buf_planes.plane_info.mp[0].height;
  stream_info.buf_planes.plane_info.frame_len =
    PAD_TO_SIZE(stream_info.buf_planes.plane_info.mp[0].len,
      CAM_PAD_TO_4K);

  /* add meta chanenl onto session*/
  meta_channel = isp_ch_util_add_channel(isp, session->session_id,
    meta_channel_id, stream->stream_idx, &stream_info,
    ISP_CHANNEL_TYPE_META_DATA);
  if (meta_channel == NULL) {
    CDBG_ERROR("%s: error, no empty slot for meta channel, identity = 0x%x\n",
      __func__, stream->stream_info.identity);
    return -1;
  }

  /*sync bundle stream config into channel,
    set natvie buffer and emta use output mask*/
  meta_channel->cfg.vfe_output_mask = stream->cfg.meta_use_out_mask;
  meta_channel->cfg.ispif_out_cfg = stream->cfg.ispif_out_cfg;
  meta_channel->cfg.ispif_out_cfg.is_split  = FALSE;
  meta_channel->cfg.sensor_cfg = stream->cfg.sensor_cfg;
  meta_channel->cfg.vfe_mask = stream->cfg.vfe_mask;
  meta_channel->divert_to_3a = stream->divert_to_3a;
  meta_channel->meta_info    = stream->meta_info;
  meta_channel->sink_port = stream->sink_port;
  meta_channel->src_ports[ISP_SRC_PORT_DATA] =
    stream->src_ports[ISP_SRC_PORT_DATA];
  meta_channel->src_ports[ISP_SRC_PORT_3A] =
    stream->src_ports[ISP_SRC_PORT_3A];
  meta_channel->state = ISP_CHANNEL_STATE_USER_CFG;
  meta_channel->use_native_buf = 1;

  stream->channel_idx_mask |=
    (1 << isp_ch_util_get_channel_idx(meta_channel));

  CDBG_HIGH("%s: X, channel added mask %x streamid %x\n", __func__,
    stream->channel_idx_mask, stream->stream_id);
  return rc;
}

/** isp_video_hdr_config
*    @isp: isp ctrl instance
*    @session_id: current senssion object
*    @stream_id: current stream id
*
*  This method set video hdr mode to vfe hw and save vhdr param.
*
*  Returns 0 for success and negative error for failure
**/
int isp_video_hdr_config(isp_t *isp, uint32_t stream_id, uint32_t session_id,
  sensor_meta_t *meta_info)
{
  uint32_t is_valid;
  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  if (!meta_info) {
    CDBG_ERROR("%s: error, null meta_info, session_id = %d, stream_id = %d\n",
      __func__, session_id, stream_id);
    return -1;
  }
  is_valid = meta_info->is_valid;
  return (isp_util_set_video_hdr(isp, session_id, stream_id, &is_valid));
}

/** isp_sink_port_stream_config
*    @isp: isp ctrl instance
*    @isp_sink_port: current senssion object
*    @ispif_src_stream_cfg: ispif source port stream cfg
*
*  This method processes User stream config, decides if dual isp
*  co-operative mode is to be used. create hw, vfe output mask
*  and init stats configuration
*
*  Returns 0 for success and negative error for failure
**/
int isp_sink_port_stream_config(isp_t *isp, isp_port_t *isp_sink_port,
   ispif_src_port_stream_cfg_t *ispif_src_stream_cfg)
{
  int i, rc = 0;
  cam_flash_mode_t flash_mode;
  isp_sink_port_t *sink_port = &isp_sink_port->u.sink_port;
  isp_stream_t *stream = NULL;
  isp_session_t *session = NULL;
  int open_vfe_ids = 0;

  ISP_DBG(ISP_MOD_COM,"%s: E", __func__);

  stream = isp_util_find_stream(isp, ispif_src_stream_cfg->session_id,
    ispif_src_stream_cfg->stream_id);
  if (!stream) {
    /* cannot find the strae. error */
    CDBG_ERROR("%s: stream (session_id = %d, stream_id = %d) cannot be found\n",
      __func__, ispif_src_stream_cfg->session_id,
      ispif_src_stream_cfg->stream_id);
    return -1;
  }
  CDBG_HIGH("%s: E, session_id = %d, stream_id = %d, stream_type = %d\n",
    __func__, stream->session_id, stream->stream_id,
    stream->stream_info.stream_type);

  if (stream->state > ISP_STREAM_STATE_USER_CFG) {
     CDBG_HIGH("%s: already configured HW. session_id = %d, streamid = %d,"
       "stream_type = %d\n", __func__, stream->session_id ,
       stream->stream_id, stream->stream_info.stream_type);
     return 0;
  }

  stream->cfg.sensor_cfg = ispif_src_stream_cfg->sensor_cfg;

  /* if both vfe is needed for one stream, co-operative mode is required */
  stream->cfg.ispif_out_cfg.is_split =
    (ispif_src_stream_cfg->vfe_output_mask & 0xffff &&
     ispif_src_stream_cfg->vfe_output_mask & 0xffff0000);
  stream->cfg.vfe_output_mask = ispif_src_stream_cfg->vfe_output_mask;
  stream->cfg.meta_use_out_mask = ispif_src_stream_cfg->meta_use_output_mask;
  stream->cfg.vfe_mask = ispif_src_stream_cfg->vfe_mask;
  CDBG_HIGH("%s: session_id = %d, stream_id = %d, is_split = %d\n",
    __func__, stream->session_id, stream->stream_id,
    stream->cfg.ispif_out_cfg.is_split);
  stream->state = ISP_STREAM_STATE_USER_CFG;
  session = (isp_session_t *)stream->session;
#if 0
  if (isp_ch_util_sync_stream_cfg_to_channel(isp, session, stream) < 0) {
    /* only error is no stream 2 */
    CDBG_ERROR("%s: isp_ch_util_sync_stream_cfg_to_channel error\n", __func__);
    return 0;
  }
#endif
  /* if one stream requires ISP pipeline we save the bit in session */
  if (sink_port->caps.use_pix && session->use_pipeline == 0)
    session->use_pipeline = TRUE;

  open_vfe_ids = session->vfe_ids;

  if (stream->cfg.vfe_mask) {
    /* save the vfe_id mask in session. */
    session->vfe_ids = stream->cfg.vfe_mask;
  }
  CDBG_HIGH("%s: old vfe_id_mask = 0x%x, new vfe_id_mask = 0x%x\n",
    __func__, open_vfe_ids, session->vfe_ids);
  if (session->vfe_ids & (1 << VFE0)) {
    if ((open_vfe_ids & (1 << VFE0)) == 0) {
      rc = isp_util_create_hw(isp, VFE0, 1);
      if (rc < 0) {
        CDBG_ERROR("%s: cannot create ISP HW %d, rc = %d\n",
          __func__, VFE0, rc);
        return rc;
      }
    }
  } else if (open_vfe_ids & (1 << VFE0)) {
    CDBG_HIGH("%s: vfe_id %d not used in session %d, close it\n",
      __func__, VFE0, session->session_id);
    isp_util_destroy_hw(isp, VFE0, 1);
  }

  if (session->vfe_ids & (1 << VFE1)) {
    if ((open_vfe_ids & (1 << VFE1)) == 0) {
      rc = isp_util_create_hw(isp, VFE1, 1);
      if (rc < 0) {
        CDBG_ERROR("%s: cannot create ISP HW %d, rc = %d\n", __func__, VFE1,
          rc);
        return rc;
      }
    }
  } else if (open_vfe_ids & (1 << VFE1)) {
    CDBG_HIGH("%s: vfe_id %d not used in session %d, close it\n",
      __func__, VFE0, session->session_id);
    isp_util_destroy_hw(isp, VFE1, 1);
  }

  rc = isp_util_gen_init_stats_cfg(session, stream);
  if (rc < 0) {
      CDBG_ERROR("%s: cannot init stats parm %d, rc = %d\n",
        __func__, VFE1, rc);
      return rc;
  }

  if (ispif_src_stream_cfg->sensor_cfg.prep_flash_on == TRUE) {
    session->saved_params.flash_mode = CAM_FLASH_MODE_ON;
  }


  ISP_DBG(ISP_MOD_COM,"%s: x", __func__);
  return rc;
}
/** isp_proc_eztune_command
 *  @isp
 *  @session_id
 *  @stream_id
 *  @eztune_payload
 *
 **/
int isp_proc_eztune_command(isp_t *isp,
  uint32_t session_id, uint32_t stream_id,
  void* eztune_payload)
{
  int rc = 0;
  tune_cmd_t *ezt_cmd = (tune_cmd_t*)eztune_payload;
  optype_t type = ezt_cmd->type;
  vfemodule_t mod  = ezt_cmd->module;
  isp_mod_trigger_t mod_trig;
  switch(type) {
  case SET_STATUS: {
    if (mod == VFE_MODULE_ALL){
     rc = isp_util_set_eztune_diagnostics(isp,
       session_id, stream_id, &ezt_cmd->value);
    }
  }
    break;

  case SET_CONTROLENABLE: {
    mod_trig.module = ezt_cmd->module;
    mod_trig.enable = ezt_cmd->value;
    isp_util_set_module_trigger(isp,
     session_id, stream_id, &mod_trig);

  }
    break;

  case SET_ENABLE: {
    mod_trig.module = ezt_cmd->module;
    mod_trig.enable = ezt_cmd->value;
    isp_util_set_module_enable(isp,
     session_id, stream_id, &mod_trig);
  }
    break;

  default: {
    rc = -1;  //Default Case
  }
    break;
 }
  return rc;
}

/** isp_set_hal_param
*    @isp: isp ctrl instance
*    @isp_sink_port: current senssion object
*    @session_id: current session id
*    @stream_id: current stream id
*    @event: mct event object
*
*  Set diff control paramsreceived from HAL, such as sharpness,
*  cpntrast, specialeffects, rolloff, saturation , mce, sce,
*  recording hint etc.
*
*  Returns 0 always
**/
int isp_set_hal_param(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, mct_event_t *event)
{
  int rc = 0;
  mct_event_control_t *ctrl_event = &event->u.ctrl_event;
  mct_event_control_parm_t *param = ctrl_event->control_event_data;
  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  switch (param->type) {
  case CAM_INTF_PARM_SHARPNESS: {
    /* TODO: need to know how to cast void ptr */
    rc = isp_util_set_sharpness(isp, isp_sink_port,
      session_id, stream_id, (int32_t *)param->parm_data);
  }
    break;

  case CAM_INTF_PARM_CONTRAST: {
    rc = isp_util_set_contrast(isp, isp_sink_port,
      session_id, stream_id, (int32_t *)param->parm_data);
  }
    break;

  case CAM_INTF_PARM_SATURATION: {
    rc = isp_util_set_saturation(isp, isp_sink_port,
      session_id, stream_id, (int32_t *)param->parm_data, FALSE);
  }
    break;

  case CAM_INTF_PARM_ZOOM: {
    rc = isp_util_buffered_set_param_zoom(isp,
      session_id, stream_id, (int32_t *)param->parm_data);
    /* check YUV sensor to setup PPROC-only zoom*/
    if(isp_util_check_yuv_sensor_from_stream(isp,
      session_id, stream_id)) {
      isp_util_do_pproc_zoom(isp,
      session_id,  (int32_t *)param->parm_data);
    }
  }
    break;

  case CAM_INTF_PARM_EFFECT: {
    rc = isp_util_set_effect(isp, isp_sink_port,
      session_id, stream_id, (int32_t *)param->parm_data);
  }
    break;

  case CAM_INTF_PARM_ROLLOFF: {
    /* TODO: need to know how to cast void ptr */
  }
    break;

  case CAM_INTF_PARM_AEC_ROI: {
  }
    break;

  case CAM_INTF_PARM_BESTSHOT_MODE: {
    rc = isp_util_set_bestshot(isp, isp_sink_port,
      session_id, stream_id, (cam_scene_mode_type *)param->parm_data);
  }
    break;

  case CAM_INTF_PARM_SCE_FACTOR: {
    rc = isp_util_set_skin_color_enhance(isp, isp_sink_port,
      session_id, stream_id, (int32_t *)param->parm_data);
  }
    break;

  case CAM_INTF_PARM_MCE: {
    /* TODO: need to know how to cast void ptr */
  }
    break;

  case CAM_INTF_PARM_HFR: {
    rc = isp_util_set_hfr(isp, session_id, stream_id,
     (int32_t *)param->parm_data);
  }
    break;

  case CAM_INTF_PARM_DIS_ENABLE: {
    rc = isp_util_set_dis(isp, session_id, stream_id,
      (int32_t *)param->parm_data);
  }
    break;

  case CAM_INTF_PARM_HISTOGRAM: {
    /* TODO: need to know how to cast void ptr */
  }
    break;

  case CAM_INTF_PARM_FRAMESKIP: {
    rc = isp_util_set_frame_skip(isp, session_id, stream_id,
      (int32_t *)param->parm_data);
  }
    break;

  case CAM_INTF_PARM_RECORDING_HINT: {
    rc = isp_util_set_recording_hint(isp, session_id, stream_id,
      (int32_t *)param->parm_data);
  }
    break;

  case CAM_INTF_PARM_TINTLESS: {
    rc = isp_util_set_param_tintless(isp, session_id,
           (uint8_t *)param->parm_data);
  }
    break;

  case CAM_INTF_PARM_SET_VFE_COMMAND: {
    rc = isp_proc_eztune_command(isp, session_id, stream_id,
     (void *)param->parm_data);
  }
    break;

  case CAM_INTF_PARM_VT: {
    rc = isp_util_set_vt(isp, session_id,
      (int32_t *)param->parm_data);
  }
    break;

  case CAM_INTF_PARM_CDS_MODE: {
    rc = isp_util_set_cds_mode(isp, session_id, param->parm_data);
  }
    break;

  case CAM_INTF_PARM_LONGSHOT_ENABLE: {
    rc = isp_util_set_longshot(isp, session_id,
         (int32_t *)param->parm_data);
  }
    break;

  case CAM_INTF_PARM_LOW_POWER_ENABLE: {
    rc = isp_util_set_lowpowermode(isp, session_id,
         param->parm_data);
  }
    break;

  case CAM_INTF_PARM_ISP_DEBUG_MASK: {
    ISP_MOD_COM = ISP_MOD_MAX_NUM;
    isp_debug_mask = *(int32_t *)param->parm_data;
  }
    break;

  default: {
    ISP_DBG(ISP_MOD_COM,"%s: Type %d not supported", __func__, param->type);
  }
    break;
  }
  return 0;
}

/** isp_set_hal_stream_param
*    @isp: isp ctrl instance
*    @isp_sink_port: current senssion object
*    @session_id: current session id
*    @stream_id: current stream id
*    @event: mct event object
*
*  Set stream params received from HAl, bundling information
*  related to stream bundling.
*
*  Returns 0 always
**/
int isp_set_hal_stream_param(isp_t *isp, isp_port_t *isp_sink_port,
   uint32_t session_id, uint32_t stream_id, mct_event_t *event)
{
  int rc = 0;
  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %d stream_id %d", __func__, (void *)isp,
    session_id, stream_id);

  cam_stream_parm_buffer_t *param = event->u.ctrl_event.control_event_data;

  switch (param->type) {
  case CAM_STREAM_PARAM_TYPE_SET_BUNDLE_INFO: {
    rc = isp_util_set_bundle(isp, isp_sink_port,
      session_id, stream_id, &param->bundleInfo);
  }
    break;

  default: {
    CDBG_HIGH("%s: Type %d not supported", __func__, param->type);
  }
    break;
  }
  ISP_DBG(ISP_MOD_COM,"%s: x", __func__);
  return 0;
}

/** isp_proc_set_hw_params
*    @isp: isp ctrl instance
*    @session: current senssion object
*
*  Process set hw params call
*
*  Returns 0 always
**/
int isp_proc_set_hw_params(isp_t *isp, isp_session_t *session)
{
  int rc = 0;
  isp_hw_params_t new_params;

  ISP_DBG(ISP_MOD_COM,"%s: E", __func__);
  pthread_mutex_lock(
    &isp->data.session_critical_section[session->session_idx]);
  if (!session->buffered_hw_params.in_service) {
    CDBG_HIGH("%s: session_id = %d, not in service\n",
      __func__, session->session_id);
    pthread_mutex_unlock(
      &isp->data.session_critical_section[session->session_idx]);
    return rc;
  }
  new_params = session->buffered_hw_params.new_params;
  session->buffered_hw_params.hw_update_pending = FALSE;

  /* zero the new params since we are sending to hw now. */
  memset(&session->buffered_hw_params.new_params, 0,
    sizeof(session->buffered_hw_params.new_params));
  pthread_mutex_unlock(
    &isp->data.session_critical_section[session->session_idx]);

  /* apply zoom */
  rc = isp_util_set_param_zoom(isp, session, &new_params);
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return rc;
}

/** isp_create
*    @isp: isp ctrl instance
*
*  Allocate isp module base, initializes mutexes, generate hw
*  caps, init buf mgr, reousrce mgr
*
*  Returns 0 always
**/
int isp_create(isp_t **isp_ctrl)
{
  int i, rc = 0;
  isp_t *isp = NULL;
  isp_info_t isp_info[ISP_MAX_NUM];

  ISP_DBG(ISP_MOD_COM,"%s: E", __func__);
  *isp_ctrl = NULL;
  /* allocate isp module base */
  isp = (isp_t *)malloc(sizeof(isp_t));
  if (!isp) {
    /* no mem */
    CDBG_ERROR("%s: no mem", __func__);
    return -1;
  }

  memset(isp, 0,  sizeof(isp_t));
  pthread_mutex_init(&isp->mutex, NULL);

  for (i = 0; i < ISP_MAX_HW; i++)
    pthread_mutex_init(&isp->data.hw[i].mutex, NULL);

  rc = isp_util_gen_hws_caps(isp);
  if ((rc) ||
      (isp->data.sd_info.num < 1) ||
      (isp->data.sd_info.num > ISP_MAX_NUM)) {
    CDBG_ERROR("%s: cannot generate ISP capabilities\n", __func__);
    isp_destroy(isp);
    goto end;
  }

  for (i = 0; i < ISP_MAX_SESSIONS; i++) {
    pthread_mutex_init(&isp->data.session_critical_section[i], NULL);
    pthread_mutex_init(&isp->data.sessions[i].state_mutex, NULL);
  }

  isp->data.tintless = isp_tintless_create(isp->data.sd_info.sd_info[0].isp_version);
  if (!isp->data.tintless) {
    CDBG_ERROR("%s: tintless failed\n", __func__);
    isp_destroy(isp);
    goto end;
  }

  isp->data.zoom = isp_zoom_create(isp->data.sd_info.sd_info[0].isp_version);
  if (!isp->data.zoom) {
    CDBG_ERROR("%s: isp_zoom_init failed\n", __func__);
    isp_destroy(isp);
    goto end;
  }

  *isp_ctrl = isp;
  isp_resource_mgr_init(isp->data.sd_info.sd_info[0].isp_version, (void *)isp);
  for (i = 0; i < isp->data.sd_info.num; i++) {
    isp_info[i] = isp->data.sd_info.sd_info[i].cap.isp_info;
  }
  isp_set_info(isp->data.sd_info.num, isp_info);

  isp_init_buf_mgr(&isp->data.buf_mgr);
end:
  ISP_DBG(ISP_MOD_COM,"%s: X, isp = %p\n", __func__, *isp_ctrl);
  return rc;
}

/** isp_destroy
*    @isp: isp instance
*
*  Detroys all the mutex, zoom, deinit the buf mgr and release
*  isp struct
*
*  Returns 0 always
**/
int isp_destroy (isp_t *isp)
{
  int i;
  ISP_DBG(ISP_MOD_COM,"%s: E", __func__);
  if (!isp)
    return 0;

  if (isp->data.tintless) {
    isp_tintless_destroy(isp->data.tintless);
    isp->data.tintless = NULL;
  }

  if (isp->data.zoom) {
    isp_zoom_destroy(isp->data.zoom);
    isp->data.zoom = NULL;
  }

  for (i = 0; i < ISP_MAX_SESSIONS; i++) {
    pthread_mutex_destroy(&isp->data.session_critical_section[i]);
    pthread_mutex_destroy(&isp->data.sessions[i].state_mutex);
  }

  for (i = 0; i < ISP_MAX_HW; i++)
    pthread_mutex_destroy(&isp->data.hw[i].mutex);
  isp_deinit_buf_mgr(&isp->data.buf_mgr);
  pthread_mutex_destroy(&isp->mutex);
  free(isp);
  isp_resouirce_mgr_destroy();
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return 0;
}

/** isp_set_cam_bracketing_ctrl
*    @isp: isp ctrl instance
*    @isp_sink_port: isp port object
*    @data:
*
*  TODO
*
*  Returns 0 for success and negative error for failure
**/
int isp_set_cam_bracketing_ctrl(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, void *data)
{
  int rc = 0;
  int isp_id = 0;

  mct_bracket_ctrl_t *bracket = (mct_bracket_ctrl_t *)data;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  if (!session) {
    CDBG_ERROR("%s: session is not existing. sessionid = %d\n",
      __func__, session_id);
    return -1;
  }
  CDBG_ERROR("%s:%d] session_id %d stream_id %d state %d", __func__,
    __LINE__, session_id, stream_id, bracket->state);

  pthread_mutex_lock(
    &isp->data.session_critical_section[session->session_idx]);

  session->bracket_info = *bracket;

  switch (session->bracket_info.state) {
  case MCT_BRACKET_CTRL_STORE_3A:
    session->saved_params.aec_stored_stats =
      session->saved_params.aec_stats_update;
    session->saved_params.awb_stored_stats =
      session->saved_params.awb_stats_update;
    break;
  case MCT_BRACKET_CTRL_RESTORE_LOCK_3A:
    session->saved_params.aec_stats_update =
      session->saved_params.aec_stored_stats;
    session->saved_params.awb_stats_update =
      session->saved_params.awb_stored_stats;
    break;
  default:;
  }
  pthread_mutex_unlock(
     &isp->data.session_critical_section[session->session_idx]);

  isp_util_set_cam_bracketing_ctrl_param(isp, session, *bracket);

  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return rc;
}


