/* mct_bus.c
 *
 * This file contains the bus implementation.
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "mct_bus.h"
#include "camera_dbg.h"
#include <sys/syscall.h>
#include <sys/prctl.h>

#if 0
#undef CDBG
#define CDBG CDBG_ERROR
#endif

#define MCT_BUS_SOF_TIMEOUT 5000000000 /*in ns unit*/
#define MCT_BUS_NANOSECOND_SCALER 1000000000
#define MAX_MCT_BUS_QUEUE_LENGTH 1000
static boolean mct_bus_queue_free(void *data, void *user_data)
{
  mct_bus_msg_t *pdata = data;

  if (pdata) {
    if (pdata->msg) {
      free(pdata->msg);
      pdata->msg = NULL;
    }
    free(pdata);
    pdata = NULL;
  }

  return TRUE;
}

/*
 * mct_bus_timeout_wait:
 *  cond:    POSIX conditional variable;
 *  mutex:   POSIX mutex;
 *  timeout: type of signed long long,  specified
 *           timeout measured in nanoseconds;
 *           timeout = -1 means no timeout, it becomes
 *           to regular conditional timewait.
 *
 *  Commonly used for timeout waiting.
 * */
static int mct_bus_timeout_wait(pthread_cond_t *cond, pthread_mutex_t *mutex,
                         signed long long timeout) {
  signed long long end_time;
  struct timeval r;
  struct timespec ts;
  int ret;
  pthread_mutex_lock(mutex);
  if (timeout != -1) {
    gettimeofday(&r, NULL);
    end_time = (((((signed long long)r.tv_sec) * 1000000) + r.tv_usec) +
      (timeout / 1000));
    ts.tv_sec  = (end_time / 1000000);
    ts.tv_nsec = ((end_time % 1000000) * 1000);
    ret = pthread_cond_timedwait(cond, mutex, &ts);
  } else {
    ret = pthread_cond_wait(cond, mutex);
  }
  pthread_mutex_unlock(mutex);
  return ret;
}

static void* mct_bus_sof_thread_run(void *data)
{
  mct_bus_t *bus = (mct_bus_t *)data;
  signed long long timeout =
    (((signed long long)(bus->thread_wait_time)) * MCT_BUS_NANOSECOND_SCALER);
  int ret;
  CDBG_ERROR("%s thread_id is %d\n",__func__, syscall(SYS_gettid));
  pthread_mutex_lock(&bus->bus_sof_init_lock);
  pthread_cond_signal(&bus->bus_sof_init_cond);
  pthread_mutex_unlock(&bus->bus_sof_init_lock);
  bus->thread_run = 1;
  CDBG_HIGH("%s thread_id is %d\n",__func__, syscall(SYS_gettid));
  prctl(PR_SET_NAME, "mct_bus_thread", 0, 0, 0);
  while(bus->thread_run) {
    ret = mct_bus_timeout_wait(&bus->bus_sof_msg_cond,
                         &bus->bus_sof_msg_lock, timeout);
    if (ret == ETIMEDOUT) {
      CDBG_ERROR("%s: Sending event to dump info for sof freeze\n", __func__);
      break;
    }
  }
  if (bus->thread_run == 1) {
    /*Things went wrong*/
    mct_bus_msg_t bus_msg;
    bus_msg.type = MCT_BUS_MSG_NOTIFY_KERNEL;
    bus_msg.size = 0;
    bus_msg.sessionid = bus->session_id;
    bus->post_msg_to_bus(bus, &bus_msg);
  }

  while(bus->thread_run) {
    ret = mct_bus_timeout_wait(&bus->bus_sof_msg_cond,
                         &bus->bus_sof_msg_lock, timeout);
    if (ret == ETIMEDOUT) {
      CDBG_ERROR("%s: SOF freeze; Sending error message\n", __func__);
      break;
    }
  }
  if (bus->thread_run == 1) {
    /*Things went wrong*/
    mct_bus_msg_t bus_msg;
    bus_msg.type = MCT_BUS_MSG_SEND_HW_ERROR;
    bus_msg.size = 0;
    bus_msg.sessionid = bus->session_id;
    bus->post_msg_to_bus(bus, &bus_msg);
  }
  return NULL;
}
static void start_sof_check_thread(mct_bus_t *bus)
{
  int rc = 0;
  if (bus->thread_run == 1)
    return;
  CDBG_HIGH("%s: Starting SOF timeout thread\n", __func__);
  pthread_mutex_init(&bus->bus_sof_msg_lock, NULL);
  pthread_cond_init(&bus->bus_sof_msg_cond, NULL);
  pthread_mutex_lock(&bus->bus_sof_init_lock);
  rc = pthread_create(&bus->bus_sof_tid, NULL, mct_bus_sof_thread_run, bus);
  if(!rc) {
    pthread_cond_wait(&bus->bus_sof_init_cond, &bus->bus_sof_init_lock);
  }
  pthread_mutex_unlock(&bus->bus_sof_init_lock);
}

static void stop_sof_check_thread(mct_bus_t *bus)
{
  if (bus->thread_run == 0)
    return;
  CDBG_HIGH("%s: Stopping SOF timeout thread\n", __func__);
  bus->thread_run = 0;
  pthread_mutex_lock(&bus->bus_sof_msg_lock);
  pthread_cond_signal(&bus->bus_sof_msg_cond);
  pthread_mutex_unlock(&bus->bus_sof_msg_lock);
  pthread_join(bus->bus_sof_tid, NULL);
  pthread_cond_destroy(&bus->bus_sof_msg_cond);
  pthread_mutex_destroy(&bus->bus_sof_msg_lock);
}

static boolean msg_bus_post_msg(mct_bus_t *bus, mct_bus_msg_t *bus_msg)
{
  mct_bus_msg_t *local_msg;
  int post_msg = FALSE;
  unsigned int payload_size;

  if (!bus_msg) {
    CDBG_ERROR("%s:%d NULL ptr", __func__, __LINE__);
    goto error_2;
  }

  if (bus->bus_queue->length > MAX_MCT_BUS_QUEUE_LENGTH) {
      pthread_mutex_lock(&bus->bus_msg_q_lock);
      mct_bus_queue_flush(bus);
      CDBG_HIGH("%s : Discard the bus msg's that got stagnated in the queue\n",__func__);
      pthread_mutex_unlock(&bus->bus_msg_q_lock);
      return TRUE;
  }

  switch (bus_msg->type) {
    case MCT_BUS_MSG_ISP_SOF:
      payload_size = sizeof(mct_bus_msg_isp_sof_t);
      if (bus->thread_run == 1) {
        pthread_mutex_lock(&bus->bus_sof_msg_lock);
        pthread_cond_signal(&bus->bus_sof_msg_cond);
        pthread_mutex_unlock(&bus->bus_sof_msg_lock);
      }
      break;
    case MCT_BUS_MSG_Q3A_AF_STATUS:
      payload_size = sizeof(mct_bus_msg_af_status_t);
      break;
    case MCT_BUS_MSG_ASD_HDR_SCENE_STATUS:
      payload_size = sizeof(mct_bus_msg_asd_hdr_status_t);
      break;
    case MCT_BUS_MSG_FACE_INFO:
      payload_size = sizeof(cam_face_detection_data_t);
      break;
    case MCT_BUS_MSG_HIST_STATS_INFO:
      payload_size = sizeof(cam_hist_stats_t);
      break;
    case MCT_BUS_MSG_PREPARE_HW_DONE:
      payload_size = sizeof(cam_prep_snapshot_state_t);
      break;
    case MCT_BUS_MSG_ZSL_TAKE_PICT_DONE:
      payload_size = sizeof(cam_frame_idx_range_t);
      break;
    case MCT_BUS_MSG_SET_SENSOR_INFO:
      payload_size = sizeof(mct_bus_msg_sensor_metadata_t);
      break;
    case MCT_BUS_MSG_SET_STATS_AEC_INFO:
      payload_size = bus_msg->size;
      break;
    case MCT_BUS_MSG_SET_ISP_GAMMA_INFO:
    case MCT_BUS_MSG_SET_ISP_STATS_AWB_INFO:
      payload_size = bus_msg->size;
      break;
   case MCT_BUS_MSG_ISP_STREAM_CROP:
      payload_size = sizeof(mct_bus_msg_stream_crop_t);
      break;
   case MCT_BUS_MSG_SET_AEC_STATE:
      payload_size = sizeof(int32_t);
      break;
   case MCT_BUS_MSG_SET_AEC_PRECAPTURE_ID:
      payload_size = sizeof(int32_t);
      break;
   case MCT_BUS_MSG_SET_AEC_RESET:
      payload_size = 0;
      break;
   case MCT_BUS_MSG_SET_AF_STATE:
      payload_size = sizeof(int32_t);
      break;
    case MCT_BUS_MSG_UPDATE_AF_FOCUS_POS:
      payload_size = sizeof(cam_focus_pos_info_t);
      break;
   case MCT_BUS_MSG_SET_AF_TRIGGER_ID:
      payload_size = sizeof(int32_t);
    case MCT_BUS_MSG_AUTO_SCENE_DECISION:
      payload_size = sizeof(mct_bus_msg_asd_decision_t);
      break;
   case MCT_BUS_MSG_ERROR_MESSAGE:
      payload_size = sizeof(mct_bus_msg_error_message_t);
      break;
   case MCT_BUS_MSG_AE_INFO:
      payload_size = sizeof(cam_ae_params_t);
      break;
   case MCT_BUS_MSG_AWB_INFO:
      payload_size = sizeof(cam_awb_params_t);
      break;
    case MCT_BUS_MSG_AE_EXIF_DEBUG_INFO:
       payload_size = sizeof(cam_ae_exif_debug_t);
       break;
    case MCT_BUS_MSG_AWB_EXIF_DEBUG_INFO:
       payload_size = sizeof(cam_awb_exif_debug_t);
       break;
   case MCT_BUS_MSG_AF_EXIF_DEBUG_INFO:
       payload_size = sizeof(cam_af_exif_debug_t);
       break;
   case MCT_BUS_MSG_ASD_EXIF_DEBUG_INFO:
       payload_size = sizeof(cam_asd_exif_debug_t);
       break;
   case MCT_BUS_MSG_STATS_EXIF_DEBUG_INFO:
       payload_size = sizeof(cam_stats_buffer_exif_debug_t);
       break;
   case MCT_BUS_MSG_SENSOR_INFO:
      payload_size = sizeof(cam_sensor_params_t);
      break;
   case MCT_BUS_MSG_NOTIFY_KERNEL:
      payload_size = 0;
      post_msg = TRUE;
      break;
   case MCT_BUS_MSG_SEND_HW_ERROR:
      payload_size = 0;
      post_msg = TRUE;
      pthread_mutex_lock(&bus->bus_msg_q_lock);
      mct_bus_queue_flush(bus);
      pthread_mutex_unlock(&bus->bus_msg_q_lock);
      break;
   case MCT_BUS_MSG_SENSOR_STARTING:
      bus->thread_wait_time = bus_msg->thread_wait_time;
      start_sof_check_thread(bus);
      return TRUE;
      break;
   case MCT_BUS_MSG_SENSOR_STOPPING:
      stop_sof_check_thread(bus);
      return TRUE;
   case MCT_BUS_MSG_SENSOR_AF_STATUS:
      payload_size = sizeof(mct_bus_msg_af_status_t);
      break;
   case MCT_BUS_MSG_META_VALID:
      payload_size = sizeof(mct_bus_msg_meta_valid);
      break;
   /* bus msg for 3a update */
   case MCT_BUS_MSG_AE_EZTUNING_INFO:
   case MCT_BUS_MSG_AWB_EZTUNING_INFO:
   case MCT_BUS_MSG_AF_EZTUNING_INFO:
   case MCT_BUS_MSG_AF_MOBICAT_INFO:
      payload_size = bus_msg->size;
      break;
   case MCT_BUS_MSG_ISP_CHROMATIX_LITE:
   case MCT_BUS_MSG_PP_CHROMATIX_LITE:
      payload_size = bus_msg->size;
      break;
   case MCT_BUS_MSG_ISP_META:
      payload_size = bus_msg->size;
      break;
   case MCT_BUS_MSG_FRAME_INVALID:
      payload_size = sizeof(cam_frame_idx_range_t);
      break;
   case MCT_BUS_MSG_SENSOR_META:
      payload_size = bus_msg->size;
      break;
   case MCT_BUS_MSG_PREPARE_HDR_ZSL_DONE:
      payload_size = sizeof(cam_prep_snapshot_state_t);
      break;
   case MCT_BUS_MSG_REPROCESS_STAGE_DONE:
      payload_size = 0;
      post_msg = TRUE;
      break;
   case MCT_BUS_MSG_SEND_EZTUNE_EVT:
      payload_size = 0;
      post_msg = TRUE;
      break;
   case MCT_BUS_MSG_PP_SET_META:
      payload_size = bus_msg->size;
      post_msg = TRUE;
      break;
   case MCT_BUS_MSG_WM_BUS_OVERFLOW_RECOVERY:
      payload_size = bus_msg->size;
      post_msg = TRUE;
      break;
   default:
      CDBG("%s: bus_msg type is not valid", __func__);
      goto error_2;
  }
  if (bus->msg_to_send_metadata == bus_msg->type) {
    post_msg = TRUE;
  }

  local_msg = malloc(sizeof(mct_bus_msg_t));

  if (!local_msg) {
    CDBG_ERROR("%s:%d Can't allocate memory", __func__, __LINE__);
    goto error_2;
  }

  local_msg->sessionid = bus_msg->sessionid;
  local_msg->type = bus_msg->type;
  local_msg->size = bus_msg->size;

  if (payload_size) {
    local_msg->msg = malloc(payload_size);
    if (!local_msg->msg) {
      CDBG_ERROR("%s:%d Can't allocate memory", __func__, __LINE__);
      goto error_1;
    }
    memcpy(local_msg->msg, bus_msg->msg, payload_size);
  } else {
    local_msg->msg = NULL;
  }

  /* Push message to Media Controller/Pipeline BUS Queue
   * and post signal to Media Controller */
  pthread_mutex_lock(&bus->bus_msg_q_lock);
  mct_queue_push_tail(bus->bus_queue, local_msg);
  pthread_mutex_unlock(&bus->bus_msg_q_lock);

  if (post_msg) {
    pthread_mutex_lock(bus->mct_mutex);
    bus->bus_cmd_q_flag = TRUE;
    pthread_cond_signal(bus->mct_cond);
    pthread_mutex_unlock(bus->mct_mutex);
  }


  return TRUE;

error_1:
  free(local_msg);
error_2:
  return FALSE;
}



mct_bus_t *mct_bus_create(unsigned int session)
{
  mct_bus_t *new_bus;
  new_bus = malloc(sizeof(mct_bus_t));
  if (!new_bus) {
    /* print error code here strerror(errno) */
    return FALSE;
  }

  memset(new_bus, 0 , sizeof(mct_bus_t));
  pthread_mutex_init(&new_bus->bus_msg_q_lock, NULL);
  pthread_mutex_init(&new_bus->bus_sof_init_lock, NULL);
  pthread_cond_init(&new_bus->bus_sof_init_cond, NULL);

  new_bus->bus_queue = mct_queue_new;
  if (!new_bus->bus_queue)
    goto busmsgq_error;

  mct_queue_init(new_bus->bus_queue);
  new_bus->post_msg_to_bus = msg_bus_post_msg;

  new_bus->session_id = session;
  return new_bus;

busmsgq_error:
  pthread_cond_destroy(&new_bus->bus_sof_init_cond);
  pthread_mutex_destroy(&new_bus->bus_sof_init_lock);
  pthread_mutex_destroy(&new_bus->bus_msg_q_lock);
  free(new_bus);
  return NULL;
}

void mct_bus_destroy(mct_bus_t *bus)
{
  pthread_mutex_lock(&bus->bus_msg_q_lock);
  if (!MCT_QUEUE_IS_EMPTY(bus->bus_queue))
    mct_queue_free_all(bus->bus_queue, mct_bus_queue_free);
  else
    free(bus->bus_queue);
  bus->bus_queue = NULL;

  pthread_mutex_unlock(&bus->bus_msg_q_lock);

  pthread_cond_destroy(&bus->bus_sof_init_cond);
  pthread_mutex_destroy(&bus->bus_sof_init_lock);
  pthread_mutex_destroy(&bus->bus_msg_q_lock);
  free(bus);
  bus = NULL;
  return;
}


void mct_bus_queue_flush(mct_bus_t *bus)
{
  if (!bus)
    return;

  if (!MCT_QUEUE_IS_EMPTY(bus->bus_queue))
    mct_queue_flush(bus->bus_queue, mct_bus_queue_free);

  return;
}

