/* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential. */

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

#include "camera.h"
#include "cam_list.h"
#include "camera_dbg.h"
#include "cpp.h"

#define MAX_CPP_DEV 2
/* Max client number always should be ^2 */
#define MAX_CPP_CLIENT (1 << 2)
#define MAX_CPP_CLIENT_QUEUED_FRAME 8

struct cpp_client_t {
  uint32_t client_inst_id;
  pthread_cond_t frame_done_cond;
  struct msm_cpp_frame_info_t *frame;
  uint32_t client_num_queued_frames;
  cpp_process_queue_t frame_done_queue;
};

struct cpp_driver_t {
  uint8_t ref_count;
  uint8_t num_cpp_devices;
  uint8_t subdev_node_index[MAX_CPP_DEV];
  int32_t cpp_fd[MAX_CPP_DEV];
  pthread_t pid;
  pthread_mutex_t mutex;
  pthread_cond_t cond_v;
  int is_cpp_thread_ready;
  cpp_process_queue_t process_queue;
  int num_queued_frames;
  int num_pending_done;
  uint32_t inst_id;
  uint8_t initialized;

  uint32_t cpp_client_cnt;
  struct cpp_client_t cpp_client_info[MAX_CPP_CLIENT];

};

static struct cpp_driver_t cpp_driver;

static struct cpp_client_t *cpp_get_client_info(uint32_t inst_id) {
  uint32_t client_idx = (inst_id & (MAX_CPP_CLIENT - 1));
  CDBG("%s: handle = 0x%x, client_idx = %d", __func__, inst_id, client_idx);
  if (cpp_driver.cpp_client_info[client_idx].client_inst_id == inst_id)
    return &cpp_driver.cpp_client_info[client_idx];
  return NULL;
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

    if (strncmp(mdev_info.model, "qcamera", sizeof(mdev_info.model) != 0)) {
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
        CDBG("Done enumerating media entities\n");
        rc = 0;
        break;
      }
      if (entity.type == MEDIA_ENT_T_DEVNODE_V4L &&
          !strncmp(entity.name, name, sizeof(entity.name)) &&
          max_num_dev > *num_dev) {
        LOGE("CPP entity name: %s node id: %d\n", entity.name, entity.revision);
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
    snprintf(dev_name, sizeof(dev_name), "/dev/v4l-subdev%d", cpp_driver.subdev_node_index[i]);
    cpp_driver.cpp_fd[i] = open(dev_name, O_RDWR | O_NONBLOCK);
    if (cpp_driver.cpp_fd[i] < 0) {
      CDBG_ERROR("Cannot open cpp devices\n");
      rc = CPP_STATUS_DEV_ERROR;
      break;
    }
  }
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
  while (pos != head) {
    entry = member_of(pos, cpp_process_queue_t, list);
    if (entry) {
      if (entry->frame_id == frame_id) {
        cam_list_del_node(pos);
        return (void *)entry;
      } else {
        pos = pos->next;
      }
    } else {
      LOGE("Invalid entry in frame done queue\n");
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
  LOGE("%s: E\n", __func__);
  if (cpp_client == NULL) {
    LOGE("%s: Invalid Client\n", __func__);
  } else {
    while(done_frame == NULL) {
      done_frame =
        cpp_find_done_frame(&cpp_client->frame_done_queue, frame_id);
      if (done_frame != NULL)
        break;
      LOGE("%s: Condition wait\n", __func__);
      pthread_cond_wait(&cpp_client->frame_done_cond, &cpp_driver.mutex);
      LOGE("%s: Condition wait done\n", __func__);
    }
    free(done_frame);
    cpp_client->client_num_queued_frames--;
  }
  pthread_mutex_unlock(&cpp_driver.mutex);
  LOGE("%s: X\n", __func__);
  return CPP_STATUS_OK;
}

static void cpp_notify_client(struct msm_cpp_frame_info_t *frame)
{
  struct cpp_client_t *cpp_client = NULL;
  cpp_process_queue_t *done_frame = NULL;
  LOGE("%s: E\n", __func__);
  pthread_mutex_lock(&cpp_driver.mutex);
  cpp_client = cpp_get_client_info(frame->client_id);
  if (cpp_client == NULL) {
    LOGE("%s: Invalid Client\n", __func__);
  } else {
    done_frame = (cpp_process_queue_t *) malloc(sizeof(cpp_process_queue_t));
    if (!done_frame) {
      LOGE("CPP cannot allocation memory for done frame\n");
      pthread_mutex_unlock(&cpp_driver.mutex);
      return;
    }
    done_frame->frame_id = frame->frame_id;
    cam_list_add_tail_node(&done_frame->list, &cpp_client->frame_done_queue.list);
    LOGE("%s: Signal\n", __func__);
    pthread_cond_signal(&cpp_client->frame_done_cond);
    LOGE("%s: Signal done\n", __func__);
  }
  pthread_mutex_unlock(&cpp_driver.mutex);
  LOGE("%s: X\n", __func__);
  return;
}

struct msm_cpp_frame_info_t *cpp_gen_frame_info(cpp_process_queue_t *process_frame) {
  struct msm_cpp_frame_info_t *frame_info = malloc(sizeof(struct msm_cpp_frame_info_t));
  memset(frame_info, 0, sizeof(struct msm_cpp_frame_info_t));
  frame_info->client_id = process_frame->client_id;
  frame_info->frame_id = process_frame->frame_id;
  frame_info->inst_id = cpp_driver.inst_id;
  cpp_prepare_frame_info(&process_frame->frame_info, frame_info);
  return frame_info;
}

void *cpp_processing_thread(void *data)
{
  int rc = 0;
  struct v4l2_event_subscription sub;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  struct msm_cpp_frame_info_t frame;
  pthread_mutex_lock(&cpp_driver.mutex);
  cpp_driver.is_cpp_thread_ready = TRUE;
  pthread_cond_signal(&cpp_driver.cond_v);
  pthread_mutex_unlock(&cpp_driver.mutex);

  memset(&frame, 0, sizeof(struct msm_cpp_frame_info_t));
  v4l2_ioctl.ioctl_ptr = &frame;
  rc = ioctl(cpp_driver.cpp_fd[0], VIDIOC_MSM_CPP_GET_INST_INFO, &v4l2_ioctl);
  LOGE("CPP inst id: %d\n", frame.inst_id);
  cpp_driver.inst_id = frame.inst_id;

  sub.id = frame.inst_id;
  sub.type = V4L2_EVENT_CPP_FRAME_DONE;
  rc = ioctl(cpp_driver.cpp_fd[0], VIDIOC_SUBSCRIBE_EVENT, &sub);

  struct pollfd fds[cpp_driver.num_cpp_devices];
  fds[0].fd = cpp_driver.cpp_fd[0];
  fds[0].events = POLLPRI;
  LOGE("CPP subscribe \n");
  do {
    rc = poll(fds, 1, 15);
    //LOGE("%s: received events = 0x%x\n", __func__, fds[0].revents);
    if (rc != 0) {
      struct v4l2_event ev;
      rc = ioctl(cpp_driver.cpp_fd[0], VIDIOC_DQEVENT, &ev);
      v4l2_ioctl.ioctl_ptr = &frame;
      rc = ioctl(cpp_driver.cpp_fd[0], VIDIOC_MSM_CPP_GET_EVENTPAYLOAD, &v4l2_ioctl);
      LOGE("CPP %s: fid: %d done\n", __func__, frame.frame_id);
      cpp_notify_client(&frame);
      cpp_driver.num_pending_done--;
    }
    pthread_mutex_lock(&cpp_driver.mutex);
    if (cpp_driver.num_queued_frames > 0) {
      cpp_process_queue_t *process_frame;
      struct msm_cpp_frame_info_t *frame_info;
      process_frame = cpp_get_next_frame(&cpp_driver.process_queue);
      frame_info = cpp_gen_frame_info(process_frame);
      v4l2_ioctl.ioctl_ptr = frame_info;
      LOGE("CPP %s: fid: %d queue\n", __func__, frame_info->frame_id);
      rc = ioctl(cpp_driver.cpp_fd[0], VIDIOC_MSM_CPP_CFG, &v4l2_ioctl);
      cam_list_del_node(&process_frame->list);
      free(process_frame);
      free(frame_info->strip_info);
      free(frame_info);
      cpp_driver.num_queued_frames--;
      cpp_driver.num_pending_done++;
    }
    pthread_mutex_unlock(&cpp_driver.mutex);
  } while (cpp_driver.is_cpp_thread_ready || cpp_driver.num_pending_done > 0);
  pthread_cond_signal(&cpp_driver.cond_v);

  LOGE("CPP unsubscribe\n");
  rc = ioctl(cpp_driver.cpp_fd[0], VIDIOC_UNSUBSCRIBE_EVENT, &sub);
  return NULL;
}

static CPP_STATUS cpp_launch_thread(void)
{
  int rc = CPP_STATUS_NOT_SUPPORTED;
  pthread_cond_init(&cpp_driver.cond_v, NULL);

  rc = pthread_create(&cpp_driver.pid, NULL, cpp_processing_thread, NULL);
  if (rc < 0) {
    CDBG_ERROR("%s: Cannot launch cpp_processing_thread rc = %d", __func__, rc);
    return rc;
  }
  LOGE("thread create done\n");
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

  LOGE("CPP %s\n", __func__);
  pthread_join(cpp_driver.pid, NULL);
  return rc;
}

static CPP_STATUS cpp_driver_init(void)
{
  CPP_STATUS rc = CPP_STATUS_NOT_SUPPORTED;
  if (cpp_driver.initialized == FALSE) {
    discover_subdev_node("msm_cpp", &cpp_driver.num_cpp_devices,
                         cpp_driver.subdev_node_index, MAX_CPP_DEV);
    cpp_open_subdev();
    cpp_driver.num_queued_frames = 0;
    cpp_driver.num_pending_done = 0;
    cpp_driver.ref_count = 0;
    cam_list_init(&cpp_driver.process_queue.list);
    cpp_driver.initialized = TRUE;
    cpp_launch_thread();
    LOGE("CPP thread launched\n");
  }
  cpp_driver.ref_count++;
  LOGE("CPP %s cpp_driver.ref_count %d init %d\n", __func__, cpp_driver.ref_count, cpp_driver.initialized);
  return CPP_STATUS_OK;
  err:
  return rc;
}

static CPP_STATUS cpp_driver_deinit(void)
{
  CPP_STATUS rc = CPP_STATUS_NOT_SUPPORTED;
  cpp_driver.ref_count--;
  LOGE("CPP %s cpp_driver.ref_count %d init %d\n", __func__, cpp_driver.ref_count, cpp_driver.initialized);
  if (cpp_driver.ref_count == 0 && cpp_driver.initialized == TRUE) {
    LOGE("CPP %s\n", __func__);
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
  if (!cpp_client) {
    LOGE("CPP Invalid client id\n");
    return CPP_STATUS_DEV_ERROR;
  }
  pthread_mutex_lock(&cpp_driver.mutex);
  if (cpp_client->client_num_queued_frames > MAX_CPP_CLIENT_QUEUED_FRAME) {
    LOGE("CPP Maximum number of queued frame reached\n");
    return CPP_STATUS_CLIENT_QUEUE_FULL;
  }
  enqueue_frame = (cpp_process_queue_t *) malloc(sizeof(cpp_process_queue_t));
  if (!enqueue_frame) {
    LOGE("CPP cannot allocation memory for new frame\n");
    return CPP_STATUS_NO_MEM;
  }
  memcpy(enqueue_frame, new_frame, sizeof(cpp_process_queue_t));
  cam_list_add_tail_node(&enqueue_frame->list, &cpp_driver.process_queue.list);
  cpp_driver.num_queued_frames++;
  cpp_client->client_num_queued_frames++;
  pthread_mutex_unlock(&cpp_driver.mutex);
  return CPP_STATUS_OK;
}

CPP_STATUS cpp_get_instance(uint32_t *cpp_client_inst_id)
{
  int i;
  struct cpp_client_t *new_cpp_client = NULL;
  pthread_mutex_lock(&cpp_driver.mutex);
  for (i = 0; i < MAX_CPP_CLIENT; i++) {
    if (cpp_driver.cpp_client_info[i].client_inst_id == 0) {
      new_cpp_client = &cpp_driver.cpp_client_info[i];
      break;
    }
  }
  LOGE("%s: Found free instance\n", __func__);

  if (!new_cpp_client) {
    LOGE("No free CPP instance\n");
    pthread_mutex_unlock(&cpp_driver.mutex);
    return CPP_STATUS_DEV_ERROR;
  } else {
    memset(new_cpp_client, 0, sizeof(struct cpp_client_t));
    LOGE("%s: Condition wait init\n", __func__);
    pthread_cond_init(&new_cpp_client->frame_done_cond, NULL);
    cam_list_init(&new_cpp_client->frame_done_queue.list);
    new_cpp_client->client_inst_id = cpp_gen_client_inst_id(i);
    *cpp_client_inst_id = new_cpp_client->client_inst_id;
  }

  cpp_driver_init();

  pthread_mutex_unlock(&cpp_driver.mutex);
  return CPP_STATUS_OK;
}

CPP_STATUS cpp_free_instance(uint32_t cpp_client_inst_id)
{
  struct cpp_client_t *cpp_client = NULL;
  pthread_mutex_lock(&cpp_driver.mutex);
  cpp_client = cpp_get_client_info(cpp_client_inst_id);
  if (cpp_client == NULL) {
    LOGE("%s: CPP client not found\n", __func__);
    pthread_mutex_unlock(&cpp_driver.mutex);
    return CPP_STATUS_DEV_ERROR;
  }

  cpp_driver_deinit();
  cpp_client->client_inst_id = 0;
  pthread_mutex_unlock(&cpp_driver.mutex);

  return CPP_STATUS_OK;
}
