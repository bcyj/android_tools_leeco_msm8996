/*============================================================================
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <pthread.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>
#include <semaphore.h>
#include "camera_dbg.h"
#include "isp_event.h"
#include "isp_hw_module_ops.h"
#include "isp_hw.h"
#include "isp.h"
#include "isp_log.h"
#include <sys/syscall.h>
#include <sys/prctl.h>
#include "server_debug.h"

#if 0
#undef CDBG
#define CDBG ALOGE
#endif

#undef CDBG_ERROR
#define CDBG_ERROR ALOGE

/** isp_thread_proc_cmd
 *
 * DESCRIPTION: thread to proc cmd sent from pipe,
 *              called by mainloop
 *
 **/
static int isp_thread_proc_cmd(isp_hw_t *isp_hw,
  isp_thread_t *thread_data, int *thread_exit, boolean use_pipe)
{
  ssize_t read_len;
  int rc = 0;
  boolean sem_posted = FALSE;
  boolean wait_for_reset = 0;

  if (use_pipe) {
    read_len = read(thread_data->pipe_fds[0],
      &thread_data->cmd, sizeof(thread_data->cmd));

    if (read_len != sizeof(uint32_t)) {
      /* each cmd takes 4 bytes
         kill the thread and recreate the thread.*/
      CDBG_ERROR("%s: read cmd from pipe error, readlen = %d\n",
        __func__, (int)read_len);
      thread_data->return_code = -EPIPE;
      sem_post(&thread_data->sig_sem);
      *thread_exit = 1;
      return -EPIPE;
    }
  }
  pthread_mutex_lock(&isp_hw->overflow_mutex);
  if (isp_hw->is_overflow) {
    wait_for_reset = 1;
  }
  pthread_mutex_unlock(&isp_hw->overflow_mutex);
  if (wait_for_reset) {
    sem_wait(&isp_hw->reset_done);
  }


  switch (thread_data->cmd) {
  case MM_ISP_CMD_NOTIFY_OPS_INIT: {
    if(!thread_data->init_cmd) {
      CDBG_ERROR("%s: ERROR init_cmd is NULL", __func__);
      thread_data->return_code = -200;
      goto end;
    }
    thread_data->return_code = isp_hw_proc_init((void *)isp_hw,
       thread_data->init_cmd->in_params, thread_data->init_cmd->notify_ops);
  }
    break;

  case MM_ISP_CMD_SET_PARAMS: {
    if(!thread_data->set_param_cmd) {
      CDBG_ERROR("%s: ERROR set_param_cmd is NULL", __func__);
      thread_data->return_code = -200;
      goto end;
    }
    thread_data->return_code =
      isp_hw_proc_set_params((void *)isp_hw,
        thread_data->set_param_cmd->params_id,
        thread_data->set_param_cmd->in_params,
        thread_data->set_param_cmd->in_params_size);
  }
    break;

  case MM_ISP_CMD_GET_PARAMS: {
    if(!thread_data->get_param_cmd) {
      CDBG_ERROR("%s: ERROR get_param_cmd is NULL", __func__);
      thread_data->return_code = -200;
      goto end;
    }
    thread_data->return_code = isp_hw_proc_get_params(
       (void *)isp_hw, thread_data->get_param_cmd->params_id,
       thread_data->get_param_cmd->in_params,
       thread_data->get_param_cmd->in_params_size,
       thread_data->get_param_cmd->out_params,
       thread_data->get_param_cmd->out_params_size);
  }
    break;

  case MM_ISP_CMD_ACTION: {
    if(!thread_data->action_cmd) {
      CDBG_ERROR("%s: ERROR action_cmd is NULL", __func__);
      thread_data->return_code = -200;
      goto end;
    }

    /* take care 3 special case here:
       1. start/stop
       2. start/stop ack
       3. wake up at sof cmd
       4. default proc action cmd*/
    switch(thread_data->action_cmd->action_code) {
    case ISP_HW_ACTION_CODE_WAKE_UP_AT_SOF: {
      CDBG_ERROR("%s: WAKE_UP_AT_SOF, thread_data = %p\n", __func__, thread_data);
      thread_data->wake_up_at_sof = TRUE;
      return 0;
    }
      break;

    case ISP_HW_ACTION_CODE_STREAM_START:
    case ISP_HW_ACTION_CODE_STREAM_STOP: {
      uint32_t action_code = thread_data->action_cmd->action_code;
      start_stop_stream_t *data = thread_data->action_cmd->data;
      start_stop_stream_t param = *data;
      uint32_t data_size = thread_data->action_cmd->data_size;
      thread_data->return_code = 0;
      thread_data->action_cmd = NULL;

      sem_post(&thread_data->sig_sem);
      sem_posted = TRUE;

      CDBG_ERROR("%s: start/stop, thread_data = %p, action_code = %d\n",
        __func__, thread_data, action_code);
      thread_data->async_ret = isp_hw_proc_action((void *)isp_hw, action_code,
        &param, data_size, thread_data->return_code);
    }
      break;

    case ISP_HW_ACTION_CODE_STREAM_START_ACK:
    case ISP_HW_ACTION_CODE_STREAM_STOP_ACK: {
      uint32_t action_code = thread_data->action_cmd->action_code;

      thread_data->return_code =
        isp_hw_proc_action((void *)isp_hw, thread_data->action_cmd->action_code,
        thread_data->action_cmd->data, thread_data->action_cmd->data_size,
        thread_data->async_ret);
      thread_data->action_cmd = NULL;

      sem_post(&thread_data->sig_sem);
      sem_posted = TRUE;

      CDBG_ERROR("%s: start/stop ack done, thread_data = %p, action_code = %d, rc = %d\n",
        __func__, thread_data, action_code, rc);
    }
      break;

    default:
      thread_data->return_code =
        isp_hw_proc_action((void *)isp_hw,
        thread_data->action_cmd->action_code, thread_data->action_cmd->data,
        thread_data->action_cmd->data_size, thread_data->async_ret);
      thread_data->action_cmd = NULL;
      break;
    }
  }
    break;

  case MM_ISP_CMD_SOF_UPDATE: {
    thread_data->return_code = 0;
    thread_data->action_cmd = NULL;
    struct msm_isp_event_data sof_parm;
    isp_session_t *current_session = NULL;
    isp_t *isp = (isp_t*)isp_hw->notify_ops->parent;
    current_session = isp_util_find_session(isp, isp_hw->pipeline.session_id[0]);
    if (current_session == NULL) {
        CDBG_ERROR("%s: No seeison found with session id %d", __func__, isp_hw->pipeline.session_id[0]);
        return -1;
    }
    /*local copy before post caller thread*/
    sof_parm = thread_data->sof_parm;

    /*non blocking call, unblock after confirm receving cmd*/
    sem_post(&thread_data->sig_sem);
    sem_posted = TRUE;
    isp_hw_updating_notify_t notify_data;

    /*Execute trigger update at SOF*/
    thread_data->return_code=
      isp_hw_proc_update_params_at_sof(isp_hw, &sof_parm);

    pthread_mutex_lock(&current_session->state_mutex);
    if (current_session->reg_update_info.reg_update_state !=
        ISP_REG_UPDATE_STATE_CONSUMED) {
         pthread_mutex_unlock(&current_session->state_mutex);
         pthread_mutex_lock(&thread_data->busy_mutex);
         thread_data->thread_busy = FALSE;
         pthread_mutex_unlock(&thread_data->busy_mutex);
        goto end;
    }
    pthread_mutex_unlock(&current_session->state_mutex);

    /* Execute hw update */
    thread_data->return_code = isp_hw_proc_hw_update((void *)isp_hw, &sof_parm);
    if (thread_data->return_code < 0) {
      pthread_mutex_lock(&thread_data->busy_mutex);
      thread_data->thread_busy = FALSE;
      pthread_mutex_unlock(&thread_data->busy_mutex);
      goto end;
    }
    isp_hw_proc_hw_request_reg_update(isp_hw, (void *)current_session);
    pthread_mutex_lock(&current_session->state_mutex);
    current_session->reg_update_info.reg_update_state = ISP_REG_UPDATE_STATE_PENDING;
    pthread_mutex_unlock(&current_session->state_mutex);

    notify_data.session_id = isp_hw->pipeline.session_id[0];
    notify_data.is_hw_updating = 0;
    notify_data.dev_idx = isp_hw->init_params.dev_idx;
    rc = isp_hw->notify_ops->notify((void*)isp_hw->notify_ops->parent,
      isp_hw->init_params.dev_idx, ISP_HW_NOTIFY_HW_UPDATING,
      &notify_data, sizeof(isp_hw_updating_notify_t));

    pthread_mutex_lock(&thread_data->busy_mutex);
    thread_data->thread_busy = FALSE;
    pthread_mutex_unlock(&thread_data->busy_mutex);
  }
    break;

  case MM_ISP_CMD_TIMER:
    break;

  case MM_ISP_CMD_DESTROY:
    thread_data->return_code = isp_hw_proc_destroy(
       (void *)isp_hw);
    break;

  }

  if (thread_data->cmd == MM_ISP_CMD_DESTROY) {
    /* exit the thread */
    ISP_DBG(ISP_MOD_COM,"%s: HW thread exitting now\n", __func__);
    *thread_exit = 1;
  } else if (!sem_posted && thread_data->cmd != MM_ISP_CMD_SOF_UPDATE) {
    /* zero out the pointer. Since it's union we
       just use the action_code pointer.*/
    thread_data->action_cmd = NULL;
  }

end:
  if (!sem_posted) {
    sem_post(&thread_data->sig_sem);
  }

  return 0;
}


/** isp_thread_main_loop
 *
 * DESCRIPTION: main loop to proc cmd or poll v4l2 event
 *
 **/
static void *isp_thread_main_loop(void *data)
{
  int rc = 0, i;
  int timeout;
  int thread_exit = 0;
  isp_thread_t *thread_data = (isp_thread_t *)data;
  isp_hw_t *isp_hw = (isp_hw_t *)thread_data->hw_ptr;

  CDBG_HIGH("%s thread_id is %d\n",__func__, syscall(SYS_gettid));
  prctl(PR_SET_NAME, "isp_main", 0, 0, 0);
  timeout = thread_data->poll_timeoutms;
  /* wake up the creater first */
  sem_post(&thread_data->sig_sem);
  while(!thread_exit) {
    for(i = 0; i < thread_data->num_fds; i++)
      thread_data->poll_fds[i].events = POLLIN|POLLRDNORM|POLLPRI;

    rc = poll(thread_data->poll_fds, thread_data->num_fds, timeout);
    if(rc > 0) {
      if ((thread_data->poll_fds[0].revents & POLLIN) &&
        (thread_data->poll_fds[0].revents & POLLRDNORM)) {
        /* if we have data on pipe, we only process pipe in this iteration */
        rc = isp_thread_proc_cmd(isp_hw, thread_data, &thread_exit, TRUE);
      } else {
        if ((thread_data->poll_fds[1].revents & POLLPRI) ||
          (thread_data->poll_fds[1].revents & POLLIN) ||
          (thread_data->poll_fds[1].revents & POLLRDNORM)) {
          /* if we have data on subdev */
          isp_hw_proc_subdev_event(isp_hw, thread_data);
        }
      }
    }
  }

  if (thread_data->return_code == -EPIPE) {
    isp_hw->notify_ops->notify(isp_hw->notify_ops->parent,
      isp_hw->notify_ops->handle, ISP_HW_NOTIFY_ISP_ERR_HALT_AXI,
      NULL, 0);
  }
  if (thread_data->pipe_fds[0] > 0) {
    close(thread_data->pipe_fds[0]);
    thread_data->pipe_fds[0] = 0;
  }
  if (thread_data->pipe_fds[1] > 0) {
    close(thread_data->pipe_fds[1]);
    thread_data->pipe_fds[1] = 0;
  }

  return NULL;
}

/** isp_sem_thread_main
 *
 * DESCRIPTION: thread to proc cmd
 *
 **/
static void *isp_sem_thread_main(void *data)
{
  int rc = 0;
  int thread_exit = 0;
  isp_thread_t *thread_data = (isp_thread_t *)data;
  isp_hw_t *isp_hw = (isp_hw_t *)thread_data->hw_ptr;

  CDBG_HIGH("%s thread_id is %d\n",__func__, syscall(SYS_gettid));
  prctl(PR_SET_NAME, "isp_sem_thread", 0, 0, 0);
  /* wake up the creater first */
  sem_post(&thread_data->sig_sem);

  while(!thread_exit) {
    sem_wait(&thread_data->thread_wait_sem);
    rc = isp_thread_proc_cmd(isp_hw, thread_data, &thread_exit, FALSE);

    /* TODO: before exit we need to halt axi immediately
     * and disable VFE clock to make VFE be able to restart later */
    if (thread_data->return_code == -EPIPE) {
      isp_hw->notify_ops->notify(isp_hw->notify_ops->parent,
        isp_hw->notify_ops->handle, ISP_HW_NOTIFY_ISP_ERR_HALT_AXI,
        NULL, 0);
    }
  }

  return NULL;
}

/** isp_thread_session_task
 *
 * DESCRIPTION:
 *
 **/
static void* isp_thread_session_task(void *data)
{
  isp_async_task_t *task = (isp_async_task_t *)data;
  isp_t* isp = task->isp;
  isp_session_t *session = task->session;
  task->thread_started = 1;

  CDBG_HIGH("%s thread_id is %d\n",__func__, syscall(SYS_gettid));
  prctl(PR_SET_NAME, "isp_session", 0, 0, 0);
  sem_post(&task->sync_sem);

  for (;;) {
    isp_async_cmd_t *cmd = NULL;
    sem_wait(&task->task_q_sem);

    pthread_mutex_lock(&task->task_q_mutex);
    cmd = mct_queue_pop_head(&task->task_q);
    if(!cmd) {
      /* null node */
      CDBG_ERROR("%s: null cmd. EXIT!!!!\n", __func__);
      pthread_mutex_unlock(&task->task_q_mutex);
      break;
    }
    pthread_mutex_unlock(&task->task_q_mutex);

    if (cmd->cmd_id != ISP_ASYNC_COMMAND_EXIT) {
      (void)isp_proc_async_command(isp, session, cmd);
      free(cmd);
    } else {
      free(cmd); /* free mem */
      break;     /* break out the thread main loop */
    }
  }

  return NULL;
}

/** isp_thread_async_task_start
 *
 * DESCRIPTION:
 *
 **/
int isp_thread_async_task_start(isp_t *isp, isp_session_t *session)
{
  int rc = 0;
  int idx = session->session_idx;
  session->async_task.isp = isp;
  session->async_task.session = session;
  session->async_task.thread_started = 0;

  mct_queue_init(&session->async_task.task_q);
  pthread_mutex_init(&session->async_task.task_q_mutex, NULL);
  pthread_mutex_init(&session->async_task.sync_mutex, NULL);
  sem_init(&session->async_task.sync_sem, 0, 0);
  sem_init(&session->async_task.task_q_sem, 0, 0);
  sem_init(&session->async_task.hw_wait_sem, 0, 0);

  rc = pthread_create(&session->async_task.thread, NULL,
        &isp_thread_session_task, (void *)&session->async_task);
  pthread_setname_np(session->async_task.thread, "CAM_isp_async");
  if(!rc) {
    sem_wait(&session->async_task.sync_sem);
  } else {
    CDBG_ERROR("%s: session task creation failed\n", __func__);
  }

  return rc;
}

/** isp_thread_async_task_stop
 *
 * DESCRIPTION:
 *
 **/
int isp_thread_async_task_stop(isp_t *isp, isp_session_t *session)
{
  if (session->async_task.thread_started) {
    isp_async_cmd_t *cmd;
    isp_async_cmd_t *exit_cmd;

    pthread_mutex_lock(&session->async_task.task_q_mutex);
    /* flush all pending command and then queue the 'exit' command */
    while ((cmd = mct_queue_pop_head(&session->async_task.task_q)))
      free(cmd);

    exit_cmd = malloc(sizeof(isp_async_cmd_t));
    if (exit_cmd) {
      memset(exit_cmd, 0, sizeof(isp_async_cmd_t));
      exit_cmd->cmd_id = ISP_ASYNC_COMMAND_EXIT;
      mct_queue_push_tail(&session->async_task.task_q, (void *)exit_cmd);
    }
    pthread_mutex_unlock(&session->async_task.task_q_mutex);

    sem_post(&session->async_task.task_q_sem);
    pthread_join(session->async_task.thread, NULL);

  }

  sem_destroy(&session->async_task.sync_sem);
  sem_destroy(&session->async_task.task_q_sem);
  sem_destroy(&session->async_task.hw_wait_sem);
  pthread_mutex_destroy(&session->async_task.task_q_mutex);
  pthread_mutex_destroy(&session->async_task.sync_mutex);

  return 0;
}

/** isp_thread_start
 *
 * DESCRIPTION:
 *
 **/
int isp_thread_start(isp_thread_t *thread_data, void *hw_ptr, int poll_fd)
{
  int rc = 0;
  thread_data->hw_ptr = hw_ptr;

  if ((thread_data->pipe_fds[0]) >= MAX_FD_PER_PROCESS) {
    dump_list_of_daemon_fd();
    thread_data->pipe_fds[0] = -1;
    return -1;
  }
  rc = pipe(thread_data->pipe_fds);
  if(rc < 0) {
    CDBG_ERROR("%s: pipe open error = %d\n", __func__, rc);
    return -1;
  }

  thread_data->poll_timeoutms = -1;
  thread_data->poll_fds[0].fd = thread_data->pipe_fds[0];
  thread_data->num_fds = 1;
  thread_data->poll_fd = poll_fd;
  if (poll_fd)
    thread_data->poll_fds[thread_data->num_fds++].fd = poll_fd;

  pthread_mutex_init(&thread_data->cmd_mutex, NULL);
  pthread_mutex_init(&thread_data->busy_mutex, NULL);
  sem_init(&thread_data->sig_sem, 0, 0);

  rc = pthread_create(&thread_data->pid, NULL,
    isp_thread_main_loop, (void *)thread_data);
  pthread_setname_np(thread_data->pid, "CAM_isp_main");
  if(!rc) {
    sem_wait(&thread_data->sig_sem);
  } else {
    CDBG_ERROR("%s: pthread_creat error = %d\n",
      __func__, rc);
    /* setting EPIPE error code triggers exit hw without join thread. */
    thread_data->return_code = -EPIPE;
    rc = thread_data->return_code;
  }

  return rc;
}

/** isp_sem_thread_start
 *
 * DESCRIPTION:
 *
 **/
int isp_sem_thread_start(isp_thread_t *thread_data, void *hw_ptr)
{
    int rc = 0;

    thread_data->hw_ptr = hw_ptr;
    pthread_mutex_init(&thread_data->cmd_mutex, NULL);
    pthread_mutex_init(&thread_data->busy_mutex, NULL);
    sem_init(&thread_data->sig_sem, 0, 0);
    sem_init(&thread_data->thread_wait_sem, 0, 0);

    rc = pthread_create(&thread_data->pid, NULL,
      isp_sem_thread_main, (void *)thread_data);
    pthread_setname_np(thread_data->pid, "CAM_isp_sem");
    if(!rc) {
      sem_wait(&thread_data->sig_sem);
    } else {
      /* setting EPIPE error code triggers exit hw without join thread. */
      CDBG_ERROR("%s: pthread_creat error = %d, thread_data = %p\n",
        __func__, rc, thread_data);
      thread_data->return_code = -EPIPE;
      rc = thread_data->return_code;
    }

    return rc;
}

/** isp_sem_thread_stop
 *
 * DESCRIPTION:
 *
 **/
int isp_sem_thread_stop(isp_thread_t *thread_data)
{

  pthread_mutex_lock(&thread_data->cmd_mutex);
  thread_data->cmd = MM_ISP_CMD_DESTROY;
  sem_post(&thread_data->thread_wait_sem);
  pthread_join(thread_data->pid, NULL);
  pthread_mutex_unlock(&thread_data->cmd_mutex);
  sem_destroy(&thread_data->sig_sem);
  sem_destroy(&thread_data->thread_wait_sem);
  pthread_mutex_destroy(&thread_data->cmd_mutex);
  pthread_mutex_destroy(&thread_data->busy_mutex);

  return 0;
}
