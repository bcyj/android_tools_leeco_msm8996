/*============================================================================

  Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "eztune_diagnostics.h"
#include "cpp_hardware.h"
#include "cpp_module.h"
#include "cpp_log.h"
#include <linux/media.h>
#include <media/msmb_generic_buf_mgr.h>
#include <fcntl.h>
#include <errno.h>
#include "server_debug.h"
#ifdef _ANDROID_
#include <cutils/properties.h>
#endif
#define ATRACE_TAG ATRACE_TAG_CAMERA
#include <cutils/trace.h>
#if 0
#define CPPDUMP(fmt, args...) \
  ALOGE("CPPMETA "fmt"\n", ##args)
#else
#define CPPDUMP(fmt, args...) \
  do {} while (0);
#endif

/* cpp_hardware_select_fw_version:
*
* @hw_version: cpp hw version
* @fw_version: micro firmware version
* @fw_filename: firmware file name
* Description:
*     Returns firmware version and file name for corresponding
*     CPP hardware version
* Return: int
*     0: if success, negative value otherwise.
**/
int32_t cpp_hardware_select_fw_version(uint32_t hw_version,
  uint32_t *fw_version, char *fw_filename)
{
  if (!fw_filename || !fw_version) {
    CDBG_ERROR("fw_filename=%p, fw_version=%p", fw_filename, fw_version);
    return -EINVAL;
  }
  switch (hw_version) {
  case CPP_HW_VERSION_1_1_0:
    *fw_version = CPP_FW_VERSION_1_2_0;
    break;
  case CPP_HW_VERSION_1_1_1:
    *fw_version = CPP_FW_VERSION_1_2_0;
    break;
  case CPP_HW_VERSION_2_0_0:
    *fw_version = CPP_FW_VERSION_1_2_0;
    break;
  case CPP_HW_VERSION_4_0_0:
    *fw_version = CPP_FW_VERSION_1_4_0;
    break;
  case CPP_HW_VERSION_4_1_0:
  case CPP_HW_VERSION_4_2_0:
    *fw_version = CPP_FW_VERSION_1_4_0;
    break;
  default:
    CDBG_ERROR("invalid hw version=0x%08x\n", hw_version);
    *fw_version = 0xffffffff;
    return -EINVAL;
  }
  snprintf(fw_filename, CPP_MAX_FW_NAME_LEN,
    "cpp_firmware_v%d_%d_%d.fw",
    CPP_GET_FW_MAJOR_VERSION(*fw_version),
    CPP_GET_FW_MINOR_VERSION(*fw_version),
    CPP_GET_FW_STEP_VERSION(*fw_version));
  return 0;
}

/* cpp_hardware_create:
 *
 *  creates new cpp_hardware instance. Finds the cpp subdev in kernel,
 *  allocates memory and initializes the structure.
 *
 **/
cpp_hardware_t* cpp_hardware_create()
{
  cpp_hardware_t *cpphw;
  int rc;
  cpphw = (cpp_hardware_t *) malloc(sizeof(cpp_hardware_t));
  if(!cpphw) {
    CDBG_ERROR("%s:%d: malloc() failed\n", __func__, __LINE__);
    return NULL;
  }
  memset(cpphw, 0x00, sizeof(cpp_hardware_t));
  rc = cpp_hardware_find_subdev(cpphw);
  if (rc < 0) {
    CDBG_ERROR("%s:%d: error: cannot find cpp subdev\n", __func__, __LINE__);
    free(cpphw);
    return NULL;
  }
  cpphw->subdev_opened = FALSE;
  cpphw->status = CPP_HW_STATUS_INVALID;

  /* initialize the stream_status */
  int i;
  for (i=0; i<CPP_HARDWARE_MAX_STREAMS; i++) {
    cpphw->stream_status[i].valid = FALSE;
    cpphw->stream_status[i].identity = 0x00;
    cpphw->stream_status[i].pending_buf = 0;
    cpphw->stream_status[i].stream_off_pending = FALSE;
  }
  pthread_cond_init(&(cpphw->no_pending_cond), NULL);
  pthread_mutex_init(&(cpphw->mutex), NULL);
  return cpphw;
}

/* cpp_hardware_open_subdev:
 *
 *  opens the cpp subdev and updates instance info.
 *  updates the hardware status.
 *  currently only one cpp subdevice is supported.
 **/
int32_t cpp_hardware_open_subdev(cpp_hardware_t *cpphw)
{
  int fd, rc=0;
  char dev_name[SUBDEV_NAME_SIZE_MAX];
  if(!cpphw) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  /* make sure all code-paths unlock this mutex */
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  if (cpphw->subdev_opened == TRUE) {
    CDBG_HIGH("%s:%d: subdev already open\n", __func__, __LINE__);
    rc = -EFAULT;
    goto error_mutex;
  }
  snprintf(dev_name, sizeof(dev_name), "/dev/v4l-subdev%d",
    cpphw->subdev_ids[0]);
  fd = open(dev_name, O_RDWR | O_NONBLOCK);
  if (fd >= MAX_FD_PER_PROCESS) {
    dump_list_of_daemon_fd();
    fd = -1;
  }
  if (fd < 0) {
    CDBG_ERROR("%s:%d: error: cannot open cpp subdev: %s\n",
      __func__, __LINE__, dev_name);
    rc = -EIO;
    goto error_mutex;
  }
  cpphw->subdev_fd = fd;
  cpphw->subdev_opened = TRUE;
  /* get the instance info */
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  struct msm_cpp_frame_info_t inst_info;
  memset(&inst_info, 0x00, sizeof(struct msm_cpp_frame_info_t));
  v4l2_ioctl.ioctl_ptr = &inst_info;
  v4l2_ioctl.len = sizeof(inst_info);
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_GET_INST_INFO, &v4l2_ioctl);
  if (rc < 0) {
    CDBG_ERROR("%s:%d: v4l2 ioctl() failed, rc=%d\n", __func__, __LINE__, rc);
    rc = -EIO;
    goto error_open;
  }
  cpphw->inst_id = inst_info.inst_id;
  /* update hw state */
  if (cpphw->status == CPP_HW_STATUS_FW_LOADED) {
    cpphw->status = CPP_HW_STATUS_READY;
  } else {
    cpphw->status = CPP_HW_STATUS_INVALID;
  }
  CDBG("%s:%d, cpp subdev opened, subdev_fd=%d, inst_id=0x%x, status=%d",
    __func__, __LINE__, cpphw->subdev_fd, cpphw->inst_id, cpphw->status);

  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;

error_open:
  close(fd);
error_mutex:
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return rc;
}

/* cpp_hardware_close_subdev:
 *
 *  close the cpp subdev and update hardware status accordingly
 *
 **/
int32_t cpp_hardware_close_subdev(cpp_hardware_t *cpphw)
{
  if(!cpphw) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  close(cpphw->subdev_fd);
  cpphw->subdev_opened = FALSE;
  /* if status is ready or busy, fw would be loaded */
  if (cpphw->status != CPP_HW_STATUS_INVALID) {
    cpphw->status = CPP_HW_STATUS_FW_LOADED;
  } else {
    cpphw->status = CPP_HW_STATUS_INVALID;
  }
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}

/* cpp_hardware_destroy:
 *
 *  destroy the hardware data structure and free memory
 *
 **/
int32_t cpp_hardware_destroy(cpp_hardware_t *cpphw)
{
  if(!cpphw) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  if (cpphw->subdev_opened == TRUE) {
    cpp_hardware_close_subdev(cpphw);
  }
  pthread_cond_destroy(&(cpphw->no_pending_cond));
  pthread_mutex_destroy(&(cpphw->mutex));
  free(cpphw);
  return 0;
}

/* cpp_hardware_get_status:
 *
 *  get current status of the hardware. Hardware structure access is
 *  protected by the mutex
 *
 **/
cpp_hardware_status_t cpp_hardware_get_status(cpp_hardware_t *cpphw)
{
  cpp_hardware_status_t status;
  if (!cpphw) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return FALSE;
  }
  int32_t num_pending=0;
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  int i;
  for (i=0; i<CPP_HARDWARE_MAX_STREAMS; i++) {
    if (cpphw->stream_status[i].valid) {
      num_pending += cpphw->stream_status[i].pending_buf;
    }
  }
  if (num_pending < CPP_HARDWARE_MAX_PENDING_BUF) {
    status = CPP_HW_STATUS_READY;
  } else {
    status = CPP_HW_STATUS_BUSY;
  }
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return status;
}

/* cpp_hardware_send_buf_done:
 *
 *  Send buffer done to the processed divert buffer. All accesses to the
 *  shared hardware data structure are protected by mutex.
 *
 **/
static int32_t cpp_hardware_send_buf_done(cpp_hardware_t *cpphw,
  cpp_hardware_event_data_t *event_data)
{
  int rc;
  struct msm_pproc_queue_buf_info queue_buf_info;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;

  memset(&queue_buf_info, 0, sizeof(struct msm_pproc_queue_buf_info));
  queue_buf_info.buff_mgr_info.session_id =
    ((event_data->identity >> 16) & 0xFFFF);
  queue_buf_info.buff_mgr_info.stream_id = (event_data->identity & 0xFFFF);
  queue_buf_info.buff_mgr_info.frame_id = event_data->frame_id;
  queue_buf_info.buff_mgr_info.timestamp = event_data->timestamp;
  queue_buf_info.buff_mgr_info.index = event_data->out_buf_idx;
  queue_buf_info.is_buf_dirty = event_data->is_buf_dirty;

  v4l2_ioctl.ioctl_ptr = &queue_buf_info;
  v4l2_ioctl.len = sizeof(struct msm_pproc_queue_buf_info);
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_QUEUE_BUF, &v4l2_ioctl);
  if (rc < 0) {
    CDBG_ERROR("%s:%d: v4l2 ioctl() failed, rc=%d\n", __func__, __LINE__, rc);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EIO;
  }
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}

/* cpp_hardware_set_clock:
 *
 **/
static int32_t cpp_hardware_set_clock(cpp_hardware_t *cpphw,
  cpp_hardware_clock_settings_t *clock_settings)
{
  int rc=0;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  struct msm_cpp_frame_info_t inst_info;
  char dev_name[SUBDEV_NAME_SIZE_MAX];
  if(!cpphw) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  /* make sure all code-paths unlock this mutex */
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  v4l2_ioctl.ioctl_ptr = clock_settings;
  v4l2_ioctl.len = sizeof(cpp_hardware_clock_settings_t);
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_SET_CLOCK, &v4l2_ioctl);
  if (rc < 0) {
    CDBG_ERROR("%s:%d: v4l2 ioctl() failed, rc=%d\n", __func__, __LINE__, rc);
    rc = -EIO;
  }

error_mutex:
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return rc;
}


/* cpp_hardware_process_command:
 *
 *  processes the command given to the hardware. Hardware state is
 *  updated during the process. All accesses to the shared hardware
 *  data structure are protected by mutex.
 *
 **/
int32_t cpp_hardware_process_command(cpp_hardware_t *cpphw,
  cpp_hardware_cmd_t cmd)
{
  int rc = 0;
  if (!cpphw) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  if (cpphw->subdev_opened == FALSE) {
    CDBG_ERROR("%s:%d: failed, subdev not opened\n", __func__, __LINE__);
    return -EINVAL;
  }

  switch (cmd.type) {
  case CPP_HW_CMD_GET_CAPABILITIES:
    rc = cpp_hardware_get_capabilities(cpphw);
    break;
  case CPP_HW_CMD_SUBSCRIBE_EVENT:
    rc = cpp_hardware_subcribe_v4l2_event(cpphw);
    break;
  case CPP_HW_CMD_UNSUBSCRIBE_EVENT:
    rc = cpp_hardware_unsubcribe_v4l2_event(cpphw);
    break;
  case CPP_HW_CMD_NOTIFY_EVENT:
    rc = cpp_hardware_notify_event_get_data(cpphw, cmd.u.event_data);
    break;
  case CPP_HW_CMD_STREAMON:
    rc = cpp_hardware_process_streamon(cpphw, cmd.u.stream_buff_list);
    break;
  case CPP_HW_CMD_STREAMOFF:
    rc = cpp_hardware_process_streamoff(cpphw, cmd.u.streamoff_data);
    break;
  case CPP_HW_CMD_LOAD_FIRMWARE:
    rc = cpp_hardware_load_firmware(cpphw);
    break;
  case CPP_HW_CMD_PROCESS_FRAME:
    rc = cpp_hardware_process_frame(cpphw, cmd.u.hw_params);
    break;
  case CPP_HW_CMD_QUEUE_BUF:
    rc = cpp_hardware_send_buf_done(cpphw, cmd.u.event_data);
    break;
  case CPP_HW_CMD_SET_CLK:
    rc = cpp_hardware_set_clock(cpphw, &cmd.u.clock_settings);
    break;
  case CPP_HW_CMD_BUF_UPDATE:
    rc = cpp_hardware_update_buffer_list(cpphw, cmd.u.hw_params);
    break;
  default:
    CDBG_ERROR("%s:%d, error: bad command type=%d",
      __func__, __LINE__, cmd.type);
    rc = -EINVAL;
  }
  return rc;
}

/* cpp_hardware_get_capabilities:
 *
 * Description:
 *  Get hardware capabilities from kernel
 *
 **/
int32_t cpp_hardware_get_capabilities(cpp_hardware_t *cpphw)
{
  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  /* TODO: read from hw reg */
  cpphw->caps.caps_mask = CPP_CAPS_DENOISE | CPP_CAPS_SCALE |
                          CPP_CAPS_SHARPENING | CPP_CAPS_CROP |
                          CPP_CAPS_ROTATION | CPP_CAPS_FLIP;

  cpphw->caps.scaling_caps.max_scale_factor = 8.0;
  cpphw->caps.scaling_caps.min_scale_factor = 1.0/16.0;
  cpphw->caps.rotation_caps = ROTATION_90 | ROTATION_180 | ROTATION_270;
  cpphw->caps.filp_caps.h_flip = TRUE;
  cpphw->caps.filp_caps.v_flip = TRUE;
  cpphw->caps.sharpness_caps.max_value = CPP_MAX_SHARPNESS;
  cpphw->caps.sharpness_caps.min_value = CPP_MIN_SHARPNESS;
  cpphw->caps.sharpness_caps.def_value = CPP_DEFAULT_SHARPNESS;
  cpphw->caps.sharpness_caps.step = (CPP_MAX_SHARPNESS - CPP_MIN_SHARPNESS) /
                                      CPP_TOTAL_SHARPNESS_LEVELS;

  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}

double cpp_get_sharpness_ratio(int32_t sharpness)
{
  return (double)(sharpness) / CPP_DEFAULT_SHARPNESS;
}

/* cpp_hardware_load_firmware:
 * Description:
 *   Calls kernel ioctl to load the firmware for CPP micro controller. Updates
 *   the states
 *
 **/
static int32_t cpp_hardware_load_firmware(cpp_hardware_t *cpphw)
{
  int rc;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  CDBG("%s:%d, loading firmware", __func__, __LINE__);

  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));

  /* firmware should be loaded only once when state is invalid */
  if (cpphw->status != CPP_HW_STATUS_INVALID) {
    CDBG_ERROR("%s:%d: bad hw status=%d\n", __func__, __LINE__, cpphw->status);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EINVAL;
  }

  v4l2_ioctl.ioctl_ptr = (void *)&(cpphw->hwinfo);
  v4l2_ioctl.len = sizeof(cpp_hardware_info_t);
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_GET_HW_INFO,
    &v4l2_ioctl);
  if (rc < 0) {
    CDBG_ERROR("%s:%d: v4l2 ioctl() failed, rc=%d\n", __func__, __LINE__, rc);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EIO;
  }
  CDBG_ERROR("%s:%d, cpphw->hwinfo.version = 0x%x", __func__, __LINE__,
     cpphw->hwinfo.version);
  /* Get corresponding fw version based on hw version */
  char fw_filename[CPP_MAX_FW_NAME_LEN];
  rc = cpp_hardware_select_fw_version(cpphw->hwinfo.version,
    &cpphw->fw_version, fw_filename);
  CDBG_HIGH("hw_version=0x%08x, fw_version=0x%08x",
    cpphw->hwinfo.version, cpphw->fw_version);
  if (rc < 0) {
    CDBG_ERROR("failed");
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EFAULT;
  }
  v4l2_ioctl.ioctl_ptr = fw_filename;
  v4l2_ioctl.len = strlen(fw_filename);
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_LOAD_FIRMWARE,
    &v4l2_ioctl);
  if (rc < 0) {
    CDBG_ERROR("%s:%d: v4l2 ioctl() failed, rc=%d\n", __func__, __LINE__, rc);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EIO;
  }
  cpphw->status = CPP_HW_STATUS_FW_LOADED;
  CDBG("%s:%d, cpp firmware loaded", __func__, __LINE__);
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}


/* cpp_hardware_update_buffer_list
 *
 *  @cpphw - structure holding hardware parameters
 *  @hw_strm_buff_info - structure holding information for the buufers that
 *             have to be appended to the buffer queue
 *
 *  Send all the information for the buffers that have to be added to the
 *  stream queue to the kernel.
 *
 *  Returns 0 on success
 *
 **/
int32_t cpp_hardware_update_buffer_list(cpp_hardware_t *cpphw,
  cpp_hardware_stream_buff_info_t *hw_strm_buff_info)
{
  int rc;
  uint32_t i;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  struct msm_cpp_stream_buff_info_t cpp_strm_buff_info;

  if (NULL == hw_strm_buff_info) {
    CDBG_ERROR("%s:%d] error hw_strm_buff_info:%p\n", __func__, __LINE__,
      hw_strm_buff_info);
    return -EINVAL;
  }

  /* Translate to msm stream buff list */
   cpp_strm_buff_info.identity = hw_strm_buff_info->identity;
   cpp_strm_buff_info.num_buffs = hw_strm_buff_info->num_buffs;
   cpp_strm_buff_info.buffer_info =
     malloc(sizeof(struct msm_cpp_buffer_info_t) *
     hw_strm_buff_info->num_buffs);
   if (NULL == cpp_strm_buff_info.buffer_info) {
     CDBG_ERROR("%s:%d] error allocating buffer info\n", __func__, __LINE__);
     return -ENOMEM;
   }

   for (i = 0; i < cpp_strm_buff_info.num_buffs; i++) {
     cpp_strm_buff_info.buffer_info[i].fd =
       hw_strm_buff_info->buffer_info[i].fd;
     cpp_strm_buff_info.buffer_info[i].index =
       hw_strm_buff_info->buffer_info[i].index;
     cpp_strm_buff_info.buffer_info[i].native_buff =
       hw_strm_buff_info->buffer_info[i].native_buff;
     cpp_strm_buff_info.buffer_info[i].offset =
       hw_strm_buff_info->buffer_info[i].offset;
     cpp_strm_buff_info.buffer_info[i].processed_divert =
       hw_strm_buff_info->buffer_info[i].processed_divert;
   }

   /* note: make sure to unlock this on each return path */
   PTHREAD_MUTEX_LOCK(&(cpphw->mutex));

   v4l2_ioctl.len = sizeof(cpp_strm_buff_info);
   v4l2_ioctl.ioctl_ptr = (void *)&cpp_strm_buff_info;
   rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_APPEND_STREAM_BUFF_INFO,
     &v4l2_ioctl);
   free(cpp_strm_buff_info.buffer_info);
   if (rc < 0) {
     CDBG_ERROR("%s:%d: v4l2 ioctl() failed, rc=%d\n", __func__, __LINE__, rc);
     PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
     return -EIO;
   }
   PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));

   return 0;
}


/* cpp_hardware_process_streamon:
 *
 **/
int32_t cpp_hardware_process_streamon(cpp_hardware_t *cpphw,
  cpp_hardware_stream_buff_info_t *hw_strm_buff_info)
{
  int rc;
  uint32_t i;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl, v4l2_ioctl_iommu;
  struct msm_cpp_stream_buff_info_t cpp_strm_buff_info;
  struct msm_camera_smmu_attach_type cpp_attach_info;
  CDBG("%s:%d, streaming on\n", __func__, __LINE__);
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  if (cpphw->num_iommu_cnt == 0) {
    cpp_attach_info.attach = NON_SECURE_MODE;
    v4l2_ioctl_iommu.len = sizeof(cpp_attach_info);
    v4l2_ioctl_iommu.ioctl_ptr = (void*)&cpp_attach_info;
    rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_IOMMU_ATTACH,
               &v4l2_ioctl_iommu);
    if (rc < 0) {
      CDBG_ERROR("%s:%d: IOMMMU Attach v4l2 ioctl() failed, rc=%d\n",
        __func__, __LINE__, rc);
      PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
      return -EIO;
    }
    cpphw->num_iommu_cnt++;
  } else
    cpphw->num_iommu_cnt++;

  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  if (NULL == hw_strm_buff_info) {
    CDBG_ERROR("%s:%d] error hw_strm_buff_info:%p\n", __func__, __LINE__,
      hw_strm_buff_info);
    return -EINVAL;
  }

  /* Translate to msm stream buff list */
  cpp_strm_buff_info.identity = hw_strm_buff_info->identity;
  cpp_strm_buff_info.num_buffs = hw_strm_buff_info->num_buffs;
  cpp_strm_buff_info.buffer_info =
    malloc(sizeof(struct msm_cpp_buffer_info_t) *
    hw_strm_buff_info->num_buffs);
  if (NULL == cpp_strm_buff_info.buffer_info) {
    CDBG_ERROR("%s:%d] error allocating buffer info\n", __func__, __LINE__);
    return -ENOMEM;
  }

  for (i = 0; i < cpp_strm_buff_info.num_buffs; i++) {
    cpp_strm_buff_info.buffer_info[i].fd =
      hw_strm_buff_info->buffer_info[i].fd;
    cpp_strm_buff_info.buffer_info[i].index =
      hw_strm_buff_info->buffer_info[i].index;
    cpp_strm_buff_info.buffer_info[i].native_buff =
      hw_strm_buff_info->buffer_info[i].native_buff;
    cpp_strm_buff_info.buffer_info[i].offset =
      hw_strm_buff_info->buffer_info[i].offset;
    cpp_strm_buff_info.buffer_info[i].processed_divert =
      hw_strm_buff_info->buffer_info[i].processed_divert;
  }

  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));

  v4l2_ioctl.len = sizeof(cpp_strm_buff_info);
  v4l2_ioctl.ioctl_ptr = (void *)&cpp_strm_buff_info;
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_ENQUEUE_STREAM_BUFF_INFO,
    &v4l2_ioctl);
  free(cpp_strm_buff_info.buffer_info);
  if (rc < 0) {
    CDBG_ERROR("%s:%d: v4l2 ioctl() failed, rc=%d\n", __func__, __LINE__, rc);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EIO;
  }

  /* make stream_status valid for this stream */
  for (i=0; i<CPP_HARDWARE_MAX_STREAMS; i++) {
    if (cpphw->stream_status[i].valid == FALSE) {
      cpphw->stream_status[i].identity = hw_strm_buff_info->identity;
      cpphw->stream_status[i].pending_buf = 0;
      cpphw->stream_status[i].valid = TRUE;
      cpphw->stream_status[i].stream_off_pending = FALSE;
      break;
    }
  }
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  CDBG("%s:%d, stream on done\n", __func__, __LINE__);
  return 0;
}


/* cpp_hardware_process_streamoff:
 *
 **/
int32_t cpp_hardware_process_streamoff(cpp_hardware_t *cpphw,
  cpp_hardware_streamoff_event_t streamoff_data)
{
  int rc, i;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl, v4l2_ioctl_iommu;
  struct msm_camera_smmu_attach_type cpp_attach_info;

  CDBG("%s:%d, identity=0x%x", __func__, __LINE__,
    streamoff_data.streamoff_identity);
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  cpp_hardware_stream_status_t* stream_status =
    cpp_hardware_get_stream_status(cpphw, streamoff_data.streamoff_identity);
  if (!stream_status) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EFAULT;
  }
  stream_status->stream_off_pending = TRUE;
  /* wait for all pending buffers to be processed */
  while (stream_status->pending_buf != 0) {
    CDBG_LOW("%s:%d, waiting for pending buf, identity=0x%x, pending_buf=%d",
      __func__, __LINE__, streamoff_data.streamoff_identity,
      stream_status->pending_buf);
    pthread_cond_wait(&(cpphw->no_pending_cond), &(cpphw->mutex));
  }
  stream_status->stream_off_pending = FALSE;
  stream_status->valid = FALSE;

  cpp_hardware_stream_status_t* duplicate_stream_status =
    cpp_hardware_get_stream_status(cpphw,
    streamoff_data.duplicate_identity);
  CDBG("%s:%d] skip_iden:0x%x, duplicate_stream_status:%p\n",
    __func__, __LINE__, streamoff_data.duplicate_identity,
    duplicate_stream_status);

  if (duplicate_stream_status) {
    duplicate_stream_status->stream_off_pending = TRUE;
    /* wait for all pending buffers to be processed */
    while (duplicate_stream_status->pending_buf != 0) {
      CDBG_LOW("%s:%d, waiting for pending buf, identity=0x%x, pending_buf=%d, wait_identity:0x%x\n",
        __func__, __LINE__, streamoff_data.streamoff_identity,
        duplicate_stream_status->pending_buf, duplicate_stream_status->identity);
      pthread_cond_wait(&(cpphw->no_pending_cond), &(cpphw->mutex));
    }
    duplicate_stream_status->stream_off_pending = FALSE;
  }

  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));

  CDBG("%s:%d, pending buffers done, hw streaming off. identity=0x%x\n",
    __func__, __LINE__, streamoff_data.streamoff_identity);
  v4l2_ioctl.ioctl_ptr = (void *)&streamoff_data.streamoff_identity;
  v4l2_ioctl.len = sizeof(streamoff_data.streamoff_identity);
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_DEQUEUE_STREAM_BUFF_INFO,
    &v4l2_ioctl);
  if (rc < 0) {
    CDBG_ERROR("%s:%d: v4l2 ioctl() failed, rc=%d\n", __func__, __LINE__, rc);
    return -EIO;
  }
  CDBG("%s:%d, hw stream off done. identity=0x%x\n", __func__, __LINE__,
    streamoff_data.streamoff_identity);
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  if (cpphw->num_iommu_cnt > 0) {
    cpphw->num_iommu_cnt--;
  } else {
    CDBG_ERROR("%s: INVALID IOMMU cnt=%d\n",__func__, cpphw->num_iommu_cnt);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EIO;
  }
  if (cpphw->num_iommu_cnt == 0) {
    cpp_attach_info.attach = NON_SECURE_MODE;
    v4l2_ioctl_iommu.ioctl_ptr = (void*)&cpp_attach_info;
    v4l2_ioctl_iommu.len = sizeof(cpp_attach_info);
    rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_IOMMU_DETACH,
               &v4l2_ioctl_iommu);
    if (rc < 0) {
      CDBG_ERROR("%s:%d: IOMMMU detach v4l2 ioctl() failed, rc=%d\n",
        __func__, __LINE__, rc);
      PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
      return -EIO;
    }
  }
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}

/* cpp_hardware_subcribe_v4l2_event:
 *
 **/
static int32_t cpp_hardware_subcribe_v4l2_event(cpp_hardware_t *cpphw)
{
  int rc;
  struct v4l2_event_subscription sub;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;

  sub.id = cpphw->inst_id;
  sub.type = V4L2_EVENT_CPP_FRAME_DONE;
  rc = ioctl(cpphw->subdev_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
  if (rc < 0) {
    CDBG_ERROR("%s:%d: v4l2 ioctl() failed, rc=%d\n", __func__, __LINE__, rc);
    return -EIO;
  }

  cpphw->event_subs_info.valid = TRUE;
  cpphw->event_subs_info.id = sub.id;
  cpphw->event_subs_info.type = sub.type;
  return 0;
}

/* cpp_hardware_get_stream_status:
 *
 **/
static cpp_hardware_stream_status_t*
  cpp_hardware_get_stream_status(cpp_hardware_t* cpphw, uint32_t identity)
{
  int i;
  for (i=0; i<CPP_HARDWARE_MAX_STREAMS; i++) {
    if (cpphw->stream_status[i].valid == TRUE) {
      if (cpphw->stream_status[i].identity == identity) {
        return &(cpphw->stream_status[i]);
      }
    }
  }
  return NULL;
}

/* cpp_hardware_get_event_data:
 *
 **/
static int32_t cpp_hardware_notify_event_get_data(cpp_hardware_t *cpphw,
  cpp_hardware_event_data_t *event_data)
{
  int rc;
  struct msm_cpp_frame_info_t frame;
  ATRACE_INT("Camera:CPP",0);
  if(!event_data) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  struct v4l2_event event;
  rc = ioctl(cpphw->subdev_fd, VIDIOC_DQEVENT, &event);
  if (rc < 0) {
    CDBG_ERROR("%s:%d: v4l2 ioctl() failed, rc=%d\n", __func__, __LINE__, rc);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EIO;
  }
  /* TODO: use event type to distinguish */
  v4l2_ioctl.ioctl_ptr = &frame;
  v4l2_ioctl.len = sizeof(struct msm_cpp_frame_info_t);
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_GET_EVENTPAYLOAD, &v4l2_ioctl);
  if (rc < 0) {
    CDBG_ERROR("%s:%d: v4l2 ioctl() failed, rc=%d\n", __func__, __LINE__, rc);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EIO;
  }
  event_data->frame_id = frame.frame_id;
  event_data->buf_idx = frame.input_buffer_info.index;
  event_data->out_buf_idx = frame.output_buffer_info[0].index;
  event_data->out_fd = frame.output_buffer_info[0].fd;
  event_data->identity = frame.identity;
  event_data->timestamp = frame.timestamp;
  event_data->cookie = frame.cookie;
  CDBG_LOW("%s:%d, frame_id=%d, buf_idx=%d, identity=0x%x", __func__, __LINE__,
    event_data->frame_id, event_data->buf_idx, event_data->identity);

  CDBG_LOW("%s:%d, in_time=%ld.%ldus out_time=%ld.%ldus, ",
    __func__, __LINE__, frame.in_time.tv_sec, frame.in_time.tv_usec,
    frame.out_time.tv_sec, frame.out_time.tv_usec);
  CDBG("%s:%d, processing time = %6ld us,frame_id=%d,input_buf_idx=%d,"
    "output_buf_idx = %d,identity = 0x%x",
    __func__, __LINE__,
    (frame.out_time.tv_sec - frame.in_time.tv_sec)*1000000L +
    (frame.out_time.tv_usec - frame.in_time.tv_usec),event_data->frame_id,
    event_data->buf_idx,event_data->out_buf_idx,event_data->identity);
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}

/* cpp_hardware_create_hw_frame:
 *
 **/
static struct msm_cpp_frame_info_t*
  cpp_hardware_create_hw_frame(cpp_hardware_t *cpphw,
  struct cpp_frame_info_t *cpp_frame_info)
{
  int32_t rc;
  /* note: make sure to unlock this on each return path */
  struct msm_cpp_frame_info_t *msm_cpp_frame_info =
    malloc(sizeof(struct msm_cpp_frame_info_t));
  if (!msm_cpp_frame_info) {
    CDBG_ERROR("%s:%d, malloc failed\n", __func__, __LINE__);
    return NULL;
  }
  memset(msm_cpp_frame_info, 0, sizeof(struct msm_cpp_frame_info_t));
  /* create frame msg */
  msm_cpp_frame_info->client_id = 0;
  msm_cpp_frame_info->inst_id = cpphw->inst_id;
  cpp_params_prepare_frame_info(cpphw, cpp_frame_info, msm_cpp_frame_info);
  return msm_cpp_frame_info;
}

/* cpp_hardware_destroy_hw_frame:
 *
 **/
static void cpp_hardware_destroy_hw_frame(
  struct msm_cpp_frame_info_t* msm_cpp_frame_info)
{
  if (!msm_cpp_frame_info) {
    CDBG_ERROR("%s:%d, warning: msm_cpp_frame_info=NULL\n",
      __func__, __LINE__);
    return;
  }
  free(msm_cpp_frame_info->cpp_cmd_msg);
  free(msm_cpp_frame_info);
}

/* cpp_hardware_get_fw_version:
*
* @cpphw: pointer to cpp_hardware_t that has info about CPP HW
* Description:
*     Returns firmware version
* Return: int
*     CPP firmware version
**/
uint32_t cpp_hardware_get_fw_version(cpp_hardware_t *cpphw)
{
  return cpphw->fw_version;
}

/* cpp_hardware_fill_bus_message_params:
 *
 **/
static int32_t cpp_hardware_dump_metadata(cpp_hardware_t *cpphw,
  cpp_hardware_params_t *hw_params,
  struct msm_cpp_frame_info_t *msm_cpp_frame_info,
  struct cpp_frame_info_t *cpp_frame_info)
{
  uint32_t                i = 0;
  int32_t                 enabled = 0;
  char                    value[PROPERTY_VALUE_MAX];
  int32_t                 meta_frame_count = 0;
  pproc_meta_data_t      *meta_data;
  cpp_info_t             *cpp_meta_info;
  fe_config_t            *fe_config;
  cpp_module_hw_cookie_t *cookie;
  uint32_t                component_revision_no = 0x0;

  if (!hw_params || !msm_cpp_frame_info) {
    CDBG_ERROR("%s:%d invalid params hw_params %p frame info %p\n",
      __func__, __LINE__, hw_params, msm_cpp_frame_info);
    return -EINVAL;
  }

  property_get("persist.camera.dumpmetadata", value, "0");
  enabled = atoi(value);
  cookie = hw_params->cookie;
  if (!cookie || !cookie->meta_data) {
    return 0;
  }
  meta_data = (pproc_meta_data_t *)cookie->meta_data;
  meta_data->entry[PPROC_META_DATA_CPP_IDX].dump_type =
    PPROC_META_DATA_INVALID;
  meta_data->entry[PPROC_META_DATA_FE_IDX].dump_type =
    PPROC_META_DATA_INVALID;
  meta_data->entry[PPROC_META_DATA_CPP_IDX].pproc_meta_dump = NULL;
  meta_data->entry[PPROC_META_DATA_FE_IDX].pproc_meta_dump = NULL;

  if (enabled == 0) {
    return 0;
  }

  cpp_meta_info = (cpp_info_t *)malloc(sizeof(cpp_info_t));
  if (!cpp_meta_info) {
    CDBG_ERROR("%s:%d] malloc failed\n", __func__, __LINE__);
    meta_data->entry[PPROC_META_DATA_CPP_IDX].pproc_meta_dump = NULL;
    return 0;
  }
  memset(cpp_meta_info, 0, sizeof(cpp_info_t));
  cpp_meta_info->cpp_frame_msg =
    (uint32_t *)malloc(msm_cpp_frame_info->msg_len * sizeof(unsigned int));
  if (!cpp_meta_info->cpp_frame_msg) {
    CDBG_ERROR("%s:%d] malloc failed\n", __func__, __LINE__);
    free(cpp_meta_info);
    meta_data->entry[PPROC_META_DATA_CPP_IDX].pproc_meta_dump = NULL;
    return 0;
  }
  fe_config = (fe_config_t *)malloc(sizeof(fe_config_t));
  if (!fe_config) {
    CDBG_ERROR("%s:%d] malloc failed\n", __func__, __LINE__);
    free(cpp_meta_info->cpp_frame_msg);
    free(cpp_meta_info);
    meta_data->entry[PPROC_META_DATA_CPP_IDX].pproc_meta_dump = NULL;
    return 0;
  }
  memset(fe_config, 0, sizeof(fe_config_t));
  fe_config->luma_x = cpp_frame_info->plane_info[0].h_scale_initial_phase;
  fe_config->luma_y = cpp_frame_info->plane_info[0].v_scale_initial_phase;
  fe_config->chroma_x = cpp_frame_info->plane_info[1].h_scale_initial_phase;
  fe_config->chroma_y = cpp_frame_info->plane_info[1].v_scale_initial_phase;
  if ((hw_params->rotation == 0) || (hw_params->rotation == 2)) {
    fe_config->luma_dx = cpp_frame_info->plane_info[0].h_scale_ratio *
      cpp_frame_info->plane_info[0].dst_width;
    fe_config->luma_dy = cpp_frame_info->plane_info[0].v_scale_ratio *
      cpp_frame_info->plane_info[0].dst_height;
    fe_config->chroma_dx = cpp_frame_info->plane_info[1].h_scale_ratio *
      cpp_frame_info->plane_info[1].dst_width;
    fe_config->chroma_dy = cpp_frame_info->plane_info[1].v_scale_ratio *
      cpp_frame_info->plane_info[1].dst_height;
  } else {
    fe_config->luma_dx = cpp_frame_info->plane_info[0].v_scale_ratio *
      cpp_frame_info->plane_info[0].dst_width;
    fe_config->luma_dy = cpp_frame_info->plane_info[0].h_scale_ratio *
      cpp_frame_info->plane_info[0].dst_height;
    fe_config->chroma_dx = cpp_frame_info->plane_info[1].v_scale_ratio *
      cpp_frame_info->plane_info[1].dst_width;
    fe_config->chroma_dy = cpp_frame_info->plane_info[1].h_scale_ratio *
      cpp_frame_info->plane_info[1].dst_height;
  }

  memcpy(cpp_meta_info->cpp_frame_msg, msm_cpp_frame_info->cpp_cmd_msg,
    msm_cpp_frame_info->msg_len * sizeof(unsigned int));
  cpp_meta_info->size = msm_cpp_frame_info->msg_len;

  meta_data->entry[PPROC_META_DATA_CPP_IDX].dump_type =
    PPROC_META_DATA_CPP;
  meta_data->entry[PPROC_META_DATA_CPP_IDX].len =
    cpp_meta_info->size;
  meta_data->entry[PPROC_META_DATA_CPP_IDX].lux_idx =
    hw_params->aec_trigger.lux_idx;
  meta_data->entry[PPROC_META_DATA_CPP_IDX].gain =
    hw_params->aec_trigger.gain;
  meta_data->entry[PPROC_META_DATA_CPP_IDX].pproc_meta_dump = cpp_meta_info;
  meta_data->entry[PPROC_META_DATA_CPP_IDX].component_revision_no =
    cpp_hardware_get_fw_version(cpphw);

  meta_data->entry[PPROC_META_DATA_FE_IDX].dump_type =
    PPROC_META_DATA_FE;
  meta_data->entry[PPROC_META_DATA_FE_IDX].len =
    sizeof(fe_config_t);
  meta_data->entry[PPROC_META_DATA_FE_IDX].lux_idx =
    hw_params->aec_trigger.lux_idx;
  meta_data->entry[PPROC_META_DATA_FE_IDX].gain =
    hw_params->aec_trigger.gain;
  meta_data->entry[PPROC_META_DATA_FE_IDX].pproc_meta_dump = fe_config;

  return 0;
}

/* cpp_hardware_process_frame:
 *
 **/
static int32_t cpp_hardware_process_frame(cpp_hardware_t *cpphw,
  cpp_hardware_params_t *hw_params)
{
  int32_t rc = 0;
  int in_frame_fd;
  struct cpp_frame_info_t cpp_frame_info;
  struct msm_cpp_frame_info_t *msm_cpp_frame_info;

  memset(&cpp_frame_info, 0, sizeof(struct cpp_frame_info_t));
  if (!cpphw) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EINVAL;
  }

  CDBG_LOW("%s:%d, identity=0x%x", __func__, __LINE__, hw_params->identity);

  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  cpp_hardware_stream_status_t *stream_status =
    cpp_hardware_get_stream_status(cpphw, hw_params->identity);
  if (!stream_status) {
#if 0
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EFAULT;
#else
    /* Invalid stream at this layer means the stream is invalidated due to
       racing streamoff and this is considered as a racing process frame
       before queue invalidate */
     /* Need to trigger ack so return -EAGAIN*/
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EAGAIN;
#endif
  }

  /* TODO: cant do this, thread wont be able to release ack for this frame */
#if 0
  /* if a stream of pending for this identity, drop this frame and return */
  if (stream_status->stream_off_pending == TRUE) {
    CDBG("%s:%d, pending stream-off, frame dropped, identity=0x%x",
      __func__, __LINE__, hw_params->identity);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return 0;
  }
#endif

  CDBG_HIGH("%s:%d] stream_status:%p, iden:0x%x, stream type: %d,"
    "cur_frame_id:%d\n",
    __func__, __LINE__, stream_status,
    stream_status->identity, hw_params->stream_type,hw_params->frame_id);

  cpp_params_create_frame_info(hw_params, &cpp_frame_info);

  /* Translate the sysetm interface params to msm frame cfg params */
  cpp_frame_info.frame_id = hw_params->frame_id;
  cpp_frame_info.timestamp = hw_params->timestamp;
  cpp_frame_info.buff_index = hw_params->buffer_info.index;
  cpp_frame_info.native_buff = hw_params->buffer_info.native_buff;
  cpp_frame_info.in_buff_identity = hw_params->buffer_info.identity;
  cpp_frame_info.cookie = hw_params->cookie;
  cpp_frame_info.identity = hw_params->identity;
  cpp_frame_info.processed_divert = (int32_t)hw_params->processed_divert;
  cpp_frame_info.frame_type = MSM_CPP_REALTIME_FRAME;
  cpp_frame_info.plane_info[0].src_fd = hw_params->buffer_info.fd;
  cpp_frame_info.plane_info[1].src_fd = hw_params->buffer_info.fd;
  cpp_frame_info.out_buff_info = hw_params->output_buffer_info;
  /* TODO: Removal of this member needs careful kernel/userspace dependency */
  cpp_frame_info.plane_info[0].dst_fd = -1;
  cpp_frame_info.plane_info[1].dst_fd = -1;

  CDBG("%s:%d, punit: dup=%d, dup_id=0x%x", __func__, __LINE__,
    hw_params->duplicate_output, hw_params->duplicate_identity);

  cpp_frame_info.dup_output = 0;
  cpp_frame_info.dup_identity = 0x00000000;
  if (hw_params->duplicate_output == TRUE) {
    /* The stream status needs to be checked because the duplicate flag is
      tagged before streamoff call is received. The streamoff call will not be
      clearing the duplicate flag. */
    cpp_hardware_stream_status_t *dup_stream_status =
      cpp_hardware_get_stream_status(cpphw, hw_params->duplicate_identity);
    if (dup_stream_status) {
      /* The stream is ON */
      cpp_frame_info.dup_output = hw_params->duplicate_output;
      cpp_frame_info.dup_identity = hw_params->duplicate_identity;
    } else {
      CDBG_ERROR("%s:%d] Duplication stream is already off\n", __func__, __LINE__);
    }
  }

  msm_cpp_frame_info = cpp_hardware_create_hw_frame(cpphw, &cpp_frame_info);
  if (!msm_cpp_frame_info) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    cpp_hardware_destroy_hw_frame(msm_cpp_frame_info);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -EFAULT;
  }

  /* Fill bus message params */
  rc = cpp_hardware_dump_metadata(cpphw, hw_params, msm_cpp_frame_info,
    &cpp_frame_info);
  if (rc < 0) {
    CDBG_ERROR("%s:%d failed: cpp_hardware_fill_bus_message_params rc %d\n",
      __func__, __LINE__, rc);
  }

  /* send kernel ioctl for processing */
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  int32_t status = 0;
  msm_cpp_frame_info->status = &status;
  v4l2_ioctl.ioctl_ptr = msm_cpp_frame_info;
  v4l2_ioctl.len = sizeof(struct msm_cpp_frame_info_t);
  ATRACE_INT("Camera:CPP",1);
  rc = ioctl(cpphw->subdev_fd, VIDIOC_MSM_CPP_CFG, &v4l2_ioctl);
  if (rc < 0) {
    CDBG_ERROR("%s:%d, v4l2 ioctl() failed. rc:%d, trans_code:%d, "
               "frame_id=%d, identity=0x%x, stream type: %d\n",
               __func__,__LINE__, rc,
               *msm_cpp_frame_info->status,
                msm_cpp_frame_info->frame_id,
                msm_cpp_frame_info->identity,
                hw_params->stream_type);
    if (*msm_cpp_frame_info->status == -EAGAIN) {
      CDBG("%s:%d] drop this frame\n", __func__, __LINE__);
      rc = -EAGAIN;
    }
    cpp_hardware_destroy_hw_frame(msm_cpp_frame_info);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return rc;
  }
  CDBG_LOW("%s:%d, frame scheduled for processing, frame_id=%d, "
           "buf_idx=%d, identity=0x%x", __func__, __LINE__,
           msm_cpp_frame_info->frame_id,
           msm_cpp_frame_info->input_buffer_info.index,
           msm_cpp_frame_info->identity);

  /* Copy the current diagnostics parameters */
  if(hw_params->diagnostic_enable)
  {
    if(hw_params->ez_tune_asf_enable)
      cpp_hw_params_copy_asf_diag_params(&cpp_frame_info, &hw_params->asf_diag);
    if(hw_params->ez_tune_wnr_enable)
      cpp_hw_params_copy_wnr_diag_params(&cpp_frame_info, &hw_params->wnr_diag);
  }

  cpp_hardware_destroy_hw_frame(msm_cpp_frame_info);

  /* update stream status */
  stream_status->pending_buf++;
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}

/* cpp_hardware_unsubcribe_v4l2_event:
 *
 **/
static int32_t cpp_hardware_unsubcribe_v4l2_event(cpp_hardware_t *cpphw)
{
  int rc;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  struct v4l2_event_subscription sub;

  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  if (cpphw->event_subs_info.valid == TRUE) {
    cpphw->event_subs_info.valid = FALSE;
    sub.id = cpphw->event_subs_info.id;
    sub.type = cpphw->event_subs_info.type;
    rc = ioctl(cpphw->subdev_fd, VIDIOC_UNSUBSCRIBE_EVENT, &sub);
    if (rc < 0) {
      CDBG_ERROR("%s:%d: v4l2 ioctl() failed, rc=%d\n", __func__, __LINE__, rc);
      PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
      return -EIO;
    }
  }
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}

/* cpp_hardware_find_subdev:
 *
 **/
static int32_t cpp_hardware_find_subdev(cpp_hardware_t *cpphw)
{
  int i=0, rc=0;
  int fd;
  struct media_device_info mdev_info;
  char name[SUBDEV_NAME_SIZE_MAX];
  if (!cpphw) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  /* note: make sure to unlock this on each return path */
  PTHREAD_MUTEX_LOCK(&(cpphw->mutex));
  cpphw->num_subdev = 0;
  while(1) {
    snprintf(name, sizeof(name), "/dev/media%d", i);
    fd = open(name, O_RDWR | O_NONBLOCK);
    if (fd >= MAX_FD_PER_PROCESS) {
      dump_list_of_daemon_fd();
      fd = -1;
    }
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
      if (cpphw->num_subdev >= MAX_CPP_DEVICES) {
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
          entity.group_id == MSM_CAMERA_SUBDEV_CPP) {
        CDBG("%s:%d: CPP entity found: name=%s\n",
           __func__, __LINE__, entity.name);
        cpphw->subdev_ids[cpphw->num_subdev] = entity.revision;
        cpphw->num_subdev++;
      }
    }
    close(fd);
  }
  if (cpphw->num_subdev == 0) {
    CDBG_ERROR("%s:%d: no cpp device found.\n", __func__, __LINE__);
    PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
    return -ENODEV;
  }
  PTHREAD_MUTEX_UNLOCK(&(cpphw->mutex));
  return 0;
}

uint8_t cpp_hardware_fw_version_1_2_x(cpp_hardware_t *cpphw)
{
  uint8_t fw_version_1_2_x = 0;
  if ((CPP_HW_VERSION_1_1_0 == cpphw->hwinfo.version) ||
      (CPP_HW_VERSION_1_1_1 == cpphw->hwinfo.version) ||
      (CPP_HW_VERSION_2_0_0 == cpphw->hwinfo.version)) {
    fw_version_1_2_x = 1;
  }
  return fw_version_1_2_x;
}
