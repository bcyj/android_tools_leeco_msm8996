/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vpe_hardware.h"
#include "vpe_log.h"
#include <linux/media.h>
#include <fcntl.h>
#include <errno.h>

/* vpe_hardware_create:
 *
 *  creates new vpe_hardware instance. Finds the vpe subdev in kernel,
 *  allocates memory and initializes the structure.
 *
 **/
vpe_hardware_t* vpe_hardware_create()
{
  vpe_hardware_t *vpehw;
  int rc;
  vpehw = (vpe_hardware_t *) malloc(sizeof(vpe_hardware_t));
  if(!vpehw) {
    CDBG_ERROR("%s:%d: malloc() failed\n", __func__, __LINE__);
    return NULL;
  }
  memset(vpehw, 0x00, sizeof(vpe_hardware_t));
  rc = vpe_hardware_find_subdev(vpehw);
  if (rc < 0) {
    CDBG_ERROR("%s:%d: error: cannot find vpe subdev\n", __func__, __LINE__);
    free(vpehw);
    return NULL;
  }
  vpehw->subdev_opened = FALSE;
  vpehw->status = VPE_HW_STATUS_INVALID;

  /* initialize the stream_status */
  int i;
  for (i=0; i<VPE_HARDWARE_MAX_STREAMS; i++) {
    vpehw->stream_status[i].valid = FALSE;
    vpehw->stream_status[i].identity = 0x00;
    vpehw->stream_status[i].pending_buf = 0;
    vpehw->stream_status[i].stream_off_pending = FALSE;
  }
  pthread_cond_init(&(vpehw->no_pending_cond), NULL);
  pthread_mutex_init(&(vpehw->mutex), NULL);
  return vpehw;
}

/* vpe_hardware_open_subdev:
 *
 *  opens the vpe subdev and updates instance info.
 *  updates the hardware status.
 *  currently only one vpe subdevice is supported.
 **/
int32_t vpe_hardware_open_subdev(vpe_hardware_t *vpehw)
{
  int fd, rc=0;
  char dev_name[SUBDEV_NAME_SIZE_MAX];
  if(!vpehw) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  /* make sure all code-paths unlock this mutex */
  PTHREAD_MUTEX_LOCK(&(vpehw->mutex));
  if (vpehw->subdev_opened == TRUE) {
    CDBG_HIGH("%s:%d: subdev already open\n", __func__, __LINE__);
    rc = -EFAULT;
    goto error_mutex;
  }
  snprintf(dev_name, sizeof(dev_name), "/dev/v4l-subdev%d",
    vpehw->subdev_ids[0]);
  fd = open(dev_name, O_RDWR | O_NONBLOCK);
  if (fd < 0) {
    CDBG_ERROR("%s:%d: error: cannot open vpe subdev: %s\n",
      __func__, __LINE__, dev_name);
    rc = -EIO;
    goto error_mutex;
  }
  vpehw->subdev_fd = fd;
  vpehw->subdev_opened = TRUE;
  /* get the instance info */
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  struct msm_vpe_frame_info_t inst_info;
  memset(&inst_info, 0x00, sizeof(struct msm_vpe_frame_info_t));
  v4l2_ioctl.ioctl_ptr = &inst_info;
  v4l2_ioctl.len = sizeof(inst_info);
  rc = ioctl(vpehw->subdev_fd, VIDIOC_MSM_VPE_GET_INST_INFO, &v4l2_ioctl);
  if (rc < 0) {
    CDBG_ERROR("%s:%d: v4l2 ioctl() failed, rc=%d\n", __func__, __LINE__, rc);
    rc = -EIO;
    goto error_open;
  }
  vpehw->inst_id = inst_info.inst_id;
  /* update hw state */
  vpehw->status = VPE_HW_STATUS_READY;
  CDBG("%s:%d, vpe subdev opened, subdev_fd=%d, status=%d",
    __func__, __LINE__, vpehw->subdev_fd, vpehw->status);

  PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
  return 0;

error_open:
  close(fd);
error_mutex:
  PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
  return rc;
}

/* vpe_hardware_close_subdev:
 *
 *  close the vpe subdev and update hardware status accordingly
 *
 **/
int32_t vpe_hardware_close_subdev(vpe_hardware_t *vpehw)
{
  if(!vpehw) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  PTHREAD_MUTEX_LOCK(&(vpehw->mutex));
  close(vpehw->subdev_fd);
  vpehw->subdev_opened = FALSE;
  /* if status is ready or busy, fw would be loaded */
  vpehw->status = VPE_HW_STATUS_INVALID;
  PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
  return 0;
}

/* vpe_hardware_destroy:
 *
 *  destroy the hardware data structure and free memory
 *
 **/
int32_t vpe_hardware_destroy(vpe_hardware_t *vpehw)
{
  if(!vpehw) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  if (vpehw->subdev_opened == TRUE) {
    vpe_hardware_close_subdev(vpehw);
  }
  pthread_cond_destroy(&(vpehw->no_pending_cond));
  pthread_mutex_destroy(&(vpehw->mutex));
  free(vpehw);
  return 0;
}

/* vpe_hardware_get_status:
 *
 *  get current status of the hardware. Hardware structure access is
 *  protected by the mutex
 *
 **/
vpe_hardware_status_t vpe_hardware_get_status(vpe_hardware_t *vpehw)
{
  vpe_hardware_status_t status;
  if (!vpehw) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return FALSE;
  }
  int32_t num_pending=0;
  PTHREAD_MUTEX_LOCK(&(vpehw->mutex));
  int i;
  for (i=0; i<VPE_HARDWARE_MAX_STREAMS; i++) {
    if (vpehw->stream_status[i].valid) {
      num_pending += vpehw->stream_status[i].pending_buf;
    }
  }
  if (num_pending < VPE_HARDWARE_MAX_PENDING_BUF) {
    status = VPE_HW_STATUS_READY;
  } else {
    status = VPE_HW_STATUS_BUSY;
  }
  PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
  return status;
}

/* vpe_hardware_process_command:
 *
 *  processes the command given to the hardware. Hardware state is
 *  updated during the process. All accesses to the shared hardware
 *  data structure are protected by mutex.
 *
 **/
int32_t vpe_hardware_process_command(vpe_hardware_t *vpehw,
  vpe_hardware_cmd_t cmd)
{
  int rc = 0;
  if (!vpehw) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  if (vpehw->subdev_opened == FALSE) {
    CDBG_ERROR("%s:%d: failed, subdev not opened\n", __func__, __LINE__);
    return -EINVAL;
  }

  switch (cmd.type) {
  case VPE_HW_CMD_GET_CAPABILITIES:
    rc = vpe_hardware_get_capabilities(vpehw);
    break;
  case VPE_HW_CMD_SUBSCRIBE_EVENT:
    rc = vpe_hardware_subcribe_v4l2_event(vpehw);
    break;
  case VPE_HW_CMD_UNSUBSCRIBE_EVENT:
    rc = vpe_hardware_unsubcribe_v4l2_event(vpehw);
    break;
  case VPE_HW_CMD_NOTIFY_EVENT:
    rc = vpe_hardware_notify_event_get_data(vpehw, cmd.u.event_data);
    break;
  case VPE_HW_CMD_STREAMON:
    rc = vpe_hardware_process_streamon(vpehw, cmd.u.stream_buff_list);
    break;
  case VPE_HW_CMD_STREAMOFF:
    rc = vpe_hardware_process_streamoff(vpehw, cmd.u.streamoff_identity);
    break;
  case VPE_HW_CMD_PROCESS_FRAME:
    rc = vpe_hardware_process_frame(vpehw, cmd.u.hw_params);
    break;
  default:
    CDBG_ERROR("%s:%d, error: bad command type=%d",
      __func__, __LINE__, cmd.type);
    rc = -EINVAL;
  }
  return rc;
}

/* vpe_hardware_get_capabilities:
 *
 * Description:
 *  Get hardware capabilities from kernel
 *
 **/
int32_t vpe_hardware_get_capabilities(vpe_hardware_t *vpehw)
{
  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(vpehw->mutex));
  /* TODO: read from hw reg */
  vpehw->caps.caps_mask = VPE_CAPS_SCALE | VPE_CAPS_CROP;

  /* MITCH TODO: these values are from vpe: */
  vpehw->caps.scaling_caps.max_scale_factor = 8.0;
  vpehw->caps.scaling_caps.min_scale_factor = 1.0/16.0;

  PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
  return 0;
}

/* vpe_hardware_process_streamon:
 *
 **/
int32_t vpe_hardware_process_streamon(vpe_hardware_t *vpehw,
  vpe_hardware_stream_buff_info_t *hw_strm_buff_info)
{
  int rc;
  uint32_t i;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  struct msm_vpe_stream_buff_info_t vpe_strm_buff_info;

  CDBG("%s:%d, streaming on\n", __func__, __LINE__);
  if (NULL == hw_strm_buff_info) {
    CDBG_ERROR("%s:%d] error hw_strm_buff_info:%p\n", __func__, __LINE__,
      hw_strm_buff_info);
    return -EINVAL;
  }

  /* Translate to msm stream buff list */
  vpe_strm_buff_info.identity = hw_strm_buff_info->identity;
  vpe_strm_buff_info.num_buffs = hw_strm_buff_info->num_buffs;
  vpe_strm_buff_info.buffer_info =
    malloc(sizeof(struct msm_vpe_buffer_info_t) *
    hw_strm_buff_info->num_buffs);
  if (NULL == vpe_strm_buff_info.buffer_info) {
    CDBG_ERROR("%s:%d] error allocating buffer info\n", __func__, __LINE__);
    return -ENOMEM;
  }

  for (i = 0; i < vpe_strm_buff_info.num_buffs; i++) {
    vpe_strm_buff_info.buffer_info[i].fd =
      hw_strm_buff_info->buffer_info[i].fd;
    vpe_strm_buff_info.buffer_info[i].index =
      hw_strm_buff_info->buffer_info[i].index;
    vpe_strm_buff_info.buffer_info[i].native_buff =
      hw_strm_buff_info->buffer_info[i].native_buff;
    vpe_strm_buff_info.buffer_info[i].offset =
      hw_strm_buff_info->buffer_info[i].offset;
    vpe_strm_buff_info.buffer_info[i].processed_divert =
      hw_strm_buff_info->buffer_info[i].processed_divert;
  }

  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(vpehw->mutex));

  v4l2_ioctl.len = sizeof(vpe_strm_buff_info);
  v4l2_ioctl.ioctl_ptr = (void *)&vpe_strm_buff_info;
  rc = ioctl(vpehw->subdev_fd, VIDIOC_MSM_VPE_ENQUEUE_STREAM_BUFF_INFO,
    &v4l2_ioctl);
  free(vpe_strm_buff_info.buffer_info);
  if (rc < 0) {
    CDBG_ERROR("%s:%d: v4l2 ioctl() failed, rc=%d\n", __func__, __LINE__, rc);
    PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
    return -EIO;
  }

  /* make stream_status valid for this stream */
  for (i=0; i<VPE_HARDWARE_MAX_STREAMS; i++) {
    if (vpehw->stream_status[i].valid == FALSE) {
      vpehw->stream_status[i].identity = hw_strm_buff_info->identity;
      vpehw->stream_status[i].pending_buf = 0;
      vpehw->stream_status[i].valid = TRUE;
      vpehw->stream_status[i].stream_off_pending = FALSE;
      break;
    }
  }
  PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
  CDBG("%s:%d, stream on done\n", __func__, __LINE__);
  return 0;
}


/* vpe_hardware_process_streamoff:
 *
 **/
int32_t vpe_hardware_process_streamoff(vpe_hardware_t *vpehw,
                                       uint32_t identity)
{
  int rc, i;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;

  CDBG("%s:%d, identity=0x%x", __func__, __LINE__, identity);
  PTHREAD_MUTEX_LOCK(&(vpehw->mutex));
  vpe_hardware_stream_status_t* stream_status =
    vpe_hardware_get_stream_status(vpehw, identity);
  if (!stream_status) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
    return -EFAULT;
  }
  stream_status->stream_off_pending = TRUE;
  /* wait for all pending buffers to be processed */
  while (stream_status->pending_buf != 0) {
    CDBG_LOW("%s:%d, waiting for pending buf, identity=0x%x, pending_buf=%d",
      __func__, __LINE__, identity, stream_status->pending_buf);
    pthread_cond_wait(&(vpehw->no_pending_cond), &(vpehw->mutex));
  }
  stream_status->stream_off_pending = FALSE;
  stream_status->valid = FALSE;
  PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));

  CDBG("%s:%d, pending buffers done, hw streaming off. identity=0x%x\n",
    __func__, __LINE__, identity);
  v4l2_ioctl.ioctl_ptr = (void *)&identity;
  v4l2_ioctl.len = sizeof(identity);
  rc = ioctl(vpehw->subdev_fd, VIDIOC_MSM_VPE_DEQUEUE_STREAM_BUFF_INFO,
    &v4l2_ioctl);
  if (rc < 0) {
    CDBG_ERROR("%s:%d: v4l2 ioctl() failed, rc=%d\n", __func__, __LINE__, rc);
    return -EIO;
  }
  CDBG("%s:%d, hw stream off done. identity=0x%x\n", __func__, __LINE__,
    identity);
  return 0;
}

/* vpe_hardware_subcribe_v4l2_event:
 *
 **/
static int32_t vpe_hardware_subcribe_v4l2_event(vpe_hardware_t *vpehw)
{
  int rc;
  struct v4l2_event_subscription sub;

  sub.id = vpehw->inst_id;
  sub.type = V4L2_EVENT_VPE_FRAME_DONE;
  rc = ioctl(vpehw->subdev_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
  if (rc < 0) {
    CDBG_ERROR("%s:%d: v4l2 ioctl() failed, rc=%d\n", __func__, __LINE__, rc);
    return -EIO;
  }

  vpehw->event_subs_info.valid = TRUE;
  vpehw->event_subs_info.id = sub.id;
  vpehw->event_subs_info.type = sub.type;
  return 0;
}

/* vpe_hardware_get_stream_status:
 *
 **/
static vpe_hardware_stream_status_t*
  vpe_hardware_get_stream_status(vpe_hardware_t* vpehw, uint32_t identity)
{
  int i;
  for (i=0; i<VPE_HARDWARE_MAX_STREAMS; i++) {
    if (vpehw->stream_status[i].valid == TRUE) {
      if (vpehw->stream_status[i].identity == identity) {
        return &(vpehw->stream_status[i]);
      }
    }
  }
  return NULL;
}

/* vpe_hardware_get_event_data:
 *
 **/
static int32_t
vpe_hardware_notify_event_get_data(vpe_hardware_t *vpehw,
                                   vpe_hardware_event_data_t *event_data)
{
  int rc;
  struct msm_vpe_frame_info_t frame;

  if(!event_data) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(vpehw->mutex));
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  struct v4l2_event event;
  rc = ioctl(vpehw->subdev_fd, VIDIOC_DQEVENT, &event);
  if (rc < 0) {
    CDBG_ERROR("%s:%d: v4l2 ioctl() failed, rc=%d\n", __func__, __LINE__, rc);
    PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
    return -EIO;
  }
  /* TODO: use event type to distinguish */
  v4l2_ioctl.ioctl_ptr = &frame;
  v4l2_ioctl.len = sizeof(struct msm_vpe_frame_info_t);
  rc = ioctl(vpehw->subdev_fd, VIDIOC_MSM_VPE_GET_EVENTPAYLOAD, &v4l2_ioctl);
  if (rc < 0) {
    CDBG_ERROR("%s:%d: v4l2 ioctl() failed, rc=%d\n", __func__, __LINE__, rc);
    PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
    return -EIO;
  }
  event_data->frame_id = frame.frame_id;
  event_data->buf_idx = frame.input_buffer_info.index;
  event_data->identity = frame.identity;
  event_data->cookie = frame.cookie;
  CDBG_LOW("%s:%d, frame_id=%d, buf_idx=%d, identity=0x%x", __func__, __LINE__,
    event_data->frame_id, event_data->buf_idx, event_data->identity);

  CDBG_LOW("%s:%d, in_time=%ld.%ldus out_time=%ld.%ldus, ",
    __func__, __LINE__, frame.in_time.tv_sec, frame.in_time.tv_usec,
    frame.out_time.tv_sec, frame.out_time.tv_usec);
  CDBG_LOW("%s:%d, processing time = %6ld us, ", __func__, __LINE__,
    (frame.out_time.tv_sec - frame.in_time.tv_sec)*1000000L +
    (frame.out_time.tv_usec - frame.in_time.tv_usec));

  /* update hardware stream_status */
  vpe_hardware_stream_status_t *stream_status =
    vpe_hardware_get_stream_status(vpehw, frame.identity);
  if (!stream_status) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
    return -EFAULT;
  }
  stream_status->pending_buf--;
  /* send signal to thread which is waiting on stream_off
     for pending buffer to be zero */
  if (stream_status->stream_off_pending == TRUE &&
    stream_status->pending_buf == 0) {
    CDBG("%s:%d, info: sending broadcast for pending stream-off",
      __func__, __LINE__);
    pthread_cond_broadcast(&(vpehw->no_pending_cond));
  }
  PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
  return 0;
}

/* vpe_hardware_create_hw_frame:
 *
 **/
static struct msm_vpe_frame_info_t*
vpe_hardware_create_hw_frame(vpe_hardware_t *vpehw,
                             struct vpe_frame_info_t *vpe_frame_info)
{
  int32_t rc;
  /* note: make sure to unlock this on each return path */
  struct msm_vpe_frame_info_t *msm_vpe_frame_info =
    malloc(sizeof(struct msm_vpe_frame_info_t));
  if (!msm_vpe_frame_info) {
    CDBG_ERROR("%s:%d, malloc failed\n", __func__, __LINE__);
    return NULL;
  }
  memset(msm_vpe_frame_info, 0, sizeof(struct msm_vpe_frame_info_t));
  /* create frame msg */
  msm_vpe_frame_info->client_id = 0;
  msm_vpe_frame_info->inst_id = vpehw->inst_id;
  vpe_params_prepare_frame_info(vpe_frame_info, msm_vpe_frame_info);
  return msm_vpe_frame_info;
}


/* vpe_hardware_destroy_hw_frame:
 *
 **/
static void vpe_hardware_destroy_hw_frame(
  struct msm_vpe_frame_info_t* msm_vpe_frame_info)
{
  if (!msm_vpe_frame_info) {
    CDBG_ERROR("%s:%d, warning: msm_vpe_frame_info=NULL\n",
      __func__, __LINE__);
    return;
  }
  /* free(msm_vpe_frame_info->vpe_cmd_msg); */
  free(msm_vpe_frame_info);
}

/* vpe_program_hardware_for_new_transaction
 *
 **/
static int32_t vpe_program_hardware_for_new_transaction
(vpe_hardware_t *vpehw,
 struct vpe_frame_info_t *frame_info,
 struct msm_vpe_frame_info_t *msm_frame_info)
{
  int rc;
  struct vpe_transaction_setup_t setup;
  memset(&setup, 0, sizeof(struct vpe_transaction_setup_t));

  vpe_config_pipeline(&setup, frame_info, msm_frame_info);

  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  v4l2_ioctl.ioctl_ptr = &setup;
  v4l2_ioctl.len = sizeof(struct vpe_transaction_setup_t);
  rc = ioctl(vpehw->subdev_fd, VIDIOC_MSM_VPE_TRANSACTION_SETUP, &v4l2_ioctl);
  if (rc < 0) {
    CDBG_ERROR("%s:%d, v4l2 ioctl() failed.\n", __func__, __LINE__);
    return -EIO;
  }
  return 0;
}

/* vpe_hardware_process_frame:
 *
 **/
static int32_t vpe_hardware_process_frame(vpe_hardware_t *vpehw,
  vpe_hardware_params_t *hw_params)
{
  int32_t rc = 0;
  int in_frame_fd;
  struct vpe_frame_info_t vpe_frame_info;
  struct msm_vpe_frame_info_t *msm_vpe_frame_info;
  memset(&vpe_frame_info, 0, sizeof(struct vpe_frame_info_t));
  if (!vpehw) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EINVAL;
  }

  CDBG_LOW("%s:%d, identity=0x%x", __func__, __LINE__, hw_params->identity);

  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(vpehw->mutex));
  vpe_hardware_stream_status_t *stream_status =
    vpe_hardware_get_stream_status(vpehw, hw_params->identity);
  if (!stream_status) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
    return -EFAULT;
  }

  /* TODO: cant do this, thread wont be able to release ack for this frame */
#if 0
  /* if a stream of pending for this identity, drop this frame and return */
  if (stream_status->stream_off_pending == TRUE) {
    CDBG("%s:%d, pending stream-off, frame dropped, identity=0x%x",
      __func__, __LINE__, hw_params->identity);
    PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
    return 0;
  }
#endif

  vpe_params_create_frame_info(hw_params, &vpe_frame_info);

  /* Translate the sysetm interface params to msm frame cfg params */
  vpe_frame_info.frame_id = hw_params->frame_id;
  vpe_frame_info.timestamp = hw_params->timestamp;
  vpe_frame_info.buff_index = hw_params->buffer_info.index;
  vpe_frame_info.native_buff = hw_params->buffer_info.native_buff;
  vpe_frame_info.cookie = hw_params->cookie;
  vpe_frame_info.identity = hw_params->identity;
  vpe_frame_info.frame_type = MSM_VPE_REALTIME_FRAME;
  vpe_frame_info.plane_info[0].src_fd = hw_params->buffer_info.fd;
  vpe_frame_info.plane_info[1].src_fd = hw_params->buffer_info.fd;
  /* TODO: Removal of this member needs careful kernel/userspace dependency */
  vpe_frame_info.plane_info[0].dst_fd = -1;
  vpe_frame_info.plane_info[1].dst_fd = -1;

  msm_vpe_frame_info = vpe_hardware_create_hw_frame(vpehw, &vpe_frame_info);
  if (!msm_vpe_frame_info) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    /* TODO: isn't this free'ing an invalid pointer? */
    /* vpe_hardware_destroy_hw_frame(msm_vpe_frame_info); */
    PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
    return -EFAULT;
  }

  rc = vpe_program_hardware_for_new_transaction(vpehw, &vpe_frame_info,
                                                msm_vpe_frame_info);
  if (rc < 0) {
    CDBG_ERROR("%s:%d, v4l2 ioctl() failed.\n", __func__, __LINE__);
    vpe_hardware_destroy_hw_frame(msm_vpe_frame_info);
    PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
    return -EIO;
  }

  /* send kernel ioctl for processing */
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  v4l2_ioctl.ioctl_ptr = msm_vpe_frame_info;
  v4l2_ioctl.len = sizeof(struct msm_vpe_frame_info_t);
  /* TODO: potential SEGFAULT issue caught in stability, at below line */
  rc = ioctl(vpehw->subdev_fd, VIDIOC_MSM_VPE_CFG, &v4l2_ioctl);
  if (rc < 0) {
    CDBG_ERROR("%s:%d, v4l2 ioctl() failed.\n", __func__, __LINE__);
    vpe_hardware_destroy_hw_frame(msm_vpe_frame_info);
    PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
    return -EIO;
  }
  CDBG_LOW("%s:%d, frame scheduled for processing, frame_id=%d, "
           "buf_idx=%d, identity=0x%x, w=%d, h=%d", __func__, __LINE__,
           msm_vpe_frame_info->frame_id,
           msm_vpe_frame_info->input_buffer_info.index,
           msm_vpe_frame_info->identity,
           msm_vpe_frame_info->strip_info.src_w,
           msm_vpe_frame_info->strip_info.src_h);
  vpe_hardware_destroy_hw_frame(msm_vpe_frame_info);

  /* update stream status */
  stream_status->pending_buf++;
  PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
  return 0;
}

/* vpe_hardware_unsubcribe_v4l2_event:
 *
 **/
static int32_t vpe_hardware_unsubcribe_v4l2_event(vpe_hardware_t *vpehw)
{
  int rc;
  struct v4l2_event_subscription sub;

  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(vpehw->mutex));
  if (vpehw->event_subs_info.valid == TRUE) {
    vpehw->event_subs_info.valid = FALSE;
    sub.id = vpehw->event_subs_info.id;
    sub.type = vpehw->event_subs_info.type;
    rc = ioctl(vpehw->subdev_fd, VIDIOC_UNSUBSCRIBE_EVENT, &sub);
    if (rc < 0) {
      CDBG_ERROR("%s:%d: v4l2 ioctl() failed, rc=%d\n", __func__, __LINE__, rc);
      PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
      return -EIO;
    }
  }
  PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
  return 0;
}

/* vpe_hardware_find_subdev:
 *
 **/
static int32_t vpe_hardware_find_subdev(vpe_hardware_t *vpehw)
{
  int i=0, rc=0;
  int fd;
  struct media_device_info mdev_info;
  char name[SUBDEV_NAME_SIZE_MAX];
  if (!vpehw) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(vpehw->mutex));
  vpehw->num_subdev = 0;
  while(1) {
    snprintf(name, sizeof(name), "/dev/media%d", i);
    fd = open(name, O_RDWR | O_NONBLOCK);
    if(fd < 0) {
      CDBG_LOW("%s:%d: no more media devices\n", __func__, __LINE__);
      break;
    }
    i++;
    rc = ioctl(fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (rc < 0) {
      CDBG_ERROR("%s:%d: ioctl() failed: %s\n", __func__, __LINE__,
        strerror(errno));
      close(fd);
      /* check for next device */
      continue;
    }
    if (strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model) != 0)) {
      /* not camera device */
      close(fd);
      continue;
    }
    int num_entities = 1;
    /* enumerate all entities in the media device */
    while(1) {
      if (vpehw->num_subdev >= MAX_VPE_DEVICES) {
        break;
      }
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities++;
      rc = ioctl(fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (rc < 0) {
        CDBG("%s:%d: done enumerating media entities\n", __func__, __LINE__);
        rc = 0;
        break;
      }
      CDBG_LOW("%s:%d: entity.name=%s entity.revision=%d\n",
        __func__, __LINE__, entity.name, entity.revision);
      if(entity.type == MEDIA_ENT_T_V4L2_SUBDEV &&
          entity.group_id == MSM_CAMERA_SUBDEV_VPE) {
        CDBG("%s:%d: VPE entity found: name=%s\n",
           __func__, __LINE__, entity.name);
        vpehw->subdev_ids[vpehw->num_subdev] = entity.revision;
        vpehw->num_subdev++;
      }
    }
    close(fd);
  }
  if (vpehw->num_subdev == 0) {
    CDBG_ERROR("%s:%d: no vpe device found.\n", __func__, __LINE__);
    PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
    return -ENODEV;
  }
  PTHREAD_MUTEX_UNLOCK(&(vpehw->mutex));
  return 0;
}
