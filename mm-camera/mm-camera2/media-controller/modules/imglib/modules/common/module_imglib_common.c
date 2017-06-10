/***************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
***************************************************************************/

#include "module_imglib_common.h"
#include <media/msmb_generic_buf_mgr.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include "server_debug.h"

/** MOD_IMGLIB_ZOOM_DENUMINATOR
 *
 * Macro to set zoom denuminator
 **/
#define MOD_IMGLIB_ZOOM_DENUMINATOR 100

/** MOD_IMGLIB_DUMP_FRAME
 *
 * Macro to enable dump frame functionality
 **/
#define MOD_IMGLIB_DUMP_FRAME

/** module_imglib_get_next_from_list
 *    @data1: not used
 *    @data2: not used
 *
 *  Gets next element from the list
 *
 *  Return TRUE always
 **/
boolean module_imglib_get_next_from_list(void *data1, void *data2)
{
  return TRUE;
}

/**
 * Function: module_imglib_msg_thread_can_wait
 *
 * Description: Queue function to check if abort is issued
 *
 * Input parameters:
 *   p_userdata - The pointer to message thread
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
int module_imglib_msg_thread_can_wait(void *p_userdata)
{
  mod_imglib_msg_th_t *p_msg_th = (mod_imglib_msg_th_t *)p_userdata;
  return (FALSE == p_msg_th->abort_flag);
}

/**
 * Function: module_imglib_msg_thread
 *
 * Description: Main message thread loop
 *
 * Input parameters:
 *   data - The pointer to message thread
 *
 * Return values:
 *     NULL
 *
 * Notes: none
 **/
void *module_imglib_msg_thread(void *data)
{
  mod_imglib_msg_th_t *p_msg_th = (mod_imglib_msg_th_t *)data;
  img_queue_t *p_q = &p_msg_th->msg_q;
  mod_img_msg_t *p_msg;
  mct_event_t mct_event;
  isp_buf_divert_ack_t buff_divert_ack;

  IDBG_HIGH("%s thread_id is %d\n",__func__, syscall(SYS_gettid));
  prctl(PR_SET_NAME, "imglib_thread", 0, 0, 0);
  /* signal the main thread */
  pthread_mutex_lock(&p_q->mutex);
  p_msg_th->is_ready = TRUE;
  pthread_cond_signal(&p_q->cond);
  pthread_mutex_unlock(&p_q->mutex);

  IDBG_MED("%s:%d] abort %d", __func__, __LINE__,
    p_msg_th->abort_flag);

  while ((p_msg = img_q_wait(&p_msg_th->msg_q,
    module_imglib_msg_thread_can_wait, p_msg_th)) != NULL) {

    switch (p_msg->type) {
    case MOD_IMG_MSG_BUF_ACK: {
      IDBG_MED("%s:%d] Send buffer divert Ack buf_id %d identity %x",
        __func__, __LINE__,
        p_msg->data.buf_ack.frame_id,
        p_msg->data.buf_ack.identity);
      memset(&mct_event,  0,  sizeof(mct_event));
      mct_event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT_ACK;
      mct_event.u.module_event.module_event_data = (void *)&buff_divert_ack;
      mct_event.type = MCT_EVENT_MODULE_EVENT;
      mct_event.identity = p_msg->data.buf_ack.identity;
      mct_event.direction = MCT_EVENT_UPSTREAM;
      memset(&buff_divert_ack,  0,  sizeof(buff_divert_ack));
      buff_divert_ack.buf_idx = p_msg->data.buf_ack.frame_id;
      buff_divert_ack.is_buf_dirty = 0;
      mct_port_send_event_to_peer(p_msg->port, &mct_event);
      break;
    }
    case MOD_IMG_MSG_DIVERT_BUF: {
      IDBG_MED("%s:%d] Buffer divert event buf_id %d identity %x p_exec %p",
        __func__, __LINE__,
        p_msg->data.buf_divert.buf_divert.buffer.index,
        p_msg->data.buf_divert.identity,
        p_msg->data.buf_divert.p_exec);
      if (p_msg->data.buf_divert.p_exec) {
        p_msg->data.buf_divert.p_exec(p_msg->data.buf_divert.userdata,
          &p_msg->data.buf_divert);
      }
      break;
    }
    case MOD_IMG_MSG_EXEC_INFO: {
      if (p_msg->data.exec_info.p_exec) {
        p_msg->data.exec_info.p_exec(p_msg->data.exec_info.p_userdata,
          p_msg->data.exec_info.data);
      }
      break;
    }
    default:;
    }
    free(p_msg);
  }
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return NULL;
}

/**
 * Function: module_imglib_destroy_msg_thread
 *
 * Description: This method is used to destroy the message
 *             thread
 *
 * Input parameters:
 *   p_msg_th - The pointer to message thread
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int module_imglib_destroy_msg_thread(mod_imglib_msg_th_t *p_msg_th)
{
  img_queue_t *p_q = &p_msg_th->msg_q;

  /* signal the main thread */
  pthread_mutex_lock(&p_q->mutex);
  p_msg_th->abort_flag = TRUE;
  pthread_cond_signal(&p_q->cond);
  pthread_mutex_unlock(&p_q->mutex);

  IDBG_MED("%s:%d] threadid %d", __func__, __LINE__,
    (int)p_msg_th->threadid);
  if (!pthread_equal(p_msg_th->threadid, pthread_self())) {
    pthread_join(p_msg_th->threadid, NULL);
  }
  IDBG_MED("%s:%d] after msg thread exit", __func__, __LINE__);

  /* delete the message queue */
  img_q_deinit(p_q);

  return IMG_SUCCESS;
}

/**
 * Function: module_imglib_send_msg
 *
 * Description: This method is used to send message to the
 *             message thread
 *
 * Input parameters:
 *   p_msg_th - The pointer to message thread
 *   p_msg - pointer to the message
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int module_imglib_send_msg(mod_imglib_msg_th_t *p_msg_th,
  mod_img_msg_t *p_msg)
{
  int status = IMG_SUCCESS;

  mod_img_msg_t *p_msg_new;

  IDBG_MED("%s:%d] E", __func__, __LINE__);
  /* create message */
  p_msg_new = (mod_img_msg_t *)malloc(sizeof(mod_img_msg_t));
  if (NULL == p_msg_new) {
    IDBG_ERROR("%s:%d] cannot create message", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }
  memcpy(p_msg_new, p_msg, sizeof(mod_img_msg_t));

  status = img_q_enqueue(&p_msg_th->msg_q, p_msg_new);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] cannot enqueue message", __func__, __LINE__);
    free(p_msg_new);
    return status;
  }
  img_q_signal(&p_msg_th->msg_q);
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return status;
}

/**
 * Function: module_imglib_create_msg_thread
 *
 * Description: This method is used to create the
 *             message thread
 *
 * Input parameters:
 *   p_msg_th - The pointer to message thread
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int module_imglib_create_msg_thread(mod_imglib_msg_th_t *p_msg_th)
{
  img_queue_t *p_q;
  int status = IMG_SUCCESS;

  IDBG_MED("%s:%d] E", __func__, __LINE__);
  memset(p_msg_th, 0x0, sizeof(mod_imglib_msg_th_t));

  /* initialize the queue */
  p_q = &p_msg_th->msg_q;
  p_msg_th->threadid = 0;
  p_msg_th->abort_flag = FALSE;
  img_q_init(p_q, "message_q");

  /* start the message thread */
  pthread_mutex_lock(&p_q->mutex);
  p_msg_th->is_ready = FALSE;
  status = pthread_create(&p_msg_th->threadid, NULL,
    module_imglib_msg_thread,
    (void *)p_msg_th);
  pthread_setname_np(p_msg_th->threadid, "CAM_img_msg");
  if (status < 0) {
    IDBG_ERROR("%s:%d] pthread creation failed %d",
      __func__, __LINE__, status);
    pthread_mutex_unlock(&p_q->mutex);
    return IMG_ERR_GENERAL;
  }
  pthread_mutex_unlock(&p_q->mutex);
  IDBG_MED("%s:%d] X", __func__, __LINE__);

  return status;
}

/**
 * Function: mod_imglib_map_fd_buffer
 *
 * Description: This method is used for updating the imglib
 * buffer structure from MCT structure with face detection buffers
 *
 * Input parameters:
 *   @data - MCT stream buffer mapping
 *   @user_data - img buffer structure
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean mod_imglib_map_fd_buffer(void *data, void *user_data)
{
  boolean rc = FALSE;
  mct_stream_map_buf_t *p_buf = (mct_stream_map_buf_t *)data;
  mod_img_buffer_info_t *p_buf_info = (mod_img_buffer_info_t *)user_data;
  int idx = 0;

  IDBG_MED("%s:%d] p_buf %p p_buf_info %p", __func__, __LINE__, p_buf,
    p_buf_info);
  if (!p_buf || !p_buf_info) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return TRUE;
  }

  /* For face detection is used stream buff type */
  if (p_buf->buf_type != CAM_MAPPING_BUF_TYPE_STREAM_BUF)
    return TRUE;

  /* Check the buffer count */
  if (p_buf_info->buf_count >= p_buf_info->total_count)
    return TRUE;

  idx = p_buf_info->buf_count;
  p_buf_info->p_buffer[idx].map_buf = *p_buf;
  IDBG_MED("%s:%d] buffer cnt %d idx %d addr %p", __func__, __LINE__,
    p_buf_info->buf_count, p_buf->buf_index,
    p_buf->buf_planes[0].buf);
  p_buf_info->buf_count++;
  return TRUE;
}

/**
 * Function: mod_imglib_map_fr_buffer
 *
 * Description: This method is used for updating the imglib
 * buffer structure from MCT structure with buffer for face regsitration
 *
 * Input parameters:
 *   @data - MCT stream buffer mapping
 *   @user_data - img buffer structure
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean mod_imglib_map_fr_buffer(void *data, void *user_data)
{
  boolean rc = FALSE;
  mct_stream_map_buf_t *p_buf = (mct_stream_map_buf_t *)data;
  mod_img_buffer_info_t *p_buf_info = (mod_img_buffer_info_t *)user_data;
  int idx = 0;

  IDBG_MED("%s:%d] p_buf %p p_buf_info %p", __func__, __LINE__, p_buf,
    p_buf_info);
  if (!p_buf || !p_buf_info) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return TRUE;
  }

  /* For face registration is used offline input buffer sent from hal */
  if (p_buf->buf_type != CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF)
    return TRUE;

  idx = p_buf_info->buf_count;
  p_buf_info->p_buffer[idx].map_buf = *p_buf;
  IDBG_MED("%s:%d] buffer cnt %d idx %d addr %p", __func__, __LINE__,
    p_buf_info->buf_count, p_buf->buf_index,
    p_buf->buf_planes[0].buf);
  p_buf_info->buf_count++;
  return TRUE;
}
/**
 * Function: mod_imglib_convert_buffer
 *
 * Description: This method is used to convert the ISP buffer
 *        to frame buffer
 *
 * Input parameters:
 *   @data - MCT stream buffer mapping
 *   @user_data - img buffer structure
 *
 * Return values:
 *     true/false
 *
 * Notes: incomplete
 **/
int mod_imglib_convert_buffer(isp_buf_divert_t *p_isp_buf,
  img_frame_t *p_frame)
{
  int status = IMG_SUCCESS;
  IDBG_MED("%s:%d] %d", __func__, __LINE__, p_isp_buf->native_buf);
  if (!p_isp_buf->native_buf) {
    /* Todo: add support for native buffers */
  }
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return status;
}

/** mod_imglib_dump_stream_info
 *    @info: stream info configuration
 *
 * Prints stream info configuration
 *
 * Returns TRUE in case of success
 **/
void mod_imglib_dump_stream_info(mct_stream_info_t* info)
{
  uint32_t i;

  if (info) {
    IDBG_MED("info->stream_type %d", info->stream_type);
    IDBG_MED("info->fmt %d", info->fmt);
    IDBG_MED("info->dim.width %d", info->dim.width);
    IDBG_MED("info->dim.height %d", info->dim.height);
    IDBG_MED("info->buf_planes.plane_info.frame_len %d",
      info->buf_planes.plane_info.frame_len);
    IDBG_MED("info->buf_planes.plane_info.num_planes %d",
      info->buf_planes.plane_info.num_planes);
    IDBG_MED("info->buf_planes.plane_info.sp.len %d",
      info->buf_planes.plane_info.sp.len);
    IDBG_MED("info->buf_planes.plane_info.sp.y_offset %d",
      info->buf_planes.plane_info.sp.y_offset);
    IDBG_MED("info->buf_planes.plane_info.sp.cbcr_offset %d",
      info->buf_planes.plane_info.sp.cbcr_offset);
    for (i=0; i<info->buf_planes.plane_info.num_planes; i++) {
      IDBG_MED("info->buf_planes.plane_info.mp[%d].len %d", i,
        info->buf_planes.plane_info.mp[i].len);
      IDBG_MED("info->buf_planes.plane_info.mp[%d].offset %d", i,
        info->buf_planes.plane_info.mp[i].offset);
      IDBG_MED("info->buf_planes.plane_info.mp[%d].offset_x %d", i,
        info->buf_planes.plane_info.mp[i].offset_x);
      IDBG_MED("info->buf_planes.plane_info.mp[%d].offset_y %d", i,
        info->buf_planes.plane_info.mp[i].offset_y);
      IDBG_MED("info->buf_planes.plane_info.mp[%d].stride %d", i,
        info->buf_planes.plane_info.mp[i].stride);
      IDBG_MED("info->buf_planes.plane_info.mp[%d].scanline %d", i,
        info->buf_planes.plane_info.mp[i].scanline);
    }
    IDBG_MED("info->streaming_mode %d", info->streaming_mode);
    IDBG_MED("info->num_burst %d", info->num_burst);
    IDBG_MED("info->img_buffer_list %p", info->img_buffer_list);
    IDBG_MED("info->parm_buf.type %d", info->parm_buf.type);
    IDBG_MED("info->parm_buf.reprocess.buf_index %d",
      info->parm_buf.reprocess.buf_index);
    IDBG_MED("info->parm_buf.reprocess.ret_val %d",
      info->parm_buf.reprocess.ret_val);
    IDBG_MED("info->reprocess_config.pp_type %d",
      info->reprocess_config.pp_type);
    if (CAM_ONLINE_REPROCESS_TYPE == info->reprocess_config.pp_type)
      IDBG_MED("info->reprocess_config.online.input_stream_id %d",
        info->reprocess_config.online.input_stream_id);
    else {
      IDBG_MED("info->reprocess_config.offline.input_fmt %d",
        info->reprocess_config.offline.input_fmt);
      IDBG_MED("info->reprocess_config.offline.input_dim.width %d",
        info->reprocess_config.offline.input_dim.width);
      IDBG_MED("info->reprocess_config.offline.input_dim.height %d",
        info->reprocess_config.offline.input_dim.height);
      IDBG_MED("info->reprocess_config.offline.num_of_bufs %d",
        info->reprocess_config.offline.num_of_bufs);
      IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
        \b\b\b\b\b\b\b\bplane_info.frame_len %d",
        info->reprocess_config.offline.input_buf_planes.plane_info.frame_len);
      IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
        \b\b\b\b\b\b\b\bplane_info.num_planes %d",
        info->reprocess_config.offline.input_buf_planes.plane_info.num_planes);
      IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
        \b\b\b\b\b\b\b\bplane_info.sp.len %d",
        info->reprocess_config.offline.input_buf_planes.plane_info.sp.len);
      IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
        \b\b\b\b\b\b\b\bplane_info.sp.y_offset %d",
        info->reprocess_config.offline.input_buf_planes.\
        plane_info.sp.y_offset);
      IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
        \b\b\b\b\b\b\b\bplane_info.sp.cbcr_offset %d",
        info->reprocess_config.offline.input_buf_planes.\
        plane_info.sp.cbcr_offset);

      for (i=0;\
        i<info->reprocess_config.offline.input_buf_planes.\
        plane_info.num_planes;\
        i++) {

        IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
          \b\b\b\b\b\b\b\b\b\bplane_info.mp[%d].len %d", i,
          info->reprocess_config.offline.input_buf_planes.\
          plane_info.mp[i].len);
        IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
          \b\b\b\b\b\b\b\b\b\bplane_info.mp[%d].offset %d", i,
          info->reprocess_config.offline.input_buf_planes.\
          plane_info.mp[i].offset);
        IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
          \b\b\b\b\b\b\b\b\b\bplane_info.mp[%d].offset_x %d", i,
          info->reprocess_config.offline.input_buf_planes.\
          plane_info.mp[i].offset_x);
        IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
          \b\b\b\b\b\b\b\b\b\bplane_info.mp[%d].offset_y %d", i,
          info->reprocess_config.offline.input_buf_planes.\
          plane_info.mp[i].offset_y);
        IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
          \b\b\b\b\b\b\b\b\b\bplane_info.mp[%d].stride %d", i,
          info->reprocess_config.offline.input_buf_planes.\
          plane_info.mp[i].stride);
        IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
          \b\b\b\b\b\b\b\b\b\bplane_info.mp[%d].scanline %d", i,
          info->reprocess_config.offline.input_buf_planes.\
          plane_info.mp[i].scanline);
      }
    }
    IDBG_MED("info->reprocess_config.pp_feature_config.feature_mask %d",
      info->reprocess_config.pp_feature_config.feature_mask);
    IDBG_MED("info->reprocess_config.pp_feature_config.\
      \b\b\b\b\b\bdenoise2d.denoise_enable %d",
      info->reprocess_config.pp_feature_config.denoise2d.denoise_enable);
    IDBG_MED("info->reprocess_config.pp_feature_config.\
      \b\b\b\b\b\bdenoise2d.process_plates %d",
      info->reprocess_config.pp_feature_config.denoise2d.process_plates);
    IDBG_MED("info->reprocess_config.pp_feature_config.input_crop.left %d",
      info->reprocess_config.pp_feature_config.input_crop.left);
    IDBG_MED("info->reprocess_config.pp_feature_config.input_crop.top %d",
      info->reprocess_config.pp_feature_config.input_crop.top);
    IDBG_MED("info->reprocess_config.pp_feature_config.input_crop.width %d",
      info->reprocess_config.pp_feature_config.input_crop.width);
    IDBG_MED("info->reprocess_config.pp_feature_config.input_crop.height %d",
      info->reprocess_config.pp_feature_config.input_crop.height);
    IDBG_MED("info->reprocess_config.pp_feature_config.rotation %d",
      info->reprocess_config.pp_feature_config.rotation);
    IDBG_MED("info->reprocess_config.pp_feature_config.flip %d",
      info->reprocess_config.pp_feature_config.flip);
    IDBG_MED("info->reprocess_config.pp_feature_config.sharpness %d",
      info->reprocess_config.pp_feature_config.sharpness);
  }
}

/** mod_imglib_dump_frame
 *    @img_frame: frame handler
 *    @number: number to be appended at the end of the file name
 *
 * Saves specified frame to folder /data/
 *
 * Returns TRUE in case of success
 **/
boolean mod_imglib_dump_frame(img_frame_t *img_frame, char* file_name,
  uint32_t number)
{
#ifdef MOD_IMGLIB_DUMP_FRAME
  boolean ret_val = FALSE;
  int32_t i;
  uint32_t size;
  uint32_t written_size;
  int32_t out_file_fd;
  char out_file_name[80];

  if (img_frame && img_frame->frame) {
    snprintf(out_file_name, sizeof(out_file_name),
      "%s%s_%d_width_%d_height_%d_stride_%d.yuv",
      "/data/misc/camera/", file_name, number, img_frame->frame->plane[0].width,
      img_frame->frame->plane[0].height, img_frame->frame->plane[0].stride);
    out_file_fd = open(out_file_name, O_RDWR | O_CREAT, 0777);
    if (out_file_fd >= 0) {

      for (i = 0; i < img_frame->frame->plane_cnt; i++) {
        size = img_frame->frame->plane[i].stride
          * img_frame->frame->plane[i].height;
        written_size = write(out_file_fd,
          img_frame->frame->plane[i].addr + img_frame->frame->plane[i].offset,
          size);
        if (size != written_size)
          IDBG_ERROR("%s:%d failed: Cannot write data to file %s\n", __func__,
            __LINE__, out_file_name);
      }

      close(out_file_fd);

      IDBG_HIGH("%s:%d: width height stride %d %d %d",
            __func__, __LINE__,
            img_frame->frame->plane[0].width,
            img_frame->frame->plane[0].height,
            img_frame->frame->plane[0].stride);

      ret_val = TRUE;
    } else
      IDBG_ERROR("%s:%d failed: Cannot open file\n", __func__, __LINE__);
  } else
    IDBG_ERROR("%s:%d failed: Null pointer detected\n", __func__, __LINE__);

  return ret_val;
#else
  return TRUE;
#endif
}

/** mod_imglib_get_timestamp
 *  @timestamp: pointer to a char buffer. The buffer should be
 *    allocated by the caller
 *
 *  Get the current timestamp and convert it to a string
 *
 *  Return: None.
 **/
void mod_imglib_get_timestamp_string(char *timestamp) {
  time_t rawtime;
  struct tm *currenttime = NULL;
  rawtime = time(NULL);
  currenttime = localtime(&rawtime);
  if (currenttime) {
    snprintf(timestamp,
      25, "%04d%02d%02d%02d%02d%02d",
      currenttime->tm_year+1900, currenttime->tm_mon, currenttime->tm_mday,
      currenttime->tm_hour, currenttime->tm_min, currenttime->tm_sec);
  }
}

/** mod_imglib_check_stream
 *    @d1: mct_stream_t* pointer to the streanm being checked
 *    @d2: uint32_t* pointer to identity
 *
 *  Check if the stream matches stream index or stream type.
 *
 *  Return: TRUE if stream matches.
 **/
static boolean mod_imglib_check_stream(void *d1, void *d2)
{
  boolean ret_val = FALSE;
  mct_stream_t *stream = (mct_stream_t *)d1;
  uint32_t *id = (uint32_t *)d2;

  if (stream && id && stream->streaminfo.identity == *id)
    ret_val = TRUE;

  return ret_val;
}

/** mod_imglib_find_module_parent
 *    @identity: required identity
 *    @module: module, whichs parents will be serached
 *
 * Finds module parent (stream) with specified identity
 *
 * Returns Pointer to stream handler in case of cucess
 *   or NULL in case of failure
 **/
mct_stream_t* mod_imglib_find_module_parent(uint32_t identity,
  mct_module_t* module)
{
  mct_stream_t* ret_val = NULL;
  mct_list_t *find_list;

  if (module && MCT_MODULE_PARENT(module)) {
    find_list = mct_list_find_custom(MCT_MODULE_PARENT(module),
      &identity, mod_imglib_check_stream);

    if (find_list)
      ret_val = find_list->data;
  }

  return ret_val;
}

/** module_imglib_common_get_bfr_mngr_subdev:
 *  @buf_mgr_fd: buffer manager file descriptor
 *
 * Function to get buffer manager file descriptor
 *
 * Returns TRUE in case of success
 **/
int module_imglib_common_get_bfr_mngr_subdev(int *buf_mgr_fd)
{
  struct media_device_info mdev_info;
  int32_t num_media_devices = 0;
  char dev_name[32];
  char subdev_name[32];
  int32_t dev_fd = 0, ioctl_ret;
  boolean ret = FALSE;
  uint32_t i = 0;

  while (buf_mgr_fd) {
    int32_t num_entities = 1;
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (dev_fd >= MAX_FD_PER_PROCESS) {
      dump_list_of_daemon_fd();
      dev_fd = -1;
      break;
    }
    if (dev_fd < 0) {
      IDBG_ERROR("%s:%d Done enumerating media devices\n", __func__, __LINE__);
      break;
    }
    num_media_devices++;
    ioctl_ret = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (ioctl_ret < 0) {
      IDBG_ERROR("%s:%d Done enumerating media devices\n", __func__, __LINE__);
      close(dev_fd);
      break;
    }
    if (strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model) != 0)) {
      close(dev_fd);
      continue;
    }
    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities++;
      IDBG_MED("%s:%d entity id %d", __func__, __LINE__, entity.id);
      ioctl_ret = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (ioctl_ret < 0) {
        IDBG_ERROR("%s:%d Done enumerating media entities\n",
          __func__, __LINE__);
        ret = FALSE;
        break;
      }
      IDBG_MED("%s:%d entity name %s type %d group id %d\n", __func__, __LINE__,
        entity.name, entity.type, entity.group_id);

      IDBG_MED("%s:group_id=%d", __func__, entity.group_id);

      if (entity.type == MEDIA_ENT_T_V4L2_SUBDEV &&
          entity.group_id == MSM_CAMERA_SUBDEV_BUF_MNGR) {
        snprintf(subdev_name, sizeof(dev_name), "/dev/%s", entity.name);

        IDBG_MED("%s:subdev_name=%s", __func__, subdev_name);

        *buf_mgr_fd = open(subdev_name, O_RDWR);
        IDBG_MED("%s: *buf_mgr_fd=%d\n", __func__, *buf_mgr_fd);
        if ((*buf_mgr_fd) >= MAX_FD_PER_PROCESS) {
          dump_list_of_daemon_fd();
          *buf_mgr_fd = -1;
          continue;
        }
        if (*buf_mgr_fd < 0) {
          IDBG_ERROR("%s: Open subdev failed\n", __func__);
          continue;
        }
        ret = TRUE;
        IDBG_MED("%s:%d:ret=%d\n", __func__, __LINE__, ret);
        close(dev_fd);
        return ret;
      }
    }
    close(dev_fd);
  }
  IDBG_MED("%s:%d] ret=%d\n", __func__, __LINE__, ret);
  return (TRUE == ret) ? IMG_SUCCESS : IMG_ERR_GENERAL;
}

/** module_imglib_common_get_buffer:
 *  @subdev_fd: buffer mgr fd
 *  @identity: stream/session id
 *
 * Function to get buffer for denoise port
 *
 * Returns buffer index
 **/
int module_imglib_common_get_buffer(int subdev_fd, uint32_t identity)
{
  struct msm_buf_mngr_info buff;
  int32_t ret;

  IDBG_MED("%s +", __func__);

  buff.session_id = IMGLIB_SESSIONID(identity);
  buff.stream_id = IMGLIB_STREAMID(identity);
  ret = ioctl(subdev_fd, VIDIOC_MSM_BUF_MNGR_GET_BUF, &buff);
  if (ret < 0) {
    IDBG_ERROR("%s:%d] Failed to get buffer from buffer manager",
      __func__, __LINE__);
    return -1;
  }
  return buff.index;
}

/** module_imglib_common_release_buffer_idx:
 *  @subdev_fd: buffer mgr fd
 *  @identity: stream/session id
 *
 * Function to get buffer for denoise port
 *
 * Returns buffer index
 **/
int module_imglib_common_release_buffer(int subdev_fd, uint32_t identity,
  uint32_t idx, uint32_t frame_id, boolean buff_done)
{
  struct msm_buf_mngr_info buff_info;
  uint32_t cmd;
  int ret = TRUE;

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  buff_info.index = idx;
  buff_info.session_id = IMGLIB_SESSIONID(identity);
  buff_info.stream_id = IMGLIB_STREAMID(identity);
  buff_info.frame_id = frame_id;

  if (buff_done)
    cmd = VIDIOC_MSM_BUF_MNGR_BUF_DONE;
  else
    cmd = VIDIOC_MSM_BUF_MNGR_PUT_BUF;
  ret = ioctl(subdev_fd, cmd, &buff_info);

  if (ret < 0) {
    IDBG_MED("%s:%d] Failed to do buf_done id 0x%x %d %d",
      __func__, __LINE__, identity, idx, frame_id);
    ret = FALSE;
  }

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  return ret;
}

/**
 * Function: module_imglib_common_post_bus_msg
 *
 * Description: post a particular message to media bus
 *
 * Arguments:
 *   @p_mct_mod - media controller module
 *   @identity - stream identity
 *   @msg_id - bus message id
 *   @msg_data - bus message data
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_imglib_common_post_bus_msg(mct_module_t *p_mct_mod,
    unsigned int identity, mct_bus_msg_type_t msg_id, void *msg_data)
{
  mct_bus_msg_t bus_msg;
  bus_msg.type = msg_id;
  bus_msg.msg = msg_data;
  bus_msg.sessionid =  IMGLIB_SESSIONID(identity);
  IDBG_MED("%s:%d] session id %d mct_mod %p", __func__, __LINE__,
    bus_msg.sessionid, p_mct_mod);
  mct_module_post_bus_msg(p_mct_mod, &bus_msg);
}

/**
 * Function: module_imglib_common_get_zoom_ratio
 *
 * Description: This function is returning current zoom ratio
 *
 * Arguments:
 *   @p_mct_mod: mct_module
 *   @zoom_level: hal zoom level
 *
 * Return values:
 *     zoom ratio, 0 if error
 *
 * Notes: none
 **/
float module_imglib_common_get_zoom_ratio(mct_module_t *p_mct_mod,
  int zoom_level)
{
  mct_stream_t* p_stream;
  mct_pipeline_t* p_pipeline;
  mct_pipeline_isp_cap_t* p_isp_cap;

  p_stream = MCT_STREAM_CAST((MCT_MODULE_PARENT(p_mct_mod))->data);
  p_pipeline = MCT_PIPELINE_CAST((MCT_STREAM_PARENT(p_stream))->data);
  p_isp_cap = &p_pipeline->query_data.isp_cap;

  if (zoom_level > p_isp_cap->zoom_ratio_tbl_cnt) {
    return 0;
  }

  return ((float)p_isp_cap->zoom_ratio_tbl[zoom_level] /
    MOD_IMGLIB_ZOOM_DENUMINATOR);
}

/**
 * Function: module_imglib_common_get_zoom_level
 *
 * Description: This function is returning current zoom ratio
 *
 * Arguments:
 *   p_mct_cap - capababilities
 *   @zoom_ratio: zoom ratio
 *
 * Return values:
 *     zoom level, -1 if error
 *
 * Notes: none
 **/
int module_imglib_common_get_zoom_level(mct_pipeline_cap_t *p_mct_cap,
  float zoom_ratio)
{
  mct_pipeline_isp_cap_t* p_isp_cap;
  int i;
  int ret_val = -1;
  int zoom_ratio_int = (int)(zoom_ratio * MOD_IMGLIB_ZOOM_DENUMINATOR);

  p_isp_cap = &p_mct_cap->isp_cap;

  for (i=0; i<p_isp_cap->zoom_ratio_tbl_cnt; i++) {
    if (zoom_ratio_int <= p_isp_cap->zoom_ratio_tbl[i]) {
      ret_val = i;
      break;
    }
  }

  return ret_val;
}

/**
 * Function: module_imglib_common_get_meta_buff
 *
 * Description: Function used as callback to find
 *   metadata buffer wht corresponding index
 *
 * Input parameters:
 *   @data - MCT stream buffer mapping
 *   @user_data - Pinter of searched buffer index
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_imglib_common_get_meta_buff(void *data, void *user_data)
{
  mct_stream_map_buf_t *p_buf = (mct_stream_map_buf_t *)data;
  uint8_t *p_buf_index = (uint8_t *)user_data;

  if (!p_buf || !p_buf_index) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  IDBG_MED("%s:%d] buf type %d buff index %d search index %d",
      __func__, __LINE__, p_buf->buf_type, p_buf->buf_index, *p_buf_index);

  /* For face detection is used stream buff type */
  if (p_buf->buf_type != CAM_MAPPING_BUF_TYPE_OFFLINE_META_BUF)
    return FALSE;

  return ((uint8_t)p_buf->buf_index == *p_buf_index);
}

/** module_imglib_common_get_metadata_buffer:
 *  @info: Stream info
 *  @meta_index: Metadata buffer index
 *
 * Function to get metadata buffer pointer
 *
 * Returns Pointer to metadata buffer / NULL on fail
 **/
cam_metadata_info_t *module_imglib_common_get_metadata(mct_stream_info_t *info,
  uint8_t meta_index)
{
  cam_metadata_info_t *metadata_buff = NULL;
  mct_list_t *temp_list;

  if (!info) {
    IDBG_ERROR("%s:%d Invalid input %p", __func__, __LINE__, info);
    return NULL;
  }

  temp_list = mct_list_find_custom(info->img_buffer_list,
      &meta_index, module_imglib_common_get_meta_buff);
  if (temp_list && temp_list->data) {
    mct_stream_map_buf_t *buff_holder = temp_list->data;
    metadata_buff = buff_holder->buf_planes[0].buf;
  } else {
    IDBG_ERROR("%s:%d] Metadata buffer idx %d is not available",
        __func__, __LINE__, meta_index);
  }

  return metadata_buff;
}

