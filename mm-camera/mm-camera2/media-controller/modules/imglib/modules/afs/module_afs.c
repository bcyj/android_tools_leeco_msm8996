/**********************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#include <linux/media.h>
#include "modules.h"
#include "mct_module.h"
#include "module_afs.h"
#include "mct_stream.h"
#include "mct_port.h"
#include "mct_pipeline.h"

/**
 * STATIC function declarations
 **/
static mct_port_t *module_afs_create_port(mct_module_t *p_mct_mod,
  mct_port_direction_t dir);

/**
 * Function: module_afs_find_client
 *
 * Description: This method is used to find the client
 *
 * Arguments:
 *   @p_fp_data: afs client
 *   @p_input: input data
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_afs_find_client(void *p_fp_data, void *p_input)
{
  afs_client_t *p_client = (afs_client_t *)p_fp_data;
  uint32_t identity = *((uint32_t *)p_input);

  return (p_client->identity == identity) ? TRUE : FALSE;
}

/**
 * Function: module_afs_find_session_params
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
static boolean module_afs_find_session_params(void *p_data, void *p_input)
{

  afs_session_params_t *stored_param = p_data;
  uint32_t session_id = *((uint32_t *)p_input);

  return (stored_param->session_id == session_id) ? TRUE : FALSE;
}

/**
 * Function: module_afs_find_identity
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
static boolean module_afs_find_identity(void *p_data, void *p_input)
{
  afs_client_t *p_client = (afs_client_t *)p_data;
  uint32_t *p_identity = (uint32_t *)p_input;

  if (!p_client || !p_input)
    return FALSE;


  return (*p_identity == p_client->identity) ? TRUE : FALSE;
}

/**
 * Function: module_afs_store_session_param
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
static int module_afs_store_session_param(module_afs_t *p_mod,
  afs_client_t *p_client, mct_event_control_parm_t *param)
{
  if (!p_mod || !param)
    return IMG_ERR_INVALID_INPUT;

  switch(param->type) {
  /*Todo: store params */
  default:
    break;
  }
  return IMG_SUCCESS;
}

/**
 * Function: module_afs_restore_session_param
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
static int module_afs_restore_session_param(module_afs_t *p_mod,
  afs_client_t *p_client)
{
  mct_list_t *p_temp_list;
  afs_session_params_t *stored_param;
  uint32_t session_id;

  if (!p_mod || !p_client)
    return IMG_ERR_INVALID_INPUT;

  session_id = IMGLIB_SESSIONID(p_client->identity);

  /* Find settings per session id */
  p_temp_list = mct_list_find_custom(p_mod->session_parms, &session_id,
    module_afs_find_session_params);
  if (!p_temp_list)
    return IMG_ERR_GENERAL;

  stored_param = p_temp_list->data;
  if (!stored_param)
    return IMG_ERR_GENERAL;

  /* Do not restore session modes when client is in face register mode */
  if (FALSE == stored_param->valid_params)
    return IMG_SUCCESS;

  /*Todo: */
  return IMG_SUCCESS;
}

/**
 * Function: module_afs_create_session_param
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
static int module_afs_create_session_param(module_afs_t *p_mod,
  uint32_t session_id)
{
  afs_session_params_t *stored_param;

  if (!p_mod)
    return IMG_ERR_INVALID_INPUT;

  stored_param = malloc(sizeof(*stored_param));
  if (NULL == stored_param)
    return IMG_ERR_NO_MEMORY;

  memset(stored_param, 0x0, sizeof(afs_session_params_t));
  stored_param->session_id = session_id;
  stored_param->valid_params = FALSE;

  p_mod->session_parms = mct_list_append(p_mod->session_parms,
    stored_param, NULL, NULL);

  return (p_mod->session_parms) ? IMG_SUCCESS : IMG_ERR_GENERAL;
}

/**
 * Function: module_afs_destroy_session_param
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
static int module_afs_destroy_session_param(module_afs_t *p_mod,
  uint32_t session_id)
{
  afs_session_params_t *stored_param;
  mct_list_t *p_temp_list;

  if (!p_mod)
    return IMG_ERR_INVALID_INPUT;

  /* Find paramters per session id */
  p_temp_list = mct_list_find_custom(p_mod->session_parms, &session_id,
    module_afs_find_session_params);
  if (!p_temp_list)
    return IMG_ERR_INVALID_INPUT;


  stored_param = ( afs_session_params_t *)p_temp_list->data;
  p_mod->session_parms = mct_list_remove(p_mod->session_parms, stored_param);
  free(stored_param);

  return IMG_SUCCESS;
}

/**
 * Function: module_afs_acquire_port
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
boolean module_afs_acquire_port(mct_module_t *p_mct_mod,
  mct_port_t *port,
  mct_stream_info_t *stream_info)
{
  int rc = IMG_SUCCESS;
  unsigned int p_identity ;
  mct_list_t *p_temp_list = NULL;
  afs_client_t *p_client = NULL;
  module_afs_t *p_mod = NULL;

  IDBG_MED("%s:%d] E", __func__, __LINE__);

  p_mod = (module_afs_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] afs module NULL", __func__, __LINE__);
    return FALSE;
  }
  p_identity =  stream_info->identity;

  /* check if its sink port*/
  if (MCT_PORT_IS_SINK(port)) {
    /* create afs client */
    rc = module_afs_client_create(p_mct_mod, port, p_identity, stream_info);
    if (IMG_SUCCEEDED(rc)) {
      /* add the client to the list */
      p_client = (afs_client_t *) port->port_private;
      p_temp_list = mct_list_append(p_mod->afs_client, p_client, NULL, NULL);
      if (NULL == p_temp_list) {
        IDBG_ERROR("%s:%d] list append failed", __func__, __LINE__);
        rc = IMG_ERR_GENERAL;
        goto error;
      }
      p_mod->afs_client = p_temp_list;
    }
  } else {
    /* update the internal connection with source port */
    p_temp_list = mct_list_find_custom(p_mod->afs_client, &p_identity,
      module_afs_find_client);
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
 * Function: module_afs_handle_streamon
 *
 * Description: Function to handle afs streamon
 *
 * Arguments:
 *   @p_mod: afs module pointer
 *   @p_client: afs client pointer
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
int module_afs_handle_streamon(module_afs_t *p_mod,
  afs_client_t *p_client)
{
  int rc;

  if (p_client->state != IMGLIB_STATE_INIT) {
     IDBG_ERROR("%s:%d] client not in init state %d", __func__, __LINE__,
       p_client->state);
     return IMG_SUCCESS;
  }

  rc = module_afs_client_start(p_client);
  if (IMG_SUCCEEDED(rc)) {
    rc = module_afs_client_map_buffers(p_client);
  } else {
    IDBG_ERROR("%s:%d] Error cannot start %d", __func__, __LINE__, rc);
    return rc;
  }

  p_client->state = IMGLIB_STATE_PROCESSING;
  return rc;
}

/**
 * Function: module_afs_handle_streamoff
 *
 * Description: Function to handle afs streamoff
 *
 * Arguments:
 *   @p_mod: afs module pointer
 *   @p_client: afs client pointer
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
int module_afs_handle_streamoff(module_afs_t *p_mod,
  afs_client_t *p_client)
{
  int rc;

  if ((p_client->state != IMGLIB_STATE_STARTED) &&
    (p_client->state != IMGLIB_STATE_PROCESSING)) {
     IDBG_ERROR("%s:%d] client not started state %d", __func__, __LINE__,
       p_client->state);
     return IMG_SUCCESS;
  }

  rc = module_afs_client_stop(p_client);
  if (IMG_SUCCEEDED(rc))
    rc = module_afs_client_unmap_buffers(p_client);
  else
    IDBG_ERROR("%s:%d] Error cannot stop %d", __func__, __LINE__, rc);
  return rc;
}

/**
 * Function: module_afs_forward_port_event
 *
 * Description: This method is used to forward an event
 *                depending on the direction.
 *
 * Arguments:
 *   @p_client: AFS client
 *   @mct_port: Port that recieved the event
 *   @event: Event recieved
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_afs_forward_port_event(afs_client_t *p_client,
  mct_port_t *port, mct_event_t *event)
{
  boolean rc = FALSE;
  mct_port_t *p_adj_port = NULL;

  if (MCT_PORT_IS_SINK(port)) {
    p_adj_port = p_client->p_srcport;
    if (NULL == p_adj_port) {
       IDBG_ERROR("%s:%d] Invalid port", __func__, __LINE__);
       return FALSE;
    }
    switch(event->direction) {
      case MCT_EVENT_UPSTREAM : {
        IDBG_ERROR("%s:%d] Error Upstream event on Sink port %d",
          __func__, __LINE__, event->type);
        break;
      }
      case MCT_EVENT_BOTH:
      case MCT_EVENT_DOWNSTREAM: {
       rc =  mct_port_send_event_to_peer(p_adj_port, event);
       if (rc == FALSE) {
         IDBG_ERROR("%s:%d] Fowarding event %d from sink port failed",
           __func__, __LINE__, event->type);
       }
       break;
     }
     default:
       IDBG_ERROR("%s:%d] Invalid port direction for event %d",
         __func__, __LINE__, event->type);
       break;
    }
  } else if (MCT_PORT_IS_SRC(port)) {
    p_adj_port = p_client->p_sinkport;
    if (NULL == p_adj_port) {
       IDBG_ERROR("%s:%d] Invalid port", __func__, __LINE__);
       return FALSE;
    }
    switch(event->direction) {
      case MCT_EVENT_DOWNSTREAM : {
        IDBG_ERROR("%s:%d] Error Downstream event on Src port %d",
          __func__, __LINE__, event->type);
        break;
      }
      case MCT_EVENT_BOTH:
      case MCT_EVENT_UPSTREAM: {
       rc =  mct_port_send_event_to_peer(p_adj_port, event);
       if (rc == FALSE) {
         IDBG_ERROR("%s:%d] Fowarding event %d from src port failed",
           __func__, __LINE__, event->type);
       }
       break;
     }
     default:
       IDBG_ERROR("%s:%d] Invalid port direction for event %d",
         __func__, __LINE__, event->type);
       break;
    }
  }
  return rc;
}

/**
 * Function: module_afs_port_event_func
 *
 * Description: Event handler function for the afs port
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
boolean module_afs_port_event_func(mct_port_t *port,
  mct_event_t *event)
{
  int rc = IMG_SUCCESS;
  mct_module_t *p_mct_mod = NULL;
  module_afs_t *p_mod = NULL;
  afs_client_t *p_client;
  boolean fwd_event = TRUE;

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

  p_mod = (module_afs_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] afs module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (afs_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] afs client NULL", __func__, __LINE__);
    return FALSE;
  }

  IDBG_LOW("%s:%d] type %d", __func__, __LINE__, event->type);
  switch (event->type) {
  case MCT_EVENT_CONTROL_CMD: {
    mct_event_control_t *p_ctrl_event = &event->u.ctrl_event;
    IDBG_MED("%s:%d] Ctrl type %d", __func__, __LINE__, p_ctrl_event->type);
    switch (p_ctrl_event->type) {
    case MCT_EVENT_CONTROL_STREAMON: {
      /* restore the parameters */
      module_afs_restore_session_param(p_mod, p_client);
      pthread_mutex_lock(&p_client->mutex);
      rc = module_afs_handle_streamon(p_mod, p_client);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("%s:%d] AFS_STREAMON failed %d", __func__, __LINE__, rc);
      } else {
        IDBG_MED("%s:%d] AFS_STREAMON %x", __func__, __LINE__, event->identity);
      }
      pthread_mutex_unlock(&p_client->mutex);
      break;
    }
    case MCT_EVENT_CONTROL_STREAMOFF: {
      pthread_mutex_lock(&p_client->frame_algo_mutex);
      while (p_client->processing) {
        pthread_cond_wait(&p_client->frame_algo_cond,
          &p_client->frame_algo_mutex);
      }
      pthread_mutex_unlock(&p_client->frame_algo_mutex);

      pthread_mutex_lock(&p_client->mutex);
      rc = module_afs_handle_streamoff(p_mod, p_client);
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("%s:%d] AFS_STREAMOFF failed %d", __func__, __LINE__, rc);
      } else {
        IDBG_MED("%s:%d] AFS_STREAOFF %x", __func__, __LINE__, event->identity);
      }
      pthread_mutex_unlock(&p_client->mutex);
      break;
    }
    case MCT_EVENT_CONTROL_SET_PARM: {
      rc = module_afs_client_handle_ctrl_parm(p_client,
        p_ctrl_event->control_event_data);
      if (IMG_SUCCEEDED(rc))
        module_afs_store_session_param(p_mod, p_client,
          p_ctrl_event->control_event_data);
      break;
    }

    default:
      break;
    }
    break;
  }
  case MCT_EVENT_MODULE_EVENT: {
    mct_event_module_t *p_mod_event = &event->u.module_event;
    IDBG_MED("%s:%d] Mod type %d", __func__, __LINE__, p_mod_event->type);
    switch (p_mod_event->type) {
    case MCT_EVENT_MODULE_BUF_DIVERT: {
      mod_img_msg_t msg;
      isp_buf_divert_t *p_buf_divert =
        (isp_buf_divert_t *)p_mod_event->module_event_data;
      int frame_idx;


      if (p_client->cur_af_cfg.enable) {
        if (!p_client->sync) {
          /* async mode, post to msg thread */
          module_afs_client_handle_buffer(p_client,
            p_buf_divert->buffer.index,
            p_buf_divert->buffer.sequence,
            &frame_idx,
            p_buf_divert);

          if ((frame_idx >= 0) && (frame_idx < MAX_NUM_AFS_FRAMES)) {
            IDBG_MED("%s:%d] frame_idx %d", __func__, __LINE__, frame_idx);
            memset(&msg, 0x0, sizeof(mod_img_msg_t));
            msg.port = port;
            msg.type = MOD_IMG_MSG_EXEC_INFO;
            msg.data.exec_info.p_userdata = p_client;
            msg.data.exec_info.data = &p_client->p_frame[frame_idx];
            msg.data.exec_info.p_exec = module_afs_client_process;
            module_imglib_send_msg(&p_mod->msg_thread, &msg);
          }
        } else {
          /* syncronous mode */
          img_frame_t frame;
          mct_stream_map_buf_t *p_map_buf;
          memset(&frame, 0x0, sizeof(img_frame_t));
          uint32_t buf_idx = p_buf_divert->buffer.index;
          p_client->frame_id = p_buf_divert->buffer.sequence;

          if (buf_idx >= p_client->buffer_cnt) {
            IDBG_ERROR("%s:%d] invalid buffer index %d frame_idx %d",
              __func__, __LINE__, buf_idx, p_client->frame_id);
          } else {
            p_map_buf = &p_client->p_map_buf[buf_idx];
            IDBG_MED("%s:%d] frame_idx %d %dx%d native %d (%p %d)",
              __func__, __LINE__,
              p_client->frame_id,
              p_client->out_dim.width,
              p_client->out_dim.height,
              p_buf_divert->native_buf,
              p_buf_divert->vaddr,
              p_buf_divert->fd);
            if (!p_buf_divert->native_buf) {
              frame.frame[0].plane[0].addr = p_map_buf->buf_planes[0].buf;
              frame.frame[0].plane[0].fd = p_map_buf->buf_planes[0].fd;
              p_client->video_mode = FALSE;
            } else {
              unsigned long *plane_addr = (unsigned long *)p_buf_divert->vaddr;
              frame.frame[0].plane[0].addr = (uint8_t *)plane_addr[0];
              frame.frame[0].plane[0].fd = p_buf_divert->fd;
              p_client->video_mode = TRUE;
            }
            frame.frame[0].plane[0].height = p_client->out_dim.height;
            frame.frame[0].plane[0].width = p_client->out_dim.width;
            frame.frame[0].plane[0].length =
              frame.frame[0].plane[0].height *
              frame.frame[0].plane[0].width;
            module_afs_client_process(p_client, &frame);
          }
        }
      }

      fwd_event = !p_client->video_mode && !p_client->cur_af_cfg.enable;
      /* indicate that the buffer is consumed */
      p_buf_divert->is_locked = FALSE;
      p_buf_divert->ack_flag = TRUE;
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

      rc = module_afs_client_set_scale_ratio(p_client, s_crop);
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
    case MCT_EVENT_MODULE_SET_AF_TUNE_PTR : {
      af_algo_tune_parms_t *tuning_info;
      tuning_info = (af_algo_tune_parms_t *)p_mod_event->module_event_data;
      IDBG_LOW("%s: Handle af_tuning header update event, h_clip_ratio %f, "
        "v_clip_ratio %f", __func__,
        tuning_info->af_vfe.config.h_clip_ratio_normal_light,
        tuning_info->af_vfe.config.v_clip_ratio_normal_light);

      p_client->af_tuning_trans_info.h_scale =
        tuning_info->af_vfe.config.h_clip_ratio_normal_light;
      p_client->af_tuning_trans_info.v_scale =
        tuning_info->af_vfe.config.v_clip_ratio_normal_light;

      break;
    }
    case MCT_EVENT_MODULE_IMGLIB_AF_CONFIG : {
      mct_imglib_af_config_t *p_cfg =
        (mct_imglib_af_config_t *)p_mod_event->module_event_data;

      if (p_cfg) {
        pthread_mutex_lock(&p_client->mutex);
        p_client->cur_af_cfg = *p_cfg;
        pthread_mutex_unlock(&p_client->mutex);
      }
      IDBG_MED("%s:%d] AF stats enable %d %d", __func__, __LINE__,
        p_client->cur_af_cfg.enable,
        p_client->cur_af_cfg.filter_type);

      module_afs_client_update_cfg(p_client);
      break;
    }

    case MCT_EVENT_MODULE_ISP_OUTPUT_DIM: {
      mct_stream_info_t *stream_info =
        (mct_stream_info_t *)(event->u.module_event.module_event_data);
      if (!stream_info) {
        IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
      } else {
        if (stream_info->dim.width != p_client->stream_info->dim.width ||
          stream_info->dim.height != p_client->stream_info->dim.height) {
          IDBG_HIGH("dim change from %dx%d to %dx%d, re-map buffer",
            p_client->stream_info->dim.width, p_client->stream_info->dim.height,
            stream_info->dim.width, stream_info->dim.height);
          module_afs_client_unmap_buffers(p_client);
          p_client->stream_info = stream_info;
          module_afs_client_map_buffers(p_client);
        }
        p_client->out_dim.width  = stream_info->dim.width;
        p_client->out_dim.height = stream_info->dim.height;
        IDBG_MED("%s:%d] MCT_EVENT_MODULE_ISP_OUTPUT_DIM %dx%d",
          __func__, __LINE__,
          p_client->out_dim.width,
          p_client->out_dim.height);
      }
    }
      break;

    default:
      break;
    }
    break;
  }
  default:
    /* forward the event */
    break;
  }

  if (fwd_event)
    module_afs_forward_port_event(p_client, port, event);

  return GET_STATUS(rc);
}

/**
 * Function: module_afs_port_event_fwd_list
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
mct_list_t *module_afs_port_event_fwd_list(unsigned int identity,
  mct_port_t *port)
{
  /*not required for afs*/
  return NULL;
}

/**
 * Function: module_afs_port_ext_link
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
boolean module_afs_port_ext_link(unsigned int identity,
  mct_port_t* port, mct_port_t *peer)
{
  int rc = IMG_SUCCESS;
  unsigned int *p_identity = NULL;
  mct_list_t *p_temp_list = NULL;
  mct_module_t *p_mct_mod = NULL;
  module_afs_t *p_mod = NULL;
  afs_client_t *p_client = NULL;

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

  p_mod = (module_afs_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] afs module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (afs_client_t *)port->port_private;
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
    /* start afs client in case of dynamic module */
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
 * Function: module_afs_port_unlink
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
void module_afs_port_unlink(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  int rc = IMG_SUCCESS;
  mct_list_t *p_temp_list = NULL;
  mct_module_t *p_mct_mod = NULL;
  module_afs_t *p_mod = NULL;
  afs_client_t *p_client = NULL;
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

  p_mod = (module_afs_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] afs module NULL", __func__, __LINE__);
    return;
  }

  p_client = (afs_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] afs client NULL", __func__, __LINE__);
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
 * Function: module_afs_port_set_caps
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
boolean module_afs_port_set_caps(mct_port_t *port,
  mct_port_caps_t *caps)
{
  int rc = IMG_SUCCESS;
  return GET_STATUS(rc);
}

/**
 * Function: module_afs_port_check_caps_reserve
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
boolean module_afs_port_check_caps_reserve(mct_port_t *port, void *peer_caps,
  void *vstream_info)
{
  boolean rc = FALSE;
  mct_stream_info_t *stream_info = (mct_stream_info_t *)vstream_info;
  mct_module_t *p_mct_mod = NULL;
  module_afs_t *p_mod = NULL;
  mct_port_caps_t *p_peer_caps = (mct_port_caps_t *)peer_caps;
  mct_port_caps_t *p_caps = (mct_port_caps_t *)&port->caps;
  afs_client_t *p_client = NULL;
  uint32_t identity;

  if (!port || !stream_info) {
    IDBG_ERROR("%s:%d invalid input %p %p", __func__, __LINE__,
      port, stream_info);
    return FALSE;
  }

  identity = stream_info->identity;
  IDBG_MED("%s:%d] E %x dir %d", __func__, __LINE__, identity,
    MCT_PORT_DIRECTION(port));
  if (MCT_PORT_IS_SINK(port) && !peer_caps) {
    IDBG_ERROR("%s:%d invalid input dir %d %p", __func__, __LINE__,
      MCT_PORT_IS_SINK(port), peer_caps);
    return FALSE;

  } else if (p_peer_caps) {
    if (p_peer_caps->port_caps_type != MCT_PORT_CAPS_FRAME) {
      IDBG_ERROR("%s:%d] invalid capabilities, cannot connect port %x",
        __func__, __LINE__, p_peer_caps->port_caps_type);
      return FALSE;
    }
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

  p_mod = (module_afs_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] afs module NULL", __func__, __LINE__);
    return FALSE;
  }

  if (port->port_private) {
    /* port is already reserved */
    IDBG_ERROR("%s:%d] port already reserved", __func__, __LINE__);
    return FALSE;
  }

  if (MCT_PORT_IS_SRC(port)) {

    mct_list_t *p_temp_list = mct_list_find_custom(p_mod->afs_client, &identity,
      module_afs_find_identity);
    if (NULL == p_temp_list) {
      IDBG_ERROR("%s:%d] cannot find client", __func__, __LINE__);
      return FALSE;
    }
    p_client = (afs_client_t *)p_temp_list->data;
    if (NULL == p_client) {
      IDBG_ERROR("%s:%d] cannot find client", __func__, __LINE__);
      return FALSE;
    }
    pthread_mutex_lock(&p_client->mutex);
    p_client->p_srcport = port;
    port->port_private = p_client;
    pthread_mutex_unlock(&p_client->mutex);
  } else {
    /* lock the module */
    pthread_mutex_lock(&p_mod->mutex);
    rc = module_afs_acquire_port(p_mct_mod, port, stream_info);
    if (FALSE == rc) {
      IDBG_ERROR("%s:%d] Error acquiring port", __func__, __LINE__);
      pthread_mutex_unlock(&p_mod->mutex);
      return FALSE;
    }

    pthread_mutex_unlock(&p_mod->mutex);
  }

  MCT_OBJECT_REFCOUNT(port)++;
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return TRUE;
}

/**
 * Function: module_afs_port_check_caps_unreserve
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
boolean module_afs_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  int rc = IMG_SUCCESS;
  mct_list_t *p_temp_list = NULL;
  mct_module_t *p_mct_mod = NULL;
  module_afs_t *p_mod = NULL;
  afs_client_t *p_client = NULL;

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

  p_mod = (module_afs_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] afs module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (afs_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] afs client NULL", __func__, __LINE__);
    return FALSE;
  }

  /* lock the module */
  pthread_mutex_lock(&p_mod->mutex);

  if (MCT_PORT_IS_SRC(port)) {
    /* do nothing */
    port->port_private = NULL;
  } else {
    /* First remove client form module list */
    p_temp_list = mct_list_find_custom(p_mod->afs_client, &identity,
      module_afs_find_client);
    if (NULL != p_temp_list) {
      p_mod->afs_client = mct_list_remove(p_mod->afs_client,
        p_temp_list->data);
    }
    /* destroy the client */
    port->port_private = NULL;
    module_afs_client_destroy(p_client);
    p_client = NULL;
  }

  pthread_mutex_unlock(&p_mod->mutex);

  MCT_OBJECT_REFCOUNT(port)--;
  /*Todo: free port??*/
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return GET_STATUS(rc);

error:
  pthread_mutex_unlock(&p_mod->mutex);
  IDBG_MED("%s:%d] Error rc = %d X", __func__, __LINE__, rc);
  return FALSE;
}

/**
 * Function: module_afs_request_new_port
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
mct_port_t *module_afs_request_new_port(void *vstream_info,
  mct_port_direction_t direction,
  mct_module_t *module,
  void *peer_caps)
{
  mct_stream_info_t *stream_info = (mct_stream_info_t *)vstream_info;
  boolean rc = IMG_SUCCESS;
  module_afs_t *p_mod = NULL;
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

  p_mod = (module_afs_t *)module->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] afs module NULL", __func__, __LINE__);
    return NULL;
  }

  pthread_mutex_lock(&p_mod->mutex);
  p_port = module_afs_create_port(module, direction);
  if (NULL == p_port) {
    IDBG_ERROR("%s:%d] Error creating port", __func__, __LINE__);
    goto error;
  }
  /*acquire port*/
  rc = module_afs_acquire_port(module, p_port, stream_info);
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
 * Function: module_afs_start_session
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
static boolean module_afs_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  int rc = IMG_SUCCESS;
  module_afs_t *p_mod;

  if (!module) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_afs_t *)module->module_private;
  if (!p_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  /* Add session settings */
  module_afs_create_session_param(p_mod, sessionid);

  /* create message thread */
  rc = module_imglib_create_msg_thread(&p_mod->msg_thread);

  return GET_STATUS(rc);
}

/**
 * Function: module_afs_start_session
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
static boolean module_afs_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  int rc = IMG_SUCCESS;
  module_afs_t *p_mod;

  if (!module) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_afs_t *)module->module_private;
  if (!p_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  /* Check if this need to be here */
  module_afs_destroy_session_param(p_mod, sessionid);

  /* destroy message thread */
  rc = module_imglib_destroy_msg_thread(&p_mod->msg_thread);
  return GET_STATUS(rc);
}

/**
 * Function: module_afs_set_mod
 *
 * Description: This function is used to set the afs module
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
static void module_afs_set_mod(mct_module_t *module,
  unsigned int module_type,
  unsigned int identity)
{
  module_afs_t *p_mod;

  if (!(module && module->module_private)) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return;
  }

  /* Do not touch module mode since currently is only one instance module */
  p_mod = (module_afs_t *)module->module_private;
  p_mod->module_type = module_type;
  mct_module_add_type(module, module_type, identity);

  return;
}

/**
 * Function: module_afs_query_mod
 *
 * Description: This function is used to query the afs module info
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
static boolean module_afs_query_mod(mct_module_t *module, void *query_buf,
  unsigned int sessionid)
{
  mct_pipeline_cap_t *p_mct_cap = (mct_pipeline_cap_t *)query_buf;
  mct_pipeline_imaging_cap_t *p_cap = NULL;
  if (!query_buf || !module) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  IDBG_MED("%s:%d", __func__, __LINE__);

  return TRUE;
}

/**
 * Function: module_afs_free_port
 *
 * Description: This function is used to free the afs ports
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static boolean module_afs_free_port(void *data, void *user_data)
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
 * Function: module_afs_create_port
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
mct_port_t *module_afs_create_port(mct_module_t *p_mct_mod,
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
  p_port->check_caps_reserve    = module_afs_port_check_caps_reserve;
  p_port->check_caps_unreserve  = module_afs_port_check_caps_unreserve;
  p_port->ext_link              = module_afs_port_ext_link;
  p_port->un_link               = module_afs_port_unlink;
  p_port->set_caps              = module_afs_port_set_caps;
  p_port->event_func            = module_afs_port_event_func;

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

/** module_afs_get_next_from_list
 *    @data1: not used
 *    @data2: not used
 *
 *  Gets next element from the list
 *
 *  Return TRUE always
 **/
static boolean module_afs_get_next_from_list(void *data1, void *data2)
{
  return TRUE;
}

/**
 * Function: module_afs_free_mod
 *
 * Description: This function is used to free the afs module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_afs_deinit(mct_module_t *p_mct_mod)
{
  module_afs_t *p_mod = NULL;
  mct_list_t* p_list;
  int rc = 0;
  int i = 0;

  if (NULL == p_mct_mod) {
    IDBG_ERROR("%s:%d] MCTL module NULL", __func__, __LINE__);
    return;
  }

  p_mod = (module_afs_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] afs module NULL", __func__, __LINE__);
    return;
  }

  pthread_mutex_destroy(&p_mod->mutex);
  pthread_cond_destroy(&p_mod->cond);

  do {
    p_list = mct_list_find_custom(MCT_MODULE_SINKPORTS(p_mct_mod), p_mct_mod,
      module_afs_get_next_from_list);
    if (p_list)
      module_afs_free_port(p_list->data, p_mct_mod);
  } while (p_list);

  do {
    p_list = mct_list_find_custom(MCT_MODULE_SRCPORTS(p_mct_mod), p_mct_mod,
      module_afs_get_next_from_list);
    if (p_list)
      module_afs_free_port(p_list->data, p_mct_mod);
  } while (p_list);

  IDBG_MED("%s:%d] delete the clients", __func__, __LINE__);

  /* unload afs module */
  module_afs_unload();

  p_mod->client_cnt = 0;
  free(p_mod);
  p_mod = NULL;
  mct_module_destroy(p_mct_mod);
}

/** module_afs_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: This function is used to initialize the afs module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_afs_init(const char *name)
{
  mct_module_t *p_mct_mod = NULL;
  module_afs_t *p_mod = NULL;
  mct_port_t *p_port = NULL;
  int rc = 0;
  int i = 0;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  p_mct_mod = mct_module_create(name);
  if (NULL == p_mct_mod) {
    IDBG_ERROR("%s:%d cannot allocate mct module", __func__, __LINE__);
    return NULL;
  }
  p_mod = malloc(sizeof(module_afs_t));
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    goto error;
  }

  p_mct_mod->module_private = (void *)p_mod;
  memset(p_mod, 0, sizeof(module_afs_t));

  pthread_mutex_init(&p_mod->mutex, NULL);
  pthread_cond_init(&p_mod->cond, NULL);

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  /* check if the afs module is present */
  rc = module_afs_load();
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d failed %d", __func__, __LINE__, rc);
    goto error;
  }

  p_mod->lib_ref_count++;
  p_mod->afs_client = NULL;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  /* create static ports */
  for (i = 0; i < MAX_AFS_STATIC_PORTS; i++) {
    /* sink port */
    p_port = module_afs_create_port(p_mct_mod, MCT_PORT_SINK);
    if (NULL == p_port) {
      IDBG_ERROR("%s:%d] create port failed", __func__, __LINE__);
      goto error;
    }

    /* src port */
    p_port = module_afs_create_port(p_mct_mod, MCT_PORT_SRC);
    if (NULL == p_port) {
      IDBG_ERROR("%s:%d] create port failed", __func__, __LINE__);
      goto error;
    }
  }

  p_mct_mod->set_mod          = module_afs_set_mod;
  p_mct_mod->query_mod        = module_afs_query_mod;
  p_mct_mod->request_new_port = module_afs_request_new_port;
  p_mct_mod->start_session    = module_afs_start_session;
  p_mct_mod->stop_session     = module_afs_stop_session;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  return p_mct_mod;

error:

  /* unload the library */
  module_afs_unload();

  if (p_mod) {
    module_afs_deinit(p_mct_mod);
  } else if (p_mct_mod) {
    mct_module_destroy(p_mct_mod);
  }
  return NULL;
}
