/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "camera_dbg.h"
#include "mct_stream.h"
#include "port_pproc_common.h"
#include "port_cpp.h"
#include "pproc_interface.h"
#include "module_pproc_common.h"
#include "port_pproc_common.h"
#include "module_pproc.h"

#if 0
#undef CDBG
#define CDBG ALOGE
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE
#endif

/** port_cpp_forward_event:
 *    @module: cpp module pointer
 *    @mct_port: port from which event originates
 *    @event: event data
 *
 *  Return: TRUE for success and FALSE on failure
 *
 **/
static boolean port_cpp_forward_event(mct_module_t *module,
  mct_port_t *mct_port, mct_event_t *event)
{
  module_pproc_common_ctrl_t *mod_ctrl = NULL;
  /* if receive from sink forward to src's peer or vice versa */
  if (mct_port->direction == MCT_PORT_SINK) {
#if 0
    return mct_list_traverse(module->srcports,
                             port_pproc_send_event_to_peer,
                             (void *)event);
#else
    CDBG("%s: Event on SINK, fwd to module\n", __func__);
    /* pass event to module if it is SINK port */
    //return module->process_event(module, event);
    //return MODULE_PPROC_COMMON_SUBMOD_EVENT_FUNC(module, mct_port, event);
    mod_ctrl = (module_pproc_common_ctrl_t *)module->module_private;
    return mod_ctrl->pproc_event_func(module, mct_port, event);
#endif
  }
  else if (mct_port->direction == MCT_PORT_SRC) {
    CDBG("%s: Event on SRC, fwd to peer\n", __func__);
    return mct_list_traverse(module->sinkports,
                           port_pproc_common_send_event_to_peer,
                           (void *)event);
  } else
    return FALSE;
}

/** port_cpp_event
 *    @port: this port from where the event should go
 *    @event: event object to send upstream or downstream
 *
 *  Return TRUE for successful event processing.
 **/
static boolean port_cpp_event(mct_port_t *port, mct_event_t *event)
{
  boolean                     rc = TRUE;
  module_pproc_ctrl_t         *module_ctrl = NULL;
  mct_list_t                  *s_list = NULL;
  mct_module_t                *module = NULL;

  CDBG("%s:%d event id %d\n", __func__, __LINE__, event->type);

  if (!port || !event) {
    CDBG_ERROR("%s:%d failed port %p event %p\n", __func__, __LINE__, port,
      event);
    return FALSE;
  }
  module = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!module) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }

  /* pass event upstream/downstream */
  return port_cpp_forward_event(module, port, event);
}

/** port_cpp_set_caps
 *    @port: port object which the caps to be set;
 *    @caps: this port's capability.
 *
 **/
static boolean port_cpp_set_caps(mct_port_t *port,
  mct_port_caps_t *caps)
{
  port->caps = *caps;
  return TRUE;
}

/** port_cpp_check_caps_reserve
 *    @port:
 *    @peer_caps:
 *    @stream_info:
 *
 **/
static boolean port_cpp_check_caps_reserve(mct_port_t *port,
  void *peer_caps, void *stream_info)
{
  mct_stream_info_t *cam_stream_info = (mct_stream_info_t *)stream_info;
  mct_port_caps_t *caps = (mct_port_caps_t *)&port->caps;
  module_pproc_common_port_private_t *private_data =
    (module_pproc_common_port_private_t *)port->port_private;
  module_pproc_common_frame_params_t *frame_params = NULL;

  boolean ret = FALSE;

  CDBG("%s:Enter\n", __func__);
  if (!cam_stream_info) {
    CDBG_ERROR("%s:%d failed stream_info %p\n", __func__, __LINE__,
      cam_stream_info);
    return ret;
  }

  CDBG("%s:%d stream streaming mode %d\n", __func__, __LINE__,
    cam_stream_info->streaming_mode);
  CDBG("%s:%d caps streaming type %d\n", __func__, __LINE__,
    private_data->streaming_mode);
  if ((caps->port_caps_type == MCT_PORT_CAPS_FRAME) &&
    (caps->u.frame.format_flag == MCT_PORT_CAP_FORMAT_YCBCR) &&
    (caps->u.frame.size_flag == MCT_PORT_CAP_SIZE_20MB) &&
    (cam_stream_info->streaming_mode == private_data->streaming_mode)) {
    frame_params = malloc(sizeof(module_pproc_common_frame_params_t));
    if (!frame_params) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      return FALSE;
    }
    memset(frame_params, 0, sizeof(module_pproc_common_frame_params_t));
    frame_params->identity = cam_stream_info->identity;
    frame_params->dst_width = cam_stream_info->dim.width;
    frame_params->dst_height = cam_stream_info->dim.height;
    frame_params->dst_stride =
      cam_stream_info->buf_planes.plane_info.mp[0].stride;
    frame_params->dst_scanline =
      cam_stream_info->buf_planes.plane_info.mp[0].scanline;
    frame_params->flip = cam_stream_info->pp_config.flip;

    CDBG("stream type %d id %x w %d h %d st %d sc %d",
      cam_stream_info->stream_type, frame_params->identity,
      frame_params->dst_width, frame_params->dst_height,
      frame_params->dst_stride, frame_params->dst_scanline);
    if (cam_stream_info->fmt == CAM_FORMAT_YUV_420_NV12) {
      frame_params->plane_fmt = PPROC_PLANE_CBCR;
    } else if (cam_stream_info->fmt == CAM_FORMAT_YUV_420_NV21) {
      frame_params->plane_fmt = PPROC_PLANE_CRCB;
    } else if (cam_stream_info->fmt == CAM_FORMAT_YUV_422_NV16) {
      frame_params->plane_fmt = PPROC_PLANE_CBCR422;
    } else if (cam_stream_info->fmt == CAM_FORMAT_YUV_422_NV61) {
      frame_params->plane_fmt = PPROC_PLANE_CRCB422;
    } else {
      CDBG_ERROR("%s: Format not supported\n", __func__);
      return FALSE;
    }
    frame_params->stream_info = cam_stream_info;
    CDBG("%s:%d stream info %p\n", __func__, __LINE__, cam_stream_info);

    module_pproc_common_init_wnr_params(frame_params);

    /* Store output width and height from stream info */
    private_data->frame_params = mct_list_append(private_data->frame_params,
      frame_params, NULL, NULL);
    CDBG("%s:%d output width %d height %d\n", __func__, __LINE__,
      frame_params->dst_width, frame_params->dst_height);
    ret = TRUE;
  }
  CDBG("%s:Exit Ret:%d\n", __func__, ret);
  return ret;
}

  /** port_cpp_check_caps_unreserve
 *    @port: this port object to remove the session/stream;
 *    @identity: session+stream identity.
 *
 * Return FALSE if the identity is not existing.
 **/
static boolean port_cpp_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  module_pproc_common_port_private_t *port_private;
  module_pproc_common_frame_params_t *frame_params;
  mct_list_t *list_node;

  CDBG("%s:%d E\n", __func__, __LINE__);
  if (!port) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }

  port_private =
    (module_pproc_common_port_private_t *)port->port_private;
  list_node = mct_list_find_custom(port_private->frame_params, &identity,
    module_pproc_common_find_identity);
  frame_params =
    (module_pproc_common_frame_params_t *)list_node->data;
  port_private->frame_params = mct_list_remove(port_private->frame_params,
    frame_params);
  free(frame_params);

  CDBG("%s:%d X\n", __func__, __LINE__);
  return TRUE;
}

/** port_cpp_int_link
 *    @port:
 *
 **/
static mct_list_t* port_cpp_int_link(unsigned int identity,
  mct_port_t* port)
{
  /* TODO */
  return NULL;
}

/** port_cpp_ext_link
 *    @identity:
 *    @port:
 *    @peer:
 *
 **/
static boolean port_cpp_ext_link(unsigned int identity,
  mct_port_t* port, mct_port_t *peer)
{
  mct_module_t               *module = NULL;
  module_pproc_common_ctrl_t *module_ctrl = NULL;

  int32_t ret = 0;

  CDBG("%s:%d E, port:%p\n", __func__, __LINE__, port);
  if (!port) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }
  if (!MCT_PORT_PEER(port)) {
    MCT_PORT_PEER(port)  = peer;
  } else { /*the link has already been established*/
  if ((MCT_PORT_PEER(port) != peer) && (port != peer))
    return FALSE;
  }
  MCT_OBJECT_REFCOUNT(MCT_OBJECT_CAST(port))++;

  CDBG("%s:%d\n", __func__, __LINE__);
  return TRUE;
}

/** port_cpp_unlink
 *
 *
 *
 **/
static void port_cpp_unlink(unsigned int identity, mct_port_t *port,
  mct_port_t *peer)
{
  mct_module_t               *p_module = NULL;
  module_pproc_common_ctrl_t *module_ctrl = NULL;
  mct_list_t                 *port_parent = NULL;
  unsigned int               *my_identity;
  mct_list_t                 *identity_holder;
  uint32_t                   info;

  CDBG("%s:%d E, port:%p\n", __func__, __LINE__, port);
  if (!port) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return;
  }
  if ((port->peer != peer) && (port != peer)) {
    CDBG_ERROR("%s:%d peer check failed\n", __func__, __LINE__);
    return;
  }

  identity_holder = mct_list_find_custom(MCT_PORT_CHILDREN(port),
    &identity, port_pproc_common_match_identity);
  if (identity_holder == NULL) {
    CDBG_ERROR("%s:%d identity not found\n", __func__, __LINE__);
    return;
  }

  MCT_OBJECT_REFCOUNT(MCT_OBJECT_CAST(port))--;
  if (!MCT_OBJECT_REFCOUNT(MCT_OBJECT_CAST(port))) {
    MCT_PORT_PEER(port) = NULL;
  }

  CDBG("%s:%d X\n", __func__, __LINE__);
  return;
}

/** port_cpp_init
 *    @port: port object to be initialized
 *
 *  Port initialization, use this function to overwrite
 *  default port methods and install capabilities.
 *
 *  Return TRUE on success.
 **/
boolean port_cpp_init(mct_port_t *port, mct_port_direction_t direction)
{
  boolean rc = TRUE;
  mct_port_caps_t caps;

  CDBG("%s: %d\n", __func__, __LINE__);

  port->direction = direction;
  caps.port_caps_type = MCT_PORT_CAPS_FRAME;
  caps.u.frame.format_flag = MCT_PORT_CAP_FORMAT_YCBCR;
  caps.u.frame.size_flag  = MCT_PORT_CAP_SIZE_20MB;

  mct_port_set_event_func(port, port_cpp_event);
  mct_port_set_int_link_func(port, port_cpp_int_link);
  mct_port_set_ext_link_func(port, port_cpp_ext_link);
  mct_port_set_unlink_func(port, port_cpp_unlink);
  mct_port_set_set_caps_func(port, port_cpp_set_caps);
  mct_port_set_check_caps_reserve_func(port, port_cpp_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port, port_cpp_check_caps_unreserve);

  if (port->set_caps)
    port->set_caps(port, &caps);

  return rc;
}
