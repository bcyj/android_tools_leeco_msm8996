/*============================================================================
Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#define ATRACE_TAG ATRACE_TAG_ALWAYS
#include <cutils/trace.h>
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

#define ISP_IMG_DUMP_ENABLE
#define PAD_TO_SIZE(size, padding)  ((size + padding - 1) & ~(padding - 1))
#define MAX_VFE_SCALERS_RATIO       (15u)
#define PRECISION_MUL               (100) /* precision multiplier */
/* The value 0.6 was arrived by experimentation */
#define CUSTOM_ROUND(a)((long)(a + 0.6))
#define SENSOR_OUT_13MP 13000000
#define SENSOR_OUT_8MP   8000000

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#if 0
#undef CDBG
#define CDBG ALOGE
#endif

#undef CDBG_ERROR
#define CDBG_ERROR ALOGE
#undef CDBG_HIGH
#define CDBG_HIGH ALOGE

static int isp_ch_util_check_min_dim_for_channel(
  isp_session_t *session, int num_channel, isp_channel_t **channel);
static int isp_ch_util_check_min_dim_for_chained_scalers(
  isp_session_t *session, isp_channel_t *continuous_channel,
  isp_channel_t *burst_channel);

/** isp_ch_util_dump_channel_planes
 *
 * DESCRIPTION: debug message for isp channel info
 *
 **/
void isp_ch_util_dump_channel_planes(isp_channel_t *channel)
{
  cam_stream_buf_plane_info_t *buf_planes = &channel->stream_info.buf_planes;
  cam_dimension_t *dim = &channel->stream_info.dim;
  uint32_t i;

   ISP_DBG(ISP_MOD_COM,"%s: sess_id = %d, channel_id = %d, "
    "width = %d, height = %d, fmt = %d, num_planes = %d\n",
    __func__, channel->session_id, channel->channel_id,
    dim->width, dim->height, channel->stream_info.fmt,
    buf_planes->plane_info.num_planes);

  for (i = 0; i < buf_planes->plane_info.num_planes; i++) {
    ISP_DBG(ISP_MOD_COM,"%s: sess_id = %d, channel_id = %d, idx = %d,"
      "stride = %d, scanline = %d, len = %d, offset = %d\n",
      __func__, channel->session_id, channel->channel_id,
      i, buf_planes->plane_info.mp[i].stride,
      buf_planes->plane_info.mp[i].scanline,
      buf_planes->plane_info.mp[i].len,
      buf_planes->plane_info.mp[i].offset);
  }
}

/** isp_ch_util_is_pix_raw_fmt
 *
 * DESCRIPTION:
 *
 **/
static boolean isp_ch_util_is_pix_raw_fmt(cam_format_t fmt)
{
  switch (fmt) {
  case CAM_FORMAT_YUV_420_NV12_VENUS:
  case CAM_FORMAT_YUV_420_NV12:
  case CAM_FORMAT_YUV_420_NV21:
  case CAM_FORMAT_YUV_420_NV21_ADRENO:
  case CAM_FORMAT_YUV_420_YV12:
  case CAM_FORMAT_YUV_422_NV16:
  case CAM_FORMAT_YUV_422_NV61:
    return FALSE;

  case CAM_FORMAT_YUV_RAW_8BIT_YUYV:
  case CAM_FORMAT_YUV_RAW_8BIT_YVYU:
  case CAM_FORMAT_YUV_RAW_8BIT_UYVY:
  case CAM_FORMAT_YUV_RAW_8BIT_VYUY:
  case CAM_FORMAT_JPEG_RAW_8BIT:
  case CAM_FORMAT_META_RAW_8BIT:
  case CAM_FORMAT_META_RAW_10BIT:
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG:
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG:
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB:
  case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_BGGR:
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG:
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG:
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB:
  case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_BGGR:
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG:
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG:
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB:
  case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GBRG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GRBG:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_RGGB:
  case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_BGGR:
    return TRUE;
  default:
    return FALSE;
  }
  return FALSE;
}

/** isp_ch_util_dump_frame
 *
 * DESCRIPTION:
 *
 **/
static void isp_ch_util_dump_frame(int ion_fd, mct_stream_info_t *stream_info,
  isp_frame_buffer_t *image_buf, uint32_t frame_idx, boolean dump_to_fs)
{
  ISP_DBG(ISP_MOD_COM,"%s: E", __func__);
  int32_t enabled = 0;
  char value[PROPERTY_VALUE_MAX];
  char buf[64];
  int frm_num = 10;
  static int sDumpFrmCnt = 0;
  /* Usage: To enable dumps
    Preview: adb shell setprop persist.camera.isp.dump 2
    Snapshot: adb shell setprop persist.camera.isp.dump 8
    Video: adb shell setprop persist.camera.isp.dump 16
    To dump 10 frames again, just reset prop value to 0 and then set again */
  property_get("persist.camera.isp.dump", value, "0");
  enabled = atoi(value);
  ISP_DBG(ISP_MOD_COM,"%s:frame_idx %d enabled %d streamtype %d width %d height %d",
    __FUNCTION__, frame_idx, enabled,stream_info->stream_type,
    stream_info->dim.width, stream_info->dim.height);

  if (stream_info->stream_type == CAM_STREAM_TYPE_METADATA) {
    if (!dump_to_fs) {
      return;
    }
    enabled = 1<<CAM_STREAM_TYPE_METADATA;
    frm_num = 1;
  }
  if(!enabled){
    sDumpFrmCnt = 0;
    return;
  }

  if((1<<(int)stream_info->stream_type) & enabled) {
    CDBG_HIGH("%s: dump enabled for stream %d", __FUNCTION__,
      stream_info->stream_type);

    if(sDumpFrmCnt >= 0 && sDumpFrmCnt <= frm_num) {
      int w, h;
      int file_fd;
      w = stream_info->dim.width;
      h = stream_info->dim.height;

      switch (stream_info->stream_type) {
      case CAM_STREAM_TYPE_PREVIEW:
        snprintf(buf, sizeof(buf), "/data/misc/camera/isp_dump_%d_preview_%d_%d.yuv",
          frame_idx, w, h);
        file_fd = open(buf, O_RDWR | O_CREAT, 0777);
        break;

      case CAM_STREAM_TYPE_VIDEO:
        snprintf(buf, sizeof(buf), "/data/misc/camera/isp_dump_%d_video_%d_%d.yuv",
          frame_idx, w, h);
        file_fd = open(buf, O_RDWR | O_CREAT, 0777);
        break;

      case CAM_STREAM_TYPE_POSTVIEW:
        snprintf(buf, sizeof(buf), "/data/misc/camera/isp_dump_%d_postview_%d_%d.yuv",
          frame_idx, w, h);
        file_fd = open(buf, O_RDWR | O_CREAT, 0777);
        break;

      case CAM_STREAM_TYPE_SNAPSHOT:
        snprintf(buf, sizeof(buf), "/data/misc/camera/isp_dump_%d_snapshot_%d_%d.yuv",
          frame_idx, w, h);
        file_fd = open(buf, O_RDWR | O_CREAT, 0777);
        break;

      case CAM_STREAM_TYPE_METADATA:
        snprintf(buf, sizeof(buf), "/data/misc/camera/isp_dump_%d_embedded_%d_%d.yuv",
          frame_idx, w, h);
        file_fd = open(buf, O_RDWR | O_CREAT, 0777);
        break;

      case CAM_STREAM_TYPE_RAW:
      case CAM_STREAM_TYPE_DEFAULT:
      default:
        w = h = 0;
        file_fd = -1;
        break;
      }

      if (file_fd < 0) {
        CDBG_ERROR("%s: cannot open file\n", __func__);
      } else {
        ISP_DBG(ISP_MOD_COM,"%s: num_planes %d", __FUNCTION__, image_buf->buffer.length);
        unsigned int i = 0;
        uint8_t * vaddr = (uint8_t *)image_buf->vaddr;

        for (i=0;i<image_buf->buffer.length; i++) {
          ISP_DBG(ISP_MOD_COM,"%s:file_fd %d vaddr %x, size %d \n", __FUNCTION__, file_fd,
          (unsigned int)(vaddr + image_buf->buffer.m.planes[i].data_offset),
          image_buf->buffer.m.planes[i].length );

          write(file_fd,
            (const void *)(vaddr + image_buf->buffer.m.planes[i].data_offset),
            image_buf->buffer.m.planes[i].length);
        }

        close(file_fd);
        /* buffer is invalidated from cache*/
        isp_do_cache_inv_ion(ion_fd, image_buf);
      }
    }
    sDumpFrmCnt++;
  }

end:
  return;
}

/** isp_ch_util_send_crop_factor_param_to_hw
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_send_crop_factor_param_to_hw(
   isp_t *isp,
   isp_session_t *session,
   int isp_id);

/** isp_ch_util_add_channel
 *
 * DESCRIPTION:
 *
 **/
isp_channel_t *isp_ch_util_add_channel(isp_t *isp, uint32_t session_id,
  uint32_t stream_id, uint32_t user_stream_idx,
  mct_stream_info_t *stream_info, isp_channel_type_t channel_type)
{
  int i, rc = 0, is_new_session = 1;
  isp_session_t *session = NULL;

  ISP_DBG(ISP_MOD_COM,"%s: sessid = %d, streamid = %d type = %d\n",
    __func__, session_id, stream_id, stream_info->stream_type);
  for (i = 0; i < ISP_MAX_SESSIONS; i++) {
    if (isp->data.sessions[i].isp_data &&
      isp->data.sessions[i].session_id == session_id) {
      /* the session already in use. */
      session = &isp->data.sessions[i];
      break;
    }
  }

  if (session == NULL) {
    CDBG_ERROR("%s: no more session availabe, max = %d\n",
      __func__, ISP_MAX_SESSIONS);
    return NULL;
  }

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (session->channel[i].state == ISP_CHANNEL_STATE_INITIAL) {
      /* found an empty slot */
      memset(&session->channel[i], 0, sizeof(session->channel[i]));
      session->channel[i].session = (void *)session;
      session->channel[i].session_id = session_id;
      session->channel[i].channel_id = stream_id;
      session->channel[i].stream_info = *stream_info;
      session->channel[i].state = ISP_CHANNEL_STATE_CREATED;
      session->channel[i].channel_idx = i;
      session->channel[i].channel_type = channel_type;
      session->channel[i].user_stream_idx_mask = (1 << user_stream_idx);
      ISP_DBG(ISP_MOD_COM,"%s: channel added: ch_idx = %d, user_stream_idx = %d, "
        "sessionid = %d, channel_id = %d, user_stream_idx_mask = 0x%x",
        __func__, i, user_stream_idx, session_id, stream_id,
        session->channel[i].user_stream_idx_mask);
      return &session->channel[i];
    }
  }

  CDBG_ERROR("%s: no more channel slot in that session\n", __func__);
  return NULL;
}

/** isp_ch_util_get_channel_idx
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_get_channel_idx(isp_channel_t *channel)
{
  return channel->channel_idx;
}

/** isp_ch_util_find_channel_in_session
 *
 * DESCRIPTION:
 *
 **/
isp_channel_t * isp_ch_util_find_channel_in_session(
  isp_session_t *sess, uint32_t channel_id)
{
  int i;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (sess->channel[i].channel_id == channel_id &&
        sess->channel[i].state != ISP_CHANNEL_STATE_INITIAL) {
      return &sess->channel[i];
    }
  }

  return NULL;
}

/** isp_ch_util_find_channel_in_session_by_idx
 *
 * DESCRIPTION:
 *
 **/
isp_channel_t * isp_ch_util_find_channel_in_session_by_idx(
  isp_session_t *session,
  int idx)
{
  return (idx < ISP_MAX_STREAMS)? &session->channel[idx] : NULL;
}

/** isp_ch_util_check_yuv_sensor
 *
 * DESCRIPTION:
 *
 **/
boolean isp_ch_util_check_yuv_sensor(isp_channel_t *channel){
  isp_info_t isp_info[VFE_MAX];
  int num_isps = 0;
  num_isps = isp_get_info(isp_info);
  /* return FALSE is we want to use PIX interface for YUV sensor */
  if (isp_info[0].use_pix_for_SOC)
    return(FALSE);
  if(!channel)
    CDBG_ERROR("%s: Error: no channel\n", __func__);
  else if ( channel->cfg.sensor_cfg.fmt >= CAM_FORMAT_YUV_RAW_8BIT_YUYV &&
            channel->cfg.sensor_cfg.fmt <= CAM_FORMAT_YUV_RAW_8BIT_VYUY)
    return(TRUE);
  return(FALSE);
}

static isp_channel_t *check_for_yuv_channel(isp_session_t *session,
  isp_stream_t *stream)
{
  int i;

  for (i = 0; i < ISP_MAX_STREAMS; i++)
    if((session->channel[i].state != ISP_CHANNEL_STATE_CREATED) &&
       (isp_ch_util_check_yuv_sensor(&(session->channel[i]))) &&
       (stream->stream_info.streaming_mode == session->channel[i].stream_info.streaming_mode) &&
       (stream->stream_info.stream_type != CAM_STREAM_TYPE_SNAPSHOT))
      return &session->channel[i];
  return NULL;
}

/** isp_ch_util_sync_stream_cfg_to_channel
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_sync_stream_cfg_to_channel(
  isp_t *isp, isp_session_t *session, isp_stream_t *stream)
{
  isp_channel_t *channel;
  isp_channel_t *prev_channel;

  channel =
    isp_ch_util_find_channel_in_session(session, stream->stream_id);

  if (!channel) {
    channel = check_for_yuv_channel(session, stream);
    if (channel) {


      channel->user_stream_idx_mask |= (1 << stream->stream_idx);

    } else {
      CDBG_ERROR("%s: cannot find channel, Adding new channel for stream id %d\n",
        __func__, stream->stream_id);
      channel = isp_ch_util_add_channel(
        isp, session-> session_id, stream->stream_id,
        stream->stream_idx, &stream->stream_info, ISP_CHANNEL_TYPE_IMAGE);
      if (channel == NULL) {
        CDBG_ERROR("%s: add channel for new stream failed\n", __func__);
        return -1;
      }
    }
    stream->channel_idx_mask |= (1 << isp_ch_util_get_channel_idx(channel));
  }

  /* deep copy cfg, sink_port and src_ports */
  channel->cfg = stream->cfg;
  channel->sink_port = stream->sink_port;
  channel->src_ports[ISP_SRC_PORT_DATA] = stream->src_ports[ISP_SRC_PORT_DATA];
  channel->src_ports[ISP_SRC_PORT_3A] = stream->src_ports[ISP_SRC_PORT_3A];
  channel->state = ISP_CHANNEL_STATE_USER_CFG;

  return 0;
}

/** isp_ch_util_get_pipeline_channel_by_mode
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_get_pipeline_channel_by_mode(
  isp_session_t *session,
  isp_channel_t *channel[ISP_MAX_STREAMS],
  cam_streaming_mode_t mode)
{
  int i;
  isp_channel_t *tmp;
  isp_port_t *isp_sink_port;
  int cnt = 0;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    tmp = &session->channel[i];
    if (tmp->sink_port == NULL)
      continue;

    isp_sink_port = tmp->sink_port->port_private;
    if (!isp_sink_port->u.sink_port.caps.use_pix)
      /* this is RDI stream. */
      continue;

    if (isp_ch_util_is_pix_raw_fmt(session->channel[i].stream_info.fmt))
      /* RAW stream. (RDI_CAMIF_IDEAL) */
      continue;

    if (tmp->stream_info.streaming_mode == mode)
      channel[cnt++] = tmp;
  }

  return cnt;
}

/** isp_ch_util_sort_channel
 *
 * DESCRIPTION:
 *
 **/
static void isp_ch_util_sort_channel(
  int num_channel,
  isp_channel_t *channel[ISP_MAX_STREAMS])
{
  int i, j, rc = 0;
  uint32_t len1, len2;
  isp_channel_t *tmp;

  for (i = 0; i < num_channel - 1; i++) {
    for (j = i + 1; j > 0; --j) {
      len1 = channel[j-1]->stream_info.dim.width *
        channel[j-1]->stream_info.dim.height;
      len2 = channel[j]->stream_info.dim.width *
      channel[j]->stream_info.dim.height;

      if (len2 > len1) {
        /* swaping position */
        tmp = channel[j-1];
        channel[j-1] = channel[j];
        channel[j] = tmp;
      }
    }
  }
}

/** isp_ch_util_check_channel_aspect_ratio
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_check_channel_aspect_ratio(
  isp_t *isp,
  isp_channel_t *channel1,
  isp_channel_t *channel2)
{
  uint32_t tmp_dim1, tmp_dim2;
  int isp_id;
  int two_aspect_ratios = 0;

  if (channel1->cfg.vfe_mask & (1 << VFE0))
    isp_id = VFE0;
  else
    isp_id = VFE1;

  /* VFE40 supports two aspect ratios */
  if (GET_ISP_MAIN_VERSION(isp->data.sd_info.sd_info[isp_id].isp_version) ==
    ISP_VERSION_40) {
    two_aspect_ratios = 1;
  }

  /* aspect ratio mismatch.
    We will return -ERANGE once we have C2D support for croping.
    For now to make CTS work we return 0 */
  tmp_dim1 = channel1->stream_info.dim.width * channel2->stream_info.dim.height;
  tmp_dim2 = channel1->stream_info.dim.height * channel2->stream_info.dim.width;
  if (!two_aspect_ratios && tmp_dim1 != tmp_dim2) {
    return 0;
  } else
    return 0;
}

/** isp_ch_util_map_user_streams_to_channel_burst
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_map_user_streams_to_channel_burst(
  isp_session_t *session, int num_channel,
  isp_channel_t *channel[ISP_MAX_STREAMS])
{
  int i, j, num = ISP_MAX_NUM_SCALER;
  uint32_t mask = 0;
  isp_stream_t *tmp;

  if (num_channel < ISP_MAX_NUM_SCALER)
    num = num_channel;

  /* decide channel type for channels*/
  channel[0]->is_encoder = TRUE;
  if (num > 1)
    channel[1]->is_encoder = FALSE;

  /* loop all channels(max 2 channel here),
     fill in channel_idx_mask in user stream*/
  for (i = 0; i < num; i++) {
    mask = channel[i]->user_stream_idx_mask;
    for (j = 0; j < ISP_MAX_STREAMS; j++) {
      if (mask & (1 << j)) {
        session->streams[j].channel_idx_mask |= (1 << channel[i]->channel_idx);
        session->streams[j].is_encoder = channel[i]->is_encoder;
      }
    }
  }

  /* take care of the rest channels if exceed 2 channels
     1. remove old association/mapping
     2. add channel to viewfinder path
     3. map stream to channel[viewfinder] by fill in user stream mask */
  for (i = ISP_MAX_NUM_SCALER; i < num_channel; i++) {
    if(channel[i]->channel_type != ISP_CHANNEL_TYPE_IMAGE) {
      continue;
    }
    mask = channel[i]->user_stream_idx_mask;
    for (j = 0; j < ISP_MAX_STREAMS; j++) {
      if (mask & (1 << j)) {
        /* remove the old association */
        session->streams[j].channel_idx_mask &= ~(1 << channel[i]->channel_idx);
        /* add to viewfinder */
        session->streams[j].channel_idx_mask |=
          (1 << channel[ISP_MAX_NUM_SCALER-1]->channel_idx);
        /* add streams[j] to channel[viewfinder] */
        channel[ISP_MAX_NUM_SCALER-1]->user_stream_idx_mask |=
          (1 << session->streams[j].stream_idx);
        session->streams[j].is_encoder =
          channel[ISP_MAX_NUM_SCALER-1]->is_encoder;
      }
    }

    isp_ch_util_del_channel(session, channel[i]);
  }
  return 0;
}

/** isp_ch_util_del_channel
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_del_channel(
  isp_session_t *session,
  isp_channel_t *channel)
{
  ISP_DBG(ISP_MOD_COM,"%s: channel = %p, sessionid = %d, channel_id = %d\n",
       __func__, channel, channel->session_id, channel->channel_id);

  memset(channel, 0, sizeof(isp_channel_t));
  return 0;
}

/** isp_ch_util_del_channel_by_mask
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_del_channel_by_mask(
  isp_session_t *session,
  uint32_t mask)
{
  int i;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if ((1 << i) & mask) {
      ISP_DBG(ISP_MOD_COM,"%s: sessionid = %d, streamid = %d\n",
        __func__, session->session_id, session->channel[i].channel_id);
      memset(&session->channel[i], 0, sizeof(isp_channel_t));
    }
  }

  return 0;
}

/** isp_ch_util_check_rotation
 *
 * DESCRIPTION:
 *
 **/
static void isp_ch_util_check_rotation(isp_session_t *session, isp_channel_t *channel)
{
  if (channel->stream_info.reprocess_config.pp_feature_config.rotation == 0 ||
      channel->stream_info.reprocess_config.pp_feature_config.rotation == ROTATE_0 ||
      channel->stream_info.reprocess_config.pp_feature_config.rotation == ROTATE_180) {
    /* if rotate 0 or 180 no op. still use HAL dimension */
    CDBG_HIGH("%s: sessioN_id = %d, stream_type = %d, channel_id = %d, "
      "no swap, width = %d, height = %d, rotation = 0x%x\n",
      __func__, session->session_id, channel->stream_info.stream_type,
      channel->channel_id, channel->stream_info.dim.width,
      channel->stream_info.dim.height,
      channel->stream_info.reprocess_config.pp_feature_config.rotation);
  } else {
    /* swap width with height */
    int32_t tmp_width = channel->stream_info.dim.width;
    channel->stream_info.dim.width = channel->stream_info.dim.height;
    channel->stream_info.dim.height = tmp_width;
    channel->use_native_buf = 1;
    CDBG_HIGH("%s: sessioN_id = %d, stream_type = %d, channel_id = %d, "
      "swaped width = %d, height = %d, rotation = 0x%x\n",
      __func__, session->session_id, channel->stream_info.stream_type,
      channel->channel_id, channel->stream_info.dim.width,
      channel->stream_info.dim.height,
      channel->stream_info.reprocess_config.pp_feature_config.rotation);
  }
}

/** isp_ch_util_check_yv12_fmt
 *
 * DESCRIPTION:
 *
 **/
static void isp_ch_util_check_yv12_fmt(
  isp_t *isp, int num_channels,
  isp_channel_t *channel[ISP_MAX_STREAMS])
{
  int i;
  uint32_t isp_ver = 0;

  isp_ver = isp->data.sd_info.sd_info[0].isp_version;


  for (i = 0; i < num_channels; i++) {
    if (channel[i]->stream_info.fmt == CAM_FORMAT_YUV_420_YV12) {
      channel[i]->stream_info.fmt = CAM_FORMAT_YUV_420_NV12;
      channel[i]->use_native_buf = TRUE;
    }
  }
}

/** isp_ch_util_select_pipeline_channel_burst
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_select_pipeline_channel_burst(
  isp_t *isp,
  isp_session_t *session)
{
  isp_channel_t *burst_channel[ISP_MAX_STREAMS];
  isp_port_t *isp_sink_port = NULL;
  isp_sink_port_t *sink_port = NULL;
  int num_burst_channel = 0, i, j, rc = 0;
  uint32_t len1, len2;
  if (session->use_pipeline == FALSE)
    return 0; /* no op */

  memset(burst_channel, 0, ISP_MAX_STREAMS * sizeof(isp_channel_t*));

  num_burst_channel = isp_ch_util_get_pipeline_channel_by_mode(
    session, burst_channel, CAM_STREAMING_MODE_BURST);

  /* no channel found, RDI case */
  if (num_burst_channel == 0)
    return 0;

  isp_ch_util_sort_channel(num_burst_channel, burst_channel);
  /* min dimension check by the max scaling factor 16x,
     min output dim = camif_size/16*/
  if((GET_ISP_MAIN_VERSION(isp->data.sd_info.sd_info[0].isp_version) >=
      ISP_VERSION_40)) {
    isp_ch_util_check_yv12_fmt(isp, num_burst_channel, burst_channel);
    rc = isp_ch_util_check_min_dim_for_channel(session,
           num_burst_channel, burst_channel);
  } else {
    /* main and second scaler chained together. Here we
     * need to check main scaler against camif. Then the
     * second scaler against the main scaler. */
    if (num_burst_channel > 1)
      rc = isp_ch_util_check_min_dim_for_chained_scalers(session,
           burst_channel[0], burst_channel[1]);
    else
      rc = isp_ch_util_check_min_dim_for_channel(session,
             num_burst_channel, burst_channel);
  }
  if (rc < 0) {
    CDBG_ERROR("%s: check VFE downscaling ratio error = %d\n", __func__, rc);
    return rc;
  }

  if (burst_channel[0])
    isp_ch_util_check_rotation(session, burst_channel[0]);

  if (burst_channel[1])
    isp_ch_util_check_rotation(session, burst_channel[1]);

  /* for VFE40, 2 different aspect ratio is allowed;
     for older target, the 2 aspect ratio need to be the same*/
  if (burst_channel[1]) {
    rc = isp_ch_util_check_channel_aspect_ratio(isp,
      burst_channel[0], burst_channel[1]);
    if (rc < 0)
      return rc;
  }

  isp_ch_util_map_user_streams_to_channel_burst(session,
    num_burst_channel, burst_channel);
  return 0;
}

/** isp_ch_util_find_burst_channel_in_continuous_mode
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_find_burst_channel_in_continuous_mode(
  isp_session_t *session, isp_channel_t **channel_ptr)
{
  int i;
  int burst_cnt = 0;

  *channel_ptr = NULL;
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (session->channel[i].sink_port == NULL ||
      session->channel[i].channel_type != ISP_CHANNEL_TYPE_IMAGE) {
      continue;
    }
    if (session->channel[i].stream_info.streaming_mode ==
        CAM_STREAMING_MODE_BURST) {
      isp_port_t *isp_sink_port = NULL;

      if (isp_ch_util_is_pix_raw_fmt(session->channel[i].stream_info.fmt)) {
        /*RAW stream. (RDI/CAMIF/IDEAL)*/
        continue;
      }

      /* only pix interface(non camif raw) needed */
      isp_sink_port = session->channel[i].sink_port->port_private;
      if (isp_sink_port->u.sink_port.caps.use_pix) {
        *channel_ptr = &session->channel[i];
        burst_cnt++;
      }
    }
  }

  /* when doing continuous streaming there is only
   * one possible burst channel which is live shot */
  if (burst_cnt <= 1) {
    return 0;
  } else {
    *channel_ptr = NULL;
    CDBG_ERROR("%s: Error, only allow one burst channel, cnt = %d\n",
      __func__, burst_cnt);
    return -1;
  }
}

/** isp_ch_util_get_pipeline_channel_ptr_by_mode
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_get_pipeline_channel_ptr_by_mode(isp_session_t *session,
  isp_channel_t *channel[ISP_MAX_STREAMS], cam_streaming_mode_t mode)
{
  int i;
  isp_channel_t *tmp_ch;
  isp_port_t *isp_sink_port;
  int cnt = 0;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    tmp_ch = &session->channel[i];

    if (session->channel[i].sink_port == NULL)
      continue;

    isp_sink_port = tmp_ch->sink_port->port_private;
    if (!isp_sink_port->u.sink_port.caps.use_pix)
      /* RDI stream. */
      continue;

    if (isp_ch_util_is_pix_raw_fmt(session->channel[i].stream_info.fmt))
      /* RAW stream. (RDI/CAMIF/IDEAL)*/
      continue;

    if (tmp_ch->channel_type != ISP_CHANNEL_TYPE_IMAGE) {
      CDBG_HIGH("%s: session_id = %d, ch_id = %d, ch_type = %d",
        __func__, tmp_ch->session_id, tmp_ch->channel_id, tmp_ch->channel_type);
      continue;
    }

    if (tmp_ch->stream_info.streaming_mode == mode)
      channel[cnt++] = tmp_ch;
  }

  return cnt;
}

/** isp_ch_util_decide_super_dim_of_continuous_channel
 *    @session: session context in which parameter should be saved
 *    @num_streams: number of active continuous streams
 *    @streams: the array of continuous streams
 *
 *  Decides if superset of stream dimensions is needed and changes
 *  them accordingly
 **/
static int isp_ch_util_decide_super_dim_of_continuous_channel(
  isp_session_t *session, int num_channel, isp_channel_t **channel)
{
  int rc = 0;
  isp_channel_t *channel0, *channel1;

 /* more than one continuous channels linked with PP.
    In this use case ISP always use native buffer.
    (In theory, if the 1st streamon channel has the larger resolution,
    we could avoid native buffer. But that makes the code logic more complex)*/
  if ((num_channel > 1) && (channel[0]->src_ports[ISP_SRC_PORT_DATA] &&
    channel[0]->src_ports[ISP_SRC_PORT_DATA] ==
    channel[1]->src_ports[ISP_SRC_PORT_DATA])) {
    channel0 = channel[0];
    channel1 = channel[1];
    channel0->use_native_buf = 1;
    channel1->use_native_buf = 1;

    if (channel0->stream_info.dim.width < channel1->stream_info.dim.width)
      channel0->stream_info.dim.width = channel1->stream_info.dim.width;

    if (channel0->stream_info.dim.height < channel1->stream_info.dim.height)
      channel0->stream_info.dim.height = channel1->stream_info.dim.height;
  }
  return 0;
}

/** isp_ch_util_get_count_pp_linked_channel
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_get_count_pp_linked_channel(
  int num_channel, isp_channel_t **channel)
{
  int i, cnt = 0;

  for (i = 0; i < num_channel; i++)
    if (channel[i] && channel[i]->src_ports[ISP_SRC_PORT_DATA])
      cnt++;

  return cnt;
}

/** isp_ch_util_find_camif_dim
 *
 * DESCRIPTION:
 *
 **/
static void isp_ch_util_find_camif_dim(isp_sink_port_t *sink_port,
  sensor_out_info_t *sensor_output, cam_dimension_t *dim)
{
  uint32_t primary_cid_idx;
  primary_cid_idx = isp_hw_find_primary_cid(sensor_output,
    &sink_port->caps.sensor_cap);

  if (SENSOR_CID_CH_MAX - 1 < primary_cid_idx) {
    CDBG_ERROR("%s:%d error out of range\n", __func__, __LINE__);
    return;
  }
  memset(dim,  0,  sizeof(cam_dimension_t));
  /* sensor always has cid[0] index */
  if (sink_port->caps.sensor_cap.sensor_cid_ch[primary_cid_idx].is_bayer_sensor)
    dim->width = sensor_output->request_crop.last_pixel -
      sensor_output->request_crop.first_pixel + 1;
  else
    dim->width = (sensor_output->request_crop.last_pixel -
      sensor_output->request_crop.first_pixel + 1) >> 1;

  dim->height = sensor_output->request_crop.last_line -
    sensor_output->request_crop.first_line + 1;
}

/** isp_ch_util_check_min_width_height
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_check_min_width_height(
  isp_sink_port_t *sink_port, sensor_out_info_t *sensor_output,
  int32_t *width, int32_t *height, boolean *use_native_buf)
{
  int32_t camif_w, camif_h;
  int32_t delta_width = *width, delta_height = *height;
  int32_t tmp_width = 0, tmp_height = 0;
  uint32_t primary_cid_idx = isp_hw_find_primary_cid(sensor_output,
    &sink_port->caps.sensor_cap);

  if (SENSOR_CID_CH_MAX - 1 < primary_cid_idx) {
    CDBG_ERROR("%s:%d error out of range\n", __func__, __LINE__);
    return -1;
  }

  if (sink_port->caps.sensor_cap.num_cid_ch < 1) {
    CDBG_ERROR("%s: error, sensor num_cid_ch = 0\n", __func__);
    return -1;
  }
  /* this implementation is used for straems using ISP pipeline */
  if (!sink_port->caps.use_pix)
    return 0;

  /* sensor always has cid[0] index */
  if (sink_port->caps.sensor_cap.sensor_cid_ch[primary_cid_idx].is_bayer_sensor)
    camif_w = sensor_output->request_crop.last_pixel -
      sensor_output->request_crop.first_pixel + 1;
  else
    camif_w = (sensor_output->request_crop.last_pixel -
      sensor_output->request_crop.first_pixel + 1) >> 1;

  camif_h = sensor_output->request_crop.last_line -
    sensor_output->request_crop.first_line + 1;

  tmp_width = CEILING16(camif_w/MAX_VFE_SCALERS_RATIO);
  tmp_height = CEILING16(camif_h/MAX_VFE_SCALERS_RATIO);

  if (tmp_width > *width || tmp_height > *height) {
    /* adjust width and height to make sure it fits scaler limitation */
    while (tmp_width > *width || tmp_height > *height) {
      /* out side of scaler limitation */
      *width += delta_width;
      *height += delta_height;
      *use_native_buf = TRUE;
    }
  }

  return 0;
}

/** isp_ch_util_check_min_width_height_2nd_scaler
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_check_min_width_height_2nd_scaler(
  isp_channel_t *encoder_channel, isp_channel_t *view_finder)
{
  int32_t w, h;
  int32_t delta_width = view_finder->stream_info.dim.width;
  int32_t delta_height = view_finder->stream_info.dim.height;
  int32_t tmp_width = 0, tmp_height = 0;

  w = encoder_channel->stream_info.dim.width;
  h = encoder_channel->stream_info.dim.height;
  tmp_width = CEILING16(encoder_channel->stream_info.dim.width/MAX_VFE_SCALERS_RATIO);
  tmp_height = CEILING16(encoder_channel->stream_info.dim.height/MAX_VFE_SCALERS_RATIO);

  if (tmp_width > view_finder->stream_info.dim.width ||
         tmp_height > view_finder->stream_info.dim.height) {
    /* adjust width and height to amke sure it fits scaler limitation */
    while (tmp_width > view_finder->stream_info.dim.width ||
      tmp_height > view_finder->stream_info.dim.height) {
      /* outside of scaler limitation */
      view_finder->stream_info.dim.width += delta_width;
      view_finder->stream_info.dim.height += delta_height;
      view_finder->use_native_buf = TRUE;
    }
  }

  return 0;
}

/** isp_ch_util_modify_channel_plane_info_for_native_buf
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_modify_channel_plane_info_for_native_buf(
  isp_channel_t *channel)
{
  cam_stream_buf_plane_info_t *buf_planes = &channel->stream_info.buf_planes;
  cam_dimension_t *dim = &channel->stream_info.dim;
  int stride = 0, scanline = 0;
  int rc = 0;

  memset(buf_planes, 0, sizeof(cam_stream_buf_plane_info_t));
  switch (channel->stream_info.fmt) {
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

    case CAM_FORMAT_YUV_RAW_8BIT_YUYV:
    case CAM_FORMAT_YUV_RAW_8BIT_YVYU:
    case CAM_FORMAT_YUV_RAW_8BIT_UYVY:
    case CAM_FORMAT_YUV_RAW_8BIT_VYUY:
    case CAM_FORMAT_JPEG_RAW_8BIT:
    case CAM_FORMAT_META_RAW_8BIT: {
      /* 1 plane */
      /* Every 16 pixels occupy 16 bytes */
      stride = PAD_TO_SIZE(dim->width, CAM_PAD_TO_16);
      buf_planes->plane_info.num_planes = 1;
      buf_planes->plane_info.mp[0].offset = 0;
      buf_planes->plane_info.mp[0].len =
        PAD_TO_SIZE(stride * scanline * 2, CAM_PAD_TO_4);
      buf_planes->plane_info.frame_len =
        PAD_TO_SIZE(buf_planes->plane_info.mp[0].len, CAM_PAD_TO_4K);
      buf_planes->plane_info.mp[0].offset_x =0;
      buf_planes->plane_info.mp[0].offset_y = 0;
      buf_planes->plane_info.mp[0].stride = stride;
      buf_planes->plane_info.mp[0].scanline = scanline;
    }
      break;

    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_BGGR: {
      /* 1 plane */
      /* Every 16 pixels occupy 16 bytes */
      stride = PAD_TO_SIZE(dim->width, CAM_PAD_TO_16);
      buf_planes->plane_info.num_planes = 1;
      buf_planes->plane_info.mp[0].offset = 0;
      buf_planes->plane_info.mp[0].len =
        PAD_TO_SIZE(stride * scanline, CAM_PAD_TO_4);
      buf_planes->plane_info.frame_len =
        PAD_TO_SIZE(buf_planes->plane_info.mp[0].len, CAM_PAD_TO_4K);
      buf_planes->plane_info.mp[0].offset_x =0;
      buf_planes->plane_info.mp[0].offset_y = 0;
      buf_planes->plane_info.mp[0].stride = stride;
      buf_planes->plane_info.mp[0].scanline = scanline;
    }
      break;

    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_BGGR: {
      /* Every 12 pixels occupy 16 bytes */
      stride = (dim->width + 11)/12 * 12;
      buf_planes->plane_info.num_planes = 1;
      buf_planes->plane_info.mp[0].offset = 0;
      buf_planes->plane_info.mp[0].len =
        PAD_TO_SIZE(stride * scanline * 8 / 6, CAM_PAD_TO_4);
      buf_planes->plane_info.frame_len =
        PAD_TO_SIZE(buf_planes->plane_info.mp[0].len, CAM_PAD_TO_4K);
      buf_planes->plane_info.mp[0].offset_x =0;
      buf_planes->plane_info.mp[0].offset_y = 0;
      buf_planes->plane_info.mp[0].stride = stride;
      buf_planes->plane_info.mp[0].scanline = scanline;
    }
      break;

    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_BGGR: {
      /* Every 10 pixels occupy 16 bytes */
      stride = (dim->width + 9)/10 * 10;
      buf_planes->plane_info.num_planes = 1;
      buf_planes->plane_info.mp[0].offset = 0;
      buf_planes->plane_info.mp[0].len =
        PAD_TO_SIZE(stride * scanline * 8 / 5, CAM_PAD_TO_4);
      buf_planes->plane_info.frame_len =
        PAD_TO_SIZE(buf_planes->plane_info.mp[0].len, CAM_PAD_TO_4K);
      buf_planes->plane_info.mp[0].offset_x =0;
      buf_planes->plane_info.mp[0].offset_y = 0;
      buf_planes->plane_info.mp[0].stride = stride;
      buf_planes->plane_info.mp[0].scanline = scanline;
    }
      break;
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_BGGR:
    case CAM_FORMAT_META_RAW_10BIT: {
      /* Every 64 pixels occupy 80 bytes */
      stride = PAD_TO_SIZE(dim->width * 5 / 4, CAM_PAD_TO_8);
      buf_planes->plane_info.num_planes = 1;
      buf_planes->plane_info.mp[0].offset = 0;
      buf_planes->plane_info.mp[0].len =
        PAD_TO_SIZE(stride * scanline, CAM_PAD_TO_4);
      buf_planes->plane_info.frame_len =
        PAD_TO_SIZE(buf_planes->plane_info.mp[0].len, CAM_PAD_TO_4K);
      buf_planes->plane_info.mp[0].offset_x =0;
      buf_planes->plane_info.mp[0].offset_y = 0;
      buf_planes->plane_info.mp[0].stride = stride;
      buf_planes->plane_info.mp[0].scanline = scanline;
    }
      break;
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_BGGR: {
      /* Every 32 pixels occupy 48 bytes */
      stride = PAD_TO_SIZE(dim->width, CAM_PAD_TO_32);
      buf_planes->plane_info.num_planes = 1;
      buf_planes->plane_info.mp[0].offset = 0;
      buf_planes->plane_info.mp[0].len =
        PAD_TO_SIZE(stride * scanline * 3 / 2, CAM_PAD_TO_4);
      buf_planes->plane_info.frame_len =
        PAD_TO_SIZE(buf_planes->plane_info.mp[0].len, CAM_PAD_TO_4K);
      buf_planes->plane_info.mp[0].offset_x =0;
      buf_planes->plane_info.mp[0].offset_y = 0;
      buf_planes->plane_info.mp[0].stride = stride;
      buf_planes->plane_info.mp[0].scanline = scanline;
    }
      break;

    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_BGGR: {
      /* Every 8 pixels occupy 16 bytes */
      stride = PAD_TO_SIZE(dim->width, CAM_PAD_TO_8);
      buf_planes->plane_info.num_planes = 1;
      buf_planes->plane_info.mp[0].offset = 0;
      buf_planes->plane_info.mp[0].len =
        PAD_TO_SIZE(stride * scanline * 2, CAM_PAD_TO_4);
      buf_planes->plane_info.frame_len =
        PAD_TO_SIZE(buf_planes->plane_info.mp[0].len, CAM_PAD_TO_4K);
      buf_planes->plane_info.mp[0].offset_x =0;
      buf_planes->plane_info.mp[0].offset_y = 0;
      buf_planes->plane_info.mp[0].stride = stride;
      buf_planes->plane_info.mp[0].scanline = scanline;
    }
      break;

  default:
    CDBG_ERROR("%s: Invalid cam_format %d",
      __func__, channel->stream_info.fmt);
    rc = -1;
    break;
  }

  return rc;
}

/** isp_ch_util_check_max_width_height
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_check_max_width_height(isp_sink_port_t *sink_port,
  sensor_out_info_t *sensor_output, int32_t *width, int32_t *height,
  boolean *use_native_buf, cam_format_t fmt)
{
  int32_t camif_w, camif_h;
  int32_t in_width = 0, in_height = 0;
  float in_aspect_ratio, camif_aspect_ratio;
  uint32_t primary_cid_idx;

  primary_cid_idx = isp_hw_find_primary_cid(sensor_output, &sink_port->caps.sensor_cap);

  if (SENSOR_CID_CH_MAX - 1 < primary_cid_idx) {
    CDBG_ERROR("%s:%d error out of range\n", __func__, __LINE__);
    return -1;
  }

  /* this implementation is used for straems using ISP pipeline */
  if (!sink_port->caps.use_pix)
    return 0;

  if (sink_port->caps.sensor_cap.num_cid_ch < 1) {
    CDBG_ERROR("%s: error, sensor num_cid_ch = 0\n", __func__);
    return -1;
  }

  *use_native_buf = FALSE;

  /* sensor always has cid[0] index
     YUV sensor has double width, so need to devide by 2*/
  if (sink_port->caps.sensor_cap.sensor_cid_ch[primary_cid_idx].is_bayer_sensor)
    camif_w = sensor_output->request_crop.last_pixel -
      sensor_output->request_crop.first_pixel + 1;
  else
    camif_w = (sensor_output->request_crop.last_pixel -
      sensor_output->request_crop.first_pixel + 1) >> 1;

  camif_h = sensor_output->request_crop.last_line -
    sensor_output->request_crop.first_line + 1;

  if (*width <= camif_w && *height <= camif_h)
    return 0;

  /* stream output larger than camif input. Need to round to camif input*/
  in_width = (*width);
  in_height = (*height);

  /* keep the same aspect as before rounding*/
  if (FALSE == isp_ch_util_is_pix_raw_fmt(fmt)) {
    in_aspect_ratio = (float) in_width/ (float) in_height;
    camif_aspect_ratio = (float) camif_w / (float) camif_h;
    if (in_aspect_ratio <= camif_aspect_ratio) {
      *height = FLOOR32(camif_h);
      *width = FLOOR32((uint32_t)((float)(*height) * in_aspect_ratio));
    } else if (in_aspect_ratio >= camif_aspect_ratio) {
      *width = FLOOR32(camif_w);
      *height = FLOOR32((uint32_t)((float)(*width) / in_aspect_ratio));
    } else {
      CDBG_ERROR("%s: cannot adjust width/height with stream aspect ratio\n",
        __func__);
      return -1;
    }
  }

  *use_native_buf = TRUE;

  return 0;
}

/** isp_ch_util_check_camif_size_and_decide_dis_for_channel
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_check_camif_size_and_decide_dis_for_channel(
  isp_session_t *session, int num_channel, isp_channel_t **channel)
{
  int rc = 0;
  isp_channel_t *ch;
  float margin;
  int pp_link_cnt = 0;
  boolean use_native_buf = FALSE;

  if (session->dis_param.dis_enable == FALSE)
    return rc;

  pp_link_cnt =
    isp_ch_util_get_count_pp_linked_channel(num_channel, channel);

  ch = isp_ch_util_find_channel_in_session(session,
    session->dis_param.stream_id);
  if (!ch) {
    CDBG_ERROR("%s: cannot find stream %d\n",
      __func__, session->dis_param.stream_id);
    return rc;
  }

  /* step 1: check camif size before doing DIS:
             if more than one stream linked with PP,
             use higher resolution stream for DIS */
  if (ch->src_ports[ISP_SRC_PORT_DATA] && pp_link_cnt > 1) {
    ch = channel[0];
  }

  if (!session->isp_fast_aec_mode) {
    rc = isp_ch_util_check_max_width_height(
           &((isp_port_t *)ch->sink_port->port_private)->u.sink_port,
           &ch->cfg.sensor_cfg, &ch->stream_info.dim.width,
           &ch->stream_info.dim.height, &use_native_buf, ch->stream_info.fmt);
  }

  if (use_native_buf) {
    ch->use_native_buf = use_native_buf;
    rc = isp_ch_util_modify_channel_plane_info_for_native_buf(ch);
  }

  session->dis_param.width = ch->stream_info.dim.width;
  session->dis_param.height = ch->stream_info.dim.height;
  /* Step 2: decide DIS dimension
             add margin on the stream dimesion */
  margin = (float)MARGIN_P_10 / 100;
  ch->stream_info.dim.width =
    CEILING32((ch->stream_info.dim.width) +
    (uint16_t)(ch->stream_info.dim.width * margin));
  ch->stream_info.dim.height =
    CEILING32((ch->stream_info.dim.height) +
    (uint16_t)(ch->stream_info.dim.height * margin));

  /*check again, see if dimension go over camif after DIS margin add-on*/
  rc = isp_ch_util_check_max_width_height(
    &((isp_port_t *)ch->sink_port->port_private)->u.sink_port,
    &ch->cfg.sensor_cfg, &ch->stream_info.dim.width,&ch->stream_info.dim.height,
    &use_native_buf, ch->stream_info.fmt);
  if (rc < 0) {
    CDBG_ERROR("%s: cannot adjust straem dimension for DIS"
      " (sessid = %d, channel_id = %d)\n",
      __func__, ch->session_id, ch->channel_id);
    return rc;
  }

  /* use native buf if DIS is on,
     modify plan info again based on dis dimension*/
  ch->use_native_buf = 1;
  ch->dis_enable = TRUE;
  rc = isp_ch_util_modify_channel_plane_info_for_native_buf(ch);

  return rc;
}

/** isp_ch_util_check_min_dim_for_channel
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_check_min_dim_for_channel(
  isp_session_t *session, int num_channel, isp_channel_t **channel)
{
  int i, rc = 0;

  for (i = 0; i < num_channel; i++) {
    rc = 0;
    if (channel[i]->channel_type != ISP_CHANNEL_TYPE_META_DATA &&
        channel[i]->src_ports[ISP_SRC_PORT_DATA]) {
      rc = isp_ch_util_check_min_width_height(
        &((isp_port_t *)channel[i]->sink_port->port_private)->u.sink_port,
        &channel[i]->cfg.sensor_cfg, &channel[i]->stream_info.dim.width,
        &channel[i]->stream_info.dim.height, &channel[i]->use_native_buf);
    }

    if (rc >= 0 && channel[i]->use_native_buf) {
      rc = isp_ch_util_modify_channel_plane_info_for_native_buf(channel[i]);
    }
  }

  return 0;
}

/** isp_ch_util_check_min_dim_for_chained_scalers
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_check_min_dim_for_chained_scalers(
  isp_session_t *session, isp_channel_t *continuous_channel,
  isp_channel_t *burst_channel)
{
  isp_channel_t *channel[ISP_MAX_NUM_SCALER];
  uint32_t tmp_dim1, tmp_dim2;
  int rc = 0;

  tmp_dim1 = continuous_channel->stream_info.dim.width *
    continuous_channel->stream_info.dim.height;
  tmp_dim2 = burst_channel->stream_info.dim.width *
    burst_channel->stream_info.dim.height;
  if (tmp_dim2 >= tmp_dim1) {
    channel[0] = burst_channel;
    channel[1] = continuous_channel;
  } else {
    channel[1] = burst_channel;
    channel[0] = continuous_channel;
  }

  /*check min dim for channel 1 and 2*/
  rc = isp_ch_util_check_min_dim_for_channel(
    session, 1, &channel[0]);
  if (rc < 0)
    return rc;
  if (rc >= 0 && channel[0]->use_native_buf) {
    rc = isp_ch_util_modify_channel_plane_info_for_native_buf(channel[0]);
  }

  rc = isp_ch_util_check_min_width_height_2nd_scaler(
     channel[0], channel[1]);
  if (rc >= 0 && channel[1]->use_native_buf) {
    rc = isp_ch_util_modify_channel_plane_info_for_native_buf(channel[1]);
  }

  return 0;
}

/** isp_ch_util_map_user_streams_to_channel_continuous
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_map_user_streams_to_channel_continuous(
  int num_continuous_channel, isp_session_t *session,
  isp_channel_t *channel[ISP_MAX_STREAMS],
  isp_channel_t *liveshot_burst_channel)
{
  int i, j, k = 0;

  boolean is_video_hint = FALSE;
  for (k = 0; k < num_continuous_channel; k++) {
    if (channel[k]->stream_info.stream_type  == CAM_STREAM_TYPE_VIDEO) {
      is_video_hint = TRUE;
      break;
    }
  }

  /* we have a burst channel when doing continuous streamming.
   * In this case, only use one ISP output for continuous streams */
  if (liveshot_burst_channel) {
    uint32_t burst_size = liveshot_burst_channel->stream_info.dim.height *
      liveshot_burst_channel->stream_info.dim.width;
    uint32_t continuous_size = channel[0]->stream_info.dim.height *
      channel[0]->stream_info.dim.width;
    channel[0]->is_encoder = TRUE;

    /* put larger size of burst or continuous to be encoder path
       the smaller one to be viewfinder*/
    if (burst_size > continuous_size) {
      liveshot_burst_channel->is_encoder = TRUE;
      channel[0]->is_encoder = FALSE;
    } else {
      liveshot_burst_channel->is_encoder = FALSE;
      channel[0]->is_encoder = TRUE;
    }

    /* always use native buffer if two continuous streams
     * are put in one src data port */
    for (i = 0; i < ISP_MAX_STREAMS; i++) {
      if (channel[0]->user_stream_idx_mask & ( 1 << i)) {
        /* user stream idx i in the mask */
        session->streams[i].channel_idx_mask |= (1 << channel[0]->channel_idx);
        session->streams[i].is_encoder = channel[0]->is_encoder;
      }
    }

    for (i = MAX_NUM_CONTINUOS_CH_LIVESHOT; i < num_continuous_channel; i++) {
      if(channel[i]->channel_type != ISP_CHANNEL_TYPE_IMAGE) {
        continue;
      }

      for (j = 0; j < ISP_MAX_STREAMS; j++) {
        if ((1 << j) & channel[i]->user_stream_idx_mask) {
          /* J in mask. Remove j and i association */
          session->streams[j].channel_idx_mask &= ~(1 << channel[i]->channel_idx);
          /* Add  j to straem2[0] */
          session->streams[j].channel_idx_mask |= (1 << channel[0]->channel_idx);
          session->streams[j].is_encoder = channel[0]->is_encoder;
          channel[0]->user_stream_idx_mask |= (1 << j);
          channel[i]->user_stream_idx_mask &= ~(1 << j);
        }
      }

      isp_ch_util_del_channel(session, channel[i]);
    }
  } else if (is_video_hint &&
             (isp_util_is_4k2k_resolution_set(channel[0]->stream_info.dim) ||
              session->saved_params.lowpowermode_yuv_enable)) {
    uint32_t continuous_size = channel[0]->stream_info.dim.height *
         channel[0]->stream_info.dim.width;
    channel[0]->is_encoder = TRUE;
    /* always use native buffer if two continuous streams
     * are put in one src data port */
    for (i = 0; i < ISP_MAX_STREAMS; i++) {
      if (channel[0]->user_stream_idx_mask & ( 1 << i)) {
        /* user stream idx i in the mask */
        session->streams[i].channel_idx_mask |= (1 << channel[0]->channel_idx);
        session->streams[i].is_encoder = channel[0]->is_encoder;
      }
    }

    for (i = MAX_NUM_CONTINUOS_CH_LIVESHOT; i < num_continuous_channel; i++) {
      if(channel[i]->channel_type != ISP_CHANNEL_TYPE_IMAGE) {
        continue;
      }

      for (j = 0; j < ISP_MAX_STREAMS; j++) {
        if ((1 << j) & channel[i]->user_stream_idx_mask) {
          /* J in mask. Remove j and i association */
          session->streams[j].channel_idx_mask &= ~(1 << channel[i]->channel_idx);
          /* Add  j to straem2[0] */
          session->streams[j].channel_idx_mask |= (1 << channel[0]->channel_idx);
          session->streams[j].is_encoder = channel[0]->is_encoder;
          channel[0]->user_stream_idx_mask |= (1 << j);
          channel[i]->user_stream_idx_mask &= ~(1 << j);
        }
      }
      isp_ch_util_del_channel(session, channel[i]);
    }
  } else {
    /* NO liveshot burst channel:
       two continuous streams use case.
       Here we borrow the burst mapping function. */
    isp_ch_util_map_user_streams_to_channel_burst(session,
      num_continuous_channel, channel);
  }

  return 0;
}

/** isp_ch_util_select_pipeline_channel_continuous
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_select_pipeline_channel_continuous(
  isp_t *isp, isp_session_t *session)
{
  isp_channel_t *continuous_channel[ISP_MAX_STREAMS];
  isp_channel_t *burst_channel = NULL;
  isp_port_t *isp_sink_port = NULL;
  isp_sink_port_t *sink_port = NULL;
  int num_continuous_channel = 0, i, j, rc = 0;
  uint32_t len1, len2;
  boolean has_rotation = FALSE;

  if (session->use_pipeline == FALSE)
    return 0; /* no op */

  memset(continuous_channel, 0, ISP_MAX_STREAMS * sizeof(isp_channel_t *));

  rc = isp_ch_util_find_burst_channel_in_continuous_mode(session, &burst_channel);
  if (rc != 0) {
    CDBG_ERROR("%s: error in counting burst pix channel\n", __func__);
    return rc;
  }

  num_continuous_channel =
    isp_ch_util_get_pipeline_channel_ptr_by_mode(session,
    continuous_channel, CAM_STREAMING_MODE_CONTINUOUS);

  /* RDI case */
  if (num_continuous_channel == 0)
    return 0;
  /* there are use cases that one stream needs rotation but the other
   * stream does need stream. Here we use pproc's rotation flag to swap
   * image width and height. After swaping the width and height VFE uses
   * its native buffer for output image
   */
  for (i = 0; i < num_continuous_channel; i++) {
    if (continuous_channel[i]->stream_info.pp_config.rotation == ROTATE_90 ||
        continuous_channel[i]->stream_info.pp_config.rotation == ROTATE_270) {
      int tmp = continuous_channel[i]->stream_info.dim.height;
      continuous_channel[i]->stream_info.dim.height =
        continuous_channel[i]->stream_info.dim.width;
      continuous_channel[i]->stream_info.dim.width = tmp;
      continuous_channel[i]->use_native_buf = TRUE;
      has_rotation = TRUE;
      tmp = continuous_channel[i]->stream_info.buf_planes.plane_info.mp[0].stride;
      continuous_channel[i]->stream_info.buf_planes.plane_info.mp[0].stride =
          continuous_channel[i]->stream_info.buf_planes.plane_info.mp[0].scanline;
      continuous_channel[i]->stream_info.buf_planes.plane_info.mp[0].scanline = tmp;
    }
  }
  isp_ch_util_sort_channel(num_continuous_channel, continuous_channel);
  /* if one stream needs rotation VFE switches to use native buffer */
  if (has_rotation)
    continuous_channel[0]->use_native_buf = TRUE;
  isp_ch_util_check_yv12_fmt(isp, num_continuous_channel, continuous_channel);
  isp_ch_util_decide_super_dim_of_continuous_channel(
    session, num_continuous_channel, continuous_channel);

  rc = isp_ch_util_check_camif_size_and_decide_dis_for_channel(session,
    num_continuous_channel, continuous_channel);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_util_decide_dis error = %d\n", __func__, rc);
    return rc;
  }

  /* min dimension check by the max scaling factor 16x,
     min output dim = camif_size/16*/
  if((GET_ISP_MAIN_VERSION(isp->data.sd_info.sd_info[0].isp_version) >=
      ISP_VERSION_40)) {
    rc = isp_ch_util_check_min_dim_for_channel(session,
           num_continuous_channel, continuous_channel);
    if (rc < 0)
      return rc;
    if (burst_channel) {
      rc = isp_ch_util_check_min_dim_for_channel(session,
           1, &burst_channel);
      if (rc < 0)
        return rc;
    }
  } else {
    /* main and second scaler chained together. Here we
     * need to check main scaler against camif. Then the
     * second scaler against the main scaler. */
    rc = isp_ch_util_check_min_dim_for_channel(session,
           num_continuous_channel, continuous_channel);
    if (rc < 0)
      return rc;
    if (burst_channel) {
      rc = isp_ch_util_check_min_dim_for_chained_scalers(session,
           continuous_channel[0], burst_channel);
      if (rc < 0)
        return rc;
    }

  }

  if (burst_channel) {
    rc = isp_ch_util_check_channel_aspect_ratio(isp,
      continuous_channel[0], burst_channel);
    if (rc < 0)
      return rc;
  } else if (continuous_channel[1]) {
    rc = isp_ch_util_check_channel_aspect_ratio(isp,
      continuous_channel[0], continuous_channel[1]);
    if (rc < 0)
      return rc;
  }

  /* if we have burst channel here, then it means liveshot burst channel*/
  isp_ch_util_map_user_streams_to_channel_continuous(num_continuous_channel,
    session, continuous_channel, burst_channel);

  return 0;
}

/** isp_ch_util_config_for_yuv_sensor
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_config_for_yuv_sensor(
  isp_t *isp, isp_session_t *session)
{
  int i;
  isp_channel_t *channel;
  isp_port_t *isp_sink_port;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    channel = &session->channel[i];
    if (channel->sink_port == NULL)
      continue;

    if (isp_ch_util_check_yuv_sensor(channel)) {

      mct_stream_info_t *stream_info = &channel->stream_info;
      cam_stream_buf_plane_info_t *buf_planes = &stream_info->buf_planes;
      sensor_dim_output_t *dim = &channel->cfg.sensor_cfg.dim_output;
      int stride = 0, scanline = 0;

      channel->use_native_buf = TRUE;

      stride = PAD_TO_SIZE(dim->width, CAM_PAD_TO_16);
      scanline = PAD_TO_SIZE(dim->height, CAM_PAD_TO_2);

      stream_info->fmt = channel->cfg.sensor_cfg.fmt;
      stream_info->dim.width = dim->width / 2;
      stream_info->dim.height = dim->height;

      memset(buf_planes, 0, sizeof(cam_stream_buf_plane_info_t));
      /* 1 plane: YCbCr */
      buf_planes->plane_info.num_planes = 1;
      buf_planes->plane_info.mp[0].offset = 0;
      buf_planes->plane_info.mp[0].len = stride * scanline;
      buf_planes->plane_info.mp[0].stride = stride / 2;
      buf_planes->plane_info.mp[0].scanline = scanline;
      buf_planes->plane_info.frame_len =
        PAD_TO_SIZE(buf_planes->plane_info.mp[0].len,
        CAM_PAD_TO_4K);
    }
  }
  return 0;
}

/** isp_ch_util_check_stream_camif_dim
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_check_stream_camif_dim(
  isp_t *isp, isp_session_t *session)
{
  int i;
  isp_channel_t *channel;
  isp_port_t *isp_sink_port;
  cam_dimension_t camif_dim;
  uint32_t tmp1, tmp2;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    channel = &session->channel[i];
    if (channel->sink_port == NULL)
      continue;

    isp_sink_port = channel->sink_port->port_private;

    /* If RDI stream or co-paramter channel. Need to ignore. */
    if (!isp_sink_port->u.sink_port.caps.use_pix ||
      channel->channel_type != ISP_CHANNEL_TYPE_IMAGE)
      continue;

    isp_ch_util_find_camif_dim(&isp_sink_port->u.sink_port,
      &channel->cfg.sensor_cfg, &camif_dim);

    /* channel stream info should not exceed camif input*/
    if (channel->stream_info.dim.width > camif_dim.width ||
      channel->stream_info.dim.height > camif_dim.height) {
      /* In case of ZSL snapshot, we also can output less than
         stream resolution, as we have PPROC offline. */
      if ((channel->stream_info.stream_type != CAM_STREAM_TYPE_SNAPSHOT) ||
           (channel->stream_info.streaming_mode !=
                  CAM_STREAMING_MODE_CONTINUOUS)) {
        /* We can output less than stream info, only if there is PPROC */
        if (channel->src_ports[ISP_SRC_PORT_DATA] == NULL) {
          CDBG_ERROR("%s: user size > camif size but no pp. error\n", __func__);
          return -1;
        }
        /* use native buffer for this channel */
        channel->use_native_buf = TRUE;
      }

      tmp1 = channel->stream_info.dim.width * camif_dim.height;
      tmp2 = channel->stream_info.dim.height * camif_dim.width;

      /* adjust channel dimension to camif size */
      if (tmp1 == tmp2) {
        channel->stream_info.dim.width = camif_dim.width;
        channel->stream_info.dim.height = camif_dim.height;
      } else if (tmp1 > tmp2) {
        /* thin image */
        channel->stream_info.dim.height = camif_dim.width *
          channel->stream_info.dim.height / channel->stream_info.dim.width;
        channel->stream_info.dim.width = camif_dim.width;
      } else {
        channel->stream_info.dim.width = camif_dim.height *
          channel->stream_info.dim.width /channel->stream_info.dim.height;
        channel->stream_info.dim.height = camif_dim.height;
      }
    }
  }

  return 0;
}

/** isp_ch_util_select_pipeline_channel
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_select_pipeline_channel(isp_t *isp, isp_session_t *session)
{
  /* check camif dimension. If user stream is larger
   * than camif adjust to camif and mark it using native buffer */
  isp_ch_util_check_stream_camif_dim(isp, session);

  if (isp_util_is_burst_streaming(session))
    return isp_ch_util_select_pipeline_channel_burst(isp, session);
  else
    return isp_ch_util_select_pipeline_channel_continuous(isp, session);
}

/** isp_ch_util_send_channel_stripe_info_upstream
 *
 * DESCRIPTION:
 *
 **/
static void isp_ch_util_send_channel_stripe_info_upstream(
  isp_session_t *session, isp_channel_t *channel,
  ispif_out_info_t *ispif_out_info)
{
  int i;
  uint32_t mask;
  mct_event_t mct_event;

  if (channel->sink_port) {
    memset(&mct_event, 0, sizeof(mct_event));
    mct_event.u.module_event.type = MCT_EVENT_MODULE_ISPIF_OUTPUT_INFO;
    mct_event.u.module_event.module_event_data = (void *)ispif_out_info;
    mct_event.type = MCT_EVENT_MODULE_EVENT;
    mask = channel->user_stream_idx_mask;

    for (i= 0; i < ISP_MAX_STREAMS; i++) {
      if (mask & (1 << i)) {
        mask &= ~(1 << i);
        mct_event.identity =
          pack_identity(channel->session_id, session->streams[i].stream_id);
        mct_event.direction = MCT_EVENT_UPSTREAM;

        mct_port_send_event_to_peer(channel->sink_port, &mct_event);
      }

      if (!mask)
        break;
    }
  }

}

/** isp_ch_util_is_right_stripe_offset_usable
 *
 * DESCRIPTION:
 *
 **/
boolean isp_ch_util_is_right_stripe_offset_usable(
  uint32_t M, uint32_t N, uint32_t offset)
{
  uint32_t mn_init, step;
  uint32_t ratio = N / M;
  uint32_t interp_reso = 3;
  if (ratio >= 16) interp_reso = 0;
  else if (ratio >= 8) interp_reso = 1;
  else if (ratio >= 4) interp_reso = 2;

  /* upscaling */
  if (N < M)
    return TRUE;

  mn_init = offset * M % N;
  step = mn_init * (1 << (13 + interp_reso)) / M >> 13;
  if (step == 0)
    return TRUE;

  mn_init = (offset + 1) * M % N;
  step = mn_init * (1 << (13 + interp_reso)) / M >> 13;

  return (step != 0 && mn_init < M);
}

/** isp_ch_util_get_rolloff_grid_info
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_get_rolloff_grid_info(isp_t *isp,
  isp_session_t *session, uint32_t *grid_width)
{
  int rc = 0;
  uint32_t isp_id;

  if (session->vfe_ids & (1 << VFE0))
    isp_id = VFE0;
  else if (session->vfe_ids & (1 << VFE1))
    isp_id = VFE1;
  else {
    CDBG_ERROR("%s: no ISP is created yet\n", __func__);
    return -1;
  }

  if (isp->data.hw[isp_id].hw_ops) {
    rc = isp->data.hw[isp_id].hw_ops->get_params(
      isp->data.hw[isp_id].hw_ops->ctrl,
      ISP_HW_GET_ROLLOFF_GRID_INFO,
      NULL, 0, grid_width, sizeof(*grid_width));
    if (rc != 0) {
       CDBG_ERROR("%s: get vfe %d rolloff grid info failed, rc = %d\n",
                  __func__, isp_id, rc);
    }
  }
  return rc;
}

/** isp_ch_util_compute_min_left_input_width
 *
 * DESCRIPTION:
 *
 **/
static uint32_t isp_ch_util_compute_min_left_input_width(
  uint32_t input_width, uint32_t output_width)
{
  /* Left output width in dual VFE case needs to be padded for buffer alignment.
     Normally the padding is 16, with extra chroma subsampling requirement,
     it becomes 32 */
  return PAD_TO_SIZE(output_width / 4, 16) * 2 * input_width / output_width;
}

/** isp_ch_util_compute_stripe_info_of_channel
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_compute_stripe_info_of_channel(
  isp_t *isp, isp_session_t *session, uint32_t initial_overlap_half)
{
  int rc = 0;
  uint32_t i;
  isp_channel_t *ch;
  isp_channel_t *channel[2];
  uint32_t input_width = 0;
  uint32_t rolloff_grid_width;
  uint32_t num_rolloff_grids;
  uint32_t output_width[2];
  uint32_t num_hw_channel = 0;

  /* make initial_overlap_half even */
  initial_overlap_half = initial_overlap_half >> 1 << 1;

  /* ensure initial overlap (on one side) is at least 32
   * pixels due to rolloff requirement */
  initial_overlap_half =
    (initial_overlap_half > 32) ? initial_overlap_half : 32;

  /* determine dual ISP split parameters based on the stream output
  dimensions now that the hw streams are finalized */
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    ch = &session->channel[i];
    if (ch->user_stream_idx_mask) {
      if (ch->cfg.ispif_out_cfg.is_split) {
        /* assert that num_hw_streams is less than or equal to 2 */
        if (num_hw_channel >= 2) {
          CDBG_ERROR("%s: found more than 2 hw streams that are getting split "
            "output from ISPIF\n", __func__);
          return -1;
        }

        input_width = ch->cfg.sensor_cfg.request_crop.last_pixel -
          ch->cfg.sensor_cfg.request_crop.first_pixel + 1;
        output_width[num_hw_channel] = ch->stream_info.dim.width;
        channel[num_hw_channel] = ch;
        num_hw_channel++;
      }
    }
  }

  /* no stream has split ispif output */
  if (num_hw_channel == 0)
    return 0;

  /* swap the output widths from low to high if needed */
  if (num_hw_channel == 2 && output_width[1] < output_width[0]) {
    uint32_t temp = output_width[0];
    output_width[0] = output_width[1];
    output_width[1] = temp;
  }

  rc = isp_ch_util_get_rolloff_grid_info(isp, session, &num_rolloff_grids);
  if (rc != 0) {
    CDBG_ERROR("%s: failed to get rolloff grid info. rc = %d\n", __func__, rc);
    return rc;
  }
  rolloff_grid_width = input_width / num_rolloff_grids;
  /* need to assert initial_overlap_half is less than rolloff_grid_width */
  if (initial_overlap_half > rolloff_grid_width)
    return -1;

  if (num_hw_channel) {
    uint32_t min_left_input = isp_ch_util_compute_min_left_input_width(
      input_width, output_width[0]);
    uint32_t mid_point = input_width / 2;
    uint32_t split_point = MAX(min_left_input, mid_point) & ~1;
    uint32_t offset = split_point - initial_overlap_half;

    /* make sure our starting point for searching for offset fits the rolloff restriction:
       it cannot be within 32 pixels to the left of the rolloff grid boundary */
    if (offset > mid_point - 32)
      offset = mid_point - 32;

    if (num_hw_channel == 1) {
      while (offset > mid_point - rolloff_grid_width) {
        if (isp_ch_util_is_right_stripe_offset_usable(
          output_width[0], input_width, offset)) {
          channel[0]->cfg.ispif_out_cfg.right_stripe_offset =
            offset + channel[0]->cfg.sensor_cfg.request_crop.first_pixel;
          channel[0]->cfg.ispif_out_cfg.overlap =
            (split_point - offset) * 2;
          /* send all the hw stream info associate all stream
             to handle hw stream is different from the streamon stream*/
          for (i = 0; i < ISP_MAX_STREAMS; i++) {
            ch = &session->channel[i];
            if (!ch->cfg.ispif_out_cfg.is_split)
              continue;

            if (ch->user_stream_idx_mask) {
              isp_ch_util_send_channel_stripe_info_upstream(session, ch,
                &(ch->cfg.ispif_out_cfg));
            }
          }
          return 0;
        }
        offset -= 2;
      }
    } else {
      while (offset > mid_point - rolloff_grid_width) {
        if (isp_ch_util_is_right_stripe_offset_usable(
          output_width[0], input_width, offset) &&
          isp_ch_util_is_right_stripe_offset_usable(
          output_width[1], input_width, offset)) {

          channel[0]->cfg.ispif_out_cfg.right_stripe_offset =
            offset + channel[0]->cfg.sensor_cfg.request_crop.first_pixel;
          channel[0]->cfg.ispif_out_cfg.overlap =
            (split_point - offset) * 2;
          channel[1]->cfg.ispif_out_cfg = channel[0]->cfg.ispif_out_cfg;

          /* send all the hw stream info associate all stream
             to handle hw stream is different from the streamon stream*/
          for (i = 0; i < ISP_MAX_STREAMS; i++) {
            ch = &session->channel[i];
            if (!ch->cfg.ispif_out_cfg.is_split)
              continue;

            if (ch->user_stream_idx_mask) {
              isp_ch_util_send_channel_stripe_info_upstream(session, ch,
                &(ch->cfg.ispif_out_cfg));
            }
          }

          return 0;
        }

        offset -= 2;
      }
    }

    /* Could not find right offset */
    return -1;
  }

  /* single ISP case */
  return 0;
}

/** isp_util_request_image_buf_hal
 *
 * DESCRIPTION:
 *
 **/
static int isp_util_request_image_buf_hal(isp_t *isp, isp_session_t *session,
  isp_channel_t *channel, mct_list_t *img_buf_list)
{
  int rc = 0;
  isp_buf_request_t buf_request;

  assert(img_buf_list != NULL);
  memset(&buf_request, 0, sizeof(buf_request));
  buf_request.buf_handle = 0;
  buf_request.session_id = channel->session_id;
  buf_request.stream_id = channel->channel_id;

  /* if channel is split between dual VFEs use shared buf type
     single ISP case use private buf type */
  if (channel->cfg.ispif_out_cfg.is_split) {
    buf_request.buf_type = ISP_SHARE_BUF;
  } else {
    buf_request.buf_type = ISP_PRIVATE_BUF;
  }
  buf_request.img_buf_list = img_buf_list;
  /*Kernel buf struct is allocated once, even for deferred buf alloc case.
  Hence we need to update count of total buffers to kernel first time*/
  buf_request.total_num_buf = channel->total_num_bufs;

  pthread_mutex_lock(&isp->data.buf_mgr.req_mutex);
  rc = isp_request_buf(&isp->data.buf_mgr, &buf_request);
  if (rc < 0)
    CDBG_ERROR("%s: isp_request_buf error= %d\n", __func__, rc);
  else {
    channel->bufq_handle = buf_request.buf_handle;
    channel->current_num_bufs = buf_request.current_num_buf;
  }
  pthread_mutex_unlock(&isp->data.buf_mgr.req_mutex);

  return rc;
}

/** isp_util_request_image_buf_native
 *
 * DESCRIPTION:
 *
 **/
static int isp_util_request_image_buf_native(
  isp_t *isp, isp_session_t *session, isp_channel_t *channel)
{
  int rc = 0;
  isp_buf_request_t buf_request;
  cam_pp_feature_config_t *pp_config;
  uint8_t extra_buffers = 0;

  if (channel->streamon_cnt >= 1) {
   /* for shared hw streams only need to request buf once
    * when the first shared stream streamon */
   return 0;
  }
  pp_config = &channel->stream_info.pp_config;
  if (pp_config->feature_mask & CAM_QCOM_FEATURE_LLVD) {
    CDBG_HIGH("%s: LLVD enabled adding 2 extra buffers\n", __func__);
    extra_buffers = 2;
  }
  memset(&buf_request, 0, sizeof(buf_request));
  buf_request.buf_handle = 0;
  buf_request.session_id = channel->session_id;
  buf_request.stream_id = channel->channel_id | ISP_NATIVE_BUF_BIT;
  buf_request.use_native_buf = channel->use_native_buf;
  /*For deferred buffer allocation, HAL buffers are updated after streamon
  For non HAL buffers current and total number of buffers is same*/
  buf_request.total_num_buf = ISP_MAX_NATIVE_BUF_NUM + extra_buffers;
  buf_request.current_num_buf = ISP_MAX_NATIVE_BUF_NUM + extra_buffers;
  channel->total_num_bufs = ISP_MAX_NATIVE_BUF_NUM + extra_buffers;
  channel->current_num_bufs = ISP_MAX_NATIVE_BUF_NUM + extra_buffers;

  /* if channel is split between dual VFEs use shared buf type
     single ISP case use private buf type */
  if (channel->cfg.ispif_out_cfg.is_split)
    buf_request.buf_type = ISP_SHARE_BUF;
  else
    buf_request.buf_type = ISP_PRIVATE_BUF;

  buf_request.cached = 1;

  buf_request.buf_info = channel->stream_info.buf_planes.plane_info;

  pthread_mutex_lock(&isp->data.buf_mgr.req_mutex);
  rc = isp_request_buf(&isp->data.buf_mgr, &buf_request);
  if (rc < 0)
    CDBG_ERROR("%s: isp_request_buf error= %d\n", __func__, rc);
  else
    channel->bufq_handle = buf_request.buf_handle;
  pthread_mutex_unlock(&isp->data.buf_mgr.req_mutex);

  return rc;
}

/** isp_ch_util_request_channel_image_buf
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_request_channel_image_buf(
  isp_t *isp, isp_session_t *session, int num_channel,
  isp_channel_t *channels[ISP_MAX_STREAMS])
{
  int i, rc = 0;
  isp_channel_t *channel;
  mct_list_t *img_buf_list;

  for (i = 0; i < num_channel; i++) {
    channel = channels[i];
    img_buf_list = channel->stream_info.img_buffer_list;
    if (!channel->use_native_buf)
      channel->total_num_bufs = channel->stream_info.num_bufs;
    ISP_DBG(ISP_MOD_COM,"%s: session_id = %d, channel_id = %d, "
      "type = %d, use_native = %d, buf_list = %p, stream_mask = 0x%x,"
      "total num_bufs %d\n", __func__, channel->session_id, channel->channel_id,
      channel->stream_info.stream_type, channel->use_native_buf,
      img_buf_list, channel->user_stream_idx_mask,
      channel->total_num_bufs);

    /*request image buffer based on hal buf or native buf*/
    if (!channel->use_native_buf)
      rc = isp_util_request_image_buf_hal(isp, session, channel, img_buf_list);
    else
      rc = isp_util_request_image_buf_native(isp, session, channel);

    if (rc < 0) {
      CDBG_ERROR("%s: error in request image buffer, rc = %d, "
        "sessionid = %d, channel_id = %d\n",
        __func__, rc, session->session_id, channel->channel_id);
      return rc;
    }
  }

  return 0;
}

/** isp_ch_util_release_channel_image_buf
 *
 * DESCRIPTION:
 *
 **/
void isp_ch_util_release_channel_image_buf(
  isp_t *isp, isp_session_t *session,
  int num_channel, isp_channel_t *channels[ISP_MAX_STREAMS])
{
  int i;
  uint32_t bufq_handle;
  isp_channel_t *channel;

  for (i = 0; i < num_channel; i++) {
    channel = channels[i];
    if (channel->streamon_cnt == 0) {
      channel->state = ISP_CHANNEL_STATE_HW_CFG;
      if (channel->bufq_handle > 0) {
        pthread_mutex_lock(&isp->data.buf_mgr.req_mutex);
        isp_release_buf(&isp->data.buf_mgr, channel->bufq_handle);
        pthread_mutex_unlock(&isp->data.buf_mgr.req_mutex);
        channel->bufq_handle= 0;
      }
    }
  }
}

/** isp_ch_util_fill_hw_stream_config_struct
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_fill_hw_stream_config_struct(
  isp_t *isp, int isp_id, isp_session_t *session,
  isp_hw_stream_cfg_t *hw_stream_cfg, int *num_hw_streams)
{
  int i, rc = 0;
  int count = 0;
  isp_channel_t *channel;
  isp_port_t *isp_sink_port = NULL;
  isp_sink_port_t *sink_port = NULL;
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    channel = &session->channel[i];
    if (channel->state != ISP_CHANNEL_STATE_USER_CFG) {
      /* already configured hw */
      continue;
    }
    if(((channel->cfg.vfe_output_mask >> (isp_id * 16)) & 0xffff) == 0) {
      /* this channel does not use specified vfe */
      continue;
    }
    isp_ch_util_dump_channel_planes(channel);

    isp_sink_port = (isp_port_t *)channel->sink_port->port_private;
    sink_port = &isp_sink_port->u.sink_port;

    /* fill in all the hw_stream_cfg_entry inside hw_stream_cfg*/
    hw_stream_cfg->entries[count].input_type = sink_port->input_type;
    hw_stream_cfg->entries[count].sink_cap = sink_port->caps.sensor_cap;
    hw_stream_cfg->entries[count].sensor_out_info = channel->cfg.sensor_cfg;
    hw_stream_cfg->entries[count].stream_id = channel->channel_id;
    hw_stream_cfg->entries[count].stream_info = channel->stream_info;
    hw_stream_cfg->entries[count].is_encoder = channel->is_encoder;
    hw_stream_cfg->entries[count].vfe_output_mask =
      channel->cfg.vfe_output_mask >> (isp_id * 16);
    hw_stream_cfg->entries[count].need_uv_subsample =
      session->uv_subsample_ctrl.curr_enable;
    hw_stream_cfg->entries[count].use_native_buf = channel->use_native_buf;
    hw_stream_cfg->entries[count].ispif_out_info = channel->cfg.ispif_out_cfg;

    /* if meta channel, always use rdi
       note: now, the current implementation only support one
       meta data so we simply hard code to one. The code needs to
       be cleaned asap. */
    if (channel->channel_type == ISP_CHANNEL_TYPE_IMAGE) {
      hw_stream_cfg->entries[count].use_pix = sink_port->caps.use_pix;
      hw_stream_cfg->entries[count].meta_ch_idx_mask = 0;
    } else {
      hw_stream_cfg->entries[count].use_pix = 0;
      hw_stream_cfg->entries[count].meta_ch_idx_mask =  (1 << 0);
    }

    /* Dual vfe case*/
    if (hw_stream_cfg->entries[count].ispif_out_info.is_split) {
      hw_stream_cfg->entries[count].isp_out_info.stripe_id =
        (isp_id == VFE0) ? ISP_STRIPE_LEFT : ISP_STRIPE_RIGHT;
      /* make sure chroma buffer offset after subsampling is still aligned */
      hw_stream_cfg->entries[count].isp_out_info.left_output_width =
        PAD_TO_SIZE(channel->stream_info.dim.width / 4, 16) * 2;

      ISP_DBG(ISP_MOD_COM,"%s: vfe %d stream_info width = %d, left_output_width = %d\n",
        __func__, isp_id, channel->stream_info.dim.width,
        hw_stream_cfg->entries[count].isp_out_info.left_output_width);

      hw_stream_cfg->entries[count].isp_out_info.right_output_width =
        channel->stream_info.dim.width -
        hw_stream_cfg->entries[count].isp_out_info.left_output_width;

      /* Notice right stripe offset from ISPIF perspective
       * and right stripe offset from ISP_HW perspective is different.
       * Stripe output from ISPIF contains pixels to
       * be cropped by CAMIF (e.g. sensor_cfg.request_crop.first_pixel)
       */
      hw_stream_cfg->entries[count].isp_out_info.right_stripe_offset =
        channel->cfg.ispif_out_cfg.right_stripe_offset -
        channel->cfg.sensor_cfg.request_crop.first_pixel;
    }

    if (channel->src_ports[ISP_SRC_PORT_DATA] ||
        hw_stream_cfg->entries[count].use_native_buf)
      hw_stream_cfg->entries[count].need_divert = TRUE;
    else
      hw_stream_cfg->entries[count].need_divert = FALSE;
    count++;
  }

  *num_hw_streams = count;
  return 0;
}

/** isp_ch_util_get_hfr_skip_pattern
 *
 * DESCRIPTION:
 *
 **/
enum msm_vfe_frame_skip_pattern isp_ch_util_get_hfr_skip_pattern(
  isp_session_t *session)
{
  if(session->isp_fast_aec_mode)
    return SKIP_ALL;

  switch (session->hfr_param.hfr_mode) {
  case CAM_HFR_MODE_60FPS:
    return EVERY_2FRAME;

  case CAM_HFR_MODE_90FPS:
    return EVERY_3FRAME;

  case CAM_HFR_MODE_120FPS:
    return EVERY_4FRAME;

  case CAM_HFR_MODE_150FPS:
    return EVERY_5FRAME;

  default:
    break;
  }

  return NO_SKIP;
}

/** isp_ch_util_fill_hfr_param
 *
 * DESCRIPTION:
 *
 **/
static void isp_ch_util_fill_hfr_param(
  isp_t *isp, isp_session_t *session,
  isp_hw_stream_cfg_t *hw_stream_cfg)
{
  isp_channel_t *channel;

  channel = isp_ch_util_find_channel_in_session(session,
              session->hfr_param.stream_id);

  if (!channel) {
    CDBG_ERROR("%s: no match channel\n", __func__);
    return;
  }

  if ((channel->src_ports[ISP_SRC_PORT_DATA]) &&
      !isp_util_is_lowpowermode_feature_enable(isp,session->session_id)
      )
    /* if linked with PP per discussion PP will do the frame skip */
    session->hfr_param.pp_do_frame_skip = TRUE;
  else
    session->hfr_param.pp_do_frame_skip = FALSE;

  hw_stream_cfg->hfr_param = session->hfr_param;
}

/** isp_ch_util_assign_hfr_skip_pattern
 *
 * DESCRIPTION:
 *
 **/
static void isp_ch_util_assign_hfr_skip_pattern(isp_t *isp,
  isp_session_t *session, isp_hw_stream_cfg_t *hw_stream_cfg)
{
  isp_channel_t *channel;
  int i;

  /* fill in HFR param first */
  isp_ch_util_fill_hfr_param(isp, session, hw_stream_cfg);

  /* now fill in skip pattern for each stream */
  for (i = 0; i < hw_stream_cfg->num_streams; i++) {
    channel = isp_ch_util_find_channel_in_session(session,
      hw_stream_cfg->entries[i].stream_id);
    if (!channel)
      continue;
    if (session->hfr_param.pp_do_frame_skip)
      hw_stream_cfg->entries[i].skip_pattern = NO_SKIP;
    else if (channel->stream_info.stream_type == CAM_STREAM_TYPE_PREVIEW){
      hw_stream_cfg->entries[i].skip_pattern =
        isp_ch_util_get_hfr_skip_pattern(session);
    } else
      hw_stream_cfg->entries[i].skip_pattern = NO_SKIP;
    if (session->isp_fast_aec_mode) {
      hw_stream_cfg->entries[i].skip_pattern = SKIP_ALL;
    }
  }
}

/** isp_ch_util_send_bundling_skip_pattern_upstream
 *
 * DESCRIPTION:
 *
 **/
static void isp_ch_util_send_bundling_skip_pattern_upstream(
   isp_t *isp, isp_session_t *session,
   isp_channel_t *channel, enum msm_vfe_frame_skip_pattern skip_pattern)
{
  mct_event_t event;
  isp_stream_skip_pattern_t skip_pattern_event;
  mct_port_t *isp_sink_port = channel->sink_port;

  if (isp_sink_port) {
    skip_pattern_event.skip_pattern = skip_pattern;
    memset(&event,  0,  sizeof(event));
    event.type = MCT_EVENT_MODULE_EVENT;
    event.u.module_event.type = MCT_EVENT_MODULE_ISP_SKIP_PATTERN;
    event.u.module_event.module_event_data = (void *)&skip_pattern_event;
    event.identity = pack_identity(channel->session_id, channel->channel_id);
    event.direction = MCT_EVENT_UPSTREAM;
    mct_port_send_event_to_peer(isp_sink_port, &event);
  }
}

/** isp_ch_util_assign_bundling_skip_pattern
 *
 * DESCRIPTION:
 *
 **/
static void isp_ch_util_assign_bundling_skip_pattern(isp_t *isp,
  isp_session_t *session, isp_hw_stream_cfg_t *hw_stream_cfg)
{
  isp_channel_t *channel;
  int i;
  float current_fps = 0.0, skip_ratio = 0.0, limit_fps = 0.0;
  float max_bw = 0;
  int32_t frame_skip;
  uint32_t sensor_res = 0, snapshot_res = 0;

  session->saved_params.use_bundled_frame_skip = FALSE;

  for (i = 0; i < hw_stream_cfg->num_streams; i++) {
    channel = isp_ch_util_find_channel_in_session(session,
      hw_stream_cfg->entries[i].stream_id);
    if (!channel)
      continue;

    /* for 30fps 2 ISP straeming use case ISP does not skip frame.
     * For other ZSL use case ISP skip frame for snapshot image per
     * discussion with HAL,stats, sensor and MCT teams.
     * Do not skip frame for preview. */
    if (channel->cfg.ispif_out_cfg.is_split ||
        channel->stream_info.stream_type != CAM_STREAM_TYPE_SNAPSHOT)
      hw_stream_cfg->entries[i].skip_pattern = NO_SKIP;
    else {
      /* for non HFR, need to make sure that we only skip main image for zsl */
      if (channel->stream_info.streaming_mode == CAM_STREAMING_MODE_CONTINUOUS) {
        if (session->saved_params.longshot_enable)
          max_bw = isp->data.sd_info.sd_info[0].cap.isp_info.longshot_max_bandwidth;
        else /*ZSL only use case*/
          max_bw = isp->data.sd_info.sd_info[0].cap.isp_info.zsl_max_bandwidth;
        if (max_bw > 0) {
          /*Limit FPS based on maximum bandwidth supported. Same VFE version
          could be used on different chipsets with different bandwidth
          allocations*/
          snapshot_res = channel->stream_info.dim.width *
            channel->stream_info.dim.height;
          /*max_bw related FPS is in Q8 format*/
          limit_fps = max_bw/(snapshot_res * Q8);
          ISP_DBG(ISP_MOD_COM,"%s: max_bw=%f, stream wxh=%dx%d = %d, limit_fps=%f",__func__,
             max_bw,channel->stream_info.dim.width,
             channel->stream_info.dim.height,snapshot_res, limit_fps);
          current_fps = MIN(1/session->saved_params.aec_stats_update.exp_time,
                            session->streams[i].cfg.sensor_cfg.max_fps);
          skip_ratio = current_fps/limit_fps;
          frame_skip = (int32_t)(CUSTOM_ROUND(skip_ratio) - 1);
          if (frame_skip < 0)
            frame_skip = NO_SKIP;
          CDBG_HIGH("%s: current_fps = %f, skip_ratio = %f, frame_skip = %d",__func__,
                  current_fps, skip_ratio, frame_skip);
          hw_stream_cfg->entries[i].skip_pattern = frame_skip;
        } else {
          hw_stream_cfg->entries[i].skip_pattern = EVERY_2FRAME;
        }

        session->saved_params.use_bundled_frame_skip = TRUE;
        session->saved_params.bundled_frame_skip.pattern =
          hw_stream_cfg->entries[i].skip_pattern;
        session->saved_params.bundled_frame_skip.session_id =
          session->session_id;
        session->saved_params.bundled_frame_skip.stream_id =
          hw_stream_cfg->entries[i].stream_id;

        isp_ch_util_send_bundling_skip_pattern_upstream(isp,
          session, channel, hw_stream_cfg->entries[i].skip_pattern);
      }
    }
  }
}

/** isp_ch_util_assign_stream_skip_pattern
 *
 * DESCRIPTION:
 *
 **/
static void isp_ch_util_assign_stream_skip_pattern(isp_t *isp,
  isp_session_t *session, isp_hw_stream_cfg_t *hw_stream_cfg)
{
  /* if not use ISP module no skip pattern */
  if (session->use_pipeline != TRUE)
    return;

  /* take care of HFR and bundling case only.
   * In bundling case only take care of pix interface */
  if (session->hfr_param.hfr_mode != CAM_HFR_MODE_OFF)
    isp_ch_util_assign_hfr_skip_pattern(isp, session, hw_stream_cfg);
  else if (session->hal_bundling_mask != 0) {
    isp_ch_util_assign_bundling_skip_pattern(isp, session, hw_stream_cfg);
  }
}

/** isp_ch_util_config_hw_streams
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_config_hw_streams(
  isp_t *isp, isp_session_t *session, int isp_id)
{
  int i, rc = 0;
  int num_channel = 0;
  isp_channel_t *chanels[ISP_MAX_STREAMS];
  isp_hw_stream_cfg_t hw_stream_cfg;

  memset(&hw_stream_cfg, 0, sizeof(hw_stream_cfg));
  memset(chanels, 0, sizeof(chanels));

  /*fill in all hw_stream_cfg_entry of this hw_stream_cfg */
  rc = isp_ch_util_fill_hw_stream_config_struct(isp, isp_id, session,
    &hw_stream_cfg, &num_channel);
  if (rc < 0) {
    CDBG_ERROR("%s: error, cannot generate hw_stream_cfg\n", __func__);
    return rc;
  }

  hw_stream_cfg.session_id = session->session_id;
  hw_stream_cfg.num_streams = num_channel;
  hw_stream_cfg.ion_fd = session->ion_fd;
  hw_stream_cfg.hfr_param = session->hfr_param;
  hw_stream_cfg.vt_enable = session->saved_params.vt_enable;

  isp_ch_util_assign_stream_skip_pattern(isp, session, &hw_stream_cfg);

  hw_stream_cfg.dev_id = isp_id;
  if (isp->data.hw[hw_stream_cfg.dev_id].hw_ops == NULL) {
    CDBG_ERROR("%s: ISP %d not inited\n", __func__, hw_stream_cfg.dev_id);
    return -1;
  }

  /*pass the hw stream config to isp_hw*/
  rc = isp->data.hw[hw_stream_cfg.dev_id].hw_ops->set_params(
     isp->data.hw[hw_stream_cfg.dev_id].hw_ops->ctrl,
     ISP_HW_SET_PARAM_STREAM_CFG, (void *)&hw_stream_cfg,
     sizeof(hw_stream_cfg));
  if (rc < 0) {
    CDBG_ERROR("%s: error sending hw_stream_cfg to hw %d, sessid = %d\n",
      __func__, hw_stream_cfg.dev_id, session->session_id);
    return -1;
  }

  return rc;
}

/** isp_ch_util_send_crop_factor_param_to_hw
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_send_crop_factor_param_to_hw(
   isp_t *isp, isp_session_t *session, int isp_id)
{
  int rc = 0;
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

  /* only adjust crop factor for dual VFE case */
  if (session->vfe_ids == ((1 << VFE0) | (1 << VFE1))) {
    rc = isp_ch_util_adjust_crop_factor(session,
      crop_factor.crop_factor, &crop_factor.crop_factor);
    if (rc < 0) {
      CDBG_ERROR("%s: error adjusting crop factor error = %d\n", __func__, rc);
      return rc;
    }
  }

  if (isp->data.hw[isp_id].hw_ops) {
    rc = isp->data.hw[isp_id].hw_ops->set_params(
      isp->data.hw[isp_id].hw_ops->ctrl, ISP_HW_SET_PARAM_CROP_FACTOR,
      (void *)&crop_factor, sizeof(crop_factor));
    if (rc) {
      CDBG_ERROR("%s: isp_id = %d zoom error = %d\n", __func__, isp_id, rc);
      return rc;
    }
  }

  return rc;
}

/** isp_ch_util_proc_initial_buffered_hw_params
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_proc_initial_buffered_hw_params(
   isp_t *isp, isp_session_t *session, int isp_id)
{
  int rc = 0;
  isp_hw_params_t new_params;

  pthread_mutex_lock(&isp->data.session_critical_section[session->session_idx]);
  new_params = session->buffered_hw_params.new_params;
  memset(&session->buffered_hw_params.new_params, 0,
    sizeof(session->buffered_hw_params.new_params));

  if (new_params.zoom.present) {
    session->zoom_val = new_params.zoom.zoom_val;
    ISP_DBG(ISP_MOD_COM,"%s: session_id = %d, initial_zoom_val = %d\n",
      __func__, session->session_id, session->zoom_val );
  }
  pthread_mutex_unlock(
    &isp->data.session_critical_section[session->session_idx]);

  rc = isp_ch_util_send_crop_factor_param_to_hw(isp, session, isp_id);
  if (rc < 0) {
    CDBG_ERROR("%s: error, isp_ch_util_send_crop_factor_param_to_hw, "
      "sessid = %d, rc = %d\n",
      __func__, session->session_id, rc);
    return rc;
  }

  return rc;
}

/** isp_ch_util_prepare_hw_config_for_streamon_int
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_prepare_hw_config_for_streamon_int(
  isp_t *isp, int isp_id, isp_session_t *session)
{
  int i, rc = 0;
  start_stop_stream_t param;
  isp_channel_t *channel;
  mct_list_t *buf_list;
  mct_stream_info_t *stream_info;
  cam_flash_mode_t flash_mode = CAM_FLASH_MODE_OFF;

  if (session->flash_streamon ||
      session->saved_params.flash_mode == CAM_FLASH_MODE_ON) {
    flash_mode = CAM_FLASH_MODE_ON;
  }

  /* session needs to be configured first so that
   * when we set params HW can validate the session id */
  rc = isp_ch_util_config_hw_streams(isp, session, isp_id);
  if (rc < 0) {
    CDBG_ERROR("%s: isp_config_hw_streams for vfe0 error = %d \n", __func__, rc);
    return rc;
  }

  if (session->use_pipeline) {
    rc = isp->data.hw[isp_id].hw_ops->set_params(
      isp->data.hw[isp_id].hw_ops->ctrl, ISP_HW_SET_RECORDING_HINT,
      (void *)&session->recording_hint, sizeof(session->recording_hint));
    if (rc < 0) {
      CDBG_ERROR("%s: error, ISP_HW_SET_RECORDING_HINT, "
        "sessid = %d, vfe_id = %d, rc = %d\n",
        __func__, session->session_id, isp_id, rc);
      return rc;
    }
    /* at this time we can send the initial buffered
     * params to HW and enable the in_service flag */

    rc = isp->data.hw[isp_id].hw_ops->set_params(
      isp->data.hw[isp_id].hw_ops->ctrl, ISP_HW_SET_PARAM_CHROMATIX,
      (void *)&session->chromatix, sizeof(session->chromatix));
    if (rc < 0) {
      CDBG_ERROR("%s: error, ISP_HW_SET_PARAM_CHROMATIX, "
        "sessid = %d, vfe_id = %d, rc = %d\n",
        __func__, session->session_id, isp_id, rc);
      return rc;
    }

    rc = isp->data.hw[isp_id].hw_ops->set_params(
      isp->data.hw[isp_id].hw_ops->ctrl, ISP_HW_SET_PARAM_STATS_CFG,
      (void *)&session->stats_config, sizeof(session->stats_config));
    if (rc < 0) {
      CDBG_ERROR("%s: error, ISP_HW_SET_PARAM_STATS_CFG, "
        "sessid = %d, vfe_id = %d, rc = %d\n",
        __func__, session->session_id, isp_id, rc);
      return rc;
    }

    rc = isp->data.hw[isp_id].hw_ops->set_params(
           isp->data.hw[isp_id].hw_ops->ctrl, ISP_HW_SET_FLASH_MODE,
           (void *)&flash_mode, sizeof(flash_mode));
    if (rc < 0) {
      CDBG_ERROR("%s: error, ISP_HW_SET_FLASH_MODE, "
        "sessid = %d, vfe_id = %d, rc = %d\n",
        __func__, session->session_id, isp_id, rc);
      return rc;
    }
   /* To make sure both VFEs use same params while streaming on, we use cached
    *  params instead of saved params. saved params can change with trigger
    *  updates from other modules. But cached copy is not updated till both
    *  VFEs are done with hw update. This is protection against split
    *  screen behaviour */
    rc = isp->data.hw[isp_id].hw_ops->set_params(
         isp->data.hw[isp_id].hw_ops->ctrl, ISP_HW_SET_PARAM_SET_SAVED_PARAMS,
         (void *)&(session->pending_update_params.hw_update_params),
         sizeof(session->pending_update_params.hw_update_params));
    if (rc < 0) {
      CDBG_ERROR("%s: error, ISP_HW_SET_PARAM_SET_SAVED_PARAMS, "
        "sessid = %d, vfe_id = %d, rc = %d\n",
        __func__, session->session_id, isp_id, rc);
      return rc;
    }

    rc = isp_ch_util_proc_initial_buffered_hw_params(isp, session, isp_id);
    if (rc < 0) {
      CDBG_ERROR("%s: error, isp_ch_util_send_crop_factor_param_to_hw, "
        "sessid = %d, vfe_id = %d, rc = %d\n",
        __func__, session->session_id, isp_id, rc);
      return rc;
    }

    /* we already configured the ISP HW.
     * Generate the DIS configuration for 3A */
    if (session->dis_param.dis_enable) {
      rc = isp_util_send_dis_config_to_stats(isp, session);
      if (rc) {
        CDBG_ERROR("%s: error, isp_ch_util_send_dis_config_to_stats, "
          "sessid = %d, vfe_id = %d, rc = %d\n",
          __func__, session->session_id, isp_id, rc);
        return rc;
      }
    }
  }

  return rc;
}

/** isp_ch_util_prepare_hw_config_for_streamon
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_prepare_hw_config_for_streamon(
  isp_t *isp, isp_session_t *session)
{
  int rc = -1;

  if (session->vfe_ids & (1 << VFE0)) {
    rc = isp_ch_util_prepare_hw_config_for_streamon_int(isp, VFE0, session);
    if (rc < 0) {
      CDBG_ERROR("%s: VFE%d: isp_ch_util_prepare_hw_config_for_streamon_int error!"
        "sessid = %d, rc = %d\n", __func__, VFE0, session->session_id, rc);
      return rc;
    }
  }

  if (session->vfe_ids & (1 << VFE1)) {
    rc = isp_ch_util_prepare_hw_config_for_streamon_int(isp, VFE1, session);
    if (rc < 0) {
     CDBG_ERROR("%s: VFE%d: isp_ch_util_prepare_hw_config_for_streamon_int error!"
        "sessid = %d, rc = %d\n", __func__, VFE1, session->session_id, rc);
      return rc;
    }
  }

  return rc;
}

/** isp_ch_util_hw_streamon_int
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_hw_streamon_int(
  isp_t *isp, int isp_id, isp_session_t *session,
  int num_hw_streams, uint32_t *hw_stream_ids, boolean wait_for_sof)
{
  int rc = 0;
  start_stop_stream_t param;

  memset(&param, 0, sizeof(param));

  if (num_hw_streams > 0) {
    param.num_streams = num_hw_streams;
    param.session_id = session->session_id;
    param.stream_ids = hw_stream_ids;
    param.wait_for_sof = wait_for_sof;
    param.fast_aec_mode = session->isp_fast_aec_mode;

    rc = isp->data.hw[isp_id].hw_ops->action(
      isp->data.hw[isp_id].hw_ops->ctrl,
      ISP_HW_ACTION_CODE_STREAM_START,
      (void *)&param, sizeof(param));
    if (rc < 0) {
      CDBG_ERROR("%s: error, ISP_HW_ACTION_CODE_STREAM_START, "
        "sessid = %d, vfe_id = %d, rc = %d\n",
        __func__, session->session_id, isp_id, rc);
    }
  }

  return rc;
}

/** isp_ch_util_streamon_ack
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_streamon_ack(
  isp_t *isp, int isp_id, isp_session_t *session,
  int num_hw_streams, uint32_t *hw_stream_ids)
{
  int rc = 0;
  start_stop_stream_t param;

  memset(&param, 0, sizeof(param));

  if (num_hw_streams > 0) {
    param.num_streams = num_hw_streams;
    param.session_id = session->session_id;
    param.stream_ids = hw_stream_ids;
    param.wait_for_sof = FALSE;

    rc = isp->data.hw[isp_id].hw_ops->action(
      isp->data.hw[isp_id].hw_ops->ctrl,
      ISP_HW_ACTION_CODE_STREAM_START_ACK,
      (void *)&param, sizeof(param));
    if (rc < 0) {
      CDBG_ERROR("%s: error, ISP_HW_ACTION_CODE_STREAM_START_ACK, "
        "sessid = %d, vfe_id = %d, rc = %d\n",
        __func__, session->session_id, isp_id, rc);
    }
  }

  return rc;
}

/** get_hw_streams_using_vfe:
 *    @session: isp session
 *    @isp_id: VFE ID for which we are searching
 *    @hw_stream_ids: list of available stream IDs
 *    @num_hw_streams: number of available streams
 *    @used_hw_stream_ids: [OUT]list of stream IDs using given VFE
 *    @used_num_hw_streams: [OUT]number of streams using given VFE
 *
 *  This function runs in MCTL thread context.
 *
 *  This function returns which HW streams from a list use given VFE.
 *
 *  Return:  0 - Success
 *          -1 - Invalid channel for some hw stream ID.
 **/
static int get_hw_streams_using_vfe(isp_session_t *session, uint32_t isp_id,
    uint32_t hw_stream_ids[], int num_hw_streams,
    uint32_t used_hw_stream_ids[], int *used_num_hw_streams)
{
    int i;
    isp_channel_t *channel;
    *used_num_hw_streams = 0;
    for (i = 0; i < num_hw_streams; i++) {
      channel = isp_ch_util_find_channel_in_session(session, hw_stream_ids[i]);
      if(!channel) {
        CDBG_ERROR("%s: error: channel invalid for HW stream ID %d!",__func__,
          hw_stream_ids[i]);
        return -1;
      } else {
        if(((channel->cfg.vfe_output_mask >> (isp_id * 16)) & 0xffff) != 0) {
            used_hw_stream_ids[*used_num_hw_streams] = hw_stream_ids[i];
            (*used_num_hw_streams)++;
        }
      }
    }
    return 0;
}

/** isp_ch_util_streamon
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_streamon(
  isp_t *isp, isp_session_t *session,
  int num_hw_channels, uint32_t *hw_channel_ids)
{
  int i, isp_id, used_num_hw_streams, num_hw_streams = 0, rc = 0;
  uint32_t hw_stream_ids[ISP_MAX_STREAMS];
  uint32_t used_hw_stream_ids[ISP_MAX_STREAMS];
  isp_channel_t *channel;
  boolean dual_vfe = FALSE;

  for (i = 0; i < num_hw_channels; i++) {
    channel = isp_ch_util_find_channel_in_session(session, hw_channel_ids[i]);
    if (!channel) {
      CDBG_ERROR("%s: cannot find channel, session_id = %d, channel_id = %d\n",
        __func__, session->session_id, hw_channel_ids[i]);
      continue;
    }

    /* only config channel when its set HW_CFG*/
    if (channel->state != ISP_CHANNEL_STATE_ACTIVE &&
      channel->state != ISP_CHANNEL_STATE_HW_CFG) {
      CDBG_ERROR("%s: invalid channel state, session_id = %d, channel_id = %d, state = %d",
        __func__, session->session_id, hw_channel_ids[i], channel->state);
      continue;
    }
    channel->streamon_cnt++;

    /* already streaming. just increase the shared count */
    if (channel->state == ISP_CHANNEL_STATE_ACTIVE) {
      CDBG_ERROR("%s: session_id = %d, channel_id = %d, already active.\n",
        __func__, channel->session_id, channel->channel_id);
      continue;
    }

    if (num_hw_streams < ISP_MAX_STREAMS){
      hw_stream_ids[num_hw_streams++] = hw_channel_ids[i];

      /* only wait for one sof if there are at least one  streams active. */
      if (!dual_vfe && channel->cfg.ispif_out_cfg.is_split &&
          session->active_count > 0) {
        dual_vfe = TRUE;
      }
    } else {
      CDBG_ERROR("%s: num_hw_streams %d out of bound\n", __func__, num_hw_streams);
    }
  }

  if (dual_vfe) {
    uint32_t num_frames_to_skip = 1;
    rc = isp_ch_util_set_param(isp, session,
      0, ISP_HW_SET_PARAM_HW_UPDATE_SKIP,
      (void *)&num_frames_to_skip, sizeof(uint32_t));
    if (rc < 0) {
      CDBG_ERROR("%s: hw streamon error! sessid = %d, rc = %d\n",
         __func__, session->session_id, rc);
      for (i = 0; i < num_hw_streams; i++) {
        channel = isp_ch_util_find_channel_in_session(session, hw_stream_ids[i]);
        if(!channel)
          CDBG_ERROR("%s: channel invalid!",__func__);
        else
          channel->streamon_cnt--;
      }
      return rc;
    }
  }
  for(isp_id = VFE0; isp_id < VFE_MAX; isp_id++) {
    if (session->vfe_ids & (1 << isp_id)) {
      boolean wait_for_sof;

      rc = get_hw_streams_using_vfe(session, isp_id, hw_stream_ids,
        num_hw_streams, used_hw_stream_ids, &used_num_hw_streams);

      /* In dual VFE use case only ask the first VFE to wait for SOF */
      wait_for_sof = (isp_id == VFE0) ? dual_vfe : FALSE;

      if (rc >= 0)
        rc = isp_ch_util_hw_streamon_int(isp, isp_id, session,
          used_num_hw_streams, used_hw_stream_ids, wait_for_sof);

      if (rc < 0) {
        CDBG_ERROR("%s: VFE%d: hw streamon error! sessid = %d, rc = %d\n",
           __func__, isp_id, session->session_id, rc);
        for (i = 0; i < used_num_hw_streams; i++) {
          channel = isp_ch_util_find_channel_in_session(session,
            used_hw_stream_ids[i]);

          if(!channel)
            CDBG_ERROR("%s: channel invalid!",__func__);
          else
            channel->streamon_cnt--;
        }

        return rc;
      }
    }
  }

  CDBG_HIGH("%s: session_id = %d, vfe_mask = 0x%x, async streamon, rc = %d\n",
    __func__, session->session_id, session->vfe_ids, rc);

  /*Streamon ACK for vfe0 & vfe1*/
  if (rc == 0) {

    for(isp_id = VFE0; isp_id < VFE_MAX; isp_id++) {
      if (session->vfe_ids & (1 << isp_id)) {

        rc = get_hw_streams_using_vfe(session, isp_id, hw_stream_ids,
          num_hw_streams, used_hw_stream_ids, &used_num_hw_streams);

        if (rc >= 0)
          rc = isp_ch_util_streamon_ack(isp, isp_id, session,
            used_num_hw_streams, used_hw_stream_ids);

        if (rc < 0) {
          CDBG_ERROR("%s: VFE%d: hw streamon error! sessid = %d, rc = %d\n",
            __func__, isp_id, session->session_id, rc);

          for (i = 0; i < used_num_hw_streams; i++) {
            channel = isp_ch_util_find_channel_in_session(session,
              used_hw_stream_ids[i]);
            if(!channel)
              CDBG_ERROR("%s: channel invalid!",__func__);
            else
              channel->streamon_cnt--;
          }
          return rc;
        }
      }
    }

    CDBG_HIGH("%s: session_id = %d, sync ack done\n",
      __func__, session->session_id);

    /*update channel state afte both ISP, set to ISP_CHANNEL_STATE_ACTIVE*/
    for (i = 0; i < num_hw_channels; i++) {
      channel = isp_ch_util_find_channel_in_session(session, hw_channel_ids[i]);
      if (!channel)
        CDBG_ERROR("%s: hw streamon error! sessid = %d i = %d\n",
           __func__, session->session_id, i);
      else
        channel->state = ISP_CHANNEL_STATE_ACTIVE;
    }
  }

  return rc;
}

/** isp_ch_util_streamoff_int
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_streamoff_int(
  isp_t *isp, int isp_id, isp_session_t *session,
  int num_hw_streams, uint32_t *hw_stream_ids, boolean wait_for_sof,
  boolean stop_immediately)
{
  int rc = 0;
  start_stop_stream_t param;

  memset(&param, 0, sizeof(param));

  if (num_hw_streams > 0) {
    param.num_streams = num_hw_streams;
    param.session_id = session->session_id;
    param.stream_ids = hw_stream_ids;
    param.wait_for_sof = wait_for_sof;
    param.fast_aec_mode = session->isp_fast_aec_mode;
    param.stop_immediately = stop_immediately;

    rc = isp->data.hw[isp_id].hw_ops->action(isp->data.hw[isp_id].hw_ops->ctrl,
      ISP_HW_ACTION_CODE_STREAM_STOP, (void *)&param, sizeof(param));
    if (rc < 0) {
      CDBG_ERROR("%s: error, ISP_HW_ACTION_CODE_STREAM_STOP, "
        "sessid = %d, vfe_id = %d, rc = %d\n",
        __func__, session->session_id, isp_id, rc);
      return rc;
    }
  }

  return rc;
}

/** isp_ch_util_streamoff_ack
 *
 * DESCRIPTION:
 *
 **/
static int isp_ch_util_streamoff_ack(isp_t *isp, int isp_id,
  isp_session_t *session, int num_hw_streams, uint32_t *hw_stream_ids)
{
  int rc = 0;
  start_stop_stream_t param;

  memset(&param, 0, sizeof(param));

  if (num_hw_streams > 0) {
    param.num_streams = num_hw_streams;
    param.session_id = session->session_id;
    param.stream_ids = hw_stream_ids;
    param.wait_for_sof = FALSE;

    rc = isp->data.hw[isp_id].hw_ops->action(isp->data.hw[isp_id].hw_ops->ctrl,
      ISP_HW_ACTION_CODE_STREAM_STOP_ACK, (void *)&param, sizeof(param));
  }
  if (rc < 0) {
    CDBG_ERROR("%s: error, ISP_HW_ACTION_CODE_STREAM_STOP_ACK, "
      "sessid = %d, vfe_id = %d, rc = %d\n",
      __func__, session->session_id, isp_id, rc);
  }

  return rc;
}

/** isp_ch_util_streamoff
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_streamoff(
  isp_t *isp, isp_session_t *session,
  int num_channels, uint32_t *channel_ids,
  boolean stop_immediately)
{
  int i, isp_id, rc = 0;
  int  used_num_hw_streams, num_hw_streams = 0;
  uint32_t hw_stream_ids[ISP_MAX_STREAMS];
  uint32_t used_hw_stream_ids[ISP_MAX_STREAMS];
  isp_channel_t *channel;

  for (i = 0; i < num_channels; i++) {
    channel = isp_ch_util_find_channel_in_session(session, channel_ids[i]);
    if (!channel) {
      CDBG_ERROR("%s: cannot find channel, session_id = %d, channel_id = %d\n",
        __func__, session->session_id, channel_ids[i]);
      continue;
    }

    if (channel->state != ISP_CHANNEL_STATE_ACTIVE) {
      CDBG_ERROR("%s: channel in invalid state, session_id = %d, channel_id = %d, state = %d",
        __func__, session->session_id, channel_ids[i], channel->state);
      continue;
    }

    channel->streamon_cnt--;
    if (channel->streamon_cnt > 0) {
      /* still have shared straem. nop for this straemoff */
      continue;
    }

    if (num_hw_streams < ISP_MAX_STREAMS) {
      hw_stream_ids[num_hw_streams++] = channel_ids[i];
      channel->state = ISP_CHANNEL_STATE_STOPPING; /* change state to stopping */
    } else {
      CDBG_ERROR("%s: num_hw_streams %d out of bound\n", __func__, num_hw_streams);
    }
  }

  uint32_t num_frames_to_skip = 1;
  rc = isp_ch_util_set_param(isp, session,
    0, ISP_HW_SET_PARAM_HW_UPDATE_SKIP,
    (void *)&num_frames_to_skip, sizeof(uint32_t));
  if (rc < 0) {
    CDBG_ERROR("%s: error, isp_util_streamoff, "
      "sessid = %d, rc = %d\n",
      __func__, session->session_id, rc);
    return rc;
  }

  for(isp_id = VFE0; isp_id < VFE_MAX; isp_id++) {
    if (session->vfe_ids & (1 << isp_id)) {

      rc = get_hw_streams_using_vfe(session, isp_id, hw_stream_ids,
        num_hw_streams, used_hw_stream_ids, &used_num_hw_streams);

      if(rc >= 0)
        rc = isp_ch_util_streamoff_int(isp, isp_id, session,
          used_num_hw_streams, used_hw_stream_ids, FALSE,
          stop_immediately);
      if (rc < 0) {
        CDBG_ERROR("%s: error, isp_util_streamoff, "
          "sessid = %d, vfe_id = %d, rc = %d\n",
          __func__, session->session_id, VFE0, rc);
        return rc;
      }
    }
  }

  CDBG_HIGH("%s: session_id = %d, vfe_mask = 0x%x, async streamoff, rc = %d\n",
    __func__, session->session_id, session->vfe_ids, rc);

  /*Streamoff ACK for VFE0 & VFE1*/
  if (rc == 0) {
    for(isp_id = VFE0; isp_id < VFE_MAX; isp_id++) {
      if (session->vfe_ids & (1 << isp_id)) {

        rc = get_hw_streams_using_vfe(session, isp_id, hw_stream_ids,
          num_hw_streams, used_hw_stream_ids, &used_num_hw_streams);

        if(rc >= 0)
          rc = isp_ch_util_streamoff_ack(isp, isp_id, session,
              used_num_hw_streams, used_hw_stream_ids);
        if (rc < 0) {
          CDBG_ERROR("%s: error, isp_util_streamoff_ack, "
            "sessid = %d, vfe_id = %d, rc = %d\n",
            __func__, session->session_id, VFE0, rc);
          return rc;
        }
      }
    }

    CDBG_HIGH("%s: session_id = %d, sync ack done\n",
      __func__, session->session_id);

    for (i = 0; i < num_hw_streams; i++) {
      channel = isp_ch_util_find_channel_in_session(session, hw_stream_ids[i]);
      /* streamoff success change state to HW_CFG */
      if(!channel){
        CDBG_ERROR("%s: cannot find channel, session_id = %d, hw_stream_id = %d\n",
          __func__, session->session_id, hw_stream_ids[i]);
      } else {
        channel->state = ISP_CHANNEL_STATE_HW_CFG;
      }
    }

  }

  return rc;
}

/** isp_ch_util_adjust_crop_factor
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_adjust_crop_factor(isp_session_t *session,
  uint32_t old_crop_factor, uint32_t *new_crop_factor)
{
  uint32_t i;
  isp_channel_t *channel;
  isp_channel_t *hw_channels[2];
  uint32_t right_stripe_offset;
  cam_dimension_t input_dim;
  cam_dimension_t output_dims[2];
  uint32_t num_hw_channels = 0;
  uint32_t M[2], N[2], cropped_N[2] = {0, 0};
  isp_port_t *isp_sink_port;

  /* no need to adjust if zoom ratio is 1 */
  if (old_crop_factor == Q12)
    return 0;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (session->channel[i].state != ISP_CHANNEL_STATE_INITIAL &&
       ((session->channel[i].channel_id & ISP_META_CHANNEL_BIT)== 0)) {
      channel = &session->channel[i];
      isp_sink_port = channel->sink_port->port_private;
      if (isp_sink_port && isp_sink_port->u.sink_port.caps.use_pix &&
        (channel->channel_type == ISP_CHANNEL_TYPE_IMAGE)) {
        if (num_hw_channels > 1) {
          CDBG_ERROR("%s: found more than 2 hw channels!\n", __func__);
          return -1;
        }
        input_dim.width = channel->cfg.sensor_cfg.request_crop.last_pixel -
          channel->cfg.sensor_cfg.request_crop.first_pixel + 1;
        input_dim.height = channel->cfg.sensor_cfg.request_crop.last_line -
          channel->cfg.sensor_cfg.request_crop.first_line + 1;
        output_dims[num_hw_channels] = channel->stream_info.dim;
        right_stripe_offset = channel->cfg.ispif_out_cfg.right_stripe_offset -
          channel->cfg.sensor_cfg.request_crop.first_pixel;
        num_hw_channels++;
      }
    }
  }

  if (num_hw_channels == 0)
    return 0;

  for (i = 0; i < num_hw_channels; i++) {
    if (input_dim.width * output_dims[i].height >
      input_dim.height * output_dims[i].width) {
      N[i] = input_dim.height;
      M[i] = output_dims[i].height;
    } else {
      N[i] = input_dim.width;
      M[i] = output_dims[i].width;
    }
  }

  cropped_N[0] = N[0] * Q12 / old_crop_factor;
  while (cropped_N[0] < N[0]) {
    uint32_t crop_factor = N[0] * Q12 / cropped_N[0];
    if (num_hw_channels == 2)
      cropped_N[1] = N[1] * Q12 / crop_factor;
    if (isp_ch_util_is_right_stripe_offset_usable(
      M[0], cropped_N[0], right_stripe_offset)) {
      if (num_hw_channels == 1 || isp_ch_util_is_right_stripe_offset_usable(
        M[1], cropped_N[1], right_stripe_offset)) {
        /* double check against precision loss */
        if (cropped_N[0] == N[0] * Q12 / crop_factor) {
          *new_crop_factor = crop_factor;
          return 0;
        }
      }
    }
    cropped_N[0]++;
  }

  CDBG_ERROR("%s: failed to find new crop factor; old = %d\n",
    __func__, old_crop_factor);

  return -1;
}

/** isp_ch_util_unconfig_channel
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_unconfig_channel(
  isp_t *isp, int isp_id, isp_channel_t *channel)
{
  int rc = 0;
  isp_hw_stream_uncfg_t stream_uncfg;

  memset(&stream_uncfg, 0, sizeof(stream_uncfg));
  stream_uncfg.session_id = channel->session_id;
  stream_uncfg.num_streams = 1;
  stream_uncfg.stream_ids[0] = channel->channel_id;

  /* if we have not started VFE yet no need to send uncfg to HW */
  if (isp->data.hw[isp_id].hw_ops) {
    rc = isp->data.hw[isp_id].hw_ops->set_params(
       isp->data.hw[isp_id].hw_ops->ctrl,
       ISP_HW_SET_PARAM_STREAM_UNCFG,
       (void *)&stream_uncfg, sizeof(stream_uncfg));
  }
  channel->state = ISP_CHANNEL_STATE_CREATED;

  return rc;
}

/** isp_ch_util_buf_divert_notify
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_buf_divert_notify(
  isp_t *isp, isp_frame_divert_notify_t *divert_event)
{
  int idx, rc = 0;
  struct msm_isp_event_data *buf_divert = divert_event->isp_event_data;
  isp_buf_divert_t pp_divert;
  mct_event_t mct_event;
  isp_port_t *isp_port;
  isp_channel_t *channel;
  isp_frame_buffer_t *image_buf;
  uint8_t src_port_idx = ISP_SRC_PORT_DATA;
  struct msm_isp_event_data buf_event;
  isp_hw_t *isp_hw;
  uint32_t isp_id;

   ATRACE_INT("Camera:FR", 1);
   ATRACE_INT("Camera:FR", 0);
  isp_session_t *session =
    isp_util_find_session(isp, buf_divert->u.buf_done.session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n",
      __func__, buf_divert->u.buf_done.session_id);
    goto error;
  }

  ISP_DBG(ISP_MOD_COM,"%s: session_id = %d, channel_id = %d\n", __func__,
    buf_divert->u.buf_done.session_id, buf_divert->u.buf_done.stream_id);

  channel = isp_ch_util_find_channel_in_session(session,
    buf_divert->u.buf_done.stream_id);
  if (!channel) {
    CDBG_ERROR("%s: error, cannot find channel, session_id = %d, channel_id = %d\n",
      __func__, buf_divert->u.buf_done.session_id,
      buf_divert->u.buf_done.stream_id);
    goto error;
  }

  if (channel->divert_to_3a) {
    src_port_idx = ISP_SRC_PORT_3A;
  }

  memset(&mct_event,  0,  sizeof(mct_event));
  mct_event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
  mct_event.u.module_event.module_event_data = (void *)&pp_divert;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.identity = pack_identity(buf_divert->u.buf_done.session_id,
    buf_divert->u.buf_done.stream_id);
  mct_event.direction = MCT_EVENT_DOWNSTREAM;

  memset(&pp_divert, 0, sizeof(pp_divert));
  pp_divert.identity = mct_event.identity;
  pp_divert.native_buf = channel->use_native_buf;
  idx = buf_divert->u.buf_done.buf_idx;
  /* Assign the below value to PProc which can be used for buf_done event*/

  pp_divert.is_skip_pproc = buf_divert->is_skip_pproc;
  pp_divert.handle = buf_divert->u.buf_done.handle;
  pp_divert.output_format = buf_divert->u.buf_done.output_format;
  pp_divert.input_intf = buf_divert->input_intf;

  image_buf = isp_get_buf_by_idx(&isp->data.buf_mgr,
    channel->bufq_handle, idx);
  if (!image_buf) {
    CDBG_ERROR("%s: isp_get_buf_by_idx error\n", __func__);
    return -1;
  }

 #ifdef ISP_IMG_DUMP_ENABLE
    isp_ch_util_dump_frame(isp->data.buf_mgr.ion_fd, &channel->stream_info,
      image_buf, buf_divert->frame_id, channel->meta_info.dump_to_fs);
 #endif

  /* TODO: This is not needed since we have copied the vaddr
   * in isp_init_hal_buffer. */
  if (channel->divert_to_3a)
    pp_divert.vaddr = image_buf->vaddr;
  else
    pp_divert.vaddr = image_buf->addr;

  pp_divert.fd = image_buf->fd;
  pp_divert.buffer = image_buf->buffer;
  pp_divert.buffer.sequence = buf_divert->frame_id;
  pp_divert.buffer.timestamp = buf_divert->timestamp;
  pp_divert.channel_id = buf_divert->u.buf_done.stream_id;
  pp_divert.is_uv_subsampled =
    (buf_divert->u.buf_done.output_format == V4L2_PIX_FMT_NV14 ||
     buf_divert->u.buf_done.output_format == V4L2_PIX_FMT_NV41);

  if (isp->prev_sent_streamids_cnt < MAX_STREAMS_NUM) {
    isp->prev_sent_streamids[isp->prev_sent_streamids_cnt++] =
      buf_divert->u.buf_done.stream_id;
  } else {
    CDBG_ERROR("%s: error: isp->prev_sent_streamids_cnt %d is more than %d\n",
      __func__, isp->prev_sent_streamids_cnt, MAX_STREAMS_NUM);
    goto error;
  }

   if (channel->meta_info.is_valid && channel->meta_info.dump_to_fs) {
    isp_queue_buf(&isp->data.buf_mgr, channel->bufq_handle,
        pp_divert.buffer.index, TRUE, 0);
    return 0;
  }


  if (channel->src_ports[src_port_idx]) {
    if (FALSE == mct_port_send_event_to_peer(
      channel->src_ports[src_port_idx], &mct_event)) {
      CDBG_ERROR("%s: error when buf_divert to pp, sessid %d, streamid %d\n",
        __func__, buf_divert->u.buf_done.session_id,
        buf_divert->u.buf_done.stream_id);

      rc = isp_queue_buf(&isp->data.buf_mgr, channel->bufq_handle,
        pp_divert.buffer.index, TRUE, 0);
      goto error;
    } else {
      /* in success case we need to check
       * if we have piggy back ack in the return */
      if (pp_divert.ack_flag == TRUE) {
        /* process the piggy back ack */
        divert_event->ack_flag = pp_divert.ack_flag;
        divert_event->is_buf_dirty = pp_divert.is_buf_dirty;

         if (divert_event->is_buf_dirty == TRUE) {
           rc = isp_queue_buf(&isp->data.buf_mgr, channel->bufq_handle,
                              pp_divert.buffer.index, pp_divert.is_buf_dirty, 0);
         } else {
           if (session->vfe_ids & (1 << VFE0))
             isp_id = VFE0;
           else if (session->vfe_ids & (1 << VFE1))
             isp_id = VFE1;
           else {
             CDBG_ERROR("%s: no ISP is created yet\n", __func__);
             return -1;
           }
           isp_hw = (isp_hw_t *)isp->data.hw[isp_id].hw_ops->ctrl;

           /* do a buf done since CPP didnt process the buf */
           CDBG_LOW("%s: LPM enable\n", __func__);
           memset(&buf_event, 0, sizeof(buf_event));
           buf_event.input_intf = pp_divert.input_intf;
           buf_event.frame_id = buf_divert->frame_id;
           buf_event.timestamp = buf_divert->timestamp;
           buf_event.u.buf_done.session_id = buf_divert->u.buf_done.session_id;
           buf_event.u.buf_done.stream_id = buf_divert->u.buf_done.stream_id;
           buf_event.u.buf_done.output_format =
              pp_divert.output_format;
           buf_event.u.buf_done.buf_idx =
              buf_divert->u.buf_done.buf_idx;
           buf_event.u.buf_done.handle =
              pp_divert.handle;

           rc = ioctl(isp_hw->fd, VIDIOC_MSM_ISP_BUF_DONE, &buf_event);
           if (rc < 0) {
             CDBG_ERROR("%s: VIDIOC_MSM_ISP_BUF_DONE error = %d\n", __func__, rc);
             return rc;
           }
         }
       }
     }
   } else {
    CDBG_ERROR("%s: no src frame port linked, sessid = %d, streamid = %d\n",
      __func__, buf_divert->u.buf_done.session_id,
      buf_divert->u.buf_done.stream_id);

    rc = isp_queue_buf(&isp->data.buf_mgr, channel->bufq_handle,
      pp_divert.buffer.index, TRUE, 0);
    goto error;
  }

  return 0;
error:
  return -1;
}

/** isp_ch_util_hw_notify_hw_updating
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_hw_notify_hw_updating(
  isp_t *isp, isp_hw_updating_notify_t *notify_data)
{
  int rc = 0;
  isp_session_t *session;

  session = isp_util_find_session(isp, notify_data->session_id);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n",
               __func__, notify_data->session_id);
    return -1;
  }

  pthread_mutex_lock(
    &isp->data.session_critical_section[session->session_idx]);
  if (notify_data->is_hw_updating)
    session->vfe_updating_mask = session->vfe_ids;
  else
    session->vfe_updating_mask &= ~(1 << notify_data->dev_idx);

  if (!session->vfe_updating_mask) {
    if (session->async_task.wait_hw_update_done) {
      session->async_task.wait_hw_update_done = FALSE;
      /* wake up the waiting async_task */
      sem_post(&session->async_task.hw_wait_sem);
    }
    if(session->uv_subsample_ctrl.min_wait_cnt > 0) {
       session->uv_subsample_ctrl.min_wait_cnt --;
       ISP_DBG(ISP_MOD_COM,"%s: uv subsampling in process min_cnt %d", __func__,
         session->uv_subsample_ctrl.min_wait_cnt);
      if (session->uv_subsample_ctrl.min_wait_cnt == 0)
        session->uv_subsample_ctrl.switch_state = ISP_UV_SWITCH_STATE_IDLE;
    }
  }
  pthread_mutex_unlock(
      &isp->data.session_critical_section[session->session_idx]);

  return rc;
}

/** isp_ch_util_hw_notify_meta_valid
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_hw_notify_meta_valid(isp_t *isp, mct_bus_msg_t *bus_msg)
{
    if (TRUE != mct_module_post_bus_msg(isp->module, bus_msg))
      CDBG_ERROR("%s: SOF to bus error\n", __func__);

   return TRUE;
}
/** isp_ch_util_hw_notify_sof
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_hw_notify_sof(isp_t *isp, mct_bus_msg_t *bus_msg,
  void *ctrl, uint32_t isp_id)
{
  int i, j, cnt = 0, rc = 0;
  uint32_t mask;
  mct_bus_msg_isp_sof_t *sof_event = bus_msg->msg;
  isp_session_t *session;
  isp_channel_t *channel;
  isp_port_t *isp_sink_port;
  boolean need_notify_sof = TRUE;
  isp_hw_t *isp_hw = (isp_hw_t *)ctrl;

  session = isp_util_find_session(isp, bus_msg->sessionid);
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n",
               __func__, bus_msg->sessionid);
    return -1;
  }

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    channel = &session->channel[i];
    if (!channel->sink_port)
      continue;

    isp_sink_port = channel->sink_port->port_private;
    if (isp_sink_port &&
      MAX_STREAMS_NUM > cnt){
      mask = channel->user_stream_idx_mask;

      for (j = 0; j < ISP_MAX_STREAMS; j++) {
        /* this stream needs pixel interface */
        if (((1 << j) & mask) && mask && MAX_STREAMS_NUM > cnt) {
          sof_event->streamids[cnt++] = session->streams[j].stream_id;
          mask &= ~(1 << j);
        }
      }
    }
  }

  for (i = 0; i < MAX_STREAMS_NUM; i++) {
    sof_event->prev_sent_streamids[i] = isp->prev_sent_streamids[i];
    isp->prev_sent_streamids[i] = 0;
  }
  isp->prev_sent_streamids_cnt = 0;

  pthread_mutex_lock(&isp->mutex);

  /*only dual isp need to check if send out sof notify
    single vfe should always send out notify*/
  if ((session->vfe_ids & (1<< VFE0)) && (session->vfe_ids & (1 << VFE1))) {
    /*if the updated sof id > any other isp id,
      means the other isps hasnt got this sof frame yet, so no sof notify*/
    session->sof_id[isp_id] = sof_event->frame_id;
    for (i = 0; i < VFE_MAX; i++) {
      if (sof_event->frame_id > session->sof_id[i])
        need_notify_sof = FALSE;
    }
  } else {
    if (sof_event->frame_id <= session->sof_id[0])
      need_notify_sof = FALSE;
    else
      session->sof_id[0] = sof_event->frame_id;
  }
  pthread_mutex_unlock(&isp->mutex);
  sof_event->num_streams = cnt;
  if (sof_event->num_streams > 0 && need_notify_sof) {
    isp_hw_updating_notify_t notify_data;
    notify_data.session_id = session->session_id;
    notify_data.is_hw_updating = 1;
    notify_data.dev_idx = session->vfe_ids;

    isp_ch_util_hw_notify_hw_updating(isp, &notify_data);
    isp_util_broadcast_zoom_crop(isp, bus_msg->sessionid,
      sof_event->num_streams, sof_event->streamids,
      sof_event->frame_id, &sof_event->timestamp);
    isp_util_broadcast_pproc_zoom_crop(isp, bus_msg->sessionid,
      sof_event->num_streams, sof_event->streamids,
      sof_event->frame_id, &sof_event->timestamp);
    /* The order that send SOF notification to modules
     * and then to media bus is based on the discussion
     * of sensor and mct teams. Sensor asked this order.
     */
    isp_util_broadcast_sof_msg_to_modules(isp,
      bus_msg->sessionid, sof_event->streamids[0], sof_event);

    /* post Meta dump bus message, only post with SOF(Before SOF)*/
    mct_bus_msg_t bus_msg_meta_dump;
    bus_msg_meta_dump.type = MCT_BUS_MSG_ISP_META;
    bus_msg_meta_dump.size = sizeof(isp_meta_t);
    bus_msg_meta_dump.msg = (void *)&isp_hw->dump_info.meta_data;
    bus_msg_meta_dump.sessionid = bus_msg->sessionid;
    if (TRUE != mct_module_post_bus_msg(isp->module, &bus_msg_meta_dump))
      CDBG_ERROR("%s: meta dump data to bus error\n", __func__);
    /* post vfe diag bus message, only post with SOF(Before SOF)*/
    if (isp_hw->isp_diag.has_user_dianostics) {
      mct_bus_msg_t bus_msg_vfe_diag;
      bus_msg_vfe_diag.type = MCT_BUS_MSG_ISP_CHROMATIX_LITE;
      bus_msg_vfe_diag.size = sizeof(vfe_diagnostics_t);
      bus_msg_vfe_diag.msg = (void *)&isp_hw->isp_diag.user_vfe_diagnostics;
      bus_msg_vfe_diag.sessionid = bus_msg->sessionid;
      if (TRUE != mct_module_post_bus_msg(isp->module, &bus_msg_vfe_diag)) {
        CDBG_ERROR("%s: vfe_diag to bus error\n", __func__);
      }
    }
    /* send out SOF after meta bus msg*/
    if (TRUE != mct_module_post_bus_msg(isp->module, bus_msg))
      CDBG_ERROR("%s: SOF to bus error\n", __func__);
  }

  return rc;
}

/** isp_ch_util_convert_crop_to_stream
 *
 * DESCRIPTION:
 *
 **/
void isp_ch_util_convert_crop_to_stream(isp_session_t *session,
  isp_stream_t *user_stream, mct_bus_msg_stream_crop_t *stream_crop,
  isp_zoom_scaling_param_entry_t *entry, isp_t *isp)
{
  isp_channel_t *channel = NULL;
  int i;
  uint32_t mask;

  mask = user_stream->channel_idx_mask;
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (mask & (1 << i)) {
      channel = isp_ch_util_find_channel_in_session_by_idx(session, i);
      break;
    }
  }
  if (!channel)
    return;

  if (!entry->in_width || !entry->in_height) {
    stream_crop->crop_out_x = channel->stream_info.dim.width;
    stream_crop->crop_out_y = channel->stream_info.dim.height;
    stream_crop->x = 0;
    stream_crop->y = 0;
  } else {
    if (GET_ISP_MAIN_VERSION(isp->data.sd_info.sd_info[0].isp_version)
      == ISP_VERSION_40){
      stream_crop->crop_out_x =
        channel->stream_info.dim.width * entry->out_width / entry->in_width;
      if ((int)stream_crop->crop_out_x > (int)channel->stream_info.dim.width)
        stream_crop->crop_out_x = (uint32_t)channel->stream_info.dim.width;

      stream_crop->crop_out_y =
        channel->stream_info.dim.height * entry->out_height / entry->in_height;
      if ((int)stream_crop->crop_out_y > (int)channel->stream_info.dim.height)
        stream_crop->crop_out_y = (uint32_t)channel->stream_info.dim.height;

    } else {
      stream_crop->crop_out_x = entry->in_width;
      stream_crop->crop_out_y = entry->in_height;
    }

    stream_crop->x =
      (channel->stream_info.dim.width - stream_crop->crop_out_x) >> 1;
    stream_crop->y =
      (channel->stream_info.dim.height - stream_crop->crop_out_y) >> 1;
  }

  /* fill in roi map info*/
  stream_crop->x_map = entry->roi_map_info.first_pixel;
  stream_crop->y_map = entry->roi_map_info.first_line;

  stream_crop->width_map =
    entry->roi_map_info.last_pixel - entry->roi_map_info.first_pixel + 1;
  stream_crop->height_map =
    entry->roi_map_info.last_line - entry->roi_map_info.first_line + 1;

  ISP_DBG(ISP_MOD_COM,"%s: identy = 0x%x, roi_x_map = %d, roi_y_map = %d, roi_width_map = %d, roi_height_map = %d, "
    "x = %d, y = %d, crop_x = %d, crop_y = %d\n",
     __func__, user_stream->stream_info.identity,
    stream_crop->x_map,  stream_crop->y_map,
    stream_crop->width_map, stream_crop->height_map,
    stream_crop->x, stream_crop->y,
    stream_crop->crop_out_x, stream_crop->crop_out_y);
}

/** isp_ch_util_get_image_channel
 *
 * DESCRIPTION:
 *
 **/
isp_channel_t *isp_ch_util_get_image_channel(
  isp_session_t *session, uint32_t channel_mask)
{
  int i;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (channel_mask & (1 << i) &&
        session->channel[i].channel_type == ISP_CHANNEL_TYPE_IMAGE) {
      return &session->channel[i];;
    }
  }

  return NULL;
}

/** isp_ch_util_get_channel_bufq_handle
 *
 * DESCRIPTION:
 *
 **/
static uint32_t isp_ch_util_get_channel_bufq_handle(
  isp_session_t *session, uint32_t channel_id)
{
  int i;
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (session->channel[i].channel_id == channel_id &&
        session->channel[i].state != ISP_CHANNEL_STATE_INITIAL) {
      return session->channel[i].bufq_handle;
    }
  }

  return 0;
}

/** get_channel_by_stream_id:
 *    @session: isp session
 *    @channel_id: ID of channel we look for
 *
 *  This function runs in MCTL thread context.
 *
 *  This function returns a channel with given ID.
 *
 *  Return: NULL      - Channel not found.
 *          Otherwise - Pointer to the channel.
 **/
static isp_channel_t *get_channel_by_stream_id(
  isp_session_t *session, uint32_t stream_id)
{
  int i, stream_idx;

  for(stream_idx = 0; stream_idx < ISP_MAX_STREAMS; stream_idx++)
    if(session->streams[stream_idx].stream_id == stream_id)
      break;

  if(stream_idx < ISP_MAX_STREAMS)
    for (i = 0; i < ISP_MAX_STREAMS; i++) {
      if (session->channel[i].user_stream_idx_mask & (1 << stream_idx)) {
        return &(session->channel[i]);
      }
    }

  return NULL;
}

/** isp_ch_util_divert_ack
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_divert_ack(isp_t *isp, isp_session_t *session,
  isp_buf_divert_ack_t *divert_ack)
{
  int rc = 0;
  isp_hw_buf_divert_ack_t ack;
  uint32_t bufq_handle = 0;

  bufq_handle = isp_ch_util_get_channel_bufq_handle(session, divert_ack->channel_id);
  if (bufq_handle == 0) {
    CDBG_ERROR("%s: error, session_id = %d, channel_id = %d, bufq_handle = %d\n",
      __func__, session->session_id, divert_ack->channel_id, bufq_handle);
    return -1;
  }

  rc = isp_queue_buf(&isp->data.buf_mgr, bufq_handle,
    divert_ack->buf_idx, divert_ack->is_buf_dirty, 0);
  if (rc < 0)
    CDBG_ERROR("%s: error, session_id = %d, channel_id = %d, rc = %d\n",
      __func__, session->session_id, divert_ack->channel_id, rc);

  return rc;
}

/** isp_ch_util_set_param
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_set_param(isp_t *isp, isp_session_t *session,
  int stream_id, isp_hw_set_params_id_t param_id,
  void *data, uint32_t size)
{
  int rc = 0, isp_id;

  if (session->vfe_ids & (1 << VFE0)) {
    isp_id = VFE0;

    if (isp->data.hw[isp_id].hw_ops) {
      rc = isp->data.hw[isp_id].hw_ops->set_params(
      isp->data.hw[isp_id].hw_ops->ctrl,
      param_id, (void *)data, size);
      if (rc < 0) {
        CDBG_ERROR("%s: error, session_id = %d, "
          "stream_id = %d, isp_id = %d, set_param_id = %d, rc = %d",
          __func__, session->session_id, stream_id, isp_id, param_id, rc);

        return rc;
      }
    }
  }

  if (session->vfe_ids & (1 << VFE1)) {
    isp_id = VFE1;

    if (isp->data.hw[isp_id].hw_ops) {
      rc = isp->data.hw[isp_id].hw_ops->set_params(
      isp->data.hw[isp_id].hw_ops->ctrl,
      param_id, (void *)data, size);
      if (rc < 0) {
        CDBG_ERROR("%s: error, session_id = %d, "
          "stream_id = %d, isp_id = %d, set_param_id = %d, rc = %d",
          __func__, session->session_id, stream_id, isp_id, param_id, rc);
        return rc;
      }
    }
  }

  return rc;
}

/** isp_ch_util_get_param
 *
 * DESCRIPTION:
 *
 **/
int isp_ch_util_get_param(isp_t *isp, isp_session_t *session,
  int stream_id, isp_hw_set_params_id_t param_id, void *in_data,
  uint32_t in_size, void *out_data, uint32_t out_size)
{
  int rc = 0, isp_id;

  if (!isp || !session || !in_data || !out_data) {
    CDBG_ERROR("%s:%d failed: %p %p %p %p\n", __func__, __LINE__,
      isp, session, in_data, out_data);
    return -1;
  }

  if (session->vfe_ids & (1 << VFE0)) {
    isp_id = VFE0;
    if (!isp->data.hw[isp_id].hw_ops) {
      CDBG_ERROR("%s:%d failed: hw_ops NULL for VFE0\n", __func__, __LINE__);
      return -1;
    }
    rc = isp->data.hw[isp_id].hw_ops->get_params(
    isp->data.hw[isp_id].hw_ops->ctrl,
    param_id, in_data, in_size, out_data, out_size);
    if (rc < 0) {
      CDBG_ERROR("%s: error, session_id = %d, "
        "stream_id = %d, isp_id = %d, set_param_id = %d, rc = %d",
        __func__, session->session_id, stream_id, isp_id, param_id, rc);
      return rc;
    }
  }

  if (session->vfe_ids & (1 << VFE1)) {
    isp_id = VFE1;
    if (!isp->data.hw[isp_id].hw_ops) {
      CDBG_ERROR("%s:%d failed: hw_ops NULL for VFE1\n", __func__, __LINE__);
      return -1;
    }
    rc = isp->data.hw[isp_id].hw_ops->get_params(
      isp->data.hw[isp_id].hw_ops->ctrl, param_id, in_data,
      in_size, out_data, out_size);
    if (rc < 0) {
      CDBG_ERROR("%s: error, session_id = %d, "
        "stream_id = %d, isp_id = %d, set_param_id = %d, rc = %d",
        __func__, session->session_id, stream_id, isp_id, param_id, rc);
      return rc;
    }
  }

  return rc;
}

/** isp_ch_util_all_streams_off
 *
 * DESCRIPTION:
 *
 **/
void isp_ch_util_all_streams_off(isp_t *isp, isp_session_t *session)
{
  int i;

  if (session->active_count == 0) {
    /* after all streams are off. WE reset straem state to user_cfg. */
    for (i = 0; i < ISP_MAX_STREAMS; i++) {
      if (session->channel[i].channel_id > 0) {
        session->channel[i].state = ISP_CHANNEL_STATE_USER_CFG;
      }
    }
  }
}

/** isp_ch_util_reg_buf_list_update
 *    @isp: isp instance
 *    @session: session instance
 *    @stream_id: stream id for which buffers will be updated
 *    @map_buffer_list: Contains information of buffer to be
 *                    appended
 *
 *  Using the MCT buffer received, this function prepares
 *  information to be passed to buffer manager, such as buf mgr,
 *  bufq handle.
 *
 *  Returns 0 for success and negative error for failure
 **/
int isp_ch_util_reg_buf_list_update(isp_t *isp, isp_session_t *session,
  int stream_id, mct_stream_map_buf_t * map_buffer_list)
{
  int rc = 0;
  isp_buf_request_t buf_request;
  isp_channel_t * channel;

  ISP_DBG(ISP_MOD_COM,"%s: E, sessid = %d, stream_id = %d\n", __func__,
    session->session_id, stream_id);

  uint32_t bufq_handle = isp_ch_util_get_channel_bufq_handle(session, stream_id);
  ISP_DBG(ISP_MOD_COM,"%s: E, bufq_handle = %x map_buffer_list %p\n", __func__, bufq_handle,
     map_buffer_list);

  assert(map_buffer_list != NULL);
  memset(&buf_request, 0, sizeof(buf_request));
  buf_request.buf_handle = bufq_handle;
  buf_request.session_id = session->session_id;
  buf_request.stream_id = stream_id;

  channel = get_channel_by_stream_id(session, stream_id);

  if (!channel) {
    CDBG_ERROR("%s: error: cannot find channel %d\n", __func__, stream_id);
    return rc;
  }

  /* if channel is split between dual VFEs use shared buf type
     single ISP case use private buf type */
  if (channel->cfg.ispif_out_cfg.is_split)
    buf_request.buf_type = ISP_SHARE_BUF;
  else
    buf_request.buf_type = ISP_PRIVATE_BUF;

  /* MCT sends one buffer at a time, we update internal buf array
     for which we have to pass a buf list. Create a list using buf
     received from MCT*/
  buf_request.img_buf_list = mct_list_append(buf_request.img_buf_list,
    map_buffer_list,NULL, NULL);

  rc = isp_register_buf_list_update(&isp->data.buf_mgr, bufq_handle, &buf_request, 0);
  if (rc < 0) {
    CDBG_ERROR("%s: unable to update new buf list!\n", __func__);
    return rc;
  }
  buf_request.img_buf_list = mct_list_remove(buf_request.img_buf_list,
    map_buffer_list);
  ISP_DBG(ISP_MOD_COM,"%s: X, rc = %d\n", __func__, rc);
  return rc;
}
