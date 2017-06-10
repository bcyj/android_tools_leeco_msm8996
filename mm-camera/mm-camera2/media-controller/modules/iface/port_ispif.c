/*============================================================================
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <stdlib.h>
#include <assert.h>
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
#include "ispif.h"
#include "ispif_util.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#ifdef PORT_ISPIF_DEBUG
#undef CDBG
#define CDBG ALOGE
#endif

/** port_ispif_free_mem_func:
 *    @data: MCTL port
 *    @user_data: not used
 *
 *  This function runs in MCTL thread context.
 *
 *  This is a visitor function: frees a port private data.
 *
 *  Return:  TRUE
 **/
static boolean port_ispif_free_mem_func(void *data, void *user_data)
{
  mct_port_t *port = (mct_port_t *)data;
  mct_module_t *module = (mct_module_t *)user_data;
  assert(port != NULL);
  assert(port->private != NULL);

  mct_object_unparent(MCT_OBJECT_CAST(port), MCT_OBJECT_CAST(module));
  if (port->port_private){
    free (port->port_private);
    mct_port_destroy(port);
    port->port_private = NULL;
  }

  return TRUE;
}

/** port_ispif_destroy_ports:
 *    @ispif: ispif instance
 *
 *  This function runs in MCTL thread context.
 *
 *  This function destroys module ports and frees their resources
 *
 *  Return: None
 **/
void port_ispif_destroy_ports(ispif_t *ispif)
{
  if (ispif->module->sinkports) {
    mct_list_traverse(ispif->module->sinkports,
              port_ispif_free_mem_func, ispif->module);
    mct_list_free_list(ispif->module->sinkports);
    ispif->module->sinkports= NULL;
  }
  if (ispif->module->srcports) {
    mct_list_traverse(ispif->module->srcports,
              port_ispif_free_mem_func, ispif->module);
    mct_list_free_list(ispif->module->srcports);
    ispif->module->srcports= NULL;
  }
}

/** port_ispif_send_event_to_peer:
 *    @data1: MCTL port to which peer will send event
 *    @user_data: MCTL event that will be send
 *
 *  This function runs in MCTL thread context.
 *
 *  This is a visitor function: sends event to peer port
 *
 *  Return: TRUE  - event sent successfuly
 **/
static boolean port_ispif_send_event_to_peer(void *data1, void *user_data)
{
  mct_port_t *mct_port = (mct_port_t *)data1;
  mct_event_t *event = (mct_event_t *)user_data;
  ispif_port_t *ispif_port = (ispif_port_t * )mct_port->port_private;
  ispif_stream_t *stream = NULL;
  ispif_stream_t **streams = NULL;
  int i;
  uint32_t identity;
  boolean rc = FALSE;

  if (ispif_port->state == ISPIF_PORT_STATE_CREATED) {
    /* not used port */

    return TRUE;
  }

  if (mct_port->direction == MCT_PORT_SINK) {
    ispif_sink_port_t *sink_port = &ispif_port->u.sink_port;
    streams = sink_port->streams;
  } else if (mct_port->direction == MCT_PORT_SRC) {
    ispif_src_port_t *src_port = &ispif_port->u.src_port;
    streams = src_port->streams;
  } else
    return TRUE;

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

  return TRUE;
}

/** port_ispif_forward_event_to_peer:
 *    @ispif: ispif instance
 *    @mct_port: MCTL port to which peer will send event
 *    @event: MCTL event that will be send
 *
 *  This function runs in MCTL thread context.
 *
 *  This function forwards event to peer port in desired direction
 *
 *  Return: TRUE  - event sent successfuly
 *          FALSE - Invalid port type
 **/
static  boolean port_ispif_forward_event_to_peer(ispif_t *ispif,
  mct_port_t *mct_port, mct_event_t *event)
{
  /* if receive from sink forward to src's peer */
  if (mct_port->direction == MCT_PORT_SINK)
    return mct_list_traverse(ispif->module->srcports,
                           port_ispif_send_event_to_peer,
                           (void *)event);

  else if (mct_port->direction == MCT_PORT_SRC)
    return mct_list_traverse(ispif->module->sinkports,
                           port_ispif_send_event_to_peer,
                           (void *)event);
  else
    return FALSE;
}

/** port_ispif_proc_mct_ctrl_cmd:
 *    @port: MCTL port to which peer will send event
 *    @event: MCTL event that contain command
 *
 *  This function runs in MCTL thread context.
 *
 *  This function processes a command event from MCTL
 *
 *  Return:  0 - Command executed and forwarded successfully
 *          -1 - Error
 **/
static int port_ispif_proc_mct_ctrl_cmd(mct_port_t *port, mct_event_t *event)
{
  int ret = 0;
  boolean rc = FALSE;
  mct_event_control_t *ctrl;
  ispif_port_t *tmp_port;
  ispif_t *ispif;

  if (!port || !event) {
    CDBG_ERROR ("%s: error: port or event is NULL: port %p, event %p", __func__,
      port, event );
    return -1;
  }

  ctrl = &event->u.ctrl_event;
  /* should not happen */
  assert(port->port_private != NULL);
  tmp_port = (ispif_port_t *)port->port_private;
  ispif = (ispif_t *)tmp_port->ispif;

  CDBG("%s: E, type = %d\n", __func__, ctrl->type);
  switch (ctrl->type) {
  case MCT_EVENT_CONTROL_STREAMON:
    rc = port_ispif_forward_event_to_peer(
      ispif, port, event);
    if (rc == 0) {
      CDBG_ERROR("%s: forward_event error\n", __func__);
      ret = -1;
    } else {
      ret = ispif_streamon(ispif, tmp_port, UNPACK_SESSION_ID(event->identity),
        UNPACK_STREAM_ID(event->identity), event);
        /* after stream on ispif checks if there is a session pending on resume.
         * If yes it will send bus message to that session's medical control
         * to trigger session resuming. */
      pthread_mutex_lock(&ispif->mutex);
      ispif_resume_pending_session(ispif);
      pthread_mutex_unlock(&ispif->mutex);
    }
    break;

  case MCT_EVENT_CONTROL_STREAMOFF:
    rc = port_ispif_forward_event_to_peer(
      ispif, port, event);
    if (rc == 0) {
      CDBG_ERROR("%s: forward_event error\n", __func__);
      ret = -1;
    } else {
      ret = ispif_streamoff(ispif, tmp_port, UNPACK_SESSION_ID(event->identity),
        UNPACK_STREAM_ID(event->identity), event);
    }
    break;

  case MCT_EVENT_CONTROL_SET_PARM:
    ret = ispif_set_hal_param(ispif, tmp_port,
      UNPACK_SESSION_ID(event->identity),
            UNPACK_STREAM_ID(event->identity), event);
    if (ret == 0) {
      rc = port_ispif_forward_event_to_peer(ispif, port, event);
      if (rc == 0) {
        CDBG_ERROR("%s: forward_event error\n", __func__);
        ret = -1;
      }
    }
    break;

  case MCT_EVENT_CONTROL_PARM_STREAM_BUF:
    ret = ispif_set_hal_stream_param(ispif, tmp_port,
      UNPACK_SESSION_ID(event->identity),
      UNPACK_STREAM_ID(event->identity), event);
    if (ret == 0) {
      rc = port_ispif_forward_event_to_peer(
        ispif, port, event);
      if (rc == 0) {
        CDBG_ERROR("%s: forward_event error\n", __func__);
        ret = -1;
      }
    }
    break;

  default:
    rc = port_ispif_forward_event_to_peer(ispif, port, event);
    if (rc == 0) {
      CDBG_ERROR("%s: forward_event error\n", __func__);
      ret = -1;
    }
    break;
  }

  rc = (ret == 0) ? TRUE : FALSE;
  CDBG("%s: X, type = %d, rc = %d\n", __func__, ctrl->type, rc);

  return rc;
}

/** port_ispif_proc_module_event:
 *    @port: MCTL port to which peer will send event
 *    @event: MCTL event that will be processed
 *
 *  This function runs in MCTL thread context.
 *
 *  This function processes a module event from MCTL
 *
 *  Return: TRUE  - Event processed and forwarded successfully
 *          FALSE - Error
 **/
static boolean port_ispif_proc_module_event(mct_port_t *port,
  mct_event_t *event)
{
  int ret = 0;
  boolean rc = FALSE;
  mct_event_module_t *mod_event;
  ispif_port_t *tmp_port;
  ispif_t *ispif;

  if (!port || !event) {
    CDBG_ERROR ("%s: error: port or event is NULL: port %p, event %p", __func__,
      port, event );
    return FALSE;
  }

  mod_event = &event->u.module_event;
  /* should not happen */
  assert(port->port_private != NULL);
  tmp_port = (ispif_port_t *)port->port_private;
  ispif = (ispif_t *)tmp_port->ispif;

  switch (mod_event->type) {
  case MCT_EVENT_MODULE_SET_STREAM_CONFIG:
    /* ISPIF uses a separate event to config ISP. */
retry:
    ret = ispif_sink_port_config(ispif, tmp_port,
      UNPACK_STREAM_ID(event->identity),
      UNPACK_SESSION_ID(event->identity),

      (sensor_out_info_t *)mod_event->module_event_data);
    if (ret == 0) {
      /*forward sensor's config to downstream. */
      rc = port_ispif_forward_event_to_peer(
             ispif, port, event);
      if (rc == FALSE) {
        CDBG_ERROR("%s: port_ispif_forward_event_to_peer error\n", __func__);
        ret = -1;
      } else if (ispif->meta_pending) {
        mct_event_t meta_event;
        ispif->meta_pending = FALSE;
        meta_event.type = MCT_EVENT_MODULE_EVENT;
        meta_event.identity = ispif->meta_identity;
        meta_event.direction = MCT_EVENT_DOWNSTREAM;
        meta_event.u.module_event.type = MCT_EVENT_MODULE_ISP_META_CONFIG;
        meta_event.u.module_event.module_event_data = &ispif->meta_info;
        rc = port_ispif_forward_event_to_peer(
          ispif, port, &meta_event);
        if (rc == FALSE) {
          CDBG_ERROR("%s: meta event_to_peer error\n", __func__);
          ret = -1;
        }
      }

    } else if (ret == -ERROR_CODE_ISP_RESOURCE_STARVING) {
      /* the reason to move teh dual isp to pip trigger here is to
       * get the ispif golbal lock.
       */
      pthread_mutex_lock(&ispif->mutex);
      ret = ispif_dual_isp_pip_switch(ispif, tmp_port,
            (sensor_out_info_t *)mod_event->module_event_data);
      pthread_mutex_unlock(&ispif->mutex);
      /* TODO: do not wait like this, but instead use a message to know
       * when there is resource available */


      usleep(1000000);
      goto retry;
    } else if (ret == -EAGAIN) {
        ispif_util_pip_switching_cap_op_pixclk(ispif, tmp_port,
            (sensor_out_info_t *)mod_event->module_event_data);
        ret = 0;
    }
    break;

  case MCT_EVENT_MODULE_SENSOR_META_CONFIG:
    ispif->meta_info = *((sensor_meta_data_t *)mod_event->module_event_data);
    ispif->meta_identity = event->identity;

    rc = ispif_meta_channel_config(ispif, UNPACK_SESSION_ID(event->identity),
      UNPACK_STREAM_ID(event->identity), tmp_port);

    if (rc < 0) {
      CDBG_ERROR("%s: port_ispif meta channel config error\n", __func__);
      ret = -1;

    }

    /*  Postpone the message to isp after stream config.
        The message for creation of meta channel should be postponed
        because isp needs some data from stream configuration */
    ispif->meta_pending = TRUE;

    rc = port_ispif_forward_event_to_peer(ispif, port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: port_ispif_forward_event_to_peer error\n", __func__);
      ret = -1;
    }
    break;

  case MCT_EVENT_MODULE_ISPIF_OUTPUT_INFO:
    ret = ispif_set_split_info(ispif,
      event->identity >> 16,     // session id
      event->identity & 0xffff,  // stream id
      (ispif_out_info_t *)mod_event->module_event_data);
    if (ret != 0)
      CDBG_ERROR("%s failed handling MCT_EVENT_MODULE_ISPIF_OUTPUT_INFO\n",
        __func__);
    break;

  case MCT_EVENT_MODULE_SET_FAST_AEC_CONVERGE_MODE:
    rc = port_ispif_forward_event_to_peer(
             ispif, port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: port_ispif_forward_event_to_peer error\n", __func__);
      ret = -1;
    } else {
      ret = ispif_set_fast_aec_mode(ispif,
        event->identity >> 16,     // session id
        mod_event->module_event_data);
    }
    break;
  case MCT_EVENT_MODULE_ISPIF_RESET:
    rc = ispif_util_proc_reset(ispif, event);
    break;

  default:
    rc = port_ispif_forward_event_to_peer(
             ispif, port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: port_ispif_forward_event_to_peer error\n", __func__);
      ret = -1;
    }
    break;
  }

  rc = (ret == 0)? TRUE : FALSE;

  return rc;
}

/** port_ispif_event_func:
 *    @port: MCTL port to which peer will send event
 *    @event: MCTL event that will be processed
 *
 *  This function runs in MCTL thread context.
 *
 *  This function processes a MCTL event
 *
 *  Return: TRUE  - Event processed and forwarded successfully
 *          FALSE - Error
 **/
static boolean port_ispif_event_func(mct_port_t *port, mct_event_t *event)
{
  boolean rc = FALSE;

  switch (event->type) {
  case MCT_EVENT_CONTROL_CMD:
    /* MCT ctrl event */
    rc = port_ispif_proc_mct_ctrl_cmd(port, event);
    break;

  case MCT_EVENT_MODULE_EVENT:
    /* Event among modules */
    rc = port_ispif_proc_module_event(port, event);
    break;

  default:
    rc = FALSE;
  }

  return rc;
}

/** port_ispif_int_link_func:
 *    @identity: identity
 *    @port: MCTL port to be linked
 *
 *  This function runs in MCTL thread context.
 *
 *  This function implements port internal link method of port object - dummy
 *
 *  Return: NULL
 **/
static mct_list_t *port_ispif_int_link_func(unsigned int identity,
  mct_port_t *port)
{
  /* dummy function. */
  return NULL;
}

/** port_ispif_ext_link_func:
 *    @identity: identity
 *    @port: MCTL port to be linked
 *    @peer: MCTL port to which port will link
 *
 *  This function runs in MCTL thread context.
 *
 *  This function implements port external link method of port object
 *
 *  Return: TRUE  - Ports linked succsessfuly
 *          FALSE - Error
 **/
static boolean port_ispif_ext_link_func(unsigned int identity, mct_port_t* port,
  mct_port_t *peer)
{
  boolean rc = TRUE;
  int ret = 0;
  ispif_port_t *tmp_port = (ispif_port_t *)port->port_private;
  ispif_t *ispif = (ispif_t *)tmp_port->ispif;

  CDBG("%s: E, identity = 0x%x, port = %p\n", __func__, identity, port);
  pthread_mutex_lock(&ispif->mutex);
  if (port->direction == MCT_PORT_SRC)
    ret = ispif_link_src_port(ispif, tmp_port, peer,
            UNPACK_SESSION_ID(identity), UNPACK_STREAM_ID(identity));
  else
    ret = ispif_link_sink_port(ispif, tmp_port, peer,
            UNPACK_SESSION_ID(identity), UNPACK_STREAM_ID(identity));

  pthread_mutex_unlock(&ispif->mutex);
  rc = (ret == 0)? TRUE : FALSE;
  CDBG("%s: X, rc = %d, identity = 0x%x, port = %p\n", __func__, rc, identity,
    port);
  return rc;
}

/** port_ispif_unlink_func:
 *    @identity: identity
 *    @port: MCTL port to be unlinked
 *    @peer: MCTL port to which port is linked
 *
 *  This function runs in MCTL thread context.
 *
 *  This function implements port unlink method of port object
 *
 *  Return: None
 **/
static void port_ispif_unlink_func(unsigned int identity, mct_port_t *port,
  mct_port_t *peer)
{
  int ret = 0;
  ispif_port_t *tmp_port = (ispif_port_t *)port->port_private;
  uint32_t params_id;
  ispif_t *ispif = (ispif_t *)tmp_port->ispif;

  CDBG("%s: E, identity = 0x%x, port = %p, direction = %d\n",
       __func__, identity, port, port->direction);
  pthread_mutex_lock(&ispif->mutex);
  if (port->direction == MCT_PORT_SRC)
    ret = ispif_unlink_sink_port(ispif, tmp_port, peer,
            UNPACK_SESSION_ID(identity), UNPACK_STREAM_ID(identity));
  else
    ret = ispif_unlink_src_port(ispif, tmp_port, peer,
            UNPACK_SESSION_ID(identity), UNPACK_STREAM_ID(identity));

  pthread_mutex_unlock(&ispif->mutex);
  CDBG("%s: X, ret = %d, identity = 0x%x, port = %p, direction = %d\n",
       __func__, ret, identity, port, port->direction);
}


/** port_ispif_set_caps_func:
 *    @port: MCTL port
 *    @caps: MCTL port capabilities
 *
 *  This function runs in MCTL thread context.
 *
 *  This function implements port set capabilities method of port object
 *
 *  Return: TRUE
 **/
static boolean port_ispif_set_caps_func(mct_port_t *port,
  mct_port_caps_t *caps)
{
  boolean rc = TRUE;
  return rc;
}

/** port_ispif_check_caps_reserve_func:
 *    @port: MCTL port
 *    @peer_caps: MCTL port capabilities
 *    @pstream_info: stream info
 *
 *  This function runs in MCTL thread context.
 *
 *  This function checks and tries to reserve port according given capabilities
 *
 *  Return: TRUE  - ports reserved succesfuly
 *          FALSE - Error
 **/
static boolean port_ispif_check_caps_reserve_func(mct_port_t *port,
  void *peer_caps, void *pstream_info)
{
  mct_stream_info_t *stream_info = pstream_info;
  int i = 0, ret = 0;
  boolean rc = FALSE;
  ispif_port_t *tmp_port = (ispif_port_t *)port->port_private;
  ispif_t *ispif = (ispif_t *)tmp_port->ispif;

  CDBG("%s: E, identity = 0x%x, port = %p, direction = %d\n",
    __func__, stream_info->identity, port, port->direction);
  pthread_mutex_lock(&ispif->mutex);
  if (port->direction == MCT_PORT_SINK) {
    ret = ispif_reserve_sink_port(ispif, tmp_port,
      (sensor_src_port_cap_t *)peer_caps, stream_info,
      UNPACK_SESSION_ID(stream_info->identity),
      UNPACK_STREAM_ID(stream_info->identity));
  } else {
    ret = ispif_reserve_src_port(ispif, tmp_port, stream_info,
      UNPACK_SESSION_ID(stream_info->identity),
      UNPACK_STREAM_ID(stream_info->identity));
  }

  pthread_mutex_unlock(&ispif->mutex);

  rc = (ret == 0)? TRUE : FALSE;

  return rc;
}

/** port_ispif_check_caps_unreserve_func:
 *    @port: MCTL port
 *    @identity: stream identity
 *
 *  This function runs in MCTL thread context.
 *
 *  This function frees reserved port according given capabilities
 *
 *  Return: TRUE  - ports unreserved succesfuly
 *          FALSE - Error
 **/
static boolean port_ispif_check_caps_unreserve_func(mct_port_t *port,
  unsigned int identity)
{
  boolean rc = TRUE;
  int i = 0, ret = 0;
  ispif_port_t *tmp_port = (ispif_port_t *)port->port_private;
  ispif_t *ispif = (ispif_t *)tmp_port->ispif;
  uint32_t params_id;

  CDBG("%s: E, identity = 0x%x, port = %p, direction = %d\n",
       __func__, identity, port, port->direction);

  pthread_mutex_lock(&ispif->mutex);
  if (port->direction == MCT_PORT_SINK)
    ret = ispif_unreserve_sink_port(ispif, tmp_port,
            UNPACK_SESSION_ID(identity), UNPACK_STREAM_ID(identity));
  else
    ret = ispif_unreserve_src_port(ispif, tmp_port,
            UNPACK_SESSION_ID(identity), UNPACK_STREAM_ID(identity));

  pthread_mutex_unlock(&ispif->mutex);
  rc = (ret == 0)? TRUE : FALSE;
  CDBG("%s: X, rc = %d, identity = 0x%x, port = %p, direction = %d\n",
       __func__, rc, identity, port, port->direction);

  return rc;
}

/** ispif_overwrite_port_funcs:
 *    @port: MCTL port
 *    @private_data: port private data
 *
 *  This function runs in MCTL thread context.
 *
 *  This function replaces port methods with custom ones
 *
 *  Return: None
 **/
static void ispif_overwrite_port_funcs(mct_port_t *port, void *private_data)
{
  port->event_func = port_ispif_event_func;
  port->int_link = port_ispif_int_link_func;
  port->ext_link = port_ispif_ext_link_func;
  port->un_link = port_ispif_unlink_func;
  port->set_caps = port_ispif_set_caps_func;
  port->check_caps_reserve = port_ispif_check_caps_reserve_func;
  port->check_caps_unreserve = port_ispif_check_caps_unreserve_func;
  port->port_private = private_data;
}

/** port_ispif_add_ports:
 *    @ispif: ispif instance
 *    @num_ports: number of ports
 *    @is_sink: true if ports are sink
 *
 *  This function runs in MCTL thread context.
 *
 *  This function adds number of sink or src ports to ispif module
 *
 *  Return: 0 - Success
 *         -1 - Error while adding ports
 **/
static int port_ispif_add_ports(ispif_t *ispif, int num_ports, int is_sink)
{
  int i;
  int rc = 0;
  ispif_port_t *ispif_port = NULL;
  char port_name[32];
  mct_port_t *mct_port = NULL;
  mct_module_t *ispif_module = ispif->module;

  for (i = 0; i < num_ports; i++) {
    ispif_port = malloc(sizeof(ispif_port_t));
    if (!ispif_port) {
      CDBG_ERROR("%s: cannot malloc ispif sink port\n", __func__);
      rc = -ENOMEM;
      goto end;
    }

    memset(ispif_port, 0, sizeof(ispif_port_t));
    if (is_sink)
      snprintf(port_name, sizeof(port_name), "iface_sink%d", i);
    else
      snprintf(port_name, sizeof(port_name), "iface_src%d", i);

    mct_port = mct_port_create(port_name);
    if (!mct_port) {
      CDBG_ERROR("%s: mct_port_create error\n", __func__);
      free (ispif_port);
      rc = -ENOMEM;
      goto end;
    }

    ispif_port->port = mct_port;

    if (is_sink)
      mct_port->direction = MCT_PORT_SINK;
    else
      mct_port->direction = MCT_PORT_SRC;

    mct_module_add_port(ispif_module, mct_port);
    mct_port->caps.port_caps_type = MCT_PORT_CAPS_OPAQUE; /* opaque type */
    if (is_sink)
      mct_port->caps.u.data = (void *)&ispif_port->u.sink_port.sensor_cap;
    else
      mct_port->caps.u.data = (void *)&ispif_port->u.src_port.caps;

    ispif_overwrite_port_funcs(ispif_port->port, (void *)ispif_port);
    ispif_port->ispif = (void *)ispif;
  }

end:
  return rc;
}

/** port_ispif_add_ports:
 *    @ispif: ispif instance
 *    @caps: ispif module capabilities
 *
 *  This function runs in MCTL thread context.
 *
 *  This function create all ports to ispif module
 *
 *  Return: 0 - Success
 *         -1 - Error while adding ports
 **/
int port_ispif_create_ports(ispif_t *ispif, ispif_caps_t *caps)
{
  int rc = 0;
  int is_sink = 1;

  rc = port_ispif_add_ports(ispif, caps->max_num_sink_ports, is_sink);
  if (rc == 0) {
    is_sink= 0;
    rc = port_ispif_add_ports(ispif, caps->max_num_src_ports, is_sink);
  }

end:
  if (rc < 0)
    port_ispif_destroy_ports(ispif);

  return rc;
}
