/* is_thread.c
 *
 * Copyright (c) 2013 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "is_thread.h"
#include "is_port.h"
#include "camera_dbg.h"
#include <sys/syscall.h>
#include <sys/prctl.h>

#if 0
#undef CDBG
#define CDBG ALOGE
#endif

#undef CDBG_HIGH
#if 1
#define CDBG_HIGH ALOGE
#else
#define CDBG_HIGH
#endif


/** is_thread_handler
 *    @data: IS thread data
 *
 *  This is the IS thread's main function.
 *
 *  Returns NULL
 **/
static void* is_thread_handler(void *data)
{
  is_thread_msg_t *msg = NULL;
  is_thread_data_t *thread_data = (is_thread_data_t *)data;
  int exit_flag = 0;
  is_port_private_t *private = thread_data->is_port->port_private;
  boolean rc;

  CDBG_HIGH("%s thread_id is %d\n",__func__, syscall(SYS_gettid));
  prctl(PR_SET_NAME, "is_thread", 0, 0, 0);
  if (!private) {
    return NULL;
  }

  thread_data->active = 1;
  sem_post(&thread_data->sem_launch);
  CDBG_HIGH("%s: Starting IS thread handler", __func__);

  do {
    pthread_mutex_lock(&thread_data->thread_mutex);
    while (thread_data->msg_q->length == 0) {
      pthread_cond_wait(&thread_data->thread_cond, &thread_data->thread_mutex);
    }
    pthread_mutex_unlock(&thread_data->thread_mutex);

    /* Get the message */
    pthread_mutex_lock(&thread_data->msg_q_lock);
    msg = (is_thread_msg_t *)
      mct_queue_pop_head(thread_data->msg_q);
    pthread_mutex_unlock(&thread_data->msg_q_lock);

    if (!msg) {
      CDBG_ERROR("%s: msg NULL", __func__);
      continue;
    }

    /* Flush the queue if it is stopping. Free the enqueued messages */
    if (thread_data->active == 0) {
      if (msg->type != MSG_IS_STOP_THREAD) {
        free(msg);
        msg = NULL;
        continue;
      }
    }

    CDBG("%s: Got event type %d", __func__, msg->type);
    switch (msg->type) {
    case MSG_IS_PROCESS:
      ATRACE_BEGIN("Camera:IS");
      rc = private->process(&msg->u.is_process_parm, &private->is_process_output);
      ATRACE_END();
      if (rc) {
        private->callback(thread_data->is_port, &private->is_process_output);
      }
      break;

    case MSG_IS_SET:
      private->set_parameters(&msg->u.is_set_parm, &private->is_info);
      break;

    case MSG_IS_STOP_THREAD:
      exit_flag = 1;
      break;

    default:
      break;
    }

    free(msg);
    msg = NULL;
  } while (!exit_flag);

  CDBG_HIGH("%s: Exiting IS thread handler", __func__);
  return NULL;
}


/** is_thread_en_q_msg
 *    @thread_data: IS thread data
 *    @msg: message to enqueue
 *
 *  Enqueues message to the IS thread's queue.
 *
 *  Returns TRUE on success, FALSE on failure.
 **/
boolean is_thread_en_q_msg(is_thread_data_t *thread_data, is_thread_msg_t *msg)
{
  boolean rc = FALSE;

  pthread_mutex_lock(&thread_data->msg_q_lock);
  if (thread_data->active) {
    rc = TRUE;
    mct_queue_push_tail(thread_data->msg_q, msg);

    if (msg->type == MSG_IS_STOP_THREAD) {
      thread_data->active = 0;
    }
  }
  pthread_mutex_unlock(&thread_data->msg_q_lock);

  if (rc) {
    pthread_mutex_lock(&thread_data->thread_mutex);
    pthread_cond_signal(&thread_data->thread_cond);
    pthread_mutex_unlock(&thread_data->thread_mutex);
  } else {
    free(msg);
  }

  return rc;
}


/** is_thread_start
 *    @thread_data: IS thread data
 *
 *  This function creates the IS thread.
 *
 *  Returns TRUE on success
 **/
boolean is_thread_start(is_thread_data_t *thread_data)
{
  boolean rc = TRUE;
  CDBG("%s: is thread start! ", __func__);

  if (!pthread_create(&thread_data->thread_id, NULL, is_thread_handler,
    (void *)thread_data)) {
    sem_wait(&thread_data->sem_launch);
  } else {
    rc = FALSE;
  }
  pthread_setname_np(thread_data->thread_id, "IS");
  return rc;
}


/** is_thread_stop
 *    @thread_data: IS thread data
 *
 *  This function puts the MSG_STOP_THREAD message to the IS thread's queue so
 *  that the thread will stop.  After the message has been successfuly queued,
 *  it waits for the IS thread to join.
 *
 *  Returns TRUE on success, FALSE on failure.
 **/
boolean is_thread_stop(is_thread_data_t *thread_data)
{
  boolean rc;
  is_thread_msg_t *msg;

  msg = malloc(sizeof(is_thread_msg_t));
  CDBG("%s: is thread stop! ", __func__);

  if (msg) {
    memset(msg, 0, sizeof(is_thread_msg_t));
    msg->type = MSG_IS_STOP_THREAD;
    rc = is_thread_en_q_msg(thread_data, msg);

    if (rc) {
      pthread_join(thread_data->thread_id, NULL);
    }
  } else {
    rc = FALSE;
  }
  return rc;
} /* is_thread_stop */


/** is_thread_init
 *
 *  Initializes the IS thread data and creates the queue.
 *
 *  Returns the thread data object, on failure returnw NULL.
 **/
is_thread_data_t* is_thread_init(void)
{
  is_thread_data_t *is_thread_data;

  is_thread_data = malloc(sizeof(is_thread_data_t));
  if (is_thread_data == NULL) {
    return NULL;
  }
  memset(is_thread_data, 0, sizeof(is_thread_data_t));

  CDBG("%s: Create IS queue ", __func__);
  is_thread_data->msg_q = (mct_queue_t *)mct_queue_new;

  if (!is_thread_data->msg_q) {
    free(is_thread_data);
    return NULL;
  }

  CDBG("%s: Initialize the IS queue! ", __func__);
  pthread_mutex_init(&is_thread_data->msg_q_lock, NULL);
  mct_queue_init(is_thread_data->msg_q);
  pthread_cond_init(&is_thread_data->thread_cond, NULL);
  pthread_mutex_init(&is_thread_data->thread_mutex, NULL);
  sem_init(&is_thread_data->sem_launch, 0, 0);

  return is_thread_data;
} /* is_thread_init */


/** is_thread_deinit
 *    @thread_data: IS thread data
 *
 *  This function frees resources associated with the IS thread.
 *
 *  Returns void.
 **/
void is_thread_deinit(is_thread_data_t *thread_data)
{
  CDBG("%s called", __func__);
  pthread_mutex_destroy(&thread_data->thread_mutex);
  pthread_cond_destroy(&thread_data->thread_cond);
  mct_queue_free(thread_data->msg_q);
  pthread_mutex_destroy(&thread_data->msg_q_lock);
  sem_destroy(&thread_data->sem_launch);
  free(thread_data);
} /* is_thread_deinit */
