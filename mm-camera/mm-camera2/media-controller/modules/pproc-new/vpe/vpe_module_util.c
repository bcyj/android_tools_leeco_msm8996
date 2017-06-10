/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vpe_module.h"
#include "vpe_port.h"
#include "camera_dbg.h"
#include "vpe_log.h"

static boolean find_port_with_identity_find_func(void *data, void *user_data)
{
  if(!data || !user_data) {
    CDBG_ERROR("%s:%d: failed, data=%p, user_data=%p\n",
                __func__, __LINE__, data, user_data);
    return FALSE;
  }
  mct_port_t *port = (mct_port_t*) data;
  uint32_t identity = *(uint32_t*) user_data;

  vpe_port_data_t *port_data = (vpe_port_data_t *) MCT_OBJECT_PRIVATE(port);
  int i;
  for(i=0; i<VPE_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state != VPE_PORT_STATE_UNRESERVED &&
        port_data->stream_data[i].identity == identity) {
      return TRUE;
    }
  }
  return FALSE;
}

mct_port_t* vpe_module_find_port_with_identity(mct_module_t *module,
  mct_port_direction_t dir, uint32_t identity)
{
  mct_port_t *port = NULL;
  mct_list_t *templist;
  switch(dir) {
  case MCT_PORT_SRC:
    templist = mct_list_find_custom(
       MCT_MODULE_SRCPORTS(module), &identity,
        find_port_with_identity_find_func);
    if(templist) {
        port = (mct_port_t*)(templist->data);
    }
    break;
  case MCT_PORT_SINK:
    templist = mct_list_find_custom(
       MCT_MODULE_SINKPORTS(module), &identity,
        find_port_with_identity_find_func);
    if(templist) {
      port = (mct_port_t*)(templist->data);
    }
    break;
  default:
    CDBG_ERROR("%s:%d: failed, bad port_direction=%d", __func__, __LINE__, dir);
    return NULL;
  }
  return port;
}

boolean ack_find_func(void* data, void* userdata)
{
  if(!data || !userdata) {
    CDBG_ERROR("%s:%d: failed, data=%p, userdata=%p\n",
                __func__, __LINE__, data, userdata);
    return FALSE;
  }
  vpe_module_ack_t* vpe_ack = (vpe_module_ack_t*) data;
  vpe_module_ack_key_t* key = (vpe_module_ack_key_t*) userdata;
  if(vpe_ack->isp_buf_divert_ack.identity == key->identity &&
     vpe_ack->isp_buf_divert_ack.buf_idx == key->buf_idx) {
    return TRUE;
  }
  return FALSE;
}

vpe_module_ack_t* vpe_module_find_ack_from_list(vpe_module_ctrl_t *ctrl,
  vpe_module_ack_key_t key)
{
  mct_list_t *templist;
  templist = mct_list_find_custom(ctrl->ack_list.list, &key, ack_find_func);
  if(templist) {
    return (vpe_module_ack_t*)(templist->data);
  }
  return NULL;
}

cam_streaming_mode_t vpe_module_get_streaming_mode(mct_module_t *module,
  uint32_t identity)
{
  if (!module) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  mct_port_t* port = vpe_module_find_port_with_identity(module, MCT_PORT_SINK,
                       identity);
  if (!port) {
    CDBG_ERROR("%s:%d port not found, identity=0x%x\n",
      __func__, __LINE__, identity);
    return -EINVAL;
  }
  vpe_port_data_t* port_data = (vpe_port_data_t*) MCT_OBJECT_PRIVATE(port);
  int i;
  for (i=0; i<VPE_MAX_STREAMS_PER_PORT; i++) {
    if (port_data->stream_data[i].identity == identity) {
      return port_data->stream_data[i].streaming_mode;
    }
  }
  return CAM_STREAMING_MODE_MAX;
}

int32_t vpe_module_get_params_for_identity(vpe_module_ctrl_t* ctrl,
  uint32_t identity, vpe_module_session_params_t** session_params,
  vpe_module_stream_params_t** stream_params)
{
  if(!ctrl || !session_params || !stream_params) {
    CDBG("%s:%d: failed, ctrl=%p, session_params=%p, stream_params=%p",
      __func__, __LINE__, ctrl, session_params, stream_params);
    return -EINVAL;
  }
  uint32_t session_id;
  int i,j;
  boolean success = FALSE;
  session_id = VPE_GET_SESSION_ID(identity);
  for(i=0; i < VPE_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i]) {
      if(ctrl->session_params[i]->session_id == session_id) {
        for(j=0; j < VPE_MODULE_MAX_STREAMS; j++) {
          if(ctrl->session_params[i]->stream_params[j]) {
            if(ctrl->session_params[i]->stream_params[j]->identity ==
                identity) {
              *stream_params = ctrl->session_params[i]->stream_params[j];
              *session_params = ctrl->session_params[i];
              success = TRUE;
              break;
            }
          }
        }
      }
    }
    if(success == TRUE) {
      break;
    }
  }
  if(success == FALSE) {
    CDBG_ERROR("%s:%d, failed, identity=0x%x", __func__, __LINE__, identity);
    return -EFAULT;
  }
  CDBG_ERROR("%s got params for identity=0x%x stream.w=%d stream.h=%d\n",
             __func__, identity,
             (*stream_params)->hw_params.input_info.width,
             (*stream_params)->hw_params.input_info.height
             );
  return 0;
}

void vpe_module_dump_stream_params(vpe_module_stream_params_t *stream_params,
  const char* func, int line)
{
  CDBG_LOW("%s:%d, ---------- Dumping stream params %p ------------",
    func, line, stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return;
  }
  CDBG_LOW("%s:%d,\t stream_params.identity=0x%x", func, line,
    stream_params->identity);
  CDBG_LOW("%s:%d,\t stream_params.divert_flags=%d\n", func, line,
    stream_params->div_info.divert_flags);
  CDBG_LOW("%s:%d,\t stream_params.num_passes=%d", func, line,
    stream_params->div_info.num_passes);
  CDBG_LOW("%s:%d,\t stream_params.priority=%d", func, line,
    stream_params->priority);
  CDBG_LOW("%s:%d, ---------------------------------------------------------",
    func, line);
}

boolean vpe_module_util_map_buffer_info(void *d1, void *d2)
{
  mct_stream_map_buf_t          *img_buf = (mct_stream_map_buf_t *)d1;
  vpe_module_stream_buff_info_t *stream_buff_info =
    (vpe_module_stream_buff_info_t *)d2;
  vpe_module_buffer_info_t      *buffer_info;
  mct_list_t                    *list_entry = NULL;

  if (!img_buf || !stream_buff_info) {
    CDBG_ERROR("%s:%d, failed. img_buf=%p stream_buff_info=%p\n", __func__,
      __LINE__, img_buf, stream_buff_info);
    return FALSE;
  }

  buffer_info = malloc(sizeof(vpe_module_buffer_info_t));
  if (NULL == buffer_info) {
    CDBG_ERROR("%s:%d, malloc() failed\n", __func__, __LINE__);
    return FALSE;
  }

  memset((void *)buffer_info, 0, sizeof(vpe_module_buffer_info_t));

  if (img_buf->common_fd == TRUE) {
    buffer_info->fd = img_buf->buf_planes[0].fd;
    buffer_info->index = img_buf->buf_index;
    /* Need to get this information from stream info stored in module.
       But because the structure is reused for all buffer operation viz.
       (Enqueue stream buffer list / process frame) the below fields can be
       set to default */
    buffer_info->offset = 0;;
    buffer_info->native_buff = FALSE;
    buffer_info->processed_divert = FALSE;
  } else {
    CDBG_ERROR("%s:%d] error in supporting multiple planar FD\n", __func__,
      __LINE__);
    free(buffer_info);
    return FALSE;
  }

  list_entry = mct_list_append(stream_buff_info->buff_list,
    buffer_info, NULL, NULL);
  if (NULL == list_entry) {
    CDBG_ERROR("%s: Error appending node\n", __func__);
    free(buffer_info);
    return FALSE;
  }

  stream_buff_info->buff_list = list_entry;
  stream_buff_info->num_buffs++;
  return TRUE;
}

boolean vpe_module_util_free_buffer_info(void *d1, void *d2)
{
  vpe_module_buffer_info_t      *buffer_info =
    (vpe_module_buffer_info_t *)d1;
  vpe_module_stream_buff_info_t *stream_buff_info =
    (vpe_module_stream_buff_info_t *)d2;

  if (!buffer_info || !stream_buff_info) {
    CDBG_ERROR("%s:%d] error buffer_info:%p stream_buff_info:%p\n", __func__,
      __LINE__, buffer_info, stream_buff_info);
    return FALSE;
  }

  if (stream_buff_info->num_buffs == 0) {
    CDBG_ERROR("%s:%d] error in num of buffs\n", __func__, __LINE__);
    return FALSE;
  }

  free(buffer_info);
  stream_buff_info->num_buffs--;
  return TRUE;
}

boolean vpe_module_util_create_hw_stream_buff(void *d1, void *d2)
{
  vpe_module_buffer_info_t        *buffer_info =
    (vpe_module_buffer_info_t *)d1;
  vpe_hardware_stream_buff_info_t *hw_strm_buff_info =
    (vpe_hardware_stream_buff_info_t *)d2;
  uint32_t num_buffs;

  if (!buffer_info || !hw_strm_buff_info) {
    CDBG_ERROR("%s:%d] error buffer_info:%p hw_strm_buff_info:%p\n",
      __func__, __LINE__, buffer_info, hw_strm_buff_info);
    return FALSE;
  }

  /* We make an assumption that a linera array will be provided */
  num_buffs = hw_strm_buff_info->num_buffs;
  hw_strm_buff_info->buffer_info[num_buffs].fd = buffer_info->fd;
  hw_strm_buff_info->buffer_info[num_buffs].index = buffer_info->index;
  hw_strm_buff_info->buffer_info[num_buffs].offset = buffer_info->offset;
  hw_strm_buff_info->buffer_info[num_buffs].native_buff =
    buffer_info->native_buff;
  hw_strm_buff_info->buffer_info[num_buffs].processed_divert =
    buffer_info->processed_divert;

  hw_strm_buff_info->num_buffs++;
  return TRUE;
}

/* vpe_module_invalidate_q_traverse_func:
 *
 * Invalidates and update ACK for one queue entry, based on the identity
 *
 **/
boolean vpe_module_invalidate_q_traverse_func(void* qdata, void* userdata)
{
  if (!qdata || !userdata) {
    CDBG_ERROR("%s:%d, failed, qdata=%p input=%p\n", __func__, __LINE__,
      qdata, userdata);
    return FALSE;
  }
  void** input = (void**)userdata;
  vpe_module_event_t* vpe_event = (vpe_module_event_t *) qdata;
  vpe_module_ctrl_t*  ctrl = (vpe_module_ctrl_t *) input[0];
  uint32_t identity = *(uint32_t*) input[1];
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return FALSE;
  }
  /* invalidate the event and update the ACK from list */
  if(vpe_event->ack_key.identity == identity) {
    vpe_event->invalid = TRUE;
    vpe_module_do_ack(ctrl, vpe_event->ack_key);
  }
  return TRUE;
}
