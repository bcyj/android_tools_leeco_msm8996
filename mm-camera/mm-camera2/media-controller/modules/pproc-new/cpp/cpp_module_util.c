/*============================================================================

  Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "eztune_diagnostics.h"
#include "cpp_port.h"
#include "cpp_module.h"
#include "camera_dbg.h"
#include "cpp_log.h"

static boolean find_port_with_identity_find_func(void *data, void *user_data)
{
  if(!data || !user_data) {
    CDBG_ERROR("%s:%d: failed, data=%p, user_data=%p\n",
                __func__, __LINE__, data, user_data);
    return FALSE;
  }
  mct_port_t *port = (mct_port_t*) data;
  uint32_t identity = *(uint32_t*) user_data;

  cpp_port_data_t *port_data = (cpp_port_data_t *) MCT_OBJECT_PRIVATE(port);
  int i;
  for(i=0; i<CPP_MAX_STREAMS_PER_PORT; i++) {
    if(port_data->stream_data[i].port_state != CPP_PORT_STATE_UNRESERVED &&
        port_data->stream_data[i].identity == identity) {
      return TRUE;
    }
  }
  return FALSE;
}

/** cpp_module_util_check_stream
 *    @d1: mct_stream_t* pointer to the streanm being checked
 *    @d2: uint32_t* pointer to identity
 *
 *  Check if the stream matches stream index or stream type.
 *
 *  Return: TRUE if stream matches.
 **/
static boolean cpp_module_util_check_stream(void *d1, void *d2)
{
  boolean ret_val = FALSE;
  mct_stream_t *stream = (mct_stream_t *)d1;
  uint32_t *id = (uint32_t *)d2;

  if (stream && id && stream->streaminfo.identity == *id)
    ret_val = TRUE;

  return ret_val;
}

mct_port_t* cpp_module_find_port_with_identity(mct_module_t *module,
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
  cpp_module_ack_t* cpp_ack = (cpp_module_ack_t*) data;
  cpp_module_ack_key_t* key = (cpp_module_ack_key_t*) userdata;
  if(cpp_ack->isp_buf_divert_ack.identity == key->identity &&
     cpp_ack->isp_buf_divert_ack.buf_idx == key->buf_idx) {
    return TRUE;
  }
  return FALSE;
}

cpp_module_ack_t* cpp_module_find_ack_from_list(cpp_module_ctrl_t *ctrl,
  cpp_module_ack_key_t key)
{
  mct_list_t *templist;
  templist = mct_list_find_custom(ctrl->ack_list.list, &key, ack_find_func);
  if(templist) {
    return (cpp_module_ack_t*)(templist->data);
  }
  return NULL;
}

static
boolean clk_rate_find_by_identity_func(void* data, void* userdata)
{
  if(!data || !userdata) {
    CDBG_ERROR("%s:%d: failed, data=%p, userdata=%p\n",
                __func__, __LINE__, data, userdata);
    return FALSE;
  }

  cpp_module_stream_clk_rate_t *clk_rate_obj =
    (cpp_module_stream_clk_rate_t*) data;
  uint32_t identity = *(uint32_t*)userdata;

  if(clk_rate_obj->identity == identity) {
    return TRUE;
  }
  return FALSE;
}

cpp_module_stream_clk_rate_t *
cpp_module_find_clk_rate_by_identity(cpp_module_ctrl_t *ctrl,
  uint32_t identity)
{
  mct_list_t *templist;

  templist = mct_list_find_custom(ctrl->clk_rate_list.list, &identity,
    clk_rate_find_by_identity_func);
  if(templist) {
    return (cpp_module_stream_clk_rate_t *)(templist->data);
  }
  return NULL;
}

static
boolean clk_rate_find_by_value_func(void* data, void** userdata)
{
  if(!data) {
    CDBG_ERROR("%s:%d: failed, data=%p\n",
                __func__, __LINE__, data);
    return FALSE;
  }
  cpp_module_stream_clk_rate_t *curent_clk_obj =
    (cpp_module_stream_clk_rate_t *) data;
  uint64_t  *total_load = *(uint64_t *)userdata;

  *total_load += curent_clk_obj->total_load;

  return TRUE;
}

int64_t cpp_module_get_total_load_by_value(cpp_module_ctrl_t *ctrl)
{
  int32_t rc;
  uint64_t total_load = 0;
  uint64_t *ptr_total_load = NULL;

  ptr_total_load = &total_load;

  rc = mct_list_traverse(ctrl->clk_rate_list.list,
    clk_rate_find_by_value_func, &ptr_total_load);

  if (rc < 0) {
    return rc;
  }

  return total_load;
}

cam_streaming_mode_t cpp_module_get_streaming_mode(mct_module_t *module,
  uint32_t identity)
{
  if (!module) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  mct_port_t* port = cpp_module_find_port_with_identity(module, MCT_PORT_SINK,
                       identity);
  if (!port) {
    CDBG_ERROR("%s:%d port not found, identity=0x%x\n",
      __func__, __LINE__, identity);
    return -EINVAL;
  }
  cpp_port_data_t* port_data = (cpp_port_data_t*) MCT_OBJECT_PRIVATE(port);
  int i;
  for (i=0; i<CPP_MAX_STREAMS_PER_PORT; i++) {
    if (port_data->stream_data[i].identity == identity) {
      return port_data->stream_data[i].streaming_mode;
    }
  }
  return CAM_STREAMING_MODE_MAX;
}

int32_t cpp_module_get_params_for_identity(cpp_module_ctrl_t* ctrl,
  uint32_t identity, cpp_module_session_params_t** session_params,
  cpp_module_stream_params_t** stream_params)
{
  if(!ctrl || !session_params || !stream_params) {
    CDBG("%s:%d: failed, ctrl=%p, session_params=%p, stream_params=%p",
      __func__, __LINE__, ctrl, session_params, stream_params);
    return -EINVAL;
  }
  uint32_t session_id;
  int i,j;
  boolean success = FALSE;
  session_id = CPP_GET_SESSION_ID(identity);
  for(i=0; i < CPP_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i]) {
      if(ctrl->session_params[i]->session_id == session_id) {
        for(j=0; j < CPP_MODULE_MAX_STREAMS; j++) {
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

void cpp_module_dump_stream_params(cpp_module_stream_params_t *stream_params,
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

boolean cpp_module_util_map_buffer_info(void *d1, void *d2)
{
  mct_stream_map_buf_t          *img_buf = (mct_stream_map_buf_t *)d1;
  cpp_module_stream_buff_info_t *stream_buff_info =
    (cpp_module_stream_buff_info_t *)d2;
  cpp_module_buffer_info_t      *buffer_info;
  mct_list_t                    *list_entry = NULL;

  if (!img_buf || !stream_buff_info) {
    CDBG_ERROR("%s:%d, failed. img_buf=%p stream_buff_info=%p\n", __func__,
      __LINE__, img_buf, stream_buff_info);
    return FALSE;
  }

  buffer_info = malloc(sizeof(cpp_module_buffer_info_t));
  if (NULL == buffer_info) {
    CDBG_ERROR("%s:%d, malloc() failed\n", __func__, __LINE__);
    return FALSE;
  }

  memset((void *)buffer_info, 0, sizeof(cpp_module_buffer_info_t));

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

boolean cpp_module_util_free_buffer_info(void *d1, void *d2)
{
  cpp_module_buffer_info_t      *buffer_info =
    (cpp_module_buffer_info_t *)d1;
  cpp_module_stream_buff_info_t *stream_buff_info =
    (cpp_module_stream_buff_info_t *)d2;

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

boolean cpp_module_util_create_hw_stream_buff(void *d1, void *d2)
{
  cpp_module_buffer_info_t        *buffer_info =
    (cpp_module_buffer_info_t *)d1;
  cpp_hardware_stream_buff_info_t *hw_strm_buff_info =
    (cpp_hardware_stream_buff_info_t *)d2;
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

/* cpp_module_invalidate_q_traverse_func:
 *
 * Invalidates queue entry and adds ack_key in key_list base on identity.
 *
 **/
boolean cpp_module_invalidate_q_traverse_func(void* qdata, void* userdata)
{
  if (!qdata || !userdata) {
    CDBG_ERROR("%s:%d, failed, qdata=%p input=%p\n", __func__, __LINE__,
      qdata, userdata);
    return FALSE;
  }
  void** input = (void**)userdata;
  cpp_module_event_t* cpp_event = (cpp_module_event_t *) qdata;
  cpp_module_ctrl_t*  ctrl = (cpp_module_ctrl_t *) input[0];
  uint32_t identity = *(uint32_t*) input[1];
  mct_list_t **key_list = (mct_list_t **) input[2];
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return FALSE;
  }
  /* invalidate the event and add key in key list */
  if(cpp_event->ack_key.identity == identity) {
    cpp_event->invalid = TRUE;
    cpp_module_ack_key_t *key =
      (cpp_module_ack_key_t *) malloc (sizeof(cpp_module_ack_key_t));
    if(!key) {
      CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
      return FALSE;
    }
    memcpy(key, &(cpp_event->ack_key), sizeof(cpp_module_ack_key_t));
    *key_list = mct_list_append(*key_list, key, NULL, NULL);
  }
  return TRUE;
}


/* cpp_module_release_ack_traverse_func:
 *
 * traverses through the list of keys and updates ACK corresponding to the
 * key.
 *
 **/
boolean cpp_module_release_ack_traverse_func(void* data, void* userdata)
{
  int32_t rc;
  if (!data || !userdata) {
    CDBG_ERROR("%s:%d, failed, data=%p userdata=%p\n", __func__, __LINE__,
      data, userdata);
    return FALSE;
  }
  cpp_module_ack_key_t* key = (cpp_module_ack_key_t *) data;
  cpp_module_ctrl_t*  ctrl = (cpp_module_ctrl_t *) userdata;
  rc = cpp_module_do_ack(ctrl, *key);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, identity=0x%x\n", __func__, __LINE__,
      key->identity);
      return FALSE;
  }
  return TRUE;
}

/* cpp_module_key_list_free_traverse_func:
 *
 * traverses through the list of keys and frees the data.
 *
 **/
boolean cpp_module_key_list_free_traverse_func(void* data, void* userdata)
{
  cpp_module_ack_key_t* key = (cpp_module_ack_key_t *) data;
  free(key);
  return TRUE;
}

/** cpp_module_update_hfr_skip:
 *
 *  Description:
 *    Based on input and output fps, calculte the skip count
 *    according to this formula,
 *      count = floor(input/output) - 1, if input > output
 *            = 0, otherwise
 *
 **/
int32_t cpp_module_update_hfr_skip(cpp_module_stream_params_t *stream_params)
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


/** cpp_module_set_output_duplication_flag:
 *
 *  Description:
 *    Based on stream's dimension info and existance of a linked
 *    stream, decide if output-duplication feature of cpp
 *    hardware can be utilized.
 *
 **/
int32_t cpp_module_set_output_duplication_flag(
  cpp_module_stream_params_t *stream_params)
{
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EINVAL;
  }
  stream_params->hw_params.duplicate_output = FALSE;
  stream_params->hw_params.duplicate_identity = 0x00;

  CDBG(
    "%s:%d, current stream w=%d, h=%d, st=%d, sc=%d, fmt=%d, identity=0x%x",
    __func__, __LINE__, stream_params->hw_params.output_info.width,
    stream_params->hw_params.output_info.height,
    stream_params->hw_params.output_info.stride,
    stream_params->hw_params.output_info.scanline,
    stream_params->hw_params.output_info.plane_fmt,
    stream_params->identity);

  /* if there is no linked stream, no need for duplication */
  if(!stream_params->linked_stream_params) {
    CDBG("%s:%d, info: no linked stream", __func__, __LINE__);
    return 0;
  }
  CDBG(
    "%s:%d, linked stream w=%d, h=%d, st=%d, sc=%d, fmt=%d, identity=0x%x",
    __func__, __LINE__,
    stream_params->linked_stream_params->hw_params.output_info.width,
    stream_params->linked_stream_params->hw_params.output_info.height,
    stream_params->linked_stream_params->hw_params.output_info.stride,
    stream_params->linked_stream_params->hw_params.output_info.scanline,
    stream_params->linked_stream_params->hw_params.output_info.plane_fmt,
    stream_params->linked_stream_params->identity);

#if CPP_OUTPUT_DUPLICATION_EN
  if(stream_params->hw_params.output_info.width ==
     stream_params->linked_stream_params->hw_params.output_info.width &&
     stream_params->hw_params.output_info.height ==
     stream_params->linked_stream_params->hw_params.output_info.height &&
     stream_params->hw_params.output_info.stride ==
     stream_params->linked_stream_params->hw_params.output_info.stride &&
     stream_params->hw_params.output_info.scanline ==
     stream_params->linked_stream_params->hw_params.output_info.scanline &&
     stream_params->hw_params.output_info.plane_fmt ==
     stream_params->linked_stream_params->hw_params.output_info.plane_fmt &&
     stream_params->hw_params.mirror ==
     stream_params->linked_stream_params->hw_params.mirror &&
     stream_params->hw_params.rotation ==
     stream_params->linked_stream_params->hw_params.rotation) {
    /* make the linked streams duplicates of each other */
    CDBG(
      "%s:%d,linked streams formats and flip match: output duplication enabled",
      __func__, __LINE__);
    stream_params->hw_params.duplicate_output = TRUE;
    stream_params->hw_params.duplicate_identity =
      stream_params->linked_stream_params->identity;
    stream_params->linked_stream_params->hw_params.duplicate_output = TRUE;
    stream_params->linked_stream_params->hw_params.duplicate_identity =
      stream_params->identity;
  }
#endif
  return 0;
}
/** cpp_module_get_divert_info:
 *
 *  Description:
 *    Based on streamon state of "this" stream and "linked"
 *      stream fetch the divert configuration sent by pproc
 *      module or otherwise return NULL
 *
 **/
pproc_divert_info_t *cpp_module_get_divert_info(uint32_t *identity_list,
  uint32_t identity_list_size, cpp_divert_info_t *cpp_divert_info)
{
  uint32_t i = 0, j = 0;
  uint8_t identity_mapped_idx = 0;
  uint8_t divert_info_config_table_idx = 0;
  pproc_divert_info_t *divert_info = NULL;

  /* Loop through the identity list to determine the corresponding index
     in the cpp_divert_info */
  for (i = 0; i < identity_list_size; i++) {
    if (identity_list[i] != PPROC_INVALID_IDENTITY) {
      /* Search the requested identity from the cpp_divert_info table */
      identity_mapped_idx = 0;
      for (j = 0; j < CPP_MAX_STREAMS_PER_PORT; j++) {
        if (cpp_divert_info->identity[j] == identity_list[i]) {
          identity_mapped_idx = j;
          break;
        }
      }
      if (j < CPP_MAX_STREAMS_PER_PORT) {
        divert_info_config_table_idx |= (1 << identity_mapped_idx);
      }
    }
  }

  if(divert_info_config_table_idx) {
    divert_info = &cpp_divert_info->config[divert_info_config_table_idx];
  }
  return divert_info;
}

/* cpp_module_set_divert_cfg_identity:
 *
 *  search for key_identity and set new_identity
 **/
int32_t cpp_module_set_divert_cfg_identity(uint32_t key_identity,
  uint32_t new_identity, cpp_divert_info_t *cpp_divert_info)
{
  if(!cpp_divert_info) {
    CDBG_ERROR("%s:%d, failed, cpp_divert_info=%p\n", __func__, __LINE__,
      cpp_divert_info);
    return -EINVAL;
  }
  int i = 0;
  for (i = 0; i < CPP_MAX_STREAMS_PER_PORT; i++) {
    if (cpp_divert_info->identity[i] == key_identity) {
      cpp_divert_info->identity[i] = new_identity;
      return 0;
    }
  }
  CDBG_ERROR("%s:%d] failed to set identity\n", __func__, __LINE__);
  return -EFAULT;
}

/* cpp_module_set_divert_cfg_entry:
 *
 *  search for key_identity and set new_identity
 **/
int32_t cpp_module_set_divert_cfg_entry(uint32_t identity,
  pproc_cfg_update_t update_mode, pproc_divert_info_t *divert_info,
  cpp_divert_info_t *cpp_divert_info)
{
  if(!cpp_divert_info ||
    (identity == PPROC_INVALID_IDENTITY)) {
    CDBG_ERROR("%s:%d, failed, cpp_divert_info=%p\n", __func__, __LINE__,
      cpp_divert_info);
    return -EINVAL;
  }

  int i = 0;
  if (update_mode == PPROC_CFG_UPDATE_DUAL) {
    cpp_divert_info->config[3] = *divert_info;
    return 0;
  } else {
    for (i = 0; i < CPP_MAX_STREAMS_PER_PORT; i++) {
      if (cpp_divert_info->identity[i] == identity) {
        cpp_divert_info->config[(1 << i)] = *divert_info;
        return 0;
      }
    }
  }
  CDBG_ERROR("%s:%d] failed set divert info\n", __func__, __LINE__);
  return -EFAULT;
}

int32_t cpp_module_util_post_diag_to_bus(mct_module_t *module,
  ez_pp_params_t *cpp_params, uint32_t identity)
{
  mct_bus_msg_t bus_msg_cpp_diag;
  mct_event_t event;

  bus_msg_cpp_diag.type = MCT_BUS_MSG_PP_CHROMATIX_LITE;
  bus_msg_cpp_diag.size = sizeof(ez_pp_params_t);
  bus_msg_cpp_diag.msg = (void *)cpp_params;
  bus_msg_cpp_diag.sessionid = (identity & 0xFFFF0000) >> 16;

  /* CPP being a sub-module inside pproc it cannot directly access mct */
  /* Create an event so that PPROC can post it to MCT */
  event.identity = identity;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_UPSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_PP_SUBMOD_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)&bus_msg_cpp_diag;

  if (TRUE != cpp_module_send_event_upstream(module, &event)) {
    CDBG_ERROR("%s%d] error posting diag to bus\n", __func__, __LINE__);
  }
  return 0;
}

int32_t cpp_module_util_update_session_diag_params(mct_module_t *module,
  cpp_hardware_params_t* hw_params)
{
  cpp_module_stream_params_t *stream_params = NULL;
  cpp_module_session_params_t *session_params = NULL;
  cpp_module_ctrl_t *ctrl;

  /* Check whether the current stream type needs update diag params */
   if ((hw_params->stream_type != CAM_STREAM_TYPE_OFFLINE_PROC) &&
     (hw_params->stream_type != CAM_STREAM_TYPE_SNAPSHOT) &&
     (hw_params->stream_type != CAM_STREAM_TYPE_PREVIEW)){
     return 0;
   }

  ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  /* Pick up session params and update diag params */
  cpp_module_get_params_for_identity(ctrl, hw_params->identity,
    &session_params, &stream_params);
  if(!session_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  if ((hw_params->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) ||
    (hw_params->stream_type == CAM_STREAM_TYPE_SNAPSHOT)){
    memcpy(&session_params->diag_params.snap_asf7x7, &hw_params->asf_diag,
      sizeof(asfsharpness7x7_t));
    memcpy(&session_params->diag_params.snap_wnr, &hw_params->wnr_diag,
      sizeof(wavelet_t));
  } else if (hw_params->stream_type == CAM_STREAM_TYPE_PREVIEW) {
    memcpy(&session_params->diag_params.prev_asf7x7, &hw_params->asf_diag,
      sizeof(asfsharpness7x7_t));
    memcpy(&session_params->diag_params.prev_wnr, &hw_params->wnr_diag,
      sizeof(wavelet_t));
  }

  if (hw_params->diagnostic_enable) {
    /* Post the updated diag params to bus if diagnostics is enabled */
    cpp_module_util_post_diag_to_bus(module, &session_params->diag_params,
      hw_params->identity);
  }
  return 0;
}

/** cpp_module_util_find_parent
 *    @identity: required identity
 *    @module: module, whichs parents will be serached
 *
 * Finds module parent (stream) with specified identity
 *
 * Returns Pointer to stream handler in case of cucess
 *   or NULL in case of failure
 **/
mct_stream_t* cpp_module_util_find_parent(uint32_t identity,
  mct_module_t* module)
{
  mct_stream_t* ret_val = NULL;
  mct_list_t *find_list;

  if (module && MCT_MODULE_PARENT(module)) {
    find_list = mct_list_find_custom(MCT_MODULE_PARENT(module),
      &identity, cpp_module_util_check_stream);

    if (find_list)
      ret_val = find_list->data;
  }

  return ret_val;
}
