/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vpe_thread.h"
#include "vpe_log.h"
#include <poll.h>
#include <unistd.h>
#include <sys/syscall.h>

#define PIPE_FD_IDX   0
#define SUBDEV_FD_IDX 1

void* vpe_thread_func(void* data)
{
  int rc;
  mct_module_t *module = (mct_module_t *) data;
  vpe_module_ctrl_t *ctrl = (vpe_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  PTHREAD_MUTEX_LOCK(&(ctrl->vpe_mutex));
  ctrl->vpe_thread_started = TRUE;
  pthread_cond_signal(&(ctrl->th_start_cond));
  PTHREAD_MUTEX_UNLOCK(&(ctrl->vpe_mutex));

  CDBG_ERROR("%s thread_id is %d\n",__func__, syscall(SYS_gettid));
  if(ctrl->vpehw->subdev_opened == FALSE) {
    CDBG_ERROR("%s:%d, failed, vpe subdev not open", __func__, __LINE__);
    vpe_thread_fatal_exit(ctrl, FALSE);
  }
  /* subscribe for event on subdev fd */
  vpe_hardware_cmd_t cmd;
  cmd.type = VPE_HW_CMD_SUBSCRIBE_EVENT;
  rc = vpe_hardware_process_command(ctrl->vpehw, cmd);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, cannot subscribe to vpe hardware event",
      __func__, __LINE__);
    vpe_thread_fatal_exit(ctrl, FALSE);
  }

  /* poll on the pipe readfd and subdev fd */
  struct pollfd pollfds[2];
  int num_fds = 2;
  int ready=0, i=0;
  pollfds[PIPE_FD_IDX].fd = ctrl->pfd[READ_FD];
  pollfds[PIPE_FD_IDX].events = POLLIN|POLLPRI;
  pollfds[SUBDEV_FD_IDX].fd = ctrl->vpehw->subdev_fd;
  pollfds[SUBDEV_FD_IDX].events = POLLIN|POLLPRI;
  CDBG_HIGH("%s:%d: vpe_thread entering the polling loop...",
    __func__, __LINE__);
  while(1) {
    /* poll on the fds with no timeout */
    ready = poll(pollfds, (nfds_t)num_fds, -1);
    if(ready > 0) {
      /* loop through the fds to see if any event has occured */
      for(i=0; i<num_fds; i++) {
        if(pollfds[i].revents & (POLLIN|POLLPRI)) {
          switch(i) {
          case PIPE_FD_IDX: {
            int num_read=0;
            vpe_thread_msg_t pipe_msg;
            CDBG_LOW("%s:%d, data in pipe", __func__, __LINE__);
            num_read = read(pollfds[i].fd, &(pipe_msg),
                         sizeof(vpe_thread_msg_t));
            if(num_read < 0) {
              CDBG_ERROR("%s:%d, read() failed, rc=%d",
                          __func__, __LINE__, num_read);
              vpe_thread_fatal_exit(ctrl, TRUE);
            } else if(num_read != sizeof(vpe_thread_msg_t)) {
              CDBG_ERROR("%s:%d, error in read(), num_read=%d, msg_size=%d",
                __func__, __LINE__, num_read, sizeof(vpe_thread_msg_t));
              vpe_thread_fatal_exit(ctrl, FALSE);
            }
            rc = vpe_thread_process_pipe_message(ctrl, pipe_msg);
            if (rc < 0) {
              CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
              vpe_thread_fatal_exit(ctrl, FALSE);
            }
            break;
          }
          case SUBDEV_FD_IDX: {
            rc = vpe_thread_process_hardware_event(ctrl);
            if(rc < 0) {
              CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
              vpe_thread_fatal_exit(ctrl, FALSE);
            }
            break;
          }
          default:
            CDBG_ERROR("%s:%d, error, bad fd index", __func__, __LINE__);
            vpe_thread_fatal_exit(ctrl, FALSE);
          } /* switch(i) */
        } /* if */
      } /* for */
    } else if(ready == 0){
      CDBG_ERROR("%s:%d, error: poll() timed out", __func__, __LINE__);
      vpe_thread_fatal_exit(ctrl, FALSE);
    } else {
      CDBG_ERROR("%s:%d, error: poll() failed", __func__, __LINE__);
      vpe_thread_fatal_exit(ctrl, FALSE);
    }
  } /* while(1) */
  return NULL;
}

/* vpe_thread_handle_divert_buf_event:
 *
 *   send a buf divert event to downstream module, if the piggy-backed ACK
 *   is received, we can update the ACK from ack_list, otherwise, the ACK will
 *   be updated when buf_divert_ack event comes from downstream module.
 *
 **/
static int32_t vpe_thread_handle_divert_buf_event(vpe_module_ctrl_t* ctrl,
  vpe_module_event_t* vpe_event)
{
  int rc;
  mct_event_t event;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.identity = vpe_event->u.divert_buf_data.div_identity;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
  event.u.module_event.module_event_data =
    &(vpe_event->u.divert_buf_data.isp_buf_divert);

  vpe_event->u.divert_buf_data.isp_buf_divert.ack_flag = FALSE;

  CDBG("%s:%d, sending unproc_div, identity=0x%x", __func__, __LINE__,
    event.identity);
  rc = vpe_module_send_event_downstream(ctrl->p_module, &event);
  if (rc < 0) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  CDBG("%s:%d, unprocessed divert ack = %d", __func__, __LINE__,
    vpe_event->u.divert_buf_data.isp_buf_divert.ack_flag);

  /* if ack is piggy backed, we can safely send ack to upstream */
  if (vpe_event->u.divert_buf_data.isp_buf_divert.ack_flag == TRUE) {
    CDBG_LOW("%s:%d, doing ack for divert event", __func__, __LINE__);
    vpe_module_do_ack(ctrl, vpe_event->ack_key);
  }
  return 0;
}

/* vpe_thread_handle_process_buf_event:
 *
 * Description:
 *
 *
 **/
static int32_t vpe_thread_handle_process_buf_event(vpe_module_ctrl_t* ctrl,
  vpe_module_event_t* vpe_event)
{
  int rc;
  int in_frame_fd;
  mct_event_t event;
  vpe_hardware_cmd_t cmd;
  vpe_module_ack_key_t* key;

#if 0 //TO have temp preview
  /* Temp: just send the ack for now, to be removed */
  CDBG_LOW("%s:%d, doing ack for process buf event", __func__, __LINE__);
  vpe_module_do_ack(ctrl, vpe_event->ack_key);
  return 0;
#endif
  key = (vpe_module_ack_key_t *) malloc(sizeof(vpe_module_ack_t));
  if(!key) {
    CDBG_ERROR("%s:%d, malloc failed.", __func__, __LINE__);
    return -ENOMEM;
  }
  memcpy(key, &(vpe_event->ack_key), sizeof(vpe_module_ack_key_t));

  if(!ctrl || !vpe_event) {
    CDBG_ERROR("%s:%d, failed, ctrl=%p, vpe_event=%p", __func__, __LINE__,
      ctrl, vpe_event);
    return -EINVAL;
  }

  vpe_hardware_params_t* hw_params;
  hw_params = &(vpe_event->u.process_buf_data.hw_params);
  if (vpe_event->u.process_buf_data.isp_buf_divert.native_buf) {
    in_frame_fd = vpe_event->u.process_buf_data.isp_buf_divert.fd;
  } else {
    in_frame_fd =
      vpe_event->u.process_buf_data.isp_buf_divert.buffer.m.planes[0].m.userptr;
  }
  hw_params->cookie = key;
  hw_params->frame_id =
    vpe_event->u.process_buf_data.isp_buf_divert.buffer.sequence;
  hw_params->timestamp =
    vpe_event->u.process_buf_data.isp_buf_divert.buffer.timestamp;
  hw_params->identity = vpe_event->u.process_buf_data.proc_identity;
  hw_params->buffer_info.fd = in_frame_fd;
  hw_params->buffer_info.index =
    vpe_event->u.process_buf_data.isp_buf_divert.buffer.index;
  hw_params->buffer_info.native_buff =
    vpe_event->u.process_buf_data.isp_buf_divert.native_buf;

  /* before giving the frame to hw, make sure the parameters are good */
  if(FALSE == vpe_hardware_validate_params(hw_params))
  {
    CDBG_ERROR("%s:%d, hw_params invalid, dropping frame.", __func__, __LINE__);
    return vpe_module_do_ack(ctrl, vpe_event->ack_key);
  }
  cmd.type = VPE_HW_CMD_PROCESS_FRAME;
  cmd.u.hw_params = hw_params;
  return vpe_hardware_process_command(ctrl->vpehw, cmd);
}

/* vpe_thread_get_event_from_queue
 *
 * Description:
 * - dq event from the queue based on priority. if there is any event in
 *   realtime queue, return it. Only when there is nothing in realtime queue,
 *   get event from offline queue.
 * - Get hardware related event only if the hardware is ready to process.
 **/
static vpe_module_event_t* vpe_thread_get_event_from_queue(
  vpe_module_ctrl_t *ctrl)
{
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return NULL;
  }
  vpe_module_event_t *vpe_event;
  /* TODO: see if this hardware related logic is suitable in this function
     or need to put it somewhere else */
  if (vpe_hardware_get_status(ctrl->vpehw) == VPE_HW_STATUS_READY) {
    PTHREAD_MUTEX_LOCK(&(ctrl->realtime_queue.mutex));
    if(MCT_QUEUE_IS_EMPTY(ctrl->realtime_queue.q) == FALSE) {
      vpe_event = (vpe_module_event_t *)
                  mct_queue_pop_head(ctrl->realtime_queue.q);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
      return vpe_event;
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
    PTHREAD_MUTEX_LOCK(&(ctrl->offline_queue.mutex));
    if(MCT_QUEUE_IS_EMPTY(ctrl->offline_queue.q) == FALSE) {
      vpe_event = (vpe_module_event_t *)
                  mct_queue_pop_head(ctrl->offline_queue.q);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
      return vpe_event;
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
  } else {
    PTHREAD_MUTEX_LOCK(&(ctrl->realtime_queue.mutex));
    if(MCT_QUEUE_IS_EMPTY(ctrl->realtime_queue.q) == FALSE) {
      vpe_event = (vpe_module_event_t *)
                    mct_queue_look_at_head(ctrl->realtime_queue.q);
      if(!vpe_event) {
          CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
          PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
          return NULL;
      }
      if (vpe_event->hw_process_flag == FALSE) {
        vpe_event = (vpe_module_event_t *)
                      mct_queue_pop_head(ctrl->realtime_queue.q);
        PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
        return vpe_event;
      }
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
    PTHREAD_MUTEX_LOCK(&(ctrl->offline_queue.mutex));
    if(MCT_QUEUE_IS_EMPTY(ctrl->offline_queue.q) == FALSE) {
      vpe_event = (vpe_module_event_t *)
                    mct_queue_look_at_head(ctrl->offline_queue.q);
      if(!vpe_event) {
          CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
          PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
          return NULL;
      }
      if (vpe_event->hw_process_flag == FALSE) {
        vpe_event = (vpe_module_event_t *)
                      mct_queue_pop_head(ctrl->offline_queue.q);
        PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
        return vpe_event;
      }
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
  }
  return NULL;
}

static int32_t vpe_thread_process_queue_event(vpe_module_ctrl_t *ctrl,
  vpe_module_event_t* vpe_event)
{
  int32_t rc = 0;
  if(!ctrl || !vpe_event) {
    CDBG_ERROR("%s:%d, failed, ctrl=%p, vpe_event=%p", __func__, __LINE__,
      ctrl, vpe_event);
    if(vpe_event) free(vpe_event);
    return -EINVAL;
  }
  /* if the event is invalid, no need to process, just free the memory */
  if(vpe_event->invalid == TRUE) {
    CDBG("%s:%d, invalidated event received.", __func__, __LINE__);
    free(vpe_event);
    return 0;
  }
  switch(vpe_event->type) {
  case VPE_MODULE_EVENT_DIVERT_BUF:
    CDBG_LOW("%s:%d, VPE_MODULE_EVENT_DIVERT_BUF", __func__, __LINE__);
    rc = vpe_thread_handle_divert_buf_event(ctrl, vpe_event);
    break;
  case VPE_MODULE_EVENT_PROCESS_BUF:
    CDBG_LOW("%s:%d, VPE_MODULE_EVENT_PROCESS_BUF", __func__, __LINE__);
    rc = vpe_thread_handle_process_buf_event(ctrl, vpe_event);
    break;
  default:
    CDBG_ERROR("%s:%d, failed, bad event type=%d", __func__, __LINE__,
      vpe_event->type);
    free(vpe_event);
    return -EINVAL;
  }
  /* free the event memory */
  free(vpe_event);
  if (rc < 0) {
    CDBG_ERROR("%s:%d, failed, rc=%d", __func__, __LINE__, rc);
  }
  return rc;
}

int32_t vpe_thread_process_pipe_message(vpe_module_ctrl_t *ctrl,
  vpe_thread_msg_t msg)
{
  int rc = 0;
  vpe_hardware_cmd_t cmd;
  switch(msg.type) {
  case VPE_THREAD_MSG_ABORT: {
    CDBG_HIGH("%s:%d, VPE_THREAD_MSG_ABORT: vpe_thread exiting..",
      __func__, __LINE__);
    ctrl->vpe_thread_started = FALSE;
    cmd.type = VPE_HW_CMD_UNSUBSCRIBE_EVENT;
    vpe_hardware_process_command(ctrl->vpehw, cmd);
    pthread_exit(NULL);
  }
  case VPE_THREAD_MSG_NEW_EVENT_IN_Q: {
    CDBG_LOW("%s:%d, VPE_THREAD_MSG_NEW_EVENT_IN_Q:", __func__, __LINE__);
    vpe_module_event_t* vpe_event;
    /* while there is some valid event in queue process it */
    while(1) {
      vpe_event = vpe_thread_get_event_from_queue(ctrl);
      if(!vpe_event) {
        break;
      }
      rc = vpe_thread_process_queue_event(ctrl, vpe_event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, vpe_thread_process_queue_event() failed",
          __func__, __LINE__);
      }
    }
    break;
  }
  default:
    CDBG_ERROR("%s:%d, error: bad msg type=%d",
      __func__, __LINE__, msg.type);
    return -EINVAL;
  }
  return rc;
}

static int32_t vpe_thread_process_hardware_event(vpe_module_ctrl_t *ctrl)
{
  int rc;
  vpe_hardware_cmd_t cmd;
  vpe_hardware_event_data_t event_data;

  /* get the event data from hardware */
  cmd.type = VPE_HW_CMD_NOTIFY_EVENT;
  cmd.u.event_data = &event_data;
  rc = vpe_hardware_process_command(ctrl->vpehw, cmd);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    vpe_thread_fatal_exit(ctrl, FALSE);
  }
  CDBG("%s:%d: vpe frame done, frame_id=%d, buf_idx=%d, identity=0x%x",
    __func__, __LINE__, event_data.frame_id, event_data.buf_idx,
    event_data.identity);

  /* update the pending ack for this buffer */
  vpe_module_ack_key_t key;
  /* Use cookie which has buffer identity now.*/
  if(!event_data.cookie) {
    CDBG_ERROR("%s:%d] failed NULL cookie\n", __func__, __LINE__);
    return -EFAULT;
  }
  key = *(vpe_module_ack_key_t *)event_data.cookie;
  rc = vpe_module_do_ack(ctrl, key);
  free(event_data.cookie);
  if(rc < 0) {
    CDBG_ERROR("%s:%d: failed, buf_idx=%d, identity=0x%x", __func__, __LINE__,
    event_data.buf_idx, event_data.identity);
    return rc;
  }

  /* if there is any pending valid event in queue, process it */
  vpe_module_event_t *vpe_event = vpe_thread_get_event_from_queue(ctrl);
  if (vpe_event) {
    vpe_thread_process_queue_event(ctrl, vpe_event);
  }
  return 0;
}

void vpe_thread_fatal_exit(vpe_module_ctrl_t *ctrl, boolean post_to_bus)
{
  vpe_hardware_cmd_t cmd;
  CDBG_ERROR("%s:%d, fatal error: killing vpe_thread....!", __func__, __LINE__);
  if(post_to_bus) {
    CDBG_ERROR("%s:%d, posting error to MCT BUS!", __func__, __LINE__);
    /* TODO: add code to post error on mct bus */
  }
  cmd.type = VPE_HW_CMD_UNSUBSCRIBE_EVENT;
  vpe_hardware_process_command(ctrl->vpehw, cmd);
  ctrl->vpe_thread_started = FALSE;
  pthread_exit(NULL);
}

int32_t vpe_thread_create(mct_module_t *module)
{
  int rc;
  if(!module) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EINVAL;
  }
  vpe_module_ctrl_t *ctrl = (vpe_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(ctrl->vpe_thread_started == TRUE) {
    CDBG_ERROR("%s:%d, failed, thread already started, "
               "can't create the thread again!", __func__, __LINE__);
    return -EFAULT;
  }
  ctrl->vpe_thread_started = FALSE;
  rc = pthread_create(&(ctrl->vpe_thread), NULL, vpe_thread_func, module);
  pthread_setname_np(ctrl->vpe_thread, "CAM_vpe");
  if(rc < 0) {
    CDBG_ERROR("%s:%d, pthread_create() failed, rc= ", __func__, __LINE__);
    return rc;
  }
  /* wait to confirm if the thread is started */
  PTHREAD_MUTEX_LOCK(&(ctrl->vpe_mutex));
  while(ctrl->vpe_thread_started == FALSE) {
    pthread_cond_wait(&(ctrl->th_start_cond), &(ctrl->vpe_mutex));
  }
  PTHREAD_MUTEX_UNLOCK(&(ctrl->vpe_mutex));
  return 0;
}
