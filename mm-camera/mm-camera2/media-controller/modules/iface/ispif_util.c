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
#include "cam_types.h"
#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "ispif.h"
#include "ispif_util.h"
#include "isp_resource_mgr.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#ifdef ISPIF_UTIL_DEBUG
#undef CDBG
#define CDBG ALOGE
#endif

/** ispif_get_match_src_port_t
  *    @ispif_sink_port
  *    @stream
  *
  * Helper struct for finding matching source port
  **/
typedef struct {
  ispif_port_t *ispif_sink_port;
  ispif_stream_t *stream;
} ispif_get_match_src_port_t;


/** ispif_util_add_stream:
 *    @ispif: ispif instance
 *    @session: session instance
 *    @stream_id: stream ID
 *    @stream_info: MCTL stream data
 *
 *  This function runs in MCTL thread context.
 *
 *  This function adds a stream to ispif module
 *
 *  Return: NULL - Error
 *          Otherwise - Pointer to added stream
 **/
ispif_stream_t *ispif_util_add_stream(ispif_t *ispif, ispif_session_t *session,
  uint32_t stream_id, mct_stream_info_t *stream_info)
{
  int i, rc = 0;
  if (session == NULL) {
    CDBG_ERROR("%s: no more session availabe, max = %d\n", __func__,
      ISP_MAX_SESSIONS);

    return NULL;
  }
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (session->streams[i].session == NULL) {
      /* found an empty slot */
      memset(&session->streams[i], 0, sizeof(session->streams[i]));
      session->streams[i].session = (void *)session;
      session->streams[i].session_id = session->session_id;
      session->streams[i].stream_id = stream_id;
      session->streams[i].stream_info = *stream_info;
      session->streams[i].state = ISPIF_STREAM_CREATED;
      session->streams[i].stream_idx = i;
      session->num_stream++;

      return &session->streams[i];
    }
  }

  return NULL;
}

/** ispif_util_del_stream:
 *    @ispif: ispif instance
 *    @stream: stream  to be deleted
 *
 *  This function runs in MCTL thread context.
 *
 *  This function deletes a stream from ispif module
 *
 *  Return: 0 - Success
 *         -1 - Stream used by some port
 **/
int ispif_util_del_stream(ispif_t *ispif,ispif_stream_t *stream)
{
  int i, rc = 0, is_new_session = 1;
  ispif_session_t *session = (ispif_session_t *)stream->session;
  ispif_stream_t *tmp_stream;

  if (stream->link_cnt > 0) {
    CDBG_ERROR("%s: stream used by sink/src port, link_cnt = %d,error\n",
      __func__, stream->link_cnt);

    return -1;
  }

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    tmp_stream = &session->streams[i];
    if (tmp_stream == stream) {
      memset(stream, 0, sizeof(ispif_stream_t));
      session->num_stream--;
      break;
    }
  }

  return 0;
}

/** ispif_util_find_stream:
 *    @ispif: ispif pointer
 *    @session_id: Session id
 *    @stream_id: Stream ID
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds a stream with certain ID in session with given id
 *
 *  Return: NULL - Stream not found
 *          Otherwise - pointer to stream found
 **/
ispif_stream_t *ispif_util_find_stream(ispif_t *ispif, uint32_t session_id,
  uint32_t stream_id)
{
  int i, k;

  for (i = 0; i < ISP_MAX_SESSIONS; i++) {
    if (ispif->sessions[i].ispif &&
        ispif->sessions[i].session_id == session_id) {
      CDBG("%s: found session %d\n", __func__, ispif->sessions[i].session_id);
      /* find the session */
      for (k = 0; k < ISP_MAX_STREAMS; k++) {
        if (ispif->sessions[i].streams[k].session &&
            ispif->sessions[i].streams[k].stream_id == stream_id) {
          CDBG("%s:Found stream: sessionid = %d, streamid = %d\n", __func__,
            ispif->sessions[i].session_id,
            ispif->sessions[i].streams[k].stream_id);

          return &ispif->sessions[i].streams[k];
        }
      }
    }
  }

  return NULL;
}

/** ispif_util_find_stream_in_session:
 *    @session: Session to find stream in
 *    @stream_id: Stream id
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds a stream with certain ID in session
 *
 *  Return: NULL - Stream not found
 *          Otherwise - pointer to stream found
 **/
ispif_stream_t *ispif_util_find_stream_in_session(ispif_session_t *session,
  uint32_t stream_id)
{
  int i;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    CDBG("%s: stream: sessionid = %d, streamid = %d\n",
      __func__, session->session_id,
      session->streams[i].stream_id);
    if (session->streams[i].session &&
          session->streams[i].stream_id == stream_id) {

      return &session->streams[i];
    }
  }

  return NULL;
}

/** ispif_util_get_session_by_id:
 *    @ispif: ispif pointer
 *    @session_id: Session ID
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds session with given ID
 *
 *  Return: NULL - Session not found
 *          Otherwise - pointer to session
 **/
ispif_session_t *ispif_util_get_session_by_id(ispif_t *ispif,
  uint32_t session_id)
{
  int i;

  for (i = 0; i < ISP_MAX_SESSIONS; i++) {
    if (ispif->sessions[i].session_id == session_id &&
        ispif->sessions[i].ispif) {

      return &ispif->sessions[i];
    }
  }

  return NULL;
};

/** ispif_util_find_stream_in_sink_port:
 *    @ispif: ispif instance
 *    @ispif_sink_port: ispif sink port
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function checks if stream is linked to given sink port.
 *
 *  Return: TRUE  - Success
 *          FALSE - Stream is not linked to this port
 **/
boolean ispif_util_find_stream_in_sink_port(ispif_t *ispif,
  ispif_port_t *ispif_sink_port, ispif_stream_t *stream)
{
  int i;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (ispif_sink_port->u.sink_port.streams[i] == stream) {

      return TRUE;
    }
  }

  return FALSE;
}
/** ispif_util_find_stream_in_src_port:
 *    @ispif: ispif isntance
 *    @ispif_src_port: ispif source port
 *    @session_id: Session ID
 *    @stream_id: Stream ID
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds a stream linked to ispif source port by ID
 *
 *  Return: NULL - Stream not found
 *          Otherwise - pointer to stream found
 **/
ispif_stream_t *ispif_util_find_stream_in_src_port( ispif_t *ispif,
  ispif_port_t *ispif_src_port, uint32_t session_id, uint32_t stream_id)
{
  int i;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (ispif_src_port->u.src_port.streams[i] != NULL &&
        ispif_src_port->u.src_port.streams[i]->session_id == session_id &&
        ispif_src_port->u.src_port.streams[i]->stream_id == stream_id) {

      return ispif_src_port->u.src_port.streams[i];
    }
  }

  return NULL;
}
/** ispif_util_add_stream_to_sink_port:
 *    @ispif: ispif isntance
 *    @ispif_sink_port: ispif sinc port
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function adds a stream to sink potr
 *
 *  Return: 0 - Success
 *         -1 - Port is connected to its max number of streams
 **/
int ispif_util_add_stream_to_sink_port(ispif_t *ispif,
  ispif_port_t *ispif_sink_port, ispif_stream_t *stream)
{
  int i;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (ispif_sink_port->u.sink_port.streams[i] == NULL) {
      ispif_sink_port->u.sink_port.streams[i] =  stream;
      stream->sink_port = ispif_sink_port->port;
      stream->state = ISPIF_STREAM_ASSOCIATED_WITH_SINK_PORT;
      ispif_sink_port->u.sink_port.num_streams++;

      /* here we choose if pix is needed for the stream */
      ispif_util_choose_isp_interface(ispif, ispif_sink_port,
        (ispif_session_t *)stream->session, stream);
      stream->link_cnt++;
      CDBG("%s: ADD sessid = %d streamid = %d, use_pix = %d, link_cnt = %d\n",
        __func__, stream->session_id, stream->stream_id, stream->use_pix,
        stream->link_cnt);

      return 0;
    }
  }

  return -1;
}

/** ispif_util_del_stream_from_sink_port:
 *    @ispif: ispif isntance
 *    @ispif_sink_port: ispif sinc port
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function deletes a stream from sink port
 *
 *  Return: 0 - Success
 *         -1 - Stream not found
 **/
int ispif_util_del_stream_from_sink_port( ispif_t *ispif,
  ispif_port_t *ispif_sink_port, ispif_stream_t *stream)
{
  int i;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (ispif_sink_port->u.sink_port.streams[i] == stream) {
      stream->link_cnt--;
      ispif_sink_port->u.sink_port.streams[i] = NULL;
      ispif_sink_port->u.sink_port.num_streams--;
      CDBG("%s: link_cnt = %d\n", __func__, stream->link_cnt);

      return 0;
    }
  }

  return -1;
}
/** ispif_util_add_stream_to_src_port:
 *    @ispif: ispif instance
 *    @ispif_src_port: ispif source port
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function ads a stream to source port
 *
 *  Return: 0 - Success
 *         -1 - Port is connected to its max number of streams
 **/
int ispif_util_add_stream_to_src_port(ispif_t *ispif,
  ispif_port_t *ispif_src_port, ispif_stream_t *stream)
{
  int i;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (ispif_src_port->u.src_port.streams[i] == NULL) {
      ispif_src_port->u.src_port.streams[i] =  stream;
      stream->link_cnt++;
      ispif_src_port->u.src_port.num_streams++;
      CDBG("%s: link_cnt = %d\n", __func__, stream->link_cnt);

      return 0;
    }
  }

  return -1;
}
/** ispif_util_del_stream_from_src_port:
 *    @ispif: ispif instance
 *    @ispif_src_port: ispif source port
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function deletes a stream from source port
 *
 *  Return: 0 - Success
 *         -1 - Stream not found
 **/
int ispif_util_del_stream_from_src_port(ispif_t *ispif,
  ispif_port_t *ispif_src_port, ispif_stream_t *stream)
{
  int i;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (ispif_src_port->u.src_port.streams[i] == stream) {
      stream->link_cnt--;
      ispif_src_port->u.src_port.streams[i] = NULL;
      ispif_src_port->u.src_port.num_streams--;
      CDBG("%s: link_cnt = %d\n", __func__, stream->link_cnt);

      return 0;
    }
  }

  return -1;
}

/** ispif_util_find_matched_src_port_by_caps:
 *    @data1: MCTL port
 *    @data2: stream and sink port struct to match against
 *
 *  This function runs in MCTL thread context.
 *
 *  This function is a visitor: finds a matching source port for given stream
 *                               in a list
 *
 *  Return: TRUE  - Success
 *          FALSE - Port does not match
 **/
static boolean ispif_util_find_matched_src_port_by_caps(void *data1, void *data2)
{
  mct_port_t *mct_port = (mct_port_t *)data1;
  ispif_get_match_src_port_t *params = (ispif_get_match_src_port_t *)data2;
  ispif_port_t *ispif_src_port = (ispif_port_t * )mct_port->port_private;
  ispif_port_t *ispif_sink_port = params->ispif_sink_port;
  ispif_sink_port_t *sink_port = &ispif_sink_port->u.sink_port;
  ispif_src_port_t *src_port = &ispif_src_port->u.src_port;

  if (ispif_src_port->state != ISPIF_PORT_STATE_CREATED &&
      memcmp(&sink_port->sensor_cap, &src_port->caps.sensor_cap,
        sizeof(sink_port->sensor_cap)) == 0 &&
      params->stream->use_pix == src_port->caps.use_pix) {

        /* this src port is for that stream */
        return TRUE;
  }
  return FALSE;
}

/** ispif_util_get_match_src_port:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: ispif pointer
 *    @stream: ispif pointer
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds matching source port for a stream
 *
 *  Return: NULL - Port not found
 *          Otherwise - Pointer to matching port
 **/
ispif_port_t *ispif_util_get_match_src_port(ispif_t *ispif,
  ispif_port_t *ispif_sink_port, ispif_stream_t *stream)
{
  int i, rc = 0;
  ispif_sink_port_t *sink_port = &ispif_sink_port->u.sink_port;
  ispif_src_port_t *src_port = NULL;
  ispif_port_t *ispif_src_port = NULL;
  ispif_get_match_src_port_t params;
  mct_list_t *src_port_list = NULL;
  mct_port_t *mct_port = NULL;

  memset(&params,  0,  sizeof(params));
  params.ispif_sink_port = ispif_sink_port;
  params.stream = stream;

  src_port_list = mct_list_find_custom (ispif->module->srcports,
    (void *)&params, ispif_util_find_matched_src_port_by_caps);

  if (src_port_list) {
    mct_port = (mct_port_t *)src_port_list->data;
    ispif_src_port = (ispif_port_t *)mct_port->port_private;
  }

  return ispif_src_port;
}

/** ispif_util_find_sink_port_by_caps:
 *    @data1: MCTL port
 *    @data2: sensor source port capabilities
 *
 *  This function runs in MCTL thread context.
 *
 *  This function is a visitor: finds a matching sink port for given sensor
 *                              port capabilities
 *
 *  Return: TRUE  - Success
 *          FALSE - Port does not match
 **/
static boolean ispif_util_find_sink_port_by_caps(void *data1, void *data2)
{
  sensor_src_port_cap_t *sensor_cap = (sensor_src_port_cap_t *)data2;
  mct_port_t *mct_port = (mct_port_t *)data1;
  ispif_port_t *ispif_sink_port = (ispif_port_t * )mct_port->port_private;
  ispif_sink_port_t *sink_port = &ispif_sink_port->u.sink_port;

  if (ispif_sink_port->state != ISPIF_PORT_STATE_CREATED &&
      memcmp(sensor_cap, &sink_port->sensor_cap,
        sizeof(sensor_src_port_cap_t)) == 0) {
      /* has the match */
    CDBG("%s: find an old sink port\n", __func__);
      return TRUE;
  } else
    return FALSE;
}

/** ispif_util_find_sink_port:
 *    @ispif: ispif instance
 *    @sensor_cap: ispif pointer
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds a sink port matching to a sensor source port with given
 *  capabilities
 *
 *  Return: NULL - Port not fouund
 *          Otherwise - Pointer to ispif sink port
 **/
ispif_port_t *ispif_util_find_sink_port(ispif_t *ispif,
  sensor_src_port_cap_t *sensor_cap)
{
  mct_list_t *sink_port_list = NULL;
  mct_port_t *mct_port = NULL;
  ispif_port_t *ispif_sink_port = NULL;

  sink_port_list = mct_list_find_custom(ispif->module->sinkports,
                                        (void *)sensor_cap,
                                        ispif_util_find_sink_port_by_caps);
  if (sink_port_list != NULL) {
    mct_port = (mct_port_t *)sink_port_list->data;
    ispif_sink_port = (ispif_port_t *)mct_port->port_private;
    CDBG("%s: old sink port used\n", __func__);
  }

  return ispif_sink_port;
}

/** ispif_util_config_src_port:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: ispif sink port
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function confugures a source port
 *
 *  Return: 0 - Success
 *         -1 - Downstream config event failed
 *        -10 - Not enough memory
 **/
int ispif_util_config_src_port(ispif_t *ispif,
  ispif_port_t *ispif_sink_port, ispif_stream_t *stream)
{
  int rc = 0;
  ispif_session_t *session = (ispif_session_t *)stream->session;
  ispif_port_t *ispif_src_port =
    (ispif_port_t *)stream->src_port->port_private;
  ispif_src_port_stream_cfg_t *stream_cfg = NULL;
  mct_port_t *isp_sink_port = ispif_src_port->port->peer;
  mct_event_t *event;

  stream_cfg = malloc(sizeof(ispif_src_port_stream_cfg_t));
  if (!stream_cfg) {
    CDBG_ERROR("%s: no memory for stream_cfg\n", __func__);

    return -10;
  }
  event = malloc(sizeof(mct_event_t));
  if (!event) {
    free (stream_cfg);

    return -10;
  }
  memset(stream_cfg,  0,  sizeof(ispif_src_port_stream_cfg_t));
  stream_cfg->sensor_cfg = ispif_sink_port->u.sink_port.sensor_cfg;

  /* hi 16 bits - VFE1, lo 16 bits VFE0 */
  stream_cfg->vfe_output_mask = stream->used_output_mask;
  stream_cfg->meta_use_output_mask = stream->meta_use_output_mask;

  /* which vfe associated */
  stream_cfg->vfe_mask =  session->vfe_mask;
  stream_cfg->session_id = stream->session_id;
  stream_cfg->stream_id = stream->stream_id;
  isp_sink_port = stream->src_port->peer;
  memset(event,  0,  sizeof(mct_event_t));

  event->type = MCT_EVENT_MODULE_EVENT;
  event->u.module_event.type = MCT_EVENT_MODULE_IFACE_SET_STREAM_CONFIG;
  event->u.module_event.module_event_data = (void *)stream_cfg;
  event->identity = pack_identity(stream->session_id, stream->stream_id);
  event->direction = MCT_EVENT_DOWNSTREAM;

  if (FALSE == isp_sink_port->event_func(isp_sink_port, event)) {
    rc = -1;
    CDBG_ERROR("%s: error in isp sink port event\n", __func__);
  }

  free (stream_cfg);
  free (event);

  return rc;
}

/** ispif_util_find_primary_cid:
 *    @sensor_cfg: Sensor configuration
 *    @sensor_cap: Sensor source port capabilities
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds primary CID for sensor source port by format
 *
 *  Return: Array index of the primary CID
 **/
uint32_t ispif_util_find_primary_cid(sensor_out_info_t *sensor_cfg,
  sensor_src_port_cap_t *sensor_cap)
{
  int i;

  if(SENSOR_CID_CH_MAX < sensor_cap->num_cid_ch) {
    CDBG_ERROR("%s:%d error out of range", __func__, __LINE__);
    return(0);
  }

  for(i = 0; i < sensor_cap->num_cid_ch; i++)
    if (sensor_cfg->fmt == sensor_cap->sensor_cid_ch[i].fmt)
      break;

  if(i == sensor_cap->num_cid_ch) {
    CDBG_ERROR("%s:%d error cannot find primary sensor format", __func__,
      __LINE__);

    return(0);
  }

  return(i);
}

/** ispif_util_dump_sensor_cfg:
 *    @sink_port: Sink port connected to sensor
 *
 *  This function runs in MCTL thread context.
 *
 *  This function dumps sensor configuration by port.
 *
 *  Return: None
 **/
void ispif_util_dump_sensor_cfg(ispif_sink_port_t *sink_port)
{
  sensor_out_info_t *sensor_cfg = &sink_port->sensor_cfg;
  uint32_t primary_cid_idx;
  primary_cid_idx =
    ispif_util_find_primary_cid(sensor_cfg, &sink_port->sensor_cap);

  if (SENSOR_CID_CH_MAX - 1 < primary_cid_idx) {
    CDBG_ERROR("%s:%d error out of range\n", __func__, __LINE__);
    return ;
  }
  CDBG("%s: sensor dim: width = %d, heght = %d, fmt = %d, is_bayer = %d\n",
    __func__, sensor_cfg->dim_output.width, sensor_cfg->dim_output.height,
    sink_port->sensor_cap.sensor_cid_ch[primary_cid_idx].fmt,
    sink_port->sensor_cap.sensor_cid_ch[primary_cid_idx].is_bayer_sensor);

  CDBG("%s: camif_crop: first_pix = %d, last_pix = %d, "
    "first_line = %d, last_line = %d, max_fps = %d\n", __func__,
    sensor_cfg->request_crop.first_pixel, sensor_cfg->request_crop.last_pixel,
    sensor_cfg->request_crop.first_line, sensor_cfg->request_crop.last_line,
    (int)sensor_cfg->max_fps);

  CDBG("%s: meta info: num_meta = %d, format = %d, W %d, H %d\n", __func__,
    sensor_cfg->meta_cfg.num_meta, sensor_cfg->meta_cfg.sensor_meta_info[0].fmt,
    sensor_cfg->meta_cfg.sensor_meta_info[0].dim.width,
    sensor_cfg->meta_cfg.sensor_meta_info[0].dim.height);
}

/** ispif_util_choose_isp_interface:
 *    @ispif: ispif pointer
 *    @ispif_sink_port: ispif sink port
 *    @session: ispif session
 *    @stream: ispif stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function determines ISP output interface for given stream
 *
 *  Return: None
 **/
void ispif_util_choose_isp_interface(ispif_t *ispif,
  ispif_port_t *ispif_sink_port, ispif_session_t *session,
  ispif_stream_t *stream)
{
  uint8_t use_pix = 0;
  choose_isp_interface(&ispif_sink_port->u.sink_port.sensor_cfg,
    &ispif_sink_port->u.sink_port.sensor_cap, &stream->stream_info, &use_pix);
  stream->use_pix = use_pix;
}

/** ispif_util_reserve_isp_resource:
 *    @ispif: ispif instance
 *    @ispif_sink_port: ispif sink port
 *
 *  This function runs in MCTL thread context.
 *
 *  This function reserves an ISP resource
 *
 *  Return: 0 - Success
 *         -1 - Couldn't reserve resource
 **/
int ispif_util_reserve_isp_resource(ispif_t *ispif,
  ispif_port_t *ispif_sink_port)
{
  int i, k;
  ispif_stream_t *stream;
  ispif_sink_port_t *sink_port = &ispif_sink_port->u.sink_port;
  sensor_src_port_cap_t *sensor_cap = &sink_port->sensor_cap;
  sensor_dim_output_t *dim_output = &sink_port->sensor_cfg.dim_output;
  ispif_session_t *session = NULL;
  uint32_t vfe_mask;
  uint32_t primary_cid_idx;
  int fps, rc = 0;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    vfe_mask = 0;
    if (sink_port->streams[i] != NULL) {
      stream = sink_port->streams[i];
      session = (ispif_session_t *)stream->session;
      if (stream->used_output_mask) {

        /* we have assigned the VFE interface. continue */
        continue;
      }

      if (session == NULL) {

        CDBG_ERROR("%s: session is NULL!!!", __func__);
        continue;
      }

      /* finding exact fps for large image 30 fps streaming is important */
      if (fabs(sink_port->sensor_cfg.max_fps - 30.0) <= 1.0){
        fps = 30;
      } else {
        /* take ceiling in other case */
        fps = (int)sink_port->sensor_cfg.max_fps;
        if ((float)fps < sink_port->sensor_cfg.max_fps)
          fps += 1;

      }
      /* fps is not use any more in resource allocation. Instead we
       * use sensor op_clk to determine if dual VFE is needed. */
      primary_cid_idx = ispif_util_find_primary_cid(&sink_port->sensor_cfg,
          &sink_port->sensor_cap);

      rc = reserve_isp_resource(stream->use_pix, TRUE, sensor_cap,
        &stream->stream_info, dim_output, primary_cid_idx,
        sink_port->sensor_cfg.op_pixel_clk, session->session_idx,
        &stream->used_output_mask, &vfe_mask);

      if (rc < 0) {
        CDBG_ERROR("%s: error in reserve isp resource, rc = %d\n",
          __func__, rc);
        break;
      }

      if (stream->use_pix)
        session->camif_cnt++;
      else
        session->rdi_cnt++;

      session->vfe_mask |= vfe_mask;

    }
  }

  return rc;
}

/** ispif_util_release_isp_resource:
 *    @ispif: ispif pointer
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function releases ISP resources associated with a stream
 *
 *  Return: None
 **/
void ispif_util_release_isp_resource(ispif_t *ispif, ispif_stream_t *stream)
{
  int rc = 0;
  ispif_session_t *session = stream->session;

  if ((stream->used_output_mask == 0) && (stream->meta_use_output_mask == 0))
    return;

  if (stream->use_pix) {
    session->camif_cnt--;
    if (session->camif_cnt == 0)
      rc = release_isp_resource(TRUE, session->session_idx,
        stream->used_output_mask, session->vfe_mask);

  } else {
    /* RDI case */
    session->rdi_cnt--;
    rc = release_isp_resource(TRUE, session->session_idx,
      stream->used_output_mask, session->vfe_mask);
  }

  if(rc <0)
    CDBG_ERROR("%s: Error in releasing resource use_pix %d, mask %x\n",
      __func__,stream->use_pix, session->vfe_mask);

  /* till here we released stream's pix or rdi interface.
   * Now, we also need to make sure that we release the
   * associated meta data resource. */
  /*release RDI, assume only one meta*/
  if (stream->num_meta > 0 && (session->num_meta > 0)) {
    /* meta data case */
    session->rdi_cnt--;
    session->num_meta--;
    stream->num_meta--;
    rc = release_isp_resource(TRUE,
      session->session_idx,
      stream->meta_use_output_mask,
      session->vfe_mask);
  }

  stream->used_output_mask = 0;
  if (session->camif_cnt == 0 && session->rdi_cnt == 0)
    session->vfe_mask = 0;

  memset(&stream->split_info, 0, sizeof(stream->split_info));
}

int ispif_util_proc_reset(ispif_t *ispif, mct_event_t *event)
{
  int rc = 0, i, j, k = 0, l = 0;
  ispif_stream_t *stream = NULL;
  uint32_t session_id = UNPACK_SESSION_ID(event->identity);
  ispif_session_t *session =
    ispif_util_get_session_by_id(ispif, session_id);
  ispif_port_t *ispif_sink_port = NULL;
  int isp_id;
  struct ispif_cfg_data cfg_cmd_local;
  struct ispif_cfg_data *cfg_cmd = &cfg_cmd_local;
  uint32_t prim_cid_idx;

  CDBG("%s E \n", __func__);
  memset(cfg_cmd, 0, sizeof(struct ispif_cfg_data));
  cfg_cmd->cfg_type = ISPIF_STOP_IMMEDIATELY;
  if (!session) {
    CDBG_ERROR("%s:get session by id failed\n",__func__);
    return -1;
  }
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    stream = &session->streams[i];
    if (stream->session_id != session_id) {
      continue;
    }

    ispif_sink_port = (ispif_port_t *)stream->sink_port->port_private;
    prim_cid_idx = ispif_util_find_primary_cid(
      &(ispif_sink_port->u.sink_port.sensor_cfg),
      &(ispif_sink_port->u.sink_port.sensor_cap));

    if (stream->num_meta > 0 && stream->meta_info[0].is_valid) {
      isp_id = (stream->meta_use_output_mask & (0xffff << VFE0))? VFE0: VFE1;
      cfg_cmd->params.entries[cfg_cmd->params.num].vfe_intf =
        (enum msm_ispif_vfe_intf)isp_id;

      cfg_cmd->params.entries[cfg_cmd->params.num].intftype =
        isp_interface_mask_to_interface_num(stream->meta_use_output_mask, (1<< isp_id));
      CDBG("%s: %d <DBG01> meta_isp_id %d meta_mask %x intftype %d\n", __func__, __LINE__,
        isp_id, stream->meta_use_output_mask,
        cfg_cmd->params.entries[cfg_cmd->params.num].intftype);
      if (isp_interface_mask_to_interface_num(stream->meta_use_output_mask, (1<< isp_id)) < 0) {
        CDBG_ERROR("%s: meta invalid ispif interface mask = %d",
                   __func__, cfg_cmd->params.entries[cfg_cmd->params.num].intftype);
        continue;
      }
      cfg_cmd->params.entries[cfg_cmd->params.num].num_cids = 1;
      cfg_cmd->params.entries[cfg_cmd->params.num].csid =
      ispif_sink_port->u.sink_port.sensor_cap.meta_ch[k].csid;
      if (ispif_sink_port->u.sink_port.sensor_cap.num_meta_ch > 1) {
        CDBG_ERROR("%s: not support one channel multiple cid yet\n",
          __func__);
        return -1;
      } else {
        cfg_cmd->params.entries[cfg_cmd->params.num].cids[k] =
          ispif_sink_port->u.sink_port.sensor_cap.meta_ch[k].cid;
      }
      CDBG("%s meta cid %d csid %d intftype %d isp_id %d params_num %d\n", __func__,
            ispif_sink_port->u.sink_port.sensor_cap.meta_ch[k].cid,
            ispif_sink_port->u.sink_port.sensor_cap.meta_ch[k].csid,
            cfg_cmd->params.entries[cfg_cmd->params.num].intftype,
            cfg_cmd->params.entries[cfg_cmd->params.num].vfe_intf,
            cfg_cmd->params.num);
      cfg_cmd->params.num++;
    } /* Meta end */
    cfg_cmd->params.entries[cfg_cmd->params.num].num_cids =
      ispif_sink_port->u.sink_port.sensor_cap.num_cid_ch;
    for (l = 0; l < ispif_sink_port->u.sink_port.sensor_cap.num_cid_ch; l++) {
      cfg_cmd->params.entries[cfg_cmd->params.num].cids[l] =
        ispif_sink_port->u.sink_port.sensor_cap.sensor_cid_ch[l].cid;
    }
    if (SENSOR_CID_CH_MAX - 1 < prim_cid_idx) {
      CDBG_ERROR("%s:%d error out of range\n", __func__, __LINE__);
      return -1;
    }
    cfg_cmd->params.entries[cfg_cmd->params.num].csid =
      ispif_sink_port->u.sink_port.sensor_cap.sensor_cid_ch[prim_cid_idx].csid;
    for (isp_id = VFE0; isp_id < VFE_MAX; isp_id++) {
      if ((session->vfe_mask & (1 << isp_id)) == 0) {
        continue;
      }
      cfg_cmd->params.entries[cfg_cmd->params.num].intftype =
        ispif_util_find_isp_intf_type(stream,
                                      stream->used_output_mask, (1 << isp_id));
      if (cfg_cmd->params.entries[cfg_cmd->params.num].intftype == INTF_MAX) {
        CDBG_ERROR("%s: invalid ispif interface mask = %d used_output_mask 0x%x\n",
                   __func__, cfg_cmd->params.entries[cfg_cmd->params.num].intftype,
                   stream->used_output_mask);
        continue;
      }
      cfg_cmd->params.entries[cfg_cmd->params.num].vfe_intf =
        (enum msm_ispif_vfe_intf)isp_id;
      memcpy(&(cfg_cmd->params.entries[cfg_cmd->params.num + 1]),
             &(cfg_cmd->params.entries[cfg_cmd->params.num]),
             sizeof(cfg_cmd->params.entries[cfg_cmd->params.num]));
      cfg_cmd->params.num++;
      CDBG("%s cid %d csid %d intftype %d isp_id %d params_num %d\n", __func__,
            ispif_sink_port->u.sink_port.sensor_cap.meta_ch[l].cid,
            ispif_sink_port->u.sink_port.sensor_cap.meta_ch[l].csid,
            cfg_cmd->params.entries[cfg_cmd->params.num].intftype,
            cfg_cmd->params.entries[cfg_cmd->params.num].vfe_intf,
            cfg_cmd->params.num);
    }
  }
  rc = ioctl(ispif->fd, VIDIOC_MSM_ISPIF_CFG, cfg_cmd);
  if (rc != 0) {
    if (errno == ETIMEDOUT) {
      CDBG_ERROR("%s, error - ISPIF stop on frame boundary timed out!",
        __func__);
    }
  }
  /* Send MCT event to start CAMIF and AXI before starting ISPIF */
  stream = ispif_util_find_stream(ispif, session_id,
                                  UNPACK_STREAM_ID(event->identity));
  if (!stream) {
    CDBG_ERROR("%s:ispif_util_find_stream failed\n",__func__);
    return -1;
  }

  mct_port_t *ispif_src_port = stream->src_port;

  mct_event_t isp_event;
  memset(&isp_event, 0, sizeof(isp_event));
  isp_event.identity = event->identity;
  isp_event.direction = MCT_EVENT_DOWNSTREAM;
  isp_event.type = MCT_EVENT_MODULE_EVENT;
  isp_event.u.module_event.type = MCT_EVENT_MODULE_ISP_RESTART;
  isp_event.u.module_event.module_event_data = &session->vfe_mask;
  rc = mct_port_send_event_to_peer(ispif_src_port, &isp_event);
  if (rc < 0) {
    CDBG_ERROR("%s Error Restarting ISP CAMIF and AXI \n", __func__);
    return -1;
  }
  /* Stop finished. Now restart */
  if (cfg_cmd->params.num > 0) {
    cfg_cmd->cfg_type = ISPIF_RESTART_FRAME_BOUNDARY;
    rc = ioctl(ispif->fd, VIDIOC_MSM_ISPIF_CFG, cfg_cmd);
    if (rc != 0) {
      if (errno == ETIMEDOUT) {
        CDBG_ERROR("%s, error - ISPIF start on frame boundary timed out!",
          __func__);
        return rc;
      }
    }
  }
  return rc;
}

/** ispif_util_find_isp_intf_type:
 *    @stream: stream
 *
 *  This function runs in MCTL thread context.
 *
 *  This function determines ISP interface type associates with a stream
 *  by output mask
 *
 *  Return: msm_ispif_intftype enumeration of interface found
 *          INTF_MAX - invalid mask
 **/
enum msm_ispif_intftype ispif_util_find_isp_intf_type(ispif_stream_t *stream,
  uint32_t used_output_mask, uint32_t vfe_mask)
{
  int intf_idx;
  ispif_session_t *session = stream->session;
  intf_idx = isp_interface_mask_to_interface_num(used_output_mask,
               vfe_mask);
  if (intf_idx == ISP_INTF_PIX)
    return PIX0;
  else if (intf_idx == ISP_INTF_RDI0)
    return RDI0;
  else if (intf_idx == ISP_INTF_RDI1)
    return RDI1;
  else if (intf_idx == ISP_INTF_RDI2)
    return RDI2;
  else
    return INTF_MAX;
}

/** ispif_util_set_bundle:
 *    @ispif: ispif instance
 *    @ispif_sink_port: ispif sink port
 *    @session_id: session id
 *    @stream_id: stream id
 *    @bundle_param: HAL bundle params
 *
 *  This function runs in MCTL thread context.
 *
 *  This function sets ispif bundling mask according to bundle configuration
 *  by output mask
 *
 *  Return: 0 - Success
 *         -1 - CAnnot find session or stream
 **/
int ispif_util_set_bundle(ispif_t *ispif, ispif_port_t *ispif_sink_port,
  uint32_t session_id, uint32_t stream_id, cam_bundle_config_t *bundle_param)
{
  int i, rc = 0;
  ispif_session_t *session = ispif_util_get_session_by_id(ispif, session_id);
  ispif_stream_t *stream;
  if (!session) {
    CDBG_ERROR("%s: cannot find session (%d)\n", __func__, session_id);

    return -1;
  }
  for (i = 0; i < bundle_param->num_of_streams; i++) {
    stream = ispif_util_find_stream_in_session(session,
      bundle_param->stream_ids[i]);
    if (!stream) {
      CDBG_ERROR("%s: stream (sessid = %d, streamid= %d)m not found\n",
        __func__, session_id, bundle_param->stream_ids[i]);

      goto error;
    }

    session->hal_bundling_mask |= (1 << stream->stream_idx);
    CDBG("%s: stream_id = %d, stream_idx = %d, hal_bundling_mask= 0x%x",
      __func__, stream->stream_id,
      stream->stream_idx, session->hal_bundling_mask);
  }

  return rc;

error:
  session->hal_bundling_mask = 0;

  return -1;
}

/** ispif_util_get_stream_ids_by_mask:
 *    @session: session
 *    @num_streams: number of active stream
 *    @stream_ids: array with stream IDs
 *
 *  This function runs in MCTL thread context.
 *
 *  This function finds stream by mask
 *
 *  Return: 0 - Success
 **/
int ispif_util_get_stream_ids_by_mask(ispif_session_t *session,
  uint32_t stream_idx_mask, int *num_streams, uint32_t *stream_ids)
{
  int i, rc = 0;

  *num_streams = 0;

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (stream_idx_mask & (1 << i)) {
      stream_ids[*num_streams] = session->streams[i].stream_id;
      *num_streams += 1;
      CDBG("%s: stream_id = %d, bit_pos = %d, mask = 0x%x", __func__,
        session->streams[i].stream_id, i, stream_idx_mask);
    }
  }

  return rc;
}

/** ispif_util_dump_sensor_src_cap:
 *    @sensor_cap: Sensor source port capabilities
 *
 *  This function runs in MCTL thread context.
 *
 *  This function dumps sensor source port capabilities
 *
 *  Return: None
 **/
void ispif_util_dump_sensor_src_cap(sensor_src_port_cap_t *sensor_cap)
{
  int i;

  for (i = 0; i < sensor_cap->num_cid_ch; i++) {
    CDBG("%s: addr = %p, sessid = %d, num_cid = %d, cididx = %d, "
      "cid = %d, csid = %d, csid_version = 0x%x", __func__, sensor_cap,
      sensor_cap->session_id, sensor_cap->num_cid_ch, i,
      sensor_cap->sensor_cid_ch[i].cid, sensor_cap->sensor_cid_ch[i].csid,
      sensor_cap->sensor_cid_ch[i].csid_version);
  }
}

/** ispif_util_has_pix_resource:
 *    @sink_port: ispif sink port
 *    @stream_info: stream info data
 *
 *  This function runs in MCTL thread context.
 *
 *  This function checks if PIX interface is needed and available
 *
 *  Return: TRUE  - There is PIX interface available or it is not needed for
 *                  this stream
 *          FALSE - There is no available PIX interface
 **/
boolean ispif_util_has_pix_resource(ispif_sink_port_t *sink_port,
  mct_stream_info_t *stream_info)
{
  sensor_src_port_cap_t *sensor_port_cap = &sink_port->sensor_cap;
  uint32_t primary_cid_idx = ispif_util_find_primary_cid(&sink_port->sensor_cfg,
    sensor_port_cap);
  if (SENSOR_CID_CH_MAX - 1 < primary_cid_idx) {
    CDBG_ERROR("%s:%d error out of range\n", __func__, __LINE__);
    return -1;
  }
  if (sensor_port_cap->sensor_cid_ch[primary_cid_idx].is_bayer_sensor &&
      sensor_port_cap->sensor_cid_ch[primary_cid_idx].fmt != stream_info->fmt) {
    switch (stream_info->fmt) {
    case CAM_FORMAT_YUV_420_NV12_VENUS:
    case CAM_FORMAT_YUV_420_NV12:
    case CAM_FORMAT_YUV_420_NV21:
    case CAM_FORMAT_YUV_420_NV21_ADRENO:
    case CAM_FORMAT_YUV_420_YV12:
    case CAM_FORMAT_YUV_422_NV16:
    case CAM_FORMAT_YUV_422_NV61:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_BGGR:
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
      return has_isp_pix_interface();
    default:
      return TRUE;
    }
  } else
    return TRUE;
}

/** ispif_util_pip_switching_cap_op_pixclk:
 *    @ispif: ispif instance
 *    @ispif_sink_port: ispif sink port
 *    @sensor_cfg: sensor configuration
 *
 *  This function runs in MCTL thread context.
 *
 *  This function switches the pixel clock when in PIP
 *
 *  Return: 0 - Success
 *         -1 - Stream not found
 **/
int ispif_util_pip_switching_cap_op_pixclk(ispif_t *ispif,
  ispif_port_t *ispif_sink_port, sensor_out_info_t *sensor_cfg)
{
  int session_idx = 0;
  mct_event_t mct_event;
  uint32_t op_pix_clk = ISP_MAX_OP_PIX_CLK;
  int i;
  ispif_sink_port_t *sink_port = &ispif_sink_port->u.sink_port;
  ispif_stream_t *stream = NULL;

  stream = NULL;
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (ispif_sink_port->u.sink_port.streams[i]) {
      stream = ispif_sink_port->u.sink_port.streams[i];
      break;
    }
  }
  if (stream == NULL) {
    CDBG_ERROR("%s: cannot find stream\n", __func__);
    return -1;
  }

  sink_port = stream->sink_port->port_private;

  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.identity = stream->stream_info.identity;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.direction = MCT_EVENT_UPSTREAM;
  mct_event.u.module_event.type = MCT_EVENT_MODULE_ISP_CHANGE_OP_PIX_CLK;
  mct_event.u.module_event.module_event_data = (void *)&op_pix_clk;
  mct_port_send_event_to_peer(stream->sink_port->peer, &mct_event);

  return 0;
}

/** ispif_util_dual_vfe_to_pip_switching:
 *    @ispif: ispif instance
 *    @ispif_sink_port: ispif sink port
 *    @sensor_cfg: sensor configuration
 *
 *  This function runs in MCTL thread context.
 *
 *  This function switches from dual ISP mode to PIP
 *
 *  Return: None
 **/
void ispif_util_dual_vfe_to_pip_switching(ispif_t *ispif,
  sensor_src_port_cap_t *sensor_port_cap, ispif_stream_t *starving_stream)
{
  int i;
  int session_idx = 0;
  ispif_session_t *session = NULL;
  ispif_stream_t *stream = NULL;
  mct_bus_msg_t bus_msg;
  mct_bus_msg_error_message_t err_msg = MCT_ERROR_MSG_RSTART_VFE_STREAMING;
  mct_event_t mct_event;
  uint32_t op_pix_clk = ISP_MAX_OP_PIX_CLK;
  ispif_sink_port_t *sink_port = NULL;
  ispif_session_t *starving_session = NULL;

  if (get_dual_vfe_session_id(&session_idx) == FALSE) {
    CDBG("%s: no needed\n", __func__);
    return;
  }

  session = &ispif->sessions[session_idx];
  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    if (session->streams[i].stream_id > 0) {
      stream = &session->streams[i];
      break;
    }
  }

  if (!stream || !stream->sink_port) {
    CDBG_ERROR("%s: error, no stream in session %d\n",
      __func__, session->session_id);

    return;
  }

  if (!stream->sink_port) {
    CDBG_ERROR("%s: error, no sink port for stream identity = 0x%x\n",
      __func__, stream->stream_info.identity);

    return;
  }

  sink_port = stream->sink_port->port_private;

  memset(&mct_event, 0, sizeof(mct_event));
  mct_event.identity = stream->stream_info.identity;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.direction = MCT_EVENT_UPSTREAM;
  mct_event.u.module_event.type = MCT_EVENT_MODULE_ISP_CHANGE_OP_PIX_CLK;
  mct_event.u.module_event.module_event_data = (void *)&op_pix_clk;
  mct_port_send_event_to_peer(stream->sink_port->peer, &mct_event);
  session->dual_single_isp_switching = TRUE;
  session->high_op_pix_clk = sink_port->sensor_cfg.op_pixel_clk;

  /* send event to starving streaming notifying ISP no resource */
  starving_session = starving_stream->session;
  /* mark a flag so that we will notify it to resume */
  starving_session->need_resume = TRUE;
  starving_session->trigger_dual_isp_restore = TRUE;
  mct_event.identity = starving_stream->stream_info.identity;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.direction = MCT_EVENT_UPSTREAM;
  mct_event.u.module_event.type = MCT_EVENT_MODULE_ISP_NO_RESOURCE;
  mct_event.u.module_event.module_event_data = NULL;
  mct_port_send_event_to_peer(stream->sink_port->peer, &mct_event);

  /* send bus msg to  dual isp */
  memset(&bus_msg, 0, sizeof(bus_msg));
  bus_msg.sessionid = session->session_id;
  bus_msg.type = MCT_BUS_MSG_ERROR_MESSAGE;
  bus_msg.size = sizeof(err_msg);
  bus_msg.msg = (void *)&err_msg;
  if (TRUE != mct_module_post_bus_msg(ispif->module, &bus_msg))
    CDBG_ERROR("%s: MCT_BUS_MSG_ERROR_MESSAGE to bus error\n", __func__);

}
