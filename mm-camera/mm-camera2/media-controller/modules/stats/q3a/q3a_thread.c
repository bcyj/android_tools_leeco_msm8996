/* q3a_thread.c
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <pthread.h>
#include "mct_queue.h"
#include "q3a_thread.h"
#include <sys/syscall.h>
#include <sys/prctl.h>
#include "camera_dbg.h"


#if Q3A_THREAD_DEBUG
#undef CDBG
#define CDBG CDBG_ERROR
#endif

/** q3a_thread_aecawb_init
 *
 *  Initializes the AECAWB thread data and creates the queues.
 *
 *  Return the the AECAWB thread data object, on failure return NULL.
 **/
q3a_thread_aecawb_data_t* q3a_thread_aecawb_init(void)
{
  q3a_thread_aecawb_data_t *aecawb;

  aecawb = malloc(sizeof(q3a_thread_aecawb_data_t));
  if (aecawb == NULL)
    return NULL;
  memset(aecawb, 0 , sizeof(q3a_thread_aecawb_data_t));

  aecawb->thread_data = malloc(sizeof(q3a_thread_data_t));
  if (aecawb->thread_data == NULL) {
    free(aecawb);
    return NULL;
  }
  memset(aecawb->thread_data, 0 , sizeof(q3a_thread_data_t));

  aecawb->thread_data->msg_q = (mct_queue_t *)mct_queue_new;
  aecawb->thread_data->p_msg_q = (mct_queue_t *)mct_queue_new;
  if (!aecawb->thread_data->msg_q || !aecawb->thread_data->p_msg_q) {
    if(aecawb->thread_data->msg_q)
      mct_queue_free(aecawb->thread_data->msg_q);
    if(aecawb->thread_data->p_msg_q)
      mct_queue_free(aecawb->thread_data->p_msg_q);
    free(aecawb->thread_data);
    free(aecawb);
    return NULL;
  }

  pthread_mutex_init(&aecawb->thread_data->msg_q_lock, NULL);
  mct_queue_init(aecawb->thread_data->msg_q);
  mct_queue_init(aecawb->thread_data->p_msg_q);
  sem_init(&aecawb->thread_data->sem_launch, 0, 0);
  pthread_cond_init(&(aecawb->thread_data->thread_cond), NULL);
  pthread_mutex_init(&(aecawb->thread_data->thread_mutex), NULL);
  CDBG("%s private->thread_data: %p", __func__, aecawb->thread_data);

  return aecawb;
} /* q3a_thread_aecawb_init */

/** q3a_thread_aecawb_deinit
 *    @aecawb_data: The pointer to the aecawb thread data
 *
 *  Deinitializes the AECAWB thread data - frees the queues, destroys the
 *  sync variables and frees the thread data object.
 *
 *  Return void.
 **/
void q3a_thread_aecawb_deinit(q3a_thread_aecawb_data_t *aecawb)
{
  CDBG("%s thread_data: %p", __func__, aecawb->thread_data);
  pthread_mutex_destroy(&aecawb->thread_data->thread_mutex);
  pthread_cond_destroy(&aecawb->thread_data->thread_cond);
  mct_queue_free(aecawb->thread_data->msg_q);
  mct_queue_free(aecawb->thread_data->p_msg_q);
  pthread_mutex_destroy(&aecawb->thread_data->msg_q_lock);
  sem_destroy(&aecawb->thread_data->sem_launch);
  free(aecawb->thread_data);
  free(aecawb);
} /* q3a_thread_aecawb_deinit */

/** q3a_aecawb_thread_en_q_msg
 *    @aecawb_data: The pointer to the aecawb thread data
 *    @msg:         The message to be put in the queue
 *
 *  Enqueues the sent message into the thread's queue. If the message has the
 *  priority flag set, it will be put in the priority queue. Upon receiving the
 *  MSG_STOP_THREAD type of message, the queue will no longer be active and
 *  no one will be able to enqueue messages in it.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
boolean q3a_aecawb_thread_en_q_msg(void *aecawb_data,
  q3a_thread_aecawb_msg_t *msg)
{
  q3a_thread_data_t *thread_data = (q3a_thread_data_t *)aecawb_data;
  boolean           rc = FALSE;
  boolean           sync_flag_set = FALSE;
  msg_sync_t        msg_sync;

  CDBG("%s: Enqueue AEC/AWB message %p", __func__, msg);

  pthread_mutex_lock(&thread_data->msg_q_lock);
  if (thread_data->active) {
    rc = TRUE;
    CDBG("%s:%d, type=%d, sync_falg=%d", __func__, __LINE__,
      msg->type, msg->sync_flag);
    if (msg->sync_flag == TRUE) {
      msg->sync_obj = &msg_sync;
      sem_init(&msg_sync.msg_sem, 0, 0);
      sync_flag_set = TRUE;
    }
    CDBG("%s:%d, lock Q", __func__, __LINE__);
    //If its a priority event, queue to priority queue, else to normal queue
    if (msg->type == MSG_BG_AEC_STATS) {
      thread_data->aec_bg_stats_cnt++;
    }
    if (msg->type == MSG_BG_AWB_STATS) {
      thread_data->awb_bg_stats_cnt++;
    }
    if (msg->is_priority) {
      mct_queue_push_tail(thread_data->p_msg_q, msg);
    } else {
      mct_queue_push_tail(thread_data->msg_q, msg);
    }

    if (msg->type == MSG_STOP_THREAD) {
      thread_data->active = 0;
      CDBG("%s:%d active is zero", __func__, __LINE__);
    }
  }
  pthread_mutex_unlock(&thread_data->msg_q_lock);

  if (rc) {
    pthread_mutex_lock(&thread_data->thread_mutex);
    pthread_cond_signal(&thread_data->thread_cond);
    pthread_mutex_unlock(&thread_data->thread_mutex);

    if (TRUE == sync_flag_set) {
      sem_wait(&msg_sync.msg_sem);
      sem_destroy(&msg_sync.msg_sem);
    }
    CDBG("%s: Signalled AWB thread handler", __func__);
  } else {
    CDBG("%s: AWB thread_data not active: %d", __func__, thread_data->active);
    free(msg);
  }
  return rc;
} /* q3a_aecawb_thread_en_q_msg */

/** aecawb_thread_handler
 *    @aecawb_data: The pointer to the aecawb thread data
 *
 *  This is the aecawb thread that will run until it receives the STOP message.
 *  While running, it will dequeue messages from the thread's queue and process
 *  them. If there are no messages to process (queue is empty), the thread will
 *  sleep until it gets signaled.
 *
 *  Return NULL
 **/
static void* aecawb_thread_handler(void *aecawb_data)
{
  q3a_thread_aecawb_data_t *aecawb = (q3a_thread_aecawb_data_t *)aecawb_data;
  q3a_thread_aecawb_msg_t  *msg = NULL;
  int                      exit_flag = 0;
  int                      rc;

  aecawb->thread_data->active = 1;
  sem_post(&aecawb->thread_data->sem_launch);

  CDBG_HIGH("%s thread_id is %d\n",__func__, syscall(SYS_gettid));
  prctl(PR_SET_NAME, "aecawb_thread", 0, 0, 0);
  do {
    pthread_mutex_lock(&aecawb->thread_data->thread_mutex);
    while ((aecawb->thread_data->msg_q->length == 0) &&
      (aecawb->thread_data->p_msg_q->length == 0)) {
      pthread_cond_wait(&aecawb->thread_data->thread_cond,
        &aecawb->thread_data->thread_mutex);
    }
    pthread_mutex_unlock(&aecawb->thread_data->thread_mutex);

    /* Get the message */
    pthread_mutex_lock(&aecawb->thread_data->msg_q_lock);
    /*Pop from priority queue first and if its empty pop from normal queue*/
    msg = (q3a_thread_aecawb_msg_t *)
      mct_queue_pop_head(aecawb->thread_data->p_msg_q);

    if (!msg) {
      msg = (q3a_thread_aecawb_msg_t *)
        mct_queue_pop_head(aecawb->thread_data->msg_q);
    }
    pthread_mutex_unlock(&aecawb->thread_data->msg_q_lock);
    if (!msg) {
      CDBG_ERROR("%s: msg NULL", __func__);
      continue;
    }

    /* Flush the queue if it is stopping. Free the enqueued messages and
     * signal the sync message owners to release their resources */
    if(aecawb->thread_data->active == 0) {
      if(msg->type != MSG_STOP_THREAD) {
        if (msg->sync_flag == TRUE) {
           sem_post(&msg->sync_obj->msg_sem);
           /* Don't free msg, the sender will do */
           msg = NULL;
        }
        if (msg) {
          /* Stats messages are not synced, so we have to free the payload
           * here. */
          if ( ((msg->type == MSG_AEC_STATS) ||
            (msg->type == MSG_BG_AEC_STATS)) &&
            ((msg->u.stats->stats_type_mask & STATS_AEC) ||
            (msg->u.stats->stats_type_mask & STATS_BG)) ) {
            free(msg->u.stats);
          }

          free(msg);
          msg = NULL;
        }
        continue;
      }
    }

    /* Process message accordingly */
    CDBG("%s: wake up type=%d, flag=%d", __func__,msg->type, msg->sync_flag);
    switch (msg->type) {
    case MSG_AEC_SET: {
      if (aecawb->aec_obj->set_parameters) {
        aecawb->aec_obj->set_parameters(&msg->u.aec_set_parm,
          aecawb->aec_obj->aec);
      } else {
        CDBG_ERROR("%s: Error: set_parameters is null", __func__);
      }
    }
      break;

    case MSG_AEC_GET: {
      if (aecawb->aec_obj->get_parameters) {
        aecawb->aec_obj->get_parameters(&msg->u.aec_get_parm,
          aecawb->aec_obj->aec);
      }
    }
      break;

    case MSG_AEC_STATS: {
      if (msg->u.stats) {
        free(msg->u.stats);
      }
    }
      break;

    case MSG_AEC_STATS_HDR:
    case MSG_BG_AEC_STATS: {
      if ((aecawb->thread_data->aec_bg_stats_cnt < 3) ||
        (msg->type == MSG_AEC_STATS_HDR)) {
        memset(&(aecawb->aec_obj->output), 0, sizeof(aec_output_data_t));
        ATRACE_BEGIN("Camera:AEC");
        rc = aecawb->aec_obj->process(msg->u.stats, aecawb->aec_obj->aec,
          &(aecawb->aec_obj->output));
        ATRACE_END();
        if (rc == TRUE) {
          aecawb->aec_obj->output.type = AEC_UPDATE;
          aecawb->aec_cb(& (aecawb->aec_obj->output), aecawb->aec_port);
        }
      }
      if (msg->u.stats) {
        if (aecawb->thread_data->aec_bg_stats_cnt && msg->type ==
          MSG_BG_AEC_STATS) {
          pthread_mutex_lock(&aecawb->thread_data->msg_q_lock);
          aecawb->thread_data->aec_bg_stats_cnt--;
          pthread_mutex_unlock(&aecawb->thread_data->msg_q_lock);
        }
        free(msg->u.stats);
      }
    }
      break;

    case MSG_AEC_SEND_EVENT: {
      aec_output_data_t output;

      output.type = AEC_SEND_EVENT;
      aecawb->aec_cb(&(output), aecawb->aec_port);
    }
      break;

    case MSG_AWB_SEND_EVENT: {
        awb_output_data_t output;

        output.type = AWB_SEND_EVENT;
        aecawb->awb_cb(&(output), aecawb->awb_port);
    }
      break;

    case MSG_AWB_SET: {
      aecawb->awb_obj->awb_ops.set_parameters(&msg->u.awb_set_parm,
        aecawb->awb_obj->awb);
    }
    break;

    case MSG_AWB_GET: {
      aecawb->awb_obj->awb_ops.get_parameters(&msg->u.awb_get_parm,
        aecawb->awb_obj->awb);
    }
      break;

    case MSG_BG_AWB_STATS: {
      if (aecawb->thread_data->awb_bg_stats_cnt < 3) {
        memset(&(aecawb->awb_obj->output), 0, sizeof(awb_output_data_t));
        ATRACE_BEGIN("Camera:AWB");
        aecawb->awb_obj->awb_ops.process(
          msg->u.stats, aecawb->awb_obj->awb, &(aecawb->awb_obj->output));
        ATRACE_END();
        aecawb->awb_cb(&(aecawb->awb_obj->output), aecawb->awb_port);
      }
      if (aecawb->thread_data->awb_bg_stats_cnt) {
        pthread_mutex_lock(&aecawb->thread_data->msg_q_lock);
        aecawb->thread_data->awb_bg_stats_cnt--;
        pthread_mutex_unlock(&aecawb->thread_data->msg_q_lock);
      }
    }
      break;

   case MSG_STOP_THREAD: {
     exit_flag = 1;
   }
     break;

    default: {
    }
      break;
    } /* end switch (msg->type) */
    if (msg->sync_flag == TRUE) {
         sem_post(&msg->sync_obj->msg_sem);
       /*don't free msg, the sender will do*/
       msg = NULL;
    }
    if (msg) {
      free(msg);
      msg = NULL;
    }
  } while (!exit_flag);
  return NULL;
} /* aecawb_thread_handler */

/** q3a_thread_aecawb_start
 *    @aecawb_data: The pointer to the aecawb thread data
 *
 *  Called to create the aecawb thread. It will wait on a semaphore until the
 *  thread is created and running.
 *
 *  Return TRUE
 **/
boolean q3a_thread_aecawb_start(q3a_thread_aecawb_data_t *aecawb_data)
{
  pthread_create(&aecawb_data->thread_data->thread_id, NULL,
    aecawb_thread_handler, aecawb_data);
  pthread_setname_np(aecawb_data->thread_data->thread_id, "AECAWB");
  sem_wait(&aecawb_data->thread_data->sem_launch);
  aecawb_data->thread_data->aec_bg_stats_cnt = 0;
  aecawb_data->thread_data->awb_bg_stats_cnt = 0;
  return TRUE;
} /* q3a_thread_aecawb_start */

/** q3a_thread_aecawb_stop
 *    @aecawb_data: The pointer to the aecawb thread data
 *
 *  Called to stop the aecawb thread. It will send a MSG_STOP_THREAD message to
 *  the queue and will wait for the thread to join if the message is enqueued
 *  successfully.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
boolean q3a_thread_aecawb_stop(q3a_thread_aecawb_data_t *aecawb_data)
{
  boolean                 rc ;
  q3a_thread_aecawb_msg_t *msg;

  msg = malloc(sizeof(q3a_thread_aecawb_msg_t));

  if (msg) {
    CDBG_ERROR("%s:%d MSG_STOP_THREAD", __func__, __LINE__);
    memset(msg, 0, sizeof(q3a_thread_aecawb_msg_t));
    msg->type = MSG_STOP_THREAD;
    aecawb_data->thread_data->aec_bg_stats_cnt = 0;
    aecawb_data->thread_data->awb_bg_stats_cnt = 0;
    rc = q3a_aecawb_thread_en_q_msg(aecawb_data->thread_data,msg);

    if (rc) {
      pthread_join(aecawb_data->thread_data->thread_id, NULL);
      CDBG("%s:%d pthread_join", __func__, __LINE__);
    }
  } else {
    rc = FALSE;
  }
  return rc;
} /* q3a_thread_aecawb_stop */

/** af_thread_handler
 *    @af_data: The pointer to the af thread data
 *
 *  This is the af thread that will run until it receives the STOP message.
 *  While running, it will dequeue messages from the thread's queue and process
 *  them. If there are no messages to process (queue is empty), the thread will
 *  sleep until it gets signaled.
 *
 *  Return NULL
 **/
static void* af_thread_handler(void *af_data)
{
  q3a_thread_af_data_t *af = (q3a_thread_af_data_t *)af_data;
  q3a_thread_af_msg_t  *msg = NULL;
  int                  exit_flag = 0;

  CDBG_HIGH("%s thread_id is %d\n",__func__, syscall(SYS_gettid));
  prctl(PR_SET_NAME, "af_thread", 0, 0, 0);
  af->thread_data->active = 1;
  sem_post(&af->thread_data->sem_launch);
  CDBG("%s: %d: Starting AF thread handler", __func__,__LINE__);

  do {
    CDBG("%s: Waiting for message", __func__);

    pthread_mutex_lock(&af->thread_data->thread_mutex);
    while ((af->thread_data->msg_q->length == 0) &&
      (af->thread_data->p_msg_q->length == 0)) {
      pthread_cond_wait(&af->thread_data->thread_cond,
        &af->thread_data->thread_mutex);
    }
    pthread_mutex_unlock(&af->thread_data->thread_mutex);
    CDBG("%s: Got signal - time to wake up!", __func__);

    /* Get the message */
    pthread_mutex_lock(&af->thread_data->msg_q_lock);
    /*Pop from priority queue first and if its empty pop from normal queue*/
    msg = (q3a_thread_af_msg_t *) mct_queue_pop_head(af->thread_data->p_msg_q);

    if (!msg) {
      msg = (q3a_thread_af_msg_t *) mct_queue_pop_head(af->thread_data->msg_q);
    }
    pthread_mutex_unlock(&af->thread_data->msg_q_lock);

    if (!msg) {
      CDBG_ERROR("%s: msg NULL", __func__);
      continue;
    }

    /* Flush the queue if it is stopping. Free the enqueued messages and
     * signal the sync message owners to release their resources */
    if (af->thread_data->active == 0) {
      if (msg->type != MSG_AF_STOP_THREAD) {
        /* Stats messages are not synced, so we have to free the payload
         * here. */
        if ((msg->type == MSG_AF_STATS) || (msg->type == MSG_BF_STATS)) {
          free(msg->u.stats);
        }
        if (msg->sync_flag == TRUE) {
           sem_post(&msg->sync_obj->msg_sem);
           /*don't free msg, the sender will do*/
           msg = NULL;
        }
        if (msg) {
          free(msg);
          msg = NULL;
        }
        continue;
      }
    }

    /* Process message accordingly */
    CDBG("%s: Got the message of type: %d", __func__, msg->type);
    switch (msg->type) {
    case MSG_AF_START: {
      CDBG("%s: Got do_AF call from HAL!", __func__);
      af->af_obj->af_ops.set_parameters(&msg->u.af_set_parm,
        &(af->af_obj->output), af->af_obj->af);
      af->af_cb(& (af->af_obj->output), af->af_port);
    }
      break;

    case MSG_AF_CANCEL: {
      CDBG("%s: Got cancel_AF call from HALL ", __func__);
      af->af_obj->af_ops.set_parameters(&msg->u.af_set_parm,
        &(af->af_obj->output), af->af_obj->af);
      af->af_cb(& (af->af_obj->output), af->af_port);
    }
      break;

    case MSG_AF_GET: {
      CDBG("%s: Get me AF parameter", __func__);
      af->af_obj->af_ops.get_parameters(&msg->u.af_get_parm, af->af_obj->af);
    }
      break;

    case MSG_AF_SET: {
      CDBG("%s: Set AF parameters ", __func__);
      af->af_obj->af_ops.set_parameters(&msg->u.af_set_parm,
        &(af->af_obj->output), af->af_obj->af);
      af->af_cb(& (af->af_obj->output), af->af_port);
    }
      break;

    case MSG_AF_STATS:
    case MSG_BF_STATS: {
      /*AF_OUTPUT_EZ_METADATA help to stop MSG_AF_SET case,when update af info to metadata*/
      CDBG("%s: Process AF stats", __func__);
      ATRACE_BEGIN("Camera:AF");
      af->af_obj->af_ops.process(msg->u.stats, &(af->af_obj->output),
        af->af_obj->af);
      ATRACE_END();
      free(msg->u.stats);
      af->af_cb(& (af->af_obj->output), af->af_port);
    }
      break;

    case MSG_AF_STOP_THREAD: {
      exit_flag = 1;
    }
      break;

    default: {
    }
      break;
    }
    if (msg->sync_flag == TRUE) {
      sem_post(&msg->sync_obj->msg_sem);
      /* Don't free msg, the sender will do */
      msg = NULL;
    }
    if (msg) {
      free(msg);
      msg = NULL;
    }
  } while (!exit_flag);
  return NULL;
} /* af_thread_handler */

/** q3a_thread_af_start
 *    @af_data: The pointer to the af thread data
 *
 *  Called to create the af thread. It will wait on a semaphore until the
 *  thread is created and running.
 *
 *  Return TRUE
 **/
boolean q3a_thread_af_start(q3a_thread_af_data_t *af_data)
{
  pthread_create(&af_data->thread_data->thread_id, NULL,
    af_thread_handler, af_data);
  pthread_setname_np(af_data->thread_data->thread_id, "AF");
  sem_wait(&af_data->thread_data->sem_launch);
  return TRUE;
} /* q3a_thread_af_start */

/** q3a_thread_af_stop
 *    @af_data: The pointer to the af thread data
 *
 *  Called to stop the af thread. It will send a MSG_STOP_THREAD message to
 *  the queue and will wait for the thread to join if the message is enqueued
 *  successfully.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
boolean q3a_thread_af_stop(q3a_thread_af_data_t *af_data)
{
  boolean             rc;
  q3a_thread_af_msg_t *msg;

  msg = malloc(sizeof(q3a_thread_af_msg_t));

  if (msg) {
    CDBG_ERROR("%s:%d MSG_STOP_THREAD", __func__, __LINE__);
    memset(msg, 0, sizeof(q3a_thread_af_msg_t));
    msg->type = MSG_AF_STOP_THREAD;
    rc = q3a_af_thread_en_q_msg(af_data->thread_data, msg);

    if (rc) {
      pthread_join(af_data->thread_data->thread_id, NULL);
      CDBG("%s:%d pthread_join", __func__, __LINE__);
    }
  } else {
    rc = FALSE;
  }
  return rc;
} /* q3a_thread_af_stop */

/** q3a_af_thread_en_q_msg
 *    @af_data: The pointer to the af thread data
 *    @msg:     The message to be put in the queue
 *
 *  Enqueues the sent message into the thread's queue. If the message has the
 *  priority flag set, it will be put in the priority queue. Upon receiving the
 *  MSG_STOP_THREAD type of message, the queue will no longer be active and
 *  no one will be able to enqueue messages in it.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
boolean q3a_af_thread_en_q_msg(void *af_data, q3a_thread_af_msg_t *msg)
{
  q3a_thread_data_t *thread_data = (q3a_thread_data_t *)af_data;
  boolean           rc = FALSE;
  boolean           sync_flag_set = FALSE;
  msg_sync_t        msg_sync;

  if (!msg || !thread_data) {
    CDBG_ERROR("%s: Invalid Parameters!", __func__);
    if(msg) {
      free(msg);
    }
    return FALSE;
  }
  CDBG("%s: Enqueue AF message of type: %d", __func__, msg->type);

  pthread_mutex_lock(&thread_data->msg_q_lock);
  if (thread_data->active) {
    if (msg->sync_flag == TRUE) {
      msg->sync_obj = &msg_sync;
      sem_init(&msg_sync.msg_sem, 0, 0);
      sync_flag_set = TRUE;
    }
    CDBG("%s:%d, lock Q", __func__, __LINE__);
   //If its a priority event queue to priority queue else to normal queue
    if(msg->is_priority) {
      mct_queue_push_tail(thread_data->p_msg_q, msg);
    } else {
      mct_queue_push_tail(thread_data->msg_q, msg);
    }

    if (msg->type == MSG_AF_STOP_THREAD) {
      thread_data->active = 0;
    }
    rc = TRUE;
  }
  pthread_mutex_unlock(&thread_data->msg_q_lock);

  if (rc) {
    pthread_mutex_lock(&thread_data->thread_mutex);
    pthread_cond_signal(&thread_data->thread_cond);
    pthread_mutex_unlock(&thread_data->thread_mutex);

    if (TRUE == sync_flag_set) {
      sem_wait(&msg_sync.msg_sem);
      sem_destroy(&msg_sync.msg_sem);
    }
  } else {
    CDBG_ERROR("%s: Failure adding AF message - handler inactive ", __func__);
    free(msg);
  }

  return rc;
} /* q3a_af_thread_en_q_msg */

/** q3a_thread_af_init
 *
 *  Initializes the AF thread data and creates the queues.
 *
 *  Return the the AF thread data object, on failure return NULL.
 **/
q3a_thread_af_data_t* q3a_thread_af_init(void)
{
  q3a_thread_af_data_t *af;

  CDBG("%s: Allocate memory for AF thread!", __func__);
  af = malloc(sizeof(q3a_thread_af_data_t));
  if (af == NULL) {
    return NULL;
  }
  memset(af, 0, sizeof(q3a_thread_af_data_t));

  CDBG("%s: Allocate memory for q3a thread data", __func__);
  af->thread_data = malloc(sizeof(q3a_thread_data_t));
  if (af->thread_data == NULL) {
    free(af);
    return NULL;
  }
  memset(af->thread_data, 0, sizeof(q3a_thread_data_t));

  CDBG("%s: Create AF queue ", __func__);
  af->thread_data->msg_q = (mct_queue_t *)mct_queue_new;
  af->thread_data->p_msg_q = (mct_queue_t *)mct_queue_new;

  if (!af->thread_data->msg_q || !af->thread_data->p_msg_q) {
    if(af->thread_data->msg_q) {
      mct_queue_free(af->thread_data->msg_q);
    }
    if(af->thread_data->p_msg_q) {
      mct_queue_free(af->thread_data->p_msg_q);
    }
    free(af->thread_data);
    free(af);
    return NULL;
  }

  CDBG("%s: Initialize the AF queue! ", __func__);
  pthread_mutex_init(&af->thread_data->msg_q_lock, NULL);
  mct_queue_init(af->thread_data->msg_q);
  mct_queue_init(af->thread_data->p_msg_q);
  pthread_cond_init(&af->thread_data->thread_cond, NULL);
  pthread_mutex_init(&af->thread_data->thread_mutex, NULL);
  sem_init(&af->thread_data->sem_launch, 0, 0);
  CDBG("%s private->thread_data: %p", __func__, af->thread_data);

  return af;
} /* q3a_thread_af_init */

/** q3a_thread_af_deinit
 *    @af_data: The pointer to the af thread data
 *
 *  Deinitializes the AF thread data - frees the queues, destroys the
 *  sync variables and frees the thread data object.
 *
 *  Return void.
 **/
void q3a_thread_af_deinit(q3a_thread_af_data_t *af)
{
  CDBG("%s thread_data: %p", __func__, af->thread_data);
  pthread_mutex_destroy(&af->thread_data->thread_mutex);
  pthread_cond_destroy(&af->thread_data->thread_cond);
  mct_queue_free(af->thread_data->msg_q);
  mct_queue_free(af->thread_data->p_msg_q);
  pthread_mutex_destroy(&af->thread_data->msg_q_lock);
  sem_destroy(&af->thread_data->sem_launch);

  free(af->thread_data);
  free(af);
} /* q3a_thread_af_deinit */
