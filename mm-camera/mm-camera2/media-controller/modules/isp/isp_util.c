/*============================================================================
Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
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
#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "isp_def.h"
#include "isp_ops.h"
#include "isp_hw.h"
#include "isp.h"
#include "isp_util.h"
#include "isp_buf_mgr.h"
#include "isp_pipeline.h"
#include "isp_resource_mgr.h"
#include "q3a_stats_hw.h"
#include "isp_log.h"
#include "server_debug.h"


#define PAD_TO_SIZE(size, padding)  ((size + padding - 1) & ~(padding - 1))
#define MAX_VFE_SCALERS_RATIO       (15u)
#define PRECISION_MUL               (100) /* precision multiplier */

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#if ISP_UTIL_DEBUG
#undef CDBG
#define CDBG ALOGE
#endif
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE

/* some local structs for mct_list traverse */
typedef struct {
  uint32_t session_id;
  uint32_t stream_id;
} isp_find_mct_stream_t;

typedef struct {
  int cnt;
  isp_frame_buffer_t *isp_map_buf;
} find_stream_map_buf_t;

typedef struct {
  ispif_src_port_caps_t *caps;
  isp_stream_t *stream;
} isp_src_port_match_param_t;

static int isp_util_find_burst_stream_in_continuous_mode(isp_session_t *session,
  isp_stream_t **stream_ptr);

uint32_t isp_util_is_lowpowermode_feature_enable(isp_t *isp, uint32_t session_id)
{
  isp_session_t *session = isp_util_find_session(isp, session_id);
  if (!session)
    return -1;
  return session->saved_params.lowpowermode_enable;
}

/** isp_util_is_video_hint_set
 *    @session: session instance
 *    @user_stream:
 *
 *  This method checks for steram type and determines if video
 *  hint is set or not.
 *
 * Returns 1 - if Video hint is set else 0 if not
 **/
static boolean isp_util_is_video_hint_set(isp_session_t *session,
  isp_stream_t *user_stream)
{
  int i, video_hint = 0;
  ISP_DBG(ISP_MOD_COM,"%s: E", __func__);

  if (user_stream->stream_info.stream_type == CAM_STREAM_TYPE_VIDEO) {
    /* this is video case */
    video_hint = 1;
  } else {
    for (i = 0; i < ISP_MAX_STREAMS; i++) {
      if (session->streams[i].sink_port == NULL)
        continue;
      if (session->streams[i].stream_info.streaming_mode ==
          CAM_STREAMING_MODE_CONTINUOUS) {
        if (session->streams[i].stream_info.stream_type ==
             CAM_STREAM_TYPE_VIDEO) {
          /* this is video case. Need to put two continusous
           * streams into one src port*/
          video_hint = 1;
          break;
        }
      }
    }
  }
  ISP_DBG(ISP_MOD_COM,"%s: X video_hint %d", __func__, video_hint);
  return video_hint;
}

boolean isp_util_is_4k2k_resolution_set(cam_dimension_t dim)
{
  bool enabled = false;
  if ((dim.width == 4096 && dim.height == 2160) ||
    (dim.width == 3840 && dim.height == 2160) ){
    enabled = true;
  }
  return enabled;
}

/** isp_util_find_matched_existing_src_port
 *    @isp: session instance
 *    @isp_sink_port:
 *    @cap_type:
 *    @user_stream:
 *
 * TODO
 *
 * Returns TODO
 **/
static mct_port_t *isp_util_find_matched_existing_src_port(isp_t *isp,
  isp_port_t *isp_sink_port, mct_port_caps_type_t cap_type,
  isp_stream_t *user_stream)
{
  isp_sink_port_t *sink_port = &isp_sink_port->u.sink_port;
  isp_src_port_t *src_port = NULL;
  isp_port_t *isp_src_port = NULL;
  mct_port_t *mct_port;
  isp_session_t *session;
  int i, num_src_ports = 0;
  uint32_t size;
  uint32_t isp_ver;

  ISP_DBG(ISP_MOD_COM,"%s: E", __func__);

  /* we always use ISP 0 version now. */
  isp_ver = isp->data.sd_info.sd_info[0].isp_version;
  session = isp_util_find_session(isp, user_stream->session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session %d\n",
      __func__, user_stream->session_id);
    return NULL;
  }
  ISP_DBG(ISP_MOD_COM,"%s: session_id = %d, stream_id = %d, stream_type = %d,"
    "num_data_port = %d\n", __func__,
    user_stream->session_id, user_stream->stream_id,
    user_stream->stream_info.stream_type,
    user_stream->stream_info.streaming_mode);

  if (session->num_src_data_port < 2) {
    /* for burst. we can allow allocate two src frame ports */
    if (user_stream->stream_info.streaming_mode !=
        CAM_STREAMING_MODE_CONTINUOUS) {
      CDBG_ERROR("%s#%d: X", __func__, __LINE__);
      return NULL;
    } else {
      /* if we already has a burst stream linked we need to
       * put all continuous streams into one src port */
      isp_stream_t *tmp_stream = NULL;
      isp_util_find_burst_stream_in_continuous_mode(session, &tmp_stream);

      if (tmp_stream) {
        if (tmp_stream->src_ports[ISP_SRC_PORT_DATA] ||
            (tmp_stream->src_ports[ISP_SRC_PORT_DATA] == NULL &&
             session->num_src_data_port == 0)) {

          /* This is liveshot case, we put two continuous streams into
             one src port. */
          CDBG_ERROR("%s#%d: X", __func__, __LINE__);
          return NULL;
        }
      } else {
        /* for the first continuous stream we always use a new src port */
        if (session->num_src_data_port == 0) {
          CDBG_ERROR("%s#%d: X", __func__, __LINE__);
          return NULL;
        }
        /* Here, we already have one continuous stream with a src port.
         * if there is one video straem it's camcorder use case put
         * both continuous streams into the same src port only if
         * its 4k2k resolution or YUV.
         */
        if (!isp_util_is_video_hint_set(session, user_stream) ||
            (!isp_util_is_4k2k_resolution_set(user_stream->stream_info.dim) &&
            sink_port->caps.use_pix)) {
          CDBG_ERROR("%s#%d: X", __func__, __LINE__);
          return NULL;
        }
      }
    }
  }

  /* Two situations reach this code:
   * (1) Badger 4k by 2k video
   * (2) ISP alraedy used two output ports. */
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (session->streams[i].state == ISP_STREAM_STATE_INITIAL)
      continue;
    mct_port = session->streams[i].src_ports[ISP_SRC_PORT_DATA];
    if (!mct_port) {
      continue;
    }
    isp_src_port = mct_port->port_private;
    src_port = &isp_src_port->u.src_port;
    if (memcmp(&sink_port->caps, &src_port->caps,
      sizeof(ispif_src_port_caps_t)) == 0) {
      /* cap matches. Now need to make sure that
       * streaming mode matches */
      if (user_stream->stream_info.streaming_mode == src_port->streaming_mode) {
        /* streaming mode also match */
        ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
        return mct_port;
      }
    }
  } /* end for loop */
  ISP_DBG(ISP_MOD_COM,"%s#%d: X", __func__, __LINE__);
  return NULL;
}

/** isp_util_need_zoom
 *    @session: session instance
 *    @stream:
 *
 * ZOOM only apply to VFE outputs going through VFE pipeline
 *
 * Returns TRUE - if zoom mod is neded and FALSE - if not needed
 **/
static boolean isp_util_need_zoom(isp_session_t *session, isp_stream_t *stream)
{
  ISP_DBG(ISP_MOD_COM,"%s: E,stream fmt %d", __func__, stream->stream_info.fmt);
  switch(stream->stream_info.fmt) {
  case CAM_FORMAT_YUV_420_NV12_VENUS:
  case CAM_FORMAT_YUV_420_NV12:
  case CAM_FORMAT_YUV_420_NV21:
  case CAM_FORMAT_YUV_420_NV21_ADRENO:
  case CAM_FORMAT_YUV_420_YV12:
  case CAM_FORMAT_YUV_422_NV16:
  case CAM_FORMAT_YUV_422_NV61: {
    ISP_DBG(ISP_MOD_COM,"%s: X TRUE", __func__);
    return TRUE;
  }

  default: {
  }
    break;
  }
  ISP_DBG(ISP_MOD_COM,"%s: X FALSE", __func__);
  return FALSE;
}

/** isp_util_discover_subdev_nodes
 *    @isp: session instance
 *
 * Discover and open all the subdevs
 *
 * Returns nothing
 **/
static void isp_util_discover_subdev_nodes(isp_t *isp)
{
  struct media_device_info mdev_info;
  int num_media_devices = 0;
  char dev_name[32];
  isp_subdev_info_t *subdev_info = NULL;
  int rc = 0, dev_fd = 0;
  int isp_id = 0;
  int num_isps = 0;
  int num_entities;

  ISP_DBG(ISP_MOD_COM,"%s: E", __func__);
  while (1) {
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (dev_fd >= MAX_FD_PER_PROCESS) {
      dump_list_of_daemon_fd();
      dev_fd = -1;
      break;
    }
    if (dev_fd < 0) {
      ISP_DBG(ISP_MOD_COM,"Done discovering media devices\n");
      break;
    }

    num_media_devices++;
    rc = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (rc < 0) {
      CDBG_ERROR("Error: ioctl media_dev failed: %s\n", strerror(errno));
      close(dev_fd);
      break;
    }

    if (strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model)) != 0) {
      close(dev_fd);
      continue;
    }

    num_entities = 1;

    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities++;
      rc = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (rc < 0) {
        ISP_DBG(ISP_MOD_COM,"Done enumerating media entities\n");
        rc = 0;
        break;
      }
      if (entity.type == MEDIA_ENT_T_V4L2_SUBDEV &&
          entity.group_id == MSM_CAMERA_SUBDEV_VFE &&
          isp->data.sd_info.num < ISP_SD_NODE_ID_MAX_NUM) {
        subdev_info = &isp->data.sd_info.sd_info[isp->data.sd_info.num];
        snprintf(subdev_info->subdev_name, sizeof(subdev_info->subdev_name),
          "/dev/%s", entity.name);
        isp->data.sd_info.num++;
      }
    }
    ISP_DBG(ISP_MOD_COM,"%s: close dev_fd: %d", __func__, dev_fd);
    close(dev_fd);
  }
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
}

/** isp_util_fill_pix_streamids
 *    @isp: session instance
 *    @session_id:
 *    @streamids:
 *    @max_num_streams:
 *
 * Update streamids array with streams needing pix interface
 *
 * Returns TODO
 **/
static int isp_util_fill_pix_streamids(isp_t *isp, uint32_t session_id,
  uint32_t *streamids, int max_num_streams)
{
  int i, cnt = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;
  isp_port_t *isp_sink_port;

  ISP_DBG(ISP_MOD_COM,"%s: E", __func__);
  if (!session)
    return -1;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    stream = &session->streams[i];
    if (!stream)
      return -1;
    if (!stream->sink_port)
      continue;

    isp_sink_port = stream->sink_port->port_private;
    if (isp_sink_port && isp_sink_port->u.sink_port.caps.use_pix &&
        max_num_streams > cnt){
      /* this stream needs pixel interface */
      streamids[cnt++] = stream->stream_id;
    }
  }
  ISP_DBG(ISP_MOD_COM,"%s: X, cnt %d", __func__, cnt);
  return cnt;
}

/** isp_util_broadcast_sof_msg_to_modules
 *    @isp: ISP pointer
 *    @session_id: session id
 *    @stream_id: straem id
 *    @sof_event: sof event pointer
 *
 * Process any v4l2_event received from kernel ISP driver.
 *
 * Return: 0 - success, negative vale - error.
 **/
void isp_util_broadcast_sof_msg_to_modules(isp_t *isp, uint32_t session_id,
  uint32_t stream_id, mct_bus_msg_isp_sof_t *sof_event)
{
  isp_port_t *isp_port;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;
  mct_event_t mct_event;

  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_d %d, stream_id %d", __func__, (void *)isp,
       session_id, stream_id);
  if (!session)
    return;

  stream = isp_util_find_stream_in_session(session, stream_id);
  if (!stream)
    return;

  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.u.module_event.type = MCT_EVENT_MODULE_SOF_NOTIFY;
  mct_event.u.module_event.module_event_data = (void *)sof_event;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.identity = pack_identity(session_id, stream_id);
  mct_event.direction = MCT_EVENT_UPSTREAM;

  /* broadcast sof upstream */
  mct_port_send_event_to_peer(stream->sink_port, &mct_event);
  /* broadcast sof downstream */
  if (stream->src_ports[ISP_SRC_PORT_3A]) {
    mct_event.direction = MCT_EVENT_DOWNSTREAM;
    mct_port_send_event_to_peer(stream->src_ports[ISP_SRC_PORT_3A],
      &mct_event);
  }
  if (stream->src_ports[ISP_SRC_PORT_DATA]) {
    mct_event.direction = MCT_EVENT_DOWNSTREAM;
    mct_port_send_event_to_peer(stream->src_ports[ISP_SRC_PORT_DATA],
      &mct_event);
  }
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
}

/** isp_util_hw_notify_stats_awb_info
 *    @isp: session instance
 *    @bus_msg:
 *
 * sends STATS_AWB_INFO bus notification to stats
 *
 * Returns 0-always
 **/
static int isp_util_hw_notify_stats_awb_info(isp_t *isp, mct_bus_msg_t *bus_msg)
{
  int rc = 0;
  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p", __func__, (void *)isp);
  if (TRUE != mct_module_post_bus_msg(isp->module, bus_msg))
    CDBG_ERROR("%s: STATS_AWB_INFO to bus error\n", __func__);

  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return rc;
}

/** isp_util_hw_notify_stats_be_config
 *
 *    @isp: session instance
 *    @data:
 *    @size:
 *
 * Returns 0-success, -1 on error
 **/
static int isp_util_hw_notify_stats_be_config(isp_t *isp, void *notify_data,
  uint32_t notify_data_size)
{
  int rc = 0;
  isp_session_t *session;
  isp_tintless_notify_data_t *tintless_data = notify_data;

  if (notify_data_size != sizeof(isp_tintless_notify_data_t)) {
    CDBG_ERROR("%s: Type mismatch\n", __func__);
    rc = -1;
    goto error;
  }

  session = isp_util_find_session(isp, tintless_data->session_id);
  if (!session){
    CDBG_ERROR("%s: session not found\n", __func__);
    rc = -1;
    goto error;
  }

  /* feed tintless with BE stats config */
  rc = isp_tintless_be_config(session->tintless_session, tintless_data);

error:
  return rc;
}

/** isp_util_hw_notify_stats_bg_config
 *
 *    @isp: session instance
 *    @data:
 *    @size:
 *
 * Returns 0-success, -1 on error
 **/
static int isp_util_hw_notify_stats_bg_config(isp_t *isp, void *notify_data,
  uint32_t notify_data_size)
{
  int rc = 0;
  isp_session_t *session;
  isp_tintless_notify_data_t *tintless_data = notify_data;

  if (notify_data_size != sizeof(isp_tintless_notify_data_t)) {
    CDBG_ERROR("%s: Type mismatch\n", __func__);
    rc = -1;
    goto error;
  }

  session = isp_util_find_session(isp, tintless_data->session_id);
  if (!session){
    CDBG_ERROR("%s: session not found\n", __func__);
    rc = -1;
    goto error;
  }

  /* feed tintless with BE stats config */
  rc = isp_tintless_bg_config(session->tintless_session, tintless_data);

error:
  return rc;
}
/** isp_util_hw_notify_rolloff_config
 *
 *    @isp: session instance
 *    @data:
 *    @size:
 *
 * Returns 0-success, -1 on error
 **/
static int isp_util_hw_notify_rolloff_config(isp_t *isp, void *notify_data,
  uint32_t notify_data_size)
{
  int rc = 0;
  isp_session_t *session;
  isp_tintless_notify_data_t *tintless_data = notify_data;

  if (notify_data_size != sizeof(isp_tintless_notify_data_t)) {
    CDBG_ERROR("%s: Type mismatch\n", __func__);
    rc = -1;
    goto error;
  }

  session = isp_util_find_session(isp, tintless_data->session_id);
  if (!session){
    CDBG_ERROR("%s: session not found\n", __func__);
    rc = -1;
    goto error;
  }

  /* feed tintless with mesh rolloff config */
  rc = isp_tintless_rolloff_config(session->tintless_session, tintless_data);

error:
  return rc;
}

/** isp_util_hw_notify_rolloff_get
 *
 *    @isp: session instance
 *    @data:
 *    @size:
 *
 * Returns 0-success, -1 on error
 **/
static int isp_util_hw_notify_rolloff_get(isp_t *isp, void *notify_data,
  uint32_t notify_data_size)
{
  int rc = 0;
  isp_session_t *session;
  isp_tintless_notify_data_t *tintless_data = notify_data;

  if (notify_data_size != sizeof(isp_tintless_notify_data_t)) {
    CDBG_ERROR("%s: Type mismatch\n", __func__);
    rc = -1;
    goto error;
  }

  session = isp_util_find_session(isp, tintless_data->session_id);
  if (!session){
    CDBG_ERROR("%s: session not found\n", __func__);
    rc = -1;
    goto error;
  }

  /* get rolloff table from tintless module */
  rc = isp_tintless_get_table(session->tintless_session, tintless_data);

error:
  return rc;
}

/** isp_util_hw_notify_stats
 *    @isp: session instance
 *    @notify_data:
 *
 * send stats event to 3A
 *
 * Returns 0 - sucess and negative value - failure
 **/
static int isp_util_hw_notify_stats(isp_t *isp, void *notify_data,
  void *ctrl)
{
  boolean brc = FALSE;
  int rc = 0;
  mct_event_t event;
  isp_pipeline_stats_parse_t *stats_event = notify_data;
  isp_stream_t *stream;
  isp_hw_t *isp_hw = (isp_hw_t *)ctrl;

  ISP_DBG(ISP_MOD_COM,"%s: E", __func__);
  isp_session_t *session = isp_util_find_session(isp, stats_event->session_id);
  if (!session){
    ISP_DBG(ISP_MOD_COM,"%s: no stats module linked\n", __func__);
    goto error;
  }
  stream = isp_util_find_3a_stream(session);
  if (!stream){
    ISP_DBG(ISP_MOD_COM,"%s: no stats module linked\n", __func__);
    goto error;
  }

  if (stats_event->parsed_stats_event->stats_mask &
      (1 << MSM_ISP_STATS_IHIST)) {
     q3a_ihist_stats_t *ihist_stats = (q3a_ihist_stats_t *)
       stats_event->parsed_stats_event->
         stats_data[MSM_ISP_STATS_IHIST].stats_buf;

    /*save the ihist in session*/
    if (ihist_stats != 0) {
      pthread_mutex_lock(
        &isp->data.session_critical_section[session->session_idx]);
      session->ihist_update = 1;
      session->ihist_stats = *ihist_stats;
      session->saved_params.ihist_update = 1;
      session->saved_params.ihist_stats = *ihist_stats;
      pthread_mutex_unlock(
         &isp->data.session_critical_section[session->session_idx]);
    } else
      ISP_DBG(ISP_MOD_COM,"%s: ihist_Stats NULL, ihist_stats = %p\n", __func__, ihist_stats);
  }

  if (stats_event->parsed_stats_event->stats_mask &
      (1 << MSM_ISP_STATS_BHIST)) {
    q3a_bhist_stats_t *bhist_stats = (q3a_bhist_stats_t *)
      stats_event->parsed_stats_event->
        stats_data[MSM_ISP_STATS_BHIST].stats_buf;

    if (bhist_stats != 0) {
      int i;
      cam_hist_stats_t hist;
      mct_bus_msg_t bus_msg;

      memset(&bus_msg, 0, sizeof(bus_msg));
      bus_msg.type = MCT_BUS_MSG_HIST_STATS_INFO;
      bus_msg.msg = (void *)&hist;
      bus_msg.sessionid = session->session_id;

      hist.type = CAM_HISTOGRAM_TYPE_BAYER;

      if (CAM_HISTOGRAM_STATS_SIZE != MAX_HIST_STATS_NUM) {
        CDBG_ERROR("%s: Size mismatch error\n", __func__);
        goto error;
      }

      hist.bayer_stats.r_stats.max_hist_value = 0;
      hist.bayer_stats.b_stats.max_hist_value = 0;
      hist.bayer_stats.gr_stats.max_hist_value = 0;
      hist.bayer_stats.gb_stats.max_hist_value = 0;

      memcpy(&hist.bayer_stats.r_stats.hist_buf, &bhist_stats->bayer_r_hist,
        sizeof(bhist_stats->bayer_r_hist));
      memcpy(&hist.bayer_stats.b_stats.hist_buf, &bhist_stats->bayer_b_hist,
        sizeof(bhist_stats->bayer_b_hist));
      memcpy(&hist.bayer_stats.gr_stats.hist_buf, &bhist_stats->bayer_gr_hist,
        sizeof(bhist_stats->bayer_gr_hist));
      memcpy(&hist.bayer_stats.gb_stats.hist_buf, &bhist_stats->bayer_gb_hist,
        sizeof(bhist_stats->bayer_gb_hist));

      if (TRUE !=
          mct_module_post_bus_msg(isp->module,(mct_bus_msg_t *)&bus_msg)) {
        CDBG_ERROR("%s: session_id = %d error\n",
          __func__, session->session_id);
      }
    }
  }
  if (GET_ISP_MAIN_VERSION(isp_hw->init_params.isp_version) ==
      ISP_VERSION_32) {
     if (isp->data.tintless->tintless_data.is_supported &&
       isp->data.tintless->tintless_data.is_enabled) {
       if ((stats_event->parsed_stats_event->frame_id + 2) % 10 == 0 &&
         stats_event->parsed_stats_event->frame_id != 0) {
         isp_pipeline_set_stats_fullsize((void *)isp_hw->pipeline.private_data, TRUE);
       }
       if ((stats_event->parsed_stats_event->frame_id + 1) % 10 == 0 &&
         stats_event->parsed_stats_event->frame_id != 0) {
         isp_pipeline_set_stats_fullsize((void *)isp_hw->pipeline.private_data, FALSE);
       }
     }
  }
  /* if HW support BE, then use be stats for tintless*/
  if (isp->data.tintless->tintless_data.stats_support_type ==
    ISP_TINTLESS_STATS_TYPE_BE) {
    if ((stats_event->parsed_stats_event->stats_mask &
      (1 << MSM_ISP_STATS_BE))) {
      q3a_be_stats_t *be_stats = (q3a_be_stats_t *)
         stats_event->parsed_stats_event->
           stats_data[MSM_ISP_STATS_BE].stats_buf;

       if (isp->data.tintless->tintless_data.is_supported &&
           isp->data.tintless->tintless_data.is_enabled) {
         session->tintless_session->frame_id = stats_event->parsed_stats_event->frame_id;

         rc = isp_tintless_trigger_update(session->tintless_session,
           (void *)be_stats, MSM_ISP_STATS_BE, session->hfr_param.hfr_mode);
         if (rc < 0) {
           CDBG_ERROR("%s: tintless trigger update rc: %d\n", __func__, rc);
         }
       }
     }
  } else if (isp->data.tintless->tintless_data.stats_support_type ==
     ISP_TINTLESS_STATS_TYPE_BG) {
    if (GET_ISP_MAIN_VERSION(isp_hw->init_params.isp_version) ==
      ISP_VERSION_32) {
      if (stats_event->parsed_stats_event->is_tintless_data) {
        q3a_bg_stats_t *bg_stats = (q3a_bg_stats_t *)
          stats_event->parsed_stats_event->
          stats_data[MSM_ISP_STATS_BG].stats_buf;
        if (isp->data.tintless->tintless_data.is_supported &&
          isp->data.tintless->tintless_data.is_enabled) {
          session->tintless_session->frame_id = stats_event->parsed_stats_event->frame_id;
          isp_pipeline_set_stats_fullsize((void *)isp_hw->pipeline.private_data, FALSE);
          rc = isp_tintless_trigger_update(session->tintless_session,
            (void *)bg_stats, MSM_ISP_STATS_BG, session->hfr_param.hfr_mode);
          if (rc < 0) {
            CDBG_ERROR("%s: tintless trigger update rc: %d\n", __func__, rc);
          }
          return rc;
        }
      }
    } else {
      if (stats_event->parsed_stats_event->stats_mask &
        (1 << MSM_ISP_STATS_BG)) {
        q3a_bg_stats_t *bg_stats = (q3a_bg_stats_t *)
          stats_event->parsed_stats_event->
          stats_data[MSM_ISP_STATS_BG].stats_buf;

        if (isp->data.tintless->tintless_data.is_supported &&
          isp->data.tintless->tintless_data.is_enabled) {
          session->tintless_session->frame_id = stats_event->parsed_stats_event->frame_id;
          rc = isp_tintless_trigger_update(session->tintless_session,
            (void *)bg_stats, MSM_ISP_STATS_BG, session->hfr_param.hfr_mode);
          if (rc < 0) {
            CDBG_ERROR("%s: tintless trigger update rc: %d\n", __func__, rc);
          }
        }
      }
    }
  }
  /*send event to 3A*/
  memset(&event, 0, sizeof(event));

  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = pack_identity(stream->session_id, stream->stream_id);
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_DATA;
  event.u.module_event.module_event_data = (void *)stats_event->
    parsed_stats_event;
  brc = mct_port_send_event_to_peer(stream->src_ports[ISP_SRC_PORT_3A], &event);
  ISP_DBG(ISP_MOD_COM,"%s: X,send stats to 3A, rc = %d\n", __func__, brc);

  return 0;
error:
  return -1;
}

/** isp_ch_util_hw_notify_pca_rolloff
 *
 * DESCRIPTION:
 *
 **/
int isp_util_hw_notify_cur_rolloff(isp_t *isp, void *notify_data)
{
  int rc = 0;
  isp_pipeline_curr_rolloff_t *cur_rolloff = notify_data;
  isp_session_t *session;

  session = isp_util_find_session(isp, cur_rolloff->session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n",
               __func__, cur_rolloff->session_id);
    return -1;
  }
  memcpy(&session->tintless_session->curr_rolloff_hw, cur_rolloff->rolloff,
    sizeof(tintless_mesh_rolloff_array_t));
  return rc;
}

/** isp_util_fetch_hw_pending_update_params
 *    @isp: ISP pointer
 *    @params: copy of saved params (fetched params pointer)
 *
 * Returns copy of set of saved params. This copy will be
 * retained and used by both VFEs so that same params get
 * applied to both hardwares. Any new param updates in between
 * will be updated in saved params. This fetch param copy is
 * untouched till both hw are done hw_update.
 *
 * Return: 0 - success, negative vale - error.
 **/
static int isp_util_fetch_hw_pending_update_params(isp_t *isp,
   isp_hw_pending_update_params_t *params)
{
  int rc = 0;
  int i = 0;
  boolean wait_hw_fetch = FALSE;

  isp_session_t *session = isp_util_find_session(isp, params->session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session %d\n",
      __func__, params->session_id);
    return -1;
  }
  pthread_mutex_lock(
    &isp->data.session_critical_section[session->session_idx]);

  /*dual vfe case: check if all HW device fetched the previous pending parm yet
    single vfe: never wait for another isp*/
  if ((session->vfe_ids & (1<< VFE0)) && (session->vfe_ids & (1 << VFE1))) {
    for (i = 0; i < VFE_MAX; i++) {
      if (session->pending_update_params.hw_fetch_pending[i] == TRUE) {
        wait_hw_fetch = TRUE;
        break;
      }
     }
  }

  /* Both VFEs will ask for set of params to be applied to hw update.
   * This call is made by individual VFE during SOF. This function will make
   * a cached param copy using saved params and send same copy to both VFEs
   * by checking frame id. Cached param copy will be updated
   * when frame id changes */
  if (params->frame_id != session->pending_update_params.frame_id
      && wait_hw_fetch == FALSE) {
    session->pending_update_params.hw_update_params = session->saved_params;
    session->saved_params.uv_subsample_update = FALSE;
    session->saved_params.uv_subsample_enable = 0;
    session->saved_params.ihist_update = 0;
    if (session->saved_params.dig_gain == 0.0) {
      session->saved_params.dig_gain = 1.0;
    }

    session->saved_params.zoom_update = FALSE;
    session->pending_update_params.frame_id = params->frame_id;
    /*after update pending parm, both vfe need to fetch*/
    for (i = 0; i < VFE_MAX; i++)
      session->pending_update_params.hw_fetch_pending[i] = TRUE;
  }
  /*fetch the hw update parm from current pending parm*/
  params->hw_update_params = session->pending_update_params.hw_update_params;
  params->hw_update_params.dig_gain =
     session->pending_update_params.hw_update_params.dig_gain ;
  params->hw_update_params.stats_flag =
     session->pending_update_params.hw_update_params.stats_flag;
  params->hw_update_params.aec_stats_update =
     session->pending_update_params.hw_update_params.aec_stats_update;
  session->pending_update_params.hw_fetch_pending[params->dev_idx] = FALSE;
  pthread_mutex_unlock(
    &isp->data.session_critical_section[session->session_idx]);
  return 0;
}

static int isp_util_notify_ispif_to_reset(isp_t *isp, int hw_idx, uint32_t *session_id, int size)
{
  int rc = 0, i, j;
  isp_session_t *session = NULL;
  isp_stream_t *stream = NULL;
  mct_event_t event;
  uint32_t is_overflow = 1;
  uint32_t isp_id;

  if (size != sizeof(uint32_t)) {
    CDBG_ERROR("%s Size mismatch error \n", __func__);
    return -1;
  }
  session = isp_util_find_session(isp, *session_id);
  if (!session) {
    CDBG_ERROR("%s: isp_util_find_session failed\n",__func__);
    return -1;
  }
  if (session->vfe_ids & (1 << VFE0)) {
    isp_id = VFE0;
    rc = isp->data.hw[isp_id].hw_ops->set_params(isp->data.hw[isp_id].hw_ops->ctrl,
                                                 ISP_HW_SET_PARAM_OVERFLOW_DETECTED,
                                                 &is_overflow, sizeof(is_overflow));
    if (rc < 0) {
      CDBG_ERROR("%s Error in HW HALT \n", __func__);
      return -1;
    }
    rc = isp->data.hw[isp_id].hw_ops->action(isp->data.hw[isp_id].hw_ops->ctrl,
                                             ISP_HW_ACTION_CODE_HALT, NULL, 0);
    if (rc < 0) {
      CDBG_ERROR("%s Error in HW HALT \n", __func__);
      return -1;
    }
  }
  if (session->vfe_ids & (1 << VFE1)) {
    isp_id = VFE1;
    rc = isp->data.hw[isp_id].hw_ops->set_params(isp->data.hw[isp_id].hw_ops->ctrl,
                                                 ISP_HW_SET_PARAM_OVERFLOW_DETECTED,
                                                 &is_overflow, sizeof(is_overflow));
    if (rc < 0) {
      CDBG_ERROR("%s Error in HW HALT \n", __func__);
      return -1;
    }
    rc = isp->data.hw[isp_id].hw_ops->action(isp->data.hw[isp_id].hw_ops->ctrl,
                                             ISP_HW_ACTION_CODE_HALT, NULL, 0);
    if (rc < 0) {
      CDBG_ERROR("%s Error in HW HALT \n", __func__);
      return -1;
    }
  }

  /* Find first stream associated with the session to send MCT event using its identity */
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (session->streams[i].session_id == *session_id) {
      stream = &session->streams[i];
      break;
    }
  }

  if (i == ISP_MAX_STREAMS) {
    CDBG_ERROR("%s Error no stream found for session %d\n", __func__, *session_id);
    return -1;
  }

  memset(&event, 0, sizeof(event));
  event.direction = MCT_EVENT_UPSTREAM;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = pack_identity(stream->session_id, stream->stream_id);
  event.u.module_event.type = MCT_EVENT_MODULE_ISPIF_RESET;
  event.u.module_event.module_event_data = NULL;
  rc = mct_port_send_event_to_peer(stream->sink_port, &event);

  if (rc < 0) {
    CDBG_ERROR("%s Error sending ISPIF reset to peer \n", __func__);
  }
  return rc;
}

int isp_util_proc_restart(isp_t *isp, uint32_t session_id, uint32_t *vfe_mask)
{
  int rc = 0, i;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  uint32_t is_overflow = 0;

  if (!session) {
    CDBG_ERROR("%s: cannot find session \n",__func__);
    return -1;
  }

  for (i = 0; i < VFE_MAX; i++) {
    if (*vfe_mask & (1 << i)) {
      CDBG("%s reset to vfe %d \n", __func__, i);
      rc = isp->data.hw[i].hw_ops->action(isp->data.hw[i].hw_ops->ctrl,
                                     ISP_HW_ACTION_CODE_RESET, &session->sof_frame_id,
                                     sizeof(session->sof_frame_id));
      if (rc < 0) {
        CDBG_ERROR("%s Error in HW Reset \n", __func__);
        return rc;
      }
    }
  }
  //reset the sof_id of current session.
   for (i = 0; i < VFE_MAX; i++)
     session->sof_id[i] = 0;

  /* Restart after all VFE are reset since the kernel will flush all buffers */
  for (i = 0; i < VFE_MAX; i++) {
    if (*vfe_mask & (1 << i)) {
      CDBG("%s restart to vfe %d \n", __func__, i);
      rc = isp->data.hw[i].hw_ops->action(isp->data.hw[i].hw_ops->ctrl,
                                     ISP_HW_ACTION_CODE_RESTART, NULL, 0);
      if (rc < 0) {
        CDBG_ERROR("%s Error in HW Restart \n", __func__);
        return rc;
      }
      rc = isp->data.hw[i].hw_ops->set_params(isp->data.hw[i].hw_ops->ctrl,
        ISP_HW_SET_PARAM_OVERFLOW_DETECTED, &is_overflow, sizeof(is_overflow));
      if (rc < 0) {
        CDBG_ERROR("%s Error in HW HALT \n", __func__);
        return -1;
      }
    }
  }

  return rc;
}


/** isp_util_update_zoom_roi_params
 *
 *  @isp: ISP pointer
 *  @hw_idx: ISP hw id
 *  @isp_hw_zoom_param_t: zoom params
 *
 *  Calculate zoom roi params and update at sof
 *
 * Return: 0 - success, negative vale - error.
 **/
static int isp_util_update_zoom_roi_params(isp_t *isp, int hw_idx,
  isp_hw_zoom_param_t *params)
{
  int rc = 0;
  int i =0;
  isp_session_t *session = isp_util_find_session(isp, params->session_id);
  boolean zoom_enable = FALSE;
  if (!session) {
    CDBG_ERROR("%s: cannot find params %x session %d\n",
      __func__, (unsigned int)params, params->session_id);
    return -1;
  }
  if (hw_idx >= ISP_MAX_HW) {
    /* hw_idx exceed the max value. Should not hit here. Just protection */
    CDBG_ERROR("%s: hw_idx %d exceeds max value %d\n",
      __func__, hw_idx, ISP_MAX_HW);
    return -1;
  }

  pthread_mutex_lock(
    &isp->data.session_critical_section[session->session_idx]);

  if (session->temp_zoom_roi_hw_id_mask == 0) {
    /* save ROI and hw_idx */
    session->temp_frame_id = params->frame_id;
    session->temp_zoom_roi_hw_id_mask = (1 << hw_idx);
    session->temp_zoom_params[hw_idx] = *params;
  } else if (params->frame_id != session->temp_frame_id) {
    /* this suppose to be the second roi, but it's not.
       drop the previous roi */
    session->temp_frame_id = params->frame_id;
    session->temp_zoom_roi_hw_id_mask = (1 << hw_idx);
    session->temp_zoom_params[hw_idx] = *params;
  } else {
    /* this is the second ROI received.
       No need to update frame_id for the second VFE  */
    session->temp_zoom_roi_hw_id_mask |= (1 << hw_idx);
    session->temp_zoom_params[hw_idx] = *params;
  }
  if(session->temp_zoom_roi_hw_id_mask == (int)session->vfe_ids) {
    /* now we received all VFE's ROI maps. In dual VFE case we need to
     * combine the width with the second VFE */
    if (session->temp_zoom_roi_hw_id_mask != (1 << hw_idx)) {
      int num_scalers = ISP_ZOOM_MAX_ENTRY_NUM;
      if (session->temp_zoom_params[0].num < num_scalers)
        num_scalers = session->temp_zoom_params[0].num;
      else {
        CDBG_ERROR("%s: number of scalers %d exceeds max scaler %d\n",
          __func__, session->temp_zoom_params[0].num, num_scalers);
        /* reset to max scalers */
        session->temp_zoom_params[0].num = num_scalers;
      }

      /* adjust the ROI width per scaler for dual VFE case */
      for (i = 0; i < num_scalers; i++) {
        session->temp_zoom_params[0].entry[i].roi_map_info.last_pixel +=
        session->temp_zoom_params[1].entry[i].roi_map_info.last_pixel -
        session->temp_zoom_params[1].entry[i].roi_map_info.first_pixel + 1;
      }

      /* save the ROI to ZOOM struct */
      isp_set_zoom_scaling_parm(session->zoom_session,
        &session->temp_zoom_params[0]);
    } else {
      /* single VFE case */
      isp_set_zoom_scaling_parm(session->zoom_session, params);
    }
    /* after we finishes reset the hw_id_mask to zero for the next frame */
    session->temp_zoom_roi_hw_id_mask = 0;
  }
  pthread_mutex_unlock(
    &isp->data.session_critical_section[session->session_idx]);
  return 0;
}


/** isp_core_hw_notify
 *    @parent: ISP pointer
 *    @handle: ISP driver index
 *    @type: notify type
 *    @notify_data: notify payload pointer
 *    @notify_data_size: notify payload data size
 *
 * Process ISP driver's notify event.
 *
 * Return: 0 - success, negative vale - error.
 **/
static int isp_util_hw_notify (void *parent,  uint32_t handle, uint32_t type,
  void *notify_data, uint32_t notify_data_size)
{
  int rc = 0;
  isp_t *isp = parent;
  isp_data_t *isp_data = &isp->data;
  int hw_idx = (int)handle;
  isp_data_hw_t *hw = &isp_data->hw[hw_idx];

  ISP_DBG(ISP_MOD_COM,"%s: E, type %d", __func__, type);
  switch (type) {
  case ISP_HW_NOTIFY_STATS: {
    rc = isp_util_hw_notify_stats(isp, notify_data, hw->hw_ops->ctrl);
  }
    break;

  case ISP_HW_NOTIFY_ISPIF_TO_RESET:
    rc = isp_util_notify_ispif_to_reset
      (isp, hw_idx, notify_data, notify_data_size);
    break;

  case ISP_HW_NOTIFY_CAMIF_SOF: {
    uint32_t isp_id = handle;
    rc = isp_ch_util_hw_notify_sof(isp, (mct_bus_msg_t *)notify_data,
      hw->hw_ops->ctrl, isp_id);
  }
    break;

  case ISP_HW_NOTIFY_META_VALID: {
    rc = isp_ch_util_hw_notify_meta_valid(isp, (mct_bus_msg_t *)notify_data);
  }
    break;

  case ISP_HW_NOTIFY_HW_UPDATING: {
    rc = isp_ch_util_hw_notify_hw_updating(isp,
      (isp_hw_updating_notify_t *)notify_data);
  }
    break;

  case ISP_HW_NOTIFY_BUF_DIVERT: {
    rc = isp_ch_util_buf_divert_notify(isp,
      (isp_frame_divert_notify_t *)notify_data);
  }
    break;

  case ISP_HW_NOTIFY_METADATA_INFO:
  case ISP_HW_NOTIFY_STATS_AWB_INFO: {
    rc = isp_util_hw_notify_stats_awb_info(isp, (mct_bus_msg_t *)notify_data);
  }
    break;

  case ISP_HW_NOTIFY_STATS_BE_CONFIG: {
    rc = isp_util_hw_notify_stats_be_config(isp, notify_data, notify_data_size);
  }
    break;

  case ISP_HW_NOTIFY_ROLLOFF_CONFIG: {
    rc = isp_util_hw_notify_rolloff_config(isp, notify_data, notify_data_size);
  }
    break;

  case ISP_HW_NOTIFY_BG_PCA_STATS_CONFIG: {
    rc = isp_util_hw_notify_stats_bg_config(isp, notify_data, notify_data_size);
  }
    break;

  case ISP_HW_NOTIFY_ROLLOFF_GET: {
    rc = isp_util_hw_notify_rolloff_get(isp, notify_data, notify_data_size);
  }
    break;

  case ISP_HW_NOTIFY_FETCH_HW_UPDATE_PARAMS: {
    rc = isp_util_fetch_hw_pending_update_params(isp,
           (isp_hw_pending_update_params_t *)notify_data);
  }
    break;

  case ISP_HW_NOTIFY_CUR_ROLLOFF: {
    rc = isp_util_hw_notify_cur_rolloff(isp, notify_data);
  }
    break;

  case ISP_HW_NOTIFY_ZOOM_ROI_PARAMS: {
    rc = isp_util_update_zoom_roi_params(isp, hw_idx,
           (isp_hw_zoom_param_t *)notify_data);
  }
    break;

  default: {
    CDBG_HIGH("%s: type %d not supported", __func__, type);
  }
    break;
  }
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return rc;
}

/** isp_util_destroy_hw
 *    @isp: ISP pointer
 *    @hw_idx: vfe hw index
 *    @num_streams: notify type
 *
 * TODO
 *
 * Return: nothing
 **/
void isp_util_destroy_hw(isp_t *isp, int hw_idx, int num_streams)
{
  ISP_DBG(ISP_MOD_COM,"%s: E, hw_idx %d, num_streams %d", __func__, hw_idx, num_streams);
  pthread_mutex_lock(&isp->data.hw[hw_idx].mutex);
  if (isp->data.hw[hw_idx].ref_cnt >= num_streams) {
    isp->data.hw[hw_idx].ref_cnt -= num_streams;
    if (isp->data.hw[hw_idx].ref_cnt > 0) {
      pthread_mutex_unlock(&isp->data.hw[hw_idx].mutex);
      return;
    }
    isp->data.hw[hw_idx].hw_ops->destroy(isp->data.hw[hw_idx].hw_ops->ctrl);
        isp->data.hw[hw_idx].hw_ops = NULL;
  } else
    ISP_DBG(ISP_MOD_COM,"%s: hw_idx = %d, ref_cnt = %d, num_streams = %d\n",
      __func__, hw_idx, isp->data.hw[hw_idx].ref_cnt, num_streams);
  pthread_mutex_unlock(&isp->data.hw[hw_idx].mutex);
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
}

/** isp_util_create_hw
 *    @isp: ISP pointer
 *    @hw_idx: vfe hw index
 *    @num_streams: notify type
 *
 * TODO
 *
 * Return: nothing
 **/
int isp_util_create_hw(isp_t *isp, int hw_idx, int num_streams)
{
  int rc = 0;
  isp_hw_init_params_t init_params;

  ISP_DBG(ISP_MOD_COM,"%s: E, hw_idx %d, num_streams %d", __func__, hw_idx, num_streams);
  pthread_mutex_lock(&isp->data.hw[hw_idx].mutex);
  if (isp->data.hw[hw_idx].ref_cnt > 0) {
    isp->data.hw[hw_idx].ref_cnt += num_streams;
    pthread_mutex_unlock(&isp->data.hw[hw_idx].mutex);
    return 0;
  }

  isp->data.hw[hw_idx].notify_ops.handle = hw_idx;
  isp->data.hw[hw_idx].notify_ops.parent = isp;
  isp->data.hw[hw_idx].notify_ops.notify = isp_util_hw_notify;
  isp->data.hw[hw_idx].hw_ops = isp_hw_create(
     isp->data.sd_info.sd_info[hw_idx].subdev_name);

  if (isp->data.hw[hw_idx].hw_ops == NULL) {
    rc = -1;
    CDBG_ERROR("%s: cannot create hw, dev_name = '%s'\n",
      __func__, isp->data.sd_info.sd_info[hw_idx].subdev_name);
    pthread_mutex_unlock(&isp->data.hw[hw_idx].mutex);
    return rc;
  }

  isp->data.hw[hw_idx].ref_cnt += num_streams;
  init_params.cap = isp->data.sd_info.sd_info[hw_idx].cap;
  init_params.isp_version = isp->data.sd_info.sd_info[hw_idx].isp_version;
  init_params.dev_idx = hw_idx;
  init_params.buf_mgr = &isp->data.buf_mgr;

  rc = isp->data.hw[hw_idx].hw_ops->init(
    isp->data.hw[hw_idx].hw_ops->ctrl, (void *)&init_params,
    &isp->data.hw[hw_idx].notify_ops);
  if (rc < 0) {
    CDBG_ERROR("%s: error in init, rc = %d\n", __func__, rc);
    pthread_mutex_unlock(&isp->data.hw[hw_idx].mutex);
    isp_util_destroy_hw(isp, hw_idx, num_streams);
  }
  pthread_mutex_unlock(&isp->data.hw[hw_idx].mutex);
  ISP_DBG(ISP_MOD_COM,"%s: X, rc %d", __func__, rc);
  return rc;
}

/** isp_util_gen_hws_caps
 *    @isp: ISP pointer
 *    @hw_idx: vfe hw index
 *    @num_streams: notify type
 *
 * TODO
 *
 * Return: nothing
 **/
int isp_util_gen_hws_caps(isp_t *isp)
{
  int sd_num;
  int rc = 0;
  isp_hw_cap_t hw_cap;
  uint32_t action_code, params_id;

  ISP_DBG(ISP_MOD_COM,"%s: E", __func__);
  isp_util_discover_subdev_nodes(isp);
  CDBG_ERROR("%s: integrating kernel vfe is pending\n", __func__);
  for (sd_num = 0; sd_num < isp->data.sd_info.num; sd_num++) {
    rc = isp_hw_query_caps(isp->data.sd_info.sd_info[sd_num].subdev_name,
      &isp->data.sd_info.sd_info[sd_num].isp_version,
      &isp->data.sd_info.sd_info[sd_num].cap, sd_num);
  }
  ISP_DBG(ISP_MOD_COM,"%s: X,rc %d", __func__, rc);
  return rc;
}

/** isp_util_find_stream
 *    @isp: ISP pointer
 *    @session_id: session_id
 *    @stream_id: stream_id
 *
 * TODO
 *
 * Return: pointer to isp streamobject
 **/
isp_stream_t *isp_util_find_stream(isp_t *isp, uint32_t session_id,
  uint32_t stream_id)
{
  int i, k;

  ISP_DBG(ISP_MOD_COM,"%s: E, session_id %d, stream_id %d", __func__, session_id, stream_id);
  for (i = 0; i < ISP_MAX_SESSIONS; i++) {
    if (isp->data.sessions[i].isp_data &&
        isp->data.sessions[i].session_id == session_id) {
      /* find the session */
      for (k = 0; k < ISP_MAX_STREAMS; k++) {
        if (isp->data.sessions[i].streams[k].session &&
            isp->data.sessions[i].streams[k].stream_id == stream_id) {
          return &isp->data.sessions[i].streams[k];
        }
      }
    }
  }
  CDBG_ERROR("%s: X NULL", __func__);
  return NULL;
}

/** isp_util_find_sink_port_by_caps
 *    @port_data: ISP pointer
 *    @user_data: session_id
 *
 * TODO
 *
 * Return: TRUE - succes and FALSE - failure
 **/
static boolean isp_util_find_sink_port_by_caps(void *port_data, void *user_data)
{
  ispif_src_port_caps_t *caps = (ispif_src_port_caps_t *)user_data;
  mct_port_t *mct_port = (mct_port_t *)port_data;
  isp_port_t *isp_sink_port = (isp_port_t * )mct_port->port_private;
  isp_sink_port_t *sink_port = &isp_sink_port->u.sink_port;

  ISP_DBG(ISP_MOD_COM,"%s: E port state %d", __func__, isp_sink_port->state);
  if (isp_sink_port->state != ISP_PORT_STATE_CREATED &&
      memcmp(caps, &sink_port->caps, sizeof(ispif_src_port_caps_t)) == 0) {
      /* has the match */
      ISP_DBG(ISP_MOD_COM,"%s: X has the match", __func__);
      return TRUE;
  } else {
    ISP_DBG(ISP_MOD_COM,"%s: X no match found", __func__);
    return FALSE;
  }
}

/** isp_util_find_sink_port
 *    @isp: isp object
 *    @ispif_src_cap: ispif source caps
 *
 * TODO
 *
 * Return: pointer to isp sink port object
 **/
isp_port_t *isp_util_find_sink_port(isp_t *isp,
  ispif_src_port_caps_t *ispif_src_cap)
{
  mct_list_t *sink_port_list = NULL;
  mct_port_t *mct_port = NULL;
  isp_port_t *isp_sink_port = NULL;

  ISP_DBG(ISP_MOD_COM,"%s: E", __func__);
  sink_port_list = mct_list_find_custom (isp->module->sinkports,
    (void *)ispif_src_cap, isp_util_find_sink_port_by_caps);
  if (sink_port_list != NULL) {
    mct_port = (mct_port_t *)sink_port_list->data;
    isp_sink_port = (isp_port_t *)mct_port->port_private;
  }
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return isp_sink_port;
}

/** isp_util_add_stream
 *    @isp: ISP pointer
 *    @stream_info: mct streamo bject
 *
 * TODO
 *
 * Return: pointer to isp sink port object
 **/
isp_stream_t *isp_util_add_stream(isp_t *isp, uint32_t session_id,
  uint32_t stream_id, mct_stream_info_t *stream_info)
{
  int i, rc = 0, is_new_session = 1;
  isp_session_t *session = NULL;
  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_d %d, stream_id %d", __func__, (void *)isp,
    session_id, stream_id);
  for (i = 0; i < ISP_MAX_SESSIONS; i++) {
    if (isp->data.sessions[i].isp_data &&
        isp->data.sessions[i].session_id == session_id) {
      /* the session already in use. */
      session = &isp->data.sessions[i];
      break;
    }
  }

  if (session == NULL) {
    CDBG_ERROR("%s: no more session availabe, max = %d\n", __func__,
      ISP_MAX_SESSIONS);
    return NULL;
  }

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (session->streams[i].session == NULL) {
      /* found an empty slot */
      isp_channel_t *channel = NULL;
      memset(&session->streams[i], 0, sizeof(session->streams[i]));
      session->streams[i].session = (void *)session;
      session->streams[i].session_id = session_id;
      session->streams[i].stream_id = stream_id;
      session->streams[i].stream_info = *stream_info;
      session->streams[i].state = ISP_STREAM_STATE_CREATED;
      session->streams[i].stream_idx = i;
      ISP_DBG(ISP_MOD_COM,"%s: i = %d, sessid = %d, streamid = %d, stream = %p\n",
        __func__, i, session->streams[i].session_id,
        session->streams[i].stream_id, &session->streams[i]);
#if 0
      channel = isp_ch_util_add_channel(isp,
        session_id, stream_id, session->streams[i].stream_idx, stream_info,
        ISP_CHANNEL_TYPE_IMAGE);
      if (channel == NULL) {
        memset(&session->streams[i], 0, sizeof(session->streams[i]));
        CDBG_ERROR("%s: no HW stream slot available\n", __func__);
        return NULL;
      }
#endif
      session->num_stream++;
      /* ZOOM only apply to VFE outputs going through VFE pipeline */
      if (isp_util_need_zoom(session, &session->streams[i]))
        session->zoom_stream_cnt++;
#if 0
      session->streams[i].channel_idx_mask |=
        (1 << isp_ch_util_get_channel_idx(channel));
#endif
      return &session->streams[i];
    }
  }
  CDBG_ERROR("%s: no more stream slot in that session\n", __func__);
  return NULL;
}

/** isp_util_add_stream_to_sink_port
 *    @isp: ISP pointer
 *    @isp_sink_port: isp sink port object
 *
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_add_stream_to_sink_port(isp_t *isp, isp_port_t *isp_sink_port,
  isp_stream_t *stream)
{
  int i;
  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p", __func__, (void *)isp);
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (isp_sink_port->u.sink_port.streams[i] == NULL) {
      isp_sink_port->u.sink_port.streams[i] =  stream;
      stream->sink_port = isp_sink_port->port;
      stream->state = ISP_STREAM_STATE_ASSOCIATED_WITH_SINK_PORT;
      isp_sink_port->u.sink_port.num_streams++;
      stream->link_cnt++;
      ISP_DBG(ISP_MOD_COM,"%s: link_cnt = %d\n", __func__, stream->link_cnt);
      return 0;
    }
  }
  CDBG_ERROR("%s: cannot find empty slot to add the stream\n", __func__);
  return -1;
}

/** isp_util_del_stream_from_sink_port
 *    @isp: ISP pointer
 *    @isp_sink_port: isp sink port object
 *
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_del_stream_from_sink_port(isp_t *isp, isp_port_t *isp_sink_port,
   isp_stream_t *stream)
{
  int i;
  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p", __func__, (void *)isp);
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (isp_sink_port->u.sink_port.streams[i] == stream) {
      stream->link_cnt--;
      isp_sink_port->u.sink_port.streams[i] = NULL;
      isp_sink_port->u.sink_port.num_streams--;
      ISP_DBG(ISP_MOD_COM,"%s: link_cnt = %d\n", __func__, stream->link_cnt);
      return 0;
    }
  }
  CDBG_ERROR("%s: cannot find stream (%d, %d)\n", __func__, stream->session_id,
    stream->stream_id);
  return -1;
}

/** isp_util_add_stream_to_src_port
 *    @isp: ISP pointer
 *    @isp_src_port: isp source port object
 *
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_add_stream_to_src_port(isp_t *isp, isp_port_t *isp_src_port,
   isp_stream_t *stream)
{
  int i;
  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p", __func__, (void *)isp);
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (isp_src_port->u.src_port.streams[i] == NULL) {
      isp_src_port->u.src_port.streams[i] = stream;
      isp_src_port->u.src_port.num_streams++;
      stream->link_cnt++;
      ISP_DBG(ISP_MOD_COM,"%s: sessionid = %d, streamid = %d, num_straem = %d,"
        "stream_link_cnt = %d\n", __func__, stream->session_id,
        stream->stream_id, isp_src_port->u.src_port.num_streams,
        stream->link_cnt);
      return 0;
    }
  }
  CDBG_ERROR("%s: cannot find empty slot to add the stream\n", __func__);
  return -1;
}

/** isp_util_del_stream_from_src_port
 *    @isp: ISP pointer
 *    @isp_src_port: isp source port object
 *
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_del_stream_from_src_port(isp_t *isp, isp_port_t *isp_src_port,
   isp_stream_t *stream)
{
  int i;
  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p,", __func__, (void *)isp);
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (isp_src_port->u.src_port.streams[i] == stream) {
      stream->link_cnt--;
      isp_src_port->u.src_port.streams[i] = NULL;
      isp_src_port->u.src_port.num_streams--;
      ISP_DBG(ISP_MOD_COM,"%s: link_cnt = %d\n", __func__, stream->link_cnt);
      return 0;
    }
  }
  CDBG_ERROR("%s: cannot find stream (%d, %d)\n", __func__, stream->session_id,
    stream->stream_id);
  return -1;
}

/** isp_util_del_stream
 *    @isp: ISP pointer
 *    @isp_src_port: isp source port object
 *
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_del_stream(isp_t *isp, isp_stream_t *stream)
{
  int i, rc = 0, is_new_session = 1;
  isp_session_t *session = (isp_session_t *)stream->session;
  isp_stream_t *tmp_stream;
  ISP_DBG(ISP_MOD_COM,"%s: stream = %p, sessionid = %d, streamid = %d\n",
    __func__, stream, stream->session_id, stream->stream_id);

  if (stream->link_cnt > 0) {
    CDBG_ERROR("%s: stream used by sink/src port, link_cnt = %d,error\n",
      __func__, stream->link_cnt);
    return -1;
  }

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    tmp_stream = &session->streams[i];
    if (tmp_stream == stream) {
      ISP_DBG(ISP_MOD_COM,"%s: found stream - stream = %p, sessionid = %d, "
        "streamid = %d, channel_idx_mask = 0x%x\n", __func__, tmp_stream,
        tmp_stream->session_id, tmp_stream->stream_id,
        tmp_stream->channel_idx_mask);

      if (tmp_stream->channel_idx_mask > 0) {
        /* this is defensive code to avoid mistake. When we enter here
         * we need to debug the root cause since the channel should be
         * deleted when the last straemoff happens. */
        CDBG_ERROR("%s: error, identity = 0x%x, channel_idx_mask = 0x%x\n",
          __func__, tmp_stream->stream_info.identity,
          tmp_stream->channel_idx_mask);
        isp_ch_util_del_channel_by_mask(session, tmp_stream->channel_idx_mask);
      }

      memset(stream, 0, sizeof(isp_stream_t));
      session->num_stream--;
      /* ZOOM only apply to VFE outputs going through VFE pipeline */
      if (isp_util_need_zoom(session, &session->streams[i]))
        session->zoom_stream_cnt--;
      break;
    }
  }
  ISP_DBG(ISP_MOD_COM,"%s: X", __func__);
  return 0;
}

/** isp_util_find_stream_from_sink_port
 *    @isp: ISP pointer
 *    @isp_sink_port: isp sink port object
 *
 * Find stream from sink matching with streamid passed
 *
 * Return: 0 - success and negative value - failure
 **/
isp_stream_t *isp_util_find_stream_from_sink_port(isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id)
{
  int i;
  isp_stream_t *stream;
  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_d %d, stream_id %d", __func__,
    (void *)isp_sink_port, session_id, stream_id);
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    stream = isp_sink_port->u.sink_port.streams[i];
    if (stream->session_id == session_id &&
        stream->stream_id == stream_id) {
      ISP_DBG(ISP_MOD_COM,"%s: X, Found stream", __func__);
      return stream;
    }
  }
  CDBG_ERROR("%s: cannot find stream (%d, %d)\n",
             __func__, stream->session_id, stream->stream_id);
  return NULL;
}

/** isp_util_find_stream_from_src_port
 *    @isp: ISP pointer
 *    @isp_src_port:isp source port object
 *
 * Find stream from source matching with streamid passed
 *
 * Return: 0 - success and negative value - failure
 **/
isp_stream_t *isp_util_find_stream_from_src_port(isp_port_t *isp_src_port,
  uint32_t session_id, uint32_t stream_id)
{
  int i;
  isp_stream_t *stream;
  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_d %d, stream_id %d", __func__,
   (void *)isp_src_port, session_id, stream_id);
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    stream = isp_src_port->u.src_port.streams[i];
    if (stream->session_id == session_id &&
        stream->stream_id == stream_id) {
      ISP_DBG(ISP_MOD_COM,"%s: X, Found stream", __func__);
      return stream;
    }
  }
  CDBG_ERROR("%s: cannot find stream (%d, %d)\n", __func__, stream->session_id,
    stream->stream_id);
  return NULL;
}

/** isp_util_need_pix
 *    @isp: ISP pointer
 *    @vfe_output_mask: vfe output mask
 *
 * Check vfe output mask for both vfes if pix interface is
 * enabled
 *
 * Return: TRUE - success and FALSE - failure
 **/
boolean isp_util_need_pix(isp_t *isp, uint32_t vfe_output_mask)
{
  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, vfe_output_mask %x", __func__, (void *)isp,
       vfe_output_mask);
  if (vfe_output_mask & (1 << ISP_INTF_PIX)) {
    ISP_DBG(ISP_MOD_COM,"%s: VFE 0", __func__);
    return TRUE; /* VFE0 */
  } else if (vfe_output_mask & (1 << (16 + ISP_INTF_PIX))) {
    ISP_DBG(ISP_MOD_COM,"%s: VFE 1", __func__);
    return TRUE; /* VFE1 */
  } else {
    CDBG_HIGH("%s: pix intf not needed", __func__);
    return FALSE;
  }
}

/** isp_util_find_session
 *    @isp: ISP pointer
 *    @vfe_output_mask: vfe output mask
 *
 * In all ISP sessions find the the one matching with session id
 * passed
 *
 * Return: pointer to isp session, NULL for failure
 **/
isp_session_t *isp_util_find_session(isp_t *isp, uint32_t session_id)
{
  int i;
  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p, session_id %x", __func__, (void *)isp,
       session_id);
  for (i = 0; i < ISP_MAX_SESSIONS; i++) {
    if (isp->data.sessions[i].isp_data &&
        isp->data.sessions[i].session_id == session_id) {
      return &isp->data.sessions[i];
    }
  }
  CDBG_ERROR("%s: X, session %d not found", __func__, session_id);
  return NULL;
}

/** isp_util_is_stream_in_sink_port
 *    @isp: ISP pointer
 *    @isp_sink_port: isp sink port object
 *
 * In all sink port streams check if stream passed is present
 *
 * Return: TRUE if stream is present in sink port, FALSE if not
 **/
boolean isp_util_is_stream_in_sink_port(isp_t *isp, isp_port_t *isp_sink_port,
   isp_stream_t *stream)
{
  int i;
  ISP_DBG(ISP_MOD_COM,"%s: E, isp %p", __func__, (void *)isp);
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (isp_sink_port->u.sink_port.streams[i] == stream) {
      return TRUE;
    }
  }
  CDBG_HIGH("%s: X, stream not present in sink port", __func__);
  return FALSE;
}

/** isp_util_find_stream_in_session
 *    @sess: ISP session object
 *    @stream_id: stream_id
 *
 * Find the stream from all current session streams the one
 * matching passed stream id
 *
 * Return: pointer to isp stream if found in session, NULL on
 * failure
 **/
isp_stream_t * isp_util_find_stream_in_session(isp_session_t *sess,
  uint32_t stream_id)
{
  int i;
  ISP_DBG(ISP_MOD_COM,"%s: E, stream_id %d", __func__, stream_id);
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (sess->streams[i].stream_id == stream_id &&
        sess->streams[i].state != ISP_STREAM_STATE_INITIAL) {
      return &sess->streams[i];
    }
  }
  CDBG_HIGH("%s: X, stream not present in session", __func__);
  return NULL;
}

/** isp_util_find_3a_stream
 *    @sess: ISP session object
 *
 * Frmo all session streams find 3a stream
 *
 * Return: pointer to isp stream if found in session, NULL on
 * failure
 **/
isp_stream_t * isp_util_find_3a_stream(isp_session_t *sess)
{
  int i;
  ISP_DBG(ISP_MOD_COM,"%s: E", __func__);
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (sess->streams[i].src_ports[ISP_SRC_PORT_3A] != NULL &&
        sess->streams[i].state != ISP_STREAM_STATE_INITIAL) {
      return &sess->streams[i];
    }
  }
  CDBG_HIGH("%s: X, stream not present", __func__);
  return NULL;
}


/** isp_util_find_matched_src_stats_port_by_caps
 *    @port_data: mct port pbject
 *    @userdata : isp_src_port_match_param_t
 *
 * TODO
 *
 * Return: TRUE on success, FLASE if matched port not found
 **/
static boolean isp_util_find_matched_src_stats_port_by_caps(void *port_data,
  void *userdata)
{
  mct_port_t *mct_port = (mct_port_t *)port_data;
  isp_src_port_match_param_t *match_param = userdata;
  ispif_src_port_caps_t *caps = match_param->caps;
  isp_stream_t *stream = match_param->stream;
  isp_port_t *isp_src_port = (isp_port_t * )mct_port->port_private;
  isp_src_port_t *src_port = &isp_src_port->u.src_port;

  if (isp_src_port->port->caps.port_caps_type != MCT_PORT_CAPS_STATS)
    return FALSE;
  if (isp_src_port->state != ISP_PORT_STATE_CREATED &&
      memcmp(&caps->sensor_cap, &src_port->caps.sensor_cap, sizeof(sensor_src_port_cap_t)) == 0) {
        /* isp sink port caps matches so this src port is for the stream */
        return TRUE;
  }
  return FALSE;
}

/** isp_util_get_matched_src_port
 *    @isp: mct port pbject
 *    @cap_type : mct_port_caps_type_t
 *    @isp_sink_port: isp sink port object
 *    @stream: isp stream pointer
 *
 * TODO
 *
 * Return: macthing source port object or NULL on if not found
 **/
isp_port_t *isp_util_get_matched_src_port(isp_t *isp,
  mct_port_caps_type_t cap_type, isp_port_t *isp_sink_port,
  isp_stream_t *stream)
{
  int i, rc = 0;
  isp_sink_port_t *sink_port = &isp_sink_port->u.sink_port;
  isp_src_port_t *src_port = NULL;
  isp_port_t *isp_src_port = NULL;
  mct_list_t *src_port_list = NULL;
  mct_port_t *mct_port = NULL;
  isp_src_port_match_param_t match_param;

  memset(&match_param, 0, sizeof(match_param));
  match_param.caps = &sink_port->caps;
  match_param.stream = stream;
  if (cap_type == MCT_PORT_CAPS_FRAME) {
    mct_port = isp_util_find_matched_existing_src_port(isp,
      isp_sink_port, cap_type, stream);
    if (mct_port) {
      /* found a match. Reuse the matched src port */
      isp_src_port = (isp_port_t *)mct_port->port_private;
      return isp_src_port;
    }
  } else {
    src_port_list = mct_list_find_custom (isp->module->srcports,
      (void *)&match_param, isp_util_find_matched_src_stats_port_by_caps);
    if (src_port_list) {
      mct_port = (mct_port_t *)src_port_list->data;
      isp_src_port = (isp_port_t *)mct_port->port_private;
      return isp_src_port;
    }
  }

  return NULL;
}

/** isp_util_gen_init_stats_cfg
 *    @isp_session: isp session object
 *    @stream: isp stream pointer
 *
 * TODO
 *
 * Return: 0 - success and negative value on failure
 **/
int isp_util_gen_init_stats_cfg(isp_session_t *isp_session,
  isp_stream_t *stream)
{
  int rc = 0;

  uint32_t af_window_width;
  uint32_t af_window_height;

  uint32_t camif_window_width = stream->cfg.sensor_cfg.request_crop.last_pixel -
    stream->cfg.sensor_cfg.request_crop.first_pixel + 1;
  uint32_t camif_window_height = stream->cfg.sensor_cfg.request_crop.last_line -
    stream->cfg.sensor_cfg.request_crop.first_line + 1;
  uint32_t out_width = stream->stream_info.dim.width;
  uint32_t out_height = stream->stream_info.dim.height;

  aec_config_t *aec_init_cfg = &isp_session->stats_config.aec_config;
  awb_config_t *awb_init_cfg = &isp_session->stats_config.awb_config;
  af_config_t *af_init_cfg = &isp_session->stats_config.af_config;

  /* awb aec default setting whole window*/
  aec_init_cfg->bg_config.grid_info.h_num = 64;
  aec_init_cfg->bg_config.grid_info.v_num = 48;
  aec_init_cfg->bg_config.roi.left = camif_window_width % 64;
  aec_init_cfg->bg_config.roi.top = camif_window_height % 48;
  aec_init_cfg->bg_config.roi.width = camif_window_width;
  aec_init_cfg->bg_config.roi.height = camif_window_height;
  aec_init_cfg->bg_config.r_Max = 255 -16;
  aec_init_cfg->bg_config.gr_Max = 255 -16;
  aec_init_cfg->bg_config.b_Max = 255 -16;
  aec_init_cfg->bg_config.gb_Max = 255 -16;

  aec_init_cfg->bhist_config.grid_info.h_num = 64;
  aec_init_cfg->bhist_config.grid_info.v_num = 48;
  aec_init_cfg->bhist_config.roi.left = camif_window_width % 64;
  aec_init_cfg->bhist_config.roi.top = camif_window_height % 48;
  aec_init_cfg->bhist_config.roi.width = camif_window_width;
  aec_init_cfg->bhist_config.roi.height = camif_window_height;

  awb_init_cfg->grid_info.h_num = 64;
  awb_init_cfg->grid_info.v_num = 48;
  awb_init_cfg->roi.left = camif_window_width % 64;
  awb_init_cfg->roi.top = camif_window_height % 48;
  awb_init_cfg->roi.width = camif_window_width;
  awb_init_cfg->roi.height = camif_window_height;


  /* af will convert roi to hw cfg, roi based on output dimension*/
  af_window_width = camif_window_width /4;
  af_window_height = camif_window_height /4;

  af_init_cfg->grid_info.h_num = 5;
  af_init_cfg->grid_info.v_num = 5;
  af_init_cfg->roi.left = out_width /4;
  af_init_cfg->roi.top = out_height /4;
  af_init_cfg->roi.width = af_window_width;
  af_init_cfg->roi.height = af_window_height;
  af_init_cfg->r_min = 31;
  af_init_cfg->b_min = 31;
  af_init_cfg->gr_min = 31;
  af_init_cfg->gb_min = 31;
  af_init_cfg->hpf[0] = -4;
  af_init_cfg->hpf[1] = 0;
  af_init_cfg->hpf[2] = -2;
  af_init_cfg->hpf[3] = 0;
  af_init_cfg->hpf[4] = -4;
  af_init_cfg->hpf[5] = -1;
  af_init_cfg->hpf[6] = -1;
  af_init_cfg->hpf[7] = 14;
  af_init_cfg->hpf[8] = -1;
  af_init_cfg->hpf[9] = -1;

  return rc;
}

/** isp_util_check_stream_aspect_ratio
 *    @isp: isp session object
 *    @stream1: isp stream pointer
 *    @stream2: isp stream pointer
 *
 * TODO
 *
 * Return: 0 - success and negative value on failure
 **/
int isp_util_check_stream_aspect_ratio(isp_t *isp, isp_stream_t *stream1,
  isp_stream_t *stream2)
{
  uint32_t tmp1, tmp2;
  int isp_id;
  int two_aspect_ratios = 0;

  if (stream1->cfg.vfe_mask & (1 << VFE0))
    isp_id = VFE0;
  else
    isp_id = VFE1;

  if (GET_ISP_MAIN_VERSION(isp->data.sd_info.sd_info[isp_id].isp_version) ==
      ISP_VERSION_40)
    two_aspect_ratios = 1; /* VFE40 supports two aspect ratios */

  tmp1 = stream1->stream_info.dim.width * stream2->stream_info.dim.height;
  tmp2 = stream1->stream_info.dim.height * stream2->stream_info.dim.width;
  if (!two_aspect_ratios && tmp1 != tmp2) {
    /* aspect ratio mismatch */
    /* We will return -ERANGE once we have C2D support for croping,
       For now to make CTS work we return 0 */
    return 0;
  } else
    return 0;
}

/** isp_util_is_burst_streaming
 *    @session: isp session object
 *
 * Check if stream has burst streaming mode
 *
 * Return: TRUE - success and FALSE - failure
 **/
boolean isp_util_is_burst_streaming(isp_session_t *session)
{
  boolean burst_streaming = TRUE;
  int i;
  isp_stream_t *stream;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    stream = &session->streams[i];
    if (stream->sink_port == NULL)
      continue;
    if (stream->stream_info.streaming_mode == CAM_STREAMING_MODE_CONTINUOUS) {
      burst_streaming= FALSE;
      break;
    }
  }
  return burst_streaming;
}

/** isp_util_find_burst_stream_in_continuous_mode
 *    @session: isp session object
 *    @stream_ptr:
 *
 * From tje streams of current session, check the stream with
 * Burst streaming mode
 *
 * Return: 0 - success and -1 - failure
 **/
static int isp_util_find_burst_stream_in_continuous_mode(isp_session_t *session,
  isp_stream_t **stream_ptr)
{
  int i;
  int burst_cnt = 0;

  *stream_ptr = NULL;
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (session->streams[i].sink_port == NULL)
      continue;
    if (session->streams[i].stream_info.streaming_mode ==
        CAM_STREAMING_MODE_BURST) {
      *stream_ptr = &session->streams[i];
      burst_cnt++;
    }
  }
  if (burst_cnt <= 1)
    return 0;
  else {
    *stream_ptr = NULL;
    CDBG_ERROR("%s: Error, only allow one burst stream, cnt = %d\n",
      __func__, burst_cnt);
    return -1;
  }
}

/** isp_util_get_pipeline_streams_by_mode
 *    @session: isp session object
 *    @mode:
 * TODO
 *
 * Return: 0 - success and -1 - failure
 **/
static int isp_util_get_pipeline_streams_by_mode(isp_session_t *session,
  isp_stream_t *streams[ISP_MAX_STREAMS], cam_streaming_mode_t mode)
{
  int i;
  isp_stream_t *stream;
  isp_port_t *isp_sink_port;
  int cnt = 0;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    stream = &session->streams[i];
    if (session->streams[i].sink_port == NULL)
      continue;

    isp_sink_port = stream->sink_port->port_private;
    if (!isp_sink_port->u.sink_port.caps.use_pix)
      /* this is RDI stream. */
      continue;

    if (stream->stream_info.streaming_mode == mode)
      streams[cnt++] = stream;
  }

  return cnt;
}

/** isp_util_get_count_pp_linked_streams
 *    @num_streams:
 *    @mode:
 * TODO
 *
 * Return: 0 - success and -1 - failure
 **/
static int isp_util_get_count_pp_linked_streams(int num_streams,
  isp_stream_t **streams)
{
  int i, cnt = 0;

  for (i = 0; i < num_streams; i++)
    if (streams[i] && streams[i]->src_ports[ISP_SRC_PORT_DATA])
      cnt++;
  return cnt;
}

/** isp_util_modify_stream_plane_info_for_native_buf
 *    @num_streams:
 *    @mode:
 * TODO
 *
 * Return: 0 - success and -1 - failure
 **/
static int isp_util_modify_stream_plane_info_for_native_buf(
  isp_stream_t *stream)
{
  cam_stream_buf_plane_info_t *buf_planes = &stream->stream_info.buf_planes;
  cam_dimension_t *dim = &stream->stream_info.dim;
  int stride = 0, scanline = 0;
  int rc = 0;

  memset(buf_planes, 0, sizeof(cam_stream_buf_plane_info_t));

  switch (stream->stream_info.fmt) {
  case CAM_FORMAT_YUV_420_NV12_VENUS:
  case CAM_FORMAT_YUV_420_NV12:
  case CAM_FORMAT_YUV_420_NV21: {
    /* 2 planes: Y + CbCr */
    buf_planes->plane_info.num_planes = 2;
    stride = PAD_TO_SIZE(dim->width, CAM_PAD_TO_16);
    scanline = PAD_TO_SIZE(dim->height, CAM_PAD_TO_2);
    buf_planes->plane_info.mp[0].offset = 0;
    buf_planes->plane_info.mp[0].len = stride * scanline;
    buf_planes->plane_info.mp[0].stride = stride;
    buf_planes->plane_info.mp[0].scanline = scanline;

    stride = PAD_TO_SIZE(dim->width, CAM_PAD_TO_16);
    scanline = PAD_TO_SIZE(dim->height / 2, CAM_PAD_TO_2);
    buf_planes->plane_info.mp[1].offset =
      buf_planes->plane_info.mp[0].len;
    buf_planes->plane_info.mp[1].len = stride * scanline;
    buf_planes->plane_info.mp[1].stride = stride;
    buf_planes->plane_info.mp[1].scanline = scanline;

    buf_planes->plane_info.frame_len =
      PAD_TO_SIZE(buf_planes->plane_info.mp[0].len +
      buf_planes->plane_info.mp[1].len,
      CAM_PAD_TO_4K);
  }
    break;

  case CAM_FORMAT_YUV_420_NV21_ADRENO: {
    /* 2 planes: Y + CbCr */
    buf_planes->plane_info.num_planes = 2;
    stride = PAD_TO_SIZE(dim->width, CAM_PAD_TO_32);
    scanline = PAD_TO_SIZE(dim->height, CAM_PAD_TO_32);
    buf_planes->plane_info.mp[0].offset = 0;
    buf_planes->plane_info.mp[0].len =
      PAD_TO_SIZE(stride * scanline, CAM_PAD_TO_4K);
    buf_planes->plane_info.mp[0].stride = stride;
    buf_planes->plane_info.mp[0].scanline = scanline;

    stride = PAD_TO_SIZE(dim->width / 2, CAM_PAD_TO_32) * 2;
    scanline = PAD_TO_SIZE(dim->height / 2, CAM_PAD_TO_32);
    buf_planes->plane_info.mp[1].offset =
      buf_planes->plane_info.mp[0].len;
    buf_planes->plane_info.mp[1].len =
      PAD_TO_SIZE(stride * scanline, CAM_PAD_TO_4K);
    buf_planes->plane_info.mp[1].stride = stride;
    buf_planes->plane_info.mp[1].scanline = scanline;

    buf_planes->plane_info.frame_len =
      PAD_TO_SIZE(buf_planes->plane_info.mp[0].len +
      buf_planes->plane_info.mp[1].len, CAM_PAD_TO_4K);
  }
    break;

  case CAM_FORMAT_YUV_420_YV12: {
    /* 3 planes: Y + Cr + Cb */
    buf_planes->plane_info.num_planes = 3;
    stride = PAD_TO_SIZE(dim->width, CAM_PAD_TO_16);
    scanline = PAD_TO_SIZE(dim->height, CAM_PAD_TO_2);
    buf_planes->plane_info.mp[0].offset = 0;
    buf_planes->plane_info.mp[0].len = stride * scanline;
    buf_planes->plane_info.mp[0].stride = stride;
    buf_planes->plane_info.mp[0].scanline = scanline;

    stride = PAD_TO_SIZE(stride / 2, CAM_PAD_TO_16);
    scanline = scanline / 2;
    buf_planes->plane_info.mp[1].offset =
      buf_planes->plane_info.mp[0].len;
    buf_planes->plane_info.mp[1].len = stride * scanline;
    buf_planes->plane_info.mp[1].stride = stride;
    buf_planes->plane_info.mp[1].scanline = scanline;

    buf_planes->plane_info.mp[2].offset =
      buf_planes->plane_info.mp[1].offset +
      buf_planes->plane_info.mp[1].len;
    buf_planes->plane_info.mp[2].len = stride * scanline;
    buf_planes->plane_info.mp[2].stride = stride;
    buf_planes->plane_info.mp[2].scanline = scanline;

    buf_planes->plane_info.frame_len =
      PAD_TO_SIZE(buf_planes->plane_info.mp[0].len +
      buf_planes->plane_info.mp[1].len +
      buf_planes->plane_info.mp[2].len, CAM_PAD_TO_4K);
  }
    break;

  case CAM_FORMAT_YUV_422_NV16:
  case CAM_FORMAT_YUV_422_NV61: {
    /* 2 planes: Y + CbCr */
    buf_planes->plane_info.num_planes = 2;

    stride = PAD_TO_SIZE(dim->width, CAM_PAD_TO_16);
    scanline = dim->height;
    buf_planes->plane_info.mp[0].offset = 0;
    buf_planes->plane_info.mp[0].len = stride * scanline;
    buf_planes->plane_info.mp[0].stride = stride;
    buf_planes->plane_info.mp[0].scanline = scanline;

    buf_planes->plane_info.mp[1].offset =
      buf_planes->plane_info.mp[0].len;
    buf_planes->plane_info.mp[1].len = stride * scanline;
    buf_planes->plane_info.mp[1].stride = stride;
    buf_planes->plane_info.mp[1].scanline = scanline;

    buf_planes->plane_info.frame_len =
      PAD_TO_SIZE(buf_planes->plane_info.mp[0].len +
      buf_planes->plane_info.mp[1].len, CAM_PAD_TO_4K);
  }
    break;

  default: {
    CDBG_ERROR("%s: Invalid cam_format for preview %d",
      __func__, stream->stream_info.fmt);
    rc = -1;
  }
    break;
  }

  return rc;
}

/** isp_util_dump_stream_planes
 *    @stream:
 * TODO
 *
 * Return: 0 - success and -1 - failure
 **/
void isp_util_dump_stream_planes(isp_stream_t *stream)
{
  cam_stream_buf_plane_info_t *buf_planes = &stream->stream_info.buf_planes;
  cam_dimension_t *dim = &stream->stream_info.dim;
  uint32_t i;

  ISP_DBG(ISP_MOD_COM,"%s: sess_id = %d, stream_id = %d, "
    "width = %d, height = %d, fmt = %d, num_planes = %d\n",
    __func__, stream->session_id, stream->stream_id,
    dim->width, dim->height, stream->stream_info.fmt,
    buf_planes->plane_info.num_planes);

  for (i = 0; i < buf_planes->plane_info.num_planes; i++) {
    ISP_DBG(ISP_MOD_COM,"%s: sess_id = %d, stream_id = %d, idx = %d,"
      "stride = %d, scanline = %d, len = %d, offset = %d\n",
      __func__, stream->session_id, stream->stream_id,
      i, buf_planes->plane_info.mp[i].stride,
      buf_planes->plane_info.mp[i].scanline,
      buf_planes->plane_info.mp[i].len,
      buf_planes->plane_info.mp[i].offset);
  }
}

/** isp_util_select_pipeline_streams
 *    @stream:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_select_pipeline_streams(isp_t *isp, isp_session_t *session)
{
  int rc = 0;
  if (session->use_pipeline && !session->active_count) {
    rc = isp_ch_util_select_pipeline_channel(isp, session);
    if (rc < 0) {
      CDBG_ERROR("%s: select_pipeline_stream error = %d\n",
        __func__, rc);
      return rc;
    }
  }
  return rc;
}

/** isp_util_compute_stripe_info
 *    @stream:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_compute_stripe_info(isp_t *isp, isp_session_t *session,
  isp_stream_t *stream)
{
  int rc = 0;

  if (stream->cfg.ispif_out_cfg.is_split && !session->active_count) {
     /* compute in the single sensor dual ISP mode
      * the correct overlap region, now that
       the hw stream output dimensions are known */
     uint32_t initial_overlap = 128;
     rc = isp_ch_util_compute_stripe_info_of_channel(isp, session,
       initial_overlap);
     if (rc < 0) {
       CDBG_ERROR("%s: failed to compute stripe info for dual ISP mode."
         "rc = %d\n",__func__, rc);
       return rc;
     }
  }
  /* single ISP case no op */
  return rc;
}

/** isp_util_traverse_map_buf
 *    @data:
 *    @user_data:
 * TODO
 *
 * Return: TRUE - success and FASLE - failure
 **/
static boolean isp_util_traverse_map_buf(void *data, void *user_data)
{
  uint32_t i;
  find_stream_map_buf_t *map_buf = (find_stream_map_buf_t *)user_data;
  mct_stream_map_buf_t *img_buf = (mct_stream_map_buf_t *)data;
  struct v4l2_buffer *v4l2_buf = NULL;

  v4l2_buf= &map_buf->isp_map_buf[map_buf->cnt].buffer;
  /*map_buf->isp_map_buf[map_buf->cnt].fd = img_buf->common_fd;*/
  map_buf->isp_map_buf[map_buf->cnt].vaddr = NULL;
  v4l2_buf->m.planes = map_buf->isp_map_buf[map_buf->cnt].planes;
  for (i = 0; i < img_buf->num_planes; i++) {
    v4l2_buf->m.planes[i].m.userptr = img_buf->buf_planes[i].fd;
    v4l2_buf->m.planes[i].data_offset = img_buf->buf_planes[i].offset;
    v4l2_buf->m.planes[i].length = img_buf->buf_planes[i].size;
    map_buf->isp_map_buf[map_buf->cnt].addr[i] =
      (unsigned long)img_buf->buf_planes[i].buf;
  }
  v4l2_buf->length = img_buf->num_planes;
  v4l2_buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  v4l2_buf->index = img_buf->buf_index;
   v4l2_buf->memory = V4L2_MEMORY_USERPTR;
  map_buf->cnt++;
  return TRUE;
}

/** isp_util_query_stream_associated_channels
 *    @stream_ids:
 *    @num_channel:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
static int isp_util_query_stream_associated_channels(isp_t *isp,
  isp_session_t *session, int num_streams, uint32_t *stream_ids,
  int *num_channel, isp_channel_t *channel[ISP_MAX_STREAMS],
  boolean ignore_null_ch)
{
  int i, j, num_ch = 0;
  isp_stream_t *stream;

  for (i = 0; i < num_streams; i++) {
    stream = isp_util_find_stream_in_session(session, stream_ids[i]);
    if (!stream) {
      CDBG_ERROR("%s: cannot find stream %d\n", __func__, stream_ids[i]);
      return -1;
    }
    for (j = 0; j < ISP_MAX_STREAMS; j++) {
      if ((1 << j) & stream->channel_idx_mask) {
        /* has a ch associated with it */
        if (num_ch < ISP_MAX_STREAMS) {
          channel[num_ch] =
            isp_ch_util_find_channel_in_session_by_idx(session, j);
          if (!channel[num_ch]) {
            if (!ignore_null_ch)
              continue;
            CDBG_ERROR("%s: error, sessid = %d, cannot find channel idx %d\n",
              __func__, session->session_id, j);
            return -1;
          }
          num_ch++;
        }
      }
    }
  }
  *num_channel = num_ch;
  return 0;
}

/** isp_util_request_image_buf
 *    @stream_ids:
 *    @num_streams:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_request_image_buf(isp_t *isp, isp_session_t *session,
  int num_streams, uint32_t *stream_ids)
{
  int rc = 0, num_channel = 0;
  isp_channel_t *channel[ISP_MAX_STREAMS];

  rc = isp_util_query_stream_associated_channels(
    isp, session, num_streams, stream_ids, &num_channel, channel, FALSE);
  if (rc < 0) {
    CDBG_ERROR("%s: error, sessionid = %d, query channel error\n",
      __func__, session->session_id);
    return rc;
  }
  rc = isp_ch_util_request_channel_image_buf(isp, session, num_channel,
    channel);
  return rc;
}

/** isp_util_release_image_buf
 *    @stream_ids:
 *    @num_streams:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_release_image_buf(isp_t *isp, isp_session_t *session,
  int num_streams, uint32_t *stream_ids)
{
  int rc = 0, num_channel = 0;
  isp_channel_t *channel[ISP_MAX_STREAMS];

  rc = isp_util_query_stream_associated_channels(
    isp, session, num_streams, stream_ids, &num_channel, channel, TRUE);
  if (rc < 0) {
    CDBG_ERROR("%s: error, sessionid = %d, query channel error\n",
      __func__, session->session_id);
    return rc;
  }
  isp_ch_util_release_channel_image_buf(isp, session, num_channel, channel);
  return rc;
}

/** isp_util_save_effect
 *    @session: session context in which parameter should be saved
 *    @effect:  effect to save
 *
 *  Saves effect parameter in the session context.
 **/
static void isp_util_save_effect(isp_session_t *session, int32_t *effect)
{
  session->saved_params.effect = *effect;
}

/** isp_util_save_vhdr
 *    @session: session context in which parameter should be saved
 *    @effect:  effect to save
 *
 *  Saves effect parameter in the session context.
 **/
static void isp_util_save_vhdr(isp_session_t *session, uint32_t *vhdr_enable)
{
  session->saved_params.vhdr_enable = *vhdr_enable;
}

/** isp_util_save_contrast
 *    @session: session context in which parameter should be saved
 *    @contrast:  contrast to save
 *
 *  Saves contrast parameter in the session context.
 **/
static void isp_util_save_contrast(isp_session_t *session, int32_t *contrast)
{
  session->saved_params.contrast = *contrast;
}

/** isp_util_save_saturation
 *    @session: session context in which parameter should be saved
 *    @saturation:  saturation to save
 *
 *  Saves saturation parameter in the session context.
 **/
static void isp_util_save_saturation(isp_session_t *session,
  int32_t *saturation)
{
  session->saved_params.saturation = *saturation;
}

/** isp_util_save_bestshot
 *    @session: session context in which parameter should be saved
 *    @bestshot:  bestshot value to save
 *
 *  Saves bestshot parameter in the session context.
 **/
static void isp_util_save_bestshot(isp_session_t *session,
  cam_scene_mode_type *bestshot)
{
  session->saved_params.bestshot = *bestshot;
}

/** isp_util_save_sce
 *    @session: session context in which parameter should be saved
 *    @sce_factor:  SCE factorvalue to save
 *
 *  Saves SCE factor parameter in the session context.
 **/
static void isp_util_save_sce(isp_session_t *session, int32_t *sce_factor)
{
  session->saved_params.sce_factor = *sce_factor;
}

/** isp_util_set_bestshot_param
 *    @isp: isp root
 *    @session: isp session
 *    @bracketing_data: bracketing control data
 *
 *  Sets  bracketing control parameter
 **/
int isp_util_set_cam_bracketing_ctrl_param(isp_t *isp, isp_session_t *session,
  mct_bracket_ctrl_t bracketing_data)
{
  int rc = 0;

  if (session->vfe_ids & (1 << VFE0)) {
    if (isp->data.hw[VFE0].hw_ops) {
      rc = isp->data.hw[VFE0].hw_ops->set_params(
        isp->data.hw[VFE0].hw_ops->ctrl,
        ISP_HW_SET_PARAM_BRACKETING_DATA,
        (void *)&bracketing_data, sizeof(mct_bracket_ctrl_t));
    }
  }
  if (session->vfe_ids & (1 << VFE1)) {
    if (isp->data.hw[VFE1].hw_ops) {
      rc = isp->data.hw[VFE1].hw_ops->set_params(
        isp->data.hw[VFE1].hw_ops->ctrl,
        ISP_HW_SET_PARAM_BRACKETING_DATA,
        (void *)&bracketing_data, sizeof(mct_bracket_ctrl_t));
    }
  }

  return rc;
}

/** isp_util_set_bestshot_param
 *    @isp: isp root
 *    @session: isp session
 *    @bestshot: bestshot data
 *
 *  Sets bestshot parameter
 **/
static int isp_util_set_bestshot_param(isp_t *isp, isp_session_t *session,
  cam_scene_mode_type bestshot)
{
  int rc = 0;

  if (session->vfe_ids & (1 << VFE0)) {
    if (isp->data.hw[VFE0].hw_ops) {
      rc = isp->data.hw[VFE0].hw_ops->set_params(
        isp->data.hw[VFE0].hw_ops->ctrl,
        ISP_HW_SET_PARAM_BEST_SHOT,
        (void *)&bestshot, sizeof(bestshot));
    }
  }
  if (session->vfe_ids & (1 << VFE1)) {
    if (isp->data.hw[VFE1].hw_ops) {
      rc = isp->data.hw[VFE1].hw_ops->set_params(
        isp->data.hw[VFE1].hw_ops->ctrl,
        ISP_HW_SET_PARAM_BEST_SHOT,
        (void *)&bestshot, sizeof(bestshot));
    }
  }

  return rc;
}

/** isp_util_set_effect_param
 *    @isp: isp root
 *    @session: isp session
 *    @effect: effect data
 *
 *  Sets effect parameter
 **/
static int isp_util_set_effect_param(isp_t *isp, isp_session_t *session,
  int32_t effect)
{
  int rc = 0;

  if (session->vfe_ids & (1 << VFE0)) {
    if (isp->data.hw[VFE0].hw_ops) {
       rc = isp->data.hw[VFE0].hw_ops->set_params(
          isp->data.hw[VFE0].hw_ops->ctrl,
          ISP_HW_SET_PARAM_EFFECT, (void *)&effect, sizeof(effect));
    }
  }
  if (session->vfe_ids & (1 << VFE1)) {
    if (isp->data.hw[VFE1].hw_ops) {
       rc = isp->data.hw[VFE1].hw_ops->set_params(
          isp->data.hw[VFE1].hw_ops->ctrl,
          ISP_HW_SET_PARAM_EFFECT, (void *)&effect, sizeof(effect));
    }
  }

  return rc;
}

/** isp_util_set_bestshot
 *    @isp: isp root
 *    @isp_sink_port: isp sink port
 *    @session_id: session ID
 *    @stream_id: stream ID
 *    @bestshot: bestshot data
 *
 *  Sets bestshot parameter.
 **/
int isp_util_set_bestshot(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, cam_scene_mode_type *bestshot)
{
  int rc = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;

  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return rc;
  }

  isp_util_save_bestshot(session, bestshot);

  stream = isp_util_find_stream_in_session(session, stream_id);
  if ((!stream)) {
    CDBG_ERROR("%s: error: null stream\n", __func__);
    return -1;
  }

  if (session->vfe_ids == 0){
    ISP_DBG(ISP_MOD_COM,"%s: vfe not started yet\n", __func__);
    return 0;
  }

  if (session->saved_params.bestshot != ISP_DEFAULT_BESTSHOT) {
    rc = isp_util_set_effect_param(isp, session, ISP_DEFAULT_EFFECT);
    if (!rc) {
      rc = isp_util_set_bestshot_param(isp, session,
        session->saved_params.bestshot);
    }
  } else {
    rc = isp_util_set_bestshot_param(isp, session,
      session->saved_params.bestshot);
    if (!rc) {
      rc = isp_util_set_effect_param(isp, session,
        session->saved_params.effect);
    }
  }

  return rc;
}

/** isp_util_get_rolloff_table
 *    @isp: isp root
 *    @session_id: session ID
 *    @stream_id: stream ID
 *    @rolloff_table: rolloff table data
 *
 *  Gets the rolloff table.
 **/
int isp_util_get_rolloff_table(isp_t *isp, uint32_t session_id,
  uint32_t stream_id, mct_event_stats_isp_rolloff_t *rolloff_table)
{
  int rc = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return rc;
  }

  stream = isp_util_find_stream_in_session(session, stream_id);
  if ((!stream)) {
    CDBG_ERROR("%s: error: null stream\n", __func__);
    return -1;
  }

  if (session->vfe_ids == 0){
    ISP_DBG(ISP_MOD_COM,"%s: vfe not started yet\n", __func__);
    return 0;
  }

  /* note - only one VFE expected, as only VHDR mode will
     use this GET */
  if (session->vfe_ids & (1 << VFE0)) {
    if (isp->data.hw[VFE0].hw_ops) {
      rc = isp->data.hw[VFE0].hw_ops->get_params(
        isp->data.hw[VFE0].hw_ops->ctrl,
        ISP_HW_GET_PARAM_ROLLOFF_TABLE, NULL, 0,
        (void *)rolloff_table, sizeof(*rolloff_table));
    }
  }
  if (session->vfe_ids & (1 << VFE1)) {
    if (isp->data.hw[VFE1].hw_ops) {
      rc = isp->data.hw[VFE1].hw_ops->get_params(
        isp->data.hw[VFE1].hw_ops->ctrl,
        ISP_HW_GET_PARAM_ROLLOFF_TABLE, NULL, 0,
        (void *)rolloff_table, sizeof(*rolloff_table));
    }
  }

  return rc;
}

/** isp_util_set_skin_color_enhance
 *    @isp: isp root
 *    @isp_sink_port: isp sink port
 *    @session_id: session ID
 *    @stream_id: stream ID
 *    @sce_factor: sce_factor data
 *
 *  Sets sce_factor parameter.
 **/
int isp_util_set_skin_color_enhance(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, int32_t *sce_factor)
{
  int rc = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return rc;
  }

  isp_util_save_sce(session, sce_factor);

  stream = isp_util_find_stream_in_session(session, stream_id);
  if ((!stream)) {
    CDBG_ERROR("%s: error: null stream\n", __func__);
    return -1;
  }

  if (session->vfe_ids == 0){
    ISP_DBG(ISP_MOD_COM,"%s: vfe not started yet\n", __func__);
    return 0;
  }

  if (session->vfe_ids & (1 << VFE0)) {
    if (isp->data.hw[VFE0].hw_ops) {
      rc = isp->data.hw[VFE0].hw_ops->set_params(
        isp->data.hw[VFE0].hw_ops->ctrl,
        ISP_HW_SET_PARAM_SCE, (void *)sce_factor, sizeof(*sce_factor));
    }
    if (rc < 0)
      goto END;
  }
  if (session->vfe_ids & (1 << VFE1)) {
    if (isp->data.hw[VFE1].hw_ops) {
      rc = isp->data.hw[VFE1].hw_ops->set_params(
        isp->data.hw[VFE1].hw_ops->ctrl,
        ISP_HW_SET_PARAM_SCE, (void *)sce_factor, sizeof(*sce_factor));
    }
  }

END:
  return rc;
}

/** isp_util_set_effect
 *    @isp: isp root
 *    @isp_sink_port: isp sink port
 *    @session_id: session ID
 *    @stream_id: stream ID
 *    @effect: effect data
 *
 *  Sets bestshot parameter.
 **/
int isp_util_set_effect(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, int32_t *effect)
{
  int rc = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return rc;
  }

  isp_util_save_effect(session, effect);

  stream = isp_util_find_stream_in_session(session, stream_id);
  if ((!stream)) {
     CDBG_ERROR("%s: error: null stream\n", __func__);
     return -1;
  }

  if (session->vfe_ids == 0){
    ISP_DBG(ISP_MOD_COM,"%s: vfe not started yet\n", __func__);
    return 0;
  }

  isp_util_set_effect_param(isp, session, *effect);

  return rc;
}

/** isp_util_set_module_trigger
 *    @isp: isp root
 *    @session_id: session ID
 *    @stream_id: stream ID
 *    @set_param:
 *
 *  Sets bestshot parameter.
 **/
int isp_util_set_module_trigger(isp_t *isp, uint32_t session_id,
  uint32_t stream_id, isp_mod_trigger_t *set_param)
{
  int rc = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return rc;
  }

  stream = isp_util_find_stream_in_session(session, stream_id);
  if ((!stream)) {
    CDBG_ERROR("%s: error: null stream\n", __func__);
    return -1;
  }

  if (session->vfe_ids == 0){
    ISP_DBG(ISP_MOD_COM,"%s: vfe not started yet\n", __func__);
    return 0;
  }

  if (session->vfe_ids & (1 << VFE0)){
    if (isp->data.hw[VFE0].hw_ops){
       rc = isp->data.hw[VFE0].hw_ops->set_params(
          isp->data.hw[VFE0].hw_ops->ctrl,
          ISP_HW_SET_ISP_MODULE_TRIGGER, (void *)set_param, sizeof(*set_param));
    }
  }
  if (session->vfe_ids & (1 << VFE1)){
    if (isp->data.hw[VFE1].hw_ops){
       rc = isp->data.hw[VFE1].hw_ops->set_params(
          isp->data.hw[VFE1].hw_ops->ctrl,
          ISP_HW_SET_ISP_MODULE_TRIGGER, (void *)set_param, sizeof(*set_param));
    }
  }
  return rc;

}

/** isp_util_set_module_enable
 *    @isp: isp root
 *    @session_id: session ID
 *    @stream_id: stream ID
 *    @set_param:
 *
 *  Sets bestshot parameter.
 **/
int isp_util_set_module_enable(isp_t *isp, uint32_t session_id,
  uint32_t stream_id, isp_mod_trigger_t *set_param)
{
  int rc = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return rc;
  }

  stream = isp_util_find_stream_in_session(session, stream_id);
  if ((!stream)) {
     CDBG_ERROR("%s: error: null stream\n", __func__);
     return -1;
  }

  if (session->vfe_ids == 0){
    ISP_DBG(ISP_MOD_COM,"%s: vfe not started yet\n", __func__);
    return 0;
  }

  if (session->vfe_ids & (1 << VFE0)) {
    if (isp->data.hw[VFE0].hw_ops) {
       rc = isp->data.hw[VFE0].hw_ops->set_params(
          isp->data.hw[VFE0].hw_ops->ctrl,
          ISP_HW_SET_ISP_MODULE_ENABLE, (void *)set_param, sizeof(*set_param));
    }
  }
  if (session->vfe_ids & (1 << VFE1)) {
    if (isp->data.hw[VFE1].hw_ops) {
       rc = isp->data.hw[VFE1].hw_ops->set_params(
          isp->data.hw[VFE1].hw_ops->ctrl,
          ISP_HW_SET_ISP_MODULE_ENABLE, (void *)set_param, sizeof(*set_param));
    }
  }
  return rc;

}


/** isp_util_set_eztune_diagnostics
 *  @isp
 *  @session_id
 *  @stream_id
 *  @set_param
 *
 **/
int isp_util_set_eztune_diagnostics(isp_t *isp, uint32_t session_id,
  uint32_t stream_id, int32_t *set_param)
{
  int rc = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return rc;
  }

  stream = isp_util_find_stream_in_session(session, stream_id);
  if ((!stream)) {
     CDBG_ERROR("%s: error: null stream\n", __func__);
     return -1;
  }

  if (session->vfe_ids == 0){
    ISP_DBG(ISP_MOD_COM,"%s: vfe not started yet\n", __func__);
    return 0;
  }

  if (session->vfe_ids & (1 << VFE0)) {
    if (isp->data.hw[VFE0].hw_ops) {
       rc = isp->data.hw[VFE0].hw_ops->set_params(
          isp->data.hw[VFE0].hw_ops->ctrl,
          ISP_HW_SET_ISP_DIAGNOSTICS, (void *)set_param, sizeof(*set_param));
    }
  }
  if (session->vfe_ids & (1 << VFE1)) {
    if (isp->data.hw[VFE1].hw_ops) {
       rc = isp->data.hw[VFE1].hw_ops->set_params(
          isp->data.hw[VFE1].hw_ops->ctrl,
          ISP_HW_SET_ISP_DIAGNOSTICS, (void *)set_param, sizeof(*set_param));
    }
  }
  return rc;
}

/** isp_util_set_video_hdr
 *    @stream_id:
 *    @vhdr_enable:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_set_video_hdr(isp_t *isp, uint32_t session_id, uint32_t stream_id,
  uint32_t *vhdr_enable)
{
  int rc = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return rc;
  }

  isp_util_save_vhdr(session, vhdr_enable);

  stream = isp_util_find_stream_in_session(session, stream_id);
  if ((!stream)) {
     CDBG_ERROR("%s: error: null stream\n", __func__);
     return -1;
  }

  if (session->vfe_ids == 0){
    ISP_DBG(ISP_MOD_COM,"%s: vfe not started yet\n", __func__);
    return 0;
  }

  if (session->vfe_ids & (1 << VFE0)) {
    if (isp->data.hw[VFE0].hw_ops) {
      rc = isp->data.hw[VFE0].hw_ops->set_params(
         isp->data.hw[VFE0].hw_ops->ctrl,
         ISP_HW_SET_PARAM_VHDR, (void *)vhdr_enable, sizeof(*vhdr_enable));
    }
  }
  if (session->vfe_ids & (1 << VFE1)) {
    if (isp->data.hw[VFE1].hw_ops) {
      rc = isp->data.hw[VFE1].hw_ops->set_params(
         isp->data.hw[VFE1].hw_ops->ctrl,
         ISP_HW_SET_PARAM_VHDR, (void *)vhdr_enable, sizeof(*vhdr_enable));
    }
  }

  return rc;
}

/** isp_util_set_tintless
 *
 *    @isp:
 *    @session_id:
 *    @tintless:
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_set_tintless(isp_t *isp, uint32_t session_id,
  isp_tintless_data_t *tintless_data)
{
  int rc = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return rc;
  }

  if (session->vfe_ids == 0){
    ISP_DBG(ISP_MOD_COM,"%s: vfe not started yet\n", __func__);
    return 0;
  }

  if (session->vfe_ids & (1 << VFE0)) {
    if (isp->data.hw[VFE0].hw_ops) {
      rc = isp->data.hw[VFE0].hw_ops->set_params(
         isp->data.hw[VFE0].hw_ops->ctrl,
         ISP_HW_SET_PARAM_TINTLESS, (void *)tintless_data, sizeof(*tintless_data));
    }
  }
  if (session->vfe_ids & (1 << VFE1)) {
    if (isp->data.hw[VFE1].hw_ops) {
      rc = isp->data.hw[VFE1].hw_ops->set_params(
         isp->data.hw[VFE1].hw_ops->ctrl,
         ISP_HW_SET_PARAM_TINTLESS, (void *)tintless_data, sizeof(*tintless_data));
    }
  }

  return rc;
}

/** isp_util_set_contrast
  *    @isp: isp root
  *    @isp_sink_port: Sink port
  *    @session_id: session id
  *    @stream_id: stream id
  *    @contrast: contrast parameter
  *
  *  Set gamma table according required effect.
  **/
int isp_util_set_contrast(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, int32_t *contrast)
{
  int rc = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;

  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return rc;
  }

  isp_util_save_contrast(session, contrast);

  stream = isp_util_find_stream_in_session(session, stream_id);

  if ((!stream)) {
    CDBG_ERROR("%s: error: null stream\n", __func__);
    return -1;
  }

  /* if we have not started VFE yet no need to send uncfg to HW */
  if (session->vfe_ids == 0){
    ISP_DBG(ISP_MOD_COM,"%s: not started vfe yet\n", __func__);
    return 0;
  }

  if (session->vfe_ids & (1 << VFE0)) {
    if (isp->data.hw[VFE0].hw_ops) {
      rc = isp->data.hw[VFE0].hw_ops->set_params(
        isp->data.hw[VFE0].hw_ops->ctrl,
        ISP_HW_SET_PARAM_CONTRAST, (void *)contrast, sizeof(*contrast));
    }
  }

  if (session->vfe_ids & (1 << VFE1)) {
    if (isp->data.hw[VFE1].hw_ops) {
      rc = isp->data.hw[VFE1].hw_ops->set_params(
        isp->data.hw[VFE1].hw_ops->ctrl,
        ISP_HW_SET_PARAM_CONTRAST, (void *)contrast, sizeof(*contrast));
    }
  }

  return rc;
}

/** isp_util_set_saturation
  *    @isp: isp root
  *    @isp_sink_port: Sink port
  *    @session_id: session id
  *    @stream_id: stream id
  *    @saturation: saturation parameter
  *    @is_init_setting: Is the Initialization param or not
  *
  *  Set saturation according required effect.
  **/
int isp_util_set_saturation(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, int32_t *saturation,
  boolean is_init_setting)
{
  int rc = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;
  isp_saturation_setting_t sat_setting;

  sat_setting.saturation = *saturation;
  sat_setting.is_init_setting = is_init_setting;

  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return rc;
  }

  isp_util_save_saturation(session, saturation);

  stream = isp_util_find_stream_in_session(session, stream_id);

  if ((!stream)) {
    CDBG_ERROR("%s: error: null stream\n", __func__);
    return -1;
  }

  /* if we have not started VFE yet no need to send uncfg to HW */
  if (session->vfe_ids == 0){
    ISP_DBG(ISP_MOD_COM,"%s: not started vfe yet\n", __func__);
    return 0;
  }

  if (session->vfe_ids & (1 << VFE0)) {
    if (isp->data.hw[VFE0].hw_ops) {
      rc = isp->data.hw[VFE0].hw_ops->set_params(
        isp->data.hw[VFE0].hw_ops->ctrl,
        ISP_HW_SET_PARAM_SATURATION, (void *)&sat_setting,
        sizeof(isp_saturation_setting_t));
    }
  }

  if (session->vfe_ids & (1 << VFE1)) {
    if (isp->data.hw[VFE1].hw_ops) {
      rc = isp->data.hw[VFE1].hw_ops->set_params(
        isp->data.hw[VFE1].hw_ops->ctrl,
        ISP_HW_SET_PARAM_SATURATION, (void *)&sat_setting,
        sizeof(isp_saturation_setting_t));
    }
  }

  return rc;
}

/** isp_util_save_sharpness
 *    @session: session context in which parameter should be saved
 *    @sharpness:  sharpness to save
 *
 *  Saves sharpness parameter in the session context.
 **/
static void isp_util_save_sharpness(isp_session_t *session,
  int32_t *sharpness)
{
  session->saved_params.sharpness = *sharpness;
}

/** isp_util_set_sharpness
  *    @isp: isp root
  *    @isp_sink_port: Sink port
  *    @session_id: session id
  *    @stream_id: stream id
  *    @sharpness: sharpness parameter
  *
  *  Set sharpness according required effect.
  **/
int isp_util_set_sharpness(isp_t *isp,
                        isp_port_t *isp_sink_port,
                        uint32_t session_id,
                        uint32_t stream_id,
                        int32_t *sharpness)
{
  int rc = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;

  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return rc;
  }

  isp_util_save_sharpness(session, sharpness);

  stream = isp_util_find_stream_in_session(session, stream_id);

  if ((!stream)) {
     CDBG_ERROR("%s: error: null stream\n", __func__);
     return -1;
  }

  /* if we have not started VFE yet no need to send uncfg to HW */
  if (session->vfe_ids == 0){
     ISP_DBG(ISP_MOD_COM,"%s: not started vfe yet\n", __func__);
    return 0;
  }

  if (session->vfe_ids & (1 << VFE0)) {
    if (isp->data.hw[VFE0].hw_ops) {
      rc = isp->data.hw[VFE0].hw_ops->set_params(
        isp->data.hw[VFE0].hw_ops->ctrl,
        ISP_HW_SET_PARAM_SHARPNESS,
        (void *)sharpness, sizeof(*sharpness));
    }
  }

  if (session->vfe_ids & (1 << VFE1)) {
    if (isp->data.hw[VFE1].hw_ops) {
       rc = isp->data.hw[VFE1].hw_ops->set_params(
          isp->data.hw[VFE1].hw_ops->ctrl,
          ISP_HW_SET_PARAM_SHARPNESS,
          (void *)sharpness, sizeof(*sharpness));
    }
  }

  return rc;
}

/** isp_util_set_bundle
 *    @stream_id:
 *    @bundle_param:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_set_bundle(isp_t *isp, isp_port_t *isp_sink_port,
  uint32_t session_id, uint32_t stream_id, cam_bundle_config_t *bundle_param)
{
  int i, rc = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return rc;
  }
  for (i = 0; i < bundle_param->num_of_streams; i++) {
    stream = isp_util_find_stream_in_session(session,
      bundle_param->stream_ids[i]);
    if (!stream) {
      CDBG_ERROR("%s: stream (sessid = %d, streamid= %d)m not found\n",
        __func__, session_id, bundle_param->stream_ids[i]);
      goto error;
    }
    session->hal_bundling_mask |= (1 << stream->stream_idx);
    ISP_DBG(ISP_MOD_COM,"%s: stream_id = %d, stream_idx = %d, hal_bundling_mask= 0x%x",
      __func__, stream->stream_id, stream->stream_idx,
      session->hal_bundling_mask);
  }
  return rc;
error:
  session->hal_bundling_mask = 0;
  return -1;
}

/** isp_util_set_recording_hint
 *    @stream_id:
 *    @recording_hint:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_set_recording_hint(isp_t *isp, uint32_t session_id,
  uint32_t stream_id, int32_t *recording_hint)
{
  int rc = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;
  isp_param_frame_skip_pattern_t frame_skip;

  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return -1;
  }
  session->recording_hint = *recording_hint;
  if (session->recording_hint)
    session->saved_params.use_bundled_frame_skip = 0;
  else
  /* Reset frameskip when switching back to camera mode */
    session->saved_params.frame_skip.pattern = NO_SKIP;

  ISP_DBG(ISP_MOD_COM,"%s: session_id = %d, recording_hint = %d\n",
    __func__, session_id, session->recording_hint);
  return rc;
}

/** isp_util_set_frame_skip
 *    @stream_id:
 *    @skip_pattern:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_set_frame_skip(isp_t *isp, uint32_t session_id, uint32_t stream_id,
  int32_t *skip_pattern)
{
  int rc = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;
  isp_param_frame_skip_pattern_t *frame_skip;
  enum msm_vfe_frame_skip_pattern temp_skip,hfr_skip;
  isp_channel_t *channel = NULL;

  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);
    return -1;
  }
  stream = isp_util_find_stream_in_session(session, stream_id);
  if (!stream) {
    CDBG_ERROR("%s: stream (sessid = %d, streamid= %d)m not found\n",
      __func__, session_id, stream_id);
    return -1;
  }

  /* avoid frame skip of burst stream */
  channel = isp_ch_util_find_channel_in_session_by_idx(session, stream->stream_idx);
  if (!channel || channel->stream_info.streaming_mode == CAM_STREAMING_MODE_BURST){
    CDBG_ERROR("%s: channel is NULL or BURST stream, streamidx= %d\n", __func__,
      stream->stream_idx);
    return rc;
  }

  temp_skip = (enum msm_vfe_frame_skip_pattern)(*skip_pattern);
  hfr_skip = isp_ch_util_get_hfr_skip_pattern(session);
  if((session->saved_params.use_bundled_frame_skip) &&
     (session->saved_params.bundled_frame_skip.pattern > temp_skip) &&
     (session->saved_params.bundled_frame_skip.session_id == session_id) &&
     (session->saved_params.bundled_frame_skip.stream_id == stream_id)) {
     frame_skip = &session->saved_params.bundled_frame_skip;
     frame_skip->pattern = session->saved_params.bundled_frame_skip.pattern;
  } else if (session->recording_hint &&
             isp_util_is_lowpowermode_feature_enable(isp, session->session_id)) {
    /* In LPM camcorder mode preview and video constitute two different streams.
    In HFR case frameskip needs to be set for preview stream since 30FPS will
    suffice. By default frameskip will be NO_SKIP when HFR is turned off */
    frame_skip = &session->saved_params.frame_skip;
    frame_skip->pattern = hfr_skip;
  } else {
    frame_skip = &session->saved_params.frame_skip;
    frame_skip->pattern = temp_skip;
  }
  frame_skip->session_id = session_id;
  frame_skip->stream_id = stream_id;

  if (session->vfe_ids & (1 << VFE0)) {
    if (isp->data.hw[VFE0].hw_ops) {
      rc = isp->data.hw[VFE0].hw_ops->set_params(
        isp->data.hw[VFE0].hw_ops->ctrl,
        ISP_HW_SET_PARAM_FRAME_SKIP, (void *)frame_skip, sizeof(*frame_skip));
    }
  }

  if (session->vfe_ids & (1 << VFE1)) {
    if (isp->data.hw[VFE1].hw_ops) {
      rc = isp->data.hw[VFE1].hw_ops->set_params(
        isp->data.hw[VFE1].hw_ops->ctrl,
        ISP_HW_SET_PARAM_FRAME_SKIP, (void *)frame_skip, sizeof(*frame_skip));
    }
  }
  return rc;
}

/** isp_util_get_stream_ids_by_mask
 *    @stream_idx_mask:
 *    @num_streams:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_get_stream_ids_by_mask(isp_session_t *session,
  uint32_t stream_idx_mask, int *num_streams, uint32_t *stream_ids)
{
  int i, rc = 0;
  *num_streams = 0;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (stream_idx_mask & (1 << i)) {
      stream_ids[*num_streams] = session->streams[i].stream_id;
      *num_streams += 1;
      ISP_DBG(ISP_MOD_COM,"%s: stream_id = %d, bit_pos = %d, mask = 0x%x",
        __func__, session->streams[i].stream_id, i, stream_idx_mask);
    }
  }
  return rc;
}

/** isp_util_get_user_streams
 *    @session: isp session
 *    @stream_id: Stream ID
 *    @stream_ids: Stream ID
 *
 *  Gets IDs of streams from same HAL bunfle as given stream.
 **/
int isp_util_get_user_streams(isp_session_t *session, uint32_t stream_id,
  uint32_t *stream_ids)
{
  int num_streams = 0;;
  int rc = 0;
  isp_stream_t *stream;

  stream = isp_util_find_stream_in_session(session, stream_id);
  if (!stream) {
    CDBG_ERROR("%s: cannot find stream (stream_id: %d)\n",
      __func__, stream_id);
    return -1;
  }

  if (session->hal_bundling_mask != 0 &&
    (session->hal_bundling_mask & (1 << stream->stream_idx))) {
    session->streamon_bundling_mask |= (1 << stream->stream_idx);

    if (session->hal_bundling_mask != session->streamon_bundling_mask) {
      CDBG_HIGH("%s: bundling use case - bundle mask = 0x%x,"
        "streamon_bundle = 0x%x, nop\n", __func__, session->hal_bundling_mask,
        session->streamon_bundling_mask);
      /* nop */
      return 0;
    } else {
      rc = isp_util_get_stream_ids_by_mask(session,
        session->hal_bundling_mask, &num_streams, stream_ids);
    }
  } else {
    stream_ids[0] = stream_id;
    num_streams = 1;
  }

  return num_streams;
}

/** isp_util_set_hfr
 *    @stream_id:
 *    @hfr_mode:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_set_hfr(isp_t *isp, uint32_t session_id, uint32_t stream_id,
  int32_t *hfr_mode)
{
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;
  mct_event_t mct_event;

  if (!session)
    return -1;
  session->hfr_param.hfr_mode = *hfr_mode;
  session->hfr_param.stream_id = stream_id;
  stream = isp_util_find_stream_in_session(session, stream_id);
  if (!stream)
    return -1;
  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.u.module_event.type = MCT_EVENT_MODULE_HFR_MODE_NOTIFY;
  mct_event.u.module_event.module_event_data = (void *)hfr_mode;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.identity = pack_identity(session_id, stream_id);
  /* broadcast hfr downstream */
  if (stream->src_ports[ISP_SRC_PORT_3A]) {
    mct_event.direction = MCT_EVENT_DOWNSTREAM;
    mct_port_send_event_to_peer(stream->src_ports[ISP_SRC_PORT_3A],
    &mct_event);
  }
  return 0;
}

/** isp_util_set_stats_bf_filter_size
 *    @stream_id:
 *    @bf_filter_size:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_set_stats_bf_filter_size(isp_t *isp, uint32_t session_id,
         uint32_t stream_id)
{
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;
  mct_event_t mct_event;
  uint32_t isp_ver;
  mct_stats_hpf_size_type kernel_size = MCT_EVENT_STATS_HPF_2X5;

  if (!session)
    return -1;

  stream = isp_util_find_stream_in_session(session, stream_id);
  if (!stream)
    return -1;

  /* Use 2x13 BF filter for MSM8909 */
  isp_ver = isp->data.sd_info.sd_info[0].isp_version;
  if (GET_ISP_MAIN_VERSION(isp_ver) == ISP_VERSION_32 &&
      GET_ISP_SUB_VERSION(isp_ver) == ISP_REVISION_V3) {
    kernel_size = MCT_EVENT_STATS_HPF_2X13;
  }
  CDBG_HIGH("%s: BF filter size = %d\n", __func__, kernel_size);

  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.u.module_event.type = MCT_EVENT_MODULE_ISP_STATS_INFO;
  mct_event.u.module_event.module_event_data = (void *)&kernel_size;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.identity = pack_identity(session_id, stream_id);

  /* broadcast hfr downstream */
  if (stream->src_ports[ISP_SRC_PORT_3A]) {
    mct_event.direction = MCT_EVENT_DOWNSTREAM;
    mct_port_send_event_to_peer(stream->src_ports[ISP_SRC_PORT_3A],
    &mct_event);
  }
  return 0;
}

/** isp_util_set_dis
 *    @stream_id:
 *    @dis_enable:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_set_dis(isp_t *isp, uint32_t session_id, uint32_t stream_id,
  int32_t *dis_enable)
{
  isp_session_t *session = isp_util_find_session(isp, session_id);

  if (!session)
    return -1;

  session->dis_param.dis_enable = *dis_enable;
  session->dis_param.stream_id = stream_id;
  return 0;
}

/** isp_util_set_vt
 *    @session_id
 *    @vt_enbale:
 * TODO
 *
 * Return: 0- success and negative value - failure
 **/
int isp_util_set_vt(isp_t *isp, uint32_t session_id, int32_t *vt_enable)
{
  isp_session_t *session = isp_util_find_session(isp, session_id);

  if (!session) {
    return -1;
  }

  session->saved_params.vt_enable = *vt_enable;
  return 0;
}

/** isp_util_set_longshot
 *    @session_id
 *    @isLongshotEnabled:
 * TODO
 *
 * Return: 0- success and negative value - failure
 **/
int isp_util_set_longshot(isp_t *isp, uint32_t session_id,
       int32_t *longshot_enable)
{
  isp_session_t *session = isp_util_find_session(isp, session_id);

  if (!session) {
    return -1;
  }

  session->saved_params.longshot_enable = *longshot_enable;
  return 0;
}

/** isp_util_set_lowpowermode
 *    @session_id
 *    @lowpowermode_enable:
 * TODO
 *
 * Return: 0- success and negative value - failure
 **/
int isp_util_set_lowpowermode(isp_t *isp, uint32_t session_id,
       boolean *lowpowermode_enable)
{
  isp_session_t *session = isp_util_find_session(isp, session_id);

  if (!session) {
    return -1;
  }
  session->saved_params.lowpowermode_enable =
    (uint32_t)*lowpowermode_enable;
  return 0;
}

/** isp_util_set_cds_mode
 *    @isp: Isp handler
 *    @session_id: session id
 *    @cds_mode: camera cds mode
 *
 * Return: 0- success and negative value - failure
 **/
int isp_util_set_cds_mode(isp_t *isp, uint32_t session_id,
  cam_cds_mode_type_t *cds_mode)
{
  isp_session_t *session = isp_util_find_session(isp, session_id);
  int rc = 0;
  if (!session || !cds_mode) {
    return -1;
  }

  switch (*cds_mode) {
  case CAM_CDS_MODE_OFF:
    session->uv_subsample_mode = ISP_UV_SUBSAMPLE_OFF;
    rc = isp_set_uv_subsample(isp, session_id, NULL);
    break;
  case CAM_CDS_MODE_ON:
    session->uv_subsample_mode = ISP_UV_SUBSAMPLE_ON;
    rc = isp_set_uv_subsample(isp, session_id, NULL);
    break;
  case CAM_CDS_MODE_AUTO:
    session->uv_subsample_mode = ISP_UV_SUBSAMPLE_AUTO;
    break;
  default:
    CDBG_ERROR("%s%d] Invalid subsample mode %d\n",
      __func__, __LINE__, *cds_mode);
    rc = -1;
    break;
  }
  return rc;
}

/** isp_util_send_dis_config_to_stats
 *    @isp:
 *    @session:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_send_dis_config_to_stats(isp_t *isp, isp_session_t *session)
{
  isp_dis_config_info_t dis_cfg;
  isp_cs_rs_config_t cs_rs_cfg;
  isp_stream_t *stream;
  mct_event_t event;
  int rc = 0;

  stream = isp_util_find_stream_in_session(session,
    session->dis_param.stream_id);
  if (!stream) {
    CDBG_ERROR("%s: stream (sessid = %d, streamid = %d)m not found\n",
      __func__, session->session_id, session->dis_param.stream_id);
    return -1;
  }

  if (stream == NULL) {
    CDBG_ERROR("%s: cannot find hw stream for dis streamid = %d\n",
      __func__, session->dis_param.stream_id);
    return -1;
  }

  memset(&event, 0, sizeof(event));
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = pack_identity(stream->session_id, stream->stream_id);
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_DIS_CONFIG;
  event.u.module_event.module_event_data = (void *)&dis_cfg;

  dis_cfg.session_id = stream->session_id;
  dis_cfg.stream_id = stream->stream_id;
  dis_cfg.streaming_mode = stream->stream_info.streaming_mode;
  dis_cfg.height = session->dis_param.height;
  dis_cfg.width = session->dis_param.width;
  dis_cfg.col_num = 0;
  dis_cfg.row_num = 0;

  cs_rs_cfg.session_id = stream->session_id;
  if (session->vfe_ids & (1 << VFE0)) {
    if (isp->data.hw[VFE0].hw_ops) {
      rc = isp->data.hw[VFE0].hw_ops->get_params(
        isp->data.hw[VFE0].hw_ops->ctrl,
        ISP_HW_GET_CS_RS_CONFIG, NULL, 0, &cs_rs_cfg, sizeof(cs_rs_cfg));
      if (rc == 0) {
        dis_cfg.col_num += cs_rs_cfg.col_num;
        dis_cfg.row_num += cs_rs_cfg.raw_num;
      } else {
        CDBG_ERROR("%s: error in cs_rs configuration, rc = %d\n",
          __func__, rc);
        return rc;
      }
    }
  }

  if (session->vfe_ids & (1 << VFE1)) {
    if (isp->data.hw[VFE1].hw_ops) {
      rc = isp->data.hw[VFE1].hw_ops->get_params(
        isp->data.hw[VFE1].hw_ops->ctrl,
        ISP_HW_GET_CS_RS_CONFIG, NULL, 0, &cs_rs_cfg, sizeof(cs_rs_cfg));
      if (rc == 0) {
        dis_cfg.col_num += cs_rs_cfg.col_num;
        dis_cfg.row_num += cs_rs_cfg.raw_num;
      } else {
        CDBG_ERROR("%s: error in cs_rs configuration, rc = %d\n",
          __func__, rc);
        return rc;
      }
    }
  }

  if (FALSE == mct_port_send_event_to_peer(
    stream->src_ports[ISP_SRC_PORT_3A], &event)) {
    CDBG_ERROR("%s: cannot send dis config to 3A\n", __func__);
    return -1;
  }

  return rc;
}

/** isp_util_get_roi_map
 *    @isp_id:
 *    @hw_zoom_entry:
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
static int isp_util_get_roi_map(isp_t *isp, isp_session_t *session,
  int isp_id, isp_hw_zoom_param_entry_t *hw_zoom_entry)
{
  int rc = 0;

  if (isp->data.hw[isp_id].hw_ops) {
    rc = isp->data.hw[isp_id].hw_ops->get_params(
      isp->data.hw[isp_id].hw_ops->ctrl,
      ISP_HW_GET_ROI_MAP, NULL, 0, (void *)hw_zoom_entry,
      sizeof(isp_hw_zoom_param_entry_t));
    if (rc) {
      CDBG_ERROR("%s: VFE%d get roi map error = %d\n", __func__, isp_id, rc);
      return rc;
    }
  }
  return rc;
}

/** isp_util_broadcast_stream_crop_to_bus
 *    @entry:
 *    @frame_id:
 *    @timestamp:
 * TODO
 *
 * Return: nothing
 **/
static void isp_util_broadcast_stream_crop_to_bus(isp_t *isp,
  isp_session_t *session, isp_stream_t *stream,
  isp_zoom_scaling_param_entry_t *entry, uint32_t frame_id,
  struct timeval *timestamp)
{
  mct_bus_msg_stream_crop_t stream_crop;
  mct_bus_msg_t bus_msg;

  memset(&bus_msg, 0, sizeof(bus_msg));
  memset(&stream_crop, 0, sizeof(stream_crop));

  bus_msg.type = MCT_BUS_MSG_ISP_STREAM_CROP;
  bus_msg.msg = (void *)&stream_crop;
  bus_msg.sessionid = session->session_id;

  stream_crop.session_id = session->session_id;
  stream_crop.stream_id = stream->stream_id;
  stream_crop.frame_id = frame_id;
  stream_crop.timestamp = *timestamp;

  isp_ch_util_convert_crop_to_stream(session, stream,
    &stream_crop, entry,isp);

  /* In case of smaller-than-output valid area, set crop for offline PPROC */
  if((stream_crop.crop_out_x == 0) &&
      (((uint32_t)stream->stream_info.dim.width > stream_crop.width_map) ||
       ((uint32_t)stream->stream_info.dim.height > stream_crop.height_map))) {
    stream_crop.crop_out_x = stream_crop.width_map;
    stream_crop.crop_out_y = stream_crop.height_map;
    stream_crop.x = 0;
    stream_crop.y = 0;
  }

  if (TRUE != mct_module_post_bus_msg(isp->module,
                (mct_bus_msg_t *)&bus_msg)) {
    CDBG_ERROR("%s: session_id = %d, stream_id = %d, error\n",
      __func__, stream_crop.session_id, stream_crop.stream_id);
  }
}

/** isp_util_broadcast_stream_crop_downstream
 *    @entry:
 *    @frame_id:
 *    @timestamp:
 * TODO
 *
 * Return: nothing
 **/
static void isp_util_broadcast_stream_crop_downstream(isp_t *isp,
  isp_session_t *session, isp_stream_t *stream,
  isp_zoom_scaling_param_entry_t *entry, uint32_t frame_id,
  struct timeval *timestamp)
{
  mct_bus_msg_stream_crop_t stream_crop;
  mct_event_t mct_event;

  memset(&mct_event, 0, sizeof(mct_event));
  memset(&stream_crop, 0, sizeof(stream_crop));

  mct_event.u.module_event.type = MCT_EVENT_MODULE_STREAM_CROP;
  mct_event.u.module_event.module_event_data = (void *)&stream_crop;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.identity = stream->stream_info.identity;
  CDBG_LOW("%s crop identity  mct_event.identity %d\n",__func__, mct_event.identity);
  mct_event.direction = MCT_EVENT_DOWNSTREAM;

  stream_crop.session_id = session->session_id;
  stream_crop.stream_id = stream->stream_id;
  stream_crop.frame_id = frame_id;
  stream_crop.timestamp = *timestamp;
  isp_ch_util_convert_crop_to_stream(session, stream,
    &stream_crop, entry,isp);

  if (stream->src_ports[ISP_SRC_PORT_3A])
    mct_port_send_event_to_peer(stream->src_ports[ISP_SRC_PORT_3A],
      &mct_event);

  if (stream->src_ports[ISP_SRC_PORT_DATA])
    mct_port_send_event_to_peer(stream->src_ports[ISP_SRC_PORT_DATA],
      &mct_event);
}

/** isp_util_calc_pproc_crop
 *    @session:
 *    @stream:
 *    @stream_crop:
 *
 *  Calculates stream crop for pproc-only zoom.
 *
 * Return: nothing
 **/
static void isp_util_calc_pproc_crop(isp_t *isp, isp_session_t *session, isp_stream_t *stream,
  mct_bus_msg_stream_crop_t *stream_crop)
{
  int rc;
  isp_hw_set_crop_factor_t crop_factor;
  uint32_t in_x, in_y, out_x, out_y, temp_x, temp_y;
  boolean rotation_enable = FALSE;

  session->pproc_zoom_val = session->zoom_val;

  memset(&crop_factor, 0, sizeof(crop_factor));
  crop_factor.session_id = session->session_id;
  crop_factor.crop_factor = 0;
  rc = isp_zoom_get_crop_factor(session->zoom_session,
    session->zoom_val, &crop_factor.crop_factor);
  if (rc) {
    CDBG_ERROR("%s: isp_zoom_get_crop_factor error = %d\n", __func__, rc);
    return;
  }

  memset(stream_crop, 0, sizeof(*stream_crop));

  /* sensor reports bytes not pixels in case of YUV sensor */
  in_x = stream->cfg.sensor_cfg.dim_output.width / 2;
  in_y = stream->cfg.sensor_cfg.dim_output.height;

  /*In case rotation is enabled it should be ignored since
    sensor and ISP always operate in landscape. CPP will take
    care of rotation*/
  if (stream->stream_info.pp_config.rotation == ROTATE_90 ||
       stream->stream_info.pp_config.rotation == ROTATE_270) {
    out_x = stream->stream_info.dim.height;
    out_y = stream->stream_info.dim.width;
    rotation_enable = TRUE;
  } else {
    out_x = stream->stream_info.dim.width;
    out_y = stream->stream_info.dim.height;
    rotation_enable = FALSE;
  }

  if(in_x * out_y > out_x * in_y){
    temp_y = in_y;
    temp_x = in_y * out_x / out_y;
  } else {
    temp_x = in_x;
    temp_y = in_x * out_y / out_x;
  }

  if((in_x != stream_crop->width_map) || (in_y != stream_crop->height_map)) {
    stream_crop->width_map = temp_x;
    stream_crop->height_map = temp_y;
    stream_crop->x_map = (in_x - stream_crop->width_map) >> 1;
    stream_crop->y_map = (in_y - stream_crop->height_map) >> 1;
  }

  if((in_x != stream_crop->crop_out_x) || (in_y != stream_crop->crop_out_y)) {
    stream_crop->crop_out_x = isp_zoom_calc_dim(session->zoom_session, temp_x,
      crop_factor.crop_factor);
    stream_crop->crop_out_y = isp_zoom_calc_dim(session->zoom_session, temp_y,
      crop_factor.crop_factor);
    stream_crop->x = (in_x - stream_crop->crop_out_x) >> 1;
    stream_crop->y = (in_y - stream_crop->crop_out_y) >> 1;
  }

  ISP_DBG(ISP_MOD_COM,"%s width_map %u height_map %u x_map %u y_map %u", __func__,
    stream_crop->width_map, stream_crop->height_map, stream_crop->x_map,
    stream_crop->y_map);
  ISP_DBG(ISP_MOD_COM,"%s crop_out_x %u crop_out_y %u x %u y %u", __func__,
    stream_crop->crop_out_x, stream_crop->crop_out_y, stream_crop->x,
    stream_crop->y );

}

/** isp_util_broadcast_pproc_only_crop_downstream
 *    @isp:
 *    @session:
 *    @stream:
 *    @frame_id:
 *    @timestamp:
 *
 *  Broadcasts stream crop dimensions downstream.
 *
 * Return: nothing
 **/
static void isp_util_broadcast_pproc_only_crop_downstream(isp_t *isp,
  isp_session_t *session, isp_stream_t *stream, uint32_t frame_id,
  struct timeval *timestamp)
{
  mct_bus_msg_stream_crop_t stream_crop;
  mct_event_t mct_event;

  memset(&mct_event, 0, sizeof(mct_event));
  memset(&stream_crop, 0, sizeof(stream_crop));

  mct_event.u.module_event.type = MCT_EVENT_MODULE_STREAM_CROP;
  mct_event.u.module_event.module_event_data = (void *)&stream_crop;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.identity =
    pack_identity(stream->session_id, stream->stream_id);
  mct_event.direction = MCT_EVENT_DOWNSTREAM;

  stream_crop.session_id = session->session_id;
  stream_crop.stream_id = stream->stream_id;
  stream_crop.frame_id = frame_id;

  /* for initial crop configuration we do not have timestamp */
  if(timestamp)
    stream_crop.timestamp = *timestamp;

  isp_util_calc_pproc_crop(isp,session, stream,
      &stream_crop);

  if (stream->src_ports[ISP_SRC_PORT_3A])
    mct_port_send_event_to_peer(stream->src_ports[ISP_SRC_PORT_3A],
      &mct_event);

  if (stream->src_ports[ISP_SRC_PORT_DATA])
    mct_port_send_event_to_peer(stream->src_ports[ISP_SRC_PORT_DATA],
      &mct_event);
}

/** isp_util_broadcast_stream_crop
 *    @entry:
 *    @frame_id:
 *    @timestamp:
 * TODO
 *
 * Return: nothing
 **/
static void isp_util_broadcast_stream_crop(isp_t *isp, isp_session_t *session,
  isp_stream_t *stream, isp_zoom_scaling_param_entry_t *entry,
  uint32_t frame_id, struct timeval *timestamp)
{
  if (!stream->src_ports[ISP_SRC_PORT_DATA]) {
    /* the stream has no PP linked so ISP send crop to bus */
    isp_util_broadcast_stream_crop_to_bus(isp,
      session, stream, entry, frame_id, timestamp);
  }

  /* the stream has downstream link broadcast to all of them */
  isp_util_broadcast_stream_crop_downstream(isp,
    session, stream, entry, frame_id, timestamp);
}

/** isp_util_get_zoom_scaling_entry
 *    @entry:
 *    @frame_id:
 *    @timestamp:
 *
 * TODO
 *
 * Return: isp_zoom_scaling_param_entry_t pointer
 **/
static isp_zoom_scaling_param_entry_t *isp_util_get_zoom_scaling_entry(
  isp_session_t *session, isp_zoom_scaling_param_t *scaling_param,
  isp_stream_t *stream)
{
  int i, channel_id = 0;
  uint32_t mask;

  mask = stream->channel_idx_mask;
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (mask & (1 << i)) {
      channel_id = session->channel[i].channel_id;
      break;
    }
  }
  for (i = 0; i < scaling_param->num; i++) {
    if (channel_id == (int)scaling_param->entry[i].stream_id)
      return &scaling_param->entry[i];
  }
  return NULL;
}

/** isp_util_broadcast_pproc_zoom_crop
 *    @isp:
 *    @session_id:
 *    @streamids:
 *    @frame_id:
 *    @timestamp:
 *
 *  Broadcasts stream crop dimensions for pproc zoom for all streams.
 *
 * Return: isp_zoom_scaling_param_entry_t pointer
 **/
void isp_util_broadcast_pproc_zoom_crop (isp_t *isp, uint32_t session_id,
  int num_streams, uint32_t *streamids, uint32_t frame_id,
  struct timeval *timestamp)
{
  int i;
  isp_stream_t *stream;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_port_t *isp_sink_port = NULL;

  if (!session) {
    CDBG_ERROR("%s: Error, no session with id = %d found.\n",
      __func__, session_id);
    return;
  }

  for (i = 0; i < num_streams; i++) {
    if (!isp_util_check_yuv_sensor_from_stream(isp, session_id, streamids[i]))
      continue;
    stream = isp_util_find_stream_in_session(session, streamids[i]);

    if (!stream)
      continue;

    isp_util_broadcast_pproc_only_crop_downstream(isp, session,
      stream, frame_id, timestamp);
  }

}

/** isp_util_broadcast_zoom_crop
 *    @num_streams:
 *    @frame_id:
 *    @timestamp:
 *
 * TODO
 *
 * Return: isp_zoom_scaling_param_entry_t pointer
 **/
void isp_util_broadcast_zoom_crop(isp_t *isp, uint32_t session_id,
  int num_streams, uint32_t *streamids, uint32_t frame_id,
  struct timeval *timestamp)
{
  int i;
  isp_stream_t *stream;
  isp_zoom_scaling_param_t scaling_param;
  isp_zoom_scaling_param_entry_t *entry;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_port_t *isp_sink_port = NULL;

  if (!session) {
    CDBG_ERROR("%s: Error, no session with id = %d found.\n",
      __func__, session_id);
    return;
  }

  if (session->zoom_stream_cnt == 0)
    return;

  pthread_mutex_lock(
    &isp->data.session_critical_section[session->session_idx]);
  isp_zoom_get_scaling_param(session->zoom_session, &scaling_param);
  pthread_mutex_unlock(
    &isp->data.session_critical_section[session->session_idx]);

  for (i = 0; i < num_streams; i++) {
    stream = isp_util_find_stream_in_session(session, streamids[i]);
    if (!stream)
      continue;

    isp_sink_port = stream->sink_port->port_private;
    /* non PIX stream */
    if (isp_sink_port->u.sink_port.caps.use_pix == 0)
      continue;

    entry = isp_util_get_zoom_scaling_entry(session, &scaling_param,
        stream);

    if (!entry)
      continue;

    isp_util_broadcast_stream_crop(isp, session,
      stream, entry, frame_id, timestamp);
  }
}

/** isp_util_ihist_la_trigger_update
 *    @isp:
 *    @session:
 *
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_ihist_la_trigger_update(isp_t *isp, isp_session_t *session)
{
  int rc = 0;

  if (session->ihist_update == 1) {
    /*Trigger update LA for each VFE*/
    if (session->vfe_ids & (1 << VFE0)) {
      ISP_DBG(ISP_MOD_COM,"%s: VFE0 Ihist/LA trigger update\n", __func__);
      if (isp->data.hw[VFE0].hw_ops) {
        rc = isp->data.hw[VFE0].hw_ops->set_params(
          isp->data.hw[VFE0].hw_ops->ctrl,
          ISP_HW_SET_IHIST_LA_TRIGGER_UPDATE,
          (void *)&session->ihist_stats, sizeof(q3a_ihist_stats_t));
      }
      if (rc < 0) {
        session->ihist_update = 0;
        ISP_DBG(ISP_MOD_COM,"%s: VFE0 Ihist/LA trigger update failed, rc = %d\n", __func__, rc);
        return rc;
      }
    }
    if (session->vfe_ids & (1 << VFE1)) {
      ISP_DBG(ISP_MOD_COM,"%s: VFE1 Ihist/LA trigger update\n", __func__);
      if (isp->data.hw[VFE1].hw_ops) {
        rc = isp->data.hw[VFE1].hw_ops->set_params(
          isp->data.hw[VFE1].hw_ops->ctrl,
          ISP_HW_SET_IHIST_LA_TRIGGER_UPDATE,
          (void *)&session->ihist_stats, sizeof(q3a_ihist_stats_t));
      }
      if (rc < 0) {
        session->ihist_update = 0;
        ISP_DBG(ISP_MOD_COM,"%s: VFE1 Ihist/LA trigger update failed, rc = %d\n", __func__,
             rc);
        return rc;
      }
    }
    session->ihist_update = 0;
  }

  return rc;
}

/** isp_util_check_yuv_sensor_from_stream
 *  @isp
 *  @session_id
 *  @stream_id
 *
 *  check if sensor for this stream is outputing YUV.
 *
 **/
boolean isp_util_check_yuv_sensor_from_stream(isp_t *isp, uint32_t session_id,
  uint32_t stream_id)
{
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_stream_t *stream;
  bool rc = FALSE;
  isp_info_t isp_info[VFE_MAX];
  int num_isps = 0;
  num_isps = isp_get_info(isp_info);
  /* return FALSE is we want to use PIX interface for YUV sensor */
  if (isp_info[0].use_pix_for_SOC){
    return(FALSE);
  }

  if (!session)
    return rc;

  stream = isp_util_find_stream_in_session(session, stream_id);
  if (!stream)
    return rc;

  if ( stream->cfg.sensor_cfg.fmt >= CAM_FORMAT_YUV_RAW_8BIT_YUYV &&
       stream->cfg.sensor_cfg.fmt <= CAM_FORMAT_YUV_RAW_8BIT_VYUY){
    rc = TRUE;
  }
  session->saved_params.lowpowermode_yuv_enable = rc;
  return rc;
}


/** isp_util_do_pproc_zoom
 *    @isp:
 *    @session_id:
 *    @zoom_val:
 *
 *  Sets zoom parameter to session in case of pproc-only zoom.
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_do_pproc_zoom(isp_t *isp, uint32_t session_id, int32_t* zoom_val)
{
  int rc = 0;
  isp_session_t *session = isp_util_find_session(isp, session_id);

  if (!session)
    return FALSE;
  session->zoom_val = *zoom_val;
  return 0;
}

/** isp_util_do_zoom
 *    @isp:
 *    @session:
 *
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
static int isp_util_do_zoom(isp_t *isp, isp_session_t *session)
{
  int rc = 0;
  int vfe_id = -1;
  isp_hw_set_crop_factor_t crop_factor;
  isp_hw_zoom_param_entry_t *hw_zoom_entries =
    crop_factor.hw_zoom_parm.entry;

  memset(&crop_factor, 0, sizeof(crop_factor));
  crop_factor.session_id = session->session_id;
  crop_factor.crop_factor = 0;
  rc = isp_zoom_get_crop_factor(session->zoom_session,
    session->zoom_val, &crop_factor.crop_factor);
  if (rc) {
    CDBG_ERROR("%s: isp_zoom_get_crop_factor error = %d\n", __func__, rc);
    return rc;
  }

  rc = isp_ch_util_adjust_crop_factor(session, crop_factor.crop_factor,
         &crop_factor.crop_factor);
  if (rc < 0) {
    CDBG_ERROR("%s: error adjusting crop factor error = %d\n", __func__, rc);
    return rc;
  }
  pthread_mutex_lock(
    &isp->data.session_critical_section[session->session_idx]);
  session->saved_params.zoom_factor = crop_factor;
  session->saved_params.zoom_update = TRUE;
  pthread_mutex_unlock(
    &isp->data.session_critical_section[session->session_idx]);

  return 0;
}

/** isp_util_set_param_zoom
 *    @isp:
 *    @session:
 *    @new_params:
 *
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_set_param_zoom(isp_t *isp, isp_session_t *session,
   isp_hw_params_t *new_params)
{
  int rc = 0;
  if (!new_params->zoom.present)
    return 0;

  session->zoom_val = new_params->zoom.zoom_val;
  if (session->active_count == 0 || session->zoom_stream_cnt == 0) {
    /* no stream started. we just buffer zoom value */
    return 0;
  }

  rc = isp_util_do_zoom(isp, session);
  if (rc) {
    CDBG_ERROR("%s: isp_util_do_zoom error = %d\n", __func__, rc);
    return rc;
  }

  return 0;
}

/** isp_util_send_buffered_hw_params_to_hw
 *    @isp:
 *    @session:
 *
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_send_buffered_hw_params_to_hw(isp_t *isp, isp_session_t *session)
{
  int rc = 0;
  isp_hw_params_t new_params;
  isp_async_cmd_t *cmd;

  CDBG_HIGH("%s: E, session_id = %d\n", __func__, session->session_id);

  if(!session->use_pipeline)
    return rc;

  cmd = malloc(sizeof(isp_async_cmd_t));
  if (!cmd) {
    CDBG_ERROR("%s:error,  no mem for streamon cmd, session_id = %d\n",
      __func__, session->session_id);
    return -1;
  }
  memset(cmd, 0, sizeof(isp_async_cmd_t));
  cmd->cmd_id = ISP_ASYNC_COMMAND_SET_HW_PARAM;
  cmd->set_hw_params.isp = (void *)isp;
  cmd->set_hw_params.session = (void *)session;
  pthread_mutex_lock(&session->async_task.sync_mutex);
  rc = isp_enqueue_async_command(isp, session, &cmd);
  if (cmd)
    free(cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: error, isp_enqueue_async_command, session_id = %d\n",
      __func__, session->session_id);
    goto end;
  }
end:
  pthread_mutex_unlock(&session->async_task.sync_mutex);
  CDBG_HIGH("%s: X, session_id = %d\n", __func__, session->session_id);
  return rc;
}
/** isp_util_wm_bus_overflow
 *    @isp:
 *    @session:
 *
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_wm_bus_overflow_recovery(isp_t *isp, isp_hw_t *isp_hw, isp_hw_session_t *hw_session)
{
  int rc = 0;
  isp_async_cmd_t *cmd;
  isp_session_t *session = NULL;

  session = isp_util_find_session(isp, hw_session->session_id);

  if (!session) {
    CDBG_ERROR("%s unable to find session with session id %d\n", hw_session->session_id);
    return -1;
  }
  CDBG_HIGH("%s: E, session_id = %d\n", __func__, session->session_id);

  cmd = malloc(sizeof(isp_async_cmd_t));
  if (!cmd) {
    CDBG_ERROR("%s:error,  no mem for streamon cmd, session_id = %d\n",
      __func__, session->session_id);
    return -1;
  }
  memset(cmd, 0, sizeof(isp_async_cmd_t));
  cmd->cmd_id = ISP_ASYNC_COMMAND_WM_BUS_OVERFLOW_RECOVERY;
  cmd->wm_recovery.isp_hw = isp_hw;
  cmd->wm_recovery.session = hw_session;

  pthread_mutex_lock(&session->async_task.sync_mutex);
  rc = isp_enqueue_async_command(isp, session, &cmd);
  if (cmd)
    free(cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: error, isp_enqueue_async_command, session_id = %d\n",
      __func__, session->session_id);
    goto end;
  }
end:
  pthread_mutex_unlock(&session->async_task.sync_mutex);
  CDBG_HIGH("%s: X, session_id = %d\n", __func__, session->session_id);
  return rc;
}
/** isp_util_buffered_set_param_zoom
 *    @stream_id:
 *    @zoom_val:
 *
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_buffered_set_param_zoom(isp_t *isp, uint32_t session_id,
   uint32_t stream_id, int32_t *zoom_val)
{
  isp_session_t *session = isp_util_find_session(isp, session_id);
  boolean send_cmd = FALSE;
  int rc = 0;

  if (!session)
    return -1;

  pthread_mutex_lock(
    &isp->data.session_critical_section[session->session_idx]);
  session->buffered_hw_params.new_params.zoom.present = TRUE;
  session->buffered_hw_params.new_params.zoom.zoom_val = *zoom_val;
  session->buffered_hw_params.new_params.has_params = TRUE;
  if (session->buffered_hw_params.in_service &&
      !session->buffered_hw_params.hw_update_pending) {
    send_cmd = TRUE;
    session->buffered_hw_params.hw_update_pending = TRUE;
  }
  pthread_mutex_unlock(
    &isp->data.session_critical_section[session->session_idx]);
  if (send_cmd) {
    rc = isp_util_send_buffered_hw_params_to_hw(isp, session);
    if (rc < 0) {
      CDBG_ERROR("%s: session_id = %d, hw param update error = %d\n",
        __func__, session->session_id, rc);
    }
  }
  return rc;
}

/** isp_util_set_param_tintless
 *
 *    @isp:
 *    @session_id:
 *    @enabled: TRUE - enable, FALSE disable
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_set_param_tintless(isp_t *isp, uint32_t session_id,
  uint8_t *enabled)
{
  isp_session_t *session;
  int rc;

  session = isp_util_find_session(isp, session_id);
  if (!session){
    CDBG_ERROR("%s: session not found\n", __func__);
    rc = -1;
    goto error;
  }
  if (session->tintless_session != NULL){
    isp_tintless_set_param(session->tintless_session, enabled);
  }
  else {
    goto error;
  }
  return 0;
error:
  return -1;
}

/** isp_util_do_zoom_at_streamon
 *    @isp: isp root
 *    @session: isp session
 *
 *  Aplies the requred zoom at stream on.
 **/
void isp_util_do_zoom_at_streamon(isp_t *isp, isp_session_t *session)
{
  int rc;
  int vfe_id = -1;
  isp_hw_zoom_param_t hw_zoom_parm;
  isp_hw_zoom_param_entry_t *hw_zoom_entries =
    hw_zoom_parm.entry;

  if (session->zoom_stream_cnt == 0)
    return;

  if (session->vfe_ids & (1 << VFE0)) {
    vfe_id = VFE0;
    if (isp->data.hw[VFE0].hw_ops) {
      memset(&hw_zoom_parm, 0, sizeof(isp_hw_zoom_param_t));
      rc = isp->data.hw[VFE0].hw_ops->get_params(
        isp->data.hw[VFE0].hw_ops->ctrl,
        ISP_HW_GET_FOV_CROP, NULL, 0, &hw_zoom_parm, sizeof(hw_zoom_parm));
      if (rc) {
        CDBG_ERROR("%s: VFE0 zoom error = %d\n", __func__, rc);
        return;
      }
    }
  }
  if (session->vfe_ids & (1 << VFE1)) {
    vfe_id = VFE1;
    if (isp->data.hw[VFE1].hw_ops) {
      memset(&hw_zoom_parm, 0, sizeof(isp_hw_zoom_param_t));
      rc = isp->data.hw[VFE1].hw_ops->get_params(
        isp->data.hw[VFE1].hw_ops->ctrl,
        ISP_HW_GET_FOV_CROP, NULL, 0, &hw_zoom_parm, sizeof(hw_zoom_parm));
      if (rc) {
        CDBG_ERROR("%s: VFE1 zoom error = %d\n", __func__, rc);
        return;
      }
    }
  }
  if (vfe_id != -1 && hw_zoom_parm.num > 0) {
    /* Dual VFE use case */
    if (session->vfe_ids & (1 << VFE0) && session->vfe_ids & (1 << VFE1)) {
      int i;
      isp_hw_zoom_param_t right_param;
      memcpy(&right_param, &hw_zoom_parm, sizeof(isp_hw_zoom_param_t));
      isp_util_get_roi_map(isp, session, VFE0, hw_zoom_entries);
      isp_util_get_roi_map(isp, session, VFE1, right_param.entry);
      for (i = 0; i < hw_zoom_parm.num; i++)
        hw_zoom_entries[i].roi_map_info.last_pixel +=
          right_param.entry[i].roi_map_info.last_pixel -
          right_param.entry[i].roi_map_info.first_pixel + 1;
    } else {
      isp_util_get_roi_map(isp, session, vfe_id, hw_zoom_entries);
    }
    pthread_mutex_lock(
      &isp->data.session_critical_section[session->session_idx]);
    isp_set_zoom_scaling_parm(session->zoom_session, &hw_zoom_parm);
    pthread_mutex_unlock(
      &isp->data.session_critical_section[session->session_idx]);
  }

}

/** isp_util_check_streamid_match
 *    @streamids:
 *    @num:
 *
 * TODO
 *
 * Return: TRUE - success and FALSE- failure
 **/
static boolean isp_util_check_streamid_match( uint32_t streamid,
  uint32_t *streamids, int num)
{
  int i;

  for (i = 0; i < num; i++)
    if (streamid == streamids[i])
      return TRUE;
  return FALSE;
}

/** isp_util_gen_channel_streamon_list
 *    @num_hw_channels:
 *    @hw_channel_ids:
 *
 * TODO
 *
 * Return: TRUE - success and FALSE- failure
 **/
int isp_util_gen_channel_streamon_list(isp_t *isp, isp_session_t *session,
  int num_streams, uint32_t *streamids, int *num_hw_channels,
  uint32_t *hw_channel_ids)
{
  isp_stream_t *stream;
  int i, j, count = 0;
  uint32_t channel_idx_mask;
  uint32_t saved_mask = 0;

  for (i = 0; i < num_streams; i++) {
    stream = isp_util_find_stream_in_session(session, streamids[i]);
    if (!stream || stream->state == ISP_STREAM_STATE_ACTIVE)
      continue;
    channel_idx_mask = stream->channel_idx_mask;
    for (j = 0; j < ISP_MAX_STREAMS; j++) {
      if ((1 << j) & channel_idx_mask) {
        /* found a channel */
        if (count < ISP_MAX_STREAMS) {
          if (saved_mask & (1 << j))
            continue;
          saved_mask |= (1 << j);
          hw_channel_ids[count++] = session->channel[j].channel_id;
        } else {
          CDBG_ERROR("%s: channel_idx out of bound\n", __func__);
          return -1;
        }
      }
    }
  }
  *num_hw_channels = count;
  return 0;
}

/** isp_util_gen_ch_streamoff_list
 *    @num_channels:
 *    @channel_ids:
 *
 * TODO
 *
 * Return: TRUE - success and FALSE- failure
 **/
static int isp_util_gen_ch_streamoff_list(isp_t *isp, isp_session_t *session,
  int num_streams, uint32_t *streamids, int *num_channels,
  uint32_t *channel_ids)
{
  isp_stream_t *stream;
  int i, j, count = 0;
  uint32_t channel_idx_mask;
  uint32_t saved_mask = 0;

  *num_channels = 0;
  for (i = 0; i < num_streams; i++) {
    stream = isp_util_find_stream_in_session(session, streamids[i]);
    if (!stream || stream->state != ISP_STREAM_STATE_ACTIVE)
      continue;
    channel_idx_mask = stream->channel_idx_mask;
    for (j = 0; j < ISP_MAX_STREAMS; j++) {
      if ((1 << j) & channel_idx_mask) {
        /* found a channel */
        if (count < ISP_MAX_STREAMS) {
          if (saved_mask & (1 << j))
            continue;
          saved_mask |= (1 << j);
          channel_ids[count++] = session->channel[j].channel_id;
        } else {
          CDBG_ERROR("%s: channel_idx out of bound\n", __func__);
          return -1;
        }
      }
    }
  }
  *num_channels = count;
  return 0;
}

/** isp_util_send_hw_stream_output_dim_downstream_int
 *    @session:
 *    @stream:
 *
 * TODO
 *
 * Return: nothing
 **/
static void isp_util_send_hw_stream_output_dim_downstream_int(isp_t *isp,
  isp_session_t *session, isp_stream_t *stream)
{
  mct_event_t mct_event;
  mct_stream_info_t stream_info;
  isp_channel_t *channel = NULL;

  ISP_DBG(ISP_MOD_COM,"%s: session_id = %d, stream_id = %d, channel_idx_mask = 0x%x\n",
     __func__, stream->session_id, stream->stream_id, stream->channel_idx_mask);
  channel = isp_ch_util_get_image_channel(session, stream->channel_idx_mask);
  if (!channel) {
    CDBG_ERROR("%s: cannot find channel "
      "(session_id = %d, stream_id = %d, channel_mask = 0x%x)\n",
      __func__, session->session_id, stream->stream_id,
      stream->channel_idx_mask);
    return;
  }

  isp_ch_util_dump_channel_planes(channel);
  stream_info = channel->stream_info;
  stream_info.stream_type = stream->stream_info.stream_type;
  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.u.module_event.type = MCT_EVENT_MODULE_ISP_OUTPUT_DIM;
  mct_event.u.module_event.module_event_data = (void *)&stream_info;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.identity = pack_identity(session->session_id, stream->stream_id);
  mct_event.direction = MCT_EVENT_DOWNSTREAM;
  if (stream->src_ports[ISP_SRC_PORT_3A]) {
    mct_port_send_event_to_peer(
      stream->src_ports[ISP_SRC_PORT_3A], &mct_event);
  }

  if (stream->src_ports[ISP_SRC_PORT_DATA]) {
    mct_port_send_event_to_peer(
      stream->src_ports[ISP_SRC_PORT_DATA], &mct_event);
  }
}

/** isp_util_send_hw_stream_output_dim_downstream
 *    @num_streams:
 *    @stream_ids:
 *
 * TODO
 *
 * Return: nothing
 **/
void isp_util_send_hw_stream_output_dim_downstream(isp_t *isp,
  isp_session_t *session, int32_t num_streams, uint32_t *stream_ids)
{
  isp_stream_t *stream;
  mct_event_t mct_event;
  mct_stream_info_t stream_info;
  int32_t i;
  uint32_t stream_id;
  isp_channel_t *channel = NULL;

  for (i = 0; i < num_streams; i++) {
    stream_id = stream_ids[i];
    ISP_DBG(ISP_MOD_COM,"%s: stream_id = %d, i = %d\n", __func__, stream_id, i);
    stream = isp_util_find_stream_in_session(session, stream_id);
    if (!stream) {
      CDBG_ERROR("%s: cannot find stream (session_id = %d, stream_id = %d)\n",
        __func__, session->session_id, stream_id);
    } else
      isp_util_send_hw_stream_output_dim_downstream_int(isp, session, stream);
  }
}

/** isp_util_update_hal_image_buf_to_channel
 *    @session:
 *    @stream:
 *
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_update_hal_image_buf_to_channel(isp_session_t *session,
  isp_stream_t *stream)
{
  int i;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if ((1 << i) & stream->channel_idx_mask) {
      if (session->channel[i].use_native_buf == 0 &&
          session->channel[i].channel_type == ISP_CHANNEL_TYPE_IMAGE) {
        session->channel[i].stream_info.img_buffer_list =
          stream->stream_info.img_buffer_list;

        session->channel[i].total_num_bufs = stream->stream_info.num_bufs;
        ISP_DBG(ISP_MOD_COM,"%s: identity = 0x%x, channel_id = %d, stream_type = %d,"
          "hal_buf_ptr = %p num_bufs %d\n", __func__,
          stream->stream_info.identity, session->channel[i].channel_id,
          stream->stream_info.stream_type,
          session->channel[i].stream_info.img_buffer_list,
          session->channel[i].total_num_bufs);
      } else {
        return -1;
      }
      break;
    }
  }
  return 0;
}

/** isp_util_config_for_streamon
 *    @isp:
 *    @session:
 *
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_config_for_streamon(isp_t *isp, isp_session_t *session)
{
  int rc = 0;
  rc = isp_ch_util_prepare_hw_config_for_streamon(isp, session);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_ch_util_prepare_hw_config_for_streamon error!"
      "sessid = %d, rc = %d\n", __func__, session->session_id, rc);
    return rc;
  }
  return rc;
}

/** isp_util_streamon
 *    @user_num_streams:
 *    @user_stream_ids:
 *
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_streamon(isp_t *isp, isp_session_t *session, int user_num_streams,
  uint32_t *user_stream_ids)
{
  int rc = 0;
  int num_hw_channels = 0;
  uint32_t hw_channel_ids[ISP_MAX_STREAMS];

  /* prepare to streamon ISP HW stream*/
  isp_util_gen_channel_streamon_list(isp, session,
    user_num_streams, user_stream_ids, &num_hw_channels, hw_channel_ids);

  rc = isp_ch_util_streamon(isp, session,
    num_hw_channels, hw_channel_ids);
  return rc;
}

/** isp_util_streamoff
 *    @num_streams:
 *    @stream_ids:
 *
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_streamoff(isp_t *isp, isp_session_t *session, int num_streams,
  uint32_t *stream_ids, boolean stop_immediately)
{
  int rc = 0;
  int i;
  start_stop_stream_t param;
  int num_channels;
  uint32_t channel_ids[ISP_MAX_STREAMS];

  isp_util_gen_ch_streamoff_list(isp, session,
    num_streams, stream_ids,&num_channels, channel_ids);

  rc = isp_ch_util_streamoff(isp,session,num_channels,channel_ids, stop_immediately);

  /* When restarting stream, new stream id can be different.
     we set invalid value to DIS stream_id so on stream on this can
     be initialized to correct one */
  if(session->dis_param.dis_enable)
    for (i = 0; i < num_streams; i++)
      if(stream_ids[i] == session->dis_param.stream_id)
        session->dis_param.stream_id = 0;

  return rc;
}

/** isp_util_dump_af_stats_config
 *    @stats_data:
 *
 * TODO
 *
 * Return: nothing
 **/
void isp_util_dump_stats_config(stats_config_t *stats_data)
{
  af_config_t  *af_config = stats_data->af_config;

  if (af_config == NULL)
    return;

  ISP_DBG(ISP_MOD_COM,"%s: stream_id = %d, roi_type = %d, grid_h_num = %d, grid_v_num = %d, "
    "roi_left = %d, roi_top = %d, roi_w = %d, roi_h = %d\n",
    __func__, af_config->stream_id, af_config->roi_type,
    af_config->grid_info.h_num, af_config->grid_info.v_num,
    af_config->roi.left, af_config->roi.top,
    af_config->roi.width, af_config->roi.height);
}

/** isp_util_unconfig_stream
 *    @session:
 *    @stream:
 *
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_unconfig_stream(isp_t *isp, isp_session_t *session,
  isp_stream_t *stream)
{
  int i, rc = 0;
  int isp_id;
  uint32_t mask;

  if (!stream || !session) {
    CDBG_ERROR("%s: null stream %p or null session %p\n",
      __func__, stream, session);
    return -1;
  }
  if (session->vfe_ids == 0)
    return 0;
  mask = stream->channel_idx_mask;
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (mask & (1 << i)) {
      mask &= ~(1 << i);
      /* has a channel associated with the stream */
      if (session->vfe_ids & (1 << VFE0)) {
        isp_id = VFE0;
        rc = isp_ch_util_unconfig_channel(isp, isp_id, &session->channel[i]);
      }
      if (session->vfe_ids & (1 << VFE1)) {
        isp_id = VFE1;
        rc = isp_ch_util_unconfig_channel(isp, isp_id, &session->channel[i]);
      }
      if (!mask)
        break; /* doing this to avoid loop max num streams */
    }
  }
  return rc;
}

/** isp_util_unconfig_stream_by_sink_port
 *    @session:
 *    @isp_sink_port:
 *
 * TODO
 *
 * Return: 0 - success and negative value - failure
 **/
int isp_util_unconfig_stream_by_sink_port(isp_t *isp, isp_session_t *session,
  isp_port_t *isp_sink_port)
{
  int i;
  isp_stream_t *stream = NULL;
  int isp_id;
  uint32_t mask;


  if (session->vfe_ids == 0)
    return 0;
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    stream = isp_sink_port->u.sink_port.streams[i];
    if (stream)
      isp_util_unconfig_stream(isp, session, stream);
  }
  return 0;
}

/** isp_util_send_initial_zoom_crop_to_3a
 *    @num_streams:
 *    @streamids:
 *
 * This method sends zoom information downstream to 3A module
 *
 * Return: nothing
 **/
void isp_util_send_initial_zoom_crop_to_3a(isp_t *isp, uint32_t session_id,
  int num_streams, uint32_t *streamids)
{
  int i;
  isp_stream_t *stream;
  isp_zoom_scaling_param_t scaling_param;
  isp_zoom_scaling_param_entry_t *entry;
  isp_session_t *session = isp_util_find_session(isp, session_id);
  isp_port_t *isp_sink_port = NULL;

  if (!session) {
    CDBG_ERROR("%s: Error, no session with id = %d found.\n",
      __func__, session_id);
    return;
  }

  if (session->zoom_stream_cnt == 0)
    return;

  pthread_mutex_lock(
    &isp->data.session_critical_section[session->session_idx]);
  isp_zoom_get_scaling_param(session->zoom_session, &scaling_param);
  pthread_mutex_unlock(
    &isp->data.session_critical_section[session->session_idx]);

  for (i = 0; i < num_streams; i++) {
    stream = isp_util_find_stream_in_session(session, streamids[i]);
    if (!stream)
      continue;

    isp_sink_port = stream->sink_port->port_private;
    /* non PIX stream */
    if (isp_sink_port->u.sink_port.caps.use_pix == 0)
      continue;

    entry = isp_util_get_zoom_scaling_entry(session, &scaling_param,
      stream);

    if (!entry)
      continue;

    if (!stream->src_ports[ISP_SRC_PORT_DATA]) {
      mct_bus_msg_stream_crop_t stream_crop;
      mct_event_t mct_event;

      memset(&mct_event, 0, sizeof(mct_event));
      memset(&stream_crop, 0, sizeof(stream_crop));

      mct_event.u.module_event.type = MCT_EVENT_MODULE_STREAM_CROP;
      mct_event.u.module_event.module_event_data = (void *)&stream_crop;
      mct_event.type = MCT_EVENT_MODULE_EVENT;
      mct_event.identity =
        pack_identity(stream->session_id, stream->stream_id);
      mct_event.direction = MCT_EVENT_DOWNSTREAM;

      stream_crop.session_id = session->session_id;
      stream_crop.stream_id = stream->stream_id;
      stream_crop.frame_id = 0;
      isp_ch_util_convert_crop_to_stream(session, stream,
        &stream_crop, entry,isp);
      mct_port_send_event_to_peer(stream->src_ports[ISP_SRC_PORT_3A],
        &mct_event);
    }
  }
}

/** isp_util_stream_use_pipeline
 *    @isp:
 *    @stream:
 *
 * This methods checks for for valid stream, sink port and pix
 * interface.
 *
 * Return: TRUE - success and FALSE - failure
 **/
boolean isp_util_stream_use_pipeline(isp_t *isp,  isp_stream_t *stream)
{
  isp_port_t *isp_sink_port = NULL;
  isp_sink_port_t *sink_port = NULL;

  if (!stream) {
    CDBG_ERROR("%s: null stream\n", __func__);
    return FALSE;
  }
  if (!stream->sink_port) {
    CDBG_ERROR("%s: null sink port, identity = 0x%x\n",
      __func__, stream->stream_info.identity);
    return FALSE;
  }
  isp_sink_port = stream->sink_port->port_private;
  if (!isp_sink_port) {
    CDBG_ERROR("%s: no sink port, identity = 0x%x\n",
      __func__, stream->stream_info.identity);
    return FALSE;
  }
  sink_port = &isp_sink_port->u.sink_port;
  if (sink_port->caps.use_pix)
    return TRUE;
  else
    return FALSE;
}


/** isp_util_send_uv_subsample_cmd
 *    @isp: isp instance
 *    @session: session
 *    @uv_subsample: if 1 - turn ON subsample, if 0 turn OFF
 *
 * Utility function to send async command to turn ON/OFF UV
 * subsampling
 *
 * Return: nothing
 **/
int isp_util_send_uv_subsample_cmd(isp_t *isp, isp_session_t *session,
  uint32_t uv_subsample)
{
  int rc = 0;
  isp_async_cmd_t *cmd = NULL;

  ISP_DBG(ISP_MOD_COM,"%s: uv_subsample = %d\n", __func__, uv_subsample);
  cmd = malloc(sizeof(isp_async_cmd_t));
  if (!cmd) {
    ISP_DBG(ISP_MOD_COM,"%s: error, nomemory\n", __func__);
    return -1;
  }
  memset(cmd, 0, sizeof(isp_async_cmd_t));
  cmd->cmd_id = ISP_ASYNC_COMMAND_UV_SUBSAMPLE;
  cmd->uv_subsample_enable = uv_subsample;
  pthread_mutex_lock(
    &isp->data.session_critical_section[session->session_idx]);
  session->uv_subsample_ctrl.switch_state = ISP_UV_SWITCH_STATE_ENQUEUED;
  pthread_mutex_unlock(
    &isp->data.session_critical_section[session->session_idx]);
  rc = isp_enqueue_async_command(isp, session, &cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_enqueue_async_command error\n", __func__);
  }
  return rc;
}
