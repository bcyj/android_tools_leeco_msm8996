/***************************************************************************
* Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/

#include <linux/media.h>
#include "mct_module.h"
#include "module_imgbase.h"

/**
 * STATIC function declarations
 **/
static mct_port_t *module_imgbase_create_port(mct_module_t *p_mct_mod,
  mct_port_direction_t dir);

/**
 * Function: module_imgbase_find_identity
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
static boolean module_imgbase_find_identity(void *p_data, void *p_input)
{
  uint32_t *p_identity = (uint32_t *)p_data;
  uint32_t identity = *((uint32_t *)p_input);

  return (*p_identity == identity);
}

/**
 * Function: module_imgbase_find_client
 *
 * Description: This method is used to find the client
 *
 * Arguments:
 *   @p_fp_data: imgbase client
 *   @p_input: input data
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_imgbase_find_client(void *p_imgbase_data, void *p_input)
{
  imgbase_client_t *p_client = (imgbase_client_t *)p_imgbase_data;
  uint32_t identity = *((uint32_t *)p_input);
  boolean flag = FALSE;
  int32_t i = 0;

  for (i = 0; i < p_client->stream_cnt; i++) {
    if (p_client->stream[i].identity == identity) {
      flag = TRUE;
      p_client->cur_index = i;
      break;
    }
  }
  return flag;
}

/**
 * Function: module_imgbase_find_client_by_session
 *
 * Description: This method is used to find the client by
 *               session id
 *
 * Arguments:
 *   @p_fp_data: imgbase client
 *   @p_input: input data
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_imgbase_find_client_by_session(void *p_imgbase_data,
  void *p_input)
{
  imgbase_client_t *p_client = (imgbase_client_t *)p_imgbase_data;
  uint32_t identity = *((uint32_t *)p_input);
  boolean flag = FALSE;
  int32_t i = 0;

  for (i = 0; i < p_client->stream_cnt; i++) {
    if (IMGLIB_SESSIONID(p_client->stream[i].identity) ==
      IMGLIB_SESSIONID(identity)) {
      flag = TRUE;
      break;
    }
  }
  return flag;
}

/**
 * Function: module_imgbase_query_mod
 *
 * Description: This function is used to query the imgbase module
 * info
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
boolean module_imgbase_query_mod(mct_module_t *module, void *buf,
  unsigned int sessionid)
{
  mct_pipeline_cap_t *p_mct_cap = (mct_pipeline_cap_t *)buf;
  mct_pipeline_pp_cap_t *p_cap;
  module_imgbase_t *p_mod;

  if (!p_mct_cap || !module || !module->module_private) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_imgbase_t *)module->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] imgbase module NULL", __func__, __LINE__);
    return FALSE;
  }

  IDBG_MED("%s_%s:%d: E", __func__, p_mod->name, __LINE__);
  p_cap = &p_mct_cap->pp_cap;
  p_cap->feature_mask |= p_mod->feature_mask;

  if (p_mod->modparams.imgbase_query_mod)
    p_mod->modparams.imgbase_query_mod(p_mct_cap);

  return TRUE;
}

/**
 * Function: module_imgbase_forward_port_event
 *
 * Description: This method is used to forward an event
 * depending on the direction.
 *
 * Arguments:
 *   @p_client: Imblib base client
 *   @port: MCT port pointer
 *   @event: MCT event pointer
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_imgbase_forward_port_event(imgbase_client_t *p_client,
  mct_port_t *port,
  mct_event_t *event)
{
  boolean rc = FALSE;
  mct_port_t *p_adj_port = NULL;
  imgbase_stream_t *p_stream;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;

  /* Todo: handle multiple streams */
  p_stream = &p_client->stream[0];

  if (MCT_PORT_IS_SINK(port)) {
    p_adj_port = p_stream->p_srcport;
    if (NULL == p_adj_port) {
      /* Not an error for sink module */
      return TRUE;
    }
    switch(event->direction) {
      case MCT_EVENT_UPSTREAM : {
        IDBG_ERROR("%s_%s:%d] Error Upstream event on Sink port %d",
          __func__, p_mod->name, __LINE__, event->type);
        break;
      }
      case MCT_EVENT_BOTH:
      case MCT_EVENT_DOWNSTREAM: {
       rc =  mct_port_send_event_to_peer(p_adj_port, event);
       if (rc == FALSE) {
         IDBG_ERROR("%s_%s:%d] Fowarding event %d from sink port failed",
           __func__, p_mod->name, __LINE__, event->type);
       }
       break;
     }
     default:
       IDBG_ERROR("%s_%s:%d] Invalid port direction for event %d",
         __func__, p_mod->name, __LINE__, event->type);
       break;
    }
  } else if (MCT_PORT_IS_SRC(port)) {
    p_adj_port = p_stream->p_sinkport;
    if (NULL == p_adj_port) {
       IDBG_HIGH("%s_%s:%d] Invalid port", __func__, p_mod->name, __LINE__);
       return FALSE;
    }
    switch(event->direction) {
      case MCT_EVENT_DOWNSTREAM : {
        IDBG_ERROR("%s_%s:%d] Error Downstream event on Src port %d",
          __func__, p_mod->name, __LINE__, event->type);
        break;
      }
      case MCT_EVENT_BOTH:
      case MCT_EVENT_UPSTREAM: {
       rc =  mct_port_send_event_to_peer(p_adj_port, event);
       if (rc == FALSE) {
         IDBG_ERROR("%s_%s:%d] Fowarding event %d from src port failed",
           __func__, p_mod->name, __LINE__, event->type);
       }
       break;
     }
     default:
       IDBG_ERROR("%s_%s:%d] Invalid port direction for event %d",
         __func__, p_mod->name, __LINE__, event->type);
       break;
    }
  }
  return rc;
}

/**
 * module_imgbase_extract_fd_info:
 *   @faces_data: fd info
 *   @img_meta: metadata
 *
 * This function extracts fd info
 *
 * This function executes in Imaging Server context
 *
 * Return values: image lib return codes
 **/
static int module_imgbase_extract_fd_info(cam_face_detection_data_t *faces_data,
  img_meta_t *img_meta)
{
  int i = 0;
  img_fd_info_t *fd_info = &img_meta->fd_info;

  img_meta->valid_faces_detected = 0;

  IDBG_MED("%s:%d] rotation %d", __func__, __LINE__,
    img_meta->rotation);

  for (i = 0; i < faces_data->num_faces_detected; i++) {
    /* check orientation of the face; if it is in gravity direction,
        then only it should be passed to trueportrait; if not filter
        that face alone */
    IDBG_MED("%s:%d] roll_dir before sensor mount angle comp off %d, %d",
      __func__, __LINE__, faces_data->faces[i].roll_dir,
      ABS(faces_data->faces[i].roll_dir + img_meta->rotation));

    if ( ( ABS(faces_data->faces[i].roll_dir + img_meta->rotation) < FACE_TILT_CUTOFF_FOR_TP) &&
      (img_meta->valid_faces_detected < MAX_FACES_SUPPORTED_BY_TP) ) {
      fd_info->faceROIx[img_meta->valid_faces_detected] =
          faces_data->faces[i].face_boundary.left;
      fd_info->faceROIy[img_meta->valid_faces_detected] =
          faces_data->faces[i].face_boundary.top;
      fd_info->faceROIWidth[img_meta->valid_faces_detected] =
          faces_data->faces[i].face_boundary.width;
      fd_info->faceROIHeight[img_meta->valid_faces_detected] =
          faces_data->faces[i].face_boundary.height;

      img_meta->valid_faces_detected++;
  }
  IDBG_MED("%s:%d] Face info: %d %d, %d, %d", __func__, __LINE__,
    faces_data->faces[i].left_right_gaze, faces_data->faces[i].top_bottom_gaze,
    faces_data->faces[i].updown_dir,faces_data->faces[i].roll_dir);
  }
  img_meta->fd_frame_dim.width  = faces_data->fd_frame_dim.width;
  img_meta->fd_frame_dim.height = faces_data->fd_frame_dim.height;

  IDBG_MED("%s:%d] Final number of faces filtered for TP is %d outof %d, %d x %d",
    __func__, __LINE__, img_meta->valid_faces_detected, faces_data->num_faces_detected,
    img_meta->fd_frame_dim.width, img_meta->fd_frame_dim.height);

  return IMG_SUCCESS;
}


/**
 * Function: module_imgbase_port_event_func
 *
 * Description: Event handler function for the imgbase port
 *
 * Arguments:
 *   @port: mct port pointer
 *   @event: mct event
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean module_imgbase_port_event_func(mct_port_t *port,
  mct_event_t *event)
{
  int rc = IMG_SUCCESS;
  mct_module_t *p_mct_mod = NULL;
  module_imgbase_t *p_mod = NULL;
  imgbase_client_t *p_client;
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

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] imgbase module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (imgbase_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s_%s:%d] imgbase client NULL", __func__, p_mod->name, __LINE__);
    return FALSE;
  }

  IDBG_LOW("%s_%s:%d] type %d", __func__, p_mod->name, __LINE__, event->type);
  switch (event->type) {
  case MCT_EVENT_CONTROL_CMD: {
    mct_event_control_t *p_ctrl_event = &event->u.ctrl_event;
    IDBG_MED("%s_%s:%d] Ctrl type %d", __func__, p_mod->name, __LINE__,
      p_ctrl_event->type);
    switch (p_ctrl_event->type) {
    case MCT_EVENT_CONTROL_STREAMON: {
      IDBG_HIGH("%s_%s:%d] IMGLIB_BASE STREAMON", __func__, p_mod->name, __LINE__);
      module_imgbase_client_start(p_client);
      break;
    }
    case MCT_EVENT_CONTROL_STREAMOFF: {
      IDBG_MED("%s_%s:%d] imgbase STREAMOFF", __func__, p_mod->name, __LINE__);
      module_imgbase_client_stop(p_client);
      break;
    }
    case MCT_EVENT_CONTROL_PARM_STREAM_BUF: {
      cam_stream_parm_buffer_t *parm_buf =
        event->u.ctrl_event.control_event_data;
      IDBG_HIGH("%s_%s:%d] MCT_EVENT_CONTROL_PARM_STREAM_BUF %d",
        __func__, p_mod->name, __LINE__, parm_buf ? parm_buf->type : 0xffff);
      if (parm_buf &&
        (parm_buf->type == CAM_STREAM_PARAM_TYPE_GET_IMG_PROP)) {
        cam_stream_parm_buffer_t *out_buf;
        out_buf = img_q_dequeue(&p_client->stream_parm_q);
        if (out_buf) {
          *parm_buf = *out_buf;
          free(out_buf);
        }
      }
      break;
    }
    case MCT_EVENT_CONTROL_SET_PARM: {
      mct_event_control_parm_t *ctrl_parm =
        (mct_event_control_parm_t *) event->u.ctrl_event.control_event_data;
      switch (ctrl_parm->type) {
      case CAM_INTF_PARM_ROTATION: {
        cam_rotation_t rotation = *(cam_rotation_t *)ctrl_parm->parm_data;
        IDBG_LOW("%s:%d] rotation %d",
          __func__, __LINE__, *(cam_rotation_t *)ctrl_parm->parm_data);

        if (rotation == ROTATE_0) {
          p_client->current_meta.rotation = 0;
        } else if (rotation == ROTATE_90) {
          p_client->current_meta.rotation = 90;
        } else if (rotation == ROTATE_180) {
          p_client->current_meta.rotation = 180;
        } else if (rotation == ROTATE_270) {
          p_client->current_meta.rotation = -90;
        }
        break;
      }
      case CAM_INTF_PARM_MULTI_TOUCH_FOCUS_BRACKETING: {
        cam_af_bracketing_t *cam_af_bracket = NULL;
        cam_af_bracket = (cam_af_bracketing_t*) ctrl_parm->parm_data;
        IDBG_HIGH("%s:catch MTF set param event: curr # of burst: %d",
          __func__, cam_af_bracket->burst_count);
        p_mod->caps.num_input = cam_af_bracket->burst_count;
        p_client->caps.num_input = cam_af_bracket->burst_count;
        p_client->caps.num_output = p_mod->caps.num_output;
        IDBG_HIGH("%s:updated caps: # input = %d, # output = %d",
          __func__, p_client->caps.num_input, p_client->caps.num_output);
      }
      default:
        break;
      }
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
    IDBG_MED("%s_%s:%d] Mod type %d", __func__, p_mod->name, __LINE__,
      p_mod_event->type);
    switch (p_mod_event->type) {
    case MCT_EVENT_MODULE_BUF_DIVERT: {
      mod_img_msg_t msg;
      isp_buf_divert_t *p_buf_divert =
        (isp_buf_divert_t *)p_mod_event->module_event_data;
      p_buf_divert->identity = event->identity;

      IDBG_ERROR("%s_%s:%d] identity %x", __func__, p_mod->name, __LINE__,
        event->identity);

      rc = module_imgbase_client_handle_buffer(p_client, p_buf_divert);

      /* indicate that the buffer is consumed */
      if(rc == IMG_SUCCESS){
        p_buf_divert->is_locked = FALSE;
        p_buf_divert->ack_flag = FALSE;
        fwd_event = FALSE;
      }

      break;
    }
    case MCT_EVENT_MODULE_STATS_DIS_UPDATE: {
      is_update_t *is_update =
        (is_update_t *) p_mod_event->module_event_data;

      if (is_update) {
        p_client->is_update = *is_update;
        p_client->is_update_valid = TRUE;
      }

      break;
    }
    case MCT_EVENT_MODULE_STREAM_CROP: {
      mct_bus_msg_stream_crop_t *stream_crop =
        (mct_bus_msg_stream_crop_t *) p_mod_event->module_event_data;

      if (stream_crop) {
        p_client->stream_crop = *stream_crop;
        p_client->stream_crop_valid = TRUE;
      }

      break;
    }
    case MCT_EVENT_MODULE_ISP_OUTPUT_DIM: {
      mct_stream_info_t *stream_info =
        (mct_stream_info_t *)(p_mod_event->module_event_data);

      if (stream_info) {
        p_client->isp_output_dim_stream_info = *stream_info;
        p_client->isp_output_dim_stream_info_valid = TRUE;
      }
      break;
    }
    case MCT_EVENT_MODULE_FACE_INFO: {
      cam_face_detection_data_t *faces_data = (cam_face_detection_data_t *)
        p_mod_event->module_event_data;

      module_imgbase_extract_fd_info (faces_data, &p_client->current_meta);
      break;
    }
    case MCT_EVENT_MODULE_STATS_AWB_UPDATE: {
      stats_update_t *stats_update = (stats_update_t *)
        p_mod_event->module_event_data;
      break;
    }
    case MCT_EVENT_MODULE_QUERY_DIVERT_TYPE: {
      uint32_t *divert_mask = (uint32_t *)p_mod_event->module_event_data;
      *divert_mask |= PPROC_DIVERT_PROCESSED;
      p_client->divert_mask = *divert_mask;
      break;
    }
    case MCT_EVENT_MODULE_SET_CHROMATIX_PTR:
      break;
    default:
      break;
    }
    break;
  }
  default:
   break;
  }

  if (fwd_event) {
    boolean brc = module_imgbase_forward_port_event(p_client, port, event);
    rc = (brc) ? IMG_SUCCESS : IMG_ERR_GENERAL;
  }

  return GET_STATUS(rc);
}

/**
 * Function: module_imgbase_is_stream_compatible
 *
 * Description: This method is used to check if the stream is
 * compatible with the port. Certain streams can be combined on
 * the same ports depending on the requirement.
 *
 * Arguments:
 *   @stream_info: Stream requesting port
 *   @p_client: Client associated with the port
 *
 * Return values:
 *     TRUE if compatible
 *     FALSE otherwise
 *
 * Notes: none
 **/
static boolean module_imgbase_is_stream_compatible(
  mct_stream_info_t *stream_info, imgbase_client_t *p_client)
{
  boolean result = TRUE;
  int i = 0;
  unsigned int port_sess_id = 0, str_session_id = 0;
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;

  for (i = 0; i < p_client->stream_cnt; i++) {
    port_sess_id = IMGLIB_SESSIONID(p_client->stream[i].identity);
    str_session_id = IMGLIB_SESSIONID(stream_info->identity);

    IDBG_MED("%s_%s %d:] Port sessionid %d, Stream session ID %d",
      __func__, p_mod->name, __LINE__, port_sess_id, str_session_id);

    //If session IDs are diferent, cannot connect port
    if (port_sess_id != str_session_id) {
      IDBG_ERROR("%s_%s %d] Port already connected on SessionID %d",
        __func__, p_mod->name, __LINE__, port_sess_id);
      return FALSE;
    }

    IDBG_MED("%s_%s %d:] Port stream type %d, input stream type %d",
      __func__, p_mod->name, __LINE__,
      p_client->stream[i].stream_info->stream_type,
      stream_info->stream_type );

    switch (p_client->stream[i].stream_info->stream_type) {
    case CAM_STREAM_TYPE_PREVIEW:
      if ((stream_info->stream_type != CAM_STREAM_TYPE_VIDEO) &&
        (stream_info->stream_type != CAM_STREAM_TYPE_PREVIEW)) {
        result = FALSE;
      }
    break;
    case CAM_STREAM_TYPE_SNAPSHOT:
      result = FALSE;
    break;
    case CAM_STREAM_TYPE_VIDEO:
      if ((stream_info->stream_type != CAM_STREAM_TYPE_PREVIEW) &&
        (stream_info->stream_type != CAM_STREAM_TYPE_VIDEO)){
        result = FALSE;
      }
    break;
    case CAM_STREAM_TYPE_OFFLINE_PROC:
      result = FALSE;
    break;
    default:
      result = FALSE;
    break;
    }
  }
  return result;
}

/**
 * Function: module_imgbase_fill_stream
 *
 * Description: This function is used to fill the stream details
 *
 * Arguments:
 *   @p_client: imgbase client
 *   @stream_info: pointer to stream info
 *   @port: port
 *
 * Return values:
 *     error values
 *
 * Notes: none
 **/
int module_imgbase_fill_stream(imgbase_client_t *p_client,
  mct_stream_info_t *stream_info,
  mct_port_t *port)
{
  imgbase_stream_t *p_stream;
  int32_t status = IMG_ERR_NOT_FOUND;
  module_imgbase_t *p_mod = NULL;

  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] Error invalid client",
      __func__, __LINE__);
    return IMG_ERR_OUT_OF_BOUNDS;
  }

  p_mod = (module_imgbase_t *)p_client->p_mod;

  IDBG_MED("%s_%s %d] Stream %d, port %s",  __func__,  p_mod->name, __LINE__,
    stream_info->stream_type, MCT_PORT_NAME(port));
  if (p_client->stream_cnt >= MAX_IMGLIB_BASE_MAX_STREAM) {
    IDBG_ERROR("%s_%s:%d] Error max ports reached",
      __func__, p_mod->name, __LINE__);
    return IMG_ERR_OUT_OF_BOUNDS;
  }

  if (MCT_PORT_IS_SINK(port)) {
    IDBG_MED("%s_%s %d]: Port %s is sink port",  __func__,  p_mod->name,
      __LINE__, MCT_PORT_NAME(port));
    p_stream = &p_client->stream[p_client->stream_cnt];
    p_stream->stream_info = stream_info;
    p_stream->identity = stream_info->identity;
    p_stream->p_sinkport = port;
    status = IMG_SUCCESS;
    p_stream->p_srcport = NULL;
    p_client->stream_cnt++;
  } else { /* src port */
    int32_t i = 0;
    for (i = 0; i < p_client->stream_cnt; i++) {
      IDBG_MED("%s_%s:%d] Src %x %x", __func__, p_mod->name, __LINE__,
        p_client->stream[i].identity, stream_info->identity);
      if (p_client->stream[i].identity ==
        stream_info->identity) {
        p_client->stream[i].p_srcport = port;
        status = IMG_SUCCESS;
        break;
      }
    }
  }
  return status;
}

/**
 * Function: module_imgbase_port_acquire
 *
 * Description: This function is used to acquire the port
 *
 * Arguments:
 *   @p_mct_mod: mct module pointer
 *   @port: mct port pointer
 *   @stream_info: stream information
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean module_imgbase_port_acquire(mct_module_t *p_mct_mod,
  mct_port_t *port,
  mct_stream_info_t *stream_info)
{
  int rc = IMG_SUCCESS;
  unsigned int p_identity;
  mct_list_t *p_temp_list = NULL;
  imgbase_client_t *p_client = NULL;
  module_imgbase_t *p_mod = NULL;
  imgbase_stream_t *p_stream;
  boolean is_compatible = TRUE;

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] imgbase module NULL", __func__, __LINE__);
    return FALSE;
  }
  IDBG_MED("%s_%s:%d] E", __func__, p_mod->name, __LINE__);
  p_identity =  stream_info->identity;

  /* check if its sink port*/
  if (MCT_PORT_IS_SINK(port)) {

    rc = module_imglib_common_get_bfr_mngr_subdev(&p_mod->subdevfd);
    if (!rc || p_mod->subdevfd < 0) {
      IDBG_ERROR("%s_%s:%d] Error rc %d fd %d", __func__, p_mod->name, __LINE__,
        rc, p_mod->subdevfd);
      goto error;
    }
    /* create imgbase client */
    rc = module_imgbase_client_create(p_mct_mod, port, p_identity, stream_info);
    if (IMG_SUCCEEDED(rc)) {
      p_client = port->port_private;
    }
  } else {
    /*The port may already be used by another stream.
        Check if current stream is compatible*/
    if (port->object.refcount > 0) {
      is_compatible =
        module_imgbase_is_stream_compatible(stream_info, port->port_private);
    }
    if (is_compatible) {
        /* update the internal connection with source port */
        p_temp_list = mct_list_find_custom(p_mod->imgbase_client, &p_identity,
          module_imgbase_find_client);
        if (NULL != p_temp_list) {
          p_client = p_temp_list->data;
          port->port_private = p_client;
          IDBG_MED("%s_%s:%d] found client %p", __func__, p_mod->name, __LINE__,
            p_client);
        } else {
          IDBG_ERROR("%s_%s:%d] cannot find the client", __func__, p_mod->name,
            __LINE__);
          goto error;
        }
    } else {
      IDBG_ERROR("%s_%s:%d] Port already reserved", __func__, p_mod->name,
        __LINE__);
      goto error;
    }
  }

  if (IMG_SUCCEEDED(rc))
    rc = module_imgbase_fill_stream(p_client, stream_info, port);

  IDBG_MED("%s_%s:%d] port %p rc %dX", __func__, p_mod->name, __LINE__, port, rc);
  return GET_STATUS(rc);

error:

  IDBG_MED("%s_%s:%d] Error X", __func__, p_mod->name, __LINE__);
  return FALSE;

}

/**
 * Function: module_imgbase_port_check_caps_reserve
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
boolean module_imgbase_port_check_caps_reserve(mct_port_t *port,
  void *peer_caps,
  void *vstream_info)
{
  boolean rc = FALSE;
  mct_module_t *p_mct_mod = NULL;
  module_imgbase_t *p_mod = NULL;
  mct_stream_info_t *stream_info = (mct_stream_info_t *)vstream_info;
  boolean is_compatible = TRUE;
  int ret = IMG_SUCCESS;

  if (!port || !stream_info) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  IDBG_MED("%s:%d] E %s %d", __func__, __LINE__, MCT_PORT_NAME(port),
    stream_info->stream_type);
  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] imgbase module NULL", __func__, __LINE__);
    return FALSE;
  }

  /* lock the module */
  pthread_mutex_lock(&p_mod->mutex);
  int ref_count = MCT_OBJECT_REFCOUNT(port);

  if (port->port_private && (0 == ref_count)) {
    /* port is already reserved */
    IDBG_MED("%s_%s:%d] Error port is already reserved %d",
      __func__, p_mod->name, __LINE__, ref_count);
    pthread_mutex_unlock(&p_mod->mutex);
    return FALSE;
  }

  if ((ref_count > 0) && MCT_PORT_IS_SINK(port) && port->port_private) {
    imgbase_client_t *p_client = port->port_private;
    is_compatible = module_imgbase_is_stream_compatible(stream_info,
      port->port_private);
    if (is_compatible) {
      ret = module_imgbase_fill_stream(p_client, stream_info, port);
      if (IMG_SUCCEEDED(ret)) {
        rc = TRUE;
      }
    } else {
      IDBG_ERROR("%s_%s %d] Port already reserved", __func__, p_mod->name,
         __LINE__);
      pthread_mutex_unlock(&p_mod->mutex);
      rc = FALSE;
    }
  } else {
    rc = module_imgbase_port_acquire(p_mct_mod, port, stream_info);
  }
  if (FALSE == rc) {
    IDBG_ERROR("%s_%s:%d] Error acquiring port %d", __func__, p_mod->name,
      __LINE__, ref_count);
    pthread_mutex_unlock(&p_mod->mutex);
    return FALSE;
  }

  MCT_OBJECT_REFCOUNT(port)++;
  IDBG_MED("%s_%s:%d] count %d X", __func__, p_mod->name, __LINE__,
    MCT_OBJECT_REFCOUNT(port));
  pthread_mutex_unlock(&p_mod->mutex);

  return TRUE;

}

/**
 * Function: module_imgbase_port_release_client
 *
 * Description: This method is used to release the client after all the
 *                 ports are destroyed
 *
 * Arguments:
 *   @p_mod: pointer to the imgbase module
 *   @identity: stream/session id
 *   @p_client: imgbase client
 *   @identity: port identity
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_imgbase_port_release_client(module_imgbase_t *p_mod,
  mct_port_t *port,
  imgbase_client_t *p_client,
  unsigned int identity)
{
  mct_list_t *p_temp_list = NULL;
  p_temp_list = mct_list_find_custom(p_mod->imgbase_client, &identity,
    module_imgbase_find_client);
  if (NULL != p_temp_list) {
    IDBG_ERROR("%s_%s:%d] ", __func__, p_mod->name, __LINE__);
    p_mod->imgbase_client = mct_list_remove(p_mod->imgbase_client,
      p_temp_list->data);
  }
  module_imgbase_client_destroy(p_client);
}

/**
 * Function: module_imgbase_port_check_caps_unreserve
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
boolean module_imgbase_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  int rc = IMG_SUCCESS;
  mct_module_t *p_mct_mod = NULL;
  module_imgbase_t *p_mod = NULL;
  imgbase_client_t *p_client = NULL;
  uint32_t *p_identity = NULL;

  if (!port) {
    IDBG_ERROR("%s:%d invalid input ", __func__, __LINE__);
    return FALSE;
  }

  IDBG_MED("%s:%d] E", __func__, __LINE__);

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] imgbase module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (imgbase_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s_%s:%d] imgbase client NULL", __func__, p_mod->name, __LINE__);
    return FALSE;
  }

  /* lock the module */
  pthread_mutex_lock(&p_mod->mutex);
  MCT_OBJECT_REFCOUNT(port)--;

  if (MCT_PORT_IS_SRC(port)) {
    if (MCT_OBJECT_REFCOUNT(port) == 0) {
      port->port_private = NULL;
      MCT_PORT_PEER(port) = NULL;
    }
  } else {
    if (MCT_OBJECT_REFCOUNT(port) == 0) {
      module_imgbase_port_release_client(p_mod, port, p_client, identity);
      port->port_private = NULL;
      if (p_mod->subdevfd >= 0) {
        close(p_mod->subdevfd);
        p_mod->subdevfd = -1;
      }

      MCT_PORT_PEER(port) = NULL;
    }
  }

  pthread_mutex_unlock(&p_mod->mutex);

  /*Todo: free port??*/
  IDBG_MED("%s_%s:%d] count %d X", __func__, p_mod->name, __LINE__,
    MCT_OBJECT_REFCOUNT(port));
  return GET_STATUS(rc);

error:
  pthread_mutex_unlock(&p_mod->mutex);
  IDBG_MED("%s_%s:%d] Error rc = %d X", __func__, p_mod->name, __LINE__, rc);
  return FALSE;

}

/**
 * Function: module_imgbase_port_ext_link
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
boolean module_imgbase_port_ext_link(unsigned int identity,
  mct_port_t* port, mct_port_t *peer)
{
  int rc = IMG_SUCCESS;
  unsigned int *p_identity = NULL;
  mct_list_t *p_temp_list = NULL;
  mct_module_t *p_mct_mod = NULL;
  module_imgbase_t *p_mod = NULL;
  imgbase_client_t *p_client = NULL;

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

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] imgbase module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (imgbase_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s_%s:%d] invalid client", __func__, p_mod->name, __LINE__);
    return FALSE;
  }

  if (MCT_PORT_PEER(port)) {
    IDBG_ERROR("%s_%s:%d] link already established", __func__, p_mod->name, __LINE__);
    return TRUE;
  }

  MCT_PORT_PEER(port) = peer;

  /* check if its sink port*/
  if (MCT_PORT_IS_SINK(port)) {
    /* start imgbase client in case of dynamic module */
  } else {
    /* do nothing for source port */
  }
  IDBG_MED("%s_%s:%d] X", __func__, p_mod->name, __LINE__);
  return GET_STATUS(rc);

error:
  IDBG_MED("%s_%s:%d] Error X", __func__, p_mod->name, __LINE__);
  return FALSE;

}

/**
 * Function: module_imgbase_port_unlink
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
void module_imgbase_port_unlink(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  int rc = IMG_SUCCESS;
  mct_list_t *p_temp_list = NULL;
  mct_module_t *p_mct_mod = NULL;
  module_imgbase_t *p_mod = NULL;
  imgbase_client_t *p_client = NULL;
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

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] imgbase module NULL", __func__, __LINE__);
    return;
  }

  p_client = (imgbase_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] imgbase client NULL", __func__, __LINE__);
    return;
  }

  if (MCT_PORT_IS_SINK(port)) {
    /* stop the client in case of dynamic module */
  } else {
    /* do nothing for source port*/
  }

  IDBG_MED("%s_%s:%d] X", __func__, p_mod->name, __LINE__);
  return;

error:
  IDBG_MED("%s_%s:%d] Error X", __func__, p_mod->name, __LINE__);

}

/**
 * Function: module_imgbase_port_set_caps
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
boolean module_imgbase_port_set_caps(mct_port_t *port,
  mct_port_caps_t *caps)
{
  int rc = IMG_SUCCESS;
  return GET_STATUS(rc);
}

/**
 * Function: module_imgbase_free_port
 *
 * Description: This function is used to free the imgbase ports
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static boolean module_imgbase_free_port(void *data, void *user_data)
{
  mct_port_t *p_port = (mct_port_t *)data;
  mct_module_t *p_mct_mod = (mct_module_t *)user_data;
  boolean rc = FALSE;

  if (!p_port || !p_mct_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return TRUE;
  }
  IDBG_MED("%s:%d port %p p_mct_mod %p", __func__, __LINE__, p_port,
    p_mct_mod);

  rc = mct_module_remove_port(p_mct_mod, p_port);
  if (rc == FALSE) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
  }
  mct_port_destroy(p_port);
  return TRUE;
}

/**
 * Function: module_imgbase_create_port
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
mct_port_t *module_imgbase_create_port(mct_module_t *p_mct_mod,
  mct_port_direction_t dir)
{
  char portname[PORT_NAME_LEN];
  mct_port_t *p_port = NULL;
  int status = IMG_SUCCESS;
  int index = 0;
  module_imgbase_t *p_mod;

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
  IDBG_MED("%s:%d portname %s", __func__,  __LINE__, portname);

  p_port->direction = dir;
  p_port->port_private = NULL;
  p_port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;

  /* override the function pointers */
  p_port->check_caps_reserve    = module_imgbase_port_check_caps_reserve;
  p_port->check_caps_unreserve  = module_imgbase_port_check_caps_unreserve;
  p_port->ext_link              = module_imgbase_port_ext_link;
  p_port->un_link               = module_imgbase_port_unlink;
  p_port->set_caps              = module_imgbase_port_set_caps;
  p_port->event_func            = module_imgbase_port_event_func;
   /* add port to the module */
  if (!mct_module_add_port(p_mct_mod, p_port)) {
    IDBG_ERROR("%s:%d] add port failed", __func__, __LINE__);
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

/**
 * Function: module_imgbase_deinit
 *
 * Description: This function is used to free the imgbase module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_imgbase_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_t *p_mod = NULL;
  img_core_ops_t *p_core_ops = NULL;
  mct_list_t* p_list;
  int rc = 0;
  int i = 0;

  if (NULL == p_mct_mod) {
    IDBG_ERROR("%s:%d] MCTL module NULL", __func__, __LINE__);
    return;
  }

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] imgbase module NULL", __func__, __LINE__);
    return;
  }

  do {
    p_list = mct_list_find_custom(MCT_MODULE_SINKPORTS(p_mct_mod), p_mct_mod,
      module_imglib_get_next_from_list);
    if (p_list)
      module_imgbase_free_port(p_list->data, p_mct_mod);
  } while (p_list);

  do {
    p_list = mct_list_find_custom(MCT_MODULE_SRCPORTS(p_mct_mod), p_mct_mod,
      module_imglib_get_next_from_list);
    if (p_list)
      module_imgbase_free_port(p_list->data, p_mct_mod);
  } while (p_list);


  p_core_ops = &p_mod->core_ops;
  IDBG_MED("%s_%s:%d lib_ref_cnt %d", __func__, p_mod->name, __LINE__, p_mod->lib_ref_count);
  if (p_mod->lib_ref_count) {
    IMG_COMP_UNLOAD(p_core_ops);
  }
  p_mod->imgbase_client_cnt = 0;
  pthread_mutex_destroy(&p_mod->mutex);
  pthread_cond_destroy(&p_mod->cond);
  free(p_mod);
  p_mod = NULL;

  mct_module_destroy(p_mct_mod);
}

/**
 * Function: module_imgbase_start_session
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
static boolean module_imgbase_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  int rc = IMG_SUCCESS;
  module_imgbase_t *p_mod;

  if (!module) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_imgbase_t *)module->module_private;
  if (!p_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  IDBG_HIGH("%s_%s:%d] ", __func__, p_mod->name, __LINE__);
  /* create message thread */
  rc = module_imglib_create_msg_thread(&p_mod->msg_thread);

  return GET_STATUS(rc);
}

/**
 * Function: module_imgbase_stop_session
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
static boolean module_imgbase_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  int rc = IMG_SUCCESS;
  module_imgbase_t *p_mod;

  if (!module) {
    IDBG_ERROR("%s:%d failed", __func__,  __LINE__);
    return FALSE;
  }

  p_mod = (module_imgbase_t *)module->module_private;
  if (!p_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  IDBG_HIGH("%s_%s:%d] ", __func__, p_mod->name, __LINE__);
  /* destroy message thread */
  rc = module_imglib_destroy_msg_thread(&p_mod->msg_thread);
  return GET_STATUS(rc);
}

/** module_imgbase_set_parent:
 *
 *  Arguments:
 *  @p_parent - parent module pointer
 *
 * Description: This function is used to set the parent pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_imgbase_set_parent(mct_module_t *p_mct_mod, mct_module_t *p_parent)
{
  module_imgbase_t *p_mod = NULL;

  if (!p_mct_mod || !p_parent)
    return;

  p_mod = (module_imgbase_t *)p_mct_mod->module_private;

  if (!p_mod)
    return;

  p_mod->parent_mod = p_parent;
}

/** module_imgbase_init:
 *
 *  Arguments:
 *  @name - name of the module
 *  @comp_role: imaging component role
 *  @comp_name: imaging component name
 *  @mod_private: derived structure pointer
 *  @p_caps: imaging capability
 *  @lib_name: library name
 *  @feature_mask: feature mask of imaging algo
 *  @p_modparams: module parameters
 *
 * Description: This function is used to initialize the imgbase
 * module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_imgbase_init(const char *name,
  img_comp_role_t comp_role,
  char *comp_name,
  void *mod_private,
  img_caps_t *p_caps,
  char *lib_name,
  uint32_t feature_mask,
  module_imgbase_params_t *p_modparams)
{
  mct_module_t *p_mct_mod = NULL;
  module_imgbase_t *p_mod = NULL;
  img_core_ops_t *p_core_ops = NULL;
  mct_port_t *p_sinkport = NULL, *p_sourceport = NULL;
  int rc = 0;
  int i = 0;

  if (!name || !comp_name || (comp_role >= IMG_COMP_ROLE_MAX)
    || !p_caps || !lib_name || (0 == feature_mask)) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return NULL;
  }

  IDBG_MED("%s_%s:%d] ", __func__, name, __LINE__);
  p_mct_mod = mct_module_create(name);
  if (NULL == p_mct_mod) {
    IDBG_ERROR("%s_%s:%d] cannot allocate mct module", __func__, name, __LINE__);
    return NULL;
  }

  p_mod = malloc(sizeof(module_imgbase_t));
  if (NULL == p_mod) {
    IDBG_ERROR("%s_%s:%d] failed", __func__, name, __LINE__);
    goto error;
  }

  p_mct_mod->module_private = (void *)p_mod;
  memset(p_mod, 0, sizeof(module_imgbase_t));

  pthread_mutex_init(&p_mod->mutex, NULL);
  pthread_cond_init(&p_mod->cond, NULL);
  p_core_ops = &p_mod->core_ops;

  IDBG_MED("%s_%s:%d] ", __func__, name, __LINE__);
  /* check if the imgbase module is present */
  rc = img_core_get_comp(comp_role, comp_name, p_core_ops);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s_%s:%d] Error rc %d", __func__, name, __LINE__, rc);
    goto error;
  }
 /* try to load the component */
  rc = IMG_COMP_LOAD(p_core_ops, lib_name);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s_%s:%d] Error rc %d", __func__, name, __LINE__, rc);
    goto error;
  }
  p_mod->lib_ref_count++;
  p_mod->imgbase_client = NULL;
  p_mod->mod_private = mod_private;
  p_mod->caps = *p_caps;
  p_mod->subdevfd = -1;
  p_mod->feature_mask = feature_mask;
  p_mod->name = name;
  if (p_modparams)
    p_mod->modparams = *p_modparams;

  IDBG_MED("%s_%s:%d] ", __func__, p_mod->name, __LINE__);
  /* create static ports */
  for (i = 0; i < MAX_IMGLIB_BASE_STATIC_PORTS; i++) {
    p_sinkport = module_imgbase_create_port(p_mct_mod, MCT_PORT_SINK);
    if (NULL == p_sinkport) {
      IDBG_ERROR("%s_%s:%d] create SINK port failed", __func__, p_mod->name, __LINE__);
      goto error;
    }
    p_sourceport = module_imgbase_create_port(p_mct_mod, MCT_PORT_SRC);
    if (NULL == p_sourceport) {
      IDBG_ERROR("%s_%s:%d] create SINK port failed", __func__, p_mod->name, __LINE__);
      goto error;
    }
  }

  p_mct_mod->start_session    = module_imgbase_start_session;
  p_mct_mod->stop_session     = module_imgbase_stop_session;
  p_mct_mod->query_mod        = module_imgbase_query_mod;
  IDBG_MED("%s_%s:%d] %p", __func__, p_mod->name, __LINE__, p_mct_mod);
  return p_mct_mod;

error:
  if (p_mod) {
    module_imgbase_deinit(p_mct_mod);
  } else if (p_mct_mod) {
    mct_module_destroy(p_mct_mod);
    p_mct_mod = NULL;
  }
  return NULL;
}
