/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include <linux/media.h>
#include <dlfcn.h>
#include "camera_dbg.h"
#include "cam_intf.h"
#include "cam_types.h"
#include "mct_module.h"
#include "pproc_caps.h"
#include "pproc_interface.h"
#include "module_pproc.h"
#include "modules.h"
#include "mct_stream.h"
#include "mct_pipeline.h"
#include "port_pproc.h"
#include "port_pproc_common.h"
#include "module_cpp.h"
//#include "module_c2d.h"

//#define CONFIG_PPROC_DBG
#undef CDBG
#ifdef CONFIG_PPROC_DBG
#define CDBG(fmt, args...) ALOGE(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif

static mct_pproc_init_name_t pproc_modules_list[] = {
  [PPROC_CPP] = {"cpp", module_cpp_init, module_cpp_deinit, NULL},
  //[PPROC_C2D] = {"c2d", module_c2d_init, module_c2d_deinit, NULL},
  /* TODO: Add other submodules */
#if 0
  {"vpe", NULL, NULL},  /* module_vpe_init   module_vpe_deinit  */
  {"swp", NULL, NULL},  /* module_swp_init   module_swp_deinit  */
#endif
};

/** module_pproc_traverse_submodule
 *    @data: module
 *    @userdata:
 *
 **/
static boolean module_pproc_traverse_submodule(void *data, void *user_data)
{
  mct_module_t *module = MCT_MODULE_CAST(data);

  module_pproc_ctrl_t *pproc_ctrl =
    MODULE_PPROC_CTRL_CAST(MODULE_PPROC_PRIVATE_DATA(module));

  return TRUE;
}

static boolean module_pproc_find_caps_match_port(void *list_data,
  void *user_data)
{
  port_pproc_common_link_create_info_t *pproc_tplgy_info =
    (port_pproc_common_link_create_info_t *)user_data;
  mct_stream_info_t *mct_stream_info =
    (mct_stream_info_t *)pproc_tplgy_info->stream_info;
  mct_port_t *port = (mct_port_t *)list_data;
  boolean ret = FALSE;
  unsigned int *identity = NULL;
  CDBG("%s:%d Enter\n", __func__, __LINE__);
  ret = port->check_caps_reserve(port, pproc_tplgy_info->peer,
    pproc_tplgy_info->stream_info);
  if (ret == TRUE) {
    identity = malloc(sizeof(uint32_t));
    if (!identity) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      return FALSE;
    }
    *identity = mct_stream_info->identity;
    CDBG("%s:Port match, identity:%d\n", __func__, *identity);

    MCT_PORT_CHILDREN(port) = mct_list_append(MCT_PORT_CHILDREN(port), identity,
      NULL, NULL);
  }
  CDBG("%s:%d Exit ret:%d\n", __func__, __LINE__, ret);
  return ret;
}

static boolean module_pproc_find_identity_match_port(void *list_data,
  void *user_data)
{
  mct_list_t                 *identity_holder;
  mct_port_t *port = (mct_port_t *)list_data;
  CDBG("%s:%d Enter\n", __func__, __LINE__);
  identity_holder = mct_list_find_custom(MCT_PORT_CHILDREN(port),
    user_data, port_pproc_common_match_identity);
  if (identity_holder != NULL) {
    CDBG("%s:%d Exit Success\n", __func__, __LINE__);
    return TRUE;
  }
  CDBG("%s:%d Exit failure\n", __func__, __LINE__);
  return FALSE;
}

static boolean module_pproc_submodules_unreserve(mct_module_t *module,
  unsigned int identity)
{
  module_pproc_ctrl_t *pproc_ctrl =
    MODULE_PPROC_CTRL_CAST(MODULE_PPROC_PRIVATE_DATA(module));
  mct_list_t *list_port = NULL;
  mct_port_t *port = NULL;
  boolean ret = FALSE;
  mct_module_t *cpp = NULL;
  unsigned int *my_identity = NULL;
  mct_list_t *identity_holder = NULL;

  CDBG("%s:Enter\n", __func__);
  cpp = pproc_modules_list[PPROC_CPP].mod;
  if (!cpp) {
    CDBG("%s:Null: %p\n", __func__, cpp);
    return FALSE;
  }
  list_port = mct_list_find_custom(cpp->sinkports, &identity,
    module_pproc_find_identity_match_port);

  if (list_port) {
      CDBG("%s: Identity match port found\n", __func__);
      port = (mct_port_t *)list_port->data;
      ret = port->check_caps_unreserve(port, identity);
      identity_holder = mct_list_find_custom(MCT_PORT_CHILDREN(port),
        &identity, port_pproc_common_match_identity);
      if (identity_holder) {
        my_identity = (unsigned int *)identity_holder->data;
        MCT_PORT_CHILDREN(port) = mct_list_remove(
          MCT_PORT_CHILDREN(port), my_identity);
        free(my_identity);
      }
  }

  CDBG("%s:Exit Ret:%d\n", __func__, ret);
  return ret;
}

static boolean module_pproc_submodules_reserve(mct_module_t *module,
  void *info)
{
  module_pproc_ctrl_t *pproc_ctrl =
    MODULE_PPROC_CTRL_CAST(MODULE_PPROC_PRIVATE_DATA(module));
  mct_list_t *port = NULL;
  port_pproc_common_link_create_info_t *pproc_caps_info =
    (port_pproc_common_link_create_info_t *)info;
  mct_stream_info_t *mct_stream_info =
    (mct_stream_info_t *)pproc_caps_info->stream_info;
  mct_module_t *cpp = NULL;

  CDBG("%s:Enter\n", __func__);
  /* TODO: use stream_info to build topology */
  /* hardcode to active sub module list here */
  /* switch(mct_stream_info) */
  cpp = pproc_modules_list[PPROC_CPP].mod;
  if (!cpp) {
    CDBG("%s:Null: %p\n", __func__, cpp);
    return FALSE;
  }

  cpp->set_mod(cpp, MCT_MODULE_FLAG_PEERLESS,
    mct_stream_info->identity);

  port = mct_list_find_custom(cpp->sinkports, info,
    module_pproc_find_caps_match_port);

  if (!port) {
    return FALSE;
  }
  CDBG("%s:Exit reserve port:%p\n", __func__, port);
  return TRUE;
}

static boolean module_pproc_fwd_to_single_link(void *list_data, void *user_data)
{
  pproc_event_link_traverse_obj_t *traverse_obj =
    (pproc_event_link_traverse_obj_t *)user_data;
  port_pproc_common_link_t *link = (port_pproc_common_link_t *)list_data;
  mct_event_t event = (traverse_obj->event);
  mct_event_module_t *mod_event;
  boolean rc = FALSE;
  isp_buf_divert_t *isp_divert = NULL;

  CDBG("%s:%d: Enter\n", __func__, __LINE__);
  if (event.type == MCT_EVENT_MODULE_EVENT) {
    mod_event = &event.u.module_event;
    if (mod_event->type == MCT_EVENT_MODULE_BUF_DIVERT) {
      isp_divert = (isp_buf_divert_t *)mod_event->module_event_data;
      isp_divert->is_locked = 0;
    }
  }

  rc = MCT_PORT_EVENT_FUNC(link->sink_port)(link->sink_port, &event);
  if (rc == TRUE) {
    if (isp_divert) {
      isp_divert->ack_flag = FALSE;
      traverse_obj->frame_consumer_ref_count += isp_divert->is_locked;
      CDBG("%s:%d eve id %d buf id %d bufindex %d frame id %d ref count %d\n",
        __func__, __LINE__, event.identity, isp_divert->identity,
        isp_divert->buffer.index, isp_divert->buffer.sequence,
        traverse_obj->frame_consumer_ref_count);
      CDBG("%s:%d: BufferLock:%d, refcnt:%d\n", __func__, __LINE__,
        isp_divert->is_locked, traverse_obj->frame_consumer_ref_count);
      isp_divert->is_locked = 0;
    }
  }

  CDBG("%s:%d: Exit:%d\n", __func__, __LINE__, rc);
  return rc;
}

static boolean module_pproc_create_outgoing_event(
  pproc_event_link_traverse_obj_t *incoming_traverse_evnt,
  pproc_event_link_traverse_obj_t **out_traverse_event)
{
  mct_event_module_t              *event_ptr;
  mct_event_t                      incoming_mct_event, *ack_event;
  isp_buf_divert_t                *isp_divert = NULL;
  isp_buf_divert_ack_t            *buff_divert_ack = NULL;
  pproc_event_link_traverse_obj_t *queue_event = NULL;

  CDBG("%s:Enter\n", __func__);
  if (!incoming_traverse_evnt || !out_traverse_event) {
    CDBG_ERROR("%s:Invalid incoming event\n", __func__);
    return FALSE;
  }

  *out_traverse_event = NULL;
  incoming_mct_event = (incoming_traverse_evnt->event);

  if (incoming_mct_event.type == MCT_EVENT_MODULE_EVENT) {
    event_ptr = &incoming_mct_event.u.module_event;
    CDBG("%s:event type %d\n", __func__, event_ptr->type);
    if (event_ptr->type == MCT_EVENT_MODULE_BUF_DIVERT) {
      isp_divert = (isp_buf_divert_t *)event_ptr->module_event_data;
      CDBG("%s:isp_divert:%p\n", __func__, isp_divert);
      isp_divert->is_locked = 0;

      /* Create queue event which can be freed if no ref count is done */
      queue_event = (pproc_event_link_traverse_obj_t *)malloc(
        sizeof(pproc_event_link_traverse_obj_t));
      if (!queue_event) {
        CDBG_ERROR("%s:%d: Error event alloc\n", __func__, __LINE__);
        return FALSE;
      }
      memset(queue_event, 0, sizeof(pproc_event_link_traverse_obj_t));

      buff_divert_ack = (isp_buf_divert_ack_t *)malloc(
         sizeof(isp_buf_divert_ack_t));
      if (!buff_divert_ack) {
        free(queue_event);
        CDBG_ERROR("%s:%d: Error event alloc\n", __func__, __LINE__);
        return FALSE;
      }
      memset(buff_divert_ack,  0,  sizeof(isp_buf_divert_ack_t));

      ack_event = &queue_event->event;
      ack_event->u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT_ACK;
      ack_event->u.module_event.module_event_data = (void *)buff_divert_ack;
      ack_event->type = MCT_EVENT_MODULE_EVENT;
      ack_event->identity = incoming_mct_event.identity;
      ack_event->direction = MCT_EVENT_UPSTREAM;
      /* Extract the index of buffer from on-going buffer */
      buff_divert_ack->buf_idx = isp_divert->buffer.index;
      /* TODO: Need to extract buffer done/put buff from link status */
      buff_divert_ack->is_buf_dirty = 1;
    }
  }

  *out_traverse_event = queue_event;

  CDBG("%s:%d: Exit queue event %p\n", __func__, __LINE__,
    *out_traverse_event);
  return TRUE;
}

static boolean module_pproc_fwd_to_all_links(
  pproc_link_traverse_t *current_node,
  pproc_event_link_traverse_obj_t *queue_event)
{
  boolean                          rc = FALSE;
  mct_list_t                      *out_evnt_list;
  pproc_event_link_traverse_obj_t  traverse_obj;

  CDBG("%s:%d: Enter\n", __func__, __LINE__);
  if ((!current_node) || (!current_node->out_evnt_list)) {
    CDBG_ERROR("%s: Invalid node to traverse\n", __func__);
    return rc;
  }

  out_evnt_list = *(current_node->out_evnt_list);
  CDBG("%s:Traverse:%p\n", __func__, out_evnt_list);

  traverse_obj.event = current_node->incoming_traverse_evnt.event;
  traverse_obj.frame_consumer_ref_count = 0;
  rc = mct_list_traverse(current_node->links,
    current_node->traverse_func, (void *)&traverse_obj);
  CDBG("%s:%d: que_event:%p\n", __func__, __LINE__, queue_event);
  if (queue_event) {
    CDBG("%s:%d: rc:%d, refcnt:%d\n", __func__, __LINE__, rc,
               traverse_obj.frame_consumer_ref_count);
    if ((rc == TRUE) && (traverse_obj.frame_consumer_ref_count)) {
        queue_event->frame_consumer_ref_count =
          traverse_obj.frame_consumer_ref_count;
        current_node->incoming_traverse_evnt.frame_consumer_ref_count++;
        out_evnt_list = mct_list_append(out_evnt_list, queue_event,
          NULL, NULL);
        *(current_node->out_evnt_list) = out_evnt_list;
        CDBG("%s:%d:Traverse:%p\n", __func__, __LINE__, out_evnt_list);
    } else {
      CDBG("%s:%d: que_event:%p\n", __func__, __LINE__, queue_event);
      /* No ref count means no consumer of the buffer */
      free(queue_event->event.u.module_event.module_event_data);
      free(queue_event);
      queue_event = NULL;
    }
  }

  CDBG("%s:%d: Exit:%d\n", __func__, __LINE__, rc);
  return rc;
}

static boolean module_pproc_fwd_to_identity_match_stream(void *list_data,
  void *user_data)
{
  port_pproc_common_divert_link_t *current_stream_link =
    (port_pproc_common_divert_link_t *)list_data;
  pproc_link_traverse_t            current_stream_node;
  pproc_event_link_traverse_obj_t *current_incoming_event =
    (pproc_event_link_traverse_obj_t *)user_data;
  pproc_event_link_traverse_obj_t *queue_event = NULL;
  boolean                          rc = FALSE;

  CDBG("%s:%d: Enter\n", __func__, __LINE__);

  if (current_stream_link->identity != current_incoming_event->event.identity)
  {
    CDBG("%s: Req_identity:%d is no match for list_identity:%d\n", __func__,
      current_incoming_event->event.identity, current_stream_link->identity);
    return FALSE;
  }
  rc = module_pproc_create_outgoing_event(current_incoming_event,
    &queue_event);
  if (FALSE == rc) {
    CDBG_ERROR("%s: Error in creating outgoing event.\n", __func__);
    return FALSE;
  }
  current_stream_node.identity = current_stream_link->identity;
  current_stream_node.links = current_stream_link->tplgy_lnks;
  current_stream_node.out_evnt_list =
    &current_stream_link->out_evnt_list;
  CDBG("%s:Traverse:%p\n", __func__, current_stream_link->out_evnt_list);
  current_stream_node.traverse_func = module_pproc_fwd_to_single_link;
  current_stream_node.incoming_traverse_evnt =
    *current_incoming_event;
  //current_stream_node.incoming_traverse_evnt.event.identity =
      //current_stream_node.identity;
  CDBG("%s:%d identity %d\n", __func__, __LINE__,
    current_stream_node.incoming_traverse_evnt.event.identity);
  current_stream_node.incoming_traverse_evnt.frame_consumer_ref_count = 0;

  rc = module_pproc_fwd_to_all_links(&current_stream_node, queue_event);
  current_incoming_event->frame_consumer_ref_count +=
    current_stream_node.incoming_traverse_evnt.frame_consumer_ref_count;
  CDBG("%s:%d: Exit:%d, refcnt:%d\n", __func__, __LINE__, rc,
             current_incoming_event->frame_consumer_ref_count);
  return rc;
}

static boolean module_pproc_check_streamon_state_by_identity(void *list_data,
  void *user_data)
{
  port_pproc_common_divert_link_t *current_stream_link =
    (port_pproc_common_divert_link_t *)list_data;
  uint32_t *identity = (uint32_t *)user_data;

  if ((*identity == current_stream_link->identity) &&
    (current_stream_link->streaming_on)) {
    return TRUE;
  }

  return FALSE;
}

static boolean module_pproc_fwd_to_all_active_streams(void *list_data,
  void *user_data)
{
  port_pproc_common_divert_link_t *current_stream_link =
    (port_pproc_common_divert_link_t *)list_data;
  pproc_link_traverse_t            current_stream_node;
  pproc_event_link_traverse_obj_t *current_incoming_event =
    (pproc_event_link_traverse_obj_t *)user_data;
  pproc_event_link_traverse_obj_t *queue_event = NULL;
  boolean                          rc = TRUE;
  isp_buf_divert_t *isp_divert =
    (isp_buf_divert_t *)current_incoming_event->event.u.module_event.module_event_data;

  current_incoming_event->event.identity = current_stream_link->identity;
  CDBG("%s:%d: Enter\n", __func__, __LINE__);
  if (current_stream_link->streaming_on) {
    CDBG("%s:%d: StreaminON\n", __func__, __LINE__);
    rc = module_pproc_create_outgoing_event(current_incoming_event,
      &queue_event);
    if (FALSE == rc) {
      CDBG_ERROR("%s: Error in creating outgoing event.\n", __func__);
      return FALSE;
    }

    current_stream_node.identity = current_stream_link->identity;
    CDBG("%s:%d identity %d\n", __func__, __LINE__,
      current_stream_link->identity);
    current_stream_node.links = current_stream_link->tplgy_lnks;
    current_stream_node.out_evnt_list =
      &current_stream_link->out_evnt_list;
    CDBG("%s:%d:Traverse:%p\n", __func__, __LINE__, current_stream_link->out_evnt_list);
    current_stream_node.traverse_func = module_pproc_fwd_to_single_link;
    current_stream_node.incoming_traverse_evnt =
      *current_incoming_event;
    //current_stream_node.incoming_traverse_evnt.event.identity =
      //current_stream_node.identity;
    current_stream_node.incoming_traverse_evnt.frame_consumer_ref_count = 0;

    rc = module_pproc_fwd_to_all_links(&current_stream_node, queue_event);
    current_incoming_event->frame_consumer_ref_count +=
      current_stream_node.incoming_traverse_evnt.frame_consumer_ref_count;
    CDBG("%s:%d eve id %d buf id %d bufindex %d frame id %d ref count %d\n",
      __func__, __LINE__, current_incoming_event->event.identity,
      isp_divert->identity, isp_divert->buffer.index,
      isp_divert->buffer.sequence,
      current_incoming_event->frame_consumer_ref_count);
    CDBG("%s:%d: Exit:%d, refcnt:%d\n", __func__, __LINE__, rc,
               current_incoming_event->frame_consumer_ref_count);
  }

  CDBG("%s:%d: Exit:%d\n", __func__, __LINE__, rc);
  return rc;
}

static boolean module_pproc_fwd_to_streams_on_port(
  port_pproc_priv_data_t *port_priv,
  pproc_event_link_traverse_obj_t *current_incoming_event,
  mct_list_traverse_func traverse_func)
{
  pproc_link_traverse_t            current_port;
  pproc_event_link_traverse_obj_t *queue_event = NULL;
  boolean                          rc = FALSE;

  CDBG("%s:Enter\n", __func__);
  if ((!port_priv) || (!current_incoming_event)) {
    CDBG_ERROR("%s Port:%p, Event:%p\n", __func__, port_priv,
      current_incoming_event);
    return FALSE;
  }

  rc = module_pproc_create_outgoing_event(current_incoming_event,
    &queue_event);
  if (FALSE == rc) {
    CDBG_ERROR("%s: Error in creating outgoing event.\n", __func__);
    return FALSE;
  }

  current_port.identity = current_incoming_event->event.identity;
  current_port.links = port_priv->links_by_identity;
  current_port.out_evnt_list = &port_priv->out_evnt_list;
  current_port.traverse_func = traverse_func;
  current_port.incoming_traverse_evnt = *current_incoming_event;
  current_port.incoming_traverse_evnt.frame_consumer_ref_count = 0;

  rc = module_pproc_fwd_to_all_links(&current_port, queue_event);
  CDBG("%s:Exit:%d\n", __func__, rc);
  return rc;
}

boolean module_pproc_find_event_in_list(mct_list_t **evnt_list,
  mct_event_t *out_event, boolean *ref_count)
{
  pproc_event_link_traverse_obj_t *traverse_event;
  mct_list_t *list_node;
  mct_list_t *out_evnt_list = *evnt_list;
  isp_buf_divert_ack_t *buff_divert_ack = out_event->u.module_event.module_event_data;

  *ref_count = 1;
  CDBG("%s:Enter:%p\n", __func__, out_evnt_list);
  list_node = mct_list_find_custom(out_evnt_list,
    (void *)out_event, port_pproc_find_queue_event);
  if (list_node) {
    traverse_event = (pproc_event_link_traverse_obj_t *)list_node->data;
    if (traverse_event->frame_consumer_ref_count) {
      traverse_event->frame_consumer_ref_count--;
      CDBG("%s:%d eve id %d buf id %d bufindex %d ref count %d\n",
        __func__, __LINE__, out_event->identity, buff_divert_ack->identity,
        buff_divert_ack->buf_idx, traverse_event->frame_consumer_ref_count);

      if (!traverse_event->frame_consumer_ref_count) {
        /* pop the event from list */
        out_evnt_list = mct_list_remove(out_evnt_list, list_node->data);
        free(traverse_event->event.u.module_event.module_event_data);
        free(traverse_event);

        CDBG("%s:Pop the event\n", __func__);
        *evnt_list = out_evnt_list;
        *ref_count = 0;
        
        if (out_event->identity != buff_divert_ack->identity) {
          out_event->identity = buff_divert_ack->identity;
        }
      }

      CDBG("%s:Exit with success\n", __func__);
      return TRUE;
    } else {
      CDBG_ERROR("%s: Stray event found in list\n", __func__);
      /* pop the event from list */
      out_evnt_list = mct_list_remove(out_evnt_list, list_node->data);
      free(traverse_event->event.u.module_event.module_event_data);
      free(traverse_event);
      *evnt_list = out_evnt_list;

      return FALSE;
    }
  } else {
    CDBG_ERROR("%s: Event not found in list, failure\n", __func__);
    return FALSE;
  }

  CDBG("%s:Exit with failure\n", __func__);
  return FALSE;
}

boolean module_pproc_check_refcount_for_event(
  mct_port_t *port, mct_event_t *out_event, boolean *msg_ref_count)
{
  port_pproc_common_divert_link_t *stream_node;
  mct_list_t                      *list_node;
  port_pproc_priv_data_t          *pproc_port_priv = port->port_private;
  boolean                          rc = FALSE;

  *msg_ref_count = 0;
  CDBG("%s:Enter\n", __func__);
  list_node = mct_list_find_custom(pproc_port_priv->links_by_identity,
    &out_event->identity, port_pproc_find_divert_link_by_identity);
  CDBG("%s:divert_list:%p\n", __func__, list_node);
  if (list_node) {
    stream_node = (port_pproc_common_divert_link_t *)list_node->data;
    CDBG("%s:divert_list:%p\n", __func__, list_node);
    CDBG("%s:search stream_queue:%p\n", __func__,stream_node->out_evnt_list);
    rc = module_pproc_find_event_in_list(&stream_node->out_evnt_list,
      out_event, msg_ref_count);
    if (FALSE == rc) {
      CDBG_ERROR("%s: Error in event list\n", __func__);
      return rc;
    }

    CDBG("%s:stream_queue found:%d\n", __func__, *msg_ref_count);
    if (*msg_ref_count == 0) {
      module_pproc_find_event_in_list(&pproc_port_priv->out_evnt_list,
                                      out_event, msg_ref_count);
      if (FALSE == rc) {
        CDBG_ERROR("%s: Error in event list\n", __func__);
        return rc;
      }
      CDBG("%s:port queue found:%d\n", __func__, *msg_ref_count);
    }
    CDBG("%s:Exit\n", __func__);
    return TRUE;
  }

  CDBG("%s: Error in finding list\n", __func__);
  return FALSE;
}

static boolean module_pproc_find_streamtype_match_port(void *list_data,
  void *user_data)
{
  mct_port_t *port = (mct_port_t *)list_data;
  mct_stream_info_t *stream_info = (mct_stream_info_t *)user_data;
  module_pproc_common_port_private_t *private_data;

  if (!port || !stream_info || !port->port_private) {
    return FALSE;
  }

  private_data =
    (module_pproc_common_port_private_t *)port->port_private;

  if (private_data->streaming_mode == stream_info->streaming_mode) {
    //if (private_data->streaming_mode == CAM_STREAMING_MODE_BURST)
    return TRUE;
  }
  return FALSE;
}

static boolean module_pproc_priv_event_handler(mct_module_t *module,
  mct_port_t *port, pproc_priv_event_t *pproc_priv_event)
{
  port_pproc_common_link_create_info_t *create_info;
  mct_module_t *cpp = NULL;
  boolean rc = FALSE;
  mct_list_t *list_port_node;

  cpp = pproc_modules_list[PPROC_CPP].mod;
  if (!cpp) {
    CDBG("%s:Null: %p\n", __func__, cpp);
    return FALSE;
  }

  switch (pproc_priv_event->type) {
  case PPROC_PRIV_EVENT_CREATE_TOPOLOGY:
    create_info =
      (port_pproc_common_link_create_info_t *)pproc_priv_event->data;
    create_info->link->link_index = 0;
    list_port_node = mct_list_find_custom(cpp->sinkports, create_info->stream_info,
      module_pproc_find_streamtype_match_port);
    if (!list_port_node) {
      CDBG_ERROR("%s: Port match fail\n", __func__);
      return FALSE;
    }
    create_info->link->sink_port = (mct_port_t *)list_port_node->data;
    create_info->link->src_port = NULL;
    create_info->link->identity = ((mct_stream_info_t *)create_info->stream_info)->identity;
    rc = module_pproc_submodules_reserve(module, create_info);
    break;
  case PPROC_PRIV_EVENT_DELETE_TOPOLOGY:
    create_info =
      (port_pproc_common_link_create_info_t *)pproc_priv_event->data;
    rc = module_pproc_submodules_unreserve(module, create_info->link->identity);
    break;
  case PPROC_PRIV_EVENT_DISPATCH_MCT_EVENT_DWS: {
    port_pproc_priv_data_t         *pproc_port_priv = NULL;
    pproc_event_link_traverse_obj_t in_event;
    mct_event_t                    *mct_event = NULL;

    if ((!port) || (!port->port_private)) {
      CDBG_ERROR("%s: Invalid port\n", __func__);
      return FALSE;
    }

    pproc_port_priv = (port_pproc_priv_data_t *)port->port_private;
    mct_event = (mct_event_t *)pproc_priv_event->data;
    in_event.frame_consumer_ref_count = 0;
    in_event.event = *mct_event;

    switch (mct_event->type) {
    case MCT_EVENT_CONTROL_CMD: {
      mct_event_control_t *mct_event_control = &mct_event->u.ctrl_event;
      switch (mct_event_control->type) {
      case MCT_EVENT_CONTROL_STREAMON: {
        mct_list_t *list_node;
        ALOGE("%s:StreamON enter:%d\n", __func__, mct_event->identity);
        pthread_mutex_lock(&pproc_port_priv->mutex);

        in_event.frame_consumer_ref_count = 0;
        list_node = mct_list_find_custom(pproc_port_priv->links_by_identity,
          &in_event,module_pproc_fwd_to_identity_match_stream);
        if (!list_node) {
          CDBG_ERROR("%s: Identity match stream not found\n", __func__);
          pthread_mutex_unlock(&pproc_port_priv->mutex);
          return FALSE;
        }
        rc = module_pproc_fwd_to_all_active_streams(list_node->data, &in_event);

        list_node = mct_list_find_custom(pproc_port_priv->links_by_identity,
          &mct_event->identity, port_pproc_find_divert_link_by_identity);
        CDBG("%s:list_node:%p\n", __func__, list_node);
        if (list_node) {
          port_pproc_common_divert_link_t *link =
            (port_pproc_common_divert_link_t *)list_node->data;
          link->streaming_on = 1;
          CDBG("%s:link->streaming_on\n", __func__);
        }
        assert(link->out_evnt_list == NULL);
        pthread_mutex_unlock(&pproc_port_priv->mutex);
        ALOGE("%s:StreamON exit:%d\n", __func__, mct_event->identity);
        break;
      }
      case MCT_EVENT_CONTROL_STREAMOFF: {
        mct_list_t *list_node;
        port_pproc_common_divert_link_t *link;
        ALOGE("%s:%d Stream OFF IN id=%d\n", __func__, __LINE__, mct_event->identity);
        pthread_mutex_lock(&pproc_port_priv->mutex);
        pproc_port_priv->return_divert_buffer = TRUE;

        list_node = mct_list_find_custom(pproc_port_priv->links_by_identity,
          &mct_event->identity, port_pproc_find_divert_link_by_identity);
        if (list_node) {
          link = (port_pproc_common_divert_link_t *)list_node->data;
          link->streaming_on = 0;
        }
        pthread_mutex_unlock(&pproc_port_priv->mutex);

        in_event.frame_consumer_ref_count = 0;
        list_node = mct_list_find_custom(pproc_port_priv->links_by_identity,
          &in_event,module_pproc_fwd_to_identity_match_stream);
        if (!list_node) {
          CDBG_ERROR("%s: Identity match stream not found\n", __func__);
          return FALSE;
        }

        rc = module_pproc_fwd_to_all_active_streams(list_node->data, &in_event);
        assert(link->out_evnt_list == NULL);
        pthread_mutex_lock(&pproc_port_priv->mutex);
        pproc_port_priv->return_divert_buffer = FALSE;
        pthread_mutex_unlock(&pproc_port_priv->mutex);
        ALOGE("%s:%d Stream OFF out id =%d\n", __func__, __LINE__, mct_event->identity);
        break;
      }
      case MCT_EVENT_CONTROL_SET_PARM: {
        mct_list_t *list_node;
        pthread_mutex_lock(&pproc_port_priv->mutex);

        in_event.frame_consumer_ref_count = 0;
        list_node = mct_list_find_custom(pproc_port_priv->links_by_identity,
          &in_event,module_pproc_fwd_to_identity_match_stream);
        if (!list_node) {
          CDBG_ERROR("%s: Identity match stream not found\n", __func__);
          pthread_mutex_unlock(&pproc_port_priv->mutex);
          return FALSE;
        }
        rc = module_pproc_fwd_to_all_active_streams(list_node->data, &in_event);

        if (rc == FALSE) {
          CDBG("%s:%d failed\n", __func__, __LINE__);
        }
        pthread_mutex_unlock(&pproc_port_priv->mutex);
        break;
      }
      default:
        rc = mct_list_traverse(module->srcports,
          port_pproc_common_send_event_to_peer, (void *)mct_event);
        break;
      } /* mct_event_control->type */
      break;
    } /* case MCT_EVENT_CONTROL_CMD */
    case MCT_EVENT_MODULE_EVENT: {
      mct_event_module_t *mct_event_module = &mct_event->u.module_event;
      isp_buf_divert_t *buff_divert;
      switch (mct_event_module->type) {
      case MCT_EVENT_MODULE_BUF_DIVERT: {
        mct_list_t *list_node;
        pthread_mutex_lock(&pproc_port_priv->mutex);
        buff_divert = (isp_buf_divert_t *)mct_event_module->module_event_data;
        buff_divert->identity = mct_event->identity;
        if (pproc_port_priv->return_divert_buffer) {
          /* StreamOFF going on. No more stream can be taken in */
          buff_divert->ack_flag = 1;
          buff_divert->is_buf_dirty = 1;
          buff_divert->is_locked = 0;
          rc = TRUE;
        } else {
          rc = module_pproc_fwd_to_streams_on_port(pproc_port_priv, &in_event,
            module_pproc_fwd_to_all_active_streams);
        }
        pthread_mutex_unlock(&pproc_port_priv->mutex);
        break;
      }
      case MCT_EVENT_MODULE_ISP_OUTPUT_DIM: {
        mct_stream_info_t *stream_info = NULL;
        mct_list_t *list_node;
        pthread_mutex_lock(&pproc_port_priv->mutex);
        stream_info = (mct_stream_info_t *)mct_event_module->module_event_data;
        if (!stream_info) {
          CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
          pthread_mutex_unlock(&pproc_port_priv->mutex);
          return FALSE;
        }
        CDBG("%s:%d ide %d width %d height %d\n", __func__, __LINE__,
          mct_event->identity, stream_info->dim.width, stream_info->dim.height);

        in_event.frame_consumer_ref_count = 0;
        list_node = mct_list_find_custom(pproc_port_priv->links_by_identity,
          &in_event,module_pproc_fwd_to_identity_match_stream);
        if (!list_node) {
          CDBG_ERROR("%s: Identity match stream not found\n", __func__);
          pthread_mutex_unlock(&pproc_port_priv->mutex);
          return FALSE;
        }
        rc = module_pproc_fwd_to_all_active_streams(list_node->data, &in_event);
        if (rc == FALSE) {
          CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
        }
        pthread_mutex_unlock(&pproc_port_priv->mutex);
        break;
      }
      case MCT_EVENT_MODULE_STREAM_CROP: {
        mct_bus_msg_stream_crop_t *stream_crop = NULL;
        mct_list_t *list_node;
        pthread_mutex_lock(&pproc_port_priv->mutex);
        stream_crop =
          (mct_bus_msg_stream_crop_t *)mct_event_module->module_event_data;
        if (!stream_crop->crop_out_x || !stream_crop->crop_out_y) {
          pthread_mutex_unlock(&pproc_port_priv->mutex);
          break;
        }

        in_event.frame_consumer_ref_count = 0;
        list_node = mct_list_find_custom(pproc_port_priv->links_by_identity,
          &in_event,module_pproc_fwd_to_identity_match_stream);
        if (!list_node) {
          CDBG_ERROR("%s: Identity match stream not found\n", __func__);
          pthread_mutex_unlock(&pproc_port_priv->mutex);
          return FALSE;
        }
        rc = module_pproc_fwd_to_all_active_streams(list_node->data, &in_event);
        if (rc == FALSE) {
          CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
        }
        pthread_mutex_unlock(&pproc_port_priv->mutex);
        break;
      }
      case MCT_EVENT_MODULE_STATS_DIS_UPDATE: {
        is_update_t *is_crop = NULL;
        mct_list_t *list_node;
        pthread_mutex_lock(&pproc_port_priv->mutex);
        is_crop = (is_update_t *)mct_event_module->module_event_data;
        if (!is_crop) {
          CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
          pthread_mutex_unlock(&pproc_port_priv->mutex);
          break;
        }
        CDBG("%s:%d DISEIS zoom x %d y %d w %d h %d\n", __func__, __LINE__,
          is_crop->x, is_crop->y, is_crop->width, is_crop->height);
        if (!is_crop->width || !is_crop->width) {
          pthread_mutex_unlock(&pproc_port_priv->mutex);
          break;
        }

        in_event.frame_consumer_ref_count = 0;
        list_node = mct_list_find_custom(pproc_port_priv->links_by_identity,
          &in_event,module_pproc_fwd_to_identity_match_stream);
        if (!list_node) {
          CDBG_ERROR("%s: Identity match stream not found\n", __func__);
          pthread_mutex_unlock(&pproc_port_priv->mutex);
          rc = FALSE;
          break;
        }
        rc = module_pproc_fwd_to_all_active_streams(list_node->data, &in_event);
        if (rc == FALSE) {
          CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
        }
        pthread_mutex_unlock(&pproc_port_priv->mutex);
        break;
      }
      case MCT_EVENT_MODULE_SET_CHROMATIX_PTR:
      case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {
        mct_list_t *list_node;
        pthread_mutex_lock(&pproc_port_priv->mutex);
        in_event.frame_consumer_ref_count = 0;
        list_node = mct_list_find_custom(pproc_port_priv->links_by_identity,
          &in_event,module_pproc_fwd_to_identity_match_stream);
        if (!list_node) {
          CDBG_ERROR("%s: Identity match stream not found\n", __func__);
          pthread_mutex_unlock(&pproc_port_priv->mutex);
          rc = FALSE;
          break;
        }
        rc = module_pproc_fwd_to_all_active_streams(list_node->data, &in_event);
        if (rc == FALSE) {
          CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
        }
        pthread_mutex_unlock(&pproc_port_priv->mutex);
        break;
      }
      default:
        rc = mct_list_traverse(module->srcports,
          port_pproc_common_send_event_to_peer, (void *)mct_event);
        break;
      }
      break;
    } /* case MCT_EVENT_MODULE_EVENT */
    default:
      rc = FALSE;
      break;
    } /* mct_event->type */
    break;
  } /* case PPROC_PRIV_EVENT_DISPATCH_MCT_EVENT_DWS */
  case PPROC_PRIV_EVENT_DISPATCH_MCT_EVENT_UPS: {
    port_pproc_priv_data_t         *pproc_port_priv = NULL;
    pproc_event_link_traverse_obj_t out_event;
    mct_event_t                    *mct_event = NULL;

    if ((!port) || (!port->port_private)) {
      CDBG_ERROR("%s: Invalid port\n", __func__);
      return FALSE;
    }

    pproc_port_priv = (port_pproc_priv_data_t *)port->port_private;
    mct_event = (mct_event_t *)pproc_priv_event->data;
    out_event.frame_consumer_ref_count = 0;
    out_event.event = *mct_event;

    switch (mct_event->type) {
    case MCT_EVENT_CONTROL_CMD:
      rc = mct_list_traverse(module->sinkports,
        port_pproc_common_send_event_to_peer, (void *)mct_event);
      break;
    case MCT_EVENT_MODULE_EVENT: {
      mct_event_module_t *mct_event_module = &mct_event->u.module_event;
      boolean             event_ref_count = 0;
      switch (mct_event_module->type) {
      case MCT_EVENT_MODULE_BUF_DIVERT_ACK:
        pthread_mutex_lock(&pproc_port_priv->mutex);
        rc = module_pproc_check_refcount_for_event(port, &out_event.event,
          &event_ref_count);
        pthread_mutex_unlock(&pproc_port_priv->mutex);
        if ((rc == TRUE) && !event_ref_count) {
          rc = mct_port_send_event_to_peer(port, &out_event.event);
        }
        break;
      default:
        rc = mct_list_traverse(module->sinkports,
          port_pproc_common_send_event_to_peer, (void *)mct_event);
        break;
      }
      break;
    } /* case MCT_EVENT_MODULE_EVENT */
    default:
      rc = FALSE;
      break;
    } /* mct_event->type */
    break;
  }
  default:
    rc = FALSE;
    break;
  } /* pproc_priv_event->type*/

  return rc;
}

/** module_pproc_find_stream:
 *  @data1: pointer to mct_stream_t
 *  @data2: pointer to identity
 *
 *  This function compares identity of stream and identity
 *  passed to it. It is used to find the stream and validate
 *  it.
 *
 *  Return:
 *  TRUE if stream's identity matches with current identity
 *  FALSE otherwise
 **/
static boolean module_pproc_find_stream(void *data1, void *data2)
{
  mct_stream_t *stream = (mct_stream_t *)data1;
  uint32_t     *identity = (uint32_t *)data2;
  if (!data1 | !data2) {
    CDBG_ERROR("%s:%d failed data1 %p data2 %p\n", __func__, __LINE__,
      data1, data2);
    return FALSE;
  }
  CDBG("%s:%d stream id %x cur id %x\n", __func__, __LINE__,
    stream->streaminfo.identity, *identity);
  if (stream->streaminfo.identity == *identity) {
    return TRUE;
  }
  return FALSE;
}

static boolean module_pproc_call_caps_reserve(void *data1, void *data2)
{
  boolean            rc = TRUE;
  mct_port_t        *cpp_port = (mct_port_t *)data1;
  mct_stream_info_t *streaminfo = (mct_stream_info_t *)data2;
  if (!cpp_port || !streaminfo) {
    CDBG_ERROR("%s:%d failed cpp_port %p streaminfo %p\n", __func__, __LINE__,
      cpp_port, streaminfo);
    return FALSE;
  }
  rc = cpp_port->check_caps_reserve(cpp_port, NULL, streaminfo);
  return rc;
}

static boolean module_pproc_offline_streamon(mct_module_t *module,
  mct_event_t *event, void *data)
{
  boolean            rc = TRUE;
  mct_module_t      *cpp = NULL;
  mct_stream_info_t *streaminfo = (mct_stream_info_t *)data;
  mct_list_t        *clist = NULL;
  mct_port_t        *cport = NULL;
  uint32_t          *identity = NULL;

  CDBG("%s:%d E streaming mode %d w*h %d*%d\n", __func__, __LINE__,
    streaminfo->streaming_mode, streaminfo->dim.width, streaminfo->dim.height);
  cpp = pproc_modules_list[PPROC_CPP].mod;
  if (!cpp || !streaminfo) {
    CDBG("%s:%d failed cpp %p streaminfo %p\n", __func__, __LINE__, cpp,
      streaminfo);
    return FALSE;
  }
  /* caps reserve CPP port */
  clist = mct_list_find_custom(MCT_MODULE_SINKPORTS(cpp), streaminfo,
    module_pproc_call_caps_reserve);
  if (!clist) {
    CDBG_ERROR("%s:%d caps reserve failed\n", __func__, __LINE__);
    return FALSE;
  }
  /* ext link CPP port */
  cport = (mct_port_t *)clist->data;
  if (!cport) {
    CDBG_ERROR("%s:%d cpp port NULL\n", __func__, __LINE__);
    return FALSE;
  }
  identity = malloc(sizeof(uint32_t));
  if (!identity) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }
  *identity = streaminfo->identity;
  CDBG("%s:Port match, identity:%d\n", __func__, *identity);

  MCT_PORT_CHILDREN(cport) = mct_list_append(MCT_PORT_CHILDREN(cport),
    identity, NULL, NULL);

  rc = MCT_PORT_EXTLINKFUNC(cport)(streaminfo->identity, cport, cport);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return rc;
  }
  /* Call stream ON on cpp port */
  rc = MCT_PORT_EVENT_FUNC(cport)(cport, event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return rc;
  }
  CDBG("%s:%d X %d\n", __func__, __LINE__, rc);
  return rc;
}

static boolean module_pproc_offline_streamoff(mct_module_t *module,
  mct_event_t *event, void *data)
{
  boolean            rc = TRUE;
  mct_module_t      *cpp = NULL;
  mct_stream_info_t *streaminfo = (mct_stream_info_t *)data;
  mct_list_t        *clist = NULL;
  mct_port_t        *cport = NULL;
  unsigned int      *my_identity = NULL;
  mct_list_t        *identity_holder = NULL;

  CDBG("%s:%d E\n", __func__, __LINE__);
  cpp = pproc_modules_list[PPROC_CPP].mod;
  if (!cpp || !streaminfo) {
    CDBG("%s:%d failed cpp %p streaminfo %p\n", __func__, __LINE__, cpp,
      streaminfo);
    return FALSE;
  }
  clist = mct_list_find_custom(MCT_MODULE_SINKPORTS(cpp), &streaminfo->identity,
    module_pproc_find_identity_match_port);
  if (!clist) {
    CDBG_ERROR("%s:%d caps reserve failed\n", __func__, __LINE__);
    return FALSE;
  }
  cport = (mct_port_t *)clist->data;
  if (!cport) {
    CDBG_ERROR("%s:%d cpp port NULL\n", __func__, __LINE__);
    return FALSE;
  }
  /* Call stream OFF on cpp port */
  rc = MCT_PORT_EVENT_FUNC(cport)(cport, event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return rc;
  }
  /* Call unlink on CPP port */
  MCT_PORT_EXTUNLINKFUNC(cport)(streaminfo->identity, cport, cport);
  /* caps unreserve CPP port */
  cport->check_caps_unreserve(cport, streaminfo->identity);
  identity_holder = mct_list_find_custom(MCT_PORT_CHILDREN(cport),
    &streaminfo->identity, port_pproc_common_match_identity);
  if (identity_holder) {
    my_identity = (unsigned int *)identity_holder->data;
    MCT_PORT_CHILDREN(cport) = mct_list_remove(
      MCT_PORT_CHILDREN(cport), my_identity);
    free(my_identity);
  }

  CDBG("%s:%d X %d\n", __func__, __LINE__, rc);
  return rc;
}

static boolean module_pproc_offline_stream_param_buf(mct_module_t *module,
  mct_event_t *event, void *data)
{
  boolean       rc = TRUE;
  mct_module_t *cpp = NULL;
  mct_list_t   *clist = NULL;
  mct_port_t   *cport = NULL;

  CDBG("%s:%d E\n", __func__, __LINE__);
  cpp = pproc_modules_list[PPROC_CPP].mod;
  if (!cpp) {
    CDBG("%s:Null: %p\n", __func__, cpp);
    return FALSE;
  }
  clist = mct_list_find_custom(MCT_MODULE_SINKPORTS(cpp), &event->identity,
    module_pproc_find_identity_match_port);
  if (!clist) {
    CDBG_ERROR("%s:%d caps reserve failed\n", __func__, __LINE__);
    return FALSE;
  }
  /* ext link CPP port */
  cport = (mct_port_t *)clist->data;
  if (!cport) {
    CDBG_ERROR("%s:%d cpp port NULL\n", __func__, __LINE__);
    return FALSE;
  }
  /* Call STREAM PARAM on cpp port */
  rc = MCT_PORT_EVENT_FUNC(cport)(cport, event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return rc;
  }
  CDBG("%s:%d X %d\n", __func__, __LINE__, rc);
  return rc;
}

static boolean module_pproc_mct_event_handler(mct_module_t *module,
  mct_port_t *port, mct_event_t *event)
{
  boolean       rc = TRUE;
  mct_stream_t *stream = NULL;
  mct_list_t   *stream_list = NULL;

  CDBG("%s:%d offcpp identity %x\n", __func__, __LINE__, event->identity);
  stream_list = mct_list_find_custom(MCT_MODULE_PARENT(module),
    &event->identity, module_pproc_find_stream);
  if (!stream_list) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }
  stream = (mct_stream_t *)stream_list->data;
  if (!stream) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }

  CDBG("%s:%d stream type %d\n", __func__, __LINE__,
    stream->streaminfo.stream_type);
  if (stream->streaminfo.stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) {
    CDBG_ERROR("%s:%d event not handled!!!\n", __func__, __LINE__);
    return TRUE;
  }
  CDBG("event type %d", event->type);
  switch (event->type) {
  case MCT_EVENT_CONTROL_CMD: {
    mct_event_control_t *ctrl_event = &event->u.ctrl_event;
    CDBG("%s:%d ctrl event type %d\n", __func__, __LINE__,
      ctrl_event->type);
    switch (ctrl_event->type) {
    case MCT_EVENT_CONTROL_STREAMON:
      rc = module_pproc_offline_streamon(module, event,
        ctrl_event->control_event_data);
      break;
    case MCT_EVENT_CONTROL_STREAMOFF:
      rc = module_pproc_offline_streamoff(module, event,
        ctrl_event->control_event_data);
      break;
    case MCT_EVENT_CONTROL_PARM_STREAM_BUF:
      rc = module_pproc_offline_stream_param_buf(module, event,
        ctrl_event->control_event_data);
      break;
    default:
      break;
    }
    break;
  }
  case MCT_EVENT_MODULE_EVENT:
  default:
    break;
  }
  return rc;
}

static boolean module_pproc_event_handler(mct_module_t *module,
  mct_port_t *port, void *data)
{
  mct_module_t *cpp = NULL;
  pproc_event_t *pproc_event = (pproc_event_t *)data;
  mct_list_t *list_port = NULL;
  boolean rc = FALSE;

  switch (pproc_event->type) {
  case PPROC_EVENT_TYPE_PRIVATE:
    rc = module_pproc_priv_event_handler(module, port,
      &pproc_event->u.pproc_event);
    break;
  case PPROC_EVENT_MCT_EVENT: {
    rc = module_pproc_mct_event_handler(module, port,
      pproc_event->u.mct_event);
    break;
  }
  default:
    break;
  }

  return rc;
}

/** module_pproc_process_event: process event for sensor module
 *
 *  @streamid: streamid associated with event
 *  @module: mct module handle
 *  @event: event to be processed
 *
 *  Return: 0 for success and negative error on failure
 *
 *  This function handles all events and sends those events
 *  downstream / upstream *   */

static boolean module_pproc_process_event(mct_module_t *module,
  mct_event_t *event)
{
  boolean rc = FALSE;
  pproc_event_t pproc_event;
  mct_port_t *port = NULL;
  pproc_event.type = PPROC_EVENT_MCT_EVENT;
  pproc_event.u.mct_event = event;
  CDBG("%s:%d Enter\n", __func__, __LINE__);
  if (!module || !event) {
    CDBG_ERROR("%s:%d failed port %p event %p\n", __func__, __LINE__, module,
      event);
    return FALSE;
  }

  rc = module_pproc_event_handler(module, NULL, &pproc_event);

  CDBG("%s:%d Exit rc%d\n", __func__, __LINE__, rc);
  return rc;
}

static boolean module_pproc_submodules_unlink(mct_module_t *module,
  unsigned int identity, mct_port_t *peer)
{
  module_pproc_ctrl_t *pproc_ctrl =
    MODULE_PPROC_CTRL_CAST(MODULE_PPROC_PRIVATE_DATA(module));
  mct_list_t *list_port = NULL;
  mct_port_t *port = NULL;
  mct_module_t *cpp = NULL;

  CDBG("%s:Enter\n", __func__);
  cpp = pproc_modules_list[PPROC_CPP].mod;
  if (!cpp) {
    CDBG("%s:Null: %p\n", __func__, cpp);
    return FALSE;
  }
  list_port = mct_list_find_custom(cpp->sinkports, &identity,
    module_pproc_find_identity_match_port);

  if (list_port) {
      CDBG("%s: Identity match port found\n", __func__);
      port = (mct_port_t *)list_port->data;
      port->un_link(identity, port, peer);
  }

  CDBG("%s:Exit\n", __func__);
  return TRUE;
}

static boolean module_pproc_submodules_link(mct_module_t *module,
  unsigned int identity, mct_port_t *peer)
{
  module_pproc_ctrl_t *pproc_ctrl =
    MODULE_PPROC_CTRL_CAST(MODULE_PPROC_PRIVATE_DATA(module));
  mct_list_t *list_port = NULL;
  mct_port_t *port = NULL;
  boolean ret = FALSE;
  mct_module_t *cpp = NULL;

  CDBG("%s:Enter\n", __func__);
  cpp = pproc_modules_list[PPROC_CPP].mod;
  if (!cpp) {
    CDBG("%s:Null: %p\n", __func__, cpp);
    return FALSE;
  }
  list_port = mct_list_find_custom(cpp->sinkports, &identity,
    module_pproc_find_identity_match_port);

  if (list_port) {
      CDBG("%s: Identity match port found\n", __func__);
      port = (mct_port_t *)list_port->data;
      ret = port->ext_link(identity, port, peer);
  }

  CDBG("%s:Exit Ret:%d\n", __func__, ret);
  return ret;
}
/** module_pproc_set_mod
 *
 *
 *
 **/
static void module_pproc_set_mod(mct_module_t *module, unsigned int module_type,
  unsigned int identity)
{
  CDBG("%s:%d Enter\n", __func__, __LINE__);
  module->type = (mct_module_type_t)module_type;
  mct_module_set_process_event_func(module,
    module_pproc_process_event);
  CDBG("%s:%d Exit\n", __func__, __LINE__);
  return;
}

/** module_pproc_deinit
 *
 *
 *
 **/
void module_pproc_deinit(mct_module_t *mod)
{
  /* TODO */
}

/** module_pproc_query_mod:
 *    @query_buf:
 *    @session: session index
 *
 *  This should query each of sub-modules
 **/
static boolean module_pproc_query_mod(mct_module_t *module,
  void *query_buf, unsigned int sessionid)
{
  boolean                          ret = FALSE;
  mct_pipeline_cap_t              *pipeline_caps =
    (mct_pipeline_cap_t *)query_buf;
  uint32_t                          i = 0;

  if (!query_buf || !module) {
    CDBG_ERROR("%s:%d failed query_buf %p s_module %p\n", __func__, __LINE__,
      query_buf, module);
    return ret;
  }

  memset(&pipeline_caps->pp_cap, 0, sizeof(mct_pipeline_pp_cap_t));

  /* Traverse through active submodule module list and merge
     the capabilities */
  for (i = 0; i < (int32_t)(sizeof(pproc_modules_list) /
    sizeof(mct_pproc_init_name_t)); i++) {
    if (pproc_modules_list[i].mod) {
      pproc_modules_list[i].mod->query_mod(pproc_modules_list[i].mod,
        (void *)&pipeline_caps->pp_cap, sessionid);
    }
  }

  return TRUE;
}

/** module_pproc_request_new_port
 *    @mod: postprocessing module
 *
 *  This function is used to create port dynamically.
 **/
static mct_port_t * module_pproc_request_new_port(void *data,
  mct_port_direction_t direction, mct_module_t *mod,
  void *peer_caps)
{
  /* TODO */
  return NULL;
}
#if 0
/** module_pproc_create_default_ports 
 *    @mod: postprocessing module 
 *
 *  By default, postprocessing module creates one sink port
 *  and one source port. It supports dynamically added new
 *  ports through module_pproc_request_new_port.
 **/
static boolean module_pproc_create_default_ports(mct_module_t *mod)
{
  mct_port_t *sink, *src;

  sink = mct_port_create("sink0");
  if (!sink)
    return FALSE;

  if (port_pproc_init(sink) == FALSE ||
    mct_module_add_port(mod, sink) == FALSE)
    goto sink_port_fail;

  src = mct_port_create("src0");
  if (!src)
    goto src_port_create_fail;

  if (port_pproc_init(src) == FALSE ||
    (mct_module_add_port(mod, src) == FALSE))
    goto src_port_init_fail;

  return TRUE;

src_port_init_fail:
  mct_port_destroy(src);
src_port_create_fail:
  mct_module_remove_port(mod, sink);
sink_port_fail:
  mct_port_destroy(sink);
  return FALSE;
}
#endif

/** module_pproc_create_port:
 *    @module: pointer to module (mct_module_t)
 *    @name: port name
 *    @direction: port's direction (MCT_PORT_SINK /
 *              MCT_PORT_SRC)
 *
 *  This function creates mct port and adds it to module
 *
 *  This function executes in callers' context
 *
 *  Return:
 *  Success: TRUE
 *  Failure: FALSE
 **/
static boolean module_pproc_create_port(mct_module_t *module,
  const char *name, mct_port_direction_t direction,
  cam_streaming_mode_t mode)
{
  int32_t                      rc = 0;
  mct_port_t                  *port = NULL;

  port = mct_port_create(name);
  if (!port) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }

  if (port_pproc_init(port, direction, mode) == FALSE ||
      mct_module_add_port(module, port) == FALSE) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    goto ERROR1;
  }

  return TRUE;

ERROR2:
  mct_module_remove_port(module, port);
ERROR1:
  mct_port_destroy(port);
  return FALSE;
}

/** module_pproc_common_create_default_ports:
 *    @mod: cpp module 
 *
 *  By default, pproc submodule creates one sink port and one
 *  source port. It supports dynamically added new ports through
 *  module_cpp_request_new_port.
 *
 *  This function executes in callers' context
 *
 *  Return Success if all ports are created else Failure
 **/
static boolean module_pproc_create_default_ports(mct_module_t *mod)
{
  int32_t rc = 0;
  char    name[20];

  /* Create sink port 0 */
  snprintf(name, sizeof(name), "sink0");
  rc = module_pproc_create_port(mod, name, MCT_PORT_SINK,
    CAM_STREAMING_MODE_CONTINUOUS);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    goto ERROR1;
  }

  /* Create sink port 1 */
  snprintf(name, sizeof(name), "sink1");
  rc = module_pproc_create_port(mod, name, MCT_PORT_SINK,
    CAM_STREAMING_MODE_BURST);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    goto ERROR1;
  }

  /* Create src port 0 */
  snprintf(name, sizeof(name), "src0");
  rc = module_pproc_create_port(mod, name, MCT_PORT_SRC,
    CAM_STREAMING_MODE_CONTINUOUS);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    goto ERROR2;
  }

  /* Create src port 1 */
  snprintf(name, sizeof(name), "src1");
  rc = module_pproc_create_port(mod, name, MCT_PORT_SRC,
    CAM_STREAMING_MODE_BURST);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    goto ERROR2;
  }

  return TRUE;

ERROR2:
  mct_list_traverse(mod->srcports, module_pproc_destroy_port, mod);
ERROR1:
  mct_list_traverse(mod->sinkports, module_pproc_destroy_port, mod);
  return FALSE;
}

/** module_pproc_submodule_deinit:
 *  @list_data: data from the list
 *  @user_data: data passed by
 *
 *  Return: TRUE on success. FALSE if error is encountered.
 *
 *  This function deinits internal mct_module_t contained in
 *  pproc module **/
boolean module_pproc_submodule_deinit(void *list_data)
{
  mct_list_t   *list = (mct_list_t *)list_data;
  mct_module_t *module = NULL;
  uint32_t      i = 0;
  if (!list) {
    CDBG_ERROR("%s:%d failed list NULL\n", __func__, __LINE__);
    /* Return TRUE, otherwise mct_list_traverse will terminate and other
       modules will not be de-initialized */
    return TRUE;
  }
  module = (mct_module_t *)list->data;
  if (!module) {
    CDBG_ERROR("%s:%d failed module NULL\n", __func__, __LINE__);
    /* Return TRUE, otherwise mct_list_traverse will terminate and other
       modules will not be de-initialized */
    return TRUE;
  }
  /* Call deinit */
  for (i = 0; i < (int32_t)(sizeof(pproc_modules_list) /
    sizeof(mct_pproc_init_name_t)); i++) {
    if (pproc_modules_list[i].mod) {
      pproc_modules_list[i].deinit_mod(pproc_modules_list[i].mod);
    }
  }
  return TRUE;
}

/** module_pproc_submodule_start_session:
 *    @list_data: post proc sub-module
 *    @user_data: ID of session to be stopped.
 *
 *  Return: TRUE if session is started sucessfully. FALSE if
 *  error is encountered.
 *
 *  This function is called on the submodules when session is
 *  started. This function can open a session with hardware if
 *  any **/
static boolean module_pproc_submodule_start_session(
  mct_module_t *module, unsigned int sessionid)
{
  boolean rc = FALSE;
  CDBG("%s: module: = %p,name: %s, sessionid = 0x%x\n", __func__, module,
    MCT_MODULE_NAME(module), sessionid);
  rc = module->start_session(module, sessionid);
  CDBG("%s: module: = %p,name: %s, sessionid = 0x%x Exit\n", __func__, module,
    MCT_MODULE_NAME(module), sessionid);
  return rc;
}

/** module_pproc_submodule_stop_session:
 *    @module: pproc module
 *    @sessionid: ID of session to be stopped.
 *
 *  Return: TRUE if session is closed sucessfully. FALSE if
 *  error is encountered.
 *
 *  This function is called on the submodules when session is to
 *  be stopped. This function can close the session with
 *  hardware if any. **/
static boolean module_pproc_submodule_stop_session(
  void *list_data, unsigned int sessionid)
{
  boolean rc = FALSE;
  mct_module_t *module = (mct_module_t *)list_data;
  CDBG("%s: module: = %p,sessionid = 0x%x\n", __func__, module, sessionid);
  rc = module->stop_session(module,sessionid);
  return rc;
}

/** module_pproc_start_session:
 *    @module: pproc module
 *    @sessionid: ID of session to be started.
 *
 *  Return: TRUE if session is started sucessfully. FALSE if
 *  error is encountered.
 *
 *  This function is called when session is started. **/
static boolean module_pproc_start_session(
  mct_module_t *module, unsigned int sessionid)
{
  boolean             rc = FALSE;
  module_pproc_ctrl_t *pproc_ctrl =
    (module_pproc_ctrl_t *)module->module_private;
  uint32_t            i = 0;
  CDBG("%s: sessionid = 0x%x\n", __func__, sessionid);
  for (i = 0; i < (int32_t)(sizeof(pproc_modules_list) /
    sizeof(mct_pproc_init_name_t)); i++) {
    if (pproc_modules_list[i].mod) {
      module_pproc_submodule_start_session(pproc_modules_list[i].mod,
        sessionid);
    }
  }
  CDBG("%s: Exit sessionid = 0x%x\n", __func__, sessionid);
  return rc;
}

/** module_pproc_stop_session:
 *    @module: pproc module
 *    @sessionid: ID of session to be stopped.
 *
 *  Return: TRUE if session is closed sucessfully. FALSE if
 *  error is encountered.
 *
 *  This function is called when session is to be stopped. **/
static boolean module_pproc_stop_session(
  mct_module_t *module, unsigned int sessionid)
{
  boolean rc = FALSE;
  module_pproc_ctrl_t *pproc_ctrl =
    (module_pproc_ctrl_t *)module->module_private;
  uint32_t i = 0;
  CDBG("%s: sessionid = 0x%x\n", __func__, sessionid);
  for (i = 0; i < (int32_t)(sizeof(pproc_modules_list) /
    sizeof(mct_pproc_init_name_t)); i++) {
    if (pproc_modules_list[i].mod) {
      module_pproc_submodule_stop_session(pproc_modules_list[i].mod,
        sessionid);
    }
  }
  return rc;
}

/** module_pproc_init:
 *    @name: module name("pproc")
 *
 *  Return: mct_module_t pointer corresponding to pproc, NULL if
 *  error is encountered.
 *
 *  This function creates mct_module_t for pproc module,
 *  creates internal mct_modules for individual modules, creates
 *  associated ports, fills capabilities and add them to the
 *  module **/
mct_module_t *module_pproc_init(const char *name)
{
  mct_module_t        *pproc_mod = NULL;
  module_pproc_ctrl_t *pproc_ctrl = NULL;
  int32_t              i = 0;

  CDBG("%s:%d E\n", __func__, __LINE__);
  if (strcmp(name, "pproc"))
    return NULL;

  /* create the pproc module */
  pproc_mod = mct_module_create(name);
  if (!pproc_mod)
    return NULL;

  MCT_OBJECT_LOCK(pproc_mod);

  /* Create pproc module private control structure */
  pproc_ctrl = malloc(sizeof(module_pproc_ctrl_t));
  if (!pproc_ctrl) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    MCT_OBJECT_UNLOCK(pproc_mod);
    goto module_pproc_init_fail;
  }
  memset(pproc_ctrl, 0, sizeof(module_pproc_ctrl_t));
  pproc_mod->module_private = (void *)pproc_ctrl;

  /* initialize pproc sub-modules */
  for (i = 0; i < (int32_t)(sizeof(pproc_modules_list) /
    sizeof(mct_pproc_init_name_t)); i++) {
    /* init_mod: load library and create port for the sub modules */
    pproc_modules_list[i].mod =
      pproc_modules_list[i].init_mod(pproc_modules_list[i].name);
    if (pproc_modules_list[i].mod) {
      CDBG("%s:%d %s init mod success\n", __func__, __LINE__,
        pproc_modules_list[i].name);
      /* set the pproc module as container of the sub modules */
      module_pproc_common_set_container(
        pproc_modules_list[i].mod->module_private, pproc_mod);
    }
  } /* for */

  mct_module_set_set_mod_func(pproc_mod, module_pproc_set_mod);
  mct_module_set_query_mod_func(pproc_mod, module_pproc_query_mod);
  mct_module_set_start_session_func(pproc_mod, module_pproc_start_session);
  mct_module_set_stop_session_func(pproc_mod, module_pproc_stop_session);
  mct_module_set_request_new_port_func(pproc_mod,
    module_pproc_request_new_port);
  pproc_ctrl->reserve_submod = module_pproc_submodules_reserve;
  pproc_ctrl->unreserve_submod = module_pproc_submodules_unreserve;
  pproc_ctrl->link_submod = module_pproc_submodules_link;
  pproc_ctrl->unlink_submod = module_pproc_submodules_unlink;
  pproc_ctrl->pproc_event_func = module_pproc_event_handler;

  MCT_OBJECT_UNLOCK(pproc_mod);

  if (module_pproc_create_default_ports(pproc_mod) == FALSE) {
    CDBG_ERROR("%s:%d pproc default port create failed\n",
      __func__, __LINE__);
    goto module_pproc_init_fail;
  }

  CDBG("%s:%d X\n", __func__, __LINE__);
  return pproc_mod;

module_pproc_init_fail:
  for (i = 0; i < (int32_t)(sizeof(pproc_modules_list) /
    sizeof(mct_pproc_init_name_t)); i++) {
    if (pproc_modules_list[i].mod) {
      module_pproc_submodule_deinit(pproc_modules_list[i].mod);
    }
  }
  if (MCT_OBJECT_CHILDREN(pproc_mod)) {
      /* TODO: Need to ensure port's private data is deallocated! */
      mct_list_free_all(pproc_mod->sinkports, module_pproc_destroy_port);
      mct_list_free_all(pproc_mod->srcports, module_pproc_destroy_port);
  }

  if (pproc_mod->module_private) {
      free(pproc_mod->module_private);
      pproc_mod->module_private = NULL;
  }

  mct_module_destroy(pproc_mod);
  CDBG("%s:%d X\n", __func__, __LINE__);
  return NULL;
}
