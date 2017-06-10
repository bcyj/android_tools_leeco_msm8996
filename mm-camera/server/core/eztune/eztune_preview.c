/**********************************************************************
* Copyright (c) 2011 Qualcomm Technologies, Inc.  All Rights Reserved.     *
* Qualcomm Technologies Proprietary and Confidential.                              *
**********************************************************************/

#include "eztune.h"
/* Keep following header as the last header in this list since it has
#pragma pack(1) */
#include "eztune_preview.h"

#define EZTUNE_DESIRED_CHUNK_SIZE    7168
#define EZTUNE_MAX_CHUNK_SIZE        10240

static pthread_mutex_t prev_mut = PTHREAD_MUTEX_INITIALIZER;
static eztune_prev_protocol_t *prev_handle = NULL;
static cam_ctrl_dimension_t *dim_info = NULL;
static int data_avail = 0;

/*===========================================================================
 * FUNCTION     - eztune_prev_write_status -
 *
 * DESCRIPTION:
 * ==========================================================================*/
static int32_t eztune_prev_write_status(int8_t status,
                                 int client_socket)
{
  int32_t rc = 0;

  rc = write(client_socket,
             &status,
             sizeof(status));

  return rc;
}

/*===========================================================================
 * FUNCTION     - eztune_copy_preview_frame -
 *
 * DESCRIPTION:
 * ==========================================================================*/
int32_t eztune_copy_preview_frame(struct msm_pp_frame *frame)
{
  int32_t rc = 0;
  uint32_t frame_size = 0;
  int16_t index = 0;
  uint32_t data_offset = 0;

  /* Lock mutex */
  pthread_mutex_lock(&(prev_mut));

  /* Check for pointer p */
  if(!prev_handle) {
    /* Unlock mutex */
    pthread_mutex_unlock(&(prev_mut));
    return -EINVAL;
  }

  if(frame->num_planes == 1) { /* SP */
    frame_size = frame->sp.length;
  } else { /* MP */
    for (index = 0; index < frame->num_planes; index++) {
      frame_size += frame->mp[index].length;
    }
  }

  /* Validate frame size */
  if((frame_size != prev_handle->get_frame.frame_size) ||
     (!prev_handle->prev_frame.data_buf)) {
    free(prev_handle->prev_frame.data_buf);
    prev_handle->get_frame.width = dim_info->display_width;
    prev_handle->get_frame.height = dim_info->display_height;
    prev_handle->get_frame.frame_size = frame_size;
    prev_handle->prev_frame.data_buf = (char *)malloc(frame_size);
    if(!prev_handle->prev_frame.data_buf) {
      /* Unlock mutex */
      pthread_mutex_unlock(&(prev_mut));
      return -ENOMEM;
    }
  }

  CDBG_EZ("%s\n", __func__);

  /* Copy preview frame to local */
  if(frame->num_planes == 1) { /* SP */
      memcpy((void *)prev_handle->prev_frame.data_buf,
           (const void *)(frame->sp.vaddr + frame->sp.y_off),
           prev_handle->get_frame.frame_size);
  } else { /* MP */
    data_offset = 0;
    for (index = 0; index < frame->num_planes; index++) {
      memcpy((void *)(prev_handle->prev_frame.data_buf + data_offset),
            (const void *)(frame->mp[index].vaddr +
                           frame->mp[index].data_offset),
             frame->mp[index].length);
      data_offset += frame->mp[index].length;
    }
  }

  data_avail = 1;

  /* Unlock mutex */
  pthread_mutex_unlock(&(prev_mut));
  return rc;
}

/*===========================================================================
 * FUNCTION     - eztune_init_preview_settings -
 *
 * DESCRIPTION:
 * ==========================================================================*/
int32_t eztune_init_preview_settings(eztune_prev_protocol_t *prev_data,
                                         cam_ctrl_dimension_t *dim)
{

  /* Lock mutex */
  pthread_mutex_lock(&(prev_mut));

  CDBG_EZ("%s\n", __func__);

  prev_handle = prev_data;
  dim_info = dim;

  /* Initialize get_info_ver struct */
  prev_data->get_info_ver.major_ver = 1;
  prev_data->get_info_ver.minor_ver = 0;
  prev_data->get_info_ver.header_size = sizeof(prev_data->get_info_caps);

  /* Initialize get_info_caps struct */
  prev_data->get_info_caps.target_type = 1;
  prev_data->get_info_caps.capabilities = 0x01;
  prev_data->get_info_caps.cnk_size = EZTUNE_DESIRED_CHUNK_SIZE;

  /* Initiatlize ch_cnk_size struct */
  prev_data->ch_cnk_size.cnk_size = EZTUNE_DESIRED_CHUNK_SIZE;

  /* Initialize get_frame struct */
  prev_data->get_frame.width = dim_info->display_width;
  prev_data->get_frame.height = dim_info->display_height;

  switch (dim_info->prev_format) {
    case CAMERA_YUV_420_NV12:
      prev_data->get_frame.format = EZTUNE_FORMAT_YCbCr_420;
      break;
    case CAMERA_YUV_420_NV21:
      prev_data->get_frame.format = EZTUNE_FORMAT_YCrCb_420;
      break;
    case CAMERA_YUV_420_YV12:
      prev_data->get_frame.format = EZTUNE_FORMAT_YUV_420;
      break;
    default:
      prev_data->get_frame.format = EZTUNE_FORMAT_YCrCb_420;
      break;
  }

  prev_data->get_frame.origin = EZTUNE_ORIGIN_TOP_LEFT;
  prev_data->get_frame.frame_size = ((uint32_t)prev_data->get_frame.width *
                             prev_data->get_frame.height * 3) / 2;

  /* Initialize prev_frame struct */
  prev_data->prev_frame.data_buf =
                    (char *)malloc(prev_data->get_frame.frame_size);
  if(!prev_data->prev_frame.data_buf) {
    CDBG_EZ("%s mem alloc failed\n", __func__);
    /* Unock mutex */
    pthread_mutex_unlock(&(prev_mut));
    return -1;
  }

  data_avail = 0;

  CDBG_EZ("%s get frame %d %d %d %d %d %d\n",
       __func__,
       prev_data->get_frame.status,
       prev_data->get_frame.width,
       prev_data->get_frame.height,
       prev_data->get_frame.format,
       prev_data->get_frame.origin,
       prev_data->get_frame.frame_size);

  CDBG_EZ("%s dim info %d %d %d\n",
       __func__,
       dim_info->display_width,
       dim_info->display_height,
       dim_info->prev_format);


  /* Unock mutex */
  pthread_mutex_unlock(&(prev_mut));

  return 0;
}

/*===========================================================================
 * FUNCTION     - eztune_prev_init_protocol_data -
 *
 * DESCRIPTION:
 * ==========================================================================*/
void eztune_prev_init_protocol_data(
  eztune_prev_protocol_t *prev_proc)
{
  if(prev_proc) {
      prev_proc->current_cmd    = 0xFFFF;
      prev_proc->next_recv_code = EZTUNE_RECV_COMMAND;
      prev_proc->next_recv_len  = 2;
  }
}

/*===========================================================================
 * FUNCTION     - eztune_prev_get_info -
 *
 * DESCRIPTION:
 * ==========================================================================*/
static int32_t eztune_prev_get_info(eztune_prev_protocol_t *prev_data,
                                    int client_socket)
{
  int32_t rc = 0;

  /* Lock mutex */
  pthread_mutex_lock(&(prev_mut));

  CDBG_EZ("%s\n", __func__);
  CDBG_EZ("%s get info ver %d %d %d\n",
       __func__,
       prev_data->get_info_ver.major_ver,
       prev_data->get_info_ver.minor_ver,
       prev_data->get_info_ver.header_size);
  CDBG_EZ("%s get info caps %d %d %d\n",
       __func__,
       prev_data->get_info_caps.target_type,
       prev_data->get_info_caps.capabilities,
       prev_data->get_info_caps.cnk_size);

  /* Write version and header size */
  write(client_socket,
        &prev_data->get_info_ver,
        sizeof(prev_data->get_info_ver));

  /* Write capabilities */
  write(client_socket,
        &prev_data->get_info_caps,
        sizeof(prev_data->get_info_caps));

  /* Unock mutex */
  pthread_mutex_unlock(&(prev_mut));

  return rc;
}

/*===========================================================================
 * FUNCTION     - eztune_prev_ch_cnk_size -
 *
 * DESCRIPTION:
 * ==========================================================================*/
static int32_t eztune_prev_ch_cnk_size(eztune_prev_protocol_t *prev_data,
                                       uint32_t new_cnk_size,
                                       int client_socket)
{
  int32_t rc = 0;

  /* Lock mutex */
  pthread_mutex_lock(&(prev_mut));

  CDBG_EZ("%s\n", __func__);
  CDBG_EZ("%s new chunk size = %d\n", __func__,
                       new_cnk_size);

  if(new_cnk_size <= EZTUNE_MAX_CHUNK_SIZE) {
    prev_data->ch_cnk_size.status = EZTUNE_STATUS_ZERO;
    prev_data->ch_cnk_size.cnk_size = new_cnk_size;
  }
  else {
    prev_data->ch_cnk_size.status = EZTUNE_STATUS_TWO;
    prev_data->ch_cnk_size.cnk_size = EZTUNE_MAX_CHUNK_SIZE;
  }

  CDBG_EZ("%s cur chunk size = %d\n", __func__,
            prev_data->ch_cnk_size.cnk_size);
  /* Unlock mutex */
  pthread_mutex_unlock(&(prev_mut));

  CDBG_EZ("%s ch cnk size %d %d\n",
       __func__,
      prev_data->ch_cnk_size.status,
      prev_data->ch_cnk_size.cnk_size);

  /* Write response */
  write(client_socket,
        &prev_data->ch_cnk_size,
        sizeof(prev_data->ch_cnk_size));

  return rc;
}

/*===========================================================================
 * FUNCTION     - eztune_prev_get_prev_frame -
 *
 * DESCRIPTION:
 * ==========================================================================*/
static int32_t eztune_prev_get_prev_frame(eztune_prev_protocol_t *prev_data,
                                    int client_socket)
{
  int32_t rc = 0;
  uint32_t cnk_size = 0;
  char *data_buf = NULL;
  uint32_t frame_size = 0;

  cnk_size = prev_data->ch_cnk_size.cnk_size;
  data_buf = prev_data->prev_frame.data_buf;
  frame_size = prev_data->get_frame.frame_size;

  /* Lock mutex */
  pthread_mutex_lock(&(prev_mut));

  CDBG_EZ("%s\n", __func__);

  if(!data_avail) {
    prev_data->get_frame.status = 1;
    prev_data->get_frame.frame_size= 0;

    CDBG_EZ("%s data not available\n", __func__);

    CDBG_EZ("%s get frame %d %d %d %d %d %d\n",
         __func__,
         prev_data->get_frame.status,
         prev_data->get_frame.width,
         prev_data->get_frame.height,
         prev_data->get_frame.format,
         prev_data->get_frame.origin,
         prev_data->get_frame.frame_size);

    /* Write preview frame header */
    write(client_socket,
          &prev_data->get_frame,
          sizeof(prev_data->get_frame));

    prev_data->get_frame.frame_size= frame_size;
  } else {
    prev_data->get_frame.status = 0;

    /* Write preview frame header */
    write(client_socket,
          &prev_data->get_frame,
          sizeof(prev_data->get_frame));

    CDBG_EZ("%s data available\n", __func__);
    CDBG_EZ("%s get frame %d %d %d %d %d %d\n",
         __func__,
         prev_data->get_frame.status,
         prev_data->get_frame.width,
         prev_data->get_frame.height,
         prev_data->get_frame.format,
         prev_data->get_frame.origin,
         prev_data->get_frame.frame_size);

    /* Write preview frame in multiples of chunk size */
    while(1) {
      if(frame_size > cnk_size) {
        write(client_socket,
              data_buf,
              cnk_size);
        data_buf += cnk_size;
        frame_size -= cnk_size;
      }
      else {
        write(client_socket,
              data_buf,
              frame_size);
        break;
      }
    }

    data_avail = 0;
  }
  /* Unlock mutex */
  pthread_mutex_unlock(&(prev_mut));

  return rc;
}

/*===========================================================================
 * FUNCTION     - eztune_preview_process_command -
 *
 * DESCRIPTION:
 * ==========================================================================*/
static int32_t eztune_preview_process_command (
  eztune_prev_protocol_t *prev_proc,
  int client_socket)
{
  int32_t rc = 0;
  switch (prev_proc->current_cmd) {
    case EZTUNE_PREV_GET_INFO:
      CDBG_EZ("%s EZTUNE_PREV_GET_INFO E\n", __func__);
      rc = eztune_prev_get_info(prev_proc, client_socket);
      CDBG_EZ("%s EZTUNE_PREV_GET_INFO X\n", __func__);
      break;

    case EZTUNE_PREV_CH_CNK_SIZE:
      CDBG_EZ("%s EZTUNE_PREV_CH_CNK_SIZE E\n", __func__);
      rc = eztune_prev_ch_cnk_size(prev_proc,
           prev_proc->new_cnk_size,
           client_socket);
      CDBG_EZ("%s EZTUNE_PREV_CH_CNK_SIZE X\n", __func__);
      break;

    case EZTUNE_PREV_GET_PREV_FRAME:
      CDBG_EZ("%s EZTUNE_PREV_GET_PREV_FRAME E\n", __func__);
      rc = eztune_prev_get_prev_frame(prev_proc, client_socket);
      CDBG_EZ("%s EZTUNE_PREV_GET_PREV_FRAME X\n", __func__);
      break;

    case EZTUNE_PREV_GET_JPG_SNAP:
    case EZTUNE_PREV_GET_RAW_SNAP:
    case EZTUNE_PREV_GET_RAW_PREV:
      /* Write Unsupported */
      rc = eztune_prev_write_status(EZTUNE_STATUS_MINUS_ONE,
                                         client_socket);
      CDBG_EZ("%s unsuppored X\n", __func__);
      break;

    default:
      CDBG_EZ("%s prev_proc->current_cmd: default\n", __func__);
      break;
  }
  return rc;
}
/*===========================================================================
 * FUNCTION     - eztune_preview_server_run -
 *
 * DESCRIPTION:
 * ==========================================================================*/
int32_t eztune_preview_server_run (
  eztune_prev_protocol_t *prev_proc,
  int client_socket)
{
  int32_t rc = 0;
  char recv_buffer[EZTUNE_MAX_RECV];
  uint32_t recv_len;

  CDBG_EZ("%s calling read\n", __func__);
  recv_len = read(client_socket, recv_buffer, prev_proc->next_recv_len);
  CDBG_EZ("%s recv_len = %d, val = %d\n",
         __func__, recv_len, *(uint16_t *)recv_buffer);
  if (recv_len != prev_proc->next_recv_len) {
    CDBG_EZ("%s: read fail on the connection. %d vs %d\n",
      __func__, prev_proc->next_recv_len, recv_len);
    free(prev_handle->prev_frame.data_buf);
    prev_handle = NULL;
    return -1;
  }

  switch (prev_proc->next_recv_code) {
    case EZTUNE_PREV_COMMAND:
      CDBG_EZ("%s EZTUNE_PREV_COMMAND E\n", __func__);
      prev_proc->current_cmd = *(uint16_t *)recv_buffer;
      if(prev_proc->current_cmd != EZTUNE_PREV_CH_CNK_SIZE) {
        rc = eztune_preview_process_command(prev_proc, client_socket);
        CDBG_EZ("%s EZTUNE_PREV_COMMAND X\n", __func__);
        break;
      }
      prev_proc->next_recv_code = EZTUNE_PREV_NEW_CNK_SIZE;
      prev_proc->next_recv_len = sizeof(uint32_t);
      CDBG_EZ("%s EZTUNE_PREV_COMMAND X\n", __func__);
      break;

    case EZTUNE_PREV_NEW_CNK_SIZE:
      CDBG_EZ("%s EZTUNE_PREV_NEW_CNK_SIZE E\n", __func__);
      prev_proc->new_cnk_size = *(uint32_t *)recv_buffer;
      prev_proc->next_recv_code = EZTUNE_RECV_COMMAND;
      prev_proc->next_recv_len  = 2;
      CDBG_EZ("%s EZTUNE_PREV_NEW_CNK_SIZE X\n", __func__);
      rc = eztune_preview_process_command(prev_proc, client_socket);
      break;

    default:
      CDBG_EZ("%s prev_proc->next_recv_code: default\n", __func__);
  }

  return rc;
}/*end of eztune_preview_server_run*/
