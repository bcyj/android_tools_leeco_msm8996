/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "eztune_diagnostics.h"
#include "cpp_thread.h"
#include "cpp_log.h"
#include <poll.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/prctl.h>

#define PIPE_FD_IDX   0
#define SUBDEV_FD_IDX 1

/* forward declare tuning APIs */
extern boolean mct_tunig_check_status(void);

/** cpp_thread_func:
 *
 * Description:
 *   Entry point for cpp_thread. Polls over pipe read fd and cpp
 *   hw subdev fd. If there is any new pipe message or hardware
 *   event, it is processed.
 **/
void* cpp_thread_func(void* data)
{
  int rc;
  mct_module_t *module = (mct_module_t *) data;
  cpp_module_ctrl_t *ctrl = (cpp_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));
  ctrl->cpp_thread_started = TRUE;
  pthread_cond_signal(&(ctrl->th_start_cond));
  PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));

  prctl(PR_SET_NAME, "cpp_thread", 0, 0, 0);

  if(ctrl->cpphw->subdev_opened == FALSE) {
    CDBG_ERROR("%s:%d, failed, cpp subdev not open", __func__, __LINE__);
    cpp_thread_fatal_exit(ctrl, FALSE);
  }
  /* subscribe for event on subdev fd */
  cpp_hardware_cmd_t cmd;
  cmd.type = CPP_HW_CMD_SUBSCRIBE_EVENT;
  rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, cannot subscribe to cpp hardware event",
      __func__, __LINE__);
    cpp_thread_fatal_exit(ctrl, FALSE);
  }

  /* poll on the pipe readfd and subdev fd */
  struct pollfd pollfds[2];
  int num_fds = 2;
  int ready=0, i=0;
  pollfds[PIPE_FD_IDX].fd = ctrl->pfd[READ_FD];
  pollfds[PIPE_FD_IDX].events = POLLIN|POLLPRI;
  pollfds[SUBDEV_FD_IDX].fd = ctrl->cpphw->subdev_fd;
  pollfds[SUBDEV_FD_IDX].events = POLLIN|POLLPRI;
  CDBG_HIGH("%s:%d: cpp_thread entering the polling loop...thread_id is %d\n",
    __func__, __LINE__,syscall(SYS_gettid));
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
            cpp_thread_msg_t pipe_msg;
            num_read = read(pollfds[i].fd, &(pipe_msg),
                         sizeof(cpp_thread_msg_t));
            if(num_read < 0) {
              CDBG_ERROR("%s:%d, read() failed, rc=%d",
                          __func__, __LINE__, num_read);
              cpp_thread_fatal_exit(ctrl, TRUE);
            } else if(num_read != sizeof(cpp_thread_msg_t)) {
              CDBG_ERROR("%s:%d, failed, in read(), num_read=%d, msg_size=%d",
                __func__, __LINE__, num_read, sizeof(cpp_thread_msg_t));
              cpp_thread_fatal_exit(ctrl, FALSE);
            }
            rc = cpp_thread_process_pipe_message(ctrl, pipe_msg);
            if (rc < 0) {
              CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
              cpp_thread_fatal_exit(ctrl, FALSE);
            }
            break;
          }
          case SUBDEV_FD_IDX: {
            rc = cpp_thread_process_hardware_event(ctrl);
            if(rc < 0) {
              CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
              cpp_thread_fatal_exit(ctrl, FALSE);
            }
            break;
          }
          default:
            CDBG_ERROR("%s:%d, error, bad fd index", __func__, __LINE__);
            cpp_thread_fatal_exit(ctrl, FALSE);
          } /* switch(i) */
        } /* if */
      } /* for */
    } else if(ready == 0){
      CDBG_ERROR("%s:%d, error: poll() timed out", __func__, __LINE__);
      cpp_thread_fatal_exit(ctrl, FALSE);
    } else {
      CDBG_ERROR("%s:%d, error: poll() failed", __func__, __LINE__);
      cpp_thread_fatal_exit(ctrl, FALSE);
    }
  } /* while(1) */
  return NULL;
}

/* cpp_thread_handle_divert_buf_event:
 *
 *   send a buf divert event to downstream module, if the piggy-backed ACK
 *   is received, we can update the ACK from ack_list, otherwise, the ACK will
 *   be updated when buf_divert_ack event comes from downstream module.
 *
 **/
static int32_t cpp_thread_handle_divert_buf_event(cpp_module_ctrl_t* ctrl,
  cpp_module_event_t* cpp_event)
{
  int rc;
  mct_event_t event;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.identity = cpp_event->u.divert_buf_data.div_identity;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
  event.u.module_event.module_event_data =
    &(cpp_event->u.divert_buf_data.isp_buf_divert);

  cpp_event->u.divert_buf_data.isp_buf_divert.ack_flag = FALSE;

  CDBG("%s:%d, sending unproc_div, identity=0x%x", __func__, __LINE__,
    event.identity);
  rc = cpp_module_send_event_downstream(ctrl->p_module, &event);
  if (rc < 0) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  CDBG("%s:%d, unprocessed divert ack = %d", __func__, __LINE__,
    cpp_event->u.divert_buf_data.isp_buf_divert.ack_flag);

  /* if ack is piggy backed, we can safely send ack to upstream */
  if (cpp_event->u.divert_buf_data.isp_buf_divert.ack_flag == TRUE) {
    CDBG_LOW("%s:%d, doing ack for divert event", __func__, __LINE__);
    cpp_module_do_ack(ctrl, cpp_event->ack_key);
  }
  return 0;
}

/* cpp_thread_handle_process_buf_event:
 *
 * Description:
 *
 *
 **/
static int32_t cpp_thread_handle_process_buf_event(cpp_module_ctrl_t* ctrl,
  cpp_module_event_t* cpp_event)
{
  int rc;
  int in_frame_fd;
  mct_event_t event;
  cpp_hardware_cmd_t cmd;
  cpp_module_hw_cookie_t *cookie;

  if(!ctrl || !cpp_event) {
    CDBG_ERROR("%s:%d, failed, ctrl=%p, cpp_event=%p", __func__, __LINE__,
      ctrl, cpp_event);
    return -EINVAL;
  }

  /* cookie is used to attach data to kernel frame, which is to be retrieved
     once processing is done */
  cookie = (cpp_module_hw_cookie_t *) malloc(sizeof(cpp_module_hw_cookie_t));
  if(!cookie) {
    CDBG_ERROR("%s:%d, malloc failed", __func__, __LINE__);
    return -ENOMEM;
  }
  cookie->key = cpp_event->ack_key;
  cookie->proc_div_required = cpp_event->u.process_buf_data.proc_div_required;
  cookie->proc_div_identity = cpp_event->u.process_buf_data.proc_div_identity;
  cookie->meta_data = cpp_event->u.process_buf_data.isp_buf_divert.meta_data;

  cpp_hardware_params_t* hw_params;
  hw_params = &(cpp_event->u.process_buf_data.hw_params);
  hw_params->cookie = cookie;
  if (cpp_event->u.process_buf_data.isp_buf_divert.native_buf) {
    in_frame_fd = cpp_event->u.process_buf_data.isp_buf_divert.fd;
  } else {
    in_frame_fd =
      cpp_event->u.process_buf_data.isp_buf_divert.buffer.m.planes[0].m.userptr;
  }
  hw_params->frame_id =
    cpp_event->u.process_buf_data.isp_buf_divert.buffer.sequence;
  hw_params->timestamp =
    cpp_event->u.process_buf_data.isp_buf_divert.buffer.timestamp;
  hw_params->identity = cpp_event->u.process_buf_data.proc_identity;
  hw_params->buffer_info.fd = in_frame_fd;
  hw_params->buffer_info.index =
    cpp_event->u.process_buf_data.isp_buf_divert.buffer.index;
  hw_params->buffer_info.native_buff =
    cpp_event->u.process_buf_data.isp_buf_divert.native_buf;
  if (hw_params->buffer_info.native_buff) {
    hw_params->buffer_info.identity = hw_params->identity;
  } else {
    hw_params->buffer_info.identity =
        cpp_event->u.process_buf_data.isp_buf_divert.identity;
  }
  hw_params->uv_upsample_enable =
    cpp_event->u.process_buf_data.isp_buf_divert.is_uv_subsampled;
  hw_params->processed_divert = cpp_event->u.process_buf_data.proc_div_required;

  /*Before validation, swap dimensions if 90 or 270 degrees rotation*/
  cpp_hardware_rotation_swap(hw_params);

  /* before giving the frame to hw, make sure the parameters are good */
  if(FALSE == cpp_hardware_validate_params(hw_params))
  {
    free(cookie);
    CDBG_ERROR("%s:%d, hw_params invalid, dropping frame.", __func__, __LINE__);
    return cpp_module_do_ack(ctrl, cpp_event->ack_key);
  }
  cmd.type = CPP_HW_CMD_PROCESS_FRAME;
  cmd.u.hw_params = hw_params;
  rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
  if (rc < 0) {
    free(cookie);
    if (rc == -EAGAIN) {
      CDBG("%s:%d, dropped frame id=%d identity=0x%x\n",
        __func__, __LINE__, hw_params->frame_id, hw_params->identity);
      return cpp_module_do_ack(ctrl, cpp_event->ack_key);
    }
  }

  /* Update and post the current session's diag parameters */
  cpp_module_util_update_session_diag_params(ctrl->p_module, hw_params);
  return rc;
}

/* cpp_thread_get_event_from_queue
 *
 * Description:
 * - dq event from the queue based on priority. if there is any event in
 *   realtime queue, return it. Only when there is nothing in realtime queue,
 *   get event from offline queue.
 * - Get hardware related event only if the hardware is ready to process.
 **/
static cpp_module_event_t* cpp_thread_get_event_from_queue(
  cpp_module_ctrl_t *ctrl)
{
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return NULL;
  }
  cpp_module_event_t *cpp_event;
  /* TODO: see if this hardware related logic is suitable in this function
     or need to put it somewhere else */
  if (cpp_hardware_get_status(ctrl->cpphw) == CPP_HW_STATUS_READY) {
    PTHREAD_MUTEX_LOCK(&(ctrl->realtime_queue.mutex));
    if(MCT_QUEUE_IS_EMPTY(ctrl->realtime_queue.q) == FALSE) {
      cpp_event = (cpp_module_event_t *)
                  mct_queue_pop_head(ctrl->realtime_queue.q);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
      return cpp_event;
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
    PTHREAD_MUTEX_LOCK(&(ctrl->offline_queue.mutex));
    if(MCT_QUEUE_IS_EMPTY(ctrl->offline_queue.q) == FALSE) {
      cpp_event = (cpp_module_event_t *)
                  mct_queue_pop_head(ctrl->offline_queue.q);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
      return cpp_event;
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
  } else {
    PTHREAD_MUTEX_LOCK(&(ctrl->realtime_queue.mutex));
    if(MCT_QUEUE_IS_EMPTY(ctrl->realtime_queue.q) == FALSE) {
      cpp_event = (cpp_module_event_t *)
                    mct_queue_look_at_head(ctrl->realtime_queue.q);
      if(!cpp_event) {
          CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
          PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
          return NULL;
      }
      if (cpp_event->hw_process_flag == FALSE) {
        cpp_event = (cpp_module_event_t *)
                      mct_queue_pop_head(ctrl->realtime_queue.q);
        PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
        return cpp_event;
      }
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
    PTHREAD_MUTEX_LOCK(&(ctrl->offline_queue.mutex));
    if(MCT_QUEUE_IS_EMPTY(ctrl->offline_queue.q) == FALSE) {
      cpp_event = (cpp_module_event_t *)
                    mct_queue_look_at_head(ctrl->offline_queue.q);
      if(!cpp_event) {
          CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
          PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
          return NULL;
      }
      if (cpp_event->hw_process_flag == FALSE) {
        cpp_event = (cpp_module_event_t *)
                      mct_queue_pop_head(ctrl->offline_queue.q);
        PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
        return cpp_event;
      }
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
  }
  return NULL;
}

/* cpp_thread_process_queue_event:
 *
 * Description:
 *
 **/
static int32_t cpp_thread_process_queue_event(cpp_module_ctrl_t *ctrl,
  cpp_module_event_t* cpp_event)
{
  int32_t rc = 0;
  if(!ctrl || !cpp_event) {
    CDBG_ERROR("%s:%d, failed, ctrl=%p, cpp_event=%p", __func__, __LINE__,
      ctrl, cpp_event);
    if(cpp_event) free(cpp_event);
    return -EINVAL;
  }
  /* if the event is invalid, no need to process, just free the memory */
  if(cpp_event->invalid == TRUE) {
    CDBG("%s:%d, invalidated event received.", __func__, __LINE__);
    free(cpp_event);
    return 0;
  }
  switch(cpp_event->type) {
  case CPP_MODULE_EVENT_DIVERT_BUF:
    CDBG_LOW("%s:%d, CPP_MODULE_EVENT_DIVERT_BUF", __func__, __LINE__);
    rc = cpp_thread_handle_divert_buf_event(ctrl, cpp_event);
    break;
  case CPP_MODULE_EVENT_PROCESS_BUF:
    CDBG_LOW("%s:%d, CPP_MODULE_EVENT_PROCESS_BUF", __func__, __LINE__);
    rc = cpp_thread_handle_process_buf_event(ctrl, cpp_event);
    break;
  default:
    CDBG_ERROR("%s:%d, failed, bad event type=%d", __func__, __LINE__,
      cpp_event->type);
    free(cpp_event);
    return -EINVAL;
  }
  /* free the event memory */
  free(cpp_event);
  if (rc < 0) {
    CDBG_ERROR("%s:%d, failed, rc=%d", __func__, __LINE__, rc);
  }
  return rc;
}

/* cpp_thread_process_pipe_message:
 *
 * Description:
 *
 **/
int32_t cpp_thread_process_pipe_message(cpp_module_ctrl_t *ctrl,
  cpp_thread_msg_t msg)
{
  int rc = 0;
  cpp_hardware_cmd_t cmd;
  switch(msg.type) {
  case CPP_THREAD_MSG_ABORT: {
    CDBG_HIGH("%s:%d, CPP_THREAD_MSG_ABORT: cpp_thread exiting..",
      __func__, __LINE__);
    ctrl->cpp_thread_started = FALSE;
    cmd.type = CPP_HW_CMD_UNSUBSCRIBE_EVENT;
    cpp_hardware_process_command(ctrl->cpphw, cmd);
    pthread_exit(NULL);
  }
  case CPP_THREAD_MSG_NEW_EVENT_IN_Q: {
    CDBG_LOW("%s:%d, CPP_THREAD_MSG_NEW_EVENT_IN_Q:", __func__, __LINE__);
    cpp_module_event_t* cpp_event;
    /* while there is some valid event in queue process it */
    while(1) {
      cpp_event = cpp_thread_get_event_from_queue(ctrl);
      if(!cpp_event) {
        break;
      }
      rc = cpp_thread_process_queue_event(ctrl, cpp_event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, cpp_thread_process_queue_event() failed",
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

/* cpp_thread_send_processed_divert:
 *
 * Description:
 *
 **/
static int32_t cpp_thread_send_processed_divert(cpp_module_ctrl_t *ctrl,
  isp_buf_divert_t *buf_divert, uint32_t event_identity)
{
  if(!ctrl || !buf_divert) {
    CDBG_ERROR("%s:%d, failed ctrl:%p, buf_divert:%p\n", __func__, __LINE__,
      ctrl, buf_divert);
    return -EINVAL;
  }
  mct_event_t event;
  int rc;
  memset(&event, 0x00, sizeof(mct_event_t));
  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = event_identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
  event.u.module_event.module_event_data = (void *)buf_divert;
  rc = cpp_module_send_event_downstream(ctrl->p_module, &event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
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

/* cpp_thread_process_hardware_event:
 *
 * Description:
 *
 **/
static int32_t cpp_thread_process_hardware_event(cpp_module_ctrl_t *ctrl)
{
  int rc;
  cpp_hardware_cmd_t cmd;
  cpp_hardware_event_data_t event_data;
  cpp_module_event_t *cpp_event;
  boolean is_eztune_running = FALSE;

  /* get the event data from hardware */
  cmd.type = CPP_HW_CMD_NOTIFY_EVENT;
  cmd.u.event_data = &event_data;
  rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    cpp_thread_fatal_exit(ctrl, FALSE);
  }
  CDBG("%s:%d: cpp frame done, frame_id=%d, buf_idx=%d, identity=0x%x",
    __func__, __LINE__, event_data.frame_id, event_data.buf_idx,
    event_data.identity);

  /* update the pending ack for this buffer */
  cpp_module_ack_key_t key;
  cpp_module_hw_cookie_t *cookie;
  /* Use cookie which has buffer identity now.*/
  if(!event_data.cookie) {
    CDBG_ERROR("%s:%d, failed. cookie=NULL\n", __func__, __LINE__);
    return -EFAULT;
  }
  cookie = (cpp_module_hw_cookie_t *)event_data.cookie;
  CDBG_LOW("%s:%d, proc_div_req=%d, proc_div_identity=0x%x", __func__, __LINE__,
    cookie->proc_div_required, cookie->proc_div_identity);
  rc = cpp_module_do_ack(ctrl, cookie->key);
  if(rc < 0) {
    CDBG_ERROR("%s:%d: failed, buf_idx=%d, identity=0x%x", __func__, __LINE__,
    event_data.buf_idx, event_data.identity);
    return rc;
  }

  is_eztune_running = mct_tunig_check_status();
  /* If processed divert is enabled send the processed buffer downstream */
  if (cookie->proc_div_required ||
        TRUE == is_eztune_running) {
    isp_buf_divert_t buf_divert;
    struct v4l2_plane plane;
    memset(&buf_divert, 0, sizeof(buf_divert));
    memset(&plane, 0, sizeof(plane));
    /* Fill the paramters for buffer divert structure for downstream module */
    buf_divert.buffer.sequence = event_data.frame_id;
    buf_divert.buffer.index = event_data.out_buf_idx;
    buf_divert.buffer.timestamp = event_data.timestamp;
    /* TODO: Need to consider multiplanar when sending downstream. */
    buf_divert.buffer.m.planes = &plane;
    buf_divert.buffer.m.planes[0].m.userptr = event_data.out_fd;
    buf_divert.fd = event_data.out_fd;
    buf_divert.pass_through = 0;
    buf_divert.native_buf = 0;
    buf_divert.identity = event_data.identity;

    if (cookie->proc_div_required) {
      rc = cpp_thread_send_processed_divert(ctrl, &buf_divert,
        cookie->proc_div_identity);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed processed divert\n", __func__, __LINE__);
      }

      /* Release this processed divert buffer in kernel if downstream module is
         giving piggy-back ack */
      if (buf_divert.ack_flag == 1) {
        event_data.is_buf_dirty = buf_divert.is_buf_dirty;
        cmd.type = CPP_HW_CMD_QUEUE_BUF;
        cmd.u.event_data = &event_data;
        rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
        if(rc < 0) {
          CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
          free(cookie);
          cpp_thread_fatal_exit(ctrl, FALSE);
        }
      }
    }

    if (TRUE == is_eztune_running) {
      mct_module_t *module = ctrl->p_module;
      mct_stream_t *stream = cpp_module_util_find_parent(event_data.identity, module);

      if (stream != NULL) {
        if (stream->streaminfo.stream_type == CAM_STREAM_TYPE_PREVIEW) {
          mct_tuning_notify_preview_frame(&buf_divert, stream);
        }
      }
    }
  }

  free(cookie);
  PTHREAD_MUTEX_LOCK(&(ctrl->cpphw->mutex));
  cpp_hardware_stream_status_t *stream_status =
    cpp_hardware_get_stream_status(ctrl->cpphw, event_data.identity);
  if (!stream_status) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    PTHREAD_MUTEX_UNLOCK(&(ctrl->cpphw->mutex));
    return -EFAULT;
  }
  stream_status->pending_buf--;
  /* send signal to thread which is waiting on stream_off
     for pending buffer to be zero */
  if (stream_status->stream_off_pending == TRUE &&
    stream_status->pending_buf == 0) {
    CDBG("%s:%d, info: sending broadcast for pending stream-off",
      __func__, __LINE__);
    pthread_cond_broadcast(&(ctrl->cpphw->no_pending_cond));
  }
  PTHREAD_MUTEX_UNLOCK(&(ctrl->cpphw->mutex));

  /* if there is any pending valid event in queue, process it */
  while(1) {
    cpp_event = cpp_thread_get_event_from_queue(ctrl);
    if(!cpp_event) {
      break;
    }
    rc = cpp_thread_process_queue_event(ctrl, cpp_event);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, cpp_thread_process_queue_event() failed",
        __func__, __LINE__);
    }
  }

  return 0;
}

/* cpp_thread_fatal_exit:
 *
 * Description:
 *
 **/
void cpp_thread_fatal_exit(cpp_module_ctrl_t *ctrl, boolean post_to_bus)
{
  cpp_hardware_cmd_t cmd;
  CDBG_ERROR("%s:%d, fatal error: killing cpp_thread....!", __func__, __LINE__);
  if(post_to_bus) {
    CDBG_ERROR("%s:%d, posting error to MCT BUS!", __func__, __LINE__);
    /* TODO: add code to post error on mct bus */
  }
  cmd.type = CPP_HW_CMD_UNSUBSCRIBE_EVENT;
  cpp_hardware_process_command(ctrl->cpphw, cmd);
  ctrl->cpp_thread_started = FALSE;
  pthread_exit(NULL);
}

/* cpp_thread_create:
 *
 * Description:
 *
 **/
int32_t cpp_thread_create(mct_module_t *module)
{
  int rc;
  if(!module) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EINVAL;
  }
  cpp_module_ctrl_t *ctrl = (cpp_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(ctrl->cpp_thread_started == TRUE) {
    CDBG_ERROR("%s:%d, failed, thread already started, "
               "can't create the thread again!", __func__, __LINE__);
    return -EFAULT;
  }
  ctrl->cpp_thread_started = FALSE;
  rc = pthread_create(&(ctrl->cpp_thread), NULL, cpp_thread_func, module);
  pthread_setname_np(ctrl->cpp_thread, "CAM_cpp");
  if(rc < 0) {
    CDBG_ERROR("%s:%d, pthread_create() failed, rc= ", __func__, __LINE__);
    return rc;
  }
  /* wait to confirm if the thread is started */
  PTHREAD_MUTEX_LOCK(&(ctrl->cpp_mutex));
  while(ctrl->cpp_thread_started == FALSE) {
    pthread_cond_wait(&(ctrl->th_start_cond), &(ctrl->cpp_mutex));
  }
  PTHREAD_MUTEX_UNLOCK(&(ctrl->cpp_mutex));
  return 0;
}
