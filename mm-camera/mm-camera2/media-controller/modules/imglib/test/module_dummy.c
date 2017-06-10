/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#include <linux/media.h>
#include "mct_module.h"
#include "module_dummy.h"
#include "mct_stream.h"
#include "mct_port.h"

#undef CDBG
#define CDBG LOGE

#define DUMMY_INPUT_PATH "/data/imglib/test%d.yuv"
#define DUMMY_INPUT_COUNT 3

/**
 * STATIC function declarations
 **/
static mct_port_t *module_dummy_create_port(mct_module_t *p_mct_mod,
  mct_port_direction_t dir);

/**
 * Function: module_dummy_find_identity
 *
 * Description: This method is used to find the client
 *
 * Arguments:
 *   @p_data: data in the mct list
 *   @p_input: input data to be seeked
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_dummy_find_identity(void *p_data, void *p_input)
{
  uint32_t *p_identity = (uint32_t *)p_data;
  uint32_t identity = *((uint32_t *)p_input);

  return (*p_identity == identity) ? TRUE : FALSE;
}

/**
 * Function: module_dummy_port_event_func
 *
 * Description: Event handler function for the dummy port
 *
 * Arguments:
 *   @port: mct port pointer
 *   @event: mct event
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_dummy_port_event_func(mct_port_t *port,
  mct_event_t *event)
{
  int rc = IMG_SUCCESS;

  if (!port || !event) {
    CDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  CDBG("%s:%d] type %d", __func__, __LINE__, event->type);
  switch (event->type) {
  case MCT_EVENT_CONTROL_CMD: {
    mct_event_control_t *p_ctrl_event = &event->u.ctrl_event;
    CDBG("%s:%d] Ctrl type %d", __func__, __LINE__, p_ctrl_event->type);
    switch (p_ctrl_event->type) {
    case MCT_EVENT_CONTROL_STREAMON:
    case MCT_EVENT_CONTROL_STREAMOFF:
    default:
      break;
    }
    break;
  }
  case MCT_EVENT_MODULE_EVENT: {
    mct_event_module_t *p_mod_event = &event->u.module_event;
    CDBG("%s:%d] Mod type %d", __func__, __LINE__, p_mod_event->type);
    switch (p_mod_event->type) {
    default:
      break;
    }
    break;
  }
  default:
    /* forward the event */
    break;
  }
  return GET_STATUS(rc);
}

/**
 * Function: module_dummy_port_event_fwd_list
 *
 * Description: This method is used to forward the event list
 *
 * Arguments:
 *   @identity: identitity for the session and stream
 *   @port: mct port pointer
 *
 * Return values:
 *     list of events or NULL(during failure)
 *
 * Notes: none
 **/
mct_list_t *module_dummy_port_event_fwd_list(unsigned int identity,
  mct_port_t *port)
{
  return NULL;
}

/**
 * Function: module_dummy_port_ext_link
 *
 * Description: This method is called when the user establishes
 *              link.
 *
 * Arguments:
 *   @identity: identitity for the session and stream
 *   @port: mct port pointer
 *   @peer: peer mct port pointer
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_dummy_port_ext_link(unsigned int identity,
  mct_port_t* port, mct_port_t *peer)
{
  int rc = IMG_SUCCESS;
  unsigned int *p_identity = NULL;
  mct_list_t *p_temp_list = NULL;
  mct_module_t *p_mct_mod = NULL;
  module_dummy_t *p_mod = NULL;
  dummy_client_t *p_client = NULL;

  if (!port || !peer) {
    CDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  CDBG("%s:%d] E", __func__, __LINE__);
  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    CDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_dummy_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    CDBG_ERROR("%s:%d] dummy module NULL", __func__, __LINE__);
    return FALSE;
  }

  if (MCT_PORT_PEER(port)) {
    CDBG_ERROR("%s:%d] link already established", __func__, __LINE__);
    return FALSE;
  }

  MCT_PORT_PEER(port) = peer;

  return GET_STATUS(rc);

error:
  CDBG("%s:%d] Error X", __func__, __LINE__);
  return FALSE;
}

/**
 * Function: module_dummy_port_unlink
 *
 * Description: This method is called when the user disconnects
 *              the link.
 *
 * Arguments:
 *   @identity: identitity for the session and stream
 *   @port: mct port pointer
 *   @peer: peer mct port pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_dummy_port_unlink(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  int rc = IMG_SUCCESS;
  mct_list_t *p_temp_list = NULL;
  mct_module_t *p_mct_mod = NULL;
  module_dummy_t *p_mod = NULL;
  dummy_client_t *p_client = NULL;
  uint32_t *p_identity = NULL;

  if (!port || !peer) {
    CDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return;
  }

  CDBG("%s:%d] E", __func__, __LINE__);
  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    CDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return;
  }

  p_mod = (module_dummy_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    CDBG_ERROR("%s:%d] dummy module NULL", __func__, __LINE__);
    return;
  }

  p_client = (dummy_client_t *)port->port_private;
  if (NULL == p_client) {
    CDBG_ERROR("%s:%d] dummy client NULL", __func__, __LINE__);
    return;
  }

  MCT_PORT_PEER(port) = NULL;

  CDBG("%s:%d] X", __func__, __LINE__);
  return;

error:
  CDBG("%s:%d] Error X", __func__, __LINE__);
}

/**
 * Function: module_dummy_port_set_caps
 *
 * Description: This method is used to set the capabilities
 *
 * Arguments:
 *   @port: mct port pointer
 *   @caps: mct port capabilities
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_dummy_port_set_caps(mct_port_t *port,
  mct_port_caps_t *caps)
{
  int rc = IMG_SUCCESS;
  return GET_STATUS(rc);
}

/**
 * Function: module_dummy_port_check_caps_reserve
 *
 * Description: This function is used to reserve the port
 *
 * Arguments:
 *   @port: mct port pointer
 *   @peer_caps: pointer to peer capabilities
 *   @stream_info: stream information
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_dummy_port_check_caps_reserve(mct_port_t *port,
  void *peer_caps, mct_stream_info_t *stream_info)
{
  boolean rc = FALSE;
  mct_module_t *p_mct_mod = NULL;
  module_dummy_t *p_mod = NULL;

  CDBG("%s:%d] E", __func__, __LINE__);
  if (!port || !stream_info) {
    CDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    CDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_dummy_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    CDBG_ERROR("%s:%d] dummy module NULL", __func__, __LINE__);
    return FALSE;
  }


  CDBG("%s:%d] X", __func__, __LINE__);
  return TRUE;
}

/**
 * Function: module_dummy_port_check_caps_unreserve
 *
 * Description: This method is used to unreserve the port
 *
 * Arguments:
 *   @identity: identitity for the session and stream
 *   @port: mct port pointer
 *   @peer: peer mct port pointer
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_dummy_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  int rc = IMG_SUCCESS;
  mct_list_t *p_temp_list = NULL;
  mct_module_t *p_mct_mod = NULL;
  module_dummy_t *p_mod = NULL;
  dummy_client_t *p_client = NULL;
  uint32_t *p_identity = NULL;

  CDBG("%s:%d] E", __func__, __LINE__);
  if (!port) {
    CDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    CDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_dummy_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    CDBG_ERROR("%s:%d] dummy module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (dummy_client_t *)port->port_private;
  if (NULL == p_client) {
    CDBG_ERROR("%s:%d] dummy client NULL", __func__, __LINE__);
    return FALSE;
  }


  CDBG("%s:%d] X", __func__, __LINE__);
  return GET_STATUS(rc);

error:
  pthread_mutex_unlock(&p_mod->mutex);
  CDBG("%s:%d] Error rc = %d X", __func__, __LINE__, rc);
  return FALSE;
}

/**
 * Function: module_dummy_process_event
 *
 * Description: Event handler function for the dummy module
 *
 * Arguments:
 *   @streamid: stream id
 *   @p_mct_mod: mct module pointer
 *   @p_event: mct event
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
static boolean module_dummy_process_event(mct_module_t *module,
  mct_event_t *event)
{
  int rc = IMG_SUCCESS;
  CDBG("%s:%d] ", __func__, __LINE__);
  return GET_STATUS(rc);
}

/**
 * Function: module_dummy_request_new_port
 *
 * Description: This function is called by the mct framework
 *         when new port needs to be created
 *
 * Arguments:
 *   @stream_info: stream information
 *   @direction: direction of port
 *   @module: mct module pointer
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
mct_port_t *module_dummy_request_new_port(mct_stream_info_t *stream_info,
  mct_port_direction_t direction,
  mct_module_t *module,
  void *peer_caps)
{
  return NULL;
}

/**
 * Function: module_dummy_start_session
 *
 * Description: This function is called when a new camera
 *              session is started
 *
 * Arguments:
 *   @module: mct module pointer
 *   @sessionid: session id
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
static boolean module_dummy_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  int rc = IMG_SUCCESS;
  return GET_STATUS(rc);
}

/**
 * Function: module_dummy_start_session
 *
 * Description: This function is called when the camera
 *              session is stopped
 *
 * Arguments:
 *   @module: mct module pointer
 *   @sessionid: session id
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
static boolean module_dummy_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  int rc = IMG_SUCCESS;
  return GET_STATUS(rc);
}

/**
 * Function: module_dummy_set_mod
 *
 * Description: This function is used to set the dummy module
 *
 * Arguments:
 *   @module: mct module pointer
 *   @module_type: module type
 *   @identity: id of the stream
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static void module_dummy_set_mod(mct_module_t *module,
  unsigned int module_type,
  unsigned int identity)
{
  return;
}

/**
 * Function: module_dummy_query_mod
 *
 * Description: This function is used to query the dummy module info
 *
 * Arguments:
 *   @module: mct module pointer
 *   @query_buf: pipeline capability
 *   @sessionid: session identity
 *
 * Return values:
 *     success/failure
 *
 * Notes: none
 **/
static boolean module_dummy_query_mod(mct_module_t *module,
  mct_pipeline_cap_t *query_buf,
  unsigned int sessionid)
{
  return TRUE;
}

/**
 * Function: module_dummy_free_port
 *
 * Description: This function is used to free the dummy ports
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static boolean module_dummy_free_port(void *data, void *user_data)
{
  mct_port_t *p_port = (mct_port_t *)data;
  mct_module_t *p_mct_mod = (mct_module_t *)user_data;
  boolean rc = FALSE;

  CDBG("%s:%d port %p p_mct_mod %p", __func__, __LINE__, p_port,
    p_mct_mod);
  if (!p_port || !p_mct_mod) {
    CDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return TRUE;
  }
  rc = mct_module_remove_port(p_mct_mod, p_port);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed", __func__, __LINE__);
  }
  mct_port_destroy(p_port);
  return TRUE;
}

/**
 * Function: module_dummy_create_port
 *
 * Description: This function is used to create a port and link with the
 *              module
 *
 * Arguments:
 *   none
 *
 * Return values:
 *     MCTL port pointer
 *
 * Notes: none
 **/
mct_port_t *module_dummy_create_port(mct_module_t *p_mct_mod,
  mct_port_direction_t dir)
{
  char portname[PORT_NAME_LEN];
  mct_port_t *p_port = NULL;
  int status = IMG_SUCCESS;
  int index = 0;

  if (!p_mct_mod || (MCT_PORT_SRC != dir)) {
    CDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return NULL;
  }

  index = p_mct_mod->numsrcports;
  /*portname <mod_name>_direction_portIndex*/
  snprintf(portname, sizeof(portname), "%s_d%d_i%d",
    MCT_MODULE_NAME(p_mct_mod), dir, index);
  p_port = mct_port_create(portname);
  if (NULL == p_port) {
    CDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return NULL;
  }
  CDBG("%s:%d portname %s", __func__, __LINE__, portname);

  p_port->direction = dir;
  p_port->port_private = NULL;
  p_port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;
  p_port->caps.u.frame.format_flag = MCT_PORT_CAP_FORMAT_YCBCR;

  /* override the function pointers */
  p_port->check_caps_reserve    = module_dummy_port_check_caps_reserve;
  p_port->check_caps_unreserve  = module_dummy_port_check_caps_unreserve;
  p_port->ext_link              = module_dummy_port_ext_link;
  p_port->un_link               = module_dummy_port_unlink;
  p_port->set_caps              = module_dummy_port_set_caps;
  p_port->event_func            = module_dummy_port_event_func;

  /* add port to the module */
  if (!mct_module_add_port(p_mct_mod, p_port)) {
    CDBG_ERROR("%s: Set parent failed", __func__);
    status = IMG_ERR_GENERAL;
    goto error;
  }

  p_mct_mod->numsrcports++;

  CDBG("%s:%d ", __func__, __LINE__);
  return p_port;

error:

  CDBG_ERROR("%s:%d] failed", __func__, __LINE__);
  if (p_port) {
    mct_port_destroy(p_port);
    p_port = NULL;
  }
  return NULL;
}

/**
 * Function: module_dummy_free_mod
 *
 * Description: This function is used to free the dummy module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_dummy_deinit(mct_module_t *p_mct_mod)
{
  module_dummy_t *p_mod = NULL;
  img_core_ops_t *p_core_ops = NULL;
  int rc = 0;
  int i = 0;

  if (NULL == p_mct_mod) {
    CDBG_ERROR("%s:%d] MCTL module NULL", __func__, __LINE__);
    return;
  }

  p_mod = (module_dummy_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    CDBG_ERROR("%s:%d] dummy module NULL", __func__, __LINE__);
    return;
  }

  mct_list_traverse(MCT_MODULE_CHILDREN(p_mct_mod), module_dummy_free_port,
    p_mct_mod);

  /* free the lists */

  mct_list_free_list(MCT_MODULE_CHILDREN(p_mct_mod));
  p_mod->client_cnt = 0;
  mct_module_destroy(p_mct_mod);
  p_mct_mod = NULL;
}

/** module_dummy_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: This function is used to initialize the dummy module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_dummy_init(const char *name)
{
  mct_module_t *p_mct_mod = NULL;
  module_dummy_t *p_mod = NULL;
  img_core_ops_t *p_core_ops = NULL;
  mct_port_t *p_port = NULL;
  int rc = 0;
  int i = 0;

  p_mct_mod = mct_module_create(name);
  if (NULL == p_mct_mod) {
    CDBG_ERROR("%s:%d cannot allocate mct module", __func__, __LINE__);
    return NULL;
  }
  p_mod = malloc(sizeof(module_dummy_t));
  if (NULL == p_mod) {
    CDBG_ERROR("%s:%d failed", __func__, __LINE__);
    goto error;
  }

  p_mct_mod->module_private = (void *)p_mod;
  memset(p_mod, 0, sizeof(module_dummy_t));

  p_mod->fp_client = NULL;

  /* create ports */
  for (i = 0; i < MAX_DUMMY_STATIC_PORTS; i++) {
    p_port = module_dummy_create_port(p_mct_mod, MCT_PORT_SRC);
    if (NULL == p_port) {
      CDBG_ERROR("%s:%d] create port failed", __func__, __LINE__);
      goto error;
    }
  }
  /* set the default type*/
  p_mct_mod->type = MCT_MODULE_FLAG_SOURCE;

  p_mct_mod->process_event    = module_dummy_process_event;
  p_mct_mod->set_mod          = module_dummy_set_mod;
  p_mct_mod->query_mod        = module_dummy_query_mod;
  p_mct_mod->request_new_port = module_dummy_request_new_port;
  p_mct_mod->start_session    = module_dummy_start_session;
  p_mct_mod->stop_session     = module_dummy_stop_session;

  return p_mct_mod;

error:

  if (p_mod) {
    module_dummy_deinit(p_mct_mod);
  } else if (p_mct_mod) {
    mct_module_destroy(p_mct_mod);
    p_mct_mod = NULL;
  }
  return NULL;
}
