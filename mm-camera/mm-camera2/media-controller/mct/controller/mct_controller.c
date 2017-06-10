/* mct_controller.c
 *
 * This file contains the media controller implementation. All commands coming
 * from the server arrive here first. There is one media controller per session.
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "mct_controller.h"
#include "mct_pipeline.h"
#include "mct_bus.h"
#include "cam_intf.h"
#include "camera_dbg.h"
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <cutils/properties.h>
#include <server_process.h>
#include <server_debug.h>

#if 0
#undef CDBG
#define CDBG CDBG_ERROR
#endif
volatile unsigned int gcam_mct_loglevel = 0;
char mct_prop[PROPERTY_VALUE_MAX];

mct_list_t *mcts = NULL;

static void* mct_controller_thread_run(void *data);
/** get_mct_loglevel:
 *
 *  Args:
 *  Return:
 *    void
 **/

void get_mct_loglevel()
{
  uint32_t temp;
  uint32_t log_level;
  uint32_t debug_mask;
  memset(mct_prop, 0, sizeof(mct_prop));
  /**  Higher 4 bits : Value of Debug log level (Default level is 1 to print all CDBG_HIGH)
       Lower 28 bits : Control mode for sub module logging(Only 1 module in MCT )
   */
  property_get("persist.camera.mct.debug.mask", mct_prop, "268435457"); // 0x10000001=268435457
  temp = atoi(mct_prop);
  log_level = ((temp >> 28) & 0xF);
  debug_mask = (temp & MCT_DEBUG_MASK);
  if (debug_mask > 0)
      gcam_mct_loglevel = log_level;
  else
      gcam_mct_loglevel = 0; // Debug logs are not required if debug_mask is zero
}

/** mct_controller_new:
 *   @mods: modules list
 *   @session_idx: session index
 *   @serv_fd: file descriptor for MCT to communicate
 *             back to imaging server
 *
 *   create a new Media Controller object. This is a
 *   new session pipeline
 *
 * This function executes in SERVER context
 **/
boolean mct_controller_new(mct_list_t *mods,
  unsigned int session_idx, int serv_fd)
{
  mct_controller_t *mct = NULL;
  int               ds_fd;
  pthread_t         tid;

  get_mct_loglevel();

  mct = (mct_controller_t *)malloc(sizeof(mct_controller_t));
  if (!mct)
    goto mct_error;

  mct->pipeline = mct_pipeline_new();
  if (!mct->pipeline)
    goto pipeline_error;

  mct->pipeline->modules = mods;
  mct->pipeline->session = session_idx;

  if (!mct_pipeline_start_session(mct->pipeline))
    goto start_session_error;

  mct->serv_cmd_q = mct_queue_new;
  if (!mct->serv_cmd_q)
    goto servmsgq_error;

  mct_queue_init(mct->serv_cmd_q);

  mct->pipeline->bus = mct_bus_create(session_idx);
  if (!mct->pipeline->bus)
    goto bus_error;

  pthread_mutex_init(&mct->mctl_thread_started_mutex, NULL);
  pthread_cond_init(&mct->mctl_thread_started_cond, NULL);

  mct->serv_cmd_q_counter = 0;

  pthread_mutex_init(&mct->mctl_mutex, NULL);
  pthread_cond_init(&mct->mctl_cond, NULL);

  pthread_mutex_init(&mct->serv_msg_q_lock, NULL);

  mct->serv_fd  = serv_fd;

  pthread_mutex_lock(&mct->mctl_thread_started_mutex);

  if (pthread_create(&tid, NULL, mct_controller_thread_run, mct)) {
    pthread_mutex_unlock(&mct->mctl_thread_started_mutex);
    goto thread_error;
  }

  pthread_cond_wait(&mct->mctl_thread_started_cond,
    &mct->mctl_thread_started_mutex);
  pthread_mutex_unlock(&mct->mctl_thread_started_mutex);

  mct->mct_tid = tid;

  mct->pipeline->bus->mct_mutex = &mct->mctl_mutex;
  mct->pipeline->bus->mct_cond  = &mct->mctl_cond;

  if (!(mcts = mct_list_append(mcts, mct, NULL, NULL)))
    goto all_error;

  return TRUE;

all_error:
  mct_bus_destroy(mct->pipeline->bus);
thread_error:
  pthread_cond_destroy(&mct->mctl_thread_started_cond);
  pthread_mutex_destroy(&mct->mctl_thread_started_mutex);
  pthread_mutex_destroy(&mct->serv_msg_q_lock);
  pthread_cond_destroy(&mct->mctl_cond);
  pthread_mutex_destroy(&mct->mctl_mutex);
bus_error:
  mct_queue_free(mct->serv_cmd_q);
  mct->serv_cmd_q = NULL;

start_session_error:
servmsgq_error:
  mct_pipeline_stop_session(mct->pipeline);
  free(mct->pipeline);
pipeline_error:
  free(mct);
mct_error:
  return FALSE;
}

/** mct_controller_find_session:
 *    @d1: media controller objective
 *    @d2: session index
 *
 * To find a MCT from MCTs list based on session index.
 *
 *  Return TRUE if MCT exists.
 **/
static boolean mct_controller_find_session(void *d1, void *d2)
{
  return ((((mct_controller_t *)d1)->pipeline->session) ==
          *(unsigned int *)d2 ? TRUE : FALSE);
}


/** mct_controller_destroy:
 *    @sessionIdx: the corresponding session index's
 *       MCT to be removed
 *
 **/
boolean mct_controller_destroy(unsigned int session_idx)
{
  mct_serv_msg_t   *msg;
  mct_controller_t *mct;
  mct_list_t       *mct_list;

  mct_list = mct_list_find_custom(mcts, &session_idx,
        mct_controller_find_session);
  if (!mct_list) {
    return FALSE;
  }
  mct = (mct_controller_t *)mct_list->data;

  msg = calloc(1, sizeof(mct_serv_msg_t));
  if (!msg)
    return FALSE;

  msg->msg_type = SERV_MSG_HAL;
  msg->u.hal_msg.id = MSM_CAMERA_DEL_SESSION;

  pthread_mutex_lock(&mct->serv_msg_q_lock);
  mct_queue_push_tail(mct->serv_cmd_q, msg);
  pthread_mutex_unlock(&mct->serv_msg_q_lock);

  pthread_mutex_lock(&mct->mctl_mutex);
  mct->serv_cmd_q_counter++;
  pthread_cond_signal(&mct->mctl_cond);
  pthread_mutex_unlock(&mct->mctl_mutex);

  pthread_join(mct->mct_tid, NULL);

  mct_bus_destroy(mct->pipeline->bus);

  pthread_cond_destroy(&mct->mctl_thread_started_cond);
  pthread_mutex_destroy(&mct->mctl_thread_started_mutex);
  pthread_mutex_destroy(&mct->serv_msg_q_lock);
  if (!MCT_QUEUE_IS_EMPTY(mct->serv_cmd_q)) {
    mct_queue_free(mct->serv_cmd_q);
  }else
    free(mct->serv_cmd_q);
  mct->serv_cmd_q = NULL;

  mct_pipeline_stop_session(mct->pipeline);
  mct_pipeline_destroy(mct->pipeline);
  mcts = mct_list_remove(mcts, mct);
  free(mct);
  mct = NULL;

  return TRUE;
}

/** mct_controller_proc_servmsg:
 *    @servMsg: the message to be posted
 *
 * Post imaging server message to Media Controller's message
 *    message queue.
 *
 * This function executes in Imaging Server context
 **/
boolean mct_controller_proc_serv_msg(mct_serv_msg_t *serv_msg)
{
  mct_controller_t *mct;
  mct_list_t       *mct_list;
  mct_serv_msg_t   *msg;
  unsigned int     session;

  switch (serv_msg->msg_type) {
  case SERV_MSG_DS:
    session = serv_msg->u.ds_msg.session;
    break;

  case SERV_MSG_HAL: {
    struct msm_v4l2_event_data *data =
      (struct msm_v4l2_event_data *)(serv_msg->u.hal_msg.u.data);
    session = data->session_id;
  }
    break;

  default:
    return FALSE;
  }

  mct_list = mct_list_find_custom(mcts, &session,
    mct_controller_find_session);
  if (!mct_list) {
    return FALSE;
  }

  mct = (mct_controller_t *)mct_list->data;

  msg = malloc(sizeof(mct_serv_msg_t));
  if (!msg)
    return FALSE;

  *msg = *serv_msg;

  /* Push message to Media Controller Message Queue
   * and post signal to Media Controller */
  pthread_mutex_lock(&mct->serv_msg_q_lock);
  mct_queue_push_tail(mct->serv_cmd_q, msg);
  pthread_mutex_unlock(&mct->serv_msg_q_lock);

  pthread_mutex_lock(&mct->mctl_mutex);
  mct->serv_cmd_q_counter++;
  pthread_cond_signal(&mct->mctl_cond);
  pthread_mutex_unlock(&mct->mctl_mutex);

  return TRUE;
}

/** mct_controller_check_pipeline
 *    @pipeline: pipeline objective
 *
 * Check the pipeline's vadility
 **/
static boolean mct_controller_check_pipeline(mct_pipeline_t *pipeline)
{
  return ((pipeline->add_stream    &&
           pipeline->remove_stream &&
           pipeline->send_event    &&
           pipeline->set_bus       &&
           pipeline->get_bus) ? TRUE : FALSE);
}

/** mct_controller_proc_servmsg_internal:
 *    @mct: Media Controller Object
 *    @msg: message object from imaging server
 *
 * Media Controller process Imaging Server messages
 * Return: mct_process_ret_t
 *
 * This function executes in Media Controller's thread context
 **/
static mct_process_ret_t mct_controller_proc_serv_msg_internal(
  mct_controller_t *mct, mct_serv_msg_t *msg)
{
  mct_process_ret_t ret;
  mct_pipeline_t    *pipeline;

  memset(&ret, 0x00, sizeof(mct_process_ret_t));
  ret.type = MCT_PROCESS_RET_SERVER_MSG;
  ret.u.serv_msg_ret.error = TRUE;

  if (!mct || !msg || !mct->pipeline) {
    ret.u.serv_msg_ret.error = TRUE;
    return ret;
  }

  ret.u.serv_msg_ret.msg = *msg;
  pipeline = mct->pipeline;

  if (!mct_controller_check_pipeline(pipeline)) {
    ret.u.serv_msg_ret.error = TRUE;
    return ret;
  }

  switch (msg->msg_type) {
  case SERV_MSG_DS: {
    if (msg->u.ds_msg.operation == CAM_MAPPING_TYPE_FD_MAPPING &&
        pipeline->map_buf) {
      CDBG("[dbgHang] - Map buffer >>> enter");
      ret.u.serv_msg_ret.error = pipeline->map_buf(&msg->u.ds_msg, pipeline);
      CDBG("[dbgHang] - Map buffer >>> exit with status: %d", ret.u.serv_msg_ret.error);
    } else if (msg->u.ds_msg.operation == CAM_MAPPING_TYPE_FD_UNMAPPING &&
        pipeline->unmap_buf) {
      CDBG("[dbgHang] - UnMap buffer >>>>> enter");
      ret.u.serv_msg_ret.error = pipeline->unmap_buf(&msg->u.ds_msg, pipeline);
      CDBG("[dbgHang] - UnMap buffer >>>>> exit with status: %d", ret.u.serv_msg_ret.error);
    }
  }
    break;

  case SERV_MSG_HAL:
    if (pipeline->process_serv_msg)
      ret.u.serv_msg_ret.error = pipeline->process_serv_msg(&msg->u.hal_msg,
        pipeline);
    break;

  default:
    break;
  }

  return ret;
}

/** mct_controller_proc_bus_msg_internal:
 *    Media Controller process Bus messages
 *
 *    @mct: Media Controller Object
 *    @msg: message object from bus
 *
 * Return: mct_process_ret_t
 *
 * This function executes in Media Controller's thread context
 **/
static mct_process_ret_t mct_controller_proc_bus_msg_internal(
  mct_controller_t *mct, mct_bus_msg_t *bus_msg)
{
  mct_process_ret_t ret;
  mct_pipeline_t    *pipeline;
  pthread_attr_t attr;

  ret.u.bus_msg_ret.error = TRUE;
  ret.type = MCT_PROCESS_RET_BUS_MSG;

  if (!mct || !bus_msg || !mct->pipeline) {
    return ret;
  }

  if (!mct_controller_check_pipeline(mct->pipeline)) {
    return ret;
  }

  ret.u.bus_msg_ret.error = FALSE;
  ret.u.bus_msg_ret.msg_type = bus_msg->type;
  ret.u.bus_msg_ret.session = bus_msg->sessionid;
  pipeline = mct->pipeline;

  if (bus_msg->type == MCT_BUS_MSG_NOTIFY_KERNEL) {
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&pipeline->thread_data.pid, &attr,
      &mct_controller_dump_data_for_sof_freeze, (void *)bus_msg);
    ret.type = MCT_PROCESS_DUMP_INFO;
    return ret;
  }
  if (bus_msg->type == MCT_BUS_MSG_SEND_HW_ERROR) {
    ret.type = MCT_PROCESS_RET_ERROR_MSG;
    return ret;
  }
  if (bus_msg->type == MCT_BUS_MSG_REPROCESS_STAGE_DONE ||
      bus_msg->type == MCT_BUS_MSG_SEND_EZTUNE_EVT) {
    ret.type = MCT_PROCESS_RET_BUS_MSG;
    return ret;
  }

  if (pipeline->process_bus_msg)
    ret.u.bus_msg_ret.error = pipeline->process_bus_msg(bus_msg, pipeline);

  return ret;
}
/** mct_t_handler:
 *    @data: signal value
 *
 * mct stuck thread
 **/
void mct_t_handler(union sigval val)
{
  CDBG_ERROR("%s:Error MCT stuck during process of events\n",__func__);
  raise(SIGABRT);
  sleep(1);
}

/** mct_controller_thread_run:
 *    @data: structure of mct_controller_t
 *
 * Media Controller Thread
 **/
static void* mct_controller_thread_run(void *data)
{
  mct_controller_t  *mct_this;
  mct_process_ret_t  proc_ret;
  mct_serv_msg_t    *msg;
  mct_bus_msg_t     *bus_msg;

  int      sig, err, tmp_bus_q_cmd = FALSE;
  sigset_t sigs;
  sigset_t old_sig_set;
  int mct_t_ret = 0;
  struct sigevent mct_t_sig;
  timer_t mct_timerid;
  pthread_attr_t mct_t_attr;
  struct itimerspec mct_in, mct_out;

  CDBG_HIGH("%s thread_id is %d\n",__func__, syscall(SYS_gettid));
  prctl(PR_SET_NAME, "mct_controller", 0, 0, 0);
  mct_this = (mct_controller_t *)data;
  mct_this->mct_tid = pthread_self();

  /* signal condition variable */
  pthread_mutex_lock(&mct_this->mctl_thread_started_mutex);
  pthread_cond_signal(&mct_this->mctl_thread_started_cond);
  pthread_mutex_unlock(&mct_this->mctl_thread_started_mutex);

  /* create a timer */
  mct_t_sig.sigev_notify = SIGEV_THREAD;
  mct_t_sig.sigev_notify_function = mct_t_handler;
  mct_t_sig.sigev_value.sival_ptr = NULL;
  pthread_attr_init(&mct_t_attr);
  mct_t_sig.sigev_notify_attributes = &mct_t_attr;

  mct_t_ret = timer_create(CLOCK_REALTIME, &mct_t_sig, &mct_timerid);
  if (!mct_t_ret) {
    mct_in.it_value.tv_sec = 0;
    mct_in.it_value.tv_nsec = 0;
    mct_in.it_interval.tv_sec = mct_in.it_value.tv_sec;
    mct_in.it_interval.tv_nsec = mct_in.it_value.tv_nsec;
    timer_settime(mct_timerid, 0, &mct_in, &mct_out);
  }
  do {
    /*First make sure there aren't any pending events in the queue. This
      is required since if commands come from multiple threads for the
      same session, it may happen the signal was made while this thread
      was busy processing another command, in which case we may
      lose out on this new command*/
    pthread_mutex_lock(&mct_this->serv_msg_q_lock);
    msg = (mct_serv_msg_t *)mct_queue_pop_head(mct_this->serv_cmd_q);
    pthread_mutex_unlock(&mct_this->serv_msg_q_lock);

    if (msg) {
      if (!mct_t_ret) {
        mct_in.it_value.tv_sec = 15; //15 second mct timeout.
        mct_in.it_value.tv_nsec = 0;
        mct_in.it_interval.tv_sec = mct_in.it_value.tv_sec;
        mct_in.it_interval.tv_nsec = mct_in.it_value.tv_nsec;
        timer_settime(mct_timerid, 0, &mct_in, &mct_out);
      }
      pthread_mutex_lock(&mct_this->mctl_mutex);
      mct_this->serv_cmd_q_counter--;
      pthread_mutex_unlock(&mct_this->mctl_mutex);

      proc_ret = mct_controller_proc_serv_msg_internal(mct_this, msg);
      free(msg);

      mct_in.it_value.tv_sec = 0; //stop timer
      mct_in.it_value.tv_nsec = 0;
      mct_in.it_interval.tv_sec = mct_in.it_value.tv_sec;
      mct_in.it_interval.tv_nsec = mct_in.it_value.tv_nsec;
      timer_settime(mct_timerid, 0, &mct_in, &mct_out);

      if (proc_ret.type == MCT_PROCESS_RET_SERVER_MSG           &&
          proc_ret.u.serv_msg_ret.msg.msg_type == SERV_MSG_HAL  &&
          proc_ret.u.serv_msg_ret.msg.u.hal_msg.id == MSM_CAMERA_DEL_SESSION) {
        goto close_mct;
      }
      /* Based on process result, need to send event to server */
      write(mct_this->serv_fd, &proc_ret, sizeof(mct_process_ret_t));

      // Try to pop another message from the queue
      continue;
    }

    pthread_mutex_lock(&mct_this->mctl_mutex);

    tmp_bus_q_cmd = mct_this->pipeline->bus->bus_cmd_q_flag;
    mct_this->pipeline->bus->bus_cmd_q_flag = FALSE;
    if (!tmp_bus_q_cmd && !mct_this->serv_cmd_q_counter) {
      pthread_cond_wait(&mct_this->mctl_cond, &mct_this->mctl_mutex);
    }

    pthread_mutex_unlock(&mct_this->mctl_mutex);

    /* Received Signal from Pipeline Bus */
    while (tmp_bus_q_cmd) {
      pthread_mutex_lock(&mct_this->pipeline->bus->bus_msg_q_lock);
      bus_msg = (mct_bus_msg_t *)
        mct_queue_pop_head(mct_this->pipeline->bus->bus_queue);
      pthread_mutex_unlock(&mct_this->pipeline->bus->bus_msg_q_lock);

      if (!bus_msg) {
        break;
      }
      proc_ret = mct_controller_proc_bus_msg_internal(mct_this, bus_msg);

      if (bus_msg->msg)
        free(bus_msg->msg);

      if (bus_msg)
        free(bus_msg);

      if (proc_ret.type == MCT_PROCESS_RET_ERROR_MSG ||
          proc_ret.type == MCT_PROCESS_DUMP_INFO ||
          (proc_ret.type == MCT_PROCESS_RET_BUS_MSG &&
           proc_ret.u.bus_msg_ret.msg_type == MCT_BUS_MSG_REPROCESS_STAGE_DONE) ||
          (proc_ret.type == MCT_PROCESS_RET_BUS_MSG &&
           proc_ret.u.bus_msg_ret.msg_type == MCT_BUS_MSG_SEND_EZTUNE_EVT)) {

        write(mct_this->serv_fd, &proc_ret, sizeof(mct_process_ret_t));
      }
    }
  } while(1);

close_mct:
  if (!mct_t_ret)
    timer_delete(mct_timerid);
  return NULL;
}
