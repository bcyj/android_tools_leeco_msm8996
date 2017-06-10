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

#include "camera_dbg.h"
#include "cam_intf.h"
#include "mct_controller.h"
#include "modules.h"
#include "isp_def.h"
#include "isp_event.h"
#include "isp_ops.h"
/*#include "isp_pix_common.h"*/
#include "isp_hw.h"
#include "isp.h"
#include "isp_util.h"
#include "isp_log.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#if PORT_ISP_DEBUG
#undef CDBG
#define CDBG ALOGE
#endif
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE
#undef CDBG_HIGH
#define CDBG_HIGH ALOGE

/** port_isp_send_event_to_peer
 *   @data1: mct port instance
 *   @user_data: mct event
 *
 *  This method sends event to peer modules
 *
 *  Return: TRUE always!
 **/
static boolean port_isp_send_event_to_peer(void *data1, void *user_data)
{
  mct_port_t *mct_port = (mct_port_t *)data1;
  mct_event_t *event = (mct_event_t *)user_data;
  isp_port_t *isp_port = (isp_port_t * )mct_port->port_private;
  isp_stream_t *stream = NULL;
  isp_stream_t **streams = NULL;
  int i;
  uint32_t identity;
  boolean rc = FALSE;

  ISP_DBG(ISP_MOD_COM,"%s: E, event type %d direction %d\n", __func__,
       event->type, event->direction);
  if (isp_port->state == ISP_PORT_STATE_CREATED) {
    /* not used port */
    ISP_DBG(ISP_MOD_COM,"%s: X\n", __func__);
    return TRUE;
  }
  if (mct_port->direction == MCT_PORT_SINK) {
    isp_sink_port_t *sink_port = &isp_port->u.sink_port;
    streams = sink_port->streams;
  } else if (mct_port->direction == MCT_PORT_SRC) {
    isp_src_port_t *src_port = &isp_port->u.src_port;
    streams = src_port->streams;
  } else {
    ISP_DBG(ISP_MOD_COM,"%s: X\n", __func__);
    return TRUE;
  }

  for (i = 0; i < ISP_MAX_STREAMS; i++) {
    stream = streams[i];
    if (stream == NULL)
      continue;
    identity = pack_identity(stream->session_id, stream->stream_id);
    if (identity != (uint32_t)event->identity)
      continue;
    rc = mct_port->peer->event_func(mct_port->peer, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: direction= %d event = %d rc = FALSE\n", __func__,
        mct_port->direction, event->type);
      return rc;
    }
    break;
  }
  ISP_DBG(ISP_MOD_COM,"%s: X\n", __func__);
  return TRUE;
}

/** port_isp_forward_event_to_peer
 *   @isp: isp instance
 *   @mct_port: mct port instance
 *   @event: mct event
 *
 *  This method forwards event to source or sink module
 *
 *  Returns TRUE for success and FALSE on failure
 **/
static boolean port_isp_forward_event_to_peer(isp_t *isp, mct_port_t *mct_port,
  mct_event_t *event)
{
  ISP_DBG(ISP_MOD_COM,"%s: E, direction %d\n", __func__, mct_port->direction);
  /* if receive from sink forward to src's peer */
  if (mct_port->direction == MCT_PORT_SINK)
    return mct_list_traverse(isp->module->srcports,
                           port_isp_send_event_to_peer,
                           (void *)event);
  else if (mct_port->direction == MCT_PORT_SRC)
    return mct_list_traverse(isp->module->sinkports,
                           port_isp_send_event_to_peer,
                           (void *)event);
  else {
    CDBG_ERROR("%s: Error invalid port direction\n", __func__);
    return FALSE;
  }
}

/** port_isp_send_streamon_done_event_downstream
 *   @isp: isp instance
 *   @mct_port: mct port instance
 *   @event: mct event
 *
 *  This method sends event streamon done downstream
 *
 *  Returns 0 for success and negative error on failure
 **/
int port_isp_send_streamon_done_event_downstream(isp_t *isp, mct_port_t *port,
  mct_event_t *event)
{
  boolean rc = TRUE;
  mct_event_t streamon_done;
  ISP_DBG(ISP_MOD_COM,"%s: E\n", __func__);
  isp_session_t *session =
    isp_util_find_session(isp, UNPACK_SESSION_ID(event->identity));

  if (!session) {
    CDBG_ERROR("%s: cannot find session %d\n",
      __func__, UNPACK_SESSION_ID(event->identity));
    return 0;
  }
  if (session->hal_bundling_mask != session->streamon_bundling_mask) {
    /* ZSL use case, first streamon */
    CDBG_HIGH("%s: ignore this streamon, "
      "hal_bundling_mask = 0x%x, streamon_mask = 0x%x\n",
      __func__, session->hal_bundling_mask,
      session->streamon_bundling_mask);
    return 0;
  }
  CDBG_HIGH("%s: notify stream done downstream\n", __func__);

  memset(&streamon_done,  0,  sizeof(streamon_done));

  streamon_done.type = MCT_EVENT_MODULE_EVENT;
  streamon_done.identity = event->identity;
  streamon_done.direction = MCT_EVENT_DOWNSTREAM;
  streamon_done.timestamp = event->timestamp;
  streamon_done.u.module_event.type = MCT_EVENT_MODULE_ISP_STREAMON_DONE;
  rc = port_isp_forward_event_to_peer(isp, port, &streamon_done);
  ISP_DBG(ISP_MOD_COM,"%s: X, rc = %d\n", __func__, rc);

  return (rc == TRUE) ? 0 : -1;
}

/** port_isp_redirect_stats_event_to_frame_port
 *   @isp: isp instance
 *   @mct_port: mct port instance
 *   @event: mct event
 *
 *  Sends stats event downstream on source ports
 *
 *  Returns TRUE for success and FALSE on failure
 **/
static  boolean port_isp_redirect_stats_event_to_frame_port(isp_t *isp,
  mct_port_t *mct_port, mct_event_t *event)
{
  ISP_DBG(ISP_MOD_COM,"%s: E\n", __func__);
  isp_stream_t *stream = isp_util_find_stream(isp,
    UNPACK_SESSION_ID(event->identity),
    UNPACK_STREAM_ID(event->identity));

  if (!stream) {
    CDBG_ERROR("%s: cannot find stream (identiity = 0x%x)\n",
      __func__, event->identity);
    return FALSE;
  }

  if (!stream->src_ports[ISP_SRC_PORT_DATA]) {
    CDBG_ERROR("%s: stream (identiity = 0x%x) has no frame port linked\n",
      __func__, event->identity);
    return FALSE;
  }
  event->direction = MCT_EVENT_DOWNSTREAM;
  return mct_port_send_event_to_peer(
     stream->src_ports[ISP_SRC_PORT_DATA], event);
}

/** port_isp_redirect_frame_port_event_to_stats_port
 *   @isp: isp instance
 *   @mct_port: mct port instance
 *   @event: mct event
 *
 *  Sends stats event downstream on 3A ports
 *
 *  Returns TRUE for success and FALSE on failure
 **/
static  boolean port_isp_redirect_frame_port_event_to_stats_port(isp_t *isp,
  mct_port_t *mct_port, mct_event_t *event)
{
  ISP_DBG(ISP_MOD_COM,"%s: E\n", __func__);
  isp_stream_t *stream = isp_util_find_stream(isp,
    UNPACK_SESSION_ID(event->identity),
    UNPACK_STREAM_ID(event->identity));

  if (!stream) {
    CDBG_ERROR("%s: cannot find stream (identiity = 0x%x)\n",
      __func__, event->identity);
    return FALSE;
  }

  if (!stream->src_ports[ISP_SRC_PORT_3A]) {
    CDBG_ERROR("%s: stream (identiity = 0x%x) has no stats port linked\n",
      __func__, event->identity);
    return FALSE;
  }
  event->direction = MCT_EVENT_DOWNSTREAM;
  return mct_port_send_event_to_peer(
     stream->src_ports[ISP_SRC_PORT_3A], event);
}

/** port_isp_mct_ctrl_cmd
 *   @port: mct port instance
 *   @event: mct event
 *
 *  Handles isp control cmds such as streamon/off, set hal
 *  param,set stream param etc. and forwards to peers
 *
 *  Returns TRUE for success and FALSE on failure
 **/
static boolean port_isp_mct_ctrl_cmd(mct_port_t *port, mct_event_t *event)
{
  int ret = 0;
  mct_event_control_t *ctrl = &event->u.ctrl_event;
  isp_port_t *tmp_port = (isp_port_t *)port->port_private;
  isp_t *isp = (isp_t *)tmp_port->isp;

  ISP_DBG(ISP_MOD_COM,"%s: E, type=%d", __func__, ctrl->type);
  switch (ctrl->type) {
  case MCT_EVENT_CONTROL_STREAMON_FOR_FLASH: {
    ISP_DBG(ISP_MOD_COM,"%s: E, identity = 0x%x, STREAMON\n",
      __func__, event->identity);
    if (FALSE == port_isp_forward_event_to_peer(
             isp, port, event)){
      CDBG_ERROR("%s: STREAMON, forward_event_to_peer error\n",
        __func__);
      ret = -1;
    } else {
      ret = isp_streamon(isp, tmp_port,
              UNPACK_SESSION_ID(event->identity),
            UNPACK_STREAM_ID(event->identity), event, TRUE);
      if (ret == 0) {
        (void)port_isp_send_streamon_done_event_downstream(isp, port, event);
      } else {
        CDBG_ERROR("%s: error in isp_streamon, identity = 0x%x\n",
          __func__, event->identity);
      }
    }
    CDBG_HIGH("%s: X, identity = 0x%x, STREAMON, ret = %d\n",
      __func__, event->identity, ret);
  }
    break;
  case MCT_EVENT_CONTROL_STREAMON: {
    CDBG_HIGH("%s: E, identity = 0x%x, STREAMON\n",
      __func__, event->identity);
    if (FALSE == port_isp_forward_event_to_peer(
             isp, port, event)){
      CDBG_ERROR("%s: STREAMON, forward_event_to_peer error\n",
        __func__);
      ret = -1;
    } else {
      ret = isp_streamon(isp, tmp_port,
              UNPACK_SESSION_ID(event->identity),
            UNPACK_STREAM_ID(event->identity), event, FALSE);
      if (ret == 0) {
        (void)port_isp_send_streamon_done_event_downstream(isp, port, event);
      } else {
        CDBG_ERROR("%s: error in isp_streamon, identity = 0x%x\n",
          __func__, event->identity);
      }
    }
    CDBG_HIGH("%s: X, identity = 0x%x, STREAMON, ret = %d\n",
      __func__, event->identity, ret);
  }
    break;

  case MCT_EVENT_CONTROL_STREAMOFF_FOR_FLASH:
  case MCT_EVENT_CONTROL_STREAMOFF: {
    CDBG_HIGH("%s: E, identity = 0x%x, STREAMOFF\n",
      __func__, event->identity);
    if (FALSE == port_isp_forward_event_to_peer(
             isp, port, event)) {
      CDBG_ERROR("%s: STREAMOFF, forward_event_to_peer error\n",
        __func__);
      ret = -1;
    } else {
      ret = isp_streamoff(isp, tmp_port,
            UNPACK_SESSION_ID(event->identity),
            UNPACK_STREAM_ID(event->identity), event);
    }
    CDBG_HIGH("%s: X, identity = 0x%x, STREAMOFF, ret = %d\n",
      __func__, event->identity, ret);
  }
    break;

  case MCT_EVENT_CONTROL_SET_PARM: {
    ret = isp_set_hal_param(isp, tmp_port,
      UNPACK_SESSION_ID(event->identity),
      UNPACK_STREAM_ID(event->identity), event);
    if (ret < 0) {
      CDBG_ERROR("%s: error in isp_set_param, ret = %d\n",
        __func__, ret);
    } else {
      if (FALSE == port_isp_forward_event_to_peer(
        isp, port, event)) {
          CDBG_ERROR("%s: set_param, forward_event_to_peer error\n",
            __func__);
          ret = -1;
      }
    }
  }
    break;

  case MCT_EVENT_CONTROL_PARM_STREAM_BUF: {
    ret = isp_set_hal_stream_param(isp, tmp_port,
            UNPACK_SESSION_ID(event->identity),
            UNPACK_STREAM_ID(event->identity), event);
    if (ret < 0) {
      CDBG_ERROR("%s: error in isp_set_param, ret = %d\n",
        __func__, ret);
    } else {
      if (FALSE == port_isp_forward_event_to_peer(
             isp, port, event)) {
        CDBG_ERROR("%s: set_param, forward_event_to_peer error\n",
          __func__);
        ret = -1;
      }
    }
  }
    break;

  case MCT_EVENT_CONTROL_UPDATE_BUF_INFO: {
    CDBG_HIGH("%s: E, identity = 0x%x, UPDATE_BUF_INFO\n", __func__,
      event->identity);
    if (FALSE == port_isp_forward_event_to_peer(isp, port, event)){
      CDBG_ERROR("%s: UPDATE_BUF_INFO, forward_event_to_peer error\n",
        __func__);
      ret = -1;
    } else {
      ret = isp_update_buf_info(isp, UNPACK_SESSION_ID(event->identity),
        UNPACK_STREAM_ID(event->identity), event);
      if (ret != 0) {
        CDBG_ERROR("%s: error in isp_update_buf_info, identity = 0x%x\n",
          __func__, event->identity);
      }
    }
    CDBG_HIGH("%s: X, identity = 0x%x, UPDATE_BUF_INFO, ret = %d\n",
      __func__, event->identity, ret);

  }
    break;

  default: {
    if (FALSE == port_isp_forward_event_to_peer(
             isp, port, event)) {
      CDBG_ERROR("%s: forward_event_to_peer error\n", __func__);
      ret = -1;
    }
  }
    break;
  }
  ISP_DBG(ISP_MOD_COM,"%s: X, ret %d\n", __func__, ret);
  return ((ret == 0) ? TRUE : FALSE);
}

/** port_isp_module_event
 *   @port: mct port instance
 *   @event: mct event
 *
 *  Handles isp events such as set config, set stream param, 3A
 *  updates, set chromatix etc. and forwards to peers if
 *  applicable
 *
 *  Returns TRUE for success and FALSE on failure
 **/
static boolean port_isp_module_event(mct_port_t *port,
  mct_event_t *event)
{
  int ret = 0;
  boolean rc = FALSE;
  mct_event_module_t *mod_event = &event->u.module_event;
  isp_port_t *tmp_port = (isp_port_t *)port->port_private;
  isp_t *isp = (isp_t *)tmp_port->isp;

  ISP_DBG(ISP_MOD_COM,"%s: E\n", __func__);
  switch (mod_event->type) {
  case MCT_EVENT_MODULE_ISP_RESTART:
    ret = isp_util_proc_restart(isp, UNPACK_SESSION_ID(event->identity),
                                (uint32_t *)mod_event->module_event_data);
    break;
  case MCT_EVENT_MODULE_IFACE_SET_STREAM_CONFIG: {
    ISP_DBG(ISP_MOD_COM,"%s: E, identity = 0x%x, IFACE_SET_STREAM_CONFIG\n",
      __func__, event->identity);
    ret = isp_sink_port_stream_config(isp, tmp_port,
            (ispif_src_port_stream_cfg_t *)mod_event->module_event_data);
  }
    break;

  case MCT_EVENT_MODULE_ISP_META_CONFIG: {
    ISP_DBG(ISP_MOD_COM,"%s: received meta data config, identity = 0x%x\n",
      __func__, event->identity);
    ret = isp_meta_channel_config(isp,
      UNPACK_STREAM_ID(event->identity),
      UNPACK_SESSION_ID(event->identity),
      &((sensor_meta_data_t *)
        mod_event->module_event_data)->sensor_meta_info[0]);
    if (ret < 0) {
      CDBG_ERROR("%s: isp meta channel config err %d", __func__, ret);
      ret = -1;
      break;
    }
    /*ret = isp_video_hdr_config(isp,
      UNPACK_STREAM_ID(event->identity),
      UNPACK_SESSION_ID(event->identity),
      (sensor_meta_t *)mod_event->module_event_data);*/
    rc = port_isp_forward_event_to_peer(isp, port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: event type = %d, forward_event error\n",
        __func__, mod_event->type);
      ret = -1;
    }
  }
    break;

  case MCT_EVENT_MODULE_SET_CHROMATIX_PTR: {
    rc = port_isp_forward_event_to_peer(
             isp, port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: event_type = %d, forward_event error\n",
        __func__, mod_event->type);
      ret = -1;
    } else {
    ret = isp_set_chromatix(isp, tmp_port,
            UNPACK_SESSION_ID(event->identity),
            UNPACK_STREAM_ID(event->identity), mod_event->module_event_data);
    }
  }
    break;

  case MCT_EVENT_MODULE_SET_RELOAD_CHROMATIX: {
    rc = port_isp_forward_event_to_peer(
             isp, port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: event type = %d, forward_event error\n",
        __func__, mod_event->type);
      ret = -1;
    } else {
      ret = isp_set_reload_chromatix(isp, tmp_port,
              UNPACK_SESSION_ID(event->identity),
              UNPACK_STREAM_ID(event->identity), mod_event->module_event_data);
    }
  }
    break;

  case MCT_EVENT_MODULE_SET_AF_ROLLOFF_PARAMS: {
    ISP_DBG(ISP_MOD_COM,"%s: received Rolloff Tables Infinity\n", __func__);
    ret = isp_set_af_rolloff_params(isp, tmp_port,
            UNPACK_SESSION_ID(event->identity),
            UNPACK_STREAM_ID(event->identity), mod_event->module_event_data);
  }
    break;

  case MCT_EVENT_MODULE_SENSOR_LENS_POSITION_UPDATE: {
    ISP_DBG(ISP_MOD_COM,"%s: recevied AF Update from Sensor\n", __func__);
    ret = isp_set_sensor_lens_position_trigger_update(isp, tmp_port,
            UNPACK_SESSION_ID(event->identity),
            UNPACK_STREAM_ID(event->identity), mod_event->module_event_data);
  }
    break;

  case MCT_EVENT_MODULE_SET_FLASH_MODE:
    ISP_DBG(ISP_MOD_COM,"%s: received flash trigger\n", __func__);
    ret = isp_set_flash_mode(isp, tmp_port,
            UNPACK_SESSION_ID(event->identity),
            UNPACK_STREAM_ID(event->identity), mod_event->module_event_data);
    break;

  case MCT_EVENT_MODULE_STATS_AWB_UPDATE: {
    ISP_DBG(ISP_MOD_COM,"%s: received AWB update event, identity = 0x%x",
      __func__, event->identity);
    ret = isp_set_awb_trigger_update(isp, tmp_port,
      UNPACK_SESSION_ID(event->identity), UNPACK_STREAM_ID(event->identity),
      mod_event->module_event_data);
    rc = port_isp_forward_event_to_peer(
           isp, port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: event type = %d, forward_event error\n",
        __func__, mod_event->type);
      rc = 0;
    }
    rc = port_isp_redirect_stats_event_to_frame_port(isp,
      port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: event type = %d, forward_event error\n",
        __func__, mod_event->type);
      ret = -1;
    }
    rc = isp_send_rolloff_to_sensor(isp, UNPACK_SESSION_ID(event->identity),
      UNPACK_STREAM_ID(event->identity), port);
  }
    break;

  case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {
    ret = isp_set_uv_subsample(isp, UNPACK_SESSION_ID(event->identity),
      (stats_update_t *)mod_event->module_event_data);
    if (ret < 0)
      CDBG_ERROR("%s: isp_set_uv_subsample error: %d\n", __func__, ret);
    rc = port_isp_forward_event_to_peer(
             isp, port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: event type = %d, forward_event error\n",
        __func__, mod_event->type);
      ret = -1;
    }
    ret = isp_save_aec_param(isp, tmp_port,
      UNPACK_SESSION_ID(event->identity), UNPACK_STREAM_ID(event->identity),
      mod_event->module_event_data);
    if (ret < 0)
      CDBG_ERROR("%s: isp_save_aec_param error: %d\n", __func__, ret);
    rc = port_isp_redirect_stats_event_to_frame_port(isp,
      port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: event type = %d, forward_event error\n",
        __func__, mod_event->type);
      ret = -1;
    }
  }
    break;

  case MCT_EVENT_MODULE_STATS_ASD_UPDATE: {
    ret = isp_save_asd_param(isp, tmp_port,
      UNPACK_SESSION_ID(event->identity), UNPACK_STREAM_ID(event->identity),
      mod_event->module_event_data);
    rc = port_isp_forward_event_to_peer(
             isp, port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: event type = %d, forward_event error\n",
        __func__, mod_event->type);
      ret = -1;
    }
  }
    break;

  case MCT_EVENT_MODULE_STATS_UPDATE: {
     /* TODO: 3A team is working on new module event type,
     * after that ISP needs to change the case enum */
    rc = port_isp_forward_event_to_peer(
             isp, port, event);
    ISP_DBG(ISP_MOD_COM,"%s: received STATS_UPDATE, identity = 0x%x",
       __func__, event->identity);
    if (rc == FALSE) {
      CDBG_ERROR("%s: event type = %d, forward_event error\n",
        __func__, mod_event->type);
      ret = -1;
    }
  }
    break;

  case MCT_EVENT_MODULE_IMGLIB_AF_CONFIG:
  case MCT_EVENT_MODULE_STATS_DIS_UPDATE: {
    /* per stats module request, for this event,
     * ISP needs to change the direction and
     * redirect it to downstream */
    rc = port_isp_redirect_stats_event_to_frame_port(isp,
      port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: event type = %d, forward_event error\n",
        __func__, mod_event->type);
      ret = -1;
    }
  }
    break;
  case MCT_EVENT_MODULE_GET_AF_SW_STATS_FILTER_TYPE:
  case MCT_EVENT_MODULE_GET_AEC_LUX_INDEX:
  case MCT_EVENT_MODULE_IMGLIB_AF_OUTPUT:
  case MCT_EVENT_MODULE_FACE_INFO:
  case MCT_EVENT_MODULE_PPROC_GET_AWB_UPDATE:
  case MCT_EVENT_MODULE_PPROC_GET_AEC_UPDATE: {
    /* per image lib module request, for this event,
     * ISP needs to redirection to stats module */
    rc = port_isp_redirect_frame_port_event_to_stats_port(isp,
      port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: event type = %d, forward_event error\n",
        __func__, mod_event->type);
      ret = -1;
    }
  }
    break;

  case MCT_EVENT_MODULE_STATS_CONFIG_UPDATE: {
    ISP_DBG(ISP_MOD_COM,"%s: received AF update evt, event = 0x%x",
      __func__, event->identity);
    ret = isp_set_stats_config_update(isp, tmp_port,
      UNPACK_SESSION_ID(event->identity), UNPACK_STREAM_ID(event->identity),
      mod_event->module_event_data);
  }
     break;

  case MCT_EVENT_MODULE_SET_DIGITAL_GAIN: {
    ISP_DBG(ISP_MOD_COM,"%s: receive sensor dig gain event 0x%x\n", __func__, event->identity);
    ret = isp_set_aec_trigger_update(isp, tmp_port,
      UNPACK_SESSION_ID(event->identity), UNPACK_STREAM_ID(event->identity),
      mod_event->module_event_data);
  }
    break;

  case MCT_EVENT_MODULE_GET_ISP_TABLES: {
     ISP_DBG(ISP_MOD_COM,"%s: receive isp get LA/Gamma table evt\n", __func__);
     ret = isp_get_la_gamma_tbl(isp, tmp_port,
      UNPACK_SESSION_ID(event->identity), UNPACK_STREAM_ID(event->identity),
      mod_event->module_event_data);
  }
     break;

  case MCT_EVENT_MODULE_BUF_DIVERT_ACK: {
    ISP_DBG(ISP_MOD_COM,"%s: receive buf divert ack, identity = 0x%x\n",
      __func__, event->identity);
    ret = isp_buf_divert_ack(isp, tmp_port,
      UNPACK_SESSION_ID(event->identity), UNPACK_STREAM_ID(event->identity),
      mod_event->module_event_data);
  }
    break;

  case MCT_EVENT_MODULE_META_CHANNEL_DIVERT: {
    ISP_DBG(ISP_MOD_COM,"%s: receive 3a divert to event 0x%x\n", __func__, event->identity);
    ret = isp_set_divert_to_3a(isp, tmp_port,
      UNPACK_SESSION_ID(event->identity), UNPACK_STREAM_ID(event->identity));
  }
    break;

  case MCT_EVENT_MODULE_SET_FAST_AEC_CONVERGE_MODE:
    rc = port_isp_forward_event_to_peer(
             isp, port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: event type = %d, forward_event error\n",
        __func__, mod_event->type);
      ret = -1;
    } else {
      ret = isp_set_fast_aec_mode(isp,
        UNPACK_SESSION_ID(event->identity),
        UNPACK_STREAM_ID(event->identity), mod_event->module_event_data);
    }
    break;

  case MCT_EVENT_MODULE_SENSOR_BRACKET_CTRL:
    ret = isp_set_cam_bracketing_ctrl(isp, tmp_port,
      UNPACK_SESSION_ID(event->identity), UNPACK_STREAM_ID(event->identity),
      mod_event->module_event_data);
    break;

  case MCT_EVENT_MODULE_STATS_GET_DATA: {
    rc = port_isp_forward_event_to_peer(
      isp, port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: event type = %d, forward_event error\n",
        __func__, mod_event->type);
      ret = -1;
    }
    rc = isp_set_gain_from_sensor_req(isp, tmp_port,
      UNPACK_SESSION_ID(event->identity), UNPACK_STREAM_ID(event->identity),
      mod_event->module_event_data);
    if (rc == FALSE) {
      CDBG_ERROR("%s: event type = %d, forward_event error\n",
        __func__, mod_event->type);
      ret = -1;
    }
    break;
  }

  case MCT_EVENT_MODULE_SET_STREAM_CONFIG:
  case MCT_EVENT_MODULE_SET_STREAM_CONFIG_FOR_FLASH:
  case MCT_EVENT_MODULE_SENSOR_META_CONFIG:
  default: {
    rc = port_isp_forward_event_to_peer(
             isp, port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: event type = %d, forward_event error\n",
        __func__, mod_event->type);
      ret = -1;
    }
  }
    break;
  }
  rc = (ret == 0)? TRUE : FALSE;
  ISP_DBG(ISP_MOD_COM,"%s: X, rc %d\n", __func__, rc);
  return rc;
}

/** port_isp_event_func
 *   @port: mct port instance
 *   @event: mct event
 *
 *  Entry point for all isp mct events. Separates control
 *  commands and module events to respective handlers
 *
 *  Returns TRUE for success and FALSE on failure
 **/
static boolean port_isp_event_func(mct_port_t *port, mct_event_t *event)
{
  boolean rc = FALSE;

  switch (event->type) {
  case MCT_EVENT_CONTROL_CMD: {
    /* MCT ctrl event */
    rc = port_isp_mct_ctrl_cmd(port, event);
  }
    break;

  case MCT_EVENT_MODULE_EVENT: {
    /* Event among modules */
    rc = port_isp_module_event(port, event);
  }
    break;

  default: {
    rc = FALSE;
  }
    break;
  }
  return rc;
}

/** port_isp_int_link_func
 *   @identity: contains session_id and stream_id
 *   @port: mct port instance
 *
 *  not used
 *
 *  Returns NULL
 **/
static mct_list_t *port_isp_int_link_func(unsigned int identity,
  mct_port_t *port)
{
  return NULL;
}

/** port_isp_ext_link_func
 *   @identity: contains session_id and stream_id
 *   @port: mct port instance
 *   @peer: peer port instance
 *
 *  Linking with external ports
 *
 *  Returns TRUE for success and FALSE on failure
 **/
static boolean port_isp_ext_link_func(unsigned int identity, mct_port_t* port,
  mct_port_t *peer)
{
  boolean rc = TRUE;
  int ret = 0;
  isp_port_t *tmp_port = (isp_port_t *)port->port_private;
  isp_t *isp = (isp_t *)tmp_port->isp;
  ISP_DBG(ISP_MOD_COM,"%s: E, identity = 0x%x, port = %p, direction = %d\n",
       __func__, identity, port, port->direction);
  pthread_mutex_lock(&isp->mutex);
  if (port->direction == MCT_PORT_SRC)
    ret = isp_link_src_port(isp, tmp_port, peer, UNPACK_SESSION_ID(identity),
                            UNPACK_STREAM_ID(identity));
  else
    ret = isp_link_sink_port(isp, tmp_port, peer, UNPACK_SESSION_ID(identity),
                            UNPACK_STREAM_ID(identity));
  pthread_mutex_unlock(&isp->mutex);
  rc = (ret == 0)? TRUE : FALSE;
  ISP_DBG(ISP_MOD_COM,"%s: X, rc = %d, identity = 0x%x, port = %p, direction = %d\n",
       __func__, rc, identity, port, port->direction);
  return rc;
}

/** port_isp_unlink_func
 *   @identity: contains session_id and stream_id
 *   @port: mct port instance
 *   @peer: peer port instance
 *
 *  Unlinking with external ports
 *
 *  Returns TRUE for success and FALSE on failure
 **/
static void port_isp_unlink_func(unsigned int identity, mct_port_t *port,
  mct_port_t *peer)
{
  int ret = 0;
  isp_port_t *tmp_port = (isp_port_t *)port->port_private;
  isp_t *isp = (isp_t *)tmp_port->isp;
  ISP_DBG(ISP_MOD_COM,"%s: E, identity = 0x%x, port = %p, direction = %d\n",
       __func__, identity, port, port->direction);

  pthread_mutex_lock(&isp->mutex);
  if (port->direction == MCT_PORT_SRC)
    ret = isp_unlink_src_port(isp, tmp_port, peer,
            UNPACK_SESSION_ID(identity), UNPACK_STREAM_ID(identity));
  else
    ret = isp_unlink_sink_port(isp, tmp_port, peer,
            UNPACK_SESSION_ID(identity), UNPACK_STREAM_ID(identity));
  pthread_mutex_unlock(&isp->mutex);
  ISP_DBG(ISP_MOD_COM,"%s: X, ret = %d, identity = 0x%x, port = %p, direction = %d\n",
       __func__, ret, identity, port, port->direction);
}

/** port_isp_set_caps_func
 *   @port: mct port instance
 *   @caps: port capabilities
 *
 *  Set caps functions: not used
 *
 *  Returns 0 always
 **/
static boolean port_isp_set_caps_func(mct_port_t *port,
  mct_port_caps_t *caps)
{
  return 0;
}

/** port_isp_check_caps_reserve_func
 *   @port: mct port instance
 *   @peer_caps: peer port capabilities
 *   @info: contains session_id and stream_id
 *
 *  Reserve source and sink port resources
 *
 *  Returns TRUE for success and FALSE on failure
 **/
static boolean port_isp_check_caps_reserve_func(mct_port_t *port,
  void *peer_caps, void *info)
{
  int i = 0, ret = 0;
  boolean rc = FALSE;
  isp_port_t *tmp_port = (isp_port_t *)port->port_private;
  isp_t *isp = (isp_t *)tmp_port->isp;
  mct_stream_info_t *stream_info = (mct_stream_info_t *)info;

  ISP_DBG(ISP_MOD_COM,"%s: E, identity = 0x%x, port = %p, direction = %d\n",
       __func__, stream_info->identity, port, port->direction);

  pthread_mutex_lock(&isp->mutex);
  if (port->direction == MCT_PORT_SINK)
    ret = isp_reserve_sink_port(isp, tmp_port,
            (ispif_src_port_caps_t *)peer_caps, stream_info,
            UNPACK_SESSION_ID(stream_info->identity),
            UNPACK_STREAM_ID(stream_info->identity));
  else
    ret = isp_reserve_src_port(isp, tmp_port,
            stream_info, UNPACK_SESSION_ID(stream_info->identity),
            UNPACK_STREAM_ID(stream_info->identity));
  if(tmp_port->u.src_port.port_type == ISP_SRC_PORT_DATA) {
    switch(tmp_port->u.src_port.caps.sensor_cap.sensor_cid_ch[0].fmt) {
      case CAM_FORMAT_YUV_RAW_8BIT_YUYV: {
        if (tmp_port->u.src_port.caps.use_pix){
          port->caps.u.frame.format_flag |= MCT_PORT_CAP_FORMAT_YCBCR;
        }
        else
          port->caps.u.frame.format_flag = MCT_PORT_CAP_FORMAT_YCBCR |
            MCT_PORT_CAP_FORMAT_YCBYCR;
      }
        break;
      case CAM_FORMAT_YUV_RAW_8BIT_YVYU: {
        if (tmp_port->u.src_port.caps.use_pix){
          port->caps.u.frame.format_flag |= MCT_PORT_CAP_FORMAT_YCBCR;
        }
        else
          port->caps.u.frame.format_flag = MCT_PORT_CAP_FORMAT_YCBCR |
            MCT_PORT_CAP_FORMAT_YCRYCB;
      }
        break;
      case CAM_FORMAT_YUV_RAW_8BIT_UYVY: {
        if (tmp_port->u.src_port.caps.use_pix){
          port->caps.u.frame.format_flag |= MCT_PORT_CAP_FORMAT_YCBCR;
        }
        else
          port->caps.u.frame.format_flag = MCT_PORT_CAP_FORMAT_YCBCR |
            MCT_PORT_CAP_FORMAT_CBYCRY;
      }
        break;
      case CAM_FORMAT_YUV_RAW_8BIT_VYUY: {
        if (tmp_port->u.src_port.caps.use_pix){
          port->caps.u.frame.format_flag |= MCT_PORT_CAP_FORMAT_YCBCR;
        }
        else
          port->caps.u.frame.format_flag = MCT_PORT_CAP_FORMAT_YCBCR |
            MCT_PORT_CAP_FORMAT_CRYCBY;
      }
        break;
      default:
        if(tmp_port->u.src_port.caps.use_pix)
          port->caps.u.frame.format_flag |= MCT_PORT_CAP_FORMAT_YCBCR;
        else
          port->caps.u.frame.format_flag |= MCT_PORT_CAP_FORMAT_BAYER;
        break;
    }
  }
  pthread_mutex_unlock(&isp->mutex);
  rc = (ret == 0)? TRUE : FALSE;
  ISP_DBG(ISP_MOD_COM,"%s: X rc %d\n", __func__, rc);

  return rc;
}

/** port_isp_check_caps_unreserve_func
 *   @port: mct port instance
 *   @peer_caps: peer port capabilities
 *   @info: contains session_id and stream_id
 *
 *  Unreserve source and sink port resources
 *
 *  Returns TRUE for success and FALSE on failure
 **/
static boolean port_isp_check_caps_unreserve_func(mct_port_t *port,
  unsigned int identity)
{
  boolean rc = TRUE;
  int i = 0, ret = 0;
  isp_port_t *tmp_port = (isp_port_t *)port->port_private;
  isp_t *isp = (isp_t *)tmp_port->isp;
  ISP_DBG(ISP_MOD_COM,"%s: E, identity = 0x%x, port = %p, direction = %d\n",
       __func__, identity, port, port->direction);

  pthread_mutex_lock(&isp->mutex);
  if (port->direction == MCT_PORT_SINK) {
    ret = isp_unreserve_sink_port(isp, tmp_port,
            UNPACK_SESSION_ID(identity), UNPACK_STREAM_ID(identity));
  } else {
    ret = isp_unreserve_src_port(isp, tmp_port,
            UNPACK_SESSION_ID(identity), UNPACK_STREAM_ID(identity));
    port->caps.u.frame.format_flag = 0;
  }
  pthread_mutex_unlock(&isp->mutex);
  rc = (ret == 0)? TRUE : FALSE;
  ISP_DBG(ISP_MOD_COM,"%s: X rc %d\n", __func__, rc);

  return rc;
}

/** isp_overwrite_port_funcs
 *   @port: mct port instance
 *   @private_data: peer port capabilities
 *
 *  Assign mct port function pointers to respective isp port
 *  functions
 *
 *  Returns nothing
 **/
static void isp_overwrite_port_funcs(mct_port_t *port, void *private_data)
{
  port->event_func = port_isp_event_func;
  port->int_link = port_isp_int_link_func;
  port->ext_link = port_isp_ext_link_func;
  port->un_link = port_isp_unlink_func;
  port->set_caps = port_isp_set_caps_func;
  port->check_caps_reserve = port_isp_check_caps_reserve_func;
  port->check_caps_unreserve =port_isp_check_caps_unreserve_func;
  port->port_private = private_data;
}

/** port_isp_create_sink_ports
 *   @isp: isp instance
 *
 *  Prepares compatible mct isp port of type sink
 *
 *  Returns 0 for success and negative error on failure
 **/
static int port_isp_create_sink_ports(isp_t *isp)
{
  int i;
  int rc = 0;
  isp_port_t *isp_port = NULL;
  char port_name[32];
  mct_port_t *mct_port = NULL;
  mct_module_t *isp_module = isp->module;

  ISP_DBG(ISP_MOD_COM,"%s: E\n", __func__);
  for (i = 0; i < ISP_SINK_PORTS_NUM; i++) {

    isp_port = malloc(sizeof(isp_port_t));
    if (!isp_port) {
      CDBG_ERROR("%s: no mem for isp sink port\n", __func__);
      rc = -ENOMEM;
      goto end;
    }
    memset(isp_port,  0,  sizeof(isp_port_t));
    snprintf(port_name, sizeof(port_name), "isp_sink%d", i);
    mct_port = mct_port_create(port_name);
    if (!mct_port) {
      CDBG_ERROR("%s: mct_port_create error\n", __func__);
      free (isp_port);
      rc = -ENOMEM;
      goto end;
    }
    mct_port->direction = MCT_PORT_SINK;
    /* TODO: check return */
    mct_module_add_port(isp_module, mct_port);
    mct_port->caps.port_caps_type = MCT_PORT_CAPS_OPAQUE; /* opaque type */
    isp_overwrite_port_funcs(mct_port, (void *)isp_port);
    isp_port->port = mct_port;
    mct_port->caps.u.data = &isp_port->u.sink_port.caps;
    isp_port->isp = (void *)isp;
  }
end:
  ISP_DBG(ISP_MOD_COM,"%s: X rc %d\n", __func__, rc);
  return rc;
}

/** port_isp_create_src_ports
 *   @isp: isp instance
 *   @src_port_type: type of source port (data/3A)
 *
 *  Prepares compatible mct isp port of type source
 *
 *  Returns 0 for success and negative error on failure
 **/
static int port_isp_create_src_ports(isp_t *isp,
  isp_src_port_type_t src_port_type)
{
  int i, num;
  int rc = 0;
  isp_port_t *isp_port = NULL;
  char port_name[32];
  mct_port_t *mct_port = NULL;
  mct_module_t *isp_module = isp->module;

  ISP_DBG(ISP_MOD_COM,"%s: E\n", __func__);
  if (src_port_type == ISP_SRC_PORT_3A)
    num = ISP_SRC_PORTS_3A_NUM;
  /*else if (src_port_type == ISP_SRC_PORT_IMAGING)
    num = ISP_SRC_PORTS_IMAGING_NUM;*/
  else if (src_port_type == ISP_SRC_PORT_DATA)
    num = ISP_SRC_PORTS_BUF_NUM;
  else {
    CDBG_ERROR("%s: invalid src_port_type %d", __func__, src_port_type);
    return -1;
  }
  for (i = 0; i < num; i++) {
    if (src_port_type == ISP_SRC_PORT_3A)
      snprintf(port_name, sizeof(port_name), "isp_src_3A%d", i);
    else if (src_port_type == ISP_SRC_PORT_DATA)
      snprintf(port_name, sizeof(port_name), "isp_src_buf%d", i);
    else {
      CDBG_ERROR("%s: invalid src_port_type %d", __func__, src_port_type);
      return -1;
    }
    isp_port = malloc(sizeof(isp_port_t));
    if (!isp_port) {
      CDBG_ERROR("%s: isp_port_create error\n", __func__);
      rc = -ENOMEM;
      goto end;
    }
    memset(isp_port, 0, sizeof(isp_port_t));
    isp_port->isp = (void *)isp;
    mct_port = mct_port_create(port_name);
    if (!mct_port) {
      CDBG_ERROR("%s: mct_port_create error\n", __func__);
      free (isp_port);
      rc = -ENOMEM;
      goto end;
    }
    mct_port->direction = MCT_PORT_SRC;
    /* TODO: check return */
    mct_module_add_port(isp_module, mct_port);
    if (src_port_type == ISP_SRC_PORT_3A) {
      mct_port->caps.port_caps_type = MCT_PORT_CAPS_STATS;
      mct_port->caps.u.stats.flag =
        isp->data.sd_info.sd_info[0].cap.stats_mask;
      ISP_DBG(ISP_MOD_COM,"%s: stats_mask = 0x%x\n", __func__, mct_port->caps.u.stats.flag);
    } else {
      mct_port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;
      mct_port->caps.u.frame.priv_data = (uint32_t)isp_port;
      /* Format flag will be set when we know the type of data
         ISP module outputs */
      mct_port->caps.u.frame.format_flag = 0;
    }
    isp_overwrite_port_funcs(mct_port, (void *)isp_port);
    isp_port->port = mct_port;
  }
end:
  ISP_DBG(ISP_MOD_COM,"%s: X rc %d\n", __func__, rc);
  return rc;
}

/** port_isp_free_mem_func
 *   @data: mctl port instance
 *   @user_data: not used
 *
 *  Release the created mct ports
 *
 *  Returns TRUE always
 **/
static boolean port_isp_free_mem_func(void *data, void *user_data)
{
  mct_port_t *port = (mct_port_t *)data;
  mct_module_t *module = (mct_module_t *)user_data;

  ISP_DBG(ISP_MOD_COM,"%s: E\n", __func__);

  mct_object_unparent(MCT_OBJECT_CAST(port), MCT_OBJECT_CAST(module));
  if (port->port_private){
    free(port->port_private);
    mct_port_destroy(port);
    port->port_private = NULL;
  }
  ISP_DBG(ISP_MOD_COM,"%s: X\n", __func__);
  return TRUE;
}

/** port_isp_destroy_ports
 *   @isp: isp instance
 *
 *  Destroy all created isp ports
 *
 *  Returns nothing
 **/
void port_isp_destroy_ports(isp_t *isp)
{
  ISP_DBG(ISP_MOD_COM,"%s: E\n", __func__);
  if (isp->module->sinkports) {
    mct_list_traverse(isp->module->sinkports,
              port_isp_free_mem_func, isp->module);
    mct_list_free_list(isp->module->sinkports);
    isp->module->sinkports= NULL;
  }
  if (isp->module->srcports) {
    mct_list_traverse(isp->module->srcports,
              port_isp_free_mem_func, isp->module);
    mct_list_free_list(isp->module->srcports);
    isp->module->srcports= NULL;
  }
  ISP_DBG(ISP_MOD_COM,"%s: X\n", __func__);
}

/** port_isp_create_ports
 *   @isp: isp instance
 *
 *  Creates all the necessary isp ports like 3A, data and sink
 *
 *  Returns 0 for success and negative error for failure
 **/
int port_isp_create_ports(isp_t *isp)
{
  int i;
  int rc = 0;
  ISP_DBG(ISP_MOD_COM,"%s: E\n", __func__);
  rc = port_isp_create_sink_ports(isp);
  if (rc == 0)
    rc = port_isp_create_src_ports(isp, ISP_SRC_PORT_3A);
  /*if (rc == 0)
    rc = isp_create_src_ports(isp, ISP_SRC_PORT_IMAGING);*/
  if (rc == 0)
    rc = port_isp_create_src_ports(isp, ISP_SRC_PORT_DATA);

end:
  if (rc < 0)
    port_isp_destroy_ports(isp);
  ISP_DBG(ISP_MOD_COM,"%s: X rc %d\n", __func__, rc);
  return rc;
}
