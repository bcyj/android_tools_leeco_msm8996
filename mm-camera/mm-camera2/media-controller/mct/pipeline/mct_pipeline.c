/* mct_pipeline.c
 *
 * Copyright (c) 2014-2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "cam_intf.h"
#include "mct_controller.h"
#include "camera_dbg.h"
#include <linux/videodev2.h>
#include <media/msmb_camera.h>
#include <math.h>
#include <sys/sysinfo.h>
#include <sys/syscall.h>
#include <cutils/properties.h>
#include <sys/prctl.h>
#include "server_debug.h"

/* Forward declare tuning APIs*/
extern void mct_start_tuning_server (mct_pipeline_t *pipeline);
extern void mct_stop_tuning_server(mct_pipeline_t *pipeline);
extern void mct_tuning_notify_snapshot_frame(void *params);
extern void mct_notify_reload_tuning(void);


#if 0
#undef CDBG
#define CDBG CDBG_ERROR
#endif

#ifndef MIN
#define MIN(x,y) (((x)<(y)) ? (x) : (y))
#endif

#ifndef F_EQUAL
#define F_EQUAL(a, b) (fabs(a-b) < 1e-4)
#endif

static cam_dimension_t default_preview_sizes[] = {
  { 4096, 2160},// true 4K
  { 3840, 2160},// 4K
  { 1920, 1080}, //1080p
  { 1280, 960},
  { 1280, 720},  // 720P, reserved
  { 864, 480}, //FWVGA
  { 800, 480},   //  WVGA
  { 768, 432},
  { 720, 480},
  { 640, 480},   // VGA
  { 576, 432},
  { 480, 360},   // HVGA
  { 384, 288},
  { 352, 288},   // CIF
  { 320, 240},   // QVGA
  { 240, 160},   // SQVGA
  { 176, 144},   // QCIF
  { 160, 120},
  { 144, 176}    // QCIF for CSVT app
};

static cam_dimension_t default_picture_sizes[] = {
  { 5248, 3936}, // 20MP
  { 4608, 3456}, // 16MP
  { 4160, 3120}, // 13MP
  { 4000, 3000}, // 12MP
  { 4096, 2160},// true 4K
  { 3264, 2448}, // 8MP
  { 2592, 1944}, // 5MP
  { 2048, 1536}, // 3MP QXGA
  { 1920, 1080}, // HD1080
  { 1600, 1200}, // 2MP UXGA
  { 1280, 960},
  { 1280, 768},  // WXGA
  { 1280, 720},  // HD720
  { 1024, 768},  // 1MP XGA
  { 800, 600},   // SVGA
  { 800, 480},   // WVGA
  { 720, 480},   // 480p
  { 640, 480},   // VGA
  { 352, 288},   // CIF
  { 320, 240},   // QVGA
  { 176, 144},    // QCIF
  { 160, 120}
};

static cam_dimension_t default_liveshot_sizes[] = {
  { 4128, 3096}, //4:3
  { 4128, 2322}, //16:9
  { 4000, 3000}, // 12MP
  { 3264, 2448}, // 8MP
  { 2592, 1944}, // 5MP
  { 2048, 1536}, // 3MP QXGA
  { 1920, 1080}, // HD1080
  { 1600, 1200}, // 2MP UXGA
  { 1280, 960},
  { 1280, 768},  // WXGA
  { 1280, 720},  // HD720
  { 1024, 768},  // 1MP XGA
  { 800, 600},   // SVGA
  { 864, 480},   //FWVGA
  { 800, 480},   // WVGA
  { 720, 480},   // 480p
  { 640, 480},   // VGA
  { 352, 288},   // CIF
  { 320, 240},   // QVGA
  { 176, 144},    // QCIF
  { 160, 120}
};

static  cam_dimension_t default_video_sizes[] = {
  { 4096, 2160},// true 4K
  { 3840, 2160},// 4K
  { 1920, 1080},// 1080p
  { 1280, 960},
  { 1280, 720}, // 720p
  { 864, 480}, //FWVGA
  { 800, 480},  // WVGA
  { 720, 480},  // 480p
  { 640, 480},  // VGA
  { 480, 360},  // HVGA
  { 352, 288},  // CIF
  { 320, 240},  // QVGA
  { 176, 144},  // QCIF
  { 160, 120}
};

static  cam_fps_range_t default_fps_ranges[] = {
  { 15.0, 15.0, 15.0, 15.0},
  { 24.0, 24.0, 24.0, 24.0},
  { 30.0, 30.0, 30.0, 30.0},
};

/** mct_pipeline_check_stream_t
 *    @
 *    @
 *
 **/
typedef enum _mct_pipeline_check_stream {
  CHECK_INDEX,
  CHECK_TYPE,
  CHECK_SESSION
} mct_pipeline_check_stream_t;

/** mct_pipeline_get_stream_info_t
 *    @
 *    @
 **/
typedef struct _mct_pipeline_get_stream_info {
  mct_pipeline_check_stream_t check_type;
  unsigned int                stream_index;
  cam_stream_type_t           stream_type;
  unsigned int                session_index;
} mct_pipeline_get_stream_info_t;

/** mct_pipeline_check_stream
 *    @d1: mct_stream_t* pointer to the streanm being checked
 *    @d2: mct_pipeline_get_stream_info_t* pointer info to check against
 *
 *  Check if the stream matches stream index or stream type.
 *
 *  Return: TRUE if stream matches.
 **/
static boolean mct_pipeline_check_stream(void *d1, void *d2)
{
  mct_stream_t                   *stream = (mct_stream_t *)d1;
  mct_pipeline_get_stream_info_t *info   =
    (mct_pipeline_get_stream_info_t *)d2;

  if (info->check_type == CHECK_INDEX) {
    return (stream->streamid
      == info->stream_index ? TRUE : FALSE);
  } else if (info->check_type == CHECK_TYPE) {
    if (MCT_STREAM_STREAMINFO(stream))
      return ((cam_stream_info_t *)(MCT_STREAM_STREAMINFO(stream)))->stream_type
        == info->stream_type ? TRUE : FALSE;
  } else if (info->check_type == CHECK_SESSION) {
    return (((stream->streaminfo.identity & 0xFFFF0000) >> 16)
      == (unsigned int)info->session_index ? TRUE : FALSE);
  }

  return FALSE;
}

/** mct_pipeline_get_stream
 *    @pipeline: mct_pipeline_t object
 *    @get_info: stream information to match
 *
 *  Retrieve the stream which matches stream information in get_info.
 *
 *  Reture stream object if the stream is found.
 **/
static mct_stream_t* mct_pipeline_get_stream(mct_pipeline_t *pipeline,
  mct_pipeline_get_stream_info_t *get_info)
{
  mct_list_t *find_list = NULL;

  if (!MCT_PIPELINE_CHILDREN(pipeline)) {
    CDBG("%s: no children", __func__);
    return NULL;
  }

  MCT_OBJECT_LOCK(pipeline);
  find_list = mct_list_find_custom(MCT_PIPELINE_CHILDREN(pipeline),
    get_info, mct_pipeline_check_stream);
  MCT_OBJECT_UNLOCK(pipeline);

  if (!find_list) {
    CDBG("%s: stream not found in the list", __func__);
    return NULL;
  }

  return MCT_STREAM_CAST(find_list->data);
}

/** mct_pipeline_find_stream:
 *    @module:
 *    @identity:
 *
 **/
mct_stream_t* mct_pipeline_find_stream
  (mct_module_t *module, unsigned int session_id)
{
  mct_list_t     *find_list = NULL;
  mct_pipeline_get_stream_info_t info;

  info.check_type    = CHECK_SESSION;
  info.session_index = session_id;

  find_list = mct_list_find_custom(MCT_MODULE_PARENT(module),
    (void *)&info, mct_pipeline_check_stream);

  if (!find_list)
    return NULL;

  return MCT_STREAM_CAST(find_list->data);;
}

/** mct_pipeline_find_stream_from_stream_id:
 *    @module:
 *    @identity:
 *
 **/
mct_stream_t* mct_pipeline_find_stream_from_stream_id
  (mct_pipeline_t *pipeline, uint32_t stream_id)
{
  mct_stream_t     *stream = NULL;
  mct_pipeline_get_stream_info_t info;

  info.check_type    = CHECK_INDEX;
  info.stream_index  = stream_id;

  stream = mct_pipeline_get_stream(pipeline, &info);
  if (!stream) {
    CDBG_ERROR("%s: Couldn't find stream\n", __func__);
    return NULL;
  }

  return stream;
}

/** mct_pipeline_find_buf:
 *    @stream: mct_stream_t object which to receive the event
 *    @event:  mct_event_t object to send to this stream
 *
 *  Used for matching metadata buffer in to stream list.
 *
 *  Return TRUE on success
 **/
static boolean mct_pipeline_find_buf(void *data, void *user_data)
{

  mct_stream_map_buf_t *map_buf = data;
  uint32_t *buf_index = user_data;

  if (CAM_MAPPING_BUF_TYPE_STREAM_BUF != map_buf->buf_type) {
    return FALSE;
  }

  return (map_buf->buf_index == *buf_index);
}

void *mct_pipeline_get_buffer_ptr(mct_pipeline_t *pipeline, uint32_t buf_idx,
  uint32_t stream_id)
{
  mct_stream_map_buf_t *current_buf;

  current_buf = mct_pipeline_get_buffer(pipeline, buf_idx, stream_id);

  return current_buf ? current_buf->buf_planes[0].buf : NULL;
}

/** mct_pipeline_get_buffer:
 *
 *  Used for matching frame buffer in to stream list.
 **/
mct_stream_map_buf_t *mct_pipeline_get_buffer(mct_pipeline_t *pipeline,
  uint32_t buf_idx, uint32_t stream_id)
{
  mct_list_t *current_buf_holder;
  mct_stream_map_buf_t *current_buf;
  mct_stream_t *stream = NULL;
  mct_pipeline_get_stream_info_t info;

  info.check_type   = CHECK_INDEX;
  info.stream_index = stream_id;
  stream = mct_pipeline_get_stream(pipeline, &info);
  if (!stream) {
    CDBG_ERROR("%s: Couldn't find stream\n", __func__);
    return NULL;
  }
  current_buf_holder = mct_list_find_custom(
    stream->buffers.img_buf,
    &buf_idx, mct_pipeline_find_buf);
  if (!current_buf_holder) {
    CDBG_ERROR("%s:current_buf_holder is null", __func__);
    return NULL;
  }

  current_buf = current_buf_holder->data;
  return current_buf;
}

/** mct_pipeline_query_modules:
 *    @
 *    @
 *
 **/
static boolean mct_pipeline_query_modules(void *data, void *user_data)
{
  mct_pipeline_t *pipeline = (mct_pipeline_t *)user_data;
  mct_module_t *module   = (mct_module_t *)data;

  if (!pipeline || !module)
    return FALSE;
  if (module->query_mod) {
    module->query_mod(module, &pipeline->query_data, pipeline->session);
  }

  return TRUE;
}

/** mct_pipeline_delete_stream:
 *    @
 *    @
 *
 **/
static boolean mct_pipeline_delete_stream(void *data, void *user_data)
{
  mct_stream_t *stream = (mct_stream_t *)data;

  if (!stream)
    return FALSE;

  mct_stream_destroy(stream);

  return TRUE;
}

/** mct_pipeline_add_stream
 *
 *    @ pipeline: the current pipeline; equivalent to "this"
 *    @ streamid: the stream id for the stream to be added
 *
 *  Return: TRUE on success, FALSE on failure
 *
 *  This function creates a new stream with the stream id
 *  passed in and adds it to the pipleine's list of children.
 *
 **/
static boolean mct_pipeline_add_stream(mct_pipeline_t *pipeline,
  uint32_t stream_id)
{
  mct_stream_t *stream;
  if (!pipeline)
    return FALSE;
  uint32_t session_id = MCT_PIPELINE_SESSION(pipeline);

  stream = mct_stream_new(stream_id);
  if (!stream)
    goto stream_failed;

  /* set pipeline as this stream's parent */
  if (!mct_object_set_parent(MCT_OBJECT_CAST(stream),
        MCT_OBJECT_CAST(pipeline)))
    goto set_parent_failed;

  return TRUE;

set_parent_failed:
  mct_stream_destroy(stream);
stream_failed:
  return FALSE;
}

/** mct_pipeline_remove_stream
 *
 *    @ pipeline: the current pipeline; equivalent to "this"
 *    @ stream: the stream to be removed
 *
 *  Return: TRUE on success, FALSE on failure
 *
 *  This function destroys the stream matching the stream id
 *  passed in and removes it from the pipleine's list of children.
 *
 **/
static boolean mct_pipeline_remove_stream(mct_pipeline_t *pipeline,
  mct_stream_t *stream)
{
  if (!pipeline)
    return FALSE;

  if (MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data) != pipeline)
    goto error;

  mct_stream_destroy(stream);

  return TRUE;

error:
  return FALSE;
}

/** mct_pipeline_send_event
 *    @pipeline:
 *    @stream_id:
 *    @event:
 *
 *  Return: TRUE on success, FALSE on failure
 **/
static boolean mct_pipeline_send_event(mct_pipeline_t *pipeline,
  uint32_t stream_id, mct_event_t *event)
{
  boolean ret = TRUE;
  mct_stream_t *stream = NULL;
  mct_pipeline_get_stream_info_t info;

  if (!pipeline || !event)
    return FALSE;

  info.check_type   = CHECK_INDEX;
  info.stream_index = stream_id;

  stream = mct_pipeline_get_stream(pipeline, &info);
  if (!stream) {
    CDBG_ERROR("%s: Couldn't find stream\n", __func__);
    return FALSE;
  }

  ret = stream->send_event(stream, event);
  return ret;
}

/** mct_pipeline_set_bus:
 *    @
 *    @
 *
 **/
boolean mct_pipeline_set_bus(mct_pipeline_t *pipeline, mct_bus_t *bus)
{
  if (!pipeline)
    return FALSE;
  /* TODO */
  return TRUE;
}

/** mct_pipeline_get_bus:
 *    @pipeline:
 *
 **/
static mct_bus_t* mct_pipeline_get_bus(mct_pipeline_t *pipeline)
{
  mct_bus_t *result = NULL;

  if (!pipeline)
    return NULL;
  /* TODO */
  return pipeline->bus;
}

/** mct_pipeline_map_buf:
 *    @
 *    @
 *
 **/
static boolean mct_pipeline_map_buf(void *message, mct_pipeline_t *pipeline)
{
  boolean ret = FALSE;
  mct_serv_ds_msg_t *msg = (mct_serv_ds_msg_t *)message;
  mct_stream_t  *stream;

  if (!msg || !pipeline ||
      (msg->session != MCT_PIPELINE_SESSION(pipeline))) {
    ret = FALSE;
    goto finish;
  }

  switch (msg->buf_type) {
  case CAM_MAPPING_BUF_TYPE_PARM_BUF: {
    pipeline->config_parm = mmap(NULL, msg->size, PROT_READ | PROT_WRITE,
      MAP_SHARED, msg->fd, 0);

    if (pipeline->config_parm == MAP_FAILED) {
      ret = FALSE;
      break;
    }

    pipeline->config_parm_size = msg->size;
    pipeline->config_parm_fd   = msg->fd;
    ret = TRUE;
  }
    break;

  case CAM_MAPPING_BUF_TYPE_CAPABILITY: {
    pipeline->query_buf = mmap(NULL, msg->size, PROT_READ | PROT_WRITE,
      MAP_SHARED, msg->fd, 0);

    if (pipeline->query_buf == MAP_FAILED) {
      ret = FALSE;
      break;
    }

    pipeline->query_buf_size = msg->size;
    pipeline->query_buf_fd   = msg->fd;
    ret = TRUE;
  }
    break;

  /* Below messages are per Stream */
  case CAM_MAPPING_BUF_TYPE_OFFLINE_META_BUF:
  case CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF:
  case CAM_MAPPING_BUF_TYPE_STREAM_INFO:
  case CAM_MAPPING_BUF_TYPE_STREAM_BUF: {
    mct_pipeline_get_stream_info_t get_info;
    get_info.check_type   = CHECK_INDEX;
    get_info.stream_index = msg->stream;

    if ((stream = mct_pipeline_get_stream(pipeline, &get_info))) {
      if (stream->map_buf)
        ret = stream->map_buf(msg, stream);
    } else
     ret = FALSE;
  }
    break;

  default:
    break;
  } /* switch (msg->buf_type) */

finish:
  return ret;
}

/** mct_pipeline_unmap_buf:
 *    @
 *    @
 *
 **/
boolean mct_pipeline_unmap_buf(void *message, mct_pipeline_t *pipeline)
{
  boolean ret = FALSE;
  int32_t rc;
  mct_serv_ds_msg_t *msg = (mct_serv_ds_msg_t *)message;
  mct_stream_t  *stream;

  if (!msg || !pipeline ||
      (msg->session != MCT_PIPELINE_SESSION(pipeline))) {
    ret = FALSE;
    goto finish;
  }

  switch (msg->buf_type) {
  case CAM_MAPPING_BUF_TYPE_PARM_BUF: {
    rc = munmap(pipeline->config_parm, pipeline->config_parm_size);
    if (rc < 0) {
      ret = FALSE;
      break;
    }

    pipeline->config_parm = NULL;
    pipeline->config_parm_size = 0;
    close(pipeline->config_parm_fd);
    ret = TRUE;
  }
    break;


  case CAM_MAPPING_BUF_TYPE_CAPABILITY: {
    rc = munmap(pipeline->query_buf, pipeline->query_buf_size);
    if (rc < 0) {
      ret = FALSE;
      break;
    }
    pipeline->query_buf = NULL;
    pipeline->query_buf_size = 0;
    close(pipeline->query_buf_fd);
    ret = TRUE;
  }
    break;

  /* Below messages are per Stream */
  case CAM_MAPPING_BUF_TYPE_OFFLINE_META_BUF:
  case CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF:
  case CAM_MAPPING_BUF_TYPE_STREAM_INFO:
  case CAM_MAPPING_BUF_TYPE_STREAM_BUF: {

    mct_pipeline_get_stream_info_t get_info;
    get_info.check_type   = CHECK_INDEX;
    get_info.stream_index = msg->stream;

    stream = mct_pipeline_get_stream(pipeline, &get_info);

    if (stream && stream->unmap_buf)
      ret = stream->unmap_buf(msg, stream);
    else
      ret = FALSE;

    break;
  }

  default:
    break;
  }

finish:
  return ret;
}

/** mct_pipeline_pack_event:
 *    @
 *    @
 *    @
 *    @
 *
 *  Description:
 *
 *  Return:
 *
 **/
mct_event_t mct_pipeline_pack_event(mct_event_type type, uint32_t identity,
  mct_event_direction direction, void *payload)
{
  mct_event_t mct_event;
  mct_event.type = type;
  mct_event.identity = identity;
  mct_event.direction = direction;
  mct_event.timestamp = 0;

  if (type == MCT_EVENT_CONTROL_CMD)
    mct_event.u.ctrl_event = *((mct_event_control_t *)payload);
  else if (type == MCT_EVENT_MODULE_EVENT)
    mct_event.u.module_event = *((mct_event_module_t *)payload);

  return mct_event;
}

/** mct_pipeline_send_ctrl_events:
 *    @
 *    @
 *
 **/
boolean mct_pipeline_send_ctrl_events(mct_pipeline_t *pipeline,
  uint32_t stream_id, mct_event_control_type_t event_type)
{
  uint32_t i = 0, j = 0;
  uint32_t num_entry = 0;
  boolean ret = TRUE;
  mct_event_t cmd_event;
  mct_event_control_t event_data;
  mct_event_control_parm_t event_parm;
  parm_buffer_new_t *p_table = NULL;
  mct_stream_frame_num_idx_map_t *frame_num_id;
  mct_stream_t *meta_stream;
  mct_pipeline_get_stream_info_t info;
  parm_buffer_new_t *h_table = (parm_buffer_new_t *)pipeline->config_parm;
  parm_entry_type_new_t *curr_param = NULL;

  CDBG_ERROR("%s: Send Set Parm events", __func__);

  if (event_type == MCT_EVENT_CONTROL_SET_PARM) {
    p_table = pipeline->pending_set_parm;
  } else if (event_type == MCT_EVENT_CONTROL_GET_PARM) {
    p_table = h_table;
  }

  if (p_table == NULL) {
   CDBG_ERROR("%s: p_table is NULL",__func__);
   return FALSE;
  }

  CDBG("%s:num_entry: %d ",__func__, p_table->num_entry);

  event_data.type = event_type;
  event_data.control_event_data = &event_parm;

  cmd_event = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
    (pack_identity(MCT_PIPELINE_SESSION(pipeline), stream_id)),
     MCT_EVENT_DOWNSTREAM, &event_data);

  num_entry = p_table->num_entry;
  curr_param = (parm_entry_type_new_t *)&p_table->entry[0];

  for (j = 0; j < num_entry; j++) {
    event_parm.type = curr_param->entry_type;
    event_parm.parm_data = &curr_param->data[0];

    /* Ideally MCT should not peek in to set_param */
    if (event_parm.type == CAM_INTF_PARM_HAL_VERSION) {
      pipeline->hal_version = *(int32_t *)event_parm.parm_data;
    } else if (pipeline->hal_version == CAM_HAL_V3
               && event_parm.type == CAM_INTF_META_FRAME_NUMBER) {
      frame_num_id = malloc(sizeof(mct_stream_frame_num_idx_map_t));
      if (frame_num_id == NULL) {
        CDBG_ERROR("%s: Mapping failed with %s\n", __func__, strerror(errno));
        return FALSE;
      }
      info.check_type   = CHECK_TYPE;
      info.stream_type  = CAM_STREAM_TYPE_METADATA;
      meta_stream = mct_pipeline_get_stream(pipeline, &info);
      if (!meta_stream) {
        CDBG_ERROR("%s: Couldn't find stream\n", __func__);
        return FALSE;
      }
      frame_num_id->frame_number = *(unsigned int *)event_parm.parm_data;
      frame_num_id->frame_index  =
        meta_stream->metadata_stream.current_frame_idx;
      meta_stream->frame_num_idx_list =
        mct_list_append(meta_stream->frame_num_idx_list,
        frame_num_id, NULL, NULL);
      curr_param = GET_NEXT_PARAM(curr_param, parm_entry_type_new_t);
      continue;
    }
    else if (CAM_INTF_PARM_INT_EVT == event_parm.type) {
      continue;
    }

    if (pipeline->send_event) {
      ret = pipeline->send_event(pipeline, stream_id, &cmd_event);
      if (ret == FALSE)
        break;
    } else {
      break;
    }
    if (event_type == MCT_EVENT_CONTROL_GET_PARM) {
      //no need to handle this case specifically as get param
      //copies directly in the mapped memory
    }
    curr_param = GET_NEXT_PARAM(curr_param, parm_entry_type_new_t);
  }

  if (event_type == MCT_EVENT_CONTROL_SET_PARM) {
    memset(p_table, 0, ONE_MB_OF_PARAMS);
    p_table->tot_rem_size = ONE_MB_OF_PARAMS - sizeof(parm_buffer_new_t);
  }

  return ret;
}

/** add_pending_parm:
 *    @
 *    @
 *
 **/
static void add_pending_parm(int parm_type, parm_type_t *data,
  parm_buffer_t *p_table)
{
  int position = parm_type;
  int current, next;

  current = GET_FIRST_PARAM_ID(p_table);
  if (position == current){
    //DO NOTHING
  } else if (position < current){
    SET_NEXT_PARAM_ID(position, p_table, current);
    SET_FIRST_PARAM_ID(p_table, position);
  } else {
    /* Search for the position in the linked list where we need to slot in*/
    while (position > GET_NEXT_PARAM_ID(current, p_table))
      current = GET_NEXT_PARAM_ID(current, p_table);

    /*If node already exists no need to alter linking*/
    if (position != GET_NEXT_PARAM_ID(current, p_table)) {
      next = GET_NEXT_PARAM_ID(current, p_table);
      SET_NEXT_PARAM_ID(current, p_table, position);
      SET_NEXT_PARAM_ID(position, p_table, next);
    }
  }

  p_table->entry[parm_type].data = *data;
  return;
}

/** copy_pending_parm:
 *    @
 *    @ Copy the parameters from param buffer
 *    @
 *
 **/
static void copy_pending_parm(parm_buffer_new_t *from, parm_buffer_new_t *to)
{
  uint32_t i = 0, j = 0;
  uint32_t aligned_size_req = 0;
  boolean overwrite = FALSE;

  /*    * this is a search penalty but as the batch list is never more
   * than a few tens of entries at most,it should be ok.
   * if search performance becomes a bottleneck, we can
   * think of implementing a hashing mechanism.
   * but it is still better than the huge memory required for
   * direct indexing
   */

  parm_entry_type_new_t *from_cur = (parm_entry_type_new_t *)&from->entry[0];
  parm_entry_type_new_t *to_cur = NULL;

  for (i = 0; i < from->num_entry; i++) {
    overwrite = FALSE;
    to_cur = (parm_entry_type_new_t *)&to->entry[0];

    aligned_size_req = from_cur->aligned_size;
    for (j = 0; j < to->num_entry; j++) {
      if (from_cur->entry_type == to_cur->entry_type) {
        overwrite = TRUE;
        CDBG("%s:Overwrite existing param: %d", __func__,from_cur->entry_type);
        break;
      }
      to_cur = GET_NEXT_PARAM(to_cur, parm_entry_type_new_t);
    }

    if (aligned_size_req <= to->tot_rem_size) {
      to_cur->aligned_size = from_cur->aligned_size;
      to_cur->entry_type = from_cur->entry_type;
      to_cur->size = from_cur->size;
      memcpy(&to_cur->data[0], &from_cur->data[0], from_cur->size);

      if (CAM_INTF_PARM_INT_EVT == to_cur->entry_type) {
        mct_tuning_notify_snapshot_frame(&to_cur->data[0]);
      }

      if (FALSE == overwrite) {
        to->curr_size += aligned_size_req;
        to->tot_rem_size -= aligned_size_req;
        to->num_entry++;
      }
    }
    from_cur = GET_NEXT_PARAM(from_cur, parm_entry_type_new_t);
  }

  CDBG("%s: num params pending to be set: %d ", __func__, to->num_entry);
  return;
}


/** populate_query_cap_buffer:
 *    @
 *    @
 *
 **/
static boolean mct_pipeline_populate_query_cap_buffer(mct_pipeline_t *pipeline)
{
  mct_pipeline_cap_t *local_data = &pipeline->query_data;
  cam_capability_t   *hal_data   = pipeline->query_buf;
  if (!hal_data || !local_data) {
    CDBG_ERROR("%s:Error query_buf or query_data filled with NULL values.\n",__func__);
    return FALSE;
  }
  boolean ret = TRUE;
  uint32_t i = 0;
  uint32_t j = 0, common = FALSE;
  int32_t prev_sensor_width = 0, prev_sensor_height = 0;
  int32_t snap_sensor_width = 0, snap_sensor_height = 0;
  int32_t vid_sensor_width = 0, vid_sensor_height = 0;
  int32_t liveshot_sensor_width = 0, liveshot_sensor_height = 0;
  int32_t vhdr_liveshot_sensor_width = 0, vhdr_liveshot_sensor_height = 0;
  int32_t hfr_sensor_width[CAM_HFR_MODE_MAX];
  int32_t hfr_sensor_height[CAM_HFR_MODE_MAX];
  struct sysinfo info;
  int result;
  int enable_max_preview = 0;
  char value[PROPERTY_VALUE_MAX];

  cam_fps_range_t fps = {10.0, 30.0, 10.0, 30.0};

  memset(hfr_sensor_width, 0, sizeof(int32_t)*CAM_HFR_MODE_MAX);
  memset(hfr_sensor_height, 0, sizeof(int32_t)*CAM_HFR_MODE_MAX);

#ifdef _ANDROID_
  property_get("persist.camera.max_prev.enable", value, "0");
  enable_max_preview = atoi(value);
  if (enable_max_preview) {
    CDBG_ERROR("camera max preview enabled (persist.camera.max_prev.enable = 1)");
  }
#endif

  hal_data->modes_supported     = local_data->sensor_cap.modes_supported;
  hal_data->position            = local_data->sensor_cap.position;
  hal_data->sensor_mount_angle  = local_data->sensor_cap.sensor_mount_angle;
  hal_data->focal_length        = local_data->sensor_cap.focal_length;
  hal_data->hor_view_angle      = local_data->sensor_cap.hor_view_angle;
  hal_data->ver_view_angle      = local_data->sensor_cap.ver_view_angle;
  hal_data->min_exposure_time   = local_data->sensor_cap.min_exposure_time;
  hal_data->max_exposure_time   = local_data->sensor_cap.max_exposure_time;
  hal_data->min_iso             = local_data->sensor_cap.min_iso;
  hal_data->max_iso             = local_data->sensor_cap.max_iso;
  hal_data->near_end_distance   = local_data->sensor_cap.near_end_distance;

  for (i = 0; i < local_data->sensor_cap.dim_fps_table_count; i++) {
    if (local_data->sensor_cap.dim_fps_table[i].fps.max_fps >= 29.0 &&
        local_data->sensor_cap.dim_fps_table[i].fps.max_fps < 60.0) {
      if (local_data->sensor_cap.dim_fps_table[i].dim.width > liveshot_sensor_width) {
        if (local_data->sensor_cap.dim_fps_table[i].dim.height >
          liveshot_sensor_height) {
          liveshot_sensor_width  =
            local_data->sensor_cap.dim_fps_table[i].dim.width;
          liveshot_sensor_height =
            local_data->sensor_cap.dim_fps_table[i].dim.height;
        }
      }
    }
    if (local_data->sensor_cap.dim_fps_table[i].fps.max_fps >= 15.0 &&
        local_data->sensor_cap.dim_fps_table[i].fps.max_fps < 60.0) {
      if (local_data->sensor_cap.dim_fps_table[i].dim.width > prev_sensor_width) {
        if (local_data->sensor_cap.dim_fps_table[i].dim.height >
          prev_sensor_height) {
          prev_sensor_width  =
            local_data->sensor_cap.dim_fps_table[i].dim.width;
          prev_sensor_height =
            local_data->sensor_cap.dim_fps_table[i].dim.height;
          vid_sensor_width  =
            local_data->sensor_cap.dim_fps_table[i].dim.width;
          vid_sensor_height =
            local_data->sensor_cap.dim_fps_table[i].dim.height;
          if (local_data->sensor_cap.dim_fps_table[i].fps.max_fps < 30.0) {
             fps.min_fps = local_data->sensor_cap.dim_fps_table[i].fps.min_fps;
             fps.max_fps = 30.0;
          }
          else
            fps = local_data->sensor_cap.dim_fps_table[i].fps;

        } else { /* check if the resolution is 16:9 */
          if (fabs(((float)local_data->sensor_cap.dim_fps_table[i].dim.width)
              /((float)local_data->sensor_cap.dim_fps_table[i].dim.height)-1.778)
              <= 0.1) {
            vid_sensor_width =
              local_data->sensor_cap.dim_fps_table[i].dim.width;
            vid_sensor_height =
              local_data->sensor_cap.dim_fps_table[i].dim.height;
            liveshot_sensor_width  =
              local_data->sensor_cap.dim_fps_table[i].dim.width;
            liveshot_sensor_height =
              local_data->sensor_cap.dim_fps_table[i].dim.height;
          }
        }
      }
    } else if (local_data->sensor_cap.dim_fps_table[i].fps.max_fps >= 60.0 &&
        local_data->sensor_cap.dim_fps_table[i].fps.max_fps < 90.0) {
        /*HFR mode 60fps */
        if (local_data->sensor_cap.dim_fps_table[i].dim.width >
                                    hfr_sensor_width[CAM_HFR_MODE_60FPS]
            && local_data->sensor_cap.dim_fps_table[i].dim.height >
                                    hfr_sensor_height[CAM_HFR_MODE_60FPS] ) {
              hfr_sensor_width[CAM_HFR_MODE_60FPS]  =
                local_data->sensor_cap.dim_fps_table[i].dim.width;
              hfr_sensor_height[CAM_HFR_MODE_60FPS] =
                local_data->sensor_cap.dim_fps_table[i].dim.height;
        }
    } else if (local_data->sensor_cap.dim_fps_table[i].fps.max_fps >= 90.0 &&
        local_data->sensor_cap.dim_fps_table[i].fps.max_fps < 120.0) {
        /*HFR mode 90fps*/
        if (local_data->sensor_cap.dim_fps_table[i].dim.width >
                                    hfr_sensor_width[CAM_HFR_MODE_90FPS]
            && local_data->sensor_cap.dim_fps_table[i].dim.height >
                                    hfr_sensor_height[CAM_HFR_MODE_90FPS]) {
              hfr_sensor_width[CAM_HFR_MODE_90FPS]  =
                local_data->sensor_cap.dim_fps_table[i].dim.width;
              hfr_sensor_height[CAM_HFR_MODE_90FPS] =
                local_data->sensor_cap.dim_fps_table[i].dim.height;
        }
    } else if (local_data->sensor_cap.dim_fps_table[i].fps.max_fps >= 120.0 &&
        local_data->sensor_cap.dim_fps_table[i].fps.max_fps < 150.0) {
        /*HFR mode 120 fps*/
        if (local_data->sensor_cap.dim_fps_table[i].dim.width >
                                    hfr_sensor_width[CAM_HFR_MODE_120FPS]
            && local_data->sensor_cap.dim_fps_table[i].dim.height >
                                    hfr_sensor_height[CAM_HFR_MODE_120FPS]) {
              hfr_sensor_width[CAM_HFR_MODE_120FPS]  =
                local_data->sensor_cap.dim_fps_table[i].dim.width;
              hfr_sensor_height[CAM_HFR_MODE_120FPS] =
                local_data->sensor_cap.dim_fps_table[i].dim.height;
        }
    } else if (local_data->sensor_cap.dim_fps_table[i].fps.max_fps >= 150.0) {
        /*HFR mode 150 fps */
        if (local_data->sensor_cap.dim_fps_table[i].dim.width >
                                    hfr_sensor_width[CAM_HFR_MODE_150FPS]
            && local_data->sensor_cap.dim_fps_table[i].dim.height >
                                    hfr_sensor_height[CAM_HFR_MODE_150FPS]) {
              hfr_sensor_width[CAM_HFR_MODE_150FPS]  =
                local_data->sensor_cap.dim_fps_table[i].dim.width;
              hfr_sensor_height[CAM_HFR_MODE_150FPS] =
                local_data->sensor_cap.dim_fps_table[i].dim.height;
        }
    }

    if (local_data->sensor_cap.dim_fps_table[i].mode == (1 << 2)) {
      if (local_data->sensor_cap.dim_fps_table[i].dim.width >
        vhdr_liveshot_sensor_width &&
          local_data->sensor_cap.dim_fps_table[i].dim.height >
          vhdr_liveshot_sensor_height) {
        vhdr_liveshot_sensor_width =
          local_data->sensor_cap.dim_fps_table[i].dim.width;
        vhdr_liveshot_sensor_height =
          local_data->sensor_cap.dim_fps_table[i].dim.height;
      }
   }

    if (local_data->sensor_cap.dim_fps_table[i].dim.width >
        snap_sensor_width
        && local_data->sensor_cap.dim_fps_table[i].dim.height >
        snap_sensor_height) {
      snap_sensor_width  =
        local_data->sensor_cap.dim_fps_table[i].dim.width;
      snap_sensor_height =
        local_data->sensor_cap.dim_fps_table[i].dim.height;
    }
  } /*for loop*/
  result = sysinfo(&info);
  CDBG_HIGH("%s: totalram = %ld, freeram = %ld ", __func__, info.totalram,
    info.freeram);
  if (info.totalram <= TOTAL_RAM_SIZE_512MB) {
    if (prev_sensor_width > PICTURE_SIZE_5MP_WIDTH && prev_sensor_height >
        PICTURE_SIZE_5MP_HEIGHT) {
      prev_sensor_width = PICTURE_SIZE_5MP_WIDTH;
      prev_sensor_height = PICTURE_SIZE_5MP_HEIGHT;
    } else if (prev_sensor_width > PICTURE_SIZE_5MP_WIDTH ||
               prev_sensor_height > PICTURE_SIZE_5MP_HEIGHT) {
        for (i = 0;
             (i < (sizeof(default_preview_sizes) / sizeof(cam_dimension_t))
                && i < MAX_SIZES_CNT);
             i++) {
          if (default_preview_sizes[i].width <= PICTURE_SIZE_5MP_WIDTH &&
              default_preview_sizes[i].height <= PICTURE_SIZE_5MP_HEIGHT) {
            prev_sensor_width = default_preview_sizes[i].width;
            prev_sensor_height = default_preview_sizes[i].height;
            break;
          }
        }
    }
    if (snap_sensor_width > PICTURE_SIZE_5MP_WIDTH &&
        snap_sensor_height > PICTURE_SIZE_5MP_HEIGHT) {
      snap_sensor_width = PICTURE_SIZE_5MP_WIDTH;
      snap_sensor_height = PICTURE_SIZE_5MP_HEIGHT;
    } else if (snap_sensor_width > PICTURE_SIZE_5MP_WIDTH ||
               snap_sensor_height > PICTURE_SIZE_5MP_HEIGHT) {
        for (i = 0;
             (i < (sizeof(default_picture_sizes) / sizeof(cam_dimension_t))
                && i < MAX_SIZES_CNT);
             i++) {
          if (default_picture_sizes[i].width <= PICTURE_SIZE_5MP_WIDTH &&
              default_picture_sizes[i].height <= PICTURE_SIZE_5MP_HEIGHT) {
            snap_sensor_width = default_picture_sizes[i].width;
            snap_sensor_height = default_picture_sizes[i].height;
            break;
          }
        }
    }
    if (liveshot_sensor_width > PICTURE_SIZE_5MP_WIDTH &&
        liveshot_sensor_height > PICTURE_SIZE_5MP_HEIGHT) {
        liveshot_sensor_width = PICTURE_SIZE_5MP_WIDTH;
        liveshot_sensor_height = PICTURE_SIZE_5MP_HEIGHT;
    } else if (liveshot_sensor_width > PICTURE_SIZE_5MP_WIDTH ||
                 liveshot_sensor_height > PICTURE_SIZE_5MP_HEIGHT) {
        for (i = 0;
             (i < (sizeof(default_liveshot_sizes) / sizeof(cam_dimension_t))
                && i < MAX_SIZES_CNT);
             i++) {
          if (default_liveshot_sizes[i].width <= PICTURE_SIZE_5MP_WIDTH &&
              default_liveshot_sizes[i].height <= PICTURE_SIZE_5MP_HEIGHT) {
            liveshot_sensor_width = default_liveshot_sizes[i].width;
            liveshot_sensor_height = default_liveshot_sizes[i].height;
            break;
          }
        }
    }
  }

  /*get preview sizes and formats*/
  hal_data->preview_sizes_tbl_cnt = 0;
  if(enable_max_preview) {
    int32_t max_preview_width = 0;
    int32_t max_preview_height = 0;

    for(i = 0;
        i < (sizeof(default_picture_sizes) / sizeof(cam_dimension_t));
        i++) {
      if((default_picture_sizes[i].width > max_preview_width ||
          default_picture_sizes[i].height > max_preview_height) &&
         (default_picture_sizes[i].width <= snap_sensor_width &&
          default_picture_sizes[i].height <= snap_sensor_height)) {
        max_preview_width = default_picture_sizes[i].width;
        max_preview_height = default_picture_sizes[i].height;
      }
    }

    hal_data->preview_sizes_tbl[hal_data->preview_sizes_tbl_cnt].width =
        max_preview_width;
    hal_data->preview_sizes_tbl[hal_data->preview_sizes_tbl_cnt].height =
        max_preview_height;
    hal_data->preview_sizes_tbl_cnt++;
  }
  for (i = hal_data->preview_sizes_tbl_cnt;
       (i < (sizeof(default_preview_sizes) / sizeof(cam_dimension_t))
          && i < MAX_SIZES_CNT);
       i++) {
    if (default_preview_sizes[i].width <= prev_sensor_width &&
        default_preview_sizes[i].height <= prev_sensor_height) {
      hal_data->preview_sizes_tbl[hal_data->preview_sizes_tbl_cnt] =
            default_preview_sizes[i];
      hal_data->preview_sizes_tbl_cnt++;
    }
  }
  hal_data->supported_preview_fmt_cnt = 3;
  hal_data->supported_preview_fmts[0] = CAM_FORMAT_YUV_420_NV21;
  hal_data->supported_preview_fmts[1] = CAM_FORMAT_YUV_420_YV12;
  hal_data->supported_preview_fmts[2] = CAM_FORMAT_YUV_420_NV12_VENUS;

  /*get snapshot sizes and formats*/
#if 1
  hal_data->picture_sizes_tbl_cnt = 0;
  for (i = 0;
       (i < (sizeof(default_picture_sizes) / sizeof(cam_dimension_t))
          && i < MAX_SIZES_CNT);
       i++) {
    if (default_picture_sizes[i].width <= snap_sensor_width &&
        default_picture_sizes[i].height <= snap_sensor_height) {
      hal_data->picture_sizes_tbl[hal_data->picture_sizes_tbl_cnt] =
          default_picture_sizes[i];
      hal_data->picture_sizes_tbl_cnt++;
    }
  }
#else
  hal_data->picture_sizes_tbl_cnt = 1;
  hal_data->picture_sizes_tbl[0].width = 1024;
  hal_data->picture_sizes_tbl[0].height = 768;
#endif

  /* HFR modes */
  uint32_t idx=0;
  for (i=0; i < CAM_HFR_MODE_MAX; i++) {
    if (hfr_sensor_width[i] != 0 && hfr_sensor_height[i] != 0) {
      hal_data->hfr_tbl[idx].mode = i;
      hal_data->hfr_tbl[idx].dim.width = hfr_sensor_width[i];
      hal_data->hfr_tbl[idx].dim.height = hfr_sensor_height[i];
      hal_data->hfr_tbl[idx].livesnapshot_sizes_tbl_cnt = 0;
      for (j = 0;
          (j < (sizeof(default_liveshot_sizes) / sizeof(cam_dimension_t))
            && j < MAX_SIZES_CNT);
           j++) {
        if (default_liveshot_sizes[j].width <= hfr_sensor_width[i] &&
          default_liveshot_sizes[j].height <= hfr_sensor_height[i]) {
          hal_data->hfr_tbl[idx].livesnapshot_sizes_tbl
            [hal_data->hfr_tbl[idx].livesnapshot_sizes_tbl_cnt] = default_liveshot_sizes[j];
          hal_data->hfr_tbl[idx].livesnapshot_sizes_tbl_cnt++;
        }
      }
      idx++;
    }
  }
  hal_data->hfr_tbl_cnt = idx;

  /*get video sizes*/
#if 1
  hal_data->video_sizes_tbl_cnt = 0;
  for (i = 0;
       (i < (sizeof(default_video_sizes) / sizeof(cam_dimension_t))
          && i < MAX_SIZES_CNT);
       i++) {
    if (default_video_sizes[i].width <= vid_sensor_width &&
        default_video_sizes[i].height <= vid_sensor_height) {
      hal_data->video_sizes_tbl[hal_data->video_sizes_tbl_cnt] =
            default_video_sizes[i];
      hal_data->video_sizes_tbl_cnt++;
    }
  }
  hal_data->video_snapshot_supported = 1;
#else
  hal_data->video_sizes_tbl_cnt = 1;
  hal_data->video_sizes_tbl[0].width = 640;
  hal_data->video_sizes_tbl[0].height = 480;
#endif

  hal_data->livesnapshot_sizes_tbl_cnt = 0;
  for (i = 0;
       (i < (sizeof(default_liveshot_sizes) / sizeof(cam_dimension_t))
          && i < MAX_SIZES_CNT);
       i++) {
    if (default_liveshot_sizes[i].width <= liveshot_sensor_width &&
        default_liveshot_sizes[i].height <= liveshot_sensor_height) {
      hal_data->livesnapshot_sizes_tbl[hal_data->livesnapshot_sizes_tbl_cnt] =
            default_liveshot_sizes[i];
      hal_data->livesnapshot_sizes_tbl_cnt++;
    }
  }

  hal_data->vhdr_livesnapshot_sizes_tbl_cnt = 0;
  for (i = 0;
       (i < (sizeof(default_liveshot_sizes) / sizeof(cam_dimension_t))
          && i < MAX_SIZES_CNT);
       i++) {
    if (default_liveshot_sizes[i].width <= vhdr_liveshot_sensor_width &&
        default_liveshot_sizes[i].height <= vhdr_liveshot_sensor_height) {
      hal_data->vhdr_livesnapshot_sizes_tbl[hal_data->vhdr_livesnapshot_sizes_tbl_cnt] =
            default_liveshot_sizes[i];
      hal_data->vhdr_livesnapshot_sizes_tbl_cnt++;
    }
  }

  /*get scale picture size */
  hal_data->scale_picture_sizes_cnt = 0;
  for (i = 0;
       (i < local_data->sensor_cap.scale_picture_sizes_cnt
          && i < MAX_SCALE_SIZES_CNT);
       i++) {
    hal_data->scale_picture_sizes[i] = local_data->sensor_cap.scale_picture_sizes[i];
    hal_data->scale_picture_sizes_cnt++;
  }

  hal_data->padding_info.width_padding  = local_data->pp_cap.width_padding;
  hal_data->padding_info.height_padding = local_data->pp_cap.height_padding;
  hal_data->padding_info.plane_padding  = local_data->pp_cap.plane_padding;

  hal_data->raw_dim.width = snap_sensor_width;
  hal_data->raw_dim.height = snap_sensor_height;

  hal_data->supported_raw_fmt_cnt =
    local_data->sensor_cap.supported_raw_fmts_cnt;
  for (i = 0; i < hal_data->supported_raw_fmt_cnt; i++)
  {
    hal_data->supported_raw_fmts[i] =
      local_data->sensor_cap.supported_raw_fmts[i];
  }

    hal_data->fps_ranges_tbl_cnt = 0;
    for (i = 0;
       (i < (sizeof(default_fps_ranges) / sizeof(cam_fps_range_t))
          && i < MAX_SIZES_CNT);
       i++) {
    if (default_fps_ranges[i].min_fps >= fps.min_fps &&
        default_fps_ranges[i].max_fps <= fps.max_fps) {
      hal_data->fps_ranges_tbl[hal_data->fps_ranges_tbl_cnt] =
            default_fps_ranges[i];
      hal_data->fps_ranges_tbl_cnt++;
    }
  }
  if (hal_data->fps_ranges_tbl_cnt == 0) {
    hal_data->fps_ranges_tbl[hal_data->fps_ranges_tbl_cnt] = fps;
    hal_data->fps_ranges_tbl_cnt++;
  } else {
    for (i = 0; i < hal_data->fps_ranges_tbl_cnt && i < MAX_SIZES_CNT ; i++) {
      if ( (hal_data->fps_ranges_tbl[i].max_fps > fps.max_fps) ||
       ((F_EQUAL(hal_data->fps_ranges_tbl[i].max_fps, fps.max_fps)) &&
        (hal_data->fps_ranges_tbl[i].min_fps > fps.min_fps) )){
        break;
      }
    }
    if (MAX_SIZES_CNT > i) {
      for (j = hal_data->fps_ranges_tbl_cnt; j > i; j--) {
        hal_data->fps_ranges_tbl[j] = hal_data->fps_ranges_tbl[j-1];
      }
      hal_data->fps_ranges_tbl[i] = fps;
      hal_data->fps_ranges_tbl_cnt++;
    }
  }

  hal_data->brightness_ctrl.def_value = 3;
  hal_data->brightness_ctrl.max_value = 6;
  hal_data->brightness_ctrl.min_value = 0;
  hal_data->brightness_ctrl.step = 1;

  hal_data->sharpness_ctrl.def_value = 12;
  hal_data->sharpness_ctrl.max_value = 36;
  hal_data->sharpness_ctrl.min_value = 0;
  hal_data->sharpness_ctrl.step = 6;

  hal_data->contrast_ctrl.def_value = 5;
  hal_data->contrast_ctrl.max_value = 10;
  hal_data->contrast_ctrl.min_value = 0;
  hal_data->contrast_ctrl.step = 1;

  hal_data->saturation_ctrl.def_value = 5;
  hal_data->saturation_ctrl.max_value = 10;
  hal_data->saturation_ctrl.min_value = 0;
  hal_data->saturation_ctrl.step = 1;

  hal_data->sce_ctrl.def_value = 0;
  hal_data->sce_ctrl.max_value = 100;
  hal_data->sce_ctrl.min_value = -100;
  hal_data->sce_ctrl.step = 10;

  hal_data->supported_effects_cnt = 0;
  if (local_data->sensor_cap.sensor_format == FORMAT_BAYER) {
    for (i = 0; i < local_data->isp_cap.supported_effects_cnt; i++) {
      hal_data->supported_effects[hal_data->supported_effects_cnt] =
        local_data->isp_cap.supported_effects[i];
         hal_data->supported_effects_cnt++;
    }
  } else {
    for (i = 0; i < CAM_EFFECT_MODE_MAX; ++i) {
      if (local_data->sensor_cap.sensor_supported_effect_modes & (1 << i)) {
        hal_data->supported_effects[hal_data->supported_effects_cnt] = i;
        hal_data->supported_effects_cnt ++;
      }
    }
  }

  uint32_t supported_effect_cnt = hal_data->supported_effects_cnt;
  for (j = 0; j < local_data->pp_cap.supported_effects_cnt; j++) {
    common = FALSE;
    for (i = 0; i < supported_effect_cnt; i++) {
      if (hal_data->supported_effects[i] ==
        local_data->pp_cap.supported_effects[j]) {
        common = TRUE;
        break;
      }
    }
    if (common == TRUE)
      continue;
    hal_data->supported_effects[hal_data->supported_effects_cnt] =
      local_data->pp_cap.supported_effects[j];
       hal_data->supported_effects_cnt++;
  }
  /* Add supported scene modes */
  if (local_data->sensor_cap.scene_mode_supported) {
    hal_data->supported_scene_modes_cnt =
      local_data->stats_cap.supported_scene_modes_cnt;
    memcpy(hal_data->supported_scene_modes,
      local_data->stats_cap.supported_scene_modes,
      sizeof(cam_scene_mode_type) *
      MIN(hal_data->supported_scene_modes_cnt, CAM_SCENE_MODE_MAX));
  } else {
    for (i = 0; i < CAM_SCENE_MODE_MAX; ++i) {
      if (local_data->sensor_cap.sensor_supported_scene_modes & (1 << i)) {
        hal_data->supported_scene_modes[hal_data->supported_scene_modes_cnt] = i;
        hal_data->supported_scene_modes_cnt ++;
      }
    }
  }

  hal_data->exposure_compensation_min = -12;
  hal_data->exposure_compensation_max = 12;
  hal_data->exposure_compensation_default = 0;
  hal_data->exposure_compensation_step = (float)(1.0/6.0);

  hal_data->supported_antibandings_cnt =
    local_data->stats_cap.supported_antibandings_cnt;
  memcpy(hal_data->supported_antibandings, local_data->stats_cap.supported_antibandings,
    sizeof(cam_antibanding_mode_type) * MIN(hal_data->supported_antibandings_cnt, CAM_ANTIBANDING_MODE_MAX));

  if (local_data->sensor_cap.ae_lock_supported)
    hal_data->auto_exposure_lock_supported =
      local_data->stats_cap.auto_exposure_lock_supported;

  if (local_data->sensor_cap.wb_lock_supported)
    hal_data->auto_wb_lock_supported =
      local_data->stats_cap.auto_wb_lock_supported;

  hal_data->supported_iso_modes_cnt = local_data->stats_cap.supported_iso_modes_cnt;
  memcpy(hal_data->supported_iso_modes, local_data->stats_cap.supported_iso_modes,
         sizeof(cam_iso_mode_type) * MIN(hal_data->supported_iso_modes_cnt, CAM_ISO_MODE_MAX));

  hal_data->supported_aec_modes_cnt = local_data->stats_cap.supported_aec_modes_cnt;
  memcpy(hal_data->supported_aec_modes, local_data->stats_cap.supported_aec_modes,
         sizeof(cam_auto_exposure_mode_type) * MIN(hal_data->supported_aec_modes_cnt, CAM_AEC_MODE_MAX));

  if (local_data->sensor_cap.is_flash_supported) {
    hal_data->supported_flash_modes_cnt = local_data->stats_cap.supported_flash_modes_cnt;
    memcpy(hal_data->supported_flash_modes, local_data->stats_cap.supported_flash_modes,
         sizeof(cam_flash_mode_t) * MIN(hal_data->supported_flash_modes_cnt, CAM_FLASH_MODE_MAX));
  }

  if (!local_data->sensor_cap.af_supported) {
    hal_data->supported_focus_modes_cnt = 1;
    hal_data->supported_focus_modes[0] = CAM_FOCUS_MODE_FIXED;
    hal_data->max_num_focus_areas = 0;
  } else {
    hal_data->supported_focus_modes_cnt =
      local_data->stats_cap.supported_focus_modes_cnt;
    memcpy(hal_data->supported_focus_modes,
      local_data->stats_cap.supported_focus_modes,
      sizeof(cam_focus_mode_type) * MIN(hal_data->supported_focus_modes_cnt, CAM_FOCUS_MODE_MAX));

    hal_data->max_num_focus_areas =
    local_data->stats_cap.max_num_focus_areas;
  }

  hal_data->supported_focus_algos_cnt = local_data->stats_cap.supported_focus_algos_cnt;
  memcpy(hal_data->supported_focus_algos, local_data->stats_cap.supported_focus_algos,
     sizeof(cam_focus_algorithm_type) * MIN(hal_data->supported_focus_algos_cnt,CAM_FOCUS_ALGO_MAX));

  hal_data->supported_white_balances_cnt = local_data->stats_cap.supported_white_balances_cnt;
  memcpy(hal_data->supported_white_balances, local_data->stats_cap.supported_white_balances,
     sizeof(cam_wb_mode_type) * MIN(hal_data->supported_white_balances_cnt,CAM_WB_MODE_MAX));

  /*Zoom support*/
  if (local_data->isp_cap.zoom_ratio_tbl_cnt > 0) {
    hal_data->zoom_supported = TRUE;
    hal_data->zoom_ratio_tbl_cnt = local_data->isp_cap.zoom_ratio_tbl_cnt;
    for (i = 0; i < local_data->isp_cap.zoom_ratio_tbl_cnt; i++) {
      hal_data->zoom_ratio_tbl[i] = local_data->isp_cap.zoom_ratio_tbl[i];
    }
  } else {
    hal_data->zoom_supported = FALSE;
  }

  /* add face ROI to HAL API */
  hal_data->max_num_roi = local_data->imaging_cap.max_num_roi;

  /* add HDR capability details to HAL */
  memcpy(&hal_data->hdr_bracketing_setting,
    &local_data->imaging_cap.hdr_bracketing_setting,
    sizeof(cam_hdr_bracketing_info_t));

  /* add Ubifocus capability details to HAL */
  memcpy(&hal_data->ubifocus_af_bracketing_need,
    &local_data->imaging_cap.ubifocus_af_bracketing_need,
    sizeof(cam_af_bracketing_t));

  /* add refocus capability details to HAL */
  memcpy(&hal_data->refocus_af_bracketing_need,
    &local_data->imaging_cap.refocus_af_bracketing_need,
    sizeof(cam_af_bracketing_t));

  /* add Optizoom capability details to HAL */
  memcpy(&hal_data->opti_zoom_settings_need,
    &local_data->imaging_cap.opti_zoom_settings,
    sizeof(cam_opti_zoom_t));

  /* add trueportrait capability details to HAL */
  memcpy(&hal_data->true_portrait_settings_need,
    &local_data->imaging_cap.true_portrait_settings,
    sizeof(cam_true_portrait_t));

  /* add FSSR capability details to HAL */
  memcpy(&hal_data->fssr_settings_need,
    &local_data->imaging_cap.fssr_settings,
    sizeof(cam_fssr_t));

  memcpy(&hal_data->mtf_af_bracketing_parm,
    &local_data->imaging_cap.mtf_af_bracketing,
    sizeof(cam_af_bracketing_t));

  hal_data->max_num_metering_areas =
    local_data->stats_cap.max_num_metering_areas;

  hal_data->min_num_pp_bufs =
    local_data->pp_cap.min_num_pp_bufs;
  hal_data->is_sw_wnr =
    local_data->pp_cap.is_sw_wnr;
  hal_data->qcom_supported_feature_mask =
    local_data->sensor_cap.feature_mask | local_data->isp_cap.feature_mask |
    local_data->stats_cap.feature_mask | local_data->pp_cap.feature_mask |
    local_data->imaging_cap.feature_mask;

  /*Specific Capabilities for HAL 3*/

  hal_data->min_focus_distance = local_data->sensor_cap.min_focus_distance;
  hal_data->hyper_focal_distance = local_data->sensor_cap.hyper_focal_distance;

  for (i = 0; i < local_data->sensor_cap.focal_lengths_count; i++) {
     hal_data->focal_lengths[i] = local_data->sensor_cap.focal_lengths[i];
  }
  hal_data->focal_lengths_count = local_data->sensor_cap.focal_lengths_count;

  for (i = 0; i < local_data->sensor_cap.apertures_count; i++) {
     hal_data->apertures[i] = local_data->sensor_cap.apertures[i];
  }
  hal_data->apertures_count = local_data->sensor_cap.apertures_count;
#if 0
  /*TO DO*/
  for (i = 0; i < local_data->??.filter_densities_count; i++) {
     hal_data->filter_densities[i] = local_data->??.filter_densities[i];
  }
  hal_data->filter_densities_count = local_data->??.filter_densities_count;

  for (i = 0; i < local_data->??.optical_stab_modes_count; i++) {
     hal_data->optical_stab_modes[i] = local_data->??.optical_stab_modes[i];
  }
  hal_data->optical_stab_modes_count = local_data->??.optical_stab_modes_count;
#endif

  /*TODO: lens_shading_map_size and geo_correction_map_size*/

  for (i = 0; i < 3; i++) {
     hal_data->lens_position[i] = local_data->sensor_cap.lens_position[i];
  }

  for (i = 0; i < 2; i++) {
     hal_data->exposure_time_range[i] =
       local_data->stats_cap.exposure_time_range[i];
  }

  hal_data->max_frame_duration = local_data->sensor_cap.max_frame_duration;
  /*TODO: Color arrangement*/
  /*hal_data->color_arrangement = */

  for (i = 0; i < 2; i++) {
     hal_data->sensor_physical_size[i] =
       local_data->sensor_cap.sensor_physical_size[i];
  }
  hal_data->pixel_array_size = local_data->sensor_cap.pixel_array_size;
#if 0
  /*TODO: Active array size*/
  /*hal_data->active_array_size =*/

  hal_data->white_level = local_data->isp_cap.white_level;
  for (i = 0; i < 4; i++) {
     hal_data->black_level_pattern[i] =
       local_data->isp_cap.black_level_pattern[i];
  }
#endif
  hal_data->flash_charge_duration =
    local_data->sensor_cap.flash_charge_duration;
#if 0
  TODO:
  hal_data->max_tone_map_curve_points =
    local_data->isp_cap.max_tone_map_curve_points;

  for (i = 0; i < local_data->??.supported_scalar_format_count; i++) {
     hal_data->supported_scalar_fmts[i] = local_data->??.supported_scalar_fmts[i];
  }
  hal_data->supported_scalar_format_count =
    local_data->??.supported_scalar_format_count;
  /*TODO: Raw min duration*/

  /*TODO: */
  hal_data->raw_min_duration =
  for (i = 0; i < local_data->??.supported_sizes_tbl_count; i++) {
     hal_data->supported_sizes_tbl[i] = local_data->??.supported_sizes_tbl[i];
  }
  hal_data->supported_sizes_tbl_count =
    local_data->??.supported_sizes_tbl_count;

  hal_data->min_duration =
#endif
  hal_data->max_face_detection_count =
    local_data->imaging_cap.max_face_detection_count;
  hal_data->histogram_size = local_data->isp_cap.histogram_size;
  hal_data->max_histogram_count = local_data->isp_cap.max_histogram_count;
  /*TODO: hal_data->sharpness_map_size  = */
  hal_data->max_sharpness_map_value = local_data->isp_cap.max_sharpness_map_value;

  /*Histogram ONLY support the Bayer sensor*/
  hal_data->histogram_supported = local_data->sensor_cap.sensor_format == FORMAT_BAYER;

  hal_data->auto_hdr_supported = local_data->sensor_cap.sensor_format == FORMAT_BAYER;
  hal_data->sensor_type.sens_type = (cam_sensor_t)local_data->sensor_cap.sensor_format;
  hal_data->low_power_mode_supported = local_data->isp_cap.low_power_mode_supported;
  hal_data->use_pix_for_SOC = local_data->isp_cap.use_pix_for_SOC;
  return ret;
}

/** mct_pipeline_process_set:
 *    @
 *    @
 *
 **/
static boolean mct_pipeline_process_set(struct msm_v4l2_event_data *data,
  mct_pipeline_t *pipeline)
{
  boolean      ret = TRUE;
  mct_stream_t *stream = NULL;
  mct_pipeline_get_stream_info_t info;

  CDBG_HIGH("%s:command=%x", __func__, data->command);

  /* First find correct stream; for some commands find based on index,
   * for others (session based commands) find the appropriate stream
   * based on stream_type */
  switch (data->command) {
  case CAM_PRIV_STREAM_INFO_SYNC:
  case MSM_CAMERA_PRIV_STREAM_ON:
  case MSM_CAMERA_PRIV_STREAM_OFF:
  case MSM_CAMERA_PRIV_DEL_STREAM:
  case CAM_PRIV_STREAM_PARM: {
    info.check_type   = CHECK_INDEX;
    info.stream_index = data->stream_id;
    stream = mct_pipeline_get_stream(pipeline, &info);
    if (!stream) {
      CDBG("%s:%d: Couldn't find stream\n", __func__, __LINE__);
      return FALSE;
    }
  }
    break;

  case CAM_PRIV_PREPARE_SNAPSHOT:
  case CAM_PRIV_START_ZSL_SNAPSHOT:
  case CAM_PRIV_STOP_ZSL_SNAPSHOT:
  case CAM_PRIV_CANCEL_AUTO_FOCUS:
  case CAM_PRIV_DO_AUTO_FOCUS: {
    info.check_type   = CHECK_TYPE;
    info.stream_type  = CAM_STREAM_TYPE_PREVIEW;
    stream = mct_pipeline_get_stream(pipeline, &info);
    if (!stream) {
      CDBG_ERROR("%s:%d: Couldn't find stream\n", __func__, __LINE__);
      if (data->command == CAM_PRIV_CANCEL_AUTO_FOCUS) {
        /* preview stream is not present.
         * There is no module linked to the stream.
         * STATS module can't handle the event if sent downstream.
         * plus cancel auto focus do not have call back.
         */
        return TRUE;
      } else {
        return FALSE;
      }
    }
  }
    break;

  case CAM_PRIV_PARM: {
    /* This case could potentially hit even before a stream exists */
    if (MCT_PIPELINE_NUM_CHILDREN(pipeline) > 0) {
      info.check_type   = CHECK_TYPE;
      info.stream_type  = CAM_STREAM_TYPE_PREVIEW;
      stream = mct_pipeline_get_stream(pipeline, &info);
      if (!stream) {
        info.check_type   = CHECK_TYPE;
        info.stream_type  = CAM_STREAM_TYPE_RAW; /*RDI streaming*/
        stream = mct_pipeline_get_stream(pipeline, &info);
        if (!stream) {
          CDBG_ERROR("%s: Couldn't find preview stream; Storing for later\n",
            __func__);
        }
      }
    }
  }
    break;

  default:
    break;
  }

  /* Now process the set ctrl command on the appropriate stream */
  switch (data->command) {
  case MSM_CAMERA_PRIV_NEW_STREAM: {
    if (pipeline->add_stream)
      ret = pipeline->add_stream(pipeline, data->stream_id);
    else
      ret = FALSE;
  }
    break;

  case CAM_PRIV_STREAM_INFO_SYNC: {
  /* This will trigger module liniing*/
    if (!MCT_STREAM_STREAMINFO(stream)) {
      ret = FALSE;
    } else {
      (MCT_STREAM_LINK(stream)) ?
        (ret = (MCT_STREAM_LINK(stream))(stream)) : (ret = FALSE);
    }
  }
    break;

  case MSM_CAMERA_PRIV_STREAM_ON:
  case MSM_CAMERA_PRIV_STREAM_OFF:{
    mct_event_t         cmd_event;
    mct_event_control_t event_data;

    if (data->command == MSM_CAMERA_PRIV_STREAM_ON)
      event_data.type = MCT_EVENT_CONTROL_STREAMON;
    else
      event_data.type = MCT_EVENT_CONTROL_STREAMOFF;

    event_data.control_event_data = (void *)&stream->streaminfo;
    CDBG_HIGH("%s: stream_type = %d\n", __func__, stream->streaminfo.stream_type);
    cmd_event = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
      (pack_identity(MCT_PIPELINE_SESSION(pipeline), data->stream_id)),
       MCT_EVENT_DOWNSTREAM, &event_data);

    if (pipeline->send_event)
      ret = pipeline->send_event(pipeline, data->stream_id, &cmd_event);
    else
      ret = FALSE;

    if (data->command == MSM_CAMERA_PRIV_STREAM_ON) {
      stream->state = MCT_ST_STATE_RUNNING;
      if (stream->streaminfo.stream_type == CAM_STREAM_TYPE_PREVIEW)
        mct_notify_reload_tuning();
    } else {
      stream->state = MCT_ST_STATE_NONE;
    }
  }
    break;

  case MSM_CAMERA_PRIV_DEL_STREAM: {
    if (pipeline->remove_stream)
      ret = pipeline->remove_stream(pipeline, stream);
    else
      ret = FALSE;
  }
    break;

  case CAM_PRIV_PARM: {
    /*start reading the parm buf from HAL*/
    parm_buffer_new_t *p_table = (parm_buffer_new_t *)pipeline->config_parm;

    if (p_table && p_table->num_entry) {
      CDBG("%s: num params in set_param: %d ", __func__, p_table->num_entry);
      copy_pending_parm(p_table, pipeline->pending_set_parm);
    }
    if (stream) {
      /*Send event only if a stream exists*/
      ret = mct_pipeline_send_ctrl_events(pipeline, stream->streamid,
            MCT_EVENT_CONTROL_SET_PARM);
    } else {
      ret = TRUE;
    }
  }
  break;

  case CAM_PRIV_STREAM_PARM: {
    mct_event_t cmd_event;
    mct_event_control_t event_data;
    cam_stream_info_t *stream_info_buf;

    ret = FALSE;
    stream_info_buf = MCT_STREAM_STREAMINFO(stream);
    if (!stream_info_buf) {
      CDBG_ERROR("%s: Stream Info buffer is missing \n", __func__);
      break;
    }

    event_data.type = MCT_EVENT_CONTROL_PARM_STREAM_BUF;
    event_data.control_event_data = (void *)&stream->streaminfo.parm_buf;

    stream->streaminfo.parm_buf = stream_info_buf->parm_buf;
    cmd_event = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
      (pack_identity(MCT_PIPELINE_SESSION(pipeline), stream->streamid)),
      MCT_EVENT_DOWNSTREAM, &event_data);
    if (pipeline->send_event)
      ret = pipeline->send_event(pipeline, data->stream_id, &cmd_event);
    if (TRUE == ret)
      stream_info_buf->parm_buf.reprocess.ret_val =
          stream->streaminfo.parm_buf.reprocess.ret_val;
  }
    break;

  case CAM_PRIV_PREPARE_SNAPSHOT:
  case CAM_PRIV_START_ZSL_SNAPSHOT:
  case CAM_PRIV_STOP_ZSL_SNAPSHOT:
  case CAM_PRIV_DO_AUTO_FOCUS:
  case CAM_PRIV_CANCEL_AUTO_FOCUS: {
    mct_event_t cmd_event;
    mct_event_control_t event_data;

    if (data->command == CAM_PRIV_DO_AUTO_FOCUS)
      event_data.type = MCT_EVENT_CONTROL_DO_AF;
    else if (data->command == CAM_PRIV_CANCEL_AUTO_FOCUS)
      event_data.type = MCT_EVENT_CONTROL_CANCEL_AF;
    else if (data->command == CAM_PRIV_PREPARE_SNAPSHOT)
      event_data.type = MCT_EVENT_CONTROL_PREPARE_SNAPSHOT;
    else if (data->command == CAM_PRIV_START_ZSL_SNAPSHOT)
      event_data.type = MCT_EVENT_CONTROL_START_ZSL_SNAPSHOT;
    else if (data->command == CAM_PRIV_STOP_ZSL_SNAPSHOT)
      event_data.type = MCT_EVENT_CONTROL_STOP_ZSL_SNAPSHOT;

    event_data.control_event_data = &(data->arg_value);
    cmd_event = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
      (pack_identity(MCT_PIPELINE_SESSION(pipeline), stream->streamid)),
      MCT_EVENT_DOWNSTREAM, &event_data);
    if (pipeline->send_event)
      ret = pipeline->send_event(pipeline, stream->streamid, &cmd_event);
    else
      ret = FALSE;

    data->ret_value = *((uint32_t *)event_data.control_event_data);
  }
    break;

  default:
    ret = TRUE;
    break;
  }

  return ret;
}

/** mct_pipeline_process_get:
 *    @
 *    @
 *
 **/
static boolean mct_pipeline_process_get(struct msm_v4l2_event_data *data,
  mct_pipeline_t *pipeline)
{
  boolean      ret = TRUE;
  mct_stream_t *stream = NULL;
  mct_pipeline_get_stream_info_t info;

  switch (data->command) {
  case CAM_PRIV_STREAM_PARM: {
    info.check_type   = CHECK_INDEX;
    info.stream_index = data->stream_id;
    stream = mct_pipeline_get_stream(pipeline, &info);
    if (!stream) {
      CDBG_ERROR("%s: Couldn't find stream\n", __func__);
      return FALSE;
    }
  }
    break;

  case CAM_PRIV_PARM: {
    /* This case could potentially hit even before a stream exists */
    if (MCT_PIPELINE_NUM_CHILDREN(pipeline) > 0) {
      info.check_type   = CHECK_TYPE;
      info.stream_type  = CAM_STREAM_TYPE_PREVIEW;
      stream = mct_pipeline_get_stream(pipeline, &info);
      if (!stream) {
        info.check_type   = CHECK_TYPE;
        info.stream_type  = CAM_STREAM_TYPE_RAW; /*RDI streaming*/
        stream = mct_pipeline_get_stream(pipeline, &info);
        if (!stream) {
          CDBG_ERROR("%s: Couldn't find preview stream; Storing for later\n",
            __func__);
        }
      }
    }
  }
    break;

  default:
    break;
  }

  switch (data->command) {
  case MSM_CAMERA_PRIV_QUERY_CAP: {
    /* for queryBuf */
    if (!pipeline->query_buf || !pipeline->modules) {
      ret = FALSE;
    } else {
      memset(&pipeline->query_data, 0, sizeof(mct_pipeline_cap_t));
      mct_list_traverse(pipeline->modules, mct_pipeline_query_modules,
        pipeline);
    }
    if (!pipeline->query_buf || !(&pipeline->query_data)) {
      CDBG_ERROR("%s: Error query_data is NULL\n",__func__);
      return FALSE;
    }
    /*now fill up HAL's query buffer*/
    ret = mct_pipeline_populate_query_cap_buffer(pipeline);
  }
    break;

  case CAM_PRIV_PARM: {
    /*start reading the parm buf from HAL*/
    parm_buffer_new_t *p_table = (parm_buffer_new_t *)pipeline->config_parm;
    CDBG("%s: num params in set_param: %d ", __func__, p_table->num_entry);

    if (stream) {
      /*Send event only if a stream exists*/
      ret = mct_pipeline_send_ctrl_events(pipeline, stream->streamid,
            MCT_EVENT_CONTROL_GET_PARM);
    } else {
      ret = TRUE;
    }
  }
  break;

  case CAM_PRIV_STREAM_PARM: {
    mct_event_t cmd_event;
    mct_event_control_t event_data;
    cam_stream_info_t *stream_info_buf;

    ret = FALSE;
    stream_info_buf = MCT_STREAM_STREAMINFO(stream);
    if (!stream_info_buf) {
      CDBG_ERROR("%s: Stream Info buffer is missing \n", __func__);
      break;
    }

    event_data.type = MCT_EVENT_CONTROL_PARM_STREAM_BUF;
    event_data.control_event_data = (void *)&stream->streaminfo.parm_buf;

    stream->streaminfo.parm_buf = stream_info_buf->parm_buf;
    cmd_event = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
      (pack_identity(MCT_PIPELINE_SESSION(pipeline), stream->streamid)),
      MCT_EVENT_DOWNSTREAM, &event_data);
    if (pipeline->send_event)
      ret = pipeline->send_event(pipeline, data->stream_id, &cmd_event);
    if (TRUE == ret)
      stream_info_buf->parm_buf =
          stream->streaminfo.parm_buf;
  }
    break;

  default:
    ret = FALSE;
    break;
  }

  return ret;
}

/** mct_pipeline_process_serv_msg:
 *    @
 *    @
 *
 **/
static boolean mct_pipeline_process_serv_msg(void *message,
  mct_pipeline_t *pipeline)
{
  struct v4l2_event *msg = (struct v4l2_event *)message;
  mct_stream_t      *stream = NULL;
  boolean           ret = TRUE;
  struct msm_v4l2_event_data *data =
    (struct msm_v4l2_event_data *)(msg->u.data);

  if (!message || !pipeline || data->session_id != pipeline->session)
    return FALSE;

  switch (msg->id) {
  case MSM_CAMERA_SET_PARM:
    ret = mct_pipeline_process_set(data, pipeline);
    break;

  case MSM_CAMERA_GET_PARM:
    /* process config_w  */
    ret = mct_pipeline_process_get(data, pipeline);
    break;

  case MSM_CAMERA_DEL_SESSION: {
    /* for session ending:
     * a session has ONLY one child */
    if (MCT_PIPELINE_CHILDREN(pipeline)) {
      MCT_OBJECT_LOCK(pipeline);
      /* Delete streams */
      mct_list_free_all(MCT_PIPELINE_CHILDREN(pipeline),
        mct_pipeline_delete_stream);

      MCT_PIPELINE_CHILDREN(pipeline) = NULL;
      MCT_PIPELINE_NUM_CHILDREN(pipeline) = 0;
      MCT_OBJECT_UNLOCK(pipeline);
    }
  }
    break;

  default:
    /* something wrong */
    ret = FALSE;
    break;
  } /* switch (msg->msg_type) */

  return ret;
}

/** mct_pipeline_process_bus_msg:
 *    @
 *    @
 *
 **/
static boolean mct_pipeline_process_bus_msg(void *msg,
  mct_pipeline_t *pipeline)

{
  boolean ret = TRUE;
  mct_bus_msg_t *bus_msg = (mct_bus_msg_t *)msg;
  mct_stream_t *stream = NULL;
  mct_pipeline_get_stream_info_t info;
  mct_event_t cmd_event;
  mct_event_control_t event_data;

  info.check_type   = CHECK_TYPE;
  info.stream_type  = CAM_STREAM_TYPE_METADATA;
  stream = mct_pipeline_get_stream(pipeline, &info);
  if (stream) {
    ret = mct_stream_metadata_bus_msg(stream, bus_msg);
  } else {
    CDBG("%s: Can't find stream", __func__);
  }

  return ret;
}

/** mct_pipeline_init_mutex:
 *    @
 *
 **/
static void mct_pipeline_init_mutex(pthread_mutex_t *mutex)
{
  pthread_mutexattr_t attr;

  pthread_mutexattr_init(&attr);

  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(mutex, &attr);
  pthread_mutexattr_destroy(&attr);
}

/** mct_pipeline_get_module_num:
 *    @data1: void* pointer to the module being processed
 *    @data2: void* pointer to the pipeline object
 *
 *  Traverse module list to get the module number.
 *
 *  Return: Ture.
 **/
static boolean mct_pipeline_get_module_num(void *data1, void *data2)
{
  mct_pipeline_t *pipeline = (mct_pipeline_t *)data2;
  pipeline->thread_data.modules_num++;

  return TRUE;
}

/** mct_pipeline_sync_init:
 *    @sync_data: mct_sync_data_t*
 *
 **/
int mct_pipeline_sync_init(mct_sync_data_t *sync_data)
{
  int rc = 0;
  pthread_mutex_init(&sync_data->mutex, NULL);
  rc = pthread_cond_init(&sync_data->cond, NULL);
  sync_data->cond_posted = 0;
  return rc;
}

/** mct_pipeline_sync_post:
 *    @sync_data: mct_sync_data_t*
 *
 **/
int mct_pipeline_sync_post(mct_sync_data_t *sync_data)
{
  int rc = 0;
  pthread_mutex_lock(&sync_data->mutex);
  sync_data->cond_posted++;
  rc = pthread_cond_signal(&sync_data->cond);
  pthread_mutex_unlock(&sync_data->mutex);
  return rc;
}

/** mct_pipeline_sync_pend:
 *    @sync_data: mct_sync_data_t*
 *
 **/
int mct_pipeline_sync_pend(mct_sync_data_t *sync_data)
{
  int rc = 0;
  struct timespec timeToWait;

  pthread_mutex_lock(&sync_data->mutex);
  if (0 == sync_data->cond_posted) {
    clock_gettime(CLOCK_REALTIME, &timeToWait);
    timeToWait.tv_sec  += 1;
    pthread_cond_timedwait(&sync_data->cond, &sync_data->mutex,
      &timeToWait);
    if (ETIMEDOUT == rc)
      CDBG_ERROR("%s: ETIMEDOUT pthread_cond_timedwait \n", __func__);
  }
  sync_data->cond_posted--;
  pthread_mutex_unlock(&sync_data->mutex);
  return rc;
}

/** mct_pipeline_sync_destroy:
 *    @sync_data: mct_sync_data_t*
 *
 **/
int mct_pipeline_sync_destroy(mct_sync_data_t *sync_data)
{
  int rc = 0;
  rc = pthread_mutex_destroy(&sync_data->mutex);
  rc |= pthread_cond_destroy(&sync_data->cond);
  sync_data->cond_posted = 0;
  return rc;
}

/** mct_pipeline_stop_session_thread:
 *    @data: void* pointer to the mct_pipeline_thread_data
 *
 *  Thread implementation to stop camera module.
 *
 *  Return: NULL
 **/
static void* mct_pipeline_stop_session_thread(void *data)
{
  mct_pipeline_thread_data_t *thread_data = (mct_pipeline_thread_data_t*)data;
  mct_module_t *module = thread_data->module;
  unsigned int session_id = thread_data->session_id;
  boolean rc = TRUE;

  CDBG_HIGH("%s thread_id is %d\n",__func__, syscall(SYS_gettid));
  prctl(PR_SET_NAME, "mct_pipe_stop", 0, 0, 0);

  pthread_mutex_lock(&thread_data->mutex);
  pthread_cond_signal(&thread_data->cond_v);
  pthread_mutex_unlock(&thread_data->mutex);

  CDBG_HIGH("%s: Stop module name: %s - E", __func__, MCT_MODULE_NAME(module));
  if (module->stop_session) {
    rc = module->stop_session(module, session_id);
  }
  if(rc == FALSE) {
    ALOGE("%s: Failed to stop module name: %s", __func__, MCT_MODULE_NAME(module));
  }
  CDBG_HIGH("%s: Stop module name: %s - X", __func__, MCT_MODULE_NAME(module));

  rc = mct_pipeline_sync_post(&thread_data->sync);
  if (rc) {
    ALOGE("%s: Failed to post condition!", __func__);
  }

  return NULL;
}

/** mct_pipeline_modules_stop:
 *    @data1: void* pointer to the module being processed
 *    @data2: void* pointer to the pipeline object
 *
 *  Create thread for each module for stop session.
 *
 *  Return: True on success
 **/
static boolean mct_pipeline_modules_stop(void *data1, void *data2)
{
  int rc = 0;
  pthread_attr_t attr;
  mct_pipeline_t *pipeline = (mct_pipeline_t *)data2;
  mct_pipeline_thread_data_t *thread_data = &(pipeline->thread_data);
  thread_data->module = (mct_module_t *)data1;
  thread_data->session_id = pipeline->session;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_mutex_lock(&thread_data->mutex);
  rc = pthread_create(&pipeline->thread_data.pid, &attr,
    &mct_pipeline_stop_session_thread, (void *)thread_data);
  if(!rc) {
    pthread_cond_wait(&thread_data->cond_v, &thread_data->mutex);
  }
  pthread_mutex_unlock(&thread_data->mutex);
  reset_fd_dump();

  return TRUE;
}

/** mct_pipeline_start_session_thread:
 *    @data: void* pointer to the mct_pipeline_thread_data
 *
 *  Thread implementation to start camera module.
 *
 *  Return: NULL
 **/
static void* mct_pipeline_start_session_thread(void *data)
{
  mct_pipeline_thread_data_t *thread_data = (mct_pipeline_thread_data_t*)data;
  mct_module_t *module = thread_data->module;
  unsigned int session_id = thread_data->session_id;
  boolean ret;
  CDBG_HIGH("%s thread_id is %d\n",__func__, syscall(SYS_gettid));
  prctl(PR_SET_NAME, "mct_pipe_start", 0, 0, 0);
  unsigned int rc = 0;

  pthread_mutex_lock(&thread_data->mutex);
  pthread_cond_signal(&thread_data->cond_v);
  pthread_mutex_unlock(&thread_data->mutex);
  if (module->start_session) {
    rc = module->start_session(module, session_id);
  }
  pthread_mutex_lock(&thread_data->mutex);
  thread_data->started_num++;
  if (rc == TRUE)
    thread_data->started_num_success++;
  if(thread_data->started_num == thread_data->modules_num)
    pthread_cond_signal(&thread_data->cond_v);
  pthread_mutex_unlock(&thread_data->mutex);
  return NULL;
}
/** mct_pipeline_modules_start:
 *    @data1: void* pointer to the module being processed
 *    @data2: void* pointer to the pipeline object
 *
 *  Create thread for each module for start session.
 *
 *  Return: True on success
 **/
static boolean mct_pipeline_modules_start(void *data1, void *data2)
{
  int rc = 0;
  pthread_attr_t attr;
  mct_pipeline_t *pipeline = (mct_pipeline_t *)data2;
  mct_pipeline_thread_data_t *thread_data = &(pipeline->thread_data);
  thread_data->module = (mct_module_t *)data1;
  thread_data->session_id = pipeline->session;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_mutex_lock(&thread_data->mutex);
  rc = pthread_create(&pipeline->thread_data.pid, &attr,
    &mct_pipeline_start_session_thread, (void *)thread_data);
  if(!rc) {
    pthread_cond_wait(&thread_data->cond_v, &thread_data->mutex);
  }
  pthread_mutex_unlock(&thread_data->mutex);

  return TRUE;
}

/** mct_pipeline_stop_session:
 *    @pipeline: mct_pipeline_t object
 *
 *  Pipeline stop session, stop each camera module.
 *
 *  Return: no return value
 **/
void mct_pipeline_stop_session(mct_pipeline_t *pipeline)
{
 struct timespec timeToWait;
 int rc = 0;

 //stop tuning server
  mct_stop_tuning_server(pipeline);

  CDBG_HIGH("%s - E", __func__);
  if(mct_pipeline_sync_init(&pipeline->thread_data.sync)) {
    ALOGE("%s: Error on creating sync data!", __func__);
    return;
  }
  mct_list_traverse(pipeline->modules, mct_pipeline_modules_stop,
    pipeline);
  while (pipeline->thread_data.started_num) {
    if (mct_pipeline_sync_pend(&pipeline->thread_data.sync)) {
      ALOGE("%s: Condition pend timeout error - left modules: %d", __func__,
        pipeline->thread_data.started_num);
      break;
    }
    pipeline->thread_data.started_num--;
    ALOGE("%s: left modules: %d", __func__, pipeline->thread_data.started_num);

  }
  pipeline->max_pipeline_frame_delay = 0;
  pthread_mutex_destroy(&pipeline->thread_data.mutex);
  pthread_cond_destroy(&pipeline->thread_data.cond_v);
  mct_pipeline_sync_destroy(&pipeline->thread_data.sync);
  CDBG_HIGH("%s - X", __func__);
  return;
}

/** mct_pipeline_start_session:
 *    @pipeline: mct_pipeline_t object
 *
 *  Pipeline start session, start each camera module and query the
 *  module capacity.
 *
 *  Return: no return value
 **/
boolean mct_pipeline_start_session(mct_pipeline_t *pipeline)
{
  boolean rc;
  pthread_mutex_init(&pipeline->thread_data.mutex, NULL);
  pthread_cond_init(&pipeline->thread_data.cond_v, NULL);

  pipeline->thread_data.started_num = 0;
  pipeline->thread_data.modules_num = 0;
  pipeline->thread_data.started_num_success = 0;
  rc = mct_list_traverse(pipeline->modules, mct_pipeline_get_module_num,
    pipeline);
  rc &= mct_list_traverse(pipeline->modules, mct_pipeline_modules_start,
    pipeline);
  pthread_mutex_lock(&pipeline->thread_data.mutex);
  if (pipeline->thread_data.started_num!=pipeline->thread_data.modules_num)
    pthread_cond_wait(&pipeline->thread_data.cond_v, &pipeline->thread_data.mutex);
  pthread_mutex_unlock(&pipeline->thread_data.mutex);

  if (pipeline->thread_data.started_num_success ==
    pipeline->thread_data.modules_num) {
    rc &= TRUE;
  } else {
    rc &= FALSE;
  }
  rc &= mct_list_traverse(pipeline->modules, mct_pipeline_query_modules,
    pipeline);
#if 0
    pipeline->max_pipeline_frame_delay =
      MTYPE_MAX(pipeline->max_pipeline_frame_delay,
      pipeline->query_data.sensor_cap.max_frame_delay);
    pipeline->max_pipeline_frame_delay =
      MTYPE_MAX(pipeline->max_pipeline_frame_delay,
      pipeline->query_data.isp_cap.max_frame_delay);
    pipeline->max_pipeline_frame_delay =
      MTYPE_MAX(pipeline->max_pipeline_frame_delay,
      pipeline->query_data.stats_cap.max_frame_delay);
    pipeline->max_pipeline_frame_delay =
      MTYPE_MAX(pipeline->max_pipeline_frame_delay,
      pipeline->query_data.pp_cap.max_frame_delay);
    pipeline->max_pipeline_frame_delay =
      MTYPE_MAX(pipeline->max_pipeline_frame_delay,
      pipeline->query_data.imaging_cap.max_frame_delay);

#else
    pipeline->max_pipeline_frame_delay = 2;
#endif
    /*Call list traverse to set this number to all modules*/

    //start tuning server
    mct_start_tuning_server(pipeline);
  return rc;
}

/** mct_pipeline_new:
 *
 **/
mct_pipeline_t* mct_pipeline_new(void)
{
  mct_pipeline_t *pipeline;

  pipeline = malloc(sizeof(mct_pipeline_t));
  if (!pipeline)
    return NULL;
  memset(pipeline, 0, sizeof(mct_pipeline_t));

  pthread_mutex_init(MCT_OBJECT_GET_LOCK(pipeline), NULL);
  mct_pipeline_init_mutex(MCT_OBJECT_GET_LOCK(pipeline));


  /* FIXME: Other initilization */
  pipeline->config_parm         = NULL;
  pipeline->config_parm_size    = 0;
  pipeline->query_buf        = NULL;
  pipeline->query_buf_size   = 0;

  pipeline->pending_set_parm = malloc (ONE_MB_OF_PARAMS);
  pipeline->pending_get_parm = malloc (ONE_MB_OF_PARAMS);
  if (!pipeline->pending_set_parm || !pipeline->pending_get_parm) {
    if (pipeline->pending_set_parm)
      free(pipeline->pending_set_parm);
    if (pipeline->pending_get_parm)
      free(pipeline->pending_get_parm);
    free(pipeline);
    CDBG_ERROR("%s:mct_pipeline_new failed ",__func__);
    return NULL;
  }
  memset(pipeline->pending_set_parm, 0, ONE_MB_OF_PARAMS);
  memset(pipeline->pending_get_parm, 0, ONE_MB_OF_PARAMS);
  pipeline->pending_set_parm->tot_rem_size =
    ONE_MB_OF_PARAMS - sizeof(parm_buffer_new_t);
  pipeline->pending_get_parm->tot_rem_size =
    ONE_MB_OF_PARAMS - sizeof(parm_buffer_new_t);

  /* For SERV_MSG_BUF_MAPPING */
  pipeline->map_buf   = mct_pipeline_map_buf;

  /*For SERV_MSG_BUF_UNMAPPING */
  pipeline->unmap_buf = mct_pipeline_unmap_buf;

  /* For case SERV_MSG_SET,SERV_MSG_GET, SERV_MSG_STREAMON, SERV_MSG_STREAMOFF,
    SERV_MSG_QUERY,SERV_MSG_CLOSE_SESSION */
  pipeline->process_serv_msg= mct_pipeline_process_serv_msg;
  pipeline->process_bus_msg = mct_pipeline_process_bus_msg;

  pipeline->add_stream    = mct_pipeline_add_stream;
  pipeline->remove_stream = mct_pipeline_remove_stream;
  pipeline->send_event    = mct_pipeline_send_event;
  pipeline->set_bus       = mct_pipeline_set_bus;
  pipeline->get_bus       = mct_pipeline_get_bus;
  pipeline->hal_version   = CAM_HAL_V1;

  return pipeline;
}

/** mct_pipeline_destroy:
 *    @
 *
 **/
void mct_pipeline_destroy(mct_pipeline_t *pipeline)
{
  MCT_OBJECT_LOCK(pipeline);
  if (MCT_PIPELINE_CHILDREN(pipeline))
    mct_list_free_all(MCT_PIPELINE_CHILDREN(pipeline),
      mct_pipeline_delete_stream);

  MCT_PIPELINE_CHILDREN(pipeline) = NULL;
  MCT_PIPELINE_NUM_CHILDREN(pipeline) = 0;
  pthread_mutex_destroy(MCT_OBJECT_GET_LOCK(pipeline));

  /*unmap buffers if they still exist*/
  if (pipeline->config_parm) {
    munmap(pipeline->config_parm, pipeline->config_parm_size);
    pipeline->config_parm = NULL;
    pipeline->config_parm_size = 0;
  }
  if (pipeline->query_buf) {
    munmap(pipeline->query_buf, pipeline->query_buf_size);
    pipeline->query_buf = NULL;
    pipeline->query_buf_size = 0;
  }

  free(pipeline->pending_set_parm);
  free(pipeline->pending_get_parm);
  MCT_OBJECT_UNLOCK(pipeline);
  free(pipeline);
  pipeline = NULL;
  return;
}

void mct_pipeline_add_stream_to_linked_streams(mct_pipeline_t *pipeline,
  mct_stream_t *stream)
{
  cam_stream_type_t stream_type =
    ((cam_stream_info_t *)(MCT_STREAM_STREAMINFO(stream)))->stream_type;
    pipeline->linked_streams |= (1 << stream_type);

    CDBG_HIGH("%s: linked streams: 0x%x", __func__, pipeline->linked_streams);
}

void mct_pipeline_remove_stream_from_linked_streams(mct_pipeline_t *pipeline,
  mct_stream_t *stream)
{
  cam_stream_type_t stream_type;
  mct_stream_info_t * stream_info;

  if (NULL == pipeline) {
    CDBG_ERROR("%s: ERROR - no pipeline", __func__);
    return;
  }
  if (!stream) {
    CDBG_ERROR("%s: stream is NULL", __func__);
    return;
  }
  stream_info = &stream->streaminfo;
  if (!stream_info) {
    CDBG_ERROR("%s: no stream info", __func__);
    return;
  }
  stream_type = stream_info->stream_type;

  CDBG("%s: stream type: %d", __func__, stream_type);

  pipeline->linked_streams &= ~(1 << stream_type);

  CDBG("%s: linked streams: 0x%x", __func__, pipeline->linked_streams);
}
