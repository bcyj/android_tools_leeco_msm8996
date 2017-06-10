/* mct_port.c
 *
 * This file contains the default infrastructure and implementation for
 * the ports. Some of the functions defined here may be over-ridden
 * by the respective port objects.
 *
 * Copyright (c) 2012-2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "mct_port.h"
#include "mct_module.h"
#include "camera_dbg.h"

#if 0
#undef CDBG
#define CDBG CDBG_ERROR
#endif

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean compare_port_number(void *data1, void *data2)
{
  unsigned int *ids  = (unsigned int *)data1;
  unsigned int *data = (unsigned int *)data2;
  if (*ids == *data)
    return TRUE;

  return FALSE;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean find_linked_port(void *data1, void *data2)
{
  mct_port_t *port   = MCT_PORT_CAST(data1);
  unsigned int *info = (unsigned int *)(data2);
  if (mct_list_find_custom(MCT_PORT_CHILDREN(port),
      info, compare_port_number))
    return TRUE;

  return FALSE;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
boolean mct_port_send_event_to_peer(mct_port_t *port, mct_event_t *event)
{
  if (!port || !event)
    return FALSE;

  if (!MCT_PORT_PEER(port) || !MCT_PORT_EVENT_FUNC(MCT_PORT_PEER(port)))
    return FALSE;

  return MCT_PORT_EVENT_FUNC(MCT_PORT_PEER(port))(MCT_PORT_PEER(port), event);
}

/*
 * MctPortEventFunction:
 *  @port:   the #MctPort to handle the event.
 *  @event: the #MctEvent to handle.
 *
 * Function signature to handle an event for the port.
 *
 * Returns: TRUE if the port could handle the event.
 */
static boolean mct_port_event_default(mct_port_t *port, mct_event_t *event)
{
  boolean result = FALSE;

  if (!port || !event )
     return FALSE;

  switch (MCT_EVENT_DIRECTION(event)) {
  case MCT_EVENT_UPSTREAM: {

    if (MCT_PORT_IS_SINK(port)) {
      /* ... Process Event here ... */

      result = mct_port_send_event_to_peer(port, event);

    } else if (MCT_PORT_IS_SRC(port)) {
      mct_list_t *list;

      /* ... Process Event here ... */

      if (MCT_PORT_INTLINKFUNC(port)){
        list = MCT_PORT_INTLINKFUNC(port)(event->identity, port);

        if (list) {
          /* ... Forward event up to internal sink ports if needed ... */
        }
      }
    } /* port direction */

  } /* case MCT_EVENT_TYPE_UPSTREAM */
    break;

  case MCT_EVENT_DOWNSTREAM: {

    if (MCT_PORT_IS_SRC(port)) {
      result = mct_port_send_event_to_peer(port, event);

    } else if (MCT_PORT_IS_SINK(port)) {
      mct_list_t *list = NULL;

      /* ... Process Event here ... */

      if (MCT_PORT_INTLINKFUNC(port))
        list = MCT_PORT_INTLINKFUNC(port)(event->identity, port);

      if (list) {
        /* ... Forward event down to internal sink ports if needed ... */
      }
    }/* port direction */
  } /* case MCT_EVENT_TYPE_DOWNSTREAM */
    break;

  default:
    result = FALSE;
    break;
  }

  return result;
}

/**
 * mct_port_event_fwd_list_default:
 *
 * Iterate the list of ports to which the given port is linked to inside of
 * the parent Module.
 *
 * This is the default handler, and thus returns an MctList of all of the
 * ports inside the parent module with opposite direction.
 *
 * Port MUST implement this function
 *
 * The caller must free this MctList after use it.
 */
static mct_list_t *mct_port_event_fwd_list_default(unsigned int identity,
  mct_port_t *port)
{
  mct_list_t *selected_mct_list = NULL;
#if 0
  mct_list_t *complete_list;
  mct_port_session_stream_t session_stream;

  mct_module_t *module = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);

  if (MCT_PORT_IS_SRC(port))
    complete_list = MCT_MODULE_SINKPORTS(module);
  else if (MCT_PORT_IS_SINK(port))
    complete_list = MCT_MODULE_SRCPORTS(module);
  else
    return NULL;

  session_stream.sessionid = sessionid;
  session_stream.streamid = streamid;
  selected_mct_list = mct_list_find_and_add_custom(complete_list,
    child_list, &session_stream, find_linked_port);
#endif
  return selected_mct_list;
}

/*
 * MctPortLinkReturn
 *    @sessionid: the sessionid.
 *    @streamid:  the streamid
 *    @port:      the #MctPort that is linked.
 *    @peer:      the peer #MctPort of the link
 *
 * Function signature to handle a new external link on the port. The
 * defualt implementation here just stores the session and stream ids.
 * This function should be overridden by every port.
 *
 * Returns: the result of the link with the specified peer.
 */
static boolean mct_port_external_links_default(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  if (!MCT_PORT_PEER(port)) {
    MCT_PORT_PEER(port)  = peer;
  } else { /*the link has already been established*/
    if ((MCT_PORT_PEER(port) != peer))
    goto FAIL;
  }

  return TRUE;
FAIL:
  return FALSE;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static void mct_port_unlink_default(unsigned int identity,
 mct_port_t *port, mct_port_t *peer)
{
  if (port->peer != peer)
    return;

  if (port->port_private == NULL)
    port->peer  = NULL;

  return;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_port_set_caps_default(mct_port_t *port,
  mct_port_caps_t *caps)
{
  return TRUE;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_port_check_caps_reserve_default(mct_port_t *port,
  void *peer_caps, void *stream_info)
{
  return TRUE;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_port_check_caps_unreserve_default(
  mct_port_t *port, unsigned int identity)
{
  return TRUE;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static boolean mct_port_check_link_internal(mct_port_t **src,
  mct_port_t **sink)
{
  mct_object_t *root;
  /* Port should have only one parent:
   * These parents are modules */
  mct_object_t *src_parent, *sink_parent;
  mct_list_t *ports_created_local = NULL;

  src_parent  = MCT_OBJECT_CAST(MCT_PORT_PARENT(*src)->data);
  sink_parent = MCT_OBJECT_CAST(MCT_PORT_PARENT(*sink)->data);

  if (!src_parent || !sink_parent)
    return FALSE;

  if (mct_object_find_common_parent(src_parent, sink_parent))
    /* it should be good if both modules are on the same stream */
    return TRUE;

  return FALSE;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
boolean mct_port_check_link(mct_port_t *srcport, mct_port_t *sinkport)
{
  /* generic checks */
  if (!srcport || !sinkport)
    return FALSE;

  if (!MCT_PORT_IS_SRC(srcport))
    return FALSE;

  if (!MCT_PORT_IS_SINK(sinkport))
    return FALSE;

  if (MCT_PORT_PARENT(srcport) == NULL)
    return FALSE;

  if (MCT_PORT_PARENT(sinkport) == NULL)
    return FALSE;

  if (srcport->peer != NULL && srcport->peer != sinkport) {
    return FALSE;
  }

  if (sinkport->peer != NULL && sinkport->peer != srcport) {
    return FALSE;
  }
/*
  if (!mct_port_check_link_internal(&srcport, &sinkport))
    return FALSE;
*/

  return TRUE;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
boolean mct_port_add_child(unsigned int identity, mct_port_t *port)
{
  unsigned int *p_identity;
  p_identity = malloc(sizeof(unsigned int));
  if (!p_identity)
    return FALSE;
  *p_identity = identity;
  MCT_OBJECT_LOCK(port);
  MCT_PORT_CHILDREN(port) = mct_list_append(
    MCT_PORT_CHILDREN(port), p_identity, NULL, NULL);
  if (!MCT_PORT_CHILDREN(port)) {
    MCT_OBJECT_UNLOCK(port);
    return FALSE;
  }

  MCT_OBJECT_UNLOCK(port);
  return TRUE;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
void mct_port_remove_child(unsigned int identity, mct_port_t *port)
{
  unsigned int *p_identity;
  mct_list_t *identity_holder;
  unsigned int info;

  MCT_OBJECT_LOCK(port);
  identity_holder = mct_list_find_custom(MCT_PORT_CHILDREN(port),
    &identity, compare_port_number);

  if (!identity_holder) {
    MCT_OBJECT_UNLOCK(port);
    return;
  }
  p_identity = (unsigned int *)identity_holder->data;
  MCT_PORT_CHILDREN(port) = mct_list_remove(
    MCT_PORT_CHILDREN(port), p_identity);
  MCT_OBJECT_UNLOCK(port);
  free(p_identity);
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
boolean mct_port_establish_link(unsigned int identity,
  mct_port_t *srcport, mct_port_t *sinkport)
{
  if (!srcport || !sinkport)
    return FALSE;

  if (!MCT_PORT_IS_SRC(srcport) || !MCT_PORT_IS_SINK(sinkport))
    return FALSE;

  if (srcport->ext_link && sinkport->ext_link) {
    if (srcport->ext_link(identity, srcport, sinkport) == FALSE)
      goto FAIL;
    if (sinkport->ext_link(identity, sinkport, srcport) == FALSE)
      goto FAIL1;
    if (mct_port_add_child(identity, srcport) == FALSE)
      goto FAIL2;
    if (mct_port_add_child(identity, sinkport) == FALSE)
      goto FAIL3;
  } else
    goto FAIL;

  return TRUE;
FAIL3:
  mct_port_remove_child(identity, srcport);
FAIL2:
  srcport->un_link(identity, sinkport, srcport);
FAIL1:
  srcport->un_link(identity, srcport, sinkport);
FAIL:
  return FALSE;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
void mct_port_destroy_link(unsigned int identity,
  mct_port_t *srcport, mct_port_t *sinkport)
{
  if (!srcport || !sinkport ||
      !MCT_PORT_IS_SRC(srcport) ||
      !MCT_PORT_IS_SINK(sinkport))
    return;

  if (srcport->un_link)
    srcport->un_link(identity, srcport, sinkport);
  if (sinkport->un_link)
    sinkport->un_link(identity, sinkport, srcport);
  return;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
static void mct_port_init_default(mct_port_t *port)
{
  MCT_PORT_DIRECTION(port) = MCT_PORT_UNKNOWN;
  mct_port_set_event_func(port, mct_port_event_default);
  mct_port_set_int_link_func(port, mct_port_event_fwd_list_default);
  mct_port_set_ext_link_func(port, mct_port_external_links_default);
  mct_port_set_unlink_func(port, mct_port_unlink_default);
  mct_port_set_set_caps_func(port, mct_port_set_caps_default);
  mct_port_set_check_caps_reserve_func
    (port, mct_port_check_caps_reserve_default);
  mct_port_set_check_caps_unreserve_func
    (port, mct_port_check_caps_unreserve_default);
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
mct_port_t *mct_port_create(const char *name)
{
  mct_port_t *port;

  port = malloc(sizeof(mct_port_t));
  if (!port)
    return port;

  memset(port, 0, sizeof(mct_port_t));
  pthread_mutex_init(MCT_OBJECT_GET_LOCK(port), NULL);
  mct_object_set_name(MCT_OBJECT_CAST(port), name);
  mct_port_init_default(port);

  return port;
}

/** Name:
 *
 *  Arguments/Fields:
 *    @
 *    @
 *
 *  Return:
 *
 *  Description:
 *
 **/
void mct_port_destroy(mct_port_t *port)
{
  pthread_mutex_destroy(MCT_OBJECT_GET_LOCK(port));
  free(MCT_PORT_NAME(port));
  free(port);
  return;
}
