/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
***************************************************************************/

#include <linux/media.h>
#include "mct_module.h"
#include "module_imglib.h"
#include "module_imglib_common.h"
#include "mct_stream.h"
#include "mct_port.h"

/* forward declare tuning APIs */
extern void mct_tuning_notify_preview_frame(isp_buf_divert_t *p_buf_divert,
  mct_stream_t *stream);


/* Macro to get attached topologies per port */
#define PORT_ATTACHED_TOPO(a) \
  (((imglib_port_data_t *) \
   (a)->port_private)->internal_topo->topo_attached) \

/* Macro to get topology list */
#define GET_PORT_TOPO_LIST(a, p) \
  ((mct_list_t *)((imglib_port_data_t *) \
   (a)->port_private)->internal_topo->topo_list[(p)]) \

/** imglib_port_events_holder_t
 *   @buf_divert_data: Data used for buffer divert event
 *     @use_int_buffers: TRUE if int buffers need to be used
 *     @num_planes: Number of planes inside the buffers
 *     @addr: Array of addr to img planes
 *     @size: Array of sizes for buffer planes
 *   @pending_ack: Array of module events which are used
 *     to mask pending ack.
 *
 *   imglib_port_events_holder_t
 **/
typedef struct {
  struct {
    boolean use_int_buffers;
    uint32_t num_planes;
    void *addr[VIDEO_MAX_PLANES];
    uint32_t size[VIDEO_MAX_PLANES];
  } buf_divert_data;
  uint32_t pending_ack[MCT_EVENT_MODULE_MAX];
} imglib_port_events_holder_t;

/** imglib_port_data_t
 *   @static_p: Port is static TRUE/FALSE
 *   @topo_orphan: Topology have no parent
 *   @sessionid: Port session id
 *   @reserved_identity: Reserved identity
 *   @internal_topo: Pointer to internal topology used for this port
 *   @mirror_port: Array of pointers to mirrored ports
 *   @port_events: Array of port events holders passed to parallel topologies
 *
 *   imglib port private data
 **/
typedef struct {
  boolean static_p;
  boolean topo_orphan;
  unsigned int sessionid;
  unsigned int reserved_identity;
  unsigned int num_mirror_ports;
  module_imglib_topo_holder_t *internal_topo;
  mct_port_t *mirror_port[MODULE_IMGLIB_MAX_PAR_TOPO];
  imglib_port_events_holder_t port_events[MODULE_IMGLIB_PORT_MAX_HOLD_EVENTS];
} imglib_port_data_t;

/** imglib_port_data_t
 *   @index: Index of the mirror port.
 *   @mct_port_t: Pointer to imglib port parent of this mirror port.
 *
 *   imglib_mirror_port_data_t
 **/
typedef struct {
  uint32_t index;
  mct_port_t *p_imglib_port;
} imglib_mirror_port_data_t;

/** imglib_link_modules_data_t
 *   @stream_info: Stream info
 *   @ret_code: Ret code from the calling function
 *
 *   Imagelib module structure this function is used as
 *     argument to list operate nodes on linking internal
 *     topology
 **/
typedef struct {
  mct_stream_info_t *stream_info;
  boolean ret_code;
} imglib_link_modules_data_t;

/** imglib_unlink_modules_data_t
 *   @identity: Stream identity
 *   @ret_code: Ret code from the calling function
 *
 *   Imagelib module structure this function is used as
 *     argument to list operate nodes on unlinking internal
 *     topology
 **/
typedef struct {
  unsigned int identity;
  boolean ret_code;
} imglib_unlink_modules_data_t;

/** imglib_reserve_port_data_t
 *   @stream_info: Pointer to stream info
 *   @port_caps: Pointer to peer port capabilities
 *
 *   imglib port private data
 **/
typedef struct {
  mct_stream_info_t *stream_info;
  mct_port_caps_t   *port_caps;
} imglib_reserve_port_data_t;

/** imglib_set_mod_type_data_t
 *   @imglib_mod_type: General imglib module type
 *   @identity: Identity
 *   @p_last_mod: Result last module of topology
 *   Strucutre passed to operate nodes function
 **/
typedef struct {
  mct_module_type_t imglib_mod_type;
  unsigned int identity;
  mct_module_t *p_last_mod;
} imglib_set_mod_type_data_t;

/**
 * Function: module_imglib_port_unparent_modules
 *
 * Description: Function to unset parent(stream in our case) of
 *   internal topology modules
 *
 * Arguments:
 *   @data1: Pointer to the module child
 *   @user_data: Pointer to the parent stream object
 *
 * Return values:
 *   TRUE/FALSE
 **/
static boolean module_imglib_port_unparent_modules(void *data, void *user_data)
{
  mct_module_t *mod =  (mct_module_t *)data;
  mct_stream_t *stream = (mct_stream_t *)user_data;

  if (!mod || !stream)
    return FALSE;

  MCT_OBJECT_LOCK(mod);

  MCT_OBJECT_PARENT(mod) = mct_list_remove(MCT_OBJECT_PARENT(mod), stream);
  MCT_OBJECT_NUM_PARENTS(mod) -= 1;

  MCT_OBJECT_UNLOCK(mod);

  return TRUE;
}

/**
 * Function: module_imglib_port_parent_modules
 *
 * Description: Function to set parent(stream in our case) of
 *   internal topology modules
 *
 * Arguments:
 *   @data1: Pointer to the module child
 *   @user_data: Pointer to the parent stream object
 *
 * Return values:
 *   TRUE/FALSE
 **/
static boolean module_imglib_port_parent_modules(void *data, void *user_data)
{
  mct_module_t *mod =  (mct_module_t *)data;
  mct_stream_t *stream = (mct_stream_t *)user_data;
  mct_list_t *temp_parent;

  if (!mod || !stream)
    return FALSE;

  /* set parent */
  temp_parent = mct_list_append(MCT_MODULE_PARENT(mod), stream, NULL, NULL);
  if (!temp_parent) {
    return FALSE;
  }
  MCT_OBJECT_LOCK(mod);

  MCT_OBJECT_PARENT(mod) = temp_parent;
  MCT_OBJECT_NUM_PARENTS(mod) += 1;

  MCT_OBJECT_UNLOCK(mod);

  return TRUE;
}

/**
 * Function: module_imglib_port_set_mod_type
 *
 * Description: Function used in list operate nodes to set module types
 *  It will set module type only to source modules last module will be
 *  set outside this function so we do not which one is it
 *  First module will be set depending on imglib module type
 *  all next module will be set as indexable.
 *
 * Arguments:
 *   @data1: source module
 *   @data2: sink module
 *   @user_data: link_data private data used for returning the result
 *
 * Return values:
 *   none
 *
 * Note: Result will be stored in link_data
 **/
static void module_imglib_port_set_mod_type(void *data1, void *data2,
    const void *user_data)
{
  mct_module_t *p_mod =  (mct_module_t *)data1;
  imglib_set_mod_type_data_t *p_type_data =
      ( imglib_set_mod_type_data_t *)user_data;

  if (!data1 || !data2 || !user_data)
    return;

  /* If imglib module type is source set first module to source as well */
  if (MCT_MODULE_FLAG_SOURCE & p_type_data->imglib_mod_type ||
      MCT_MODULE_FLAG_PEERLESS & p_type_data->imglib_mod_type) {
    /* Check if this is first module in the list */
    if (!p_type_data->p_last_mod) {
      p_mod->set_mod(p_mod, MCT_MODULE_FLAG_SOURCE,  p_type_data->identity);
      goto out;
    }
  }

  p_mod->set_mod(p_mod, MCT_MODULE_FLAG_INDEXABLE,  p_type_data->identity);
out:
  /* The last module we will set outside this traversing */
  p_type_data->p_last_mod = data2;
  return;
}

/**
 * Function: module_imglib_port_link_modules
 *
 * Description: Function used in list operate nodes to
 *  link the modules in internal topology
 *
 * Arguments:
 *   @data1: source module
 *   @data2: sink module
 *   @user_data: link_data private data used for returning the result
 *
 * Return values:
 *   none
 *
 * Note: Result will be stored in link_data
 **/
static void module_imglib_port_link_modules(void *data1, void *data2,
  const void *user_data)
{
  mct_module_t *mod1 =  (mct_module_t *)data1;
  mct_module_t *mod2 =  (mct_module_t *)data2;
  imglib_link_modules_data_t *link_data =
      (imglib_link_modules_data_t *)user_data;
  boolean ret;

  if (!mod1 || !mod2 || !link_data)
    return;
  if (link_data->ret_code == FALSE)
    return;

  ret = mct_module_link((void *)link_data->stream_info, mod1, mod2);
  if (ret == FALSE) {
    IDBG_ERROR("%s: Can not connect %s -> %s", __func__,
      mod1->object.name, mod2->object.name);
    link_data->ret_code = FALSE;
  }
  return;
}

/**
 * Function: module_imglib_port_unlink_modules
 *
 * Description: Function used in list operate nodes to
 *  unlink the modules in internal topology
 *
 * Arguments:
 *   @data1: source module
 *   @data2: sink module
 *   @user_data: link_data private data used for returning the result
 *
 * Return values:
 *   none
 *
 * Note: Result will be stored in link_data
 *
 **/
static void module_imglib_port_unlink_modules(void *data1, void *data2,
  const void *user_data)
{
  mct_module_t *mod1 =  (mct_module_t *)data1;
  mct_module_t *mod2 =  (mct_module_t *)data2;
  imglib_unlink_modules_data_t *link_data =
    (imglib_unlink_modules_data_t *)user_data;

  if (!mod1 || !mod2 || !link_data)
    return;

  mct_module_unlink(link_data->identity, mod1, mod2);

  return;
}

/**
 * Function: module_imglib_port_remove_mod_types
 *
 * Description: Function used in list traverse to
 *  remove module type from internal topology modules.
 *
 * Arguments:
 *   @data: mct module pointer.
 *   @u_data: Identity.
 *
 * Return values:
 *   TRUE on success.
 **/
static boolean module_imglib_port_remove_mod_types(void *data, void *u_data)
{
  mct_module_t *p_module =  (mct_module_t *)data;
  unsigned int *p_identity = (unsigned int *)u_data;

  if (!p_module || !p_identity)
    return FALSE;

  mct_module_remove_type(p_module, *p_identity);

  IDBG_MED("%s:%d] Remove type module %s identity %d",
    __func__, __LINE__, MCT_OBJECT_NAME(p_module), *p_identity);

  return TRUE;
}

/**
 * Function: module_imglib_reserve_compatible_port
 *
 * Description: Function used in list find custom to
 *  find and reserve available port on internal topology module,
 *
 * Arguments:
 *   @data1: Port
 *   @data2: Stream info
 *
 * Return values:
 *   TRUE/FALSE
 *
 **/
static boolean module_imglib_reserve_compatible_port(void *data1, void *data2)
{
  mct_port_t        *port = (mct_port_t *)data1;
  imglib_reserve_port_data_t *reserve_data =
      (imglib_reserve_port_data_t *)data2;
  void *temp_caps;

  if (!port || !reserve_data)
    return FALSE;
  if (!reserve_data->stream_info || !reserve_data->port_caps)
    return FALSE;

  /* Taken from module link functions maybe need to change */
  temp_caps = reserve_data->port_caps;
  if (MCT_PORT_CAPS_OPAQUE == reserve_data->port_caps->port_caps_type) {
    temp_caps = reserve_data->port_caps->u.data;
  }

  return port->check_caps_reserve(port, temp_caps, reserve_data->stream_info);
}

/**
 * Function: module_imglib_check_linked_port_identity
 *
 * Description: Function used in list find custom to
 *  find and reserved port with given identity
 *
 * Arguments:
 *   @data1: Port
 *   @data2: Stream info
 *
 * Return values:
 *   TRUE/FALSE
 *
 **/
static boolean module_imglib_check_linked_port_identity(void *data1,
  void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  unsigned int *identity  = (unsigned int *) data2;
  imglib_port_data_t *port_data;

  if (!(port && port->port_private)  || !identity)
    return FALSE;

  port_data = (imglib_port_data_t *)port->port_private;

  return (port_data->reserved_identity == *identity);
}

/**
 * Function: module_imglib_check_dynamic_port_sessionid
 *
 * Description: Function used in list find custom to
 *  find dynamic port with session id
 *
 * Arguments:
 *   @data1: Pointer to Port
 *   @data2: Pointer to Session id
 *
 * Return values:
 *   TRUE/FALSE
 *
 **/
static boolean module_imglib_check_dynamic_port_sessionid(void *data1,
  void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  unsigned int *sessionid  = (unsigned int *) data2;
  imglib_port_data_t *port_data;

  if (!(port && port->port_private)  || !sessionid)
    return FALSE;

  port_data = (imglib_port_data_t *)port->port_private;

  /* Skip static ports */
  if (TRUE == port_data->static_p)
    return FALSE;

  return (port_data->sessionid == *sessionid);
}

/**
 * Function: module_imglib_sent_downstream_event
 *
 * Description: Function for sent downstream events used in
 *   mct_list_traverse.
 *   Event will be sent only on source port with same identity.
 *
 * Arguments:
 *   @data1: Pointer to mct port
 *   @user_data: Pointer to the event
 *
 * Return values:
 *   TRUE/FALSE
 **/
static boolean module_imglib_sent_downstream_event(void *data, void *user_data)
{
  mct_port_t *port =  (mct_port_t *)data;
  mct_event_t *event = (mct_event_t *)user_data;
  boolean ret = TRUE;

  if (!port || !event)
    return FALSE;

  /* Downstream events are sent only on source ports */
  if (MCT_PORT_SRC == port->direction) {
    imglib_port_data_t *port_data;
    /* Sent downstream events only to ports with same identity */
    port_data = (imglib_port_data_t *) port->port_private;
    if (port_data && (event->identity == port_data->reserved_identity)) {
        ret = mct_port_send_event_to_peer(port, event);
    }
  }

  return ret;
}

/**
 * Function: module_imglib_broadcast_event_downstream
 *
 * Description: Broadcast downstream event to all ports
 *   with same identity
 *
 * Arguments:
 *   @port: mct port pointer
 *   @event: event pointer to broadcast
 *
 * Return values:
 *   TRUE/FALSE
 *
 **/
static boolean module_imglib_broadcast_event_downstream(mct_port_t *port,
    mct_event_t *event)
{
  mct_module_t *module;
  boolean ret;

  if (!port || !event) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }
  module = MCT_MODULE_CAST((MCT_PORT_PARENT(port)->data));

  ret = TRUE;
  if (module->srcports) {
    ret = mct_list_traverse(module->srcports,
      module_imglib_sent_downstream_event, event);
  }

  return ret;
}

/**
 * Function: module_imglib_sent_event_to_mirrors
 *
 * Description: Sent event to all mirror ports
 *
 * Arguments:
 *   @port: mct port pointer
 *   @event: event pointer to broadcast
 *
 * Return values:
 *   TRUE/FALSE
 *
 **/
static boolean module_imglib_sent_event_to_mirrors(mct_port_t *port,
    mct_event_t *event)
{
  imglib_port_data_t *port_data;
  mct_port_t *mirror_port;
  boolean ret = FALSE;
  uint32_t i;

  if (!(port && port->port_private) || !event) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  port_data = (imglib_port_data_t *)port->port_private;
  for (i = 0; i < PORT_ATTACHED_TOPO(port); i++) {
    mirror_port = ((imglib_port_data_t *)port->port_private)->mirror_port[i];
    if (mirror_port->peer && mirror_port->peer->event_func) {
      ret = MCT_PORT_PEER(mirror_port)->event_func(mirror_port->peer, event);
      if (FALSE == ret)
        break;
    }
  }

  return ret;
}
/**
 * Function: module_imglib_get_topo_port
 *
 * Description: This function will reserve and
 *   return topology port which imglib module can connect.
 *
 * Arguments:
 *   @port: mct port pointer
 *   @stream_info: Stream info
 *   @topo_index: Internal topology index
 *
 * Return values:
 *   NULL/mct_port_t *
 *
 **/
static mct_port_t *module_imglib_get_topo_port(mct_port_t *port,
  mct_stream_info_t *stream_info, uint32_t topo_index)
{
  mct_list_t *topo_list;
  imglib_reserve_port_data_t reserve_data;
  mct_module_t *module = NULL;
  mct_list_t *mod_ports = NULL;
  mct_list_t *temp_list = NULL;
  mct_port_t *p_topo_port = NULL;
  imglib_port_data_t *p_port_data = NULL;

  if (!port || !stream_info) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    goto out;
  }

  topo_list = GET_PORT_TOPO_LIST(port, topo_index);
  if (!topo_list) {
    IDBG_ERROR("%s:%d Topology list is missing %d", __func__, __LINE__, topo_index);
    goto out;
  }

  if (MCT_PORT_IS_SINK(port)) {
    module =  (mct_module_t *)topo_list->data;
    if (module)
      mod_ports = MCT_MODULE_SINKPORTS(module);
  } else {
    /* Get latest module from the topology list to connect the src port*/
    do {
      temp_list = mct_list_find_custom(topo_list, NULL,
        module_imglib_get_next_from_list);
      if (temp_list)
        module = (mct_module_t *)temp_list->data;
    } while (temp_list);
    if (module)
      mod_ports = MCT_MODULE_SRCPORTS(module);
  }
  if (NULL == module) {
    IDBG_ERROR("%s:%d module is missing", __func__, __LINE__);
    goto out;
  }

  /* If there are ports in module try to reserve one of them first */
  if (mod_ports) {
    /* Find sink port in the first module in topology  */
    reserve_data.port_caps = &port->caps;
    reserve_data.stream_info = stream_info;
    temp_list = mct_list_find_custom(mod_ports, &reserve_data,
      module_imglib_reserve_compatible_port);
    p_topo_port = temp_list ? (mct_port_t *)temp_list->data : NULL;
  }

  /* If we can not reserve port, request new one */
  if (NULL == p_topo_port) {
    void *temp_caps = &port->caps;
    /* Taken from module link functions maybe need to change */
    if (MCT_PORT_CAPS_OPAQUE == port->caps.port_caps_type) {
      temp_caps = port->caps.u.data;
    }
    p_topo_port = module->request_new_port(stream_info,
        port->direction, module, temp_caps);
  }

out:
  return p_topo_port;
}

/**
 * Function: module_imglib_put_topo_port
 *
 * Description: This function will put topology port,
 *   first unlink and then unreserve topology port
 *
 * Arguments:
 *   @port: mct port pointer
 *   @identity: Stream identity
 *   @topo_index: Internal topology index
 *
 * Return values:
 *   TRUE/FALSE
 *
 **/
static boolean module_imglib_put_topo_port(mct_port_t *port,
  unsigned int identity, uint32_t topo_index)
{

  imglib_port_data_t *port_data;
  mct_port_t *internal_port;
  mct_port_t *mirror_port;

  if (!(port && port->port_private)) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  port_data = (imglib_port_data_t *)port->port_private;
  if (0 == port_data->reserved_identity) {
    IDBG_ERROR("%s:%d Port is not reserved", __func__, __LINE__);
    return FALSE;
  }

  mirror_port = (mct_port_t *)port_data->mirror_port[topo_index];
  internal_port = (mct_port_t *)mirror_port->peer;
  if (!internal_port) {
    IDBG_ERROR("%s:%d Mirror port not connected \n", __func__, __LINE__);
    return FALSE;
  }

  mct_port_destroy_link(identity, mirror_port, internal_port);
  internal_port->check_caps_unreserve(internal_port, identity);

  /* mct_port_establish_link will set identities as port children but
   * mct_port_destroy_link will not remove, MCT will remove them
   * in upper layers. Remove this code when MCT function is fixed */
  mct_port_remove_child(identity, mirror_port);
  mct_port_remove_child(identity, internal_port);

  return TRUE;
}

/**
 * Function: module_imglib_link_topo_modules
 *
 * Description: This function will link the modules in internal
 *   topology
 *
 * Arguments:
 *   @module: Pointer to imglib module
 *   @port: mct port pointer
 *   @stream_info: Stream info
 *   @topo_index: Internal topology index
 *
 * Return values:
 *   TRUE/FALSE
 *
 **/
static boolean module_imglib_link_topo_modules(mct_module_t *module,
    mct_port_t *port, mct_stream_info_t *stream_info, uint32_t topo_index)
{
  mct_list_t *topo_list;
  imglib_link_modules_data_t link_data;
  imglib_set_mod_type_data_t mod_type_data;
  boolean ret;

  if (!port || !stream_info) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  /* Get internal topology per stream type */
  topo_list = GET_PORT_TOPO_LIST(port, topo_index);
  if (!topo_list) {
    IDBG_ERROR("%s:%d] Not available topology for this stream",
        __func__, __LINE__);
    ret = FALSE;
    goto out;
  }

  /* Get module type */
  mod_type_data.imglib_mod_type =
      mct_module_find_type(module, stream_info->identity);
  if (MCT_MODULE_FLAG_INVALID == mod_type_data.imglib_mod_type) {
    IDBG_ERROR("%s:%d] Module type not set", __func__, __LINE__);
    ret = FALSE;
    goto out;
  }

  /* Initialize last module will be set after */
  mod_type_data.p_last_mod = NULL;
  mod_type_data.identity = stream_info->identity;
  mct_list_operate_nodes(topo_list, module_imglib_port_set_mod_type,
      &mod_type_data);

  /* Set last module type */
  if (NULL == mod_type_data.p_last_mod) {
    mct_module_t *p_top_module = (mct_module_t *)topo_list->data;
    p_top_module->set_mod(p_top_module, mod_type_data.imglib_mod_type,
        stream_info->identity);
  } else {
    mct_module_t *p_top_module = mod_type_data.p_last_mod;
    /* If there are more modules in the topology and our module is source,
     * set last module to be sink, afterwards on reserve source port we
     * will change it to indexable */
    if (MCT_MODULE_FLAG_SOURCE == mod_type_data.imglib_mod_type){
      p_top_module->set_mod(p_top_module, MCT_MODULE_FLAG_SINK,
          stream_info->identity);
    } else {
      p_top_module->set_mod(p_top_module, mod_type_data.imglib_mod_type,
          stream_info->identity);
    }
  }

  /* Now link the topology modules */
  link_data.ret_code = TRUE;
  link_data.stream_info = stream_info;
  mct_list_operate_nodes(topo_list, module_imglib_port_link_modules, &link_data);
  ret = link_data.ret_code;
  if (FALSE == ret) {
    IDBG_ERROR("%s:%d] Can not link topology modules", __func__, __LINE__);
    goto out;
  }

out:
  return ret;
}

/**
 * Function: module_imglib_unlink_topology_modules
 *
 * Description: This function will unlink the modules in internal
 *   topology
 *
 * Arguments:
 *   @port: mct port pointer
 *   @identity: Stream identity
 *   @topo_index: Internal topology index
 *
 * Return values:
 *   TRUE/FALSE
 *
 **/
static boolean module_imglib_unlink_topo_modules(mct_port_t *port,
  unsigned int identity, uint32_t topo_index)
{
  mct_list_t *stream_list, *p_topo_list;
  imglib_port_data_t *port_data;
  imglib_unlink_modules_data_t unlink_data;

  if (!port) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }
  port_data = (imglib_port_data_t *)port->port_private;

  unlink_data.ret_code = TRUE;
  unlink_data.identity = identity;

  p_topo_list = port_data->internal_topo->topo_list[topo_index];
  mct_list_operate_nodes(p_topo_list, module_imglib_port_unlink_modules,
    &unlink_data);

  /* We need to remove module type on unlink */
  if (unlink_data.ret_code) {
    mct_list_traverse(p_topo_list, module_imglib_port_remove_mod_types,
      &identity);
  }

  return unlink_data.ret_code;
}

/**
 * Function: module_imglib_set_topo_parent
 *
 * Description: This function will link the modules in internal
 *   topology
 *
 * Arguments:
 *   @port: mct port pointer
 *   @identity: Stream identity
 *
 * Return values:
 *   TRUE/FALSE
 *
 **/
static boolean module_imglib_set_topo_parent(mct_port_t *port,
  unsigned int identity)
{
  mct_list_t *topo_list;
  mct_stream_t *stream;
  mct_module_t *module;
  uint32_t i;
  boolean ret = FALSE;

  if (!port) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    goto out;
  }
  module = MCT_MODULE_CAST((MCT_PORT_PARENT(port)->data));

  /* Get module parent */
  stream = mod_imglib_find_module_parent(identity, module);
  if (NULL == stream) {
    IDBG_ERROR("%s:%d Module parent is not set ", __func__, __LINE__);
    goto out;
  }

  for (i = 0; i < PORT_ATTACHED_TOPO(port); i++) {
    /* Get internal topology per stream type */
    topo_list = GET_PORT_TOPO_LIST(port, i);
    if (!topo_list) {
      ret = FALSE;
      IDBG_ERROR("%s:%d] Not available topology for this stream", __func__, __LINE__);
      break;
    }
    ret = mct_list_traverse(topo_list, module_imglib_port_parent_modules, stream);
  }

out:
  return ret;
}

/**
 * Function: module_imglib_unset_topo_parent
 *
 * Description: This function will link the modules in internal
 *   topology
 *
 * Arguments:
 *   @port: mct port pointer
 *   @identity: Stream identity
 *
 * Return values:
 *   TRUE/FALSE
 *
 **/
static boolean module_imglib_unset_topo_parent(mct_port_t *port,
  unsigned int identity)
{
  mct_list_t *topo_list;
  mct_module_t *module;
  mct_stream_t *stream;
  boolean ret = FALSE;
  uint32_t i;

  if (!port) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    goto out;
  }
  module = MCT_MODULE_CAST((MCT_PORT_PARENT(port)->data));

  /* Get module parent */
  stream = mod_imglib_find_module_parent(identity, module);
  if (NULL == stream) {
    IDBG_ERROR("%s:%d Module parent is not set ", __func__, __LINE__);
    goto out;
  }

  for (i = 0; i < PORT_ATTACHED_TOPO(port); i++) {
    /* Get internal topology per stream type */
    topo_list = GET_PORT_TOPO_LIST(port, i);
    if (!topo_list) {
      ret = FALSE;
      IDBG_ERROR("%s:%d] Not available topology for this stream", __func__, __LINE__);
      break;
    }
    ret = mct_list_traverse(topo_list, module_imglib_port_unparent_modules, stream);
  }

out:
  return ret;
}

/**
 * Function: module_imglib_port_send_session_params
 *
 * Description: Function to sent stored session parameters to
 *   internal topologies
 *
 * Arguments:
 *   @port: mct port pointer
 *   @identity: Stream identity
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: This function will update stored session parameters
 **/
static boolean module_imglib_port_send_session_params(mct_port_t *port,
  unsigned int identity)
{
  parm_buffer_t *p_table;
  imglib_port_data_t *port_data;
  mct_event_t event;
  mct_event_control_parm_t param;
  uint32_t i, position, current;
  mct_module_t *p_mct_mod;
  boolean ret = TRUE;

  if (!(port && port->port_private)) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }
  port_data = (imglib_port_data_t *)port->port_private;

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);

  /* Get session parameters table */
  p_table = module_imglib_get_session_params(p_mct_mod,
    IMGLIB_SESSIONID(identity));
  if (NULL == p_table) {
    IDBG_LOW("%s:%d Session params not available skip", __func__, __LINE__);
    return TRUE;
  }

  current = GET_FIRST_PARAM_ID(p_table);

  event.type = MCT_EVENT_CONTROL_CMD;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.identity = identity;
  event.timestamp = 0;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  event.u.ctrl_event.control_event_data = &param;

  while (current != CAM_INTF_PARM_MAX) {
    param.type = current;
    param.parm_data  = POINTER_OF(current, p_table);

    /* Send only events which need to be restored */
    if (port_data->internal_topo->params_to_restore[param.type]) {
      ret = module_imglib_sent_event_to_mirrors(port, &event);
      if (FALSE == ret) {
        IDBG_ERROR("%s:%d Can not sent event to mirrors", __func__, __LINE__);
        break;
      }
    }

    current = GET_NEXT_PARAM_ID(current, p_table);
  }
  return ret;
}

/**
 * Function: module_imglib_port_release_events
 *
 * Description: Release memory needed for imglib events
 *
 * Arguments:
 *   @port_events: Pointer to port events array

 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: None;
 **/
static boolean module_imglib_port_release_events(imglib_port_events_holder_t *port_events)
{
  uint32_t i, j;

  if (!port_events) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  for (i = 0; i < MODULE_IMGLIB_PORT_MAX_HOLD_EVENTS; i++) {
    if (port_events[i].buf_divert_data.use_int_buffers) {
      for (j = 0; j < port_events[i].buf_divert_data.num_planes; j++) {
        free(port_events[i].buf_divert_data.addr[j]);
        port_events[i].buf_divert_data.addr[j] = NULL;
        port_events[i].buf_divert_data.size[j] = 0;
      }
      port_events[i].buf_divert_data.num_planes = 0;
      port_events[i].buf_divert_data.use_int_buffers = FALSE;
    }
  }

 return TRUE;
}

/**
 * Function: module_imglib_port_release_events
 *
 * Description: Allocate memory needed for imglib events
 *
 * Arguments:
 *   @port_events: Pointer to port events array
 *   @stream_info: Stream info
 *   @port_events_mask: Port event feature mask
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: None;
 **/
static boolean module_imglib_port_prepare_events(imglib_port_events_holder_t *port_events,
  mct_stream_info_t *stream_info, uint32_t port_events_mask)
{

  uint32_t i, j;
  boolean ret = TRUE;

  if (!port_events || !stream_info) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  /* Init port events */
  for (i = 0; i < MODULE_IMGLIB_PORT_MAX_HOLD_EVENTS; i++) {
    memset(port_events[i].pending_ack, 0x0, sizeof(port_events[i].pending_ack));
    /* Allocate internal buffers if flag is set */
    port_events[i].buf_divert_data.use_int_buffers =
      (MODULE_IMGLIB_PORT_USE_INT_BUFS & port_events_mask);
    if (!port_events[i].buf_divert_data.use_int_buffers)
      continue;

    port_events[i].buf_divert_data.num_planes =
      stream_info->buf_planes.plane_info.num_planes;

    for (j = 0; j < port_events[i].buf_divert_data.num_planes; j++) {
      if (1 == port_events[i].buf_divert_data.num_planes) {
        port_events[i].buf_divert_data.size[j] =
          stream_info->buf_planes.plane_info.sp.len;
        port_events[i].buf_divert_data.addr[j] =
          malloc(port_events[i].buf_divert_data.size[j]);
      } else {
        port_events[i].buf_divert_data.size[j] =
          stream_info->buf_planes.plane_info.mp[j].len;
        port_events[i].buf_divert_data.addr[j] =
          malloc(port_events[i].buf_divert_data.size[j]);
      }
      if (!port_events[i].buf_divert_data.addr[j]) {
        IDBG_ERROR("%s:%d] Out of memory ", __func__, __LINE__);
        ret = FALSE;
        break;
      }
    }
  }

  if (ret == FALSE) {
    module_imglib_port_release_events(port_events);
  }
 return ret;
}

/**
 * Function: module_imglib_port_handle_events
 *
 * Description: Helper function for handling port events sent to
 *   internal topologies. Here we are handling events sent to internal
 *   topologies. Ack of the events will be handle in the mirror event function
 *   where we received events from internal topologies
 *
 * Arguments:
 *   @port: Pointer to imglib port
 *   @event: mct event
 *
 * Return values:
 *   TRUE if event is handled
 *
 * Notes: None;
 **/
static boolean module_imglib_port_handle_events(mct_port_t *port, mct_event_t *event)
{
  mct_port_t *mirror_port;
  imglib_port_data_t *port_data;
  boolean ret = FALSE;
  boolean event_processed = FALSE;
  uint32_t i;

  if (!(port && port->port_private) || !event) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return event_processed;
  }

  /* Get mirror port and sent event to mirror port peer */
  port_data = (imglib_port_data_t *)port->port_private;

  switch (event->type) {
  case MCT_EVENT_CONTROL_CMD: {
    mct_event_control_t *p_ctrl_event = &event->u.ctrl_event;
    mct_module_t *p_mct_mod =
      MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);

    IDBG_LOW("%s:%d] Ctrl type %d", __func__, __LINE__, p_ctrl_event->type);
    switch (p_ctrl_event->type) {
    case MCT_EVENT_CONTROL_SET_PARM:
      /* Store session params */
      ret = module_imglib_store_session_params(p_mct_mod,
        p_ctrl_event->control_event_data, IMGLIB_SESSIONID(event->identity));
      if (FALSE == ret) {
        IDBG_ERROR("%s:%d] Store session params fail\n",  __func__, __LINE__);
      }
      break;
    default:
      break;
    }
    break;
  }
  case MCT_EVENT_MODULE_EVENT: {
    mct_event_module_t *p_mod_event = &event->u.module_event;
    IDBG_LOW("%s:%d] Mod type %d", __func__, __LINE__, p_mod_event->type);
    switch (p_mod_event->type) {
    case MCT_EVENT_MODULE_BUF_DIVERT: {
      isp_buf_divert_t *p_buf_divert = p_mod_event->module_event_data;
      void *origin_vaddr = p_buf_divert->vaddr;
      imglib_port_events_holder_t *port_events;
      uint32_t index;

      //notify tuning server of new preview stream buf notify event
      mct_module_t *module = MCT_MODULE_CAST(MCT_OBJECT_PARENT(port)->data);
      mct_stream_t *stream = mod_imglib_find_module_parent(event->identity, module);
      if (stream == NULL) {
        IDBG_ERROR("%s:%d unable to find imglib module parent ", __func__, __LINE__);
        return FALSE;
      }

      if ((FALSE == p_buf_divert->is_cpp_processed) &&
          (stream->streaminfo.stream_type == CAM_STREAM_TYPE_PREVIEW)) {
        mct_tuning_notify_preview_frame(p_buf_divert, stream);
      }

      /* Since there is no cookie, we need to choose some reference
       * which is returned on ack. For buffer divert use buffer index */
      index = (p_buf_divert->buffer.index % MODULE_IMGLIB_PORT_MAX_HOLD_EVENTS);
      port_events = &port_data->port_events[index];
      if (port_events->pending_ack[MCT_EVENT_MODULE_BUF_DIVERT_ACK]) {
        IDBG_MED("%s:%d] Pending events for idx %d skip \n", __func__,
          __LINE__, p_buf_divert->buffer.index);
        p_buf_divert->ack_flag = TRUE;
        event_processed = TRUE;
        break;
      }

      /* If we use internal buffer memcpy here and release the event */
      if (port_events->buf_divert_data.use_int_buffers) {
        for (i = 0; i < port_events->buf_divert_data.num_planes; i++) {
          memcpy(port_events->buf_divert_data.addr[i],
            (void *)((unsigned int *)p_buf_divert->vaddr)[i],
            port_events->buf_divert_data.size[i]);
        }
        p_buf_divert->vaddr = (void *)port_events->buf_divert_data.addr;
      }

      for (i = 0; i < PORT_ATTACHED_TOPO(port); i++) {
        mirror_port = port_data->mirror_port[i];
        ret = MCT_PORT_PEER(mirror_port)->event_func(mirror_port->peer, event);
        if (FALSE == ret) {
          IDBG_ERROR("%s:%d] Mirror port returned error \n",  __func__, __LINE__);
          break;
        }
        if (FALSE == p_buf_divert->ack_flag) {
          imglib_mirror_port_data_t *mirror_port_data = mirror_port->port_private;
          port_events->pending_ack[MCT_EVENT_MODULE_BUF_DIVERT_ACK] |=
            (1 << mirror_port_data->index);
          if (port_events->buf_divert_data.use_int_buffers) {
            p_buf_divert->ack_flag = TRUE;
          }
        }
      }
      p_buf_divert->vaddr = origin_vaddr;
      event_processed = ret;
      break;
    }
    default:
      break;
    }
    break;
  }
  default:
    break;
  }

 return event_processed;
}

/**
 * Function: module_imglib_port_handle_events
 *
 * Description: Helper function for handling mirror port events,
 *   events recieved from internal topology
 *
 *   @port: Pointer to mirror port
 *   @event: mct event
 *
 * Return values:
 *   TRUE if event is handled
 *
 * Notes: None;
 **/
static boolean module_imglib_port_handle_mirror_events(mct_port_t *port, mct_event_t *event)
{
  mct_port_t *imglib_port;
  imglib_mirror_port_data_t *mirror_port_data;
  imglib_port_data_t *port_data;
  boolean event_processed = FALSE;

  if (!(port && port->port_private) || !event) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  /* Get imglib port and sent event to original function  */
  mirror_port_data = (imglib_mirror_port_data_t *)port->port_private;
  imglib_port = mirror_port_data->p_imglib_port;
  port_data = (imglib_port_data_t *) imglib_port->port_private;

  switch (event->type) {
  case MCT_EVENT_MODULE_EVENT: {
    mct_event_module_t *p_mod_event = &event->u.module_event;
    IDBG_LOW("%s:%d] Mod type %d", __func__, __LINE__, p_mod_event->type);
    switch (p_mod_event->type) {
    case MCT_EVENT_MODULE_BUF_DIVERT_ACK: {
      uint32_t index;
      isp_buf_divert_ack_t *isp_buf_divert_ack = p_mod_event->module_event_data;
      imglib_port_events_holder_t *port_events;

      /* Here we are touching property of imglib port data so lock the object first */
      MCT_OBJECT_LOCK(imglib_port);

      /* Use index as reference */
      index = (isp_buf_divert_ack->buf_idx % MODULE_IMGLIB_PORT_MAX_HOLD_EVENTS);
      port_events = &port_data->port_events[index];

      port_events->pending_ack[MCT_EVENT_MODULE_BUF_DIVERT_ACK] &=
        ~(1 << mirror_port_data->index);

      /* Do not sent event to the imglib peer if:
       * 1. There are pending acks
       * 2. We use internal buffers - pending acks are used only for reference counting
       * of the internal buffer  */
      if (port_events->pending_ack[MCT_EVENT_MODULE_BUF_DIVERT_ACK] ||
          port_events->buf_divert_data.use_int_buffers) {
        event_processed = TRUE;
      }
      MCT_OBJECT_UNLOCK(imglib_port);
      break;
    }
    default:
      break;
    }
    break;
  }
  default:
    break;
  }

 return event_processed;
}

/**
 * Function: module_imglib_port_dummy_event_func
 *
 * Description: This function will receive ports event
 *  from the internal topology modules
 *
 * Arguments:
 *   @port: dummy port which is attached to mirror port
 *   @event: mct event
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: Dummy port is peer of mirror of real port when there is no
 * module before imglib
 **/
static boolean module_imglib_port_dummy_event_func(mct_port_t *port,
  mct_event_t *event)
{
  boolean event_processed = FALSE;

  switch (event->type) {
  case MCT_EVENT_CONTROL_CMD: {
    mct_event_control_t *p_ctrl_event = &event->u.ctrl_event;
    IDBG_LOW("%s:%d] Ctrl type %d", __func__, __LINE__, p_ctrl_event->type);
    break;
  }
  case MCT_EVENT_MODULE_EVENT: {
    mct_event_module_t *p_mod_event = &event->u.module_event;
    IDBG_LOW("%s:%d] Mod type %d", __func__, __LINE__, p_mod_event->type);
    switch (p_mod_event->type) {
    case MCT_EVENT_MODULE_BUF_DIVERT_ACK: {
      // Internal modules send buf divert ack for the native buffers,
      // which are sent from module event. It just needs to be ignored
      event_processed = TRUE;
      break;
    }
    default:
      break;
    }
    break;
  }
  default:
    IDBG_ERROR("%s:%d] event type %d", __func__, __LINE__, event->type);
    break;
  }

  return event_processed;
}

/**
 * Function: module_imglib_port_mirror_event_func
 *
 * Description: This function will receive ports event
 *  from the internal topology modules and redirect to original
 *  imglib port
 *
 * Arguments:
 *   @port: Mirror port which is attached to internal topology
 *   @event: mct event
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: Mirror port is mirror of real port and used to communicate
 *   with internal topology
 **/
static boolean module_imglib_port_mirror_event_func(mct_port_t *port,
  mct_event_t *event)
{
  mct_port_t *imglib_port;
  imglib_mirror_port_data_t *mirror_port_data;
  imglib_port_data_t *port_data;
  boolean ret;

  if (!(port && port->port_private) || !event) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  /* Get imglib port and sent event to original function  */
  mirror_port_data = (imglib_mirror_port_data_t *)port->port_private;
  imglib_port = mirror_port_data->p_imglib_port;
  port_data = (imglib_port_data_t *) imglib_port->port_private;

  if (!(port_data && imglib_port->peer && imglib_port->peer->event_func)) {
    IDBG_ERROR("%s:%d Missing imglib peer ", __func__, __LINE__);
    return FALSE;
  }

  ret = module_imglib_port_handle_mirror_events(port, event);
  if (FALSE == ret) {
    ret = MCT_PORT_PEER(imglib_port)->event_func(imglib_port->peer, event);
  }

  return ret;
}

/**
 * Function: module_imglib_port_event_func
 *
 * Description: Event handler function for the imglib port
 *
 * Arguments:
 *   @port: mct port pointer
 *   @event: mct event
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: This function will redirect the event to the
 *  internal topology modules
 **/
boolean module_imglib_port_event_func(mct_port_t *port,
  mct_event_t *event)
{
  mct_port_t *mirror_port;
  imglib_port_data_t *port_data;
  boolean ret = TRUE;
  uint32_t i;

  if (!(port && port->port_private) || !event) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  MCT_OBJECT_LOCK(port);

  /* Get mirror port and sent event to mirror port peer */
  port_data = (imglib_port_data_t *)port->port_private;
  if (NULL == port_data->internal_topo) {
    /* If there is no internal topology just broadcast event downstream */
    ret = module_imglib_broadcast_event_downstream(port, event);
    goto out;
  }

  /* If topology is orphan set stream as his parent first */
  if (TRUE == port_data->topo_orphan) {
    ret = module_imglib_set_topo_parent(port, port_data->reserved_identity);
    if (TRUE == ret) {
      port_data->topo_orphan = FALSE;
    } else {
      IDBG_ERROR("%s:%d Can not set parent", __func__, __LINE__);
      goto out;
    }
  }

  ret = module_imglib_port_handle_events(port, event);
  if (FALSE == ret) {
    /* If event is not handle sent to all mirror ports*/
    ret = module_imglib_sent_event_to_mirrors(port, event);
  }


out:
  MCT_OBJECT_UNLOCK(port);
  return ret;
}

/**
 * Function: module_imglib_port_ext_link
 *
 * Description: This method is called when the user establishes
 *              link.
 *
 * Arguments:
 *   @identity: identity for the session and stream
 *   @port: mct port pointer
 *   @peer: peer mct port pointer
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: none
 **/
boolean module_imglib_port_ext_link(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  boolean success = TRUE;
  if (!port || !peer) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  MCT_OBJECT_LOCK(port);

  if (MCT_PORT_PEER(port)) {
    IDBG_ERROR("%s:%d] link already established %s %s", __func__, __LINE__,
      port->object.name, peer->object.name);
    success = FALSE;
    goto out;
  }
  MCT_PORT_PEER(port) = peer;

out:
  MCT_OBJECT_UNLOCK(port);
  return TRUE;
}

/**
 * Function: module_imglib_port_unlink
 *
 * Description: This method is called when the user disconnects
 *              the link.
 *
 * Arguments:
 *   @identity: identity for the session and stream
 *   @port: mct port pointer
 *   @peer: peer mct port pointer
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_imglib_port_unlink(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  if (!port || !peer) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return;
  }

  MCT_OBJECT_LOCK(port);
  MCT_PORT_PEER(port) = NULL;
  MCT_OBJECT_UNLOCK(port);

  return;
}

/**
 * Function: module_imglib_port_set_caps
 *
 * Description: This method is used to set the capabilities
 *
 * Arguments:
 *   @port: mct port pointer
 *   @caps: mct port capabilities
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: none
 **/
boolean module_imglib_port_set_caps(mct_port_t *port,
  mct_port_caps_t *caps)
{
  if (!port || !caps)
    return FALSE;

  MCT_OBJECT_LOCK(port);
  port->caps = *caps;
  MCT_OBJECT_UNLOCK(port);

  return TRUE;
}

/**
 * Function: module_imglib_port_check_caps_reserve
 *
 * Description: This function is used to reserve the port
 *
 * Arguments:
 *   @port: mct port pointer
 *   @peer_caps: pointer to peer capabilities
 *   @stream_info: stream information
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: Connect to the internal topology and link
 *  All modules in internal topology
 **/
boolean module_imglib_port_check_caps_reserve(mct_port_t *port, void *peer_caps,
  void *vstream_info)
{
  mct_module_t *p_mct_mod;
  mct_stream_info_t *stream_info;
  imglib_port_data_t *port_data;
  mct_port_t *top_port = NULL;
  boolean ret = FALSE;
  uint32_t i = 0;

  if (!port || !vstream_info || !peer_caps) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }
  stream_info = (mct_stream_info_t *)vstream_info;
  port_data = (imglib_port_data_t *) port->port_private;

  MCT_OBJECT_LOCK(port);

  if ((0 == stream_info->identity) || (0 != port_data->reserved_identity)) {
    IDBG_ERROR("%s:%d port already reserved or invalid identity %d, %d",
      __func__, __LINE__, stream_info->identity, port_data->reserved_identity);
    goto out;
  }

  /* Get topology */
  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  port_data->internal_topo = module_imglib_get_topology(p_mct_mod, stream_info);
  if (NULL == port_data->internal_topo) {
    /* If there is no internal topology use the port without topology */
    ret = TRUE;
    goto out;
  }

  for (i = 0; i < PORT_ATTACHED_TOPO(port); i++) {
    /* Set mirror port direction */
    port_data->mirror_port[i]->direction = (port->direction == MCT_PORT_SRC) ?
      MCT_PORT_SINK : MCT_PORT_SRC;

    /* Get internal topology port */
    top_port = module_imglib_get_topo_port(port, stream_info, i);
    if (!top_port) {
      IDBG_ERROR("%s:%d] Can not get topology port %d", __func__, __LINE__, i);
      goto out;
    }
    /* Link mirror port with topology port */
    ret = mct_port_establish_link(stream_info->identity,
        port_data->mirror_port[i], top_port);
    if (ret == FALSE) {
      IDBG_ERROR("%s:%d] Can not link to mirror port", __func__, __LINE__);
      goto out;
    }
    /* Link internal topology connected to this mirror port*/
    if (MCT_PORT_IS_SINK(port)) {
      ret = module_imglib_link_topo_modules(p_mct_mod, port, stream_info, i);
      if (ret == FALSE) {
        IDBG_ERROR("%s:%d] Can not link internal topology ", __func__,
            __LINE__);
        goto out;
      }
    }
  }

  if (MCT_PORT_IS_SINK(port)) {
    ret = module_imglib_port_send_session_params(port, stream_info->identity);
    if (ret == FALSE) {
      IDBG_ERROR("%s:%d] Can not sent session params ", __func__,
          __LINE__);
      goto out;
    }
  }

  ret = module_imglib_port_prepare_events(port_data->port_events, stream_info,
    port_data->internal_topo->port_events_mask);
  if (ret == FALSE) {
    IDBG_ERROR("%s:%d] Can not prepare the events", __func__,
        __LINE__);
    goto out;
  }

out:
  if (TRUE == ret) {
    port_data->reserved_identity = stream_info->identity;
    port_data->sessionid = IMGLIB_SESSIONID(stream_info->identity);
  } else {
    IDBG_ERROR("%s:%d] Cap reserve failed ", __func__, __LINE__);
    /* Release mirror port */
    if (NULL != top_port) {
      for (; i > 0; i--) {
        module_imglib_put_topo_port(port, stream_info->identity, (i - 1));
      }
    }
  }

  MCT_OBJECT_UNLOCK(port);

  return ret;
}

/**
 * Function: module_imglib_port_check_caps_unreserve
 *
 * Description: This method is used to unreserved the port
 *
 * Arguments:
 *   @port: mct port pointer
 *   @identity: identity for the session and stream
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: Here we will unlink internal topology links and
 *   destroy the mirror port
 **/
static boolean module_imglib_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  boolean ret = FALSE;
  imglib_port_data_t *port_data;
  uint32_t i, j;

  if (!(port && port->port_private)) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  MCT_OBJECT_LOCK(port);

  port_data = (imglib_port_data_t *) port->port_private;

  if (NULL == port_data->internal_topo) {
    /* If there is no topology in the port just unreserve */
    ret = TRUE;
    goto out;
  }

  /* Unlink modules only on port sink unreserve */
  if (MCT_PORT_IS_SINK(port)) {

    if (FALSE == port_data->topo_orphan) {
      ret = module_imglib_unset_topo_parent(port, identity);
      if (FALSE == ret) {
        IDBG_ERROR("%s:%d] Can not unset topo parent", __func__, __LINE__);
        goto out;
      }
      port_data->topo_orphan = TRUE;
    }

    for (i = 0; i < PORT_ATTACHED_TOPO(port); i++) {
      ret = module_imglib_unlink_topo_modules(port, identity, i);
      if (FALSE == ret) {
        IDBG_ERROR("%s:%d] Can not unlink topology modules", __func__, __LINE__);
        goto out;
      }
      ret = module_imglib_put_topo_port(port, identity, i);
      if (FALSE == ret) {
        IDBG_ERROR("%s:%d] Can not put topology port ", __func__, __LINE__);
        goto out;
      }
    }
  }

  ret = module_imglib_port_release_events(port_data->port_events);

out:
  if (TRUE == ret) {
    port_data->reserved_identity = 0;
    port_data->internal_topo = NULL;
  }

  MCT_OBJECT_UNLOCK(port);
  return ret;
}

/**
 * Function: module_imglib_free_port
 *
 * Description: This function is used to free the imglib ports
 *
 * Arguments:
 *   @p_mct_mod: Mctmodule instance pointer
 *   @p_port: Mct port which need to be freed
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: none
 **/
boolean module_imglib_free_port(mct_module_t *p_mct_mod, mct_port_t *p_port)
{
  boolean rc;
  uint32_t i;
  imglib_port_data_t *port_data;

  IDBG_MED("%s:%d port %p p_mct_mod %p", __func__, __LINE__, p_port,
    p_mct_mod);

  if (!p_port || !p_mct_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return TRUE;
  }

  MCT_OBJECT_LOCK(p_port);

  /* If there is private data destroy mirror port first */
  if (p_port->port_private) {
    imglib_port_data_t *port_data =
      (imglib_port_data_t *)p_port->port_private;

    for (i = 0; i < port_data->num_mirror_ports; i++) {
      if (port_data->mirror_port[i]) {
        if (port_data->mirror_port[i]->port_private) {
          free(port_data->mirror_port[i]->port_private);
        }
        mct_port_destroy(port_data->mirror_port[i]);
        port_data->mirror_port[i] = NULL;
      }
    }

  }

  rc = mct_module_remove_port(p_mct_mod, p_port);
  if (rc == FALSE) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    goto out;
  }

  if (p_port->port_private) {
    free(p_port->port_private);
    p_port->port_private = NULL;
  }

out:
  MCT_OBJECT_UNLOCK(p_port);
  mct_port_destroy(p_port);
  return rc;
}

/**
 * Function: module_imglib_create_port
 *
 * Description: Create imglib port
 *
 * Arguments:
 *   @p_mct_mod: Pointer to imglib module
 *   @dir: Port direction
 *   @static_p: static created port
 *   @num_mirror_ports: Number of mirror ports to be
 *     created. Mirror ports are used for conecting
 *     internal topologies.
 *
 * Return values:
 *   MCTL port pointer \ NULL on fail
 *
 * Notes: Currently supported only source ports
 **/
mct_port_t *module_imglib_create_port(mct_module_t *p_mct_mod,
  mct_port_direction_t dir, boolean static_p, int num_mirror_ports)
{
  char portname[MODULE_IMGLIB_PORT_NAME_LEN];
  mct_port_t *p_port = NULL;
  mct_port_t *mirror_port = NULL;
  mct_stream_t *stream = NULL;
  imglib_port_data_t *port_data;
  imglib_mirror_port_data_t *mirror_port_data;
  int index;
  boolean ret_val;

  if (!p_mct_mod || (MCT_PORT_SINK != dir)) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return NULL;
  }

  index = (MCT_PORT_SRC == dir) ?
    p_mct_mod->numsrcports : p_mct_mod->numsinkports;

  /*portname <mod_name>_direction_portIndex*/
  snprintf(portname, sizeof(portname), "%s_d%d_i%d",
    MCT_MODULE_NAME(p_mct_mod), dir, index);
  p_port = mct_port_create(portname);
  if (NULL == p_port) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    goto error;
  }
  IDBG_MED("%s:%d portname %s", __func__, __LINE__, portname);

  port_data = calloc(1, sizeof(imglib_port_data_t));
  if (!port_data) {
    IDBG_ERROR("%s:%d Can not allocate port private data", __func__, __LINE__);
    goto error;
  }

  p_port->direction = dir;
  p_port->port_private = port_data;
  p_port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;
  p_port->caps.u.frame.format_flag = MCT_PORT_CAP_FORMAT_YCBCR;

  /* override the function pointers */
  mct_port_set_check_caps_reserve_func(p_port,
      module_imglib_port_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(p_port,
      module_imglib_port_check_caps_unreserve);
  mct_port_set_set_caps_func(p_port, module_imglib_port_set_caps);
  mct_port_set_ext_link_func(p_port, module_imglib_port_ext_link);
  mct_port_set_unlink_func(p_port, module_imglib_port_unlink);
  mct_port_set_event_func(p_port, module_imglib_port_event_func);

  /* We have caps reserve just need to create internal mirror port */

  for (index = 0; index < num_mirror_ports; index++) {
    snprintf(portname, sizeof(portname), "mirr%d_%s", index,
      MCT_MODULE_NAME(p_port));

    mirror_port = mct_port_create(portname);
    if (NULL == mirror_port) {
      IDBG_ERROR("%s:%d failed", __func__, __LINE__);
      goto error;
    }

    mirror_port_data = calloc(1, sizeof(imglib_mirror_port_data_t));
    if (!mirror_port_data) {
      IDBG_ERROR("%s:%d Can not allocate port private data", __func__, __LINE__);
      goto error;
    }
    /* Port should be with opposite direction since is mirror port */
    mirror_port->caps.port_caps_type = p_port->caps.port_caps_type;
    mirror_port->caps.u.frame.format_flag = p_port->caps.u.frame.format_flag;

    /* Override the function pointers */
    mct_port_set_set_caps_func(mirror_port, module_imglib_port_set_caps);
    mct_port_set_ext_link_func(mirror_port, module_imglib_port_ext_link);
    mct_port_set_unlink_func(mirror_port, module_imglib_port_unlink);
    mct_port_set_event_func(mirror_port, module_imglib_port_mirror_event_func);

    /* Set imglib port as mirror port private data */
    mirror_port_data->p_imglib_port = p_port;
    mirror_port_data->index = index;
    mirror_port->port_private = mirror_port_data;

    /* Init port data structure */
    port_data->mirror_port[index] = mirror_port;
  }

  port_data->num_mirror_ports = num_mirror_ports;
  port_data->topo_orphan = TRUE;
  port_data->sessionid = 0;
  port_data->static_p = static_p;

  /* add port to the module */
  ret_val = mct_module_add_port(p_mct_mod, p_port);
  if (FALSE == ret_val) {
    IDBG_ERROR("%s: Set parent failed", __func__);
    goto error;
  }

  return p_port;

error:

  IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
  if (p_port) {
    mct_port_destroy(p_port);
  }
  return NULL;
}

/**
 * Function: module_imglib_create_dummy_port
 *
 * Description: Create imglib dummy port: This is used
 *  to redirect module events in to sink port events so we need some dummy
 *  port to connect
 *
 * Arguments:
 *   @p_mct_mod: Pointer to imglib module
 *   @dir: Port direction
 *
 * Return values:
 *     MCTL port pointer \ NULL on fail
 *
 **/
mct_port_t *module_imglib_create_dummy_port(mct_module_t *p_mct_mod,
  mct_port_direction_t dir)
{
  char portname[MODULE_IMGLIB_PORT_NAME_LEN];
  mct_port_t *p_port = NULL;
  imglib_port_data_t *port_data;
  int index;
  boolean ret_val;

  if (!p_mct_mod || (MCT_PORT_UNKNOWN == dir)) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return NULL;
  }

  index = (MCT_PORT_SRC == dir) ?
    p_mct_mod->numsrcports : p_mct_mod->numsinkports;

  /*portname <mod_name>_direction_portIndex*/
  snprintf(portname, sizeof(portname), "%s_dymmy_d%d_i%d",
    MCT_MODULE_NAME(p_mct_mod), dir, index);
  p_port = mct_port_create(portname);
  if (NULL == p_port) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    goto error;
  }
  IDBG_MED("%s:%d portname %s", __func__, __LINE__, portname);

  p_port->direction = dir;
  p_port->port_private = NULL;
  p_port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;
  p_port->caps.u.frame.format_flag = MCT_PORT_CAP_FORMAT_YCBCR;

  mct_port_set_event_func(p_port, module_imglib_port_dummy_event_func);

  /* add port to the module */
  ret_val = mct_module_add_port(p_mct_mod, p_port);
  if (FALSE == ret_val) {
    IDBG_ERROR("%s: Set parent failed", __func__);
    goto error;
  }

  return p_port;

error:

  IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
  if (p_port) {
    mct_port_destroy(p_port);
  }
  return NULL;
}

/**
 * Function: module_imglib_get_and_reserve_port
 *
 * Description: Search and reserve available port
 *
 * Arguments:
 *   @p_mct_mod: Pointer to imglib module
 *   @stream_info: Stream info
 *   @dir: Port direction
 *   @peer_cap: peer port capabilites
 *
 * Return values:
 *     MCTL port pointer \ NULL on fail
 *git st
 **/
mct_port_t *module_imglib_get_and_reserve_port(mct_module_t *p_mct_mod,
   mct_stream_info_t *stream_info, mct_port_direction_t dir, void *peer_cap)
{
  mct_list_t *list_match = NULL;
  mct_port_t *our_port = NULL;
  imglib_reserve_port_data_t reserve_data;
  module_imglib_t *p_mod;

  if (!(p_mct_mod && p_mct_mod->module_private) || !stream_info || !peer_cap) {
    IDBG_ERROR("%s:%d] Invalid input arguments", __func__, __LINE__);
    return NULL;
  }

  p_mod = (module_imglib_t *)p_mct_mod->module_private;
  if (!p_mod->dummy_port) {
    IDBG_ERROR("%s:%d] Dummy port missing", __func__, __LINE__);
    return NULL;
  }

  reserve_data.port_caps = peer_cap;
  reserve_data.stream_info = stream_info;

  if (MCT_PORT_SINK == dir)
    list_match = mct_list_find_custom(MCT_MODULE_SINKPORTS(p_mct_mod),
      &reserve_data, module_imglib_reserve_compatible_port);
  else if (MCT_PORT_SRC == dir)
    list_match = mct_list_find_custom(MCT_MODULE_SRCPORTS(p_mct_mod),
      &reserve_data, module_imglib_reserve_compatible_port);

  if (list_match)
    our_port = (mct_port_t *)list_match->data;
  else
    IDBG_MED("%s:%d] Can not reserve the port", __func__, __LINE__);

  return our_port;
}

/**
 * Function: module_imglib_get_port_with_identity
 *
 * Description: Search for reserved port with given identity
 *
 * Arguments:
 *   @p_mct_mod: Pointer to imglib module
 *   @identity: Identity to search for
 *   @dir: Port direction
 *
 * Return values:
 *     MCTL port pointer \ NULL on fail
 *
 **/
mct_port_t *module_imglib_get_port_with_identity(mct_module_t *p_mct_mod,
  unsigned int identity, mct_port_direction_t dir)
{
  mct_list_t *list_match = NULL;
  mct_port_t *our_port = NULL;

  if (!p_mct_mod) {
    return NULL;
  }

  if (MCT_PORT_SINK == dir)
    list_match = mct_list_find_custom(MCT_MODULE_SINKPORTS(p_mct_mod),
      &identity, module_imglib_check_linked_port_identity);
  else if (MCT_PORT_SRC == dir)
    list_match = mct_list_find_custom(MCT_MODULE_SRCPORTS(p_mct_mod),
      &identity, module_imglib_check_linked_port_identity);

  if (list_match)
    our_port = (mct_port_t *)list_match->data;

  return list_match ? (mct_port_t *)list_match->data : NULL;
}

/**
 * Function: module_imglib_get_dyn_port_with_sessionid
 *
 * Description: Search for reserved dynamic port with given session id
 *
 * Arguments:
 *   @p_mct_mod - Pointer to imglib module
 *   @sessionid - Session id to serch for
 *   @dir - Port direction
 *
 * Return values:
 *     MCTL port pointer \ NULL on fail
 *
 **/
mct_port_t *module_imglib_get_dyn_port_with_sessionid(mct_module_t *p_mct_mod,
  unsigned int sessionid, mct_port_direction_t dir)
{
  mct_list_t *list_match = NULL;
  mct_port_t *our_port = NULL;

  if (!p_mct_mod) {
    return NULL;
  }

  if (MCT_PORT_SINK == dir)
    list_match = mct_list_find_custom(MCT_MODULE_SINKPORTS(p_mct_mod),
      &sessionid, module_imglib_check_dynamic_port_sessionid);
  else if (MCT_PORT_SRC == dir)
    list_match = mct_list_find_custom(MCT_MODULE_SRCPORTS(p_mct_mod),
      &sessionid, module_imglib_check_dynamic_port_sessionid);

  if (list_match)
    our_port = (mct_port_t *)list_match->data;

  return list_match ? (mct_port_t *)list_match->data : NULL;
}
