/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "c2d_port.h"
#include "c2d_module.h"
#include "camera_dbg.h"
#include "c2d_log.h"

static boolean find_port_with_identity_find_func(void *data, void *user_data)
{
  if(!data || !user_data) {
    CDBG_ERROR("%s:%d: failed, data=%p, user_data=%p\n",
                __func__, __LINE__, data, user_data);
    return FALSE;
  }
  mct_port_t *port = (mct_port_t*) data;
  uint32_t identity = *(uint32_t*) user_data;

  c2d_port_data_t *port_data = (c2d_port_data_t *) MCT_OBJECT_PRIVATE(port);
  int i;
  for(i=0; i<C2D_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state != C2D_PORT_STATE_UNRESERVED &&
        port_data->stream_data[i].identity == identity) {
      return TRUE;
    }
  }
  return FALSE;
}

mct_port_t* c2d_module_find_port_with_identity(mct_module_t *module,
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
  c2d_module_ack_t* c2d_ack = (c2d_module_ack_t*) data;
  c2d_module_ack_key_t* key = (c2d_module_ack_key_t*) userdata;
  if(c2d_ack->isp_buf_divert_ack.identity == key->identity &&
     c2d_ack->isp_buf_divert_ack.buf_idx == key->buf_idx) {
    return TRUE;
  }
  return FALSE;
}

c2d_module_ack_t* c2d_module_find_ack_from_list(c2d_module_ctrl_t *ctrl,
  c2d_module_ack_key_t key)
{
  mct_list_t *templist;
  templist = mct_list_find_custom(ctrl->ack_list.list, &key, ack_find_func);
  if(templist) {
    return (c2d_module_ack_t*)(templist->data);
  }
  return NULL;
}

cam_streaming_mode_t c2d_module_get_streaming_mode(mct_module_t *module,
  uint32_t identity)
{
  if (!module) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  mct_port_t* port = c2d_module_find_port_with_identity(module, MCT_PORT_SINK,
                       identity);
  if (!port) {
    CDBG_ERROR("%s:%d port not found, identity=0x%x\n",
      __func__, __LINE__, identity);
    return -EINVAL;
  }
  c2d_port_data_t* port_data = (c2d_port_data_t*) MCT_OBJECT_PRIVATE(port);
  int i;
  for (i=0; i<C2D_MAX_STREAMS_PER_PORT; i++) {
    if (port_data->stream_data[i].identity == identity) {
      return port_data->stream_data[i].streaming_mode;
    }
  }
  return CAM_STREAMING_MODE_MAX;
}

int32_t c2d_module_get_params_for_identity(c2d_module_ctrl_t* ctrl,
  uint32_t identity, c2d_module_session_params_t** session_params,
  c2d_module_stream_params_t** stream_params)
{
  if(!ctrl || !session_params || !stream_params) {
    CDBG("%s:%d: failed, ctrl=%p, session_params=%p, stream_params=%p",
      __func__, __LINE__, ctrl, session_params, stream_params);
    return -EINVAL;
  }
  uint32_t session_id;
  int i,j;
  boolean success = FALSE;
  session_id = C2D_GET_SESSION_ID(identity);
  for(i=0; i < C2D_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i]) {
      if(ctrl->session_params[i]->session_id == session_id) {
        for(j=0; j < C2D_MODULE_MAX_STREAMS; j++) {
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
  return 0;
}

void c2d_module_dump_stream_params(c2d_module_stream_params_t *stream_params,
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
  CDBG_LOW("%s:%d,\t stream_params.priority=%d", func, line,
    stream_params->priority);
  CDBG_LOW("%s:%d, ---------------------------------------------------------",
    func, line);
}

boolean c2d_module_util_map_buffer_info(void *d1, void *d2)
{
  mct_stream_map_buf_t          *img_buf = (mct_stream_map_buf_t *)d1;
  c2d_module_stream_buff_info_t *stream_buff_info =
    (c2d_module_stream_buff_info_t *)d2;
  c2d_module_buffer_info_t      *buffer_info;
  mct_list_t                    *list_entry = NULL;

  if (!img_buf || !stream_buff_info) {
    CDBG_ERROR("%s:%d, failed. img_buf=%p stream_buff_info=%p\n", __func__,
      __LINE__, img_buf, stream_buff_info);
    return FALSE;
  }

  buffer_info = malloc(sizeof(c2d_module_buffer_info_t));
  if (NULL == buffer_info) {
    CDBG_ERROR("%s:%d, malloc() failed\n", __func__, __LINE__);
    return FALSE;
  }

  memset((void *)buffer_info, 0, sizeof(c2d_module_buffer_info_t));

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

boolean c2d_module_util_free_buffer_info(void *d1, void *d2)
{
  c2d_module_buffer_info_t      *buffer_info =
    (c2d_module_buffer_info_t *)d1;
  c2d_module_stream_buff_info_t *stream_buff_info =
    (c2d_module_stream_buff_info_t *)d2;

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

boolean c2d_module_util_create_hw_stream_buff(void *d1, void *d2)
{
  c2d_module_buffer_info_t        *buffer_info =
    (c2d_module_buffer_info_t *)d1;
  c2d_hardware_stream_buff_info_t *hw_strm_buff_info =
    (c2d_hardware_stream_buff_info_t *)d2;
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

/* c2d_module_invalidate_q_traverse_func:
 *
 * Invalidates and update ACK for one queue entry, based on the identity
 *
 **/
boolean c2d_module_invalidate_q_traverse_func(void* qdata, void* userdata)
{
  if (!qdata || !userdata) {
    CDBG_ERROR("%s:%d, failed, qdata=%p input=%p\n", __func__, __LINE__,
      qdata, userdata);
    return FALSE;
  }
  void** input = (void**)userdata;
  c2d_module_event_t* c2d_event = (c2d_module_event_t *) qdata;
  c2d_module_ctrl_t*  ctrl = (c2d_module_ctrl_t *) input[0];
  uint32_t identity = *(uint32_t*) input[1];
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return FALSE;
  }
  /* invalidate the event and update the ACK from list */
  if(c2d_event->ack_key.identity == identity) {
    c2d_event->invalid = TRUE;
    c2d_module_do_ack(ctrl, c2d_event->ack_key);
  }
  return TRUE;
}

/** c2d_module_update_hfr_skip:
 *
 *  Description:
 *    Based on input and output fps, calculte the skip count
 *    according to this formula,
 *      count = floor(input/output) - 1, if input > output
 *            = 0, otherwise
 *
 **/
int32_t c2d_module_update_hfr_skip(c2d_module_stream_params_t *stream_params)
{
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EINVAL;
  }
  stream_params->hfr_skip_info.skip_count =
    floor(stream_params->hfr_skip_info.input_fps /
          stream_params->hfr_skip_info.output_fps) - 1;
  if(stream_params->hfr_skip_info.skip_count < 0) {
    stream_params->hfr_skip_info.skip_count = 0;
  }
  return 0;
}

/** c2d_module_get_divert_info:
 *
 *  Description:
 *    Based on streamon state of "this" stream and "linked"
 *      stream fetch the divert configuration sent by pproc
 *      module or otherwise return NULL
 *
 **/
pproc_divert_info_t *c2d_module_get_divert_info(uint32_t *identity_list,
  uint32_t identity_list_size, c2d_divert_info_t *c2d_divert_info)
{
  uint32_t i = 0, j = 0;
  uint8_t identity_mapped_idx = 0;
  uint8_t divert_info_config_table_idx = 0;
  pproc_divert_info_t *divert_info = NULL;

  /* Loop through the identity list to determine the corresponding index
     in the cpp_divert_info */
  for (i = 0; i < identity_list_size; i++) {
    if (identity_list[i] != PPROC_INVALID_IDENTITY) {
      /* Search the requested identity from the c2d_divert_info table */
      identity_mapped_idx = 0;
      for (j = 0; j < C2D_MAX_STREAMS_PER_PORT; j++) {
        if (c2d_divert_info->identity[j] == identity_list[i]) {
          identity_mapped_idx = j;
          break;
        }
      }
      if (j < C2D_MAX_STREAMS_PER_PORT) {
        divert_info_config_table_idx |= (1 << identity_mapped_idx);
      }
    }
  }

  if(divert_info_config_table_idx) {
    divert_info = &c2d_divert_info->config[divert_info_config_table_idx];
  }
  return divert_info;
}

/* c2d_module_set_divert_cfg_identity:
 *
 *  search for key_identity and set new_identity
 **/
int32_t c2d_module_set_divert_cfg_identity(uint32_t key_identity,
  uint32_t new_identity, c2d_divert_info_t *c2d_divert_info)
{
  if(!c2d_divert_info) {
    CDBG_ERROR("%s:%d, failed, c2d_divert_info=%p\n", __func__, __LINE__,
      c2d_divert_info);
    return -EINVAL;
  }
  int i = 0;
  for (i = 0; i < C2D_MAX_STREAMS_PER_PORT; i++) {
    if (c2d_divert_info->identity[i] == key_identity) {
      c2d_divert_info->identity[i] = new_identity;
      return 0;
    }
  }
  CDBG_ERROR("%s:%d] failed to set identity\n", __func__, __LINE__);
  return -EFAULT;
}

/* c2d_module_set_divert_cfg_entry:
 *
 *  search for key_identity and set new_identity
 **/
int32_t c2d_module_set_divert_cfg_entry(uint32_t identity,
  pproc_cfg_update_t update_mode, pproc_divert_info_t *divert_info,
  c2d_divert_info_t *c2d_divert_info)
{
  if(!c2d_divert_info ||
    (identity == PPROC_INVALID_IDENTITY)) {
    CDBG_ERROR("%s:%d, failed, c2d_divert_info=%p\n", __func__, __LINE__,
      c2d_divert_info);
    return -EINVAL;
  }

  int i = 0;
  if (update_mode == PPROC_CFG_UPDATE_DUAL) {
    c2d_divert_info->config[3] = *divert_info;
    return 0;
  } else {
    for (i = 0; i < C2D_MAX_STREAMS_PER_PORT; i++) {
      if (c2d_divert_info->identity[i] == identity) {
        c2d_divert_info->config[(1 << i)] = *divert_info;
        return 0;
      }
    }
  }
  CDBG_ERROR("%s:%d] failed set divert info\n", __func__, __LINE__);
  return -EFAULT;
}
