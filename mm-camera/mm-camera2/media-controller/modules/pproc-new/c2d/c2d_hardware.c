/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "c2d_hardware.h"
#include "c2d_log.h"
#include <linux/media.h>
#include <fcntl.h>
#include <errno.h>

/* c2d_hardware_create:
 *
 *  creates new c2d_hardware instance. Finds the c2d subdev in kernel,
 *  allocates memory and initializes the structure.
 *
 **/
c2d_hardware_t* c2d_hardware_create()
{
  c2d_hardware_t *c2dhw;
  int rc;
  c2dhw = (c2d_hardware_t *) malloc(sizeof(c2d_hardware_t));
  if(!c2dhw) {
    CDBG_ERROR("%s:%d: malloc() failed\n", __func__, __LINE__);
    return NULL;
  }
  memset(c2dhw, 0x00, sizeof(c2d_hardware_t));

  /* initialize the stream_status */
  int i;
  for (i=0; i<C2D_HARDWARE_MAX_STREAMS; i++) {
    c2dhw->stream_status[i].valid = FALSE;
    c2dhw->stream_status[i].identity = 0x00;
    c2dhw->stream_status[i].pending_buf = 0;
    c2dhw->stream_status[i].stream_off_pending = FALSE;
  }
  pthread_cond_init(&(c2dhw->no_pending_cond), NULL);
  pthread_mutex_init(&(c2dhw->mutex), NULL);
  return c2dhw;
}

/* c2d_hardware_destroy:
 *
 *  destroy the hardware data structure and free memory
 *
 **/
int32_t c2d_hardware_destroy(c2d_hardware_t *c2dhw)
{
  if(!c2dhw) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  pthread_cond_destroy(&(c2dhw->no_pending_cond));
  pthread_mutex_destroy(&(c2dhw->mutex));
  free(c2dhw);
  return 0;
}

/* c2d_hardware_get_status:
 *
 *  get current status of the hardware. Hardware structure access is
 *  protected by the mutex
 *
 **/
c2d_hardware_status_t c2d_hardware_get_status(c2d_hardware_t *c2dhw)
{
  c2d_hardware_status_t status;
  if (!c2dhw) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return FALSE;
  }
  int32_t num_pending=0;
  PTHREAD_MUTEX_LOCK(&(c2dhw->mutex));
  int i;
  for (i=0; i<C2D_HARDWARE_MAX_STREAMS; i++) {
    if (c2dhw->stream_status[i].valid) {
      num_pending += c2dhw->stream_status[i].pending_buf;
    }
  }
  if (num_pending < C2D_HARDWARE_MAX_PENDING_BUF) {
    status = C2D_HW_STATUS_READY;
  } else {
    status = C2D_HW_STATUS_BUSY;
  }
  PTHREAD_MUTEX_UNLOCK(&(c2dhw->mutex));
  return status;
}

/* c2d_hardware_process_command:
 *
 *  processes the command given to the hardware. Hardware state is
 *  updated during the process. All accesses to the shared hardware
 *  data structure are protected by mutex.
 *
 **/
int32_t c2d_hardware_process_command(c2d_hardware_t *c2dhw,
  c2d_hardware_cmd_t cmd)
{
  int rc = 0;
  if (!c2dhw) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return -EINVAL;
  }

  switch (cmd.type) {
  case C2D_HW_CMD_GET_CAPABILITIES:
    rc = c2d_hardware_get_capabilities(c2dhw);
    break;
  case C2D_HW_CMD_STREAMON:
    rc = c2d_hardware_process_streamon(c2dhw, cmd.u.stream_buff_list);
    break;
  case C2D_HW_CMD_STREAMOFF:
    rc = c2d_hardware_process_streamoff(c2dhw, cmd.u.streamoff_identity);
    break;
  case C2D_HW_CMD_PROCESS_FRAME:
    rc = c2d_hardware_process_frame(c2dhw, cmd.u.hw_params);
    break;
  case C2D_HW_CMD_RELEASE_FRAME:
    rc = c2d_hardware_release_frame(c2dhw, cmd.u.hw_params);
    break;
  default:
    CDBG_ERROR("%s:%d, error: bad command type=%d",
      __func__, __LINE__, cmd.type);
    rc = -EINVAL;
  }
  return rc;
}

/* c2d_hardware_get_capabilities:
 *
 * Description:
 *  Get hardware capabilities from kernel
 *
 **/
int32_t c2d_hardware_get_capabilities(c2d_hardware_t *c2dhw)
{
  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(c2dhw->mutex));
  /* TODO: read from hw reg */
  c2dhw->caps.caps_mask = C2D_CAPS_SCALE | C2D_CAPS_CROP |
                          C2D_CAPS_ROTATION | C2D_CAPS_FLIP;

  /* TODO find his scale factors for C2D */
  c2dhw->caps.scaling_caps.max_scale_factor = 8.0;
  c2dhw->caps.scaling_caps.min_scale_factor = 1.0/16.0;
  c2dhw->caps.rotation_caps = ROTATION_90 | ROTATION_180 | ROTATION_270;
  c2dhw->caps.filp_caps.h_flip = TRUE;
  c2dhw->caps.filp_caps.v_flip = TRUE;
  /* TODO remove sharpness after verifying offline ZSL */
  c2dhw->caps.sharpness_caps.max_value = C2D_MAX_SHARPNESS;
  c2dhw->caps.sharpness_caps.min_value = C2D_MIN_SHARPNESS;
  c2dhw->caps.sharpness_caps.def_value = C2D_DEFAULT_SHARPNESS;
  c2dhw->caps.sharpness_caps.step = (C2D_MAX_SHARPNESS - C2D_MIN_SHARPNESS) /
                                      C2D_TOTAL_SHARPNESS_LEVELS;

  PTHREAD_MUTEX_UNLOCK(&(c2dhw->mutex));
  return 0;
}

/* c2d_hardware_process_streamon:
 *
 **/
int32_t c2d_hardware_process_streamon(c2d_hardware_t *c2dhw,
  c2d_hardware_stream_buff_info_t *hw_strm_buff_info)
{
  int rc;
  uint32_t i;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;

  CDBG("%s:%d, streaming on\n", __func__, __LINE__);
  if (NULL == hw_strm_buff_info) {
    CDBG_ERROR("%s:%d] error hw_strm_buff_info:%p\n", __func__, __LINE__,
      hw_strm_buff_info);
    return -EINVAL;
  }
  /* TODO Map GPU buffer here */
  /* make stream_status valid for this stream */
  for (i=0; i<C2D_HARDWARE_MAX_STREAMS; i++) {
    if (c2dhw->stream_status[i].valid == FALSE) {
      c2dhw->stream_status[i].identity = hw_strm_buff_info->identity;
      c2dhw->stream_status[i].pending_buf = 0;
      c2dhw->stream_status[i].valid = TRUE;
      c2dhw->stream_status[i].stream_off_pending = FALSE;
      break;
    }
  }
  PTHREAD_MUTEX_UNLOCK(&(c2dhw->mutex));
  CDBG("%s:%d, stream on done\n", __func__, __LINE__);
  return 0;
}


/* c2d_hardware_process_streamoff:
 *
 **/
int32_t c2d_hardware_process_streamoff(c2d_hardware_t *c2dhw,
  uint32_t identity)
{
  int rc, i;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;

  CDBG("%s:%d, identity=0x%x", __func__, __LINE__, identity);
  PTHREAD_MUTEX_LOCK(&(c2dhw->mutex));
  c2d_hardware_stream_status_t* stream_status =
    c2d_hardware_get_stream_status(c2dhw, identity);
  if (!stream_status) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    PTHREAD_MUTEX_UNLOCK(&(c2dhw->mutex));
    return -EFAULT;
  }
  stream_status->stream_off_pending = TRUE;
  /* wait for all pending buffers to be processed */
  while (stream_status->pending_buf != 0) {
    CDBG_LOW("%s:%d, waiting for pending buf, identity=0x%x, pending_buf=%d",
      __func__, __LINE__, identity, stream_status->pending_buf);
    pthread_cond_wait(&(c2dhw->no_pending_cond), &(c2dhw->mutex));
  }
  stream_status->stream_off_pending = FALSE;
  stream_status->valid = FALSE;
  PTHREAD_MUTEX_UNLOCK(&(c2dhw->mutex));

  CDBG("%s:%d, pending buffers done, hw streaming off. identity=0x%x\n",
    __func__, __LINE__, identity);
  /* Unmap GPU address here */
  return 0;
}

/* c2d_hardware_get_stream_status:
 *
 **/
c2d_hardware_stream_status_t*
  c2d_hardware_get_stream_status(c2d_hardware_t* c2dhw, uint32_t identity)
{
  int i;
  for (i=0; i<C2D_HARDWARE_MAX_STREAMS; i++) {
    if (c2dhw->stream_status[i].valid == TRUE) {
      if (c2dhw->stream_status[i].identity == identity) {
        return &(c2dhw->stream_status[i]);
      }
    }
  }
  return NULL;
}

/* c2d_hardware_process_frame:
 *
 **/
static int32_t c2d_hardware_process_frame(c2d_hardware_t *c2dhw,
  c2d_hardware_params_t *hw_params)
{
  int32_t rc = 0;
  int in_frame_fd;
  struct c2d_frame_info_t c2d_frame_info;
  struct msm_c2d_frame_info_t *msm_c2d_frame_info;
  memset(&c2d_frame_info, 0, sizeof(struct c2d_frame_info_t));
  if (!c2dhw) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EINVAL;
  }

  CDBG_LOW("%s:%d, identity=0x%x", __func__, __LINE__, hw_params->identity);

  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(c2dhw->mutex));
  c2d_hardware_stream_status_t *stream_status =
    c2d_hardware_get_stream_status(c2dhw, hw_params->identity);
  if (!stream_status) {
#if 0
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    PTHREAD_MUTEX_UNLOCK(&(c2dhw->mutex));
    return -EFAULT;
#else
    /* Invalid stream at this layer means the stream is invalidated due to
       racing streamoff and this is considered as a racing process frame
       before queue invalidate */
     /* TODO: Do we need to do c2d_event->invalid = TRUE ?*/
     PTHREAD_MUTEX_UNLOCK(&(c2dhw->mutex));
     return -EAGAIN;
#endif
  }
  /* update stream status */
  stream_status->pending_buf++;
  PTHREAD_MUTEX_UNLOCK(&(c2dhw->mutex));
  return 0;
}

/* c2d_hardware_release_frame:
 *
 **/
static int32_t c2d_hardware_release_frame(c2d_hardware_t *c2dhw,
  c2d_hardware_params_t *hw_params)
{
  int32_t rc = 0;
  if (!c2dhw) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EINVAL;
  }

  CDBG_LOW("%s:%d, identity=0x%x", __func__, __LINE__, hw_params->identity);

  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(c2dhw->mutex));
  c2d_hardware_stream_status_t *stream_status =
    c2d_hardware_get_stream_status(c2dhw, hw_params->identity);
  if (!stream_status) {
#if 0
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    PTHREAD_MUTEX_UNLOCK(&(c2dhw->mutex));
    return -EFAULT;
#else
    /* Invalid stream at this layer means the stream is invalidated due to
       racing streamoff and this is considered as a racing process frame
       before queue invalidate */
     /* TODO: Do we need to do c2d_event->invalid = TRUE ?*/
     PTHREAD_MUTEX_UNLOCK(&(c2dhw->mutex));
     return -EAGAIN;
#endif
  }
  /* update stream status */
  stream_status->pending_buf--;
  /* send signal to thread which is waiting on stream_off
     for pending buffer to be zero */
  if (stream_status->stream_off_pending == TRUE &&
    stream_status->pending_buf == 0) {
    CDBG("%s:%d, info: sending broadcast for pending stream-off",
      __func__, __LINE__);
    pthread_cond_broadcast(&(c2dhw->no_pending_cond));
  }
  PTHREAD_MUTEX_UNLOCK(&(c2dhw->mutex));
  return 0;
}
