/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vpe_port.h"
#include "vpe_module.h"
#include "camera_dbg.h"
#include "vpe_log.h"

mct_port_t *vpe_port_create(const char* name, mct_port_direction_t dir)
{
  mct_port_t  *port;
  vpe_port_data_t* port_data;
  int i;
  CDBG("%s:%d: Enter: name=%s, dir=%d", __func__, __LINE__, name, dir);
  port = mct_port_create(name);

  if(!port) {
    CDBG_ERROR("%s:%d failed", __func__, __LINE__);
    goto port_create_error;
  }
  port->direction = dir;
  port->check_caps_reserve = vpe_port_check_caps_reserve;
  port->check_caps_unreserve = vpe_port_check_caps_unreserve;
  port->ext_link = vpe_port_ext_link_func;
  port->un_link = vpe_port_ext_unlink_func;
  port->event_func = vpe_port_event_func;
  port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;

  port_data = (vpe_port_data_t*) malloc(sizeof(vpe_port_data_t));
  if(!port_data) {
    CDBG_ERROR("%s:%d failed", __func__, __LINE__);
    goto port_data_error;
  }
  memset(port_data, 0x00, sizeof(vpe_port_data_t));
  for(i=0; i<VPE_MAX_STREAMS_PER_PORT; i++) {
    //port_data->port_state[i] = VPE_PORT_STATE_UNRESERVED;
    port_data->stream_data[i].port_state = VPE_PORT_STATE_UNRESERVED;
  }
  port_data->port_type = VPE_PORT_TYPE_INVALID;
  port_data->num_streams = 0;
  MCT_OBJECT_PRIVATE(port) = port_data;
  return port;

port_data_error:
  mct_port_destroy(port);
port_create_error:
  return NULL;
}
void vpe_port_destroy(mct_port_t *port)
{
  if(!port) {
    return;
  }
  free(MCT_OBJECT_PRIVATE(port));
  mct_port_destroy(port);
}

static boolean vpe_port_check_caps_reserve(mct_port_t *port, void *peer_caps,
  void *info)
{
  if(!port || !info) {
    CDBG_ERROR("%s:%d failed, port=%p, info=%p", __func__, __LINE__,
                port, info);
    return FALSE;
  }
  mct_stream_info_t *stream_info = (mct_stream_info_t *)info;
  mct_port_caps_t *port_caps = (mct_port_caps_t *)(&(port->caps));
  vpe_port_data_t *port_data = MCT_OBJECT_PRIVATE(port);
  uint32_t identity = stream_info->identity;
  uint32_t session_id, stream_id;
  int rc;
  session_id = VPE_GET_SESSION_ID(identity);
  stream_id = VPE_GET_STREAM_ID(identity);
  CDBG_HIGH("%s:%d, identity=0x%x\n", __func__, __LINE__, identity);

  if(port_data->num_streams >= VPE_MAX_STREAMS_PER_PORT) {
    CDBG_ERROR("%s:%d, failed. max streams reached, num=%d",
      __func__, __LINE__, port_data->num_streams);
    return FALSE;
  }
  if(port->direction == MCT_PORT_SINK) {
    /* TODO: For Offline/peerless this is not true because peer caps is NULL */
#if 0
    if(!peer_caps) {
      CDBG_ERROR("%s:%d failed, peear_caps=%p", __func__, __LINE__, peer_caps);
      return FALSE;
    }
#endif
    /* TODO: Add more peer caps checking logic */
    mct_port_caps_t *peer_port_caps = (mct_port_caps_t *)peer_caps;
    if(peer_caps) {
      if(port_caps->port_caps_type != peer_port_caps->port_caps_type) {
        return FALSE;
      }
    }
  }

  switch(port_data->port_type) {
  case VPE_PORT_TYPE_INVALID: {
    port_data->num_streams = 0;
    if(stream_info->streaming_mode == CAM_STREAMING_MODE_CONTINUOUS) {
      port_data->port_type = VPE_PORT_TYPE_STREAMING;
    } else {
      port_data->port_type = VPE_PORT_TYPE_BURST;
    }
    port_data->session_id = session_id;
    break;
  }
  case VPE_PORT_TYPE_STREAMING: {
    if(stream_info->streaming_mode != CAM_STREAMING_MODE_CONTINUOUS) {
      CDBG("%s:%d, info: streaming mode doesn't match", __func__, __LINE__);
      return FALSE;
    }
    if(port_data->session_id != session_id) {
      CDBG("%s:%d, info: session id doesn't match", __func__, __LINE__);
      return FALSE;
    }
    break;
  }
  case VPE_PORT_TYPE_BURST: {
    if(stream_info->streaming_mode != CAM_STREAMING_MODE_BURST) {
      CDBG("%s:%d, info: streaming mode doesn't match", __func__, __LINE__);
      return FALSE;
    }
    if(port_data->session_id != session_id) {
      CDBG("%s:%d, info: session id doesn't match", __func__, __LINE__);
      return FALSE;
    }
    break;
  }
  default:
    CDBG_ERROR("%s:%d, failed, bad port_type=%d\n", __func__, __LINE__,
                port_data->port_type);
    return FALSE;
  }
  int i;
  /* reserve the port for this stream */
  for(i=0; i<VPE_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state == VPE_PORT_STATE_UNRESERVED) {
      port_data->stream_data[i].port_state = VPE_PORT_STATE_RESERVED;
      port_data->stream_data[i].identity = identity;
      port_data->stream_data[i].streaming_mode = stream_info->streaming_mode;
      port_data->num_streams++;
      CDBG_HIGH("%s:%d, identity=0x%x, reserved\n",
        __func__, __LINE__, identity);
      break;
    }
  }
  if(i == VPE_MAX_STREAMS_PER_PORT) {
    CDBG_ERROR("%s:%d, failed, unexpected error!", __func__, __LINE__);
    return FALSE;
  }
  /* notify the parent module about this new stream, once for the sink port */
  if(port->direction == MCT_PORT_SINK) {
    mct_module_t* module = (mct_module_t*)(MCT_PORT_PARENT(port)->data);
    rc = vpe_module_notify_add_stream(module, port, stream_info);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed, unexpected error!", __func__, __LINE__);
      return FALSE;
    }
  }
  return TRUE;
}

static boolean vpe_port_check_caps_unreserve(mct_port_t *port,
  uint32_t identity)
{
  if(!port) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return FALSE;
  }
  CDBG_HIGH("%s:%d, identity=0x%x\n", __func__, __LINE__, identity);
  vpe_port_data_t *port_data = (vpe_port_data_t *) MCT_OBJECT_PRIVATE(port);
  int i, rc;
  for(i=0; i<VPE_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state == VPE_PORT_STATE_RESERVED &&
       port_data->stream_data[i].identity == identity) {
      port_data->stream_data[i].port_state = VPE_PORT_STATE_UNRESERVED;
      port_data->num_streams--;
      CDBG_HIGH("%s:%d, identity=0x%x, unreserved\n",
        __func__, __LINE__, identity);
      if(port_data->num_streams == 0) {
        port_data->port_type = VPE_PORT_TYPE_INVALID;
        port_data->session_id = 0x00;
      }
      break;
    }
  }
  if(i == VPE_MAX_STREAMS_PER_PORT) {
    CDBG_ERROR("%s:%d, can't find matching identity, unexpected !!",
                __func__, __LINE__);
    return FALSE;
  }

  /* notify the parent module about removal of this stream,
     once for the sink port */
  if(port->direction == MCT_PORT_SINK) {
    mct_module_t* module = (mct_module_t*)(MCT_PORT_PARENT(port)->data);
    rc = vpe_module_notify_remove_stream(module, identity);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed, unexpected error!", __func__, __LINE__);
      return FALSE;
    }
  }
  return TRUE;
}

static boolean vpe_port_ext_link_func(uint32_t identity,
  mct_port_t* port, mct_port_t *peer)
{
  if(!port || !peer) {
    CDBG_ERROR("%s:%d failed, port=%p, peer=%p", __func__, __LINE__,
                port, peer);
    return FALSE;
  }
  CDBG("%s:%d", __func__, __LINE__);
  MCT_OBJECT_LOCK(port);

  if (MCT_PORT_PEER(port) && (MCT_PORT_PEER(port) != peer)) {
    CDBG_ERROR("%s:%d] error old_peer:%s, new_peer:%s\n", __func__, __LINE__,
      MCT_OBJECT_NAME(MCT_PORT_PEER(port)), MCT_OBJECT_NAME(peer));
    MCT_OBJECT_UNLOCK(port);
    return FALSE;
  }

  vpe_port_data_t* port_data = (vpe_port_data_t *)MCT_OBJECT_PRIVATE(port);
  int i;
  for(i=0; i<VPE_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state == VPE_PORT_STATE_RESERVED &&
        port_data->stream_data[i].identity == identity) {
      port_data->stream_data[i].port_state = VPE_PORT_STATE_LINKED;
      if (MCT_OBJECT_REFCOUNT(port) == 0) {
        MCT_PORT_PEER(port) = peer;
      }
      MCT_OBJECT_REFCOUNT(port) += 1;
      MCT_OBJECT_UNLOCK(port);
      CDBG("%s:%d", __func__, __LINE__);
      return TRUE;
    }
  }
  MCT_OBJECT_UNLOCK(port);
  CDBG("%s:%d", __func__, __LINE__);
  return FALSE;
}


static void vpe_port_ext_unlink_func(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  if(!port || !peer) {
    CDBG_ERROR("%s:%d failed, port=%p, peer=%p", __func__, __LINE__,
                port, peer);
    return;
  }

  MCT_OBJECT_LOCK(port);
  if (MCT_PORT_PEER(port) != peer) {
    CDBG_ERROR("%s:%d] failed peer:%p, unlink_peer:%p\n", __func__, __LINE__,
      MCT_PORT_PEER(port), peer);
    MCT_OBJECT_UNLOCK(port);
    return;
  }

  if (MCT_OBJECT_REFCOUNT(port) == 0) {
    CDBG_ERROR("%s:%d] failed zero refcount on port\n", __func__, __LINE__);
    MCT_OBJECT_UNLOCK(port);
    return;
  }

  vpe_port_data_t* port_data = (vpe_port_data_t *)MCT_OBJECT_PRIVATE(port);
  int i;
  for(i=0; i<VPE_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state == VPE_PORT_STATE_LINKED &&
        port_data->stream_data[i].identity == identity) {
      port_data->stream_data[i].port_state = VPE_PORT_STATE_RESERVED;
      MCT_OBJECT_REFCOUNT(port) -= 1;
      if (MCT_OBJECT_REFCOUNT(port) == 0) {
        MCT_PORT_PEER(port) = NULL;
      }
      MCT_OBJECT_UNLOCK(port);
      return;
    }
  }
  MCT_OBJECT_UNLOCK(port);
  CDBG_ERROR("%s:%d failed", __func__, __LINE__);
  return;
}

static boolean vpe_port_event_func(mct_port_t *port,
                                   mct_event_t *event)
{
  int32_t rc = -EINVAL;

  if(!port || !event) {
    CDBG_ERROR("%s:%d failed, port=%p, event=%p", __func__, __LINE__,
                port, event);
    return FALSE;
  }
  mct_module_t *module;
  mct_list_t *templist = (mct_list_t*)MCT_PORT_PARENT(port);
  module = (mct_module_t*)(templist->data);

  switch(port->direction) {
  case MCT_PORT_SRC:
    rc = vpe_module_process_upstream_event(module, event);
    break;
  case MCT_PORT_SINK:
    rc = vpe_module_process_downstream_event(module, event);
    break;
  default:
    CDBG_ERROR("%s:%d failed, bad port->direction=%d", __func__,
                __LINE__, port->direction);
  }
  return rc == 0 ? TRUE : FALSE;
}

/* vpe_port_get_linked_identity:
 *
 *  finds another identity mapped on the port which is not equal to @identity
 **/
int32_t vpe_port_get_linked_identity(mct_port_t *port, uint32_t identity,
  uint32_t *linked_identity)
{
  if(!port || !linked_identity) {
    CDBG_ERROR("%s:%d, failed, port=%p, linked_identity=%p\n",
      __func__, __LINE__, port, linked_identity);
    return -EINVAL;
  }
  vpe_port_data_t *port_data = (vpe_port_data_t *) MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  if(port_data->num_streams > 0) {
    int i;
    for (i=0; i<VPE_MAX_STREAMS_PER_PORT; i++) {
      if (port_data->stream_data[i].port_state != VPE_PORT_STATE_UNRESERVED) {
        if (port_data->stream_data[i].identity != identity) {
          *linked_identity = port_data->stream_data[i].identity;
          return 1;
        }
      }
    }
  }
  *linked_identity=0x00;
  return 0;
}
