/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/
#include <linux/media.h>
#include "mct_module.h"
#include "module_cac.h"
#include "mct_stream.h"
#include "pthread.h"
#include "chromatix.h"
#include "mct_stream.h"

static int file_index = 0;

#define MODULE_CAC_PROPERTY_DUMP_DISABLE "no"
#define MODULE_CAC_PROPERTY_IN_DUMP_ENABLE "in"
#define MODULE_CAC_PROPERTY_OUT_DUMP_ENABLE "out"
#define MODULE_CAC_PROPERTY_IN_OUT_DUMP_ENABLE "in out"

/**
 * Function: module_cac_client_post_mct_msg
 *
 * Description: Post message to the MCT bus
 *
 * Arguments:
 *   p_client - CAC client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: Used only during pipelining for continuous burst shot
 **/
int module_cac_client_post_mct_msg(cac_client_t *p_client)
{
  int rc = IMG_SUCCESS;
#if 0
  cam_cac_info_t cac_info;
  mct_bus_msg_t bus_msg;
  mct_module_t *p_mct_mod = p_client->parent_mod;
  //Fill in cac info data
  cac_info.frame_id = p_client->p_buf_divert_data->buffer.sequence;
  cac_info.buf_idx = p_client->p_buf_divert_data->buffer.index;
  //Compose Bus message
  bus_msg.type = MCT_BUS_MSG_CAC_INFO;
  bus_msg.msg = (void *)&cac_info;
  bus_msg.sessionid = IMGLIB_SESSIONID(p_client->identity);
  IDBG_MED("%s:%d] session id %d mct_mod %p", __func__, __LINE__,
    bus_msg.sessionid, p_mct_mod);
  rc = mct_module_post_bus_msg(p_mct_mod, &bus_msg);
  if (!rc) {
    return IMG_ERR_GENERAL;
  }
#endif
  return IMG_SUCCESS;
}

/**
 * Function: module_cac_client_event_handler
 *
 * Description: event handler for FaceProc client
 *
 * Arguments:
 *   p_appdata - CAC test object p_event - pointer to the event
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int module_cac_client_event_handler(void* p_appdata,
  img_event_t *p_event)
{
  cac_client_t *p_client;
  img_component_ops_t *p_comp;
  int rc = IMG_SUCCESS;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  if ((NULL == p_event) || (NULL == p_appdata)) {
    IDBG_ERROR("%s:%d] invalid event", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  p_client = (cac_client_t *)p_appdata;
  p_comp = &p_client->comp;
  IDBG_LOW("%s:%d] type %d", __func__, __LINE__, p_event->type);

  switch (p_event->type) {
  case QIMG_EVT_BUF_DONE:
    //CAC processes one frame at a time currently. Nothing to be done.
    break;
  case QIMG_EVT_ERROR:
    IDBG_HIGH("%s %d: CAC Error", __func__, __LINE__);
  case QIMG_EVT_DONE:
    pthread_cond_signal(&p_client->cond);
    break;
  default:
    break;
  }
  return rc;

}

/**
 * Function: module_cac_client_getbuf
 *
 * Description: This function is to open the imaging buffer mgr
 *              and queing and dequeing the buffer.
 * Arguments:
 *   @p_client: cac client
 *   @pframe: frame pointer
 *   @native_buf: flag to indicate if its a native buffer
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_cac_client_getbuf(cac_client_t *p_client,
  img_frame_t *pframe, int native_buf)
{
  int rc = IMG_SUCCESS;
  int i = 0;
  uint32_t buf_idx;
  uint32_t size;
  uint8_t *p_addr;
  mct_module_t *p_mct_mod;
  uint32_t padded_size;
  int fd = -1;
  int stride, scanline;
  mct_stream_map_buf_t *buf_holder;

  pframe->frame_cnt = 1;
  pframe->idx = 0;
  pframe->info.width = p_client->stream_info->dim.width;
  pframe->info.height = p_client->stream_info->dim.height;
  size = pframe->info.width * pframe->info.height;
  pframe->frame[0].plane_cnt = 2;
  stride = p_client->stream_info->buf_planes.plane_info.mp[0].stride;
  scanline = p_client->stream_info->buf_planes.plane_info.mp[0].scanline;
  buf_idx = p_client->p_buf_divert_data->buffer.index;
  padded_size = stride * scanline;

  if (NULL == p_client->p_sinkport) {
    IDBG_ERROR("%s:%d] NULL Sink port", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(p_client->p_sinkport))->data);
  IDBG_MED("%s:%d] Dimension %dx%d buf_idx %d %x mod %p port %p pproc %p"
    " pad %dx%d",
    __func__, __LINE__,
    pframe->info.width, pframe->info.height, buf_idx,
    p_client->identity,
    p_mct_mod,
    p_client->p_sinkport,
    p_client->parent_mod,
    stride,
    scanline);

  if (!native_buf) {
    buf_holder = mct_module_get_buffer(buf_idx,
      p_client->parent_mod,
      IMGLIB_SESSIONID(p_client->identity),
      IMGLIB_STREAMID(p_client->identity));

    if (NULL == buf_holder) {
      IDBG_ERROR("%s:%d] NULL buff holder", __func__, __LINE__);
      return IMG_ERR_INVALID_OPERATION;
    }
    p_addr = buf_holder->buf_planes[0].buf;
    fd = buf_holder->buf_planes[0].fd;
  } else {
    p_addr = p_client->p_buf_divert_data->vaddr;
    fd = p_client->p_buf_divert_data->fd;
    IDBG_MED("%s:%d] Native Buffer addr = %p, fd = %d",
     __func__, __LINE__, p_addr, fd);
  }

  if (NULL == p_addr) {
    IDBG_ERROR("%s:%d] NULL address", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  for (i = 0; i < pframe->frame[0].plane_cnt; i++) {
    pframe->frame[0].plane[i].fd = fd;
    pframe->frame[0].plane[i].offset = 0;
    if (i == 0) { /* Y plane */
      pframe->frame[0].plane[i].addr = p_addr;
      pframe->frame[0].plane[i].width = pframe->info.width;
      pframe->frame[0].plane[i].height = pframe->info.height;
      pframe->frame[0].plane[i].stride = stride;
      pframe->frame[0].plane[i].length =
       IMG_LENGTH(pframe->frame[0].plane[i]);
    } else { /* Chroma plane */
      pframe->frame[0].plane[i].addr = p_addr + padded_size;
      pframe->frame[0].plane[i].width = pframe->info.width;
      pframe->frame[0].plane[i].height = pframe->info.height/2;
      pframe->frame[0].plane[i].stride = stride;
      pframe->frame[0].plane[i].length =
       IMG_LENGTH(pframe->frame[0].plane[i]);
    }
  }

  if (p_client->dump_input_frame) {
    mod_imglib_dump_frame(p_client->frame, "frame", ++file_index);
  }

  return rc;
}

/**
 * Function: module_cac_client_exec
 *
 * Description: This function is for handling the buffers
 *            sent from the peer modules
 *
 * Arguments:
 *   @p_client: cac client
 *   @buf_idx: index of the buffer to be processed
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_cac_client_exec(cac_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_client->comp;
  img_frame_t *p_frame = &p_client->frame[0];

  IDBG_MED("%s:%d] ", __func__, __LINE__);

  // Get Frame first since we need dimension to calculate some rnr params
  rc = module_cac_client_getbuf(p_client, p_frame,
    p_client->p_buf_divert_data->native_buf);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s : Error: Cannot get frame", __func__);
    return IMG_ERR_GENERAL;
  }
  IDBG_HIGH("%s:%d] dim %dx%d frame %p", __func__, __LINE__,
    p_frame[0].info.width, p_frame[0].info.height, &p_frame[0]);

#ifdef USE_CAC_V1
  rc = module_cac_v1_config_client(p_client);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Error: Cannot get frame", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }
#else
  rc = module_cac_v2_config_client(p_client);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Error: Cannot get frame", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }
#endif

  // Queuing only 1 buffer at a time.
  rc = IMG_COMP_Q_BUF(p_comp, &p_frame[0], IMG_IN);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_START(p_comp, NULL);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    return rc;
  }

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  return rc;
}

/**
 * Function: module_cac_client_exec
 *
 * Description: This function is for handling the buffers
 *            sent from the peer modules
 *
 * Arguments:
 *   @p_client: cac client
 *   @buf_idx: index of the buffer to be processed
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
void module_cac_client_divert_exec(void *userdata, void *data)
{
  int rc = IMG_SUCCESS;
  cac_client_t *p_client = (cac_client_t *)userdata;
  mod_img_msg_buf_divert_t *p_divert = (mod_img_msg_buf_divert_t *)data;
  mct_event_t buff_divert_event;

  p_client->p_buf_divert_data = &p_divert->buf_divert;
  pthread_mutex_lock(&p_client->mutex);
  IDBG_HIGH("%s:%d] Start", __func__, __LINE__);

  if (p_client->stream_off) {
    IDBG_HIGH("%s:%d] streamoff called return", __func__, __LINE__);
    pthread_mutex_unlock(&p_client->mutex);
    return;
  }
  /* do not perform CAC if buffer is chroma subsampled */
  if (!p_divert->buf_divert.is_uv_subsampled) {
    p_client->cac2_cfg_info.cac2_enable_flag = TRUE;
  } else {
    p_client->cac2_cfg_info.cac2_enable_flag = FALSE;
  }
  rc = module_cac_client_exec(p_client);

  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] CAC Not Successful, rc = %d", __func__, __LINE__, rc);
    pthread_mutex_unlock(&p_client->mutex);
    return;
  }
  //Currently CAC is in Syn mode. Will not wait here
  if (p_client->mode == IMG_ASYNC_MODE) {
    IDBG_HIGH("%s:%d] before wait rc %d", __func__, __LINE__, rc);
    rc = img_wait_for_completion(&p_client->cond, &p_client->mutex,
      10000);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
      pthread_mutex_unlock(&p_client->mutex);
      return;
    }
  }
  IDBG_HIGH("%s:%d] after wait rc %d", __func__, __LINE__, rc);

  pthread_mutex_unlock(&p_client->mutex);

  if (p_client->dump_output_frame) {
    mod_imglib_dump_frame(p_client->frame, "frame", ++file_index);
  }

  /* Send event to CPP */
  if (!p_client->stream_off) {
    memset(&buff_divert_event, 0x0, sizeof(mct_event_t));
    buff_divert_event.type = MCT_EVENT_MODULE_EVENT;
    buff_divert_event.identity = p_client->identity;
    buff_divert_event.direction = MCT_EVENT_DOWNSTREAM;
    buff_divert_event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
    buff_divert_event.u.module_event.module_event_data =
      p_client->p_buf_divert_data;
    rc =  mct_port_send_event_to_peer(p_client->p_srcport,
      &buff_divert_event);
  }

  /* Stop the client*/
  module_cac_client_stop(p_client);
  IDBG_HIGH("%s:%d] End", __func__, __LINE__);
  return;
}

/**
 * Function: module_cac_client_stop
 *
 * Description: This function is used to stop the CAC
 *              client
 *
 * Arguments:
 *   @p_client: CAC client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_cac_client_stop(cac_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_client->comp;

  pthread_mutex_lock(&p_client->mutex);
  rc = IMG_COMP_ABORT(p_comp, NULL);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] create failed %d", __func__, __LINE__, rc);
    pthread_mutex_unlock(&p_client->mutex);
    return rc;
  }
  p_client->state = IMGLIB_STATE_INIT;
  pthread_mutex_unlock(&p_client->mutex);
  return rc;
}

/**
 * Function: module_cac_client_destroy
 *
 * Description: This function is used to destroy the cac client
 *
 * Arguments:
 *   @p_client: cac client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
void module_cac_client_destroy(cac_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = NULL;

  if (NULL == p_client) {
    return;
  }

  p_comp = &p_client->comp;
  IDBG_MED("%s:%d] state %d", __func__, __LINE__, p_client->state);

  if (IMGLIB_STATE_STARTED == p_client->state) {
    module_cac_client_stop(p_client);
  }

  if (IMGLIB_STATE_INIT == p_client->state) {
    rc = IMG_COMP_DEINIT(p_comp);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] deinit failed %d", __func__, __LINE__, rc);
    }
    p_client->state = IMGLIB_STATE_IDLE;
  }

  if (IMGLIB_STATE_IDLE == p_client->state) {
    pthread_mutex_destroy(&p_client->mutex);
    pthread_cond_destroy(&p_client->cond);

    free(p_client);
    p_client = NULL;
  }
  IDBG_MED("%s:%d] X", __func__, __LINE__);

}


/** Function: module_cac_client_create
 *
 * Description: This function is used to create the CAC client
 *
 * Arguments:
 *   @p_mct_mod: mct module pointer
 *   @p_port: mct port pointer
 *   @identity: identity of the stream
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_cac_client_create(mct_module_t *p_mct_mod, mct_port_t *p_port,
  uint32_t identity, mct_stream_info_t *stream_info)
{
  int rc = IMG_SUCCESS;
  cac_client_t *p_client = NULL;
  img_component_ops_t *p_comp = NULL;
  img_core_ops_t *p_core_ops = NULL;
  module_cac_t *p_mod = (module_cac_t *)p_mct_mod->module_private;
  mct_list_t *p_temp_list = NULL;
#ifdef _ANDROID_
  char value[PROPERTY_VALUE_MAX];
#endif

  p_core_ops = &p_mod->core_ops;

  IDBG_MED("%s:%d]", __func__, __LINE__);
  p_client = (cac_client_t *)malloc(sizeof(cac_client_t));
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] CAC client alloc failed", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }

  /* initialize the variables */
  memset(p_client, 0x0, sizeof(cac_client_t));

  p_comp = &p_client->comp;
  pthread_mutex_init(&p_client->mutex, NULL);
  pthread_cond_init(&p_client->cond, NULL);
  p_client->state = IMGLIB_STATE_IDLE;
  p_client->stream_info = stream_info;

  rc = IMG_COMP_CREATE(p_core_ops, p_comp);
  if (IMG_ERROR(rc)) {
   IDBG_ERROR("%s:%d] create failed %d", __func__, __LINE__, rc);
    goto error;
  }

  rc = IMG_COMP_INIT(p_comp, p_client, NULL);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] init failed %d", __func__, __LINE__, rc);
    goto error;
  }
  p_client->state = IMGLIB_STATE_INIT;

  rc = IMG_COMP_SET_CB(p_comp, module_cac_client_event_handler);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] rc %d", __func__, __LINE__, rc);
    goto error;
  }

    /* add the client to the list */
  p_temp_list = mct_list_append(p_mod->cac_client, p_client, NULL, NULL);
  if (NULL == p_temp_list) {
    IDBG_ERROR("%s:%d] list append failed", __func__, __LINE__);
    rc = IMG_ERR_NO_MEMORY;
    goto error;
  }
  p_mod->cac_client = p_temp_list;
  p_client->p_sinkport = p_port;
  p_client->identity = identity;
  p_client->parent_mod = p_mod->parent_mod;
  p_client->p_mod = p_mod;
  p_port->port_private = p_client;
  memset(p_client->frame, 0x0, sizeof(img_frame_t) * MAX_NUM_FRAMES);

  p_client->dump_input_frame = FALSE;
  p_client->dump_output_frame = FALSE;

#ifdef _ANDROID_
  property_get("persist.camera.imglib.cac.dump",
    value,
    MODULE_CAC_PROPERTY_DUMP_DISABLE);

  if (!strncmp(MODULE_CAC_PROPERTY_IN_DUMP_ENABLE,
    value,
    sizeof(MODULE_CAC_PROPERTY_IN_DUMP_ENABLE))) {
      p_client->dump_input_frame = TRUE;
  } else if (!strncmp(MODULE_CAC_PROPERTY_OUT_DUMP_ENABLE,
    value,
    sizeof(MODULE_CAC_PROPERTY_OUT_DUMP_ENABLE))) {
      p_client->dump_output_frame = TRUE;
  } else if (!strncmp(MODULE_CAC_PROPERTY_IN_OUT_DUMP_ENABLE,
    value,
    sizeof(MODULE_CAC_PROPERTY_IN_OUT_DUMP_ENABLE))) {
      p_client->dump_input_frame = TRUE;
      p_client->dump_output_frame = TRUE;
  }
#endif

  IDBG_MED("%s:%d] port %p client %p X", __func__, __LINE__, p_port, p_client);
  return rc;

error:
  if (p_client) {
    module_cac_client_destroy(p_client);
    p_client = NULL;
  }
  return rc;
}
