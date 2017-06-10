/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/media.h>
#include <cutils/log.h>
#include <poll.h>
#include <sys/syscall.h>

//#include "camera.h"
#include <media/msmb_camera.h>
#include <media/msmb_pproc.h>
#include "cam_list.h"
#include "camera_dbg.h"
#include "mtype.h"
#include "pproc_interface.h"
#include "cpp.h"
#include "pproc_caps.h"

#ifdef ENABLE_CPP_DRIVER_DEBUG
#undef CDBG
#define CDBG ALOGE
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE
#endif

#define CPP_MAX_SHARPNESS         6
#define CPP_MIN_SHARPNESS         0
#define CPP_TOTAL_SHARPNESS_LEVEL 6

#define CPP_FIRMWARE_STR_LEN 23
#define CPP_FIRMWARE_VERSION 2
#define CPP_HW_VERSION_1_0_0 0x10000000
#define CPP_HW_VERSION_1_1_0 0x10010000

const char firmware_version[CPP_FIRMWARE_VERSION][CPP_FIRMWARE_STR_LEN] = {
  "cpp_firmware_v1_1_6.fw", //CPP_HW_VERSION_1_0_0
  "cpp_firmware_v1_1_6.fw", //CPP_HW_VERSION_1_1_0
};

static struct cpp_driver_t cpp_driver;

static struct cpp_client_t *cpp_get_client_info(uint32_t inst_id) {
  uint32_t tmp = (inst_id & 0xff);
  uint32_t client_idx = tmp & 0xff;
  CDBG("%s: handle = 0x%x, client_idx = %d", __func__, inst_id, client_idx);
  if (cpp_driver.cpp_client_info[client_idx].client_inst_id == inst_id)
    return &cpp_driver.cpp_client_info[client_idx];
  return NULL;
}

static double cpp_get_sharpness_ratio(int32_t sharpness)
{
  return (double)(sharpness)/(CPP_MAX_SHARPNESS - CPP_MIN_SHARPNESS);
}

void cpp_create_frame_info(pproc_frame_input_params_t *frame_params,
  struct cpp_frame_info_t *frame_info)
{
  int i = 0, j, k;
  struct cpp_plane_info_t *plane_info = frame_info->plane_info;
  for(i = 0; i < 2; i++) {
    memset(&plane_info[i], 0, sizeof(struct cpp_plane_info_t));
      plane_info[i].rotate = frame_params->rotation / 90;
      plane_info[i].mirror = frame_params->mirror;
      plane_info[i].h_scale_ratio = 1/frame_params->h_scale_ratio;
      plane_info[i].v_scale_ratio = 1/frame_params->v_scale_ratio;
      plane_info[i].h_scale_initial_phase = frame_params->process_window_first_pixel;
      plane_info[i].v_scale_initial_phase = frame_params->process_window_first_line;
      plane_info[i].src_width = frame_params->src_width;
      plane_info[i].src_height = frame_params->src_height;
      plane_info[i].src_stride = frame_params->src_stride;
      plane_info[i].dst_width =
        frame_params->process_window_width * frame_params->h_scale_ratio;
      plane_info[i].dst_height =
        frame_params->process_window_height * frame_params->v_scale_ratio;
      plane_info[i].prescale_padding = 22;
      plane_info[i].postscale_padding = 4;
      plane_info[i].bf_enable = frame_params->denoise_enable;
      if (plane_info[i].rotate == 0 || plane_info[i].rotate == 2) {
        plane_info[i].dst_stride = frame_params->dst_stride;
        plane_info[i].maximum_dst_stripe_height =
          PAD_TO_2(frame_params->dst_scanline);
      } else {
        plane_info[i].dst_stride = PAD_TO_32(frame_params->dst_scanline/2) * 2;
        plane_info[i].maximum_dst_stripe_height = frame_params->dst_stride;
      }
  }

  plane_info[0].input_plane_fmt = PLANE_Y;
  plane_info[1].src_width /= 2;
  plane_info[1].dst_width /= 2;
  plane_info[1].h_scale_initial_phase /= 2;

  if (frame_params->in_plane_fmt != PPROC_PLANE_CBCR422 &&
      frame_params->in_plane_fmt != PPROC_PLANE_CRCB422) {
    plane_info[1].src_height /= 2;
    plane_info[1].dst_height /= 2;
    plane_info[1].v_scale_initial_phase /= 2;
    plane_info[1].maximum_dst_stripe_height =
        PAD_TO_2(plane_info[1].maximum_dst_stripe_height / 2);
  }
  plane_info[1].postscale_padding = 0;
  plane_info[1].input_plane_fmt = PLANE_CBCR;
  plane_info[1].source_address =
    plane_info[0].src_stride * frame_params->src_scanline;
  plane_info[1].destination_address =
  plane_info[0].dst_stride * plane_info[0].maximum_dst_stripe_height;

#if 0
  for (i = 0; i < 2; i++) {
    cpp_debug_input_info(&plane_info[i]);
  }
#endif

  for (k = 0; k < 4; k++) {
    for (j = 0; j < 3; j++) {
      frame_info->noise_profile[j][k] = frame_params->noise_profile[j][k];
      frame_info->weight[j][k] = frame_params->weight[j][k];
      frame_info->denoise_ratio[j][k] = frame_params->denoise_ratio[j][k];
      frame_info->edge_softness[j][k] = frame_params->edge_softness[j][k];
    }
  }

  frame_info->asf_mode = frame_params->asf_mode;
  frame_info->sharpness_ratio =
    cpp_get_sharpness_ratio(frame_params->sharpness);
  frame_info->asf_info = frame_params->asf_info;
  frame_info->num_planes = 2;
  if (frame_info->asf_mode == ASF_SKETCH ||
    frame_info->asf_mode == ASF_EMBOSS) {
    frame_info->num_planes = 1;
  }

  if (frame_params->in_plane_fmt == PPROC_PLANE_CBCR422) {
    frame_info->in_plane_fmt = PLANE_CBCR;
  } else if (frame_params->in_plane_fmt == PPROC_PLANE_CRCB422) {
    frame_info->in_plane_fmt = PLANE_CRCB;
  } else {
    frame_info->in_plane_fmt = frame_params->in_plane_fmt;
  }

  //frame_info->out_plane_fmt = frame_params->out_plane_fmt;
  frame_info->out_plane_fmt = PLANE_CBCR;
  if (frame_params->in_plane_fmt != frame_params->out_plane_fmt) {
    frame_info->out_plane_fmt = PLANE_CRCB;
  }

  return;
}

static void cpp_create_process_queue_entry(pproc_frame_input_params_t *frame_params,
  cpp_process_queue_t *new_frame)
{
  cpp_create_frame_info(frame_params, &new_frame->frame_info);
  //new_frame->frame_info.out_buff_idx = frame_params->out_buff_idx;
  new_frame->frame_info.frame_type = MSM_CPP_REALTIME_FRAME;
  new_frame->frame_info.plane_info[0].src_fd = frame_params->in_frame_fd;
  new_frame->frame_info.plane_info[0].dst_fd = frame_params->out_frame_fd;
  new_frame->frame_info.plane_info[1].src_fd = frame_params->in_frame_fd;
  new_frame->frame_info.plane_info[1].dst_fd = frame_params->out_frame_fd;
  return;
}

static void *cpp_get_next_frame(cpp_process_queue_t *process_queue)
{
  struct cam_list *pos;
  struct cam_list *head = &(process_queue->list);
  cpp_process_queue_t *entry;

  pos = head->next;
  if (pos != head) {
    entry = member_of(pos, cpp_process_queue_t, list);
    CDBG("%s: Entry = %p\n", __func__, (uint32_t *)entry);
    return(void *)entry;
  }

  CDBG("%s: entry not found\n", __func__);
  return NULL;
}

int discover_subdev_node(const char *name, uint8_t *num_dev,
                         uint8_t *subdev_node_index, uint8_t max_num_dev)
{
  struct media_device_info mdev_info;
  int num_media_devices = 0;
  char dev_name[32];
  int rc = 0, dev_fd = 0;
  while (1) {
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (dev_fd < 0) {
      CDBG("Done discovering media devices\n");
      break;
    }
    num_media_devices++;
    rc = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (rc < 0) {
      CDBG_ERROR("Error: ioctl media_dev failed: %s\n", strerror(errno));
      close(dev_fd);
      break;
    }

    if (strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model) != 0)) {
      close(dev_fd);
      continue;
    }

    int num_entities = 1;
    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities++;
      rc = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (rc < 0) {
        close(dev_fd);
        CDBG("Done enumerating media entities\n");
        rc = 0;
        break;
      }

      CDBG("entity name: %s node id: %d\n", entity.name, entity.revision);
      if (entity.type == MEDIA_ENT_T_V4L2_SUBDEV &&
          entity.group_id == MSM_CAMERA_SUBDEV_CPP &&
          //entity.group_id == MSM_CAMERA_SUBDEV_SENSOR &&
          //!strncmp(entity.name, name, sizeof(entity.name)) &&
          max_num_dev > *num_dev) {
        CDBG("CPP entity name: %s node id: %d\n", entity.name, entity.revision);
        subdev_node_index[*num_dev] = entity.revision;
        *num_dev = *num_dev + 1;
      }
    }
  }
  return 0;
}

static CPP_STATUS cpp_open_subdev(void)
{
  int i;
  char dev_name[32];
  CPP_STATUS rc = CPP_STATUS_OK;
  for (i = 0; i < cpp_driver.num_cpp_devices; i++) {
    snprintf(dev_name, sizeof(dev_name), "/dev/v4l-subdev%d",
      cpp_driver.subdev_node_index[i]);
    cpp_driver.cpp_fd[i] = open(dev_name, O_RDWR | O_NONBLOCK);
    if (cpp_driver.cpp_fd[i] < 0) {
      CDBG_ERROR("Cannot open cpp devices\n");
      rc = CPP_STATUS_DEV_ERROR;
      break;
    }
  }
  return rc;
}

static CPP_STATUS cpp_init_hw_firmware(void)
{
  CPP_STATUS rc = CPP_STATUS_OK;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;

  v4l2_ioctl.ioctl_ptr = (void *)&cpp_driver.hw_info;
  rc = ioctl(cpp_driver.cpp_fd[0], VIDIOC_MSM_CPP_GET_HW_INFO,
    &v4l2_ioctl);
  if (CPP_HW_VERSION_1_1_0 == cpp_driver.hw_info.cpp_hw_version) {
    v4l2_ioctl.ioctl_ptr = (void *)&firmware_version[1][0];
  } else {
    v4l2_ioctl.ioctl_ptr = (void *)&firmware_version[0][0];
  }

  v4l2_ioctl.len = CPP_FIRMWARE_STR_LEN;
  rc = ioctl(cpp_driver.cpp_fd[0], VIDIOC_MSM_CPP_LOAD_FIRMWARE,
    &v4l2_ioctl);
  return rc;
}

static void cpp_close_subdev(void)
{
  int i;
  for (i = 0; i < cpp_driver.num_cpp_devices; i++) {
    if (cpp_driver.cpp_fd[i] > 0) {
      close(cpp_driver.cpp_fd[i]);
    }
  }
}

static void *cpp_find_done_frame(cpp_process_queue_t *process_queue, int32_t frame_id)
{
  struct cam_list *pos;
  struct cam_list *head = &(process_queue->list);
  cpp_process_queue_t *entry = NULL;

  pos = head->next;
  CDBG("%s\n", __func__);
  //CDBG("%d\n", entry->client_id);
  while (pos != head) {
    entry = member_of(pos, cpp_process_queue_t, list);
    if (entry) {
      if (entry->frame_info.frame_id == frame_id) {
        cam_list_del_node(pos);
        return (void *)entry;
      } else {
        pos = pos->next;
      }
    } else {
      CDBG("Invalid entry in frame done queue\n");
      break;
    }
  }

  CDBG("%s: frame not found\n", __func__);
  return NULL;
}

CPP_STATUS cpp_client_frame_finish(uint32_t client_id, uint32_t frame_id) {
  struct cpp_client_t *cpp_client = NULL;
  cpp_process_queue_t *done_frame = NULL;
  pthread_mutex_lock(&cpp_driver.mutex);
  cpp_client = cpp_get_client_info(client_id);

  CDBG("%s: divert frame complete, frameid:%d, clientid:%d\n", __func__, frame_id, client_id);
  if (cpp_client == NULL) {
    CDBG("%s: Invalid Client\n", __func__);
  } else {
    while(done_frame == NULL) {
      done_frame =
        cpp_find_done_frame(&cpp_client->frame_done_queue, frame_id);
      if (done_frame != NULL)
        break;
    }
    free(done_frame);
    cpp_client->client_num_queued_frames--;
  }
  pthread_mutex_unlock(&cpp_driver.mutex);
  CDBG("%s: X\n", __func__);
  return CPP_STATUS_OK;
}

#if 0
static void cpp_notify_client(struct msm_cpp_frame_info_t *frame)
{
  struct cpp_client_t *cpp_client = NULL;
  cpp_process_queue_t *done_frame = NULL;
  pthread_mutex_lock(&cpp_driver.mutex);
  cpp_client = cpp_get_client_info(frame->client_id);
  if (cpp_client == NULL) {
    CDBG("%s: Invalid Client\n", __func__);
  } else {
    done_frame = (cpp_process_queue_t *) malloc(sizeof(cpp_process_queue_t));
    if (!done_frame) {
      CDBG("CPP cannot allocation memory for done frame\n");
      pthread_mutex_unlock(&cpp_driver.mutex);
      return;
    }
    done_frame->frame_info.frame_id = frame->frame_id;
    done_frame->frame_info.out_frame_id = frame->out_frame_id;
    cam_list_add_tail_node(&done_frame->list, &cpp_client->frame_done_queue.list);
    cpp_client->client_callback.framedone_ack(done_frame->frame_info.frame_id,
      done_frame->frame_info.in_buff_idx, done_frame->frame_info.out_buff_idx,
      frame->ouNULL);
  }
  pthread_mutex_unlock(&cpp_driver.mutex);
  CDBG("%s: X\n", __func__);
  return;
}
#endif

struct msm_cpp_frame_info_t *cpp_gen_frame_info(cpp_process_queue_t *process_frame) {
  struct msm_cpp_frame_info_t *frame_info = malloc(sizeof(struct msm_cpp_frame_info_t));
  if (frame_info) {
    memset(frame_info, 0, sizeof(struct msm_cpp_frame_info_t));
    frame_info->client_id = process_frame->client_id;
    frame_info->inst_id = cpp_driver.inst_id;
    cpp_prepare_frame_info(&process_frame->frame_info, frame_info);
  }
  return frame_info;
}

void *cpp_processing_thread(void *data)
{
  int rc = 0;
  struct cpp_client_t *cpp_client = NULL;
  struct v4l2_event_subscription sub;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  struct msm_cpp_frame_info_t frame;
  pproc_interface_callback_params_t callback_params;

  CDBG_ERROR("%s thread_id is %d\n",__func__, syscall(SYS_gettid));
  pthread_mutex_lock(&cpp_driver.mutex);
  cpp_driver.is_cpp_thread_ready = TRUE;
  pthread_cond_signal(&cpp_driver.cond_v);
  pthread_mutex_unlock(&cpp_driver.mutex);

  memset(&frame, 0, sizeof(struct msm_cpp_frame_info_t));
  v4l2_ioctl.ioctl_ptr = &frame;
  rc = ioctl(cpp_driver.cpp_fd[0], VIDIOC_MSM_CPP_GET_INST_INFO, &v4l2_ioctl);
  CDBG("CPP inst id: %d\n", frame.inst_id);
  cpp_driver.inst_id = frame.inst_id;

  sub.id = frame.inst_id;
  sub.type = V4L2_EVENT_CPP_FRAME_DONE;
  rc = ioctl(cpp_driver.cpp_fd[0], VIDIOC_SUBSCRIBE_EVENT, &sub);

  struct pollfd fds[cpp_driver.num_cpp_devices];
  fds[0].fd = cpp_driver.cpp_fd[0];
  fds[0].events = POLLPRI;
  CDBG("CPP subscribe \n");
  do {
    rc = poll(fds, 1, 4);
    CDBG("%s: received events = 0x%x\n", __func__, fds[0].revents);
    CDBG("%s: Number pending=%d\n", __func__, cpp_driver.num_pending_done);
    if (rc != 0) {
      cpp_process_queue_t *pending_frame;

      struct v4l2_event ev;
      rc = ioctl(cpp_driver.cpp_fd[0], VIDIOC_DQEVENT, &ev);
      v4l2_ioctl.ioctl_ptr = &frame;
      rc = ioctl(cpp_driver.cpp_fd[0], VIDIOC_MSM_CPP_GET_EVENTPAYLOAD, &v4l2_ioctl);
      CDBG("CPP %s: fid: %d done\n", __func__, frame.frame_id);

      pthread_mutex_lock(&cpp_driver.mutex);
      pending_frame = cpp_get_next_frame(&cpp_driver.pending_done_queue);
      if (!pending_frame){
          pthread_mutex_unlock(&cpp_driver.mutex);
          return;
      }
      cpp_client = cpp_get_client_info(pending_frame->client_id);
      CDBG("%s: Client instanceid:%d\n", __func__,
           cpp_client->client_inst_id);
      CDBG("%s: Process->done queue, frameid:%d, clientid:%d\n", __func__,
           pending_frame->frame_info.frame_id,pending_frame->client_id);
      cam_list_del_node(&pending_frame->list);
      cam_list_add_tail_node(&pending_frame->list, &cpp_client->frame_done_queue.list);
      cpp_driver.num_pending_done--;
      pthread_mutex_unlock(&cpp_driver.mutex);
      CDBG("%s: Calling callback\n", __func__);

      callback_params.module = cpp_client->client_callback.data;
      callback_params.divert_frame = pending_frame->divert_frame;

      CDBG("%s: Camcorder_CPP: frmid:%d, inidx:%d, outidx:%d,buffiden:%d, eveniden:%d\n",
        __func__, callback_params.divert_frame.frame_params.frame_id,
        callback_params.divert_frame.isp_divert_buffer.v4l2_buffer_obj.index,
        callback_params.divert_frame.out_buff_idx,
        callback_params.divert_frame.isp_divert_buffer.identity,
        callback_params.divert_frame.mct_event_identity);

      cpp_client->client_callback.framedone_ack(0, &callback_params);
    }

    pthread_mutex_lock(&cpp_driver.mutex);
    if (cpp_driver.num_queued_frames > 0) {
      pproc_frame_input_params_t *frame_params = NULL;
      cpp_process_queue_t *process_frame;
      struct msm_cpp_frame_info_t *frame_info;
      CDBG("%s: Frame queued count=%d\n", __func__,
        cpp_driver.num_queued_frames);

      process_frame = cpp_get_next_frame(&cpp_driver.process_queue);

      CDBG("%s: Camcorder_CPP: frmid:%d, inidx:%d, outidx:%d,buffiden:%d, eveniden:%d\n",
        __func__, process_frame->divert_frame.frame_params.frame_id,
        process_frame->divert_frame.isp_divert_buffer.v4l2_buffer_obj.index,
        process_frame->divert_frame.out_buff_idx,
        process_frame->divert_frame.isp_divert_buffer.identity,
        process_frame->divert_frame.mct_event_identity);

      frame_params = &process_frame->divert_frame.frame_params;
      cpp_client = cpp_get_client_info(process_frame->client_id);
      /* create the frame info */
      rc = cpp_client->client_callback.create_frame(
        cpp_client->client_callback.data, &process_frame->divert_frame);

      if (rc == 0) {
        /* Drop this frame. Cannot process this frame */
        CDBG_ERROR("%s: Frame Skip, Create frame fail.\n", __func__);
        cam_list_del_node(&process_frame->list);
        /* Do frame done call back with skip information */
        cpp_driver.num_queued_frames--;
        cpp_client->client_num_queued_frames--;
        callback_params.module = cpp_client->client_callback.data;
        callback_params.divert_frame = process_frame->divert_frame;

        CDBG("%s: Camcorder_CPP: frmid:%d, inidx:%d, outidx:%d,buffiden:%d, eveniden:%d\n",
          __func__, callback_params.divert_frame.frame_params.frame_id,
          callback_params.divert_frame.isp_divert_buffer.v4l2_buffer_obj.index,
          callback_params.divert_frame.out_buff_idx,
          callback_params.divert_frame.isp_divert_buffer.identity,
          callback_params.divert_frame.mct_event_identity);

        free(process_frame);
        pthread_mutex_unlock(&cpp_driver.mutex);
        /* Both buffer skip and frame done skip */
        cpp_client->client_callback.framedone_ack(0x3, &callback_params);
        continue;
      }

      cpp_create_process_queue_entry(frame_params, process_frame);
      frame_info = cpp_gen_frame_info(process_frame);
      v4l2_ioctl.ioctl_ptr = frame_info;
      rc = ioctl(cpp_driver.cpp_fd[0], VIDIOC_MSM_CPP_CFG, &v4l2_ioctl);
      if (rc < 0) {
        /* Drop this frame. Cannot process this frame */
        CDBG_ERROR("%s: Frame Skip, Kernel IOCTL failed.\n", __func__);
        cam_list_del_node(&process_frame->list);
        /* Do frame done call back with skip information */
        cpp_driver.num_queued_frames--;
        cpp_client->client_num_queued_frames--;
        callback_params.module = cpp_client->client_callback.data;
        callback_params.divert_frame = process_frame->divert_frame;

        CDBG("%s: Camcorder_CPP: frmid:%d, inidx:%d, outidx:%d,buffiden:%d, eveniden:%d\n",
          __func__, callback_params.divert_frame.frame_params.frame_id,
          callback_params.divert_frame.isp_divert_buffer.v4l2_buffer_obj.index,
          callback_params.divert_frame.out_buff_idx,
          callback_params.divert_frame.isp_divert_buffer.identity,
          callback_params.divert_frame.mct_event_identity);

        free(frame_info->cpp_cmd_msg);
        free(frame_info);
        free(process_frame);
        pthread_mutex_unlock(&cpp_driver.mutex);
        /* Only frame done skip */
        cpp_client->client_callback.framedone_ack(0x2, &callback_params);
        continue;
      }
      cam_list_del_node(&process_frame->list);
      cam_list_add_tail_node(&process_frame->list,
        &cpp_driver.pending_done_queue.list);
      free(frame_info->cpp_cmd_msg);
      free(frame_info);
      cpp_driver.num_queued_frames--;
      cpp_driver.num_pending_done++;
    } else {
      if (cpp_driver.is_streamoff_pending == TRUE) {
        if (!cpp_driver.num_pending_done) {
          cpp_driver.is_streamoff_pending = FALSE;
          pthread_cond_signal(&cpp_driver.cond_stream_off);
        }
      }
      if (cpp_driver.flush_queue == TRUE) {
        /* Send trigger to flush the frame */
        v4l2_ioctl.ioctl_ptr = &frame;
        rc = ioctl(cpp_driver.cpp_fd[0], VIDIOC_MSM_CPP_FLUSH_QUEUE,
          &v4l2_ioctl);
        cpp_driver.flush_queue = FALSE;
      }
    }
    pthread_mutex_unlock(&cpp_driver.mutex);
  } while (cpp_driver.is_cpp_thread_ready || cpp_driver.num_pending_done > 0);
  pthread_cond_signal(&cpp_driver.cond_v);

  CDBG("CPP unsubscribe\n");
  rc = ioctl(cpp_driver.cpp_fd[0], VIDIOC_UNSUBSCRIBE_EVENT, &sub);
  return NULL;
}

static CPP_STATUS cpp_launch_thread(void)
{
  int rc = CPP_STATUS_NOT_SUPPORTED;
  pthread_cond_init(&cpp_driver.cond_v, NULL);
  pthread_cond_init(&cpp_driver.cond_stream_off, NULL);
  cpp_driver.is_streamoff_pending = FALSE;
  cpp_driver.flush_queue = FALSE;

  rc = pthread_create(&cpp_driver.pid, NULL, cpp_processing_thread, NULL);
  if (rc < 0) {
    CDBG_ERROR("%s: Cannot launch cpp_processing_thread rc = %d", __func__, rc);
    return rc;
  }
  CDBG("thread create done\n");
  if (!cpp_driver.is_cpp_thread_ready) {
    pthread_cond_wait(&cpp_driver.cond_v, &cpp_driver.mutex);
  }
  return rc;
}

static CPP_STATUS cpp_close_thread(void)
{
  int rc = CPP_STATUS_NOT_SUPPORTED;
  cpp_driver.is_cpp_thread_ready = FALSE;
  pthread_cond_wait(&cpp_driver.cond_v, &cpp_driver.mutex);

  CDBG("CPP %s\n", __func__);
  pthread_join(cpp_driver.pid, NULL);
  return rc;
}

static CPP_STATUS cpp_close_stream(void)
{
  pthread_mutex_lock(&cpp_driver.mutex);
  cpp_driver.is_streamoff_pending = TRUE;
  cpp_driver.flush_queue = TRUE;
  pthread_cond_wait(&cpp_driver.cond_stream_off, &cpp_driver.mutex);
  pthread_mutex_unlock(&cpp_driver.mutex);
  return CPP_STATUS_OK;
}

static CPP_STATUS cpp_driver_init(void)
{
  CPP_STATUS rc = CPP_STATUS_NOT_SUPPORTED;
  if (cpp_driver.initialized == FALSE) {
    discover_subdev_node("fda04000.qcom,cpp", &cpp_driver.num_cpp_devices,
                         cpp_driver.subdev_node_index, MAX_CPP_DEV);
    cpp_open_subdev();
    cpp_driver.num_queued_frames = 0;
    cpp_driver.num_pending_done = 0;
    cpp_driver.ref_count = 0;
    cam_list_init(&cpp_driver.process_queue.list);
    cam_list_init(&cpp_driver.pending_done_queue.list);
    cpp_driver.initialized = TRUE;
    cpp_launch_thread();
    CDBG("CPP thread launched\n");
  }
  cpp_driver.ref_count++;
  CDBG("CPP %s cpp_driver.ref_count %d init %d\n", __func__, cpp_driver.ref_count, cpp_driver.initialized);
  return CPP_STATUS_OK;
  err:
  return rc;
}

static CPP_STATUS cpp_driver_deinit(void)
{
  CPP_STATUS rc = CPP_STATUS_NOT_SUPPORTED;
  cpp_driver.ref_count--;
  CDBG("CPP %s cpp_driver.ref_count %d init %d\n", __func__, cpp_driver.ref_count, cpp_driver.initialized);
  if (cpp_driver.ref_count == 0 && cpp_driver.initialized == TRUE) {
    CDBG("CPP %s\n", __func__);
    cpp_close_thread();
    cpp_close_subdev();
    cpp_driver.initialized = FALSE;
  }
  return rc;
}

static uint32_t cpp_gen_client_inst_id(uint8_t client_idx)
{
  /* pattern: [24 bit count]|[4bits obj idx]|[4 bits client idx] */
  uint32_t inst_id =
  ((++cpp_driver.cpp_client_cnt) << 8) + (0xff & client_idx);
  return inst_id;
}

CPP_STATUS cpp_process_frame(cpp_process_queue_t *new_frame)
{
  struct cpp_client_t *cpp_client = NULL;
  cpp_process_queue_t *enqueue_frame = NULL;
  cpp_client = cpp_get_client_info(new_frame->client_id);
  CDBG("%s: Divert Enter\n", __func__);
  if (!cpp_client) {
    CDBG("CPP Invalid client id\n");
    return CPP_STATUS_DEV_ERROR;
  }
  pthread_mutex_lock(&cpp_driver.mutex);
  if (cpp_client->client_num_queued_frames > MAX_CPP_CLIENT_QUEUED_FRAME) {
    CDBG("CPP Maximum number of queued frame reached\n");
    return CPP_STATUS_CLIENT_QUEUE_FULL;
  }
  enqueue_frame = (cpp_process_queue_t *) malloc(sizeof(cpp_process_queue_t));
  if (!enqueue_frame) {
    CDBG("CPP cannot allocation memory for new frame\n");
    return CPP_STATUS_NO_MEM;
  }
  memcpy(enqueue_frame, new_frame, sizeof(cpp_process_queue_t));

  CDBG("%s: Camcorder_CPP: frmid:%d, inidx:%d, outidx:%d,buffiden:%d, eveniden:%d, clientid:%d\n",
    __func__, enqueue_frame->divert_frame.frame_params.frame_id,
    enqueue_frame->divert_frame.isp_divert_buffer.v4l2_buffer_obj.index,
    enqueue_frame->divert_frame.out_buff_idx,
    enqueue_frame->divert_frame.isp_divert_buffer.identity,
    enqueue_frame->divert_frame.mct_event_identity,
    cpp_client->client_inst_id);

  cam_list_add_tail_node(&enqueue_frame->list, &cpp_driver.process_queue.list);
  cpp_driver.num_queued_frames++;
  cpp_client->client_num_queued_frames++;
  pthread_mutex_unlock(&cpp_driver.mutex);
  CDBG("%s: Divert Exit\n", __func__);
  return CPP_STATUS_OK;
}

CPP_STATUS cpp_get_instance(void *id)
{
  uint32_t *cpp_client_inst_id = (uint32_t *)id;
  int i;
  struct cpp_client_t *new_cpp_client = NULL;
  pthread_mutex_lock(&cpp_driver.mutex);
  for (i = 0; i < MAX_CPP_CLIENT; i++) {
    if (cpp_driver.cpp_client_info[i].client_inst_id == 0) {
      new_cpp_client = &cpp_driver.cpp_client_info[i];
      break;
    }
  }
  CDBG("%s: Found free instance\n", __func__);

  if (!new_cpp_client) {
    CDBG("No free CPP instance\n");
    pthread_mutex_unlock(&cpp_driver.mutex);
    return CPP_STATUS_DEV_ERROR;
  } else {
    memset(new_cpp_client, 0, sizeof(struct cpp_client_t));
    cam_list_init(&new_cpp_client->frame_done_queue.list);
    new_cpp_client->client_inst_id = cpp_gen_client_inst_id(i);
    *cpp_client_inst_id = new_cpp_client->client_inst_id;
  }

  CDBG("%s: cpp_client_inst_id %p %d\n", __func__, cpp_client_inst_id, *cpp_client_inst_id);
  cpp_driver_init();

  pthread_mutex_unlock(&cpp_driver.mutex);
  return CPP_STATUS_OK;
}

void cpp_set_client_callback(struct cpp_library_params_t *cpp_lib_ctrl,
  pproc_client_callback_t *client_callback)
{
  uint32_t client_id = cpp_lib_ctrl->client_id;
  struct cpp_driver_t *driver = cpp_lib_ctrl->driver;
  struct cpp_client_t *cpp_client = NULL;
  pthread_mutex_lock(&cpp_driver.mutex);
  cpp_client = cpp_get_client_info(client_id);
  if (cpp_client == NULL) {
    CDBG("%s: CPP client not found\n", __func__);
    pthread_mutex_unlock(&cpp_driver.mutex);
    return;
  }
  CDBG("%s: CPP client callback set\n", __func__);
  cpp_client->client_callback.framedone_ack = client_callback->framedone_ack;
  cpp_client->client_callback.create_frame = client_callback->create_frame;
  cpp_client->client_callback.data = client_callback->data;
  pthread_mutex_unlock(&cpp_driver.mutex);
  return;
}

CPP_STATUS cpp_free_instance(void *id)
{
  uint32_t cpp_client_inst_id = (*(uint32_t *)id);
  struct cpp_client_t *cpp_client = NULL;
  pthread_mutex_lock(&cpp_driver.mutex);
  cpp_client = cpp_get_client_info(cpp_client_inst_id);
  if (cpp_client == NULL) {
    CDBG("%s: CPP client not found\n", __func__);
    pthread_mutex_unlock(&cpp_driver.mutex);
    return CPP_STATUS_DEV_ERROR;
  }

  cpp_driver_deinit();
  cpp_client->client_inst_id = 0;
  pthread_mutex_unlock(&cpp_driver.mutex);

  return CPP_STATUS_OK;
}

static void cpp_get_capability (pproc_caps_t *cpp_caps)
{
  /* TODO: read from hw reg */
  cpp_caps->caps_mask = CAPS_DENOISE |
    CAPS_SCALE | CAPS_SHARPENING | CAPS_CROP |
    CAPS_ROTATION | CAPS_FLIP;
  cpp_caps->caps_scale.max_scale_factor = 1.0/8.0;
  cpp_caps->caps_scale.min_scale_factor = 8.0;
  cpp_caps->caps_rotation = ROTATION_90 |
    ROTATION_180 | ROTATION_270;
  cpp_caps->caps_flip.h_flip = 1;
  cpp_caps->caps_flip.v_flip = 1;
  cpp_caps->caps_sharpness.max_value = CPP_MAX_SHARPNESS;
  cpp_caps->caps_sharpness.min_value = CPP_MIN_SHARPNESS;
  cpp_caps->caps_sharpness.step =
    (CPP_MAX_SHARPNESS - CPP_MIN_SHARPNESS) / CPP_TOTAL_SHARPNESS_LEVEL;
  cpp_caps->caps_sharpness.def_value =
    cpp_caps->caps_sharpness.step / 2;
};

static int32_t cpp_process(void *ctrl,
  pproc_interface_event_t event, void *data)
{
  CPP_STATUS status = CPP_STATUS_NOT_SUPPORTED;
  struct cpp_library_params_t *cpp_lib_ctrl = ctrl;
  uint32_t frameid = 0;
  CDBG("%s: cpp_lib_ctrl %d\n", __func__, __LINE__);
  switch (event) {
  case PPROC_IFACE_INIT:
    status = cpp_init_hw_firmware();
    break;
  case PPROC_IFACE_SET_CALLBACK:
    cpp_set_client_callback(cpp_lib_ctrl, (pproc_client_callback_t *)data);
    break;
  case PPROC_IFACE_GET_CAPABILITY:
    cpp_get_capability((pproc_caps_t *)data);
    status = CPP_STATUS_OK;
    break;
  case PPROC_IFACE_STOP_STREAM:
    status = cpp_close_stream();
    break;
  case PPROC_IFACE_PROCESS_FRAME:
    status = cpp_process_frame((cpp_process_queue_t *)data);
    break;
  case PPROC_IFACE_FRAME_DIVERT: {
    cpp_process_queue_t new_frame;
    pproc_interface_frame_divert_t *frame =
      (pproc_interface_frame_divert_t *)data;
    memset(&new_frame, 0, sizeof(cpp_process_queue_t));
    new_frame.client_id = cpp_lib_ctrl->client_id;
    new_frame.divert_frame = *frame;
    /* Used by lower layer */
    new_frame.frame_info.frame_id = frame->frame_params.frame_id;
    //new_frame.frame_info.in_buff_idx = frame->in_buff_idx;
    //new_frame.divert_frame.mct_event_identity = frame->mct_event_identity;
    //new_frame.divert_frame.frame_params = frame->frame_params;
    //new_frame.frame_params = frame->frame_params;
    CDBG("%s: Camcorder_CPP: frmid:%d, inidx:%d, outidx:%d,buffiden:%d, eveniden:%d, clientid:%d\n",
      __func__, new_frame.divert_frame.frame_params.frame_id,
      new_frame.divert_frame.isp_divert_buffer.v4l2_buffer_obj.index,
      new_frame.divert_frame.out_buff_idx,
      new_frame.divert_frame.isp_divert_buffer.identity,
      new_frame.divert_frame.mct_event_identity,
      new_frame.client_id);

    status = cpp_process_frame((cpp_process_queue_t *)&new_frame);
    break;
  }
  case PPROC_IFACE_FRAME_DONE:
    frameid = (uint32_t)data;
    CDBG("%s: Call frame done for frameid:%d\n", __func__, frameid);
    status = cpp_client_frame_finish(cpp_lib_ctrl->client_id,
      frameid);
    break;
  case PPROC_IFACE_SET_SHARPNESS:
    break;
  default:
  break;
  }
  return status;
};

int32_t cpp_open(void **data)
{
  int32_t ret = 0;
  struct cpp_library_params_t *cpp_lib_ctrl = NULL;

  cpp_lib_ctrl = malloc(sizeof(struct cpp_library_params_t));
  if (!cpp_lib_ctrl) {
    CDBG_ERROR("%s:%d malloc failed\n", __func__, __LINE__);
    return -ENOMEM;
  }
  memset(cpp_lib_ctrl, 0, sizeof(struct cpp_library_params_t));
  ret = cpp_get_instance(&cpp_lib_ctrl->client_id);
  if (ret == 0) {
    cpp_lib_ctrl->driver = &cpp_driver;
    *data = (void *)cpp_lib_ctrl;
  }
  CDBG("%s: client id %p %d\n", __func__, &cpp_lib_ctrl->client_id, cpp_lib_ctrl->client_id);
  return ret;
}

int32_t cpp_close(void *data)
{
  int32_t ret = 0;
  struct cpp_library_params_t *cpp_lib_ctrl =
    (struct cpp_library_params_t *)data;

  CDBG("%s: client id %d\n", __func__, cpp_lib_ctrl->client_id);
  if (!cpp_lib_ctrl) {
    CDBG_ERROR("%s:%d invalid pointer\n", __func__, __LINE__);
    return -EINVAL;
  }
  if (cpp_lib_ctrl) {
    ret = cpp_free_instance(&cpp_lib_ctrl->client_id);
    free(cpp_lib_ctrl);
  }
  return ret;
}

static pproc_interface_func_tbl_t cpp_func_tbl = {
  .open = &cpp_open,
  .process = &cpp_process,
  .close = &cpp_close,
};

static pproc_interface_lib_params_t cpp_lib_ptr = {
  .func_tbl = &cpp_func_tbl,
};

pproc_interface_lib_params_t *pproc_library_init(void)
{
  return &cpp_lib_ptr;
}
