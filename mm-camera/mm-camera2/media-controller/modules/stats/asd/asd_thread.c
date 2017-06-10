/* asd_thread.c
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <pthread.h>
#include "mct_queue.h"
#include "asd_thread.h"
#include "modules.h"
#include <math.h>
#include <sys/syscall.h>
#include <sys/prctl.h>

#include "camera_dbg.h"
/** asd_thread_init
 *
 **/
asd_thread_data_t* asd_thread_init(void)
{
  asd_thread_data_t *thread_data;

  thread_data = malloc(sizeof(asd_thread_data_t));
  if (thread_data == NULL)
    return NULL;

  memset(thread_data, 0, sizeof(asd_thread_data_t));
  thread_data->msg_q = (mct_queue_t *)mct_queue_new;
  if (!thread_data->msg_q) {
    free(thread_data);
    return NULL;
  }

  pthread_mutex_init(&thread_data->msg_q_lock, NULL);
  mct_queue_init(thread_data->msg_q);
  pthread_cond_init(&(thread_data->thread_cond), NULL);
  pthread_mutex_init(&(thread_data->thread_mutex), NULL);
  sem_init(&thread_data->sem_launch, 0, 0);

  return thread_data;
}

/** asd_thread_deinit
 *    @p:
 *
 *  deinitialize ASD thread related resources
 *
 *  Return: No
 **/
void asd_thread_deinit(asd_thread_data_t *thread_data)
{
  if (!thread_data) {
    CDBG_ERROR("%s thread_data is NULL", __func__);
    return;
  }
  CDBG("%s thread_data: %p", __func__, thread_data);
  pthread_mutex_destroy(&thread_data->thread_mutex);
  pthread_cond_destroy(&thread_data->thread_cond);
  mct_queue_free(thread_data->msg_q);
  pthread_mutex_destroy(&thread_data->msg_q_lock);
  sem_destroy(&thread_data->sem_launch);
  free(thread_data);
}

/** asd_thread_en_q_msg:
 *
 **/
boolean asd_thread_en_q_msg(void *asd_data,
  asd_thread_msg_t  *msg)
{
  asd_thread_data_t *thread_data = (asd_thread_data_t *)asd_data;
  boolean rc = FALSE;

  if (!msg || !asd_data) {
    ALOGE("%s: Invalid parameters!", __func__);
    return FALSE;
  }

  CDBG("%s: Enqueue ASD message", __func__);

  pthread_mutex_lock(&thread_data->msg_q_lock);
  if (thread_data->active) {
    mct_queue_push_tail(thread_data->msg_q, msg);
    if (msg->type == MSG_STOP_THREAD) {
      thread_data->active = 0;
      CDBG("%s:%d Message Stop Thread", __func__, __LINE__);
    }
    rc = TRUE;
    if (msg->type == MSG_ASD_STATS) {
      CDBG("%s: Stats msg of stats_mask: %d", __func__,
      msg->u.stats->stats_type_mask);
    }
  }
  pthread_mutex_unlock(&thread_data->msg_q_lock);

  if (rc) {
    pthread_mutex_lock(&thread_data->thread_mutex);
    pthread_cond_signal(&thread_data->thread_cond);
    pthread_mutex_unlock(&thread_data->thread_mutex);
    CDBG("%s: Singalled ASD thread handler!", __func__);
  } else {
    ALOGE("%s: ASD thread_data is not active: %d", __func__,
      thread_data->active);
    rc = FALSE;
    free(msg);
  }

  return rc;
}

/** asd_thread_handler:
 *
 **/
static void* asd_thread_handler(asd_thread_data_t *thread_data)
{
  asd_thread_msg_t *msg = NULL;
  asd_object_t *asd_obj = NULL;
  int exit_flag = 0;
  uint8_t face_info_confidence = 0;
  boolean face_info_detected = FALSE;
  const uint8_t face_detect_threshold = 10;
  boolean face_info_updated = FALSE;
  uint8_t face_update_wait_cnt = 0;
  const uint8_t face_update_threshold = 3;

  sem_post(&thread_data->sem_launch);

  CDBG_HIGH("%s thread_id is %d\n",__func__, syscall(SYS_gettid));
  prctl(PR_SET_NAME, "asd_thread", 0, 0, 0);
  if (!thread_data)
    return NULL;

  asd_obj = thread_data->asd_obj;

  CDBG("%s: Starting ASD thread handler", __func__);
  do {
    pthread_mutex_lock(&thread_data->thread_mutex);
    while (thread_data->msg_q->length == 0) {
       pthread_cond_wait(&thread_data->thread_cond,
         &thread_data->thread_mutex);
    }
    pthread_mutex_unlock(&thread_data->thread_mutex);

    CDBG("%s: Got signal - time to wake up!", __func__);
    /* Get the message */
    pthread_mutex_lock(&thread_data->msg_q_lock);
    msg = (asd_thread_msg_t *)
      mct_queue_pop_head(thread_data->msg_q);
    pthread_mutex_unlock(&thread_data->msg_q_lock);

    if (!msg) {
      continue;
    }

    if(thread_data->active == 0) {
      if(msg->type != MSG_STOP_THREAD) {
          free(msg);
          msg = NULL;
          continue;
      }
    }

    /* Process message accordingly */
    CDBG("%s: Got the message of type: %d", __func__, msg->type);
    switch (msg->type) {
    case MSG_ASD_SET:
      CDBG("%s: Set ASD parameters!", __func__);
      if (asd_obj->asd_ops.set_parameters) {
        asd_obj->asd_ops.set_parameters(&msg->u.asd_set_parm, asd_obj->asd);
      }
      break;

    case MSG_ASD_STATS:
      CDBG("%s: Received HISTO Stats for ASD!", __func__);
      /* We'll just update the stats */
      memcpy(&thread_data->process_data.stats, msg->u.stats,
        sizeof(stats_t));
      break;
    case MSG_AEC_DATA: {
      CDBG("%s: AEC data received!", __func__);
      /* don't process ASD till AWB update data is received */
      if (thread_data->process == ASD_NO_ACTION) {
        thread_data->process |= ASD_AEC_UPDATED;
        /* store aec_data, wait AWB_DATA before asd_process */
        CDBG("%s: Store AEC data and wait for AWB data!", __func__);
        thread_data->process_data.aec_data = msg->u.aec_data;
      } else {
        thread_data->process = ASD_NO_ACTION;
        break;
      }
    }
      break;

    case MSG_AWB_DATA: {
      CDBG("%s: AWB data received!", __func__);
      thread_data->process |= ASD_AWB_UPDATED;

      if (thread_data->process & ASD_ASD_PROCESS) {
        CDBG("%s: Store AWB data and start ASD process now!", __func__);
        thread_data->process_data.awb_data = msg->u.awb_data;

        ATRACE_BEGIN("Camera:ASD");
        asd_obj->asd_ops.process(&(thread_data->process_data),
          asd_obj->asd, &(asd_obj->output));
        ATRACE_END();
        thread_data->asd_cb(&(asd_obj->output), thread_data->asd_port);

        thread_data->process = ASD_NO_ACTION;
      }
    }
      break;
    case MSG_FACE_INFO: {
      size_t i;
      thread_data->process |= ASD_FACE_INFO_UPDATED;
      //need to call process if state is process? or wait for awb_update?

      /* face info is updated*/
      face_update_wait_cnt = 0;
      face_info_updated = TRUE;
      if (msg->u.face_data.face_count > 0) {
        face_info_detected = TRUE;
        if (face_info_confidence < face_detect_threshold)
          face_info_confidence++;
      }

      CDBG("%s: FACE info received w/ %d faces!", __func__, msg->u.face_data.face_count);

      //update to most recent roi
      asd_data_face_info_t * face_data =
        &thread_data->process_data.face_data;
      asd_data_face_info_t * new_face_data =
        &(msg->u.face_data);

      face_data->face_count = msg->u.face_data.face_count;

      for (i = 0; i < face_data->face_count; i++) {
        face_data->faces[i].roi = new_face_data->faces[i].roi;
        face_data->faces[i].score = new_face_data->faces[i].score;
      }
      break;
    }
    case MSG_STOP_THREAD:
      exit_flag = 1;
      break;
    case MSG_SOF:
      CDBG("%s: SOF event", __func__);
      thread_data->process_data.frame_count++;

      //sticky logic for face count
      if(!face_info_updated &&
        (face_update_wait_cnt < face_update_threshold)){
        //no face info message updated, increase wait cnt, and break.
        face_update_wait_cnt++;
        break;
      }

      //if no face detected, reduce confidence.
      if (!face_info_detected) {
        if (face_info_confidence > 0)
          face_info_confidence--;
      }

      //update ASD process state if needed
      //note that this is based off the knowledge of the
      //last X frames. we don't know yet if a face is
      //detected for this frame.
      asd_data_face_info_t * face_data =
        &thread_data->process_data.face_data;
      if (face_info_confidence == 0) {
        face_data->use_roi = 0;
      } else if (face_info_confidence == face_detect_threshold) {
        face_data->use_roi = 1;
      }

      face_info_detected = FALSE;
      /* set update to false, and wait to process next face_info*/
      face_info_updated = FALSE;
      break;
    default:
      break;
    }

    if (msg) {
      free(msg);
      msg = NULL;
    }
  } while (!exit_flag);

  return NULL;
}

/**
 *
 **/
boolean asd_thread_start(asd_thread_data_t *thread_data)
{
  pthread_t id;

  pthread_create(&id, NULL, asd_thread_handler, thread_data);
  pthread_setname_np(id, "ASD");
  sem_wait(&thread_data->sem_launch);
  thread_data->thread_id = id;
  thread_data->active    = 1;

  return TRUE;
}

/** asd_thread_stop:
 *
 **/
boolean asd_thread_stop(asd_thread_data_t *asd_data)
{
  boolean rc ;
  asd_thread_msg_t *msg = malloc(sizeof(asd_thread_msg_t));

  if (msg) {

    msg->type = MSG_STOP_THREAD;
    rc = asd_thread_en_q_msg(asd_data, msg);
    if(rc)
      pthread_join(asd_data->thread_id, NULL);
  } else {
    rc = FALSE;
  }

  return rc;
}
