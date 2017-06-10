/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#include <linux/media.h>
#include "modules.h"
#include "mct_module.h"
#include "module_faceproc.h"
#include "mct_stream.h"
#include "mct_port.h"
#include "mct_pipeline.h"


#define FD_ENABLE(p) ((p->config.fd_feature_mask & FACE_PROP_ENABLE_FD) \
  && p->p_fd_chromatix->enable)

/**
 * STATIC function declarations
 **/
static mct_port_t *module_faceproc_create_port(mct_module_t *p_mct_mod,
  mct_port_direction_t dir);

/**
 * Function: module_faceproc_find_client
 *
 * Description: This method is used to find the client
 *
 * Arguments:
 *   @p_fp_data: faceproc client
 *   @p_input: input data
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_faceproc_find_client(void *p_fp_data, void *p_input)
{
  faceproc_client_t *p_client = (faceproc_client_t *)p_fp_data;
  uint32_t identity = *((uint32_t *)p_input);

  return (p_client->identity == identity) ? TRUE : FALSE;
}

/**
 * Function: module_faceproc_find_session_params
 *
 * Description: This method is used to find the the session based parameters
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
static boolean module_faceproc_find_session_params(void *p_data, void *p_input)
{

  faceproc_session_params_t *stored_param = p_data;
  uint32_t session_id = *((uint32_t *)p_input);

  return (stored_param->session_id == session_id) ? TRUE : FALSE;
}

/**
 * Function: module_faceproc_find_identity
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
static boolean module_faceproc_find_identity(void *p_data, void *p_input)
{
  uint32_t *p_identity = (uint32_t *)p_data;
  uint32_t identity = *((uint32_t *)p_input);

  return (*p_identity == identity) ? TRUE : FALSE;
}

/**
 * Function: module_faceproc_store_session_param
 *
 * Description: This method is used to store per session based parameters
 *
 * Arguments:
 *   @p_mod: Face proc private data
 *   @p_client: Client handle to apply the restored parameters
 *   @param: Event parameters which need to be stored
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static int module_faceproc_store_session_param(module_faceproc_t *p_mod,
  faceproc_client_t *p_client, mct_event_control_parm_t *param)
{
  if (!p_mod || !param)
    return IMG_ERR_INVALID_INPUT;

  switch(param->type) {
  case CAM_INTF_PARM_FD:
  case CAM_INTF_PARM_ZOOM: {
    uint32_t session_id = IMGLIB_SESSIONID(p_client->identity);
    mct_list_t *p_temp_list;
    faceproc_session_params_t *stored_param;

    if (NULL == param->parm_data)
      return IMG_ERR_INVALID_INPUT;

    /* Find settings per session id */
    p_temp_list = mct_list_find_custom(p_mod->session_parms, &session_id,
      module_faceproc_find_session_params);
    if (!p_temp_list)
      return IMG_ERR_GENERAL;

    stored_param = (faceproc_session_params_t *) p_temp_list->data;
    stored_param->valid_params = TRUE;

    if (CAM_INTF_PARM_FD == param->type) {
      cam_fd_set_parm_t *p_fd_set_parm = (cam_fd_set_parm_t *)param->parm_data;
      int *p_zoom_val = param->parm_data;
      stored_param->param.fd_enable = FALSE;

      if (CAM_FACE_PROCESS_MASK_DETECTION & p_fd_set_parm->fd_mode)
        stored_param->param.fd_enable = TRUE;

      stored_param->param.fr_enable = FALSE;
      if (CAM_FACE_PROCESS_MASK_RECOGNITION & p_fd_set_parm->fd_mode)
        stored_param->param.fr_enable = TRUE;
    } else if (CAM_INTF_PARM_ZOOM == param->type) {
      int *p_zoom_val = (int *)param->parm_data;
      stored_param->param.zoom_val = *p_zoom_val;
    }

    break;
  }
  default:
    break;
  }
  return IMG_SUCCESS;
}

/**
 * Function: module_faceproc_restore_session_param
 *
 * Description: This method is used to restore per session based parameters
 *
 * Arguments:
 *   @p_mod: Face proc private data
 *   @p_client: Client handle to apply the restored parameters
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static int module_faceproc_restore_session_param(module_faceproc_t *p_mod,
  faceproc_client_t *p_client)
{
  mct_list_t *p_temp_list;
  faceproc_session_params_t *stored_param;
  uint32_t session_id;

  if (!p_mod || !p_client)
    return IMG_ERR_INVALID_INPUT;

  session_id = IMGLIB_SESSIONID(p_client->identity);

  /* Find settings per session id */
  p_temp_list = mct_list_find_custom(p_mod->session_parms, &session_id,
    module_faceproc_find_session_params);
  if (!p_temp_list)
    return IMG_ERR_GENERAL;

  stored_param = p_temp_list->data;
  if (!stored_param)
    return IMG_ERR_GENERAL;

  /* Do not restore session modes when client is in face register mode */
  if ((FALSE == stored_param->valid_params) || (p_client->mode == FACE_REGISTER))
    return IMG_SUCCESS;

  if (stored_param->param.fr_enable)
    module_faceproc_client_set_mode(p_client, FACE_RECOGNIZE);
  else if (stored_param->param.fd_enable)
    module_faceproc_client_set_mode(p_client, FACE_DETECT);

  p_client->zoom_val = stored_param->param.zoom_val;
  return IMG_SUCCESS;
}

/**
 * Function: module_faceproc_create_session_param
 *
 * Description: This method is used to create session parameters
 * it will add new list with parameters session based
 *
 * Arguments:
 *   @p_mod: Face proc private data
 *   @session_id Session id
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static int module_faceproc_create_session_param(module_faceproc_t *p_mod,
  uint32_t session_id)
{
  faceproc_session_params_t *stored_param;

  if (!p_mod)
    return IMG_ERR_INVALID_INPUT;

  stored_param = malloc(sizeof(*stored_param));
  if (NULL == stored_param)
    return IMG_ERR_NO_MEMORY;

  memset(stored_param, 0x0, sizeof(faceproc_session_params_t));
  stored_param->session_id = session_id;
  stored_param->valid_params = FALSE;
  stored_param->param.fd_enable = FALSE;
  stored_param->param.fr_enable = FALSE;
  stored_param->param.zoom_val = 0;

  p_mod->session_parms = mct_list_append(p_mod->session_parms,
    stored_param, NULL, NULL);

  return (p_mod->session_parms) ? IMG_SUCCESS : IMG_ERR_GENERAL;
}

/**
 * Function: module_faceproc_destroy_session_param
 *
 * Description: This method is used to destroy session parameters
 * it will add new list with parameters session based
 *
 * Arguments:
 *   @p_mod: Face proc private data
 *   @session_id Session id
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static int module_faceproc_destroy_session_param(module_faceproc_t *p_mod,
  uint32_t session_id)
{
  faceproc_session_params_t *stored_param;
  mct_list_t *p_temp_list;

  if (!p_mod)
    return IMG_ERR_INVALID_INPUT;

  /* Find paramters per session id */
  p_temp_list = mct_list_find_custom(p_mod->session_parms, &session_id,
    module_faceproc_find_session_params);
  if (!p_temp_list)
    return IMG_ERR_INVALID_INPUT;


  stored_param = ( faceproc_session_params_t *)p_temp_list->data;
  p_mod->session_parms = mct_list_remove(p_mod->session_parms, stored_param);
  free(stored_param);

  return IMG_SUCCESS;
}

/**
 * Function: module_faceproc_acquire_port
 *
 * Description: This function is used to acquire the port
 *
 * Arguments:
 *   @p_mct_mod: mct module pointer
 *   @port: mct port pointer
 *   @stream_info: stream information
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_faceproc_acquire_port(mct_module_t *p_mct_mod,
  mct_port_t *port,
  mct_stream_info_t *stream_info)
{
  int rc = IMG_SUCCESS;
  unsigned int p_identity ;
  mct_list_t *p_temp_list = NULL;
  faceproc_client_t *p_client = NULL;
  module_faceproc_t *p_mod = NULL;

  IDBG_MED("%s:%d] E", __func__, __LINE__);

  p_mod = (module_faceproc_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] faceproc module NULL", __func__, __LINE__);
    return FALSE;
  }
  p_identity =  stream_info->identity;

  /* check if its sink port*/
  if (MCT_PORT_IS_SINK(port)) {
    /* create faceproc client */
    rc = module_faceproc_client_create(p_mct_mod, port, p_identity, stream_info);
    if (IMG_SUCCEEDED(rc)) {
      /* add the client to the list */
      p_client = (faceproc_client_t *) port->port_private;
      p_temp_list = mct_list_append(p_mod->fp_client, p_client, NULL, NULL);
      if (NULL == p_temp_list) {
        IDBG_ERROR("%s:%d] list append failed", __func__, __LINE__);
        rc = IMG_ERR_GENERAL;
        goto error;
      }
      p_mod->fp_client = p_temp_list;
    }
  } else {
    /* update the internal connection with source port */
    p_temp_list = mct_list_find_custom(p_mod->fp_client, &p_identity,
      module_faceproc_find_client);
    if (NULL != p_temp_list) {
      p_client = p_temp_list->data;
      p_client->p_srcport = port;
      port->port_private = p_client;
      IDBG_MED("%s:%d] found client %p", __func__, __LINE__, p_client);
    } else {
      IDBG_ERROR("%s:%d] cannot find the client", __func__, __LINE__);
      goto error;
    }
  }
  IDBG_MED("%s:%d] port %p port_private %p X", __func__, __LINE__,
    port, port->port_private);
  return GET_STATUS(rc);

error:

  IDBG_MED("%s:%d] Error X", __func__, __LINE__);
  return FALSE;
}

/**
 * Function: module_faceproc_handle_streamon
 *
 * Description: Function to handle faceproc streamon
 *
 * Arguments:
 *   @p_mod: faceproc module pointer
 *   @p_client: faceproc client pointer
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
int module_faceproc_handle_streamon(module_faceproc_t *p_mod,
  faceproc_client_t *p_client)
{
  int rc;

  if (p_client->state != IMGLIB_STATE_INIT) {
     IDBG_ERROR("%s:%d] client not in init state %d", __func__, __LINE__,
       p_client->state);
     return IMG_SUCCESS;
  }

  rc = module_faceproc_client_start(p_client);
  if (IMG_SUCCEEDED(rc)) {
    rc = module_faceproc_client_map_buffers(p_client);
  } else {
    IDBG_ERROR("%s:%d] Error cannot start %d", __func__, __LINE__, rc);
    return rc;
  }
  if (FACE_DETECT_OFF != p_client->mode) {
    p_client->state = IMGLIB_STATE_PROCESSING;
  }
  return rc;
}

/**
 * Function: module_faceproc_handle_streamoff
 *
 * Description: Function to handle faceproc streamoff
 *
 * Arguments:
 *   @p_mod: faceproc module pointer
 *   @p_client: faceproc client pointer
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
int module_faceproc_handle_streamoff(module_faceproc_t *p_mod,
  faceproc_client_t *p_client)
{
  int rc;

  if ((p_client->state != IMGLIB_STATE_STARTED) &&
    (p_client->state != IMGLIB_STATE_PROCESSING)) {
     IDBG_ERROR("%s:%d] client not started state %d", __func__, __LINE__,
       p_client->state);
     return IMG_SUCCESS;
  }

  rc = module_faceproc_client_stop(p_client);
  if (IMG_SUCCEEDED(rc))
    rc = module_faceproc_client_unmap_buffers(p_client);
  else
    IDBG_ERROR("%s:%d] Error cannot stop %d", __func__, __LINE__, rc);
  return rc;
}

/**
 * Function: module_faceproc_port_event_func
 *
 * Description: Event handler function for the faceproc port
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
boolean module_faceproc_port_event_func(mct_port_t *port,
  mct_event_t *event)
{
  int rc = IMG_SUCCESS;
  mct_module_t *p_mct_mod = NULL;
  module_faceproc_t *p_mod = NULL;
  faceproc_client_t *p_client;

  if (!port || !event) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  IDBG_LOW("%s:%d] port %p E", __func__, __LINE__, port);
  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_faceproc_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] faceproc module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (faceproc_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] faceproc client NULL", __func__, __LINE__);
    return FALSE;
  }

  IDBG_LOW("%s:%d] type %d", __func__, __LINE__, event->type);
  switch (event->type) {
  case MCT_EVENT_CONTROL_CMD: {
    mct_event_control_t *p_ctrl_event = &event->u.ctrl_event;
    IDBG_LOW("%s:%d] Ctrl type %d", __func__, __LINE__, p_ctrl_event->type);
    switch (p_ctrl_event->type) {
    case MCT_EVENT_CONTROL_STREAMON: {
      if (!FD_ENABLE(p_client)) {
        /* hack to prevent faceproc from execution*/
        IDBG_HIGH("%s:%d] ###Disable faceproc", __func__, __LINE__);
        p_mod->active = TRUE;
      }
      /* restore the parameters */
      module_faceproc_restore_session_param(p_mod, p_client);
      pthread_mutex_lock(&p_mod->mutex);
      pthread_mutex_lock(&p_client->mutex);
      if (!p_mod->active) {
        p_client->active = TRUE;
        p_mod->active = TRUE;

        IDBG_HIGH("%s:%d] STREAMON %d", __func__, __LINE__, p_client->mode);
        p_client->streamon = TRUE;
        rc = module_faceproc_handle_streamon(p_mod, p_client);
      } else {
        IDBG_HIGH("%s:%d] STREAMON Not active", __func__, __LINE__);
        p_client->active = FALSE;
      }
      pthread_mutex_unlock(&p_client->mutex);
      pthread_mutex_unlock(&p_mod->mutex);
      break;
    }
    case MCT_EVENT_CONTROL_STREAMOFF: {
      pthread_mutex_lock(&p_mod->mutex);
      pthread_mutex_lock(&p_client->mutex);
      if (!p_client->active) {
        IDBG_HIGH("%s:%d] STREAMOFF Not active", __func__, __LINE__);
      } else {
        IDBG_HIGH("%s:%d] STREAMOFF %d", __func__, __LINE__, p_client->mode);
        p_client->streamon = FALSE;
        rc = module_faceproc_handle_streamoff(p_mod, p_client);
        p_client->active = FALSE;
        p_mod->active = FALSE;
      }
      pthread_mutex_unlock(&p_client->mutex);
      pthread_mutex_unlock(&p_mod->mutex);
      break;
    }
    case MCT_EVENT_CONTROL_SET_PARM: {
      rc = module_faceproc_client_handle_ctrl_parm(p_client,
        p_ctrl_event->control_event_data);
      if (IMG_SUCCEEDED(rc))
        module_faceproc_store_session_param(p_mod, p_client,
          p_ctrl_event->control_event_data);
      break;
    }

    case MCT_EVENT_CONTROL_PARM_STREAM_BUF: {
      cam_stream_parm_buffer_t *parm_buf;

      if (!event->u.ctrl_event.control_event_data) {
        IDBG_ERROR("%s:%d] Invalid input argument ", __func__, __LINE__);
        rc = IMG_ERR_INVALID_INPUT;
        break;
      }
      parm_buf = event->u.ctrl_event.control_event_data;
      if (parm_buf->type != CAM_STREAM_PARAM_TYPE_DO_REPROCESS) {
        IDBG_ERROR("%s:%d] Invalid type for reprocess %x %x",
          __func__, __LINE__,
          event->identity, p_client->identity);
        break;
      }

      /* Start with face recognition process */
      parm_buf->reprocess.ret_val = -1;
      rc = module_faceproc_client_map_buffers(p_client);
      if (IMG_ERROR(rc))
        break;
      rc = module_faceproc_client_process_buffers(p_client,
          parm_buf->reprocess.frame_idx);
      if (IMG_ERROR(rc))
        break;
      if (p_client->result[0].num_faces_detected)
        parm_buf->reprocess.ret_val = p_client->result[0].roi[0].unique_id;


      break;
    }
    default:
      break;
    }
    break;
  }
  case MCT_EVENT_MODULE_EVENT: {
    mct_event_module_t *p_mod_event = &event->u.module_event;
    img_component_ops_t *p_comp = &p_client->comp;
    IDBG_LOW("%s:%d] Mod type %d", __func__, __LINE__, p_mod_event->type);
    switch (p_mod_event->type) {
    case MCT_EVENT_MODULE_BUF_DIVERT: {
      mod_img_msg_t msg;
      isp_buf_divert_t *p_buf_divert =
        (isp_buf_divert_t *)p_mod_event->module_event_data;

      module_faceproc_client_handle_buffer(p_client,
        p_buf_divert->buffer.index,
        p_buf_divert->buffer.sequence);

      /* indicate that the buffer is consumed */
      p_buf_divert->is_locked = FALSE;
      p_buf_divert->ack_flag = TRUE;

      if (p_buf_divert->is_locked) {
        memset(&msg, 0x0, sizeof(mod_img_msg_t));
        msg.port = port;
        msg.type = MOD_IMG_MSG_BUF_ACK;
        msg.data.buf_ack.frame_id = p_buf_divert->buffer.index;
        msg.data.buf_ack.identity = p_client->identity;
        module_imglib_send_msg(&p_mod->msg_thread, &msg);
      }
      break;
    }
    case MCT_EVENT_MODULE_SET_STREAM_CONFIG: {
      sensor_out_info_t *sensor_info;
      sensor_info = (sensor_out_info_t *)p_mod_event->module_event_data;
      IDBG_MED("%s:%d] MCT_EVENT_MODULE_SET_STREAM_CONFIG, w = %u, h = %u",
        __func__, __LINE__,
        sensor_info->dim_output.width,
        sensor_info->dim_output.height);
      p_client->main_dim.width = sensor_info->dim_output.width;
      p_client->main_dim.height = sensor_info->dim_output.height;
      break;
    }

    case MCT_EVENT_MODULE_STREAM_CROP: {
      mct_bus_msg_stream_crop_t *s_crop =
          (mct_bus_msg_stream_crop_t *)p_mod_event->module_event_data;

      rc = module_faceproc_client_set_scale_ratio(p_client, s_crop);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("%s:%d] Can not set scale ratio ", __func__, __LINE__);
      }
      break;
    }
    case MCT_EVENT_MODULE_QUERY_DIVERT_TYPE: {
      uint32_t *divert_mask = (uint32_t *)p_mod_event->module_event_data;
      *divert_mask |= PPROC_DIVERT_UNPROCESSED;
      break;
    }
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
 * Function: module_faceproc_port_event_fwd_list
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
mct_list_t *module_faceproc_port_event_fwd_list(unsigned int identity,
  mct_port_t *port)
{
  /*not required for faceproc*/
  return NULL;
}

/**
 * Function: module_faceproc_port_ext_link
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
boolean module_faceproc_port_ext_link(unsigned int identity,
  mct_port_t* port, mct_port_t *peer)
{
  int rc = IMG_SUCCESS;
  unsigned int *p_identity = NULL;
  mct_list_t *p_temp_list = NULL;
  mct_module_t *p_mct_mod = NULL;
  module_faceproc_t *p_mod = NULL;
  faceproc_client_t *p_client = NULL;

  if (!port || !peer) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  IDBG_MED("%s:%d] port %p E", __func__, __LINE__, port);
  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_faceproc_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] faceproc module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (faceproc_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] invalid client", __func__, __LINE__);
    return FALSE;
  }

  if (MCT_PORT_PEER(port)) {
    IDBG_ERROR("%s:%d] link already established", __func__, __LINE__);
    return FALSE;
  }

  MCT_PORT_PEER(port) = peer;

  /* check if its sink port*/
  if (MCT_PORT_IS_SINK(port)) {
    /* start faceproc client in case of dynamic module */
  } else {
    /* do nothing for source port */
  }
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return GET_STATUS(rc);

error:
  IDBG_MED("%s:%d] Error X", __func__, __LINE__);
  return FALSE;
}

/**
 * Function: module_faceproc_port_unlink
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
void module_faceproc_port_unlink(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  int rc = IMG_SUCCESS;
  mct_list_t *p_temp_list = NULL;
  mct_module_t *p_mct_mod = NULL;
  module_faceproc_t *p_mod = NULL;
  faceproc_client_t *p_client = NULL;
  uint32_t *p_identity = NULL;

  if (!port || !peer) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return;
  }

  IDBG_MED("%s:%d] E", __func__, __LINE__);
  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return;
  }

  p_mod = (module_faceproc_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] faceproc module NULL", __func__, __LINE__);
    return;
  }

  p_client = (faceproc_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] faceproc client NULL", __func__, __LINE__);
    return;
  }

  if (MCT_PORT_IS_SINK(port)) {
    /* stop the client in case of dynamic module */
  } else {
    /* do nothing for source port*/
  }

  MCT_PORT_PEER(port) = NULL;

  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return;

error:
  IDBG_MED("%s:%d] Error X", __func__, __LINE__);
}

/**
 * Function: module_faceproc_port_set_caps
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
boolean module_faceproc_port_set_caps(mct_port_t *port,
  mct_port_caps_t *caps)
{
  int rc = IMG_SUCCESS;
  return GET_STATUS(rc);
}

/**
 * Function: module_faceproc_port_check_caps_reserve
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
boolean module_faceproc_port_check_caps_reserve(mct_port_t *port, void *peer_caps,
  void *vstream_info)
{
  boolean rc = FALSE;
  mct_stream_info_t *stream_info = (mct_stream_info_t *)vstream_info;
  mct_module_t *p_mct_mod = NULL;
  module_faceproc_t *p_mod = NULL;
  mct_port_caps_t *p_peer_caps = (mct_port_caps_t *)peer_caps;
  mct_port_caps_t *p_caps = (mct_port_caps_t *)&port->caps;

  IDBG_MED("%s:%d] E", __func__, __LINE__);
  if (!port || !stream_info || !p_peer_caps) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  if (p_peer_caps->port_caps_type != MCT_PORT_CAPS_FRAME) {
    IDBG_ERROR("%s:%d] invalid capabilitied, cannot connect port %x",
      __func__, __LINE__, p_peer_caps->port_caps_type);
    return FALSE;
  }

  IDBG_MED("%s:%d] caps type %d format %d", __func__, __LINE__,
    p_caps->port_caps_type,
    p_caps->u.frame.format_flag);
  if (!(p_caps->port_caps_type == MCT_PORT_CAPS_FRAME) &&
    (p_caps->u.frame.format_flag == MCT_PORT_CAP_FORMAT_YCBCR)) {
    IDBG_ERROR("%s:%d] port caps not matching", __func__, __LINE__);
  }

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_faceproc_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] faceproc module NULL", __func__, __LINE__);
    return FALSE;
  }

  /* lock the module */
  pthread_mutex_lock(&p_mod->mutex);
  if (port->port_private) {
    /* port is already reserved */
    IDBG_MED("%s:%d] port is reserved", __func__, __LINE__);
    pthread_mutex_unlock(&p_mod->mutex);
    return FALSE;
  }
  rc = module_faceproc_acquire_port(p_mct_mod, port, stream_info);
  if (FALSE == rc) {
    IDBG_ERROR("%s:%d] Error acquiring port", __func__, __LINE__);
    pthread_mutex_unlock(&p_mod->mutex);
    return FALSE;
  }

  pthread_mutex_unlock(&p_mod->mutex);
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return TRUE;
}

/**
 * Function: module_faceproc_port_check_caps_unreserve
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
boolean module_faceproc_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  int rc = IMG_SUCCESS;
  mct_list_t *p_temp_list = NULL;
  mct_module_t *p_mct_mod = NULL;
  module_faceproc_t *p_mod = NULL;
  faceproc_client_t *p_client = NULL;

  IDBG_MED("%s:%d] E", __func__, __LINE__);
  if (!port) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_faceproc_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] faceproc module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (faceproc_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] faceproc client NULL", __func__, __LINE__);
    return FALSE;
  }

  /* lock the module */
  pthread_mutex_lock(&p_mod->mutex);

  if (MCT_PORT_IS_SRC(port)) {
    /* remove the connection. lock the client */
    pthread_mutex_lock(&p_client->mutex);
    p_client->p_srcport = NULL;
    pthread_mutex_unlock(&p_client->mutex);
  } else {
    /* First remove client form module list */
    p_temp_list = mct_list_find_custom(p_mod->fp_client, &identity,
      module_faceproc_find_client);
    if (NULL != p_temp_list) {
      p_mod->fp_client = mct_list_remove(p_mod->fp_client,
        p_temp_list->data);
    }
    /* destroy the client */
    port->port_private = NULL;
    module_faceproc_client_destroy(p_client);
    p_client = NULL;
  }

  pthread_mutex_unlock(&p_mod->mutex);

  /*Todo: free port??*/
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return GET_STATUS(rc);

error:
  pthread_mutex_unlock(&p_mod->mutex);
  IDBG_MED("%s:%d] Error rc = %d X", __func__, __LINE__, rc);
  return FALSE;
}

/**
 * Function: module_faceproc_simulate_port_streamon
 *
 * Description: Currently Stream ON is passed to
 *  module directly, our module is handling this functionality
 *  on his ports, use this to redirect from module to port functions
 *
 * Arguments:
 *   @stream: mct_stream_t
 *   @module mct_module_t
 *   @port: mct_port_t
 *   @event: mct_event_t
 *
 * Return values:
 *     error/success
 *
 * Notes: Remove this function when Mct will use port functions
 *  for Source module
 **/
static boolean module_faceproc_simulate_port_streamon(mct_stream_t *stream,
  mct_module_t *module, mct_port_t *port, mct_event_t *event)
{
  boolean port_sucess;

  if (!(stream && module && port && port->check_caps_reserve))
    return FALSE;
  if (!(port->check_caps_unreserve && port->ext_link && port->un_link))
    return FALSE;

  port_sucess = port->check_caps_reserve(port, &port->caps,
    &stream->streaminfo);
  if (FALSE == port_sucess)
    return port_sucess;

  port_sucess = port->ext_link(stream->streaminfo.identity, port, port);
  if (FALSE == port_sucess) {
    port->check_caps_unreserve(port, stream->streaminfo.identity);
    return port_sucess;
  }

  port->event_func(port, event);
  if (FALSE == port_sucess) {
    port->un_link(stream->streaminfo.identity, port, port);
    port->check_caps_unreserve(port, stream->streaminfo.identity);
  }
  return port_sucess;
}

/**
 * Function: module_faceproc_simulate_port_streamoff
 *
 * Description: Currently Stream ON is passed to
 *  module directly, our module is handling this functionality
 *  on his ports, use this to redirect from module to port functions
 *
 * Arguments:
 *   @stream: mct_stream_t
 *   @module mct_module_t
 *   @port: mct_port_t
 *   @event: mct_event_t
 *
 * Return values:
 *     error/success
 *
 * Notes: Remove this function when Mct will use port functions
 *  for Source module
 **/
static boolean module_faceproc_simulate_port_streamoff(mct_stream_t *stream,
  mct_module_t *module, mct_port_t *port, mct_event_t *event)
{
  if (!(stream && module && port))
    return FALSE;
  if (!(port->check_caps_unreserve && port->event_func && port->un_link))
    return FALSE;

  port->event_func(port, event);
  port->un_link(stream->streaminfo.identity, port, port);
  port->check_caps_unreserve(port, stream->streaminfo.identity);

  return TRUE;
}

/**
 * Function: module_faceproc_process_event
 *
 * Description: Event handler function for the faceproc module
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
static boolean module_faceproc_process_event(mct_module_t *module,
  mct_event_t *event)
{
  mct_stream_t *stream;
  mct_port_t *port;
  module_faceproc_t *p_mod;
  int rc = IMG_SUCCESS;

  if (!(module && module->module_private) || !event) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  stream = MCT_STREAM_CAST((MCT_MODULE_PARENT(module))->data);
  if (!stream) {
    IDBG_MED("Error get STREAM EVENT ID ");
    goto out;
  }

  port = MCT_PORT_CAST((MCT_MODULE_CHILDREN(module))->data);
  if (!port) {
    IDBG_MED("Error get PORT ID ");
    goto out;
  }

  p_mod = (module_faceproc_t *)module->module_private;

  switch (event->type) {
  case MCT_EVENT_CONTROL_CMD: {
    mct_event_control_t *p_ctrl_event = &event->u.ctrl_event;
    IDBG_MED("%s:%d] Ctrl type %d", __func__, __LINE__, p_ctrl_event->type);
    switch (p_ctrl_event->type) {
    case MCT_EVENT_CONTROL_STREAMON: {
      if (p_mod->module_type == MCT_MODULE_FLAG_SOURCE)
          module_faceproc_simulate_port_streamon(stream, module, port ,event);
      break;
    }
    case MCT_EVENT_CONTROL_STREAMOFF:
      if (p_mod->module_type  == MCT_MODULE_FLAG_SOURCE)
          module_faceproc_simulate_port_streamoff(stream, module, port, event);
      break;
    case MCT_EVENT_CONTROL_PARM_STREAM_BUF:
    case MCT_EVENT_CONTROL_SET_PARM:
      if (port->event_func)
        port->event_func(port, event);
      break;
    default:
      break;

    }
    break;
  }
  case MCT_EVENT_MODULE_EVENT:
  default:
    /* forward the event */
    break;
  }

out:
  return GET_STATUS(rc);
}

/**
 * Function: module_faceproc_request_new_port
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
mct_port_t *module_faceproc_request_new_port(void *vstream_info,
  mct_port_direction_t direction,
  mct_module_t *module,
  void *peer_caps)
{
  mct_stream_info_t *stream_info = (mct_stream_info_t *)vstream_info;
  boolean rc = IMG_SUCCESS;
  module_faceproc_t *p_mod = NULL;
  mct_port_t *p_port = NULL;
  mct_port_caps_t *p_peer_caps = (mct_port_caps_t *)peer_caps;

  if (!module || !stream_info) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return NULL;
  }

  if (p_peer_caps) {
    if(p_peer_caps->port_caps_type != MCT_PORT_CAPS_FRAME) {
      IDBG_ERROR("%s:%d] invalid capabilities, cannot connect port %x",
      __func__, __LINE__, p_peer_caps->port_caps_type);
      return NULL;
    }
    IDBG_MED("%s:%d] caps type %d format %d", __func__, __LINE__,
      p_peer_caps->port_caps_type,
      p_peer_caps->u.frame.format_flag);
  }

  p_mod = (module_faceproc_t *)module->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] faceproc module NULL", __func__, __LINE__);
    return NULL;
  }

  pthread_mutex_lock(&p_mod->mutex);
  p_port = module_faceproc_create_port(module, direction);
  if (NULL == p_port) {
    IDBG_ERROR("%s:%d] Error creating port", __func__, __LINE__);
    goto error;
  }
  /*acquire port*/
  rc = module_faceproc_acquire_port(module, p_port, stream_info);
  if (FALSE == rc) {
    IDBG_ERROR("%s:%d] Error acquiring port", __func__, __LINE__);
    goto error;
  }
  pthread_mutex_unlock(&p_mod->mutex);
  return p_port;

error:
  pthread_mutex_unlock(&p_mod->mutex);
  return NULL;
}

/**
 * Function: module_faceproc_start_session
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
static boolean module_faceproc_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  int rc = IMG_SUCCESS;
  module_faceproc_t *p_mod;

  if (!module) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_faceproc_t *)module->module_private;
  if (!p_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  /* Add session settings */
  module_faceproc_create_session_param(p_mod, sessionid);

  /* create message thread */
  rc = module_imglib_create_msg_thread(&p_mod->msg_thread);

  return GET_STATUS(rc);
}

/**
 * Function: module_faceproc_start_session
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
static boolean module_faceproc_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  int rc = IMG_SUCCESS;
  module_faceproc_t *p_mod;

  if (!module) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_faceproc_t *)module->module_private;
  if (!p_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  /* Check if this need to be here */
  module_faceproc_destroy_session_param(p_mod, sessionid);

  /* destroy message thread */
  rc = module_imglib_destroy_msg_thread(&p_mod->msg_thread);
  return GET_STATUS(rc);
}

/**
 * Function: module_faceproc_set_mod
 *
 * Description: This function is used to set the faceproc module
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
static void module_faceproc_set_mod(mct_module_t *module,
  unsigned int module_type,
  unsigned int identity)
{
  module_faceproc_t *p_mod;

  if (!(module && module->module_private)) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return;
  }

  /* Do not touch module mode since currently is only one instance module */
  p_mod = (module_faceproc_t *)module->module_private;
  p_mod->module_type = module_type;
  mct_module_add_type(module, module_type, identity);

  return;
}

/**
 * Function: module_faceproc_query_mod
 *
 * Description: This function is used to query the faceproc module info
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
static boolean module_faceproc_query_mod(mct_module_t *module, void *query_buf,
  unsigned int sessionid)
{
  mct_pipeline_cap_t *p_mct_cap = (mct_pipeline_cap_t *)query_buf;
  mct_pipeline_imaging_cap_t *p_cap = NULL;
  if (!query_buf || !module) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  p_cap = &p_mct_cap->imaging_cap;
  p_cap->max_num_roi = MAX_FACES_TO_DETECT;
  p_cap->feature_mask = CAM_QCOM_FEATURE_FACE_DETECTION |
    CAM_QCOM_FEATURE_REGISTER_FACE;

  IDBG_MED("%s:%d Max Face ROI %d", __func__, __LINE__, p_cap->max_num_roi);

  return TRUE;
}

/**
 * Function: module_faceproc_free_port
 *
 * Description: This function is used to free the faceproc ports
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static boolean module_faceproc_free_port(void *data, void *user_data)
{
  mct_port_t *p_port = (mct_port_t *)data;
  mct_module_t *p_mct_mod = (mct_module_t *)user_data;
  boolean rc = FALSE;

  IDBG_MED("%s:%d port %p p_mct_mod %p", __func__, __LINE__, p_port,
    p_mct_mod);
  if (!p_port || !p_mct_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return TRUE;
  }
  rc = mct_module_remove_port(p_mct_mod, p_port);
  if (rc == FALSE) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
  }
  mct_port_destroy(p_port);
  return TRUE;
}

/**
 * Function: module_faceproc_create_port
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
mct_port_t *module_faceproc_create_port(mct_module_t *p_mct_mod,
  mct_port_direction_t dir)
{
  char portname[PORT_NAME_LEN];
  mct_port_t *p_port = NULL;
  int status = IMG_SUCCESS;
  int index = 0;

  if (!p_mct_mod || (MCT_PORT_UNKNOWN == dir)) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return NULL;
  }

  index = (MCT_PORT_SINK == dir) ? p_mct_mod->numsinkports :
    p_mct_mod->numsrcports;
  /*portname <mod_name>_direction_portIndex*/
  snprintf(portname, sizeof(portname), "%s_d%d_i%d",
    MCT_MODULE_NAME(p_mct_mod), dir, index);
  p_port = mct_port_create(portname);
  if (NULL == p_port) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return NULL;
  }
  IDBG_MED("%s:%d portname %s", __func__, __LINE__, portname);

  p_port->direction = dir;
  p_port->port_private = NULL;
  p_port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;
  p_port->caps.u.frame.format_flag = MCT_PORT_CAP_FORMAT_YCBCR;
  /*Todo: fill the size flag*/

  /* override the function pointers */
  p_port->check_caps_reserve    = module_faceproc_port_check_caps_reserve;
  p_port->check_caps_unreserve  = module_faceproc_port_check_caps_unreserve;
  p_port->ext_link              = module_faceproc_port_ext_link;
  p_port->un_link               = module_faceproc_port_unlink;
  p_port->set_caps              = module_faceproc_port_set_caps;
  p_port->event_func            = module_faceproc_port_event_func;

  /* add port to the module */
  if (!mct_module_add_port(p_mct_mod, p_port)) {
    IDBG_ERROR("%s: Set parent failed", __func__);
    status = IMG_ERR_GENERAL;
    goto error;
  }

  if (MCT_PORT_SRC == dir)
    p_mct_mod->numsrcports++;
  else
    p_mct_mod->numsinkports++;

  IDBG_MED("%s:%d ", __func__, __LINE__);
  return p_port;

error:

  IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
  if (p_port) {
    mct_port_destroy(p_port);
    p_port = NULL;
  }
  return NULL;
}

/** module_faceproc_get_next_from_list
 *    @data1: not used
 *    @data2: not used
 *
 *  Gets next element from the list
 *
 *  Return TRUE always
 **/
static boolean module_faceproc_get_next_from_list(void *data1, void *data2)
{
  return TRUE;
}

/**
 * Function: module_faceproc_free_mod
 *
 * Description: This function is used to free the faceproc module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_faceproc_deinit(mct_module_t *p_mct_mod)
{
  module_faceproc_t *p_mod = NULL;
  img_core_ops_t *p_core_ops = NULL;
  mct_list_t* p_list;
  int rc = 0;
  int i = 0;

  if (NULL == p_mct_mod) {
    IDBG_ERROR("%s:%d] MCTL module NULL", __func__, __LINE__);
    return;
  }

  p_mod = (module_faceproc_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] faceproc module NULL", __func__, __LINE__);
    return;
  }

  pthread_mutex_destroy(&p_mod->mutex);
  pthread_cond_destroy(&p_mod->cond);

  do {
    p_list = mct_list_find_custom(MCT_MODULE_SINKPORTS(p_mct_mod), p_mct_mod,
      module_faceproc_get_next_from_list);
    if (p_list)
      module_faceproc_free_port(p_list->data, p_mct_mod);
  } while (p_list);

  do {
    p_list = mct_list_find_custom(MCT_MODULE_SRCPORTS(p_mct_mod), p_mct_mod,
      module_faceproc_get_next_from_list);
    if (p_list)
      module_faceproc_free_port(p_list->data, p_mct_mod);
  } while (p_list);

  IDBG_MED("%s:%d] delete the clients", __func__, __LINE__);
  /*Todo*/

  p_core_ops = &p_mod->core_ops;
  IDBG_MED("%s:%d lib_ref_cnt %d", __func__, __LINE__, p_mod->lib_ref_count);
  if (p_mod->lib_ref_count) {
    IMG_COMP_UNLOAD(p_core_ops);
  }
  p_mod->client_cnt = 0;
  free(p_mod);
  mct_module_destroy(p_mct_mod);
}

/** module_faceproc_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: This function is used to initialize the faceproc module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_faceproc_init(const char *name)
{
  mct_module_t *p_mct_mod = NULL;
  module_faceproc_t *p_mod = NULL;
  img_core_ops_t *p_core_ops = NULL;
  mct_port_t *p_port = NULL;
  int rc = 0;
  int i = 0;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  p_mct_mod = mct_module_create(name);
  if (NULL == p_mct_mod) {
    IDBG_ERROR("%s:%d cannot allocate mct module", __func__, __LINE__);
    return NULL;
  }
  p_mod = malloc(sizeof(module_faceproc_t));
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    goto error;
  }

  p_mct_mod->module_private = (void *)p_mod;
  memset(p_mod, 0, sizeof(module_faceproc_t));

  pthread_mutex_init(&p_mod->mutex, NULL);
  pthread_cond_init(&p_mod->cond, NULL);
  p_core_ops = &p_mod->core_ops;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  /* check if the faceproc module is present */
  rc = img_core_get_comp(IMG_COMP_FACE_PROC, "qcom.faceproc", p_core_ops);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
    goto error;
  }
  /* try to load the component */
  rc = IMG_COMP_LOAD(p_core_ops, NULL);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
    goto error;
  }
  p_mod->lib_ref_count++;
  p_mod->fp_client = NULL;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  /* create static ports */
  for (i = 0; i < MAX_FD_STATIC_PORTS; i++) {
    p_port = module_faceproc_create_port(p_mct_mod, MCT_PORT_SINK);
    if (NULL == p_port) {
      IDBG_ERROR("%s:%d] create port failed", __func__, __LINE__);
      goto error;
    }
  }

  p_mct_mod->process_event    = module_faceproc_process_event;
  p_mct_mod->set_mod          = module_faceproc_set_mod;
  p_mct_mod->query_mod        = module_faceproc_query_mod;
  p_mct_mod->request_new_port = module_faceproc_request_new_port;
  p_mct_mod->start_session    = module_faceproc_start_session;
  p_mct_mod->stop_session     = module_faceproc_stop_session;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  return p_mct_mod;

error:

  if (p_mod) {
    module_faceproc_deinit(p_mct_mod);
  } else if (p_mct_mod) {
    mct_module_destroy(p_mct_mod);
  }
  return NULL;
}
