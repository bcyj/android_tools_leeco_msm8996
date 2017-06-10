/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "camera_dbg.h"
#include "mct_stream.h"
#include "port_pproc_common.h"
#include "port_pproc.h"
#include "module_pproc.h"
#include "cam_types.h"
#include "mct_port.h"

//#define ENABLE_CPP_LOG

#ifdef ENABLE_CPP_LOG
#undef CDBG
#define CDBG ALOGE
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE
#endif

static boolean port_pproc_create_bypass_link(port_pproc_common_link_t *link,
  uint32_t identity, mct_port_t *port);
static mct_port_t *port_pproc_find_port_attached_with_stream(mct_port_t *port,
  uint32_t *identity);

boolean port_pproc_find_divert_link_by_identity(void *list_data,
  void *user_data)
{
  port_pproc_common_divert_link_t *divert_link =
    (port_pproc_common_divert_link_t *)list_data;
  uint32_t *identity = (uint32_t *)user_data;

  CDBG("%s:Identity: req:%d, list:%d\n", __func__, *identity,
    divert_link->identity);
  if (divert_link->identity == *identity) {
    return TRUE;
  }

  return FALSE;
}

boolean port_pproc_remove_event_list(void *list_data, void *user_data)
{
  pproc_event_link_traverse_obj_t *list_event =
    (pproc_event_link_traverse_obj_t *)list_data;
  free(list_event->event.u.module_event.module_event_data);
  free(list_event);
  return TRUE;
}

boolean port_pproc_find_queue_event(void *list_data, void *user_data)
{
  pproc_event_link_traverse_obj_t *queue_evnt =
    (pproc_event_link_traverse_obj_t *)list_data;
  mct_event_t *req_event = (mct_event_t *)user_data;
  isp_buf_divert_ack_t *req_buf;
  isp_buf_divert_ack_t *queue_buf;

  CDBG("%s:que_iden:%d, req_iden:%d\n", __func__,
    queue_evnt->event.identity, req_event->identity);
  queue_buf = queue_evnt->event.u.module_event.module_event_data;
  req_buf = req_event->u.module_event.module_event_data;
  assert(queue_buf != NULL);
  assert(req_buf != NULL);
  CDBG("%s:que_buf:%d, req_buf:%d\n", __func__,
    queue_buf->buf_idx, req_buf->buf_idx);
  if (queue_buf->buf_idx == req_buf->buf_idx)
    return TRUE;

  return FALSE;
}

/** port_pproc_forward_event:
 *    @module: pproc module pointer
 *    @mct_port: port from which event originates
 *    @event: event data
 *
 *  Return: TRUE for success and FALSE on failure
 *
 **/
static boolean port_pproc_forward_event(mct_module_t *module,
  mct_port_t *mct_port, mct_event_t *event)
{
  pproc_event_t pproc_event;

  if (event->direction == MCT_EVENT_DOWNSTREAM) {
    if (mct_port->direction == MCT_PORT_SINK) {
      CDBG("%s:DOWNSTREAM_SINK\n", __func__);
      pproc_event.type = PPROC_EVENT_PRIVATE;
      pproc_event.u.pproc_event.type = PPROC_PRIV_EVENT_DISPATCH_MCT_EVENT_DWS;
      pproc_event.u.pproc_event.data = event;
      return module_pproc_event_handler_func(
        MCT_MODULE_CAST(MCT_PORT_PARENT(mct_port)->data), mct_port,
        &pproc_event);
    } else {
      CDBG("%s:DOWNSTREAM_SRC\n", __func__);
      return mct_list_traverse(module->srcports,
        port_pproc_common_send_event_to_peer, (void *)event);
    }
  } else if (event->direction == MCT_EVENT_UPSTREAM) {
    if (mct_port->direction == MCT_PORT_SINK) {
      CDBG("%s:UPSTREAM_SINK\n", __func__);
      pproc_event.type = PPROC_EVENT_PRIVATE;
      pproc_event.u.pproc_event.type =
        PPROC_PRIV_EVENT_DISPATCH_MCT_EVENT_UPS;
      pproc_event.u.pproc_event.data = event;
      return module_pproc_event_handler_func(
        MCT_MODULE_CAST(MCT_PORT_PARENT(mct_port)->data), mct_port,
        &pproc_event);
    } else {
      mct_list_t *list_match = NULL;
      mct_port_t *p_sink_port;

      CDBG("%s:UPSTREAM_SRC\n", __func__);
      /* traverse the sinkport */
      p_sink_port = port_pproc_find_port_attached_with_stream(mct_port,
        &event->identity);
      CDBG("%s:%d] UPSTREAM_SRC event_match %p", __func__, __LINE__, p_sink_port);

      if (p_sink_port != NULL)
        return MCT_PORT_EVENT_FUNC(mct_port)(p_sink_port, event);
    }
  }

  return FALSE;
}

/** port_pproc_event
 *    @port: this port from where the event should go
 *    @event: event object to send upstream or downstream
 *
 *  Return TRUE for successful event processing.
 **/
static boolean port_pproc_event(mct_port_t *port, mct_event_t *event)
{
  mct_module_t *module = NULL;

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
  return port_pproc_forward_event(module, port, event);
}

/** port_pproc_set_caps
 *    @port: port object to which the caps to be set;
 *    @caps: this port's capability.
 *
 **/
static boolean port_pproc_set_caps(mct_port_t *port,
  mct_port_caps_t *caps)
{
  port->caps = *caps;
  return TRUE;
}

boolean port_pproc_traverse_port_attached_with_stream(void *list_data,
  void *user_data)
{
  mct_list_t *identity_holder;
  mct_port_t *port = (mct_port_t *)list_data;
  uint32_t *identity = (uint32_t *)user_data;

  identity_holder = mct_list_find_custom(MCT_PORT_CHILDREN(port),
    identity, port_pproc_common_match_identity);
  if (identity_holder != NULL) {
    uint32_t *list_identity = (uint32_t *)identity_holder->data;
    if (*list_identity == *identity) {
      return TRUE;
    }
  }
  return FALSE;
}

mct_port_t *port_pproc_find_port_attached_with_stream(mct_port_t *port,
  uint32_t *identity)
{
  mct_list_t *identity_holder;

  CDBG("%s:%d %p %p", __func__, __LINE__, port, identity);
  identity_holder = mct_list_find_custom(
    MCT_MODULE_CAST(MCT_PORT_PARENT(port)->data)->sinkports,
    identity, port_pproc_traverse_port_attached_with_stream);
  if (identity_holder != NULL) {
    return (mct_port_t *)identity_holder->data;
  }
  return NULL;
}

boolean port_pproc_create_bypass_link(port_pproc_common_link_t *link,
  uint32_t identity, mct_port_t *port)
{
  mct_list_t *list_node;
  port_pproc_common_divert_link_t *divert_link;
  port_pproc_priv_data_t *port_priv;
  mct_port_t *sink_port;

  sink_port = port_pproc_find_port_attached_with_stream(port, &identity);
  if (sink_port == NULL) {
    CDBG_ERROR("%s: Caps reserve out of order\n", __func__);
    return FALSE;
  }
  port_priv = (port_pproc_priv_data_t *)sink_port->port_private;

  /* Search for existing divert link for the identity */
  list_node = mct_list_find_custom(port_priv->links_by_identity, &identity,
    port_pproc_find_divert_link_by_identity);
  CDBG("%s:list_node:%p\n", __func__, list_node);
  if (!list_node) {
    CDBG_ERROR("%s:%d Caps reserve out of order.\n", __func__, __LINE__);
    return FALSE;
  }

  divert_link = (port_pproc_common_divert_link_t *)list_node->data;
  list_node = mct_list_append(divert_link->tplgy_lnks, link, NULL, NULL);
  if (!list_node) {
    CDBG_ERROR("%s:%d mct link append failed.\n", __func__, __LINE__);
    return FALSE;
  }

  divert_link->tplgy_lnks = list_node;
  divert_link->tplgy_lnks_cnt++;

  return TRUE;
}

/** port_pproc_check_caps_reserve_on_sink_port:
 *    @port:        mct_port on which caps is being reserved
 *    @peer:        mct_port_caps_t of peer
 *    @stream_info: mct_stream_info_t associated with stream
 *                  being reseved.
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function reserves the caps for pproc port. Stream
 *  info and peer caps info are passed to help the caps reserve.
 **/
static boolean port_pproc_check_caps_reserve(mct_port_t *port,
  void *peer, void *stream_info)
{
  mct_stream_info_t                   *cam_stream_info =
    (mct_stream_info_t *)stream_info;
  mct_port_caps_t                     *peer_caps = (mct_port_caps_t *)peer;
  mct_port_caps_t                     *caps;
  boolean                              ret = FALSE;
  port_pproc_common_link_create_info_t pproc_tplgy_info;
  mct_list_t                          *list_node;
  pproc_event_t                        pproc_event;
  port_pproc_common_divert_link_t     *divert_link, *free_this_if_error;
  boolean                              new_divert_link = TRUE;
  port_pproc_priv_data_t              *port_priv = NULL;

  CDBG("%s:%d Enter\n", __func__, __LINE__);
  if (!port || !stream_info) {
    CDBG_ERROR("%s:%d NULL port or stream_info.\n", __func__, __LINE__);
    goto CAPS_RESERVE_ERROR0;
  }

  caps = (mct_port_caps_t *)&port->caps;

  if (port->direction == MCT_PORT_SINK) {
    if (!peer_caps) {
      CDBG_ERROR("%s:%d Invalid port.\n", __func__, __LINE__);
      goto CAPS_RESERVE_ERROR0;
    }

    if (peer_caps->port_caps_type != MCT_PORT_CAPS_FRAME) {
      CDBG("%s:%d Caps Type:%d not supported.\n", __func__, __LINE__,
        peer_caps->port_caps_type);
      goto CAPS_RESERVE_ERROR0;
    }
  }

  if ((caps->port_caps_type != MCT_PORT_CAPS_FRAME) ||
    (caps->u.frame.format_flag != MCT_PORT_CAP_FORMAT_YCBCR) ||
    (caps->u.frame.size_flag != MCT_PORT_CAP_SIZE_20MB)) {
    CDBG_ERROR("%s:%d port_pproc caps is wrong.\n", __func__, __LINE__);
    goto CAPS_RESERVE_ERROR0;
  }

  port_priv = (port_pproc_priv_data_t *)port->port_private;
  if (!port_priv) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    goto CAPS_RESERVE_ERROR0;
  }

  CDBG("%s:%d port streaming %d stream mode %d\n", __func__, __LINE__,
    port_priv->streaming_mode, cam_stream_info->streaming_mode);

  if (port_priv->streaming_mode != cam_stream_info->streaming_mode) {
    CDBG("%s:%d port streaming %d != cam_stream_info mode %d\n", __func__,
      __LINE__, port_priv->streaming_mode, cam_stream_info->streaming_mode);
    return FALSE;
  }
  /* Build topology based on the features in stream info. Caps reserve on
     selected sub modules and establish link if multiple submodules are
     needed.*/
  pproc_tplgy_info.peer = peer;
  pproc_tplgy_info.stream_info = stream_info;
  pproc_tplgy_info.link = (port_pproc_common_link_t *)malloc(sizeof(
    port_pproc_common_link_t));
  if (!pproc_tplgy_info.link) {
    CDBG_ERROR("%s:%d link malloc failed\n", __func__, __LINE__);
    goto CAPS_RESERVE_ERROR0;
  }

  memset(pproc_tplgy_info.link, 0, sizeof(port_pproc_common_link_t));
  /* Create a direct link for now if source port (img module) is connected */
  if (port->direction == MCT_PORT_SRC) {
    pproc_tplgy_info.link->identity = cam_stream_info->identity;
    pproc_tplgy_info.link->sink_port = port;
    pproc_tplgy_info.link->src_port = NULL;
    return port_pproc_create_bypass_link(pproc_tplgy_info.link,
      cam_stream_info->identity, port);
  } else {
    pproc_event.type = PPROC_EVENT_TYPE_PRIVATE;
    pproc_event.u.pproc_event.type = PPROC_PRIV_EVENT_CREATE_TOPOLOGY;
    pproc_event.u.pproc_event.data = (void *)&pproc_tplgy_info;
    ret = module_pproc_event_handler_func(MCT_MODULE_CAST(
      MCT_PORT_PARENT(port)->data), port, &pproc_event);
    if (ret == FALSE) {
      CDBG_ERROR("%s:%d Create link failed.\n", __func__, __LINE__);
      goto CAPS_RESERVE_ERROR1;
    }
  }

  /* Search for existing divert link for the identity */
  list_node = mct_list_find_custom(port_priv->links_by_identity,
    &cam_stream_info->identity, port_pproc_find_divert_link_by_identity);
  CDBG("%s:list_node:%p\n", __func__, list_node);
  if (!list_node) {
    /* Create the divert link object */
    divert_link = (port_pproc_common_divert_link_t *)malloc(
       sizeof(port_pproc_common_divert_link_t));
    if (!divert_link) {
      CDBG_ERROR("%s:%d mct link append failed.\n", __func__, __LINE__);
      goto CAPS_RESERVE_ERROR2;
    }
    memset(divert_link, 0, sizeof(port_pproc_common_divert_link_t));
    CDBG("%s:Traverse:%p\n", __func__, divert_link->out_evnt_list);
    free_this_if_error = divert_link;
  } else {
    divert_link = (port_pproc_common_divert_link_t *)list_node->data;
    free_this_if_error = NULL;
    new_divert_link = FALSE;
  }

  list_node = mct_list_append(divert_link->tplgy_lnks, pproc_tplgy_info.link,
    NULL, NULL);
  if (!list_node) {
    CDBG_ERROR("%s:%d mct link append failed.\n", __func__, __LINE__);
    if (free_this_if_error)
        free(free_this_if_error);
    goto CAPS_RESERVE_ERROR2;
  }

  divert_link->tplgy_lnks = list_node;
  divert_link->tplgy_lnks_cnt++;
  divert_link->stream_info = (void *)cam_stream_info;
  divert_link->identity = cam_stream_info->identity;
  CDBG("%s:%d caps reserve identity %d\n", __func__, __LINE__,
    divert_link->identity);

  if (new_divert_link == TRUE) {
    /* Append the link object in internal peers list */
    list_node = mct_list_append(port_priv->links_by_identity,
      divert_link, NULL, NULL);
    if (!list_node) {
      CDBG_ERROR("%s:%d mct link append failed.\n", __func__, __LINE__);
      free(free_this_if_error);
      goto CAPS_RESERVE_ERROR2;
    }
    port_priv->links_by_identity = list_node;
  }
  CDBG("%s:%d Exit.\n", __func__, __LINE__);

  return TRUE;

CAPS_RESERVE_ERROR2:
  if (port->direction == MCT_PORT_SINK) {
    pproc_event.u.pproc_event.type = PPROC_PRIV_EVENT_DELETE_TOPOLOGY;
    module_pproc_event_handler_func(MCT_MODULE_CAST(
      MCT_PORT_PARENT(port)->data), port, &pproc_event);
  }
CAPS_RESERVE_ERROR1:
  free(pproc_tplgy_info.link);
CAPS_RESERVE_ERROR0:
  return FALSE;
}

/** port_pproc_check_caps_unreserve:
 *    @port:     mct_port on which caps is being unreserved
 *    @identity: sessionid/streamid which is to be unreserved.
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function unreserves the caps for pproc port.
 **/
static boolean port_pproc_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  boolean                     rc = FALSE;
  port_pproc_common_divert_link_t *divert_link;
  port_pproc_priv_data_t *port_priv = NULL;

  mct_list_t *list_node;
  pproc_event_t pproc_event;
  port_pproc_common_link_create_info_t pproc_tplgy_info;

  CDBG("%s:identity=%x\n", __func__, identity);
  if (!port) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }

  if (port) {
    port_priv = (port_pproc_priv_data_t *)port->port_private;
  }
  else
  {
    CDBG_ERROR("%s:port is NULL\n", __func__);
    return FALSE;
  }

  if (port->direction == MCT_PORT_SRC)
    return TRUE;

  /* Search for existing divert link for the identity */
  list_node = mct_list_find_custom(port_priv->links_by_identity, &identity,
    port_pproc_find_divert_link_by_identity);
  if (!list_node) {
    CDBG_ERROR("%s:%d Identity match failed\n", __func__, __LINE__);
    return FALSE;
  }
  divert_link = (port_pproc_common_divert_link_t *)list_node->data;

  pproc_event.type = PPROC_EVENT_TYPE_PRIVATE;
  pproc_event.u.pproc_event.type = PPROC_PRIV_EVENT_DELETE_TOPOLOGY;
  pproc_tplgy_info.stream_info = divert_link->stream_info;
  for (; divert_link->tplgy_lnks_cnt > 0; ) {
    /* TODO: delete topology link */
    pproc_tplgy_info.link = divert_link->tplgy_lnks->data;
    pproc_event.u.pproc_event.data = (void *)&pproc_tplgy_info;
    module_pproc_event_handler_func(MCT_MODULE_CAST(MCT_PORT_PARENT(port)->data), port, &pproc_event);
    list_node = divert_link->tplgy_lnks;
    divert_link->tplgy_lnks = mct_list_remove(divert_link->tplgy_lnks,
      pproc_tplgy_info.link);
    free(pproc_tplgy_info.link);
    divert_link->tplgy_lnks_cnt--;
  }

  mct_list_free_all(divert_link->out_evnt_list, port_pproc_remove_event_list);

  list_node = mct_list_remove(port_priv->links_by_identity, divert_link);
  port_priv->links_by_identity = list_node;
  if (!list_node) {
    /* All streams attached to the port is unreserved so free the queue */
    mct_list_free_all(port_priv->out_evnt_list, port_pproc_remove_event_list);
    CDBG("%s:path_2,port:out_list:%p\n", __func__, port_priv->out_evnt_list);
  }
  free(divert_link);
  CDBG("%s:%d Exit\n", __func__, __LINE__);

  return TRUE;
}

/** port_pproc_int_link
 *    @port:
 *
 **/
static mct_list_t* port_pproc_int_link(unsigned int identity,
  mct_port_t *port)
{
  /* TODO */
  return NULL;
}

/** port_pproc_ext_link
 *    @identity:
 *    @port:
 *    @peer:
 *
 **/
static boolean port_pproc_ext_link(unsigned int identity,
  mct_port_t* port, mct_port_t *peer)
{
  mct_module_t               *module = NULL;
  module_pproc_common_ctrl_t *module_ctrl = NULL;
  boolean rc = FALSE;

  CDBG("%s:%d\n", __func__, __LINE__);
  if (!port) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }

  if (port->direction == MCT_PORT_SINK) {
    rc = module_pproc_link_submod(MCT_MODULE_CAST(
       (MCT_PORT_PARENT(port))->data), identity, port);
    if (rc == FALSE) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      return FALSE;
    }
  }
  if (!MCT_PORT_PEER(port)) {
    MCT_PORT_PEER(port) = peer;
    CDBG("%s:Sucessfully set peer:%p\n", __func__, peer);
  } else { /*the link has already been established*/
    CDBG("%s:Redundant link on :%p\n", __func__, port);
    if ((MCT_PORT_PEER(port) != peer)) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      return FALSE;
    }
  }
  MCT_OBJECT_REFCOUNT(MCT_OBJECT_CAST(port))++;

  CDBG("%s:%d\n", __func__, __LINE__);
  return TRUE;
}

/** port_pproc_unlink
 *
 *
 *
 **/
static void port_pproc_unlink(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  mct_module_t               *p_module = NULL;
  module_pproc_common_ctrl_t *module_ctrl = NULL;
  mct_list_t                 *port_parent = NULL;
  unsigned int               *my_identity;
  mct_list_t                 *identity_holder;
  uint32_t                   info;
  boolean                    rc = FALSE;

  CDBG("%s:%d E\n", __func__, __LINE__);
  if (!port) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return;
  }
  if (port->peer != peer) {
    CDBG_ERROR("%s:%d peer check failed\n", __func__, __LINE__);
    return;
  }

  if (port->direction == MCT_PORT_SINK) {
    rc = module_pproc_unlink_submod(MCT_MODULE_CAST(
       (MCT_PORT_PARENT(port))->data), identity, port);
    if (rc == FALSE) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      return;
    }
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

/** port_pproc_init
 *    @port: port object to be initialized
 *
 *  Port initialization, use this function to overwrite
 *  default port methods and install capabilities.
 *
 *  Return TRUE on success.
 **/
boolean port_pproc_init(mct_port_t *port, mct_port_direction_t direction,
  cam_streaming_mode_t mode)
{
  boolean rc = TRUE;
  mct_port_caps_t caps;
  port_pproc_priv_data_t *port_priv;

  port->direction = direction;
  caps.port_caps_type = MCT_PORT_CAPS_FRAME;
  caps.u.frame.format_flag = MCT_PORT_CAP_FORMAT_YCBCR;
  caps.u.frame.size_flag  = MCT_PORT_CAP_SIZE_20MB;

  mct_port_set_event_func(port, port_pproc_event);
  mct_port_set_int_link_func(port, port_pproc_int_link);
  mct_port_set_ext_link_func(port, port_pproc_ext_link);
  mct_port_set_unlink_func(port, port_pproc_unlink);
  mct_port_set_set_caps_func(port, port_pproc_set_caps);
  mct_port_set_check_caps_reserve_func(port, port_pproc_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port, port_pproc_check_caps_unreserve);

  if (port->set_caps)
    port->set_caps(port, &caps);

  port_priv = (port_pproc_priv_data_t *)malloc(
    sizeof(port_pproc_priv_data_t));
  if (!port_priv) {
    CDBG_ERROR("%s:%d malloc failed\n", __func__, __LINE__);
    return FALSE;
  }

  memset(port_priv, 0, sizeof(port_pproc_priv_data_t));
  port_priv->streaming_mode = mode;
  pthread_mutex_init(&port_priv->mutex, NULL);
  port->port_private = port_priv;
  return rc;
}
