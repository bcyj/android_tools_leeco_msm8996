/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "vpe_module.h"
#include "vpe_port.h"
#include "vpe_log.h"
#include "server_debug.h"

#define VPE_NUM_SINK_PORTS    2
#define VPE_NUM_SOURCE_PORTS  2

/** vpe_module_init:
 *  Args:
 *    @name: module name
 *  Return:
 *    - mct_module_t pointer corresponding to vpe on SUCCESS
 *    - NULL in case of FAILURE or if VPE hardware does not
 *      exist
 **/
mct_module_t *vpe_module_init(const char *name)
{
  mct_module_t *module;
  vpe_module_ctrl_t* ctrl;
  CDBG_ERROR("%s:%d name=%s", __func__, __LINE__, name);
  module = mct_module_create(name);
  if(!module) {
    CDBG_ERROR("%s:%d failed.", __func__, __LINE__);
    return NULL;
  }
  ctrl = vpe_module_create_vpe_ctrl();
  if(!ctrl) {
    CDBG_ERROR("%s:%d failed", __func__, __LINE__);
    goto error_cleanup_module;
  }
  MCT_OBJECT_PRIVATE(module) = ctrl;
  ctrl->p_module = module;
  module->set_mod = vpe_module_set_mod;
  module->query_mod = vpe_module_query_mod;
  module->start_session = vpe_module_start_session;
  module->stop_session = vpe_module_stop_session;

  mct_port_t* port;
  int i;
  /* Create default ports */
  for(i=0; i < VPE_NUM_SOURCE_PORTS; i++) {
    port = vpe_port_create("vpe-src", MCT_PORT_SRC);
    if(!port) {
      CDBG_ERROR("%s:%d failed.", __func__, __LINE__);
      return NULL;
    }
    module->srcports = mct_list_append(module->srcports, port, NULL, NULL);
    MCT_PORT_PARENT(port) = mct_list_append(MCT_PORT_PARENT(port), module,
                              NULL, NULL);
  }
  for(i=0; i < VPE_NUM_SINK_PORTS; i++) {
    port = vpe_port_create("vpe-sink", MCT_PORT_SINK);
    if(!port) {
      CDBG_ERROR("%s:%d failed.", __func__, __LINE__);
      return NULL;
    }
    module->sinkports = mct_list_append(module->sinkports, port, NULL, NULL);
    MCT_PORT_PARENT(port) = mct_list_append(MCT_PORT_PARENT(port), module,
                              NULL, NULL);
  }
  CDBG_HIGH("%s:%d: info: VPE module_init successful", __func__, __LINE__);
  return module;

error_cleanup_module:
  mct_module_destroy(module);
  return NULL;
}

/** vpe_module_deinit:
 *
 *  Args:
 *    @module: pointer to vpe mct module
 *  Return:
 *    void
 **/
void vpe_module_deinit(mct_module_t *module)
{
  vpe_module_ctrl_t *ctrl;
  ctrl = MCT_OBJECT_PRIVATE(module);
  vpe_module_destroy_vpe_ctrl(ctrl);
  /* TODO: free other dynamically allocated resources in module */
  mct_module_destroy(module);
}

static vpe_module_ctrl_t* vpe_module_create_vpe_ctrl(void)
{
  vpe_module_ctrl_t *ctrl = NULL;
  mct_queue_t *q;
  int rc;
  ctrl = (vpe_module_ctrl_t *) malloc(sizeof(vpe_module_ctrl_t));
  if(!ctrl) {
    CDBG_ERROR("%s:%d, malloc failed", __func__, __LINE__);
    return NULL;
  }
  memset(ctrl, 0x00, sizeof(vpe_module_ctrl_t));

  /* create real-time queue */
  ctrl->realtime_queue.q = (mct_queue_t*) malloc(sizeof(mct_queue_t));
  if(!ctrl->realtime_queue.q) {
    CDBG_ERROR("%s:%d, malloc failed", __func__, __LINE__);
    goto error_queue;
  }
  memset(ctrl->realtime_queue.q, 0x00, sizeof(mct_queue_t));
  mct_queue_init(ctrl->realtime_queue.q);
  pthread_mutex_init(&(ctrl->realtime_queue.mutex), NULL);

  /* create offline queue*/
  ctrl->offline_queue.q = (mct_queue_t*) malloc(sizeof(mct_queue_t));
  if(!ctrl->offline_queue.q) {
    CDBG_ERROR("%s:%d, malloc failed", __func__, __LINE__);
    goto error_queue;
  }
  memset(ctrl->offline_queue.q, 0x00, sizeof(mct_queue_t));
  mct_queue_init(ctrl->offline_queue.q);
  pthread_mutex_init(&(ctrl->offline_queue.mutex), NULL);

  /* create ack list */
  ctrl->ack_list.list = NULL;
  ctrl->ack_list.size = 0;
  pthread_mutex_init(&(ctrl->ack_list.mutex), NULL);

  /* Create PIPE for communication with vpe_thread */
  rc = pipe(ctrl->pfd);
  if ((ctrl->pfd[0]) >= MAX_FD_PER_PROCESS) {
    dump_list_of_daemon_fd();
    ctrl->pfd[0] = -1;
    goto error_pipe;
  }
  if(rc < 0) {
    CDBG_ERROR("%s:%d, pipe() failed", __func__, __LINE__);
    goto error_pipe;
  }
  pthread_cond_init(&(ctrl->th_start_cond), NULL);
  ctrl->session_count = 0;

  /* Create the VPE hardware instance */
  ctrl->vpehw = vpe_hardware_create();
  if(ctrl->vpehw == NULL) {
    CDBG_ERROR("%s:%d, failed, cannnot create vpe hardware instance\n",
      __func__, __LINE__);
    goto error_hw;
  }

  return ctrl;

error_hw:
  close(ctrl->pfd[READ_FD]);
  close(ctrl->pfd[WRITE_FD]);
error_pipe:
  free(ctrl->realtime_queue.q);
  free(ctrl->offline_queue.q);
error_queue:
  free(ctrl);
  return NULL;
}

static int32_t vpe_module_destroy_vpe_ctrl(vpe_module_ctrl_t *ctrl)
{
  if(!ctrl) {
    return 0;
  }
  /* TODO: remove all entries from queues */
  mct_queue_free(ctrl->realtime_queue.q);
  mct_queue_free(ctrl->offline_queue.q);
  pthread_mutex_destroy(&(ctrl->realtime_queue.mutex));
  pthread_mutex_destroy(&(ctrl->offline_queue.mutex));
  pthread_mutex_destroy(&(ctrl->ack_list.mutex));
  pthread_cond_destroy(&(ctrl->th_start_cond));
  close(ctrl->pfd[READ_FD]);
  close(ctrl->pfd[WRITE_FD]);
  vpe_hardware_destroy(ctrl->vpehw);
  free(ctrl);
  return 0;
}

void vpe_module_set_mod(mct_module_t *module, unsigned int module_type,
  unsigned int identity)
{
  CDBG("%s:%d, module_type=%d\n", __func__, __LINE__, module_type);
  if(!module) {
    CDBG_ERROR("%s:%d: failed", __func__, __LINE__);
    return;
  }
  if (mct_module_find_type(module, identity) != MCT_MODULE_FLAG_INVALID) {
    mct_module_remove_type(module, identity);
  }
  mct_module_add_type(module, module_type, identity);
}

boolean vpe_module_query_mod(mct_module_t *module, void *buf,
  unsigned int sessionid)
{
  int rc;
  if(!module || !buf) {
    CDBG_ERROR("%s:%d: failed, module=%p, query_buf=%p",
                __func__, __LINE__, module, buf);
    return FALSE;
  }
  mct_pipeline_cap_t *query_buf = (mct_pipeline_cap_t *)buf;
  mct_pipeline_pp_cap_t *pp_cap = &(query_buf->pp_cap);

  vpe_module_ctrl_t *ctrl = (vpe_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);

  /* TODO: Fill pp cap according to VPE HW caps*/
  vpe_hardware_cmd_t cmd;
  cmd.type = VPE_HW_CMD_GET_CAPABILITIES;
  rc = vpe_hardware_process_command(ctrl->vpehw, cmd);
  if(rc < 0) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return FALSE;
  }
  /* TODO: Need a linking function to fill pp cap based on HW caps? */
  pp_cap->min_num_pp_bufs += MODULE_VPE_MIN_NUM_PP_BUFS;
  pp_cap->feature_mask |= CAM_QCOM_FEATURE_CROP;
  return TRUE;
}

boolean vpe_module_start_session(mct_module_t *module, unsigned int sessionid)
{
  int32_t rc;
  CDBG_HIGH("%s:%d, info: starting session %d", __func__, __LINE__, sessionid);
  if(!module) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return FALSE;
  }
  vpe_module_ctrl_t *ctrl = (vpe_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return FALSE;
  }
  if(ctrl->session_count >= VPE_MODULE_MAX_SESSIONS) {
    CDBG_ERROR("%s:%d, failed, too many sessions, count=%d",
      __func__, __LINE__, ctrl->session_count);
    return FALSE;
  }

  /* create a new session specific params structure */
  int i;
  for(i=0; i < VPE_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i] == NULL) {
      ctrl->session_params[i] =
        (vpe_module_session_params_t*)
           malloc(sizeof(vpe_module_session_params_t));
      memset(ctrl->session_params[i], 0x00,
        sizeof(vpe_module_session_params_t));
      ctrl->session_params[i]->session_id = sessionid;
      break;
    }
  }

  /* start the thread only when first session starts */
  if(ctrl->session_count == 0) {
    /* open the vpe hardware */
    rc = vpe_hardware_open_subdev(ctrl->vpehw);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, vpe_thread_create() failed", __func__, __LINE__);
      return FALSE;
    }
    /* spawn the vpe thread */
    rc = vpe_thread_create(module);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, vpe_thread_create() failed", __func__, __LINE__);
      return FALSE;
    }
    CDBG_HIGH("%s:%d, info: vpe_thread created.", __func__, __LINE__);
  }
  ctrl->session_count++;
  CDBG_HIGH("%s:%d, info: session %d started.", __func__, __LINE__, sessionid);
  return TRUE;
}

boolean vpe_module_stop_session(mct_module_t *module, unsigned int sessionid)
{
  int32_t rc;
  if(!module) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return FALSE;
  }
  vpe_module_ctrl_t *ctrl = (vpe_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return FALSE;
  }
  CDBG_HIGH("%s:%d, info: stopping session %d ...", __func__, __LINE__, sessionid);
  ctrl->session_count--;
  /* stop the thread only when last session terminates */
  if(ctrl->session_count == 0) {
    /* stop the VPE thread */
    CDBG("%s:%d, info: stopping vpe_thread...", __func__, __LINE__);
    vpe_thread_msg_t msg;
    msg.type = VPE_THREAD_MSG_ABORT;
    rc = vpe_module_post_msg_to_thread(module, msg);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, vpe_module_post_msg_to_thread() failed",
        __func__, __LINE__);
      return FALSE;
    }
    /* wait for thread completion */
    pthread_join(ctrl->vpe_thread, NULL);
    /* close the vpe hardware */
    CDBG("%s:%d, closing vpe subdev...", __func__, __LINE__);
    vpe_hardware_close_subdev(ctrl->vpehw);
  }
  /* remove the session specific params */
  int i;
  for(i=0; i < VPE_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i]) {
      if(ctrl->session_params[i]->session_id == sessionid) {
        free(ctrl->session_params[i]);
        ctrl->session_params[i] = NULL;
        break;
      }
    }
  }
  CDBG_HIGH("%s:%d, info: session %d stopped.", __func__, __LINE__, sessionid);
  return TRUE;
}

/* vpe_module_handle_buf_divert_event:
 *
 * Description:
 *  Handle the MCT_EVENT_MODULE_BUF_DIVERT event. First put corresponding
 *  acknowledgement in a list which will be sent later. Depending on the
 *  stream's parameters, divert and processing events are added in
 *  vpe's priority queue. vpe_thread will pick up these events one by one in
 *  order and when all events corresponding to the ACK are processed,
 *  the ACK will be removed from list and will be sent upstream.
 *
 **/
int32_t vpe_module_handle_buf_divert_event(mct_module_t* module,
  mct_event_t* event)
{
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  vpe_module_ctrl_t* ctrl = (vpe_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  struct timeval tv1, tv2;
  gettimeofday(&tv1, NULL);
  isp_buf_divert_t* isp_buf =
    (isp_buf_divert_t*)(event->u.module_event.module_event_data);
  uint32_t frame_id = isp_buf->buffer.sequence;

  /* get stream parameters based on the event identity */
  vpe_module_stream_params_t *stream_params, *linked_stream_params;
  vpe_module_session_params_t *session_params;
  vpe_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  linked_stream_params = stream_params->linked_stream_params;

  vpe_module_stream_params_t *first_params=NULL, *second_params=NULL;
  uint32_t unproc_div_identity=0x00;
  boolean skip_first=FALSE, skip_second=FALSE;
  boolean unproc_required=FALSE, second_pass_required=FALSE;
  /* note: unlock these on all return paths */
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));

  /* decide processing requirements based on the stream params */
  if(linked_stream_params) { /* linked stream case */
    PTHREAD_MUTEX_LOCK(&(linked_stream_params->mutex));
    /* if both streams in the pair are off, drop frame */
    if (stream_params->is_stream_on == FALSE &&
        linked_stream_params->is_stream_on == FALSE) {
      CDBG("%s:%d, stream is off, drop frame and piggy-back ACK\n",
        __func__, __LINE__);
      isp_buf->ack_flag = TRUE;
      isp_buf->is_buf_dirty = 1;
      PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
      PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
      return 0;
    }
    /* if current stream is on and linked stream is off */
    else if (stream_params->is_stream_on == TRUE &&
        linked_stream_params->is_stream_on == FALSE) {
      /* only one pass required on current identity */
      first_params = stream_params;
      second_pass_required = FALSE;
    }
    /* if current stream is off and linked stream is on */
    else if(stream_params->is_stream_on == FALSE &&
        linked_stream_params->is_stream_on == TRUE) {
      /* only one pass required on linked identity */
      first_params = linked_stream_params;
      second_pass_required = FALSE;
    }
    /* if both streams are on */
    else if(stream_params->is_stream_on == TRUE &&
        linked_stream_params->is_stream_on == TRUE) {
      /* first pass on current identity and second pass on linked identity */
      first_params = stream_params;
      second_params = linked_stream_params;
      second_pass_required = TRUE;
    }
  } else { /* non-linked stream case */
    /* if stream is off, drop frame */
    if (stream_params->is_stream_on == FALSE) {
      CDBG("%s:%d, stream is off, drop frame and piggy-back ACK\n",
        __func__, __LINE__);
      isp_buf->ack_flag = TRUE;
      isp_buf->is_buf_dirty = 1;
      PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
      return 0;
    }
    first_params = stream_params;
    second_pass_required = FALSE;
  }
  /* decide if unprocessed divert is required */
  if(stream_params->div_info.divert_flags & PPROC_DIVERT_UNPROCESSED) {
      /* unprocess divert is required */
      unproc_required = TRUE;
      /* TODO: which identity to divert this */
      unproc_div_identity = stream_params->div_info.div_unproc_identity;
  }

  /* decide if skip is required for HFR */
  if (first_params) {
    if (first_params->hfr_skip_required) {
      if (vpe_decide_hfr_skip(frame_id - first_params->frame_offset,
        first_params->hfr_skip_count)) {
        CDBG("%s:%d, skipping frame_id=%d for identity=0x%x", __func__, __LINE__,
          frame_id, first_params->identity);
        CDBG("%s:%d, skip_count=%d, offset=%d", __func__, __LINE__,
         first_params->hfr_skip_count, first_params->frame_offset);
        skip_first = TRUE;
      }
    }
  }
  if (second_params) {
    if (second_params->hfr_skip_required) {
      if (vpe_decide_hfr_skip(frame_id - second_params->frame_offset,
        second_params->hfr_skip_count)) {
        CDBG("%s:%d, skip frame_id=%d for identity=0x%x", __func__, __LINE__,
          frame_id, second_params->identity);
        skip_second = TRUE;
      }
    }
  }

  /* create a key for ack with original event identity, this key will be
     put in all corresponding events in queue and used to release the ack */
  vpe_module_ack_key_t key;
  key.identity = event->identity;
  key.buf_idx = isp_buf->buffer.index;

  /* Decide the events to be queued to process this buffer */
  int event_idx = 0, num_events = 0;
  /* based on configuration, at max 3 events are queued for one buffer */
  vpe_module_event_t* vpe_event[3];
  /* Step 1. if unprocessed divert is needed, add an event for that */
  if(unproc_required == TRUE) {
    vpe_event[event_idx] = (vpe_module_event_t*)
                              malloc(sizeof(vpe_module_event_t));
    if(!vpe_event[event_idx]) {
      CDBG_ERROR("%s:%d, malloc() failed\n", __func__, __LINE__);
      PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
      if(linked_stream_params)
        PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
      return -ENOMEM;
    }
    memset(vpe_event[event_idx], 0x00, sizeof(vpe_module_event_t));
    vpe_event[event_idx]->type = VPE_MODULE_EVENT_DIVERT_BUF;
    vpe_event[event_idx]->ack_key = key;
    /* this is not a hw processing event */
    vpe_event[event_idx]->hw_process_flag = FALSE;
    /* by default all events are created valid */
    vpe_event[event_idx]->invalid = FALSE;
    /* copy isp buf and other data from the mct event */
    memcpy(&(vpe_event[event_idx]->u.divert_buf_data.isp_buf_divert),
      (isp_buf_divert_t*)(isp_buf), sizeof(isp_buf_divert_t));
    vpe_event[event_idx]->u.divert_buf_data.div_identity = unproc_div_identity;
    /* put original identity inside buf, for divert event only */
    vpe_event[event_idx]->u.divert_buf_data.isp_buf_divert.identity =
      event->identity;
    event_idx++;
  }
  /* Step 2. add event for 1st processing pass */
  if (skip_first == FALSE && first_params) {
    vpe_event[event_idx] = (vpe_module_event_t*)
                              malloc(sizeof(vpe_module_event_t));
    if(!vpe_event[event_idx]) {
      CDBG_ERROR("%s:%d, malloc() failed\n", __func__, __LINE__);
      PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
      if(linked_stream_params)
        PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
      return -ENOMEM;
    }
    memset(vpe_event[event_idx], 0x00, sizeof(vpe_module_event_t));
    vpe_event[event_idx]->type = VPE_MODULE_EVENT_PROCESS_BUF;
    vpe_event[event_idx]->ack_key = key;
    /* this is hw processing event */
    vpe_event[event_idx]->hw_process_flag = TRUE;
    /* by default all events are created valid */
    vpe_event[event_idx]->invalid = FALSE;
    /* copy isp buf and other data from the mct event */
    memcpy(&(vpe_event[event_idx]->u.process_buf_data.isp_buf_divert),
      (isp_buf_divert_t*)(isp_buf), sizeof(isp_buf_divert_t));
    vpe_event[event_idx]->u.process_buf_data.proc_identity =
      first_params->identity;
    /* copy the stream hw params in event */
    CDBG_ERROR("%s memcpy'ing hw_params...\n", __func__);
    memcpy(&(vpe_event[event_idx]->u.process_buf_data.hw_params),
      &(first_params->hw_params), sizeof(vpe_hardware_params_t));
    event_idx++;
  }

  /* Step 3. add event for 2nd processing pass, if needed. */
  /* TODO: need to handle this case properly */
  if(second_pass_required == TRUE && skip_second == FALSE) {
     if(!second_params) {
       CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
       PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
       if(linked_stream_params)
        PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
       return -EFAULT;
     }
    vpe_event[event_idx] = (vpe_module_event_t*)
                              malloc(sizeof(vpe_module_event_t));
    if(!vpe_event[event_idx]) {
      CDBG_ERROR("%s:%d, malloc() failed\n", __func__, __LINE__);
      PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
      if(linked_stream_params)
        PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
      return -ENOMEM;
    }
    memset(vpe_event[event_idx], 0x00, sizeof(vpe_module_event_t));
    vpe_event[event_idx]->type = VPE_MODULE_EVENT_PROCESS_BUF;
    vpe_event[event_idx]->ack_key = key;
    /* this is a hw processing event */
    vpe_event[event_idx]->hw_process_flag = TRUE;
    /* by default all events are created valid */
    vpe_event[event_idx]->invalid = FALSE;
    /* copy isp buf and other data from the mct event */
    memcpy(&(vpe_event[event_idx]->u.process_buf_data.isp_buf_divert),
      (isp_buf_divert_t*)(isp_buf), sizeof(isp_buf_divert_t));
    vpe_event[event_idx]->u.process_buf_data.proc_identity =
      second_params->identity;
    /* copy the stream hw params in event */
    CDBG_ERROR("%s memcpy'ing hw_params on 2nd pass...\n", __func__);
    memcpy(&(vpe_event[event_idx]->u.process_buf_data.hw_params),
      &(second_params->hw_params), sizeof(vpe_hardware_params_t));
    event_idx++;
  }
  num_events = event_idx;
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  if(linked_stream_params)
    PTHREAD_MUTEX_UNLOCK(&(linked_stream_params->mutex));
  /* if no events needs to be queued, do a piggy-back ACK */
  if (num_events == 0) {
    isp_buf->ack_flag = TRUE;
    isp_buf->is_buf_dirty = 1;
    return 0;
  }
  /* before queuing any events, first put corresponding ACK in the ack_list */
  vpe_module_put_new_ack_in_list(ctrl, key, 1, num_events);

  /* now enqueue all events one by one in priority queue */
  int i, rc;
  for(i=0; i<num_events; i++) {
    rc = vpe_module_enq_event(module, vpe_event[i],
           stream_params->priority);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed, i=%d\n", __func__, __LINE__, i);
      return -EFAULT;
    }
  }
  /* notify the thread about this new events */
  vpe_thread_msg_t msg;
  msg.type = VPE_THREAD_MSG_NEW_EVENT_IN_Q;
  vpe_module_post_msg_to_thread(module, msg);

  gettimeofday(&tv2, NULL);
  CDBG_LOW("%s:%d, downstream event time = %6ld us, ", __func__, __LINE__,
    (tv2.tv_sec - tv1.tv_sec)*1000000L +
    (tv2.tv_usec - tv1.tv_usec));
  return 0;
}

/* vpe_module_handle_isp_out_dim_event:
 *
 * Description:
 *
 **/
int32_t vpe_module_handle_isp_out_dim_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  vpe_module_ctrl_t* ctrl = (vpe_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  mct_stream_info_t *stream_info =
    (mct_stream_info_t *)(event->u.module_event.module_event_data);
  if(!stream_info) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  CDBG("%s:%d identity=0x%x, dim=%dx%d\n", __func__, __LINE__,
    event->identity, stream_info->dim.width, stream_info->dim.height);
  /* get stream parameters based on the event identity */
  vpe_module_stream_params_t *stream_params;
  vpe_module_session_params_t *session_params;
  vpe_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  /* update the dimension of the stream */
  stream_params->hw_params.input_info.width = stream_info->dim.width;
  stream_params->hw_params.input_info.height = stream_info->dim.height;
  stream_params->hw_params.input_info.stride =
    stream_info->buf_planes.plane_info.mp[0].stride;
  stream_params->hw_params.input_info.scanline =
    stream_info->buf_planes.plane_info.mp[0].scanline;
  /* format info */
  if (stream_info->fmt == CAM_FORMAT_YUV_420_NV12) {
    stream_params->hw_params.input_info.plane_fmt = VPE_PARAM_PLANE_CBCR;
  } else if (stream_info->fmt == CAM_FORMAT_YUV_420_NV21) {
    stream_params->hw_params.input_info.plane_fmt = VPE_PARAM_PLANE_CRCB;
  } else {
    CDBG_ERROR("%s:%d] Format not supported\n", __func__, __LINE__);
    PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
    return -EINVAL;
  }
  /* init crop info */
  /* stream_params->hw_params.crop_info.stream_crop.x = 0; */
  /* stream_params->hw_params.crop_info.stream_crop.y = 0; */
  /* stream_params->hw_params.crop_info.stream_crop.dx = stream_info->dim.width; */
  /* stream_params->hw_params.crop_info.stream_crop.dy = stream_info->dim.height; */
  /* stream_params->hw_params.crop_info.is_crop.x = 0; */
  /* stream_params->hw_params.crop_info.is_crop.y = 0; */
  /* stream_params->hw_params.crop_info.is_crop.dx = stream_info->dim.width; */
  /* stream_params->hw_params.crop_info.is_crop.dy = stream_info->dim.height; */
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  rc = vpe_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
      __func__, __LINE__, event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/* vpe_module_handle_stream_crop_event:
 *
 * Description:
 *
 **/
int32_t vpe_module_handle_stream_crop_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  vpe_module_ctrl_t* ctrl = (vpe_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  CDBG("%s:%d identity=0x%x", __func__, __LINE__, event->identity);
  mct_bus_msg_stream_crop_t *stream_crop =
    (mct_bus_msg_stream_crop_t *) event->u.module_event.module_event_data;
  if(!stream_crop) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  /* if crop is (0, 0, 0, 0) ignore the event */
  if (stream_crop->x == 0 && stream_crop->y == 0 &&
      stream_crop->crop_out_x == 0 && stream_crop->crop_out_x == 0) {
    return 0;
  }
  /* get stream parameters based on the event identity */
  vpe_module_stream_params_t *stream_params;
  vpe_module_session_params_t *session_params;
  vpe_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  /* stream_params->hw_params.crop_info.stream_crop.x = stream_crop->x; */
  /* stream_params->hw_params.crop_info.stream_crop.y = stream_crop->y; */
  /* stream_params->hw_params.crop_info.stream_crop.dx = stream_crop->crop_out_x; */
  /* stream_params->hw_params.crop_info.stream_crop.dy = stream_crop->crop_out_y; */
  CDBG_LOW("%s:%d stream_crop.x=%d, stream_crop.y=%d, stream_crop.dx=%d,"
           " stream_crop.dy=%d, identity=0x%x", __func__, __LINE__,
           stream_crop->x, stream_crop->y, stream_crop->crop_out_x,
           stream_crop->crop_out_y, event->identity);
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  rc = vpe_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
      __func__, __LINE__, event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/* vpe_module_handle_dis_update_event:
 *
 * Description:
 *
 **/
int32_t vpe_module_handle_dis_update_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  vpe_module_ctrl_t* ctrl = (vpe_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  is_update_t *is_update =
    (is_update_t *) event->u.module_event.module_event_data;
  if(!is_update) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  /* get stream parameters based on the event identity */
  vpe_module_stream_params_t *stream_params;
  vpe_module_session_params_t *session_params;
  vpe_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  /* stream_params->hw_params.crop_info.is_crop.x = is_update->x; */
  /* stream_params->hw_params.crop_info.is_crop.y = is_update->y; */
  /* stream_params->hw_params.crop_info.is_crop.dx = is_update->width; */
  /* stream_params->hw_params.crop_info.is_crop.dy = is_update->height; */
  CDBG_LOW("%s:%d is_crop.x=%d, is_crop.y=%d, is_crop.dx=%d, is_crop.dy=%d,"
    " identity=0x%x", __func__, __LINE__, is_update->x, is_update->y,
    is_update->width, is_update->height, event->identity);
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  rc = vpe_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
      __func__, __LINE__, event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/* vpe_module_handle_stream_cfg_event:
 *
 * Description:
 *
 **/
int32_t vpe_module_handle_stream_cfg_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  vpe_module_ctrl_t* ctrl = (vpe_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  sensor_out_info_t *sensor_out_info =
    (sensor_out_info_t *)(event->u.module_event.module_event_data);
  if (!sensor_out_info) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  /* get stream parameters based on the event identity */
  vpe_module_stream_params_t *stream_params;
  vpe_module_session_params_t *session_params;
  vpe_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  stream_params->frame_offset = sensor_out_info->num_frames_skip + 1;
  CDBG("%s:%d frame_offset=%d, identity=0x%x",
    __func__, __LINE__, stream_params->frame_offset, event->identity);
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  rc = vpe_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
      __func__, __LINE__, event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}


/**vpe_module_handle_div_info_event:
 *
 * Description:
 *
 **/
int32_t vpe_module_handle_div_info_event(mct_module_t* module,
  mct_event_t* event)
{
  int32_t rc;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  vpe_module_ctrl_t* ctrl = (vpe_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  pproc_divert_info_t *div_info =
    (pproc_divert_info_t *)(event->u.module_event.module_event_data);
  if (!div_info) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
#if 0 //temp
  if (!div_info->name) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  /* check if this config is intended for this module */
  if (strncmp(MCT_OBJECT_NAME(module), div_info->name,
       sizeof(MCT_OBJECT_NAME(module))) != 0) {
    rc = vpe_module_send_event_downstream(module, event);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
        __func__, __LINE__, event->u.module_event.type, event->identity);
      return -EFAULT;
    }
    return 0;
  }
#endif
  /* get stream parameters based on the event identity */
  vpe_module_stream_params_t *stream_params;
  vpe_module_session_params_t *session_params;
  vpe_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  CDBG("%s:%d div_flags=0x%x, unproc_div_identity=0x%x, event_identity=0x%x",
    __func__, __LINE__, div_info->divert_flags,
    div_info->div_unproc_identity, event->identity);
  memcpy(&(stream_params->div_info), div_info, sizeof(pproc_divert_info_t));
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  return 0;
}

/* vpe_module_set_parm_hfr_mode:
 *
 **/
static int32_t vpe_module_set_parm_hfr_mode(vpe_module_ctrl_t *ctrl,
  uint32_t identity, cam_hfr_mode_t hfr_mode)
{
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  /* get parameters based on the event identity */
  vpe_module_stream_params_t *stream_params;
  vpe_module_session_params_t *session_params;
  vpe_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!session_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  /* apply this to all streams where hfr skip is required */
  int i;
  for(i=0; i<VPE_MODULE_MAX_STREAMS; i++) {
    if(session_params->stream_params[i]) {
      PTHREAD_MUTEX_LOCK(&(session_params->stream_params[i]->mutex));
      if(session_params->stream_params[i]->hfr_skip_required) {
        switch(hfr_mode) {
        case CAM_HFR_MODE_OFF:
          session_params->stream_params[i]->hfr_skip_count = 0;
          break;
        case CAM_HFR_MODE_60FPS:
          session_params->stream_params[i]->hfr_skip_count = 1;
          break;
        case CAM_HFR_MODE_90FPS:
          session_params->stream_params[i]->hfr_skip_count = 2;
          break;
        case CAM_HFR_MODE_120FPS:
          session_params->stream_params[i]->hfr_skip_count = 3;
          break;
        case CAM_HFR_MODE_150FPS:
          session_params->stream_params[i]->hfr_skip_count = 4;
          break;
        default:
          CDBG_ERROR("%s:%d, bad hfr_mode=%d", __func__, __LINE__, hfr_mode);
          PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
          return -EINVAL;
        }
        PTHREAD_MUTEX_UNLOCK(&(session_params->stream_params[i]->mutex));
      }
    }
  }
  return 0;
}

/* vpe_module_handle_set_parm_event:
 *
 * Description:
 *   Handle the set_parm event.
 **/
int32_t vpe_module_handle_set_parm_event(mct_module_t* module,
  mct_event_t* event)
{
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p", __func__, __LINE__,
                module, event);
    return -EINVAL;
  }
  mct_event_control_parm_t *ctrl_parm =
    (mct_event_control_parm_t *) event->u.ctrl_event.control_event_data;
  if(!ctrl_parm) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  vpe_module_ctrl_t *ctrl = (vpe_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  int32_t rc;
  switch (ctrl_parm->type) {
  case CAM_INTF_PARM_HFR: {
    if(!(ctrl_parm->parm_data)) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return -EFAULT;
    }
    cam_hfr_mode_t hfr_mode =
      *(cam_hfr_mode_t *)(ctrl_parm->parm_data);
    CDBG("%s:%d, CAM_INTF_PARM_HFR, mode=%d, identity=0x%x",
      __func__, __LINE__, hfr_mode, event->identity);
    rc = vpe_module_set_parm_hfr_mode(ctrl, event->identity, hfr_mode);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
      return rc;
    }
    break;
  }
  default:
    break;
  }

  rc = vpe_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
      __func__, __LINE__, event->u.module_event.type, event->identity);
    return -EFAULT;
  }
  return 0;
}

/* vpe_module_handle_streamon_event:
 *
 **/
int32_t vpe_module_handle_streamon_event(mct_module_t* module,
  mct_event_t* event)
{
  vpe_module_stream_buff_info_t   stream_buff_info;
  vpe_hardware_stream_buff_info_t hw_strm_buff_info;
  mct_stream_info_t              *streaminfo =
    (mct_stream_info_t *)event->u.ctrl_event.control_event_data;
  vpe_module_ctrl_t              *ctrl =
    (vpe_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  vpe_hardware_cmd_t              cmd;
  boolean                         rc = -EINVAL;
  mct_event_t                     new_event;
  stats_get_data_t                stats_get;

  /* get stream parameters */
  vpe_module_session_params_t* session_params = NULL;
  vpe_module_stream_params_t*  stream_params = NULL;
  vpe_module_get_params_for_identity(ctrl, event->identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  memset(&stream_buff_info, 0, sizeof(vpe_module_stream_buff_info_t));
  memset(&hw_strm_buff_info, 0, sizeof(vpe_hardware_stream_buff_info_t));

  /* attach the identity */
  stream_buff_info.identity = event->identity;
  /* traverse through the mct stream buff list and create vpe module's
     own list of buffer info */
  if (mct_list_traverse(streaminfo->img_buffer_list,
    vpe_module_util_map_buffer_info, &stream_buff_info) == FALSE) {
    CDBG_ERROR("%s:%d, error creating stream buff list\n", __func__,
      __LINE__);
    goto VPE_MODULE_STREAMON_ERROR1;
  }

  /* create and translate to hardware buffer array */
  hw_strm_buff_info.buffer_info = (vpe_hardware_buffer_info_t *)malloc(
    sizeof(vpe_hardware_buffer_info_t) * stream_buff_info.num_buffs);
  if(NULL == hw_strm_buff_info.buffer_info) {
    CDBG_ERROR("%s:%d, error creating hw buff list\n", __func__,
      __LINE__);
    goto VPE_MODULE_STREAMON_ERROR1;
  }

  hw_strm_buff_info.identity = stream_buff_info.identity;
  if (mct_list_traverse(stream_buff_info.buff_list,
    vpe_module_util_create_hw_stream_buff, &hw_strm_buff_info) == FALSE) {
    CDBG_ERROR("%s:%d, error creating stream buff list\n", __func__,
      __LINE__);
    goto VPE_MODULE_STREAMON_ERROR2;
  }

  if(hw_strm_buff_info.num_buffs != stream_buff_info.num_buffs) {
    CDBG_ERROR("%s:%d, error creating stream buff list\n", __func__,
      __LINE__);
    goto VPE_MODULE_STREAMON_ERROR2;
  }

  cmd.type = VPE_HW_CMD_STREAMON;
  cmd.u.stream_buff_list = &hw_strm_buff_info;
  rc = vpe_hardware_process_command(ctrl->vpehw, cmd);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    goto VPE_MODULE_STREAMON_ERROR2;
  }
  rc = vpe_module_send_event_downstream(module,event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    goto VPE_MODULE_STREAMON_ERROR2;
  }
  /* change state to stream ON */
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  stream_params->is_stream_on = TRUE;
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));
  CDBG_HIGH("%s:%d, identity=0x%x, stream-on done", __func__, __LINE__,
    event->identity);

  rc = 0;

VPE_MODULE_STREAMON_ERROR2:
  free(hw_strm_buff_info.buffer_info);

VPE_MODULE_STREAMON_ERROR1:
  mct_list_traverse(stream_buff_info.buff_list,
    vpe_module_util_free_buffer_info, &stream_buff_info);
  mct_list_free_list(stream_buff_info.buff_list);

  return rc;
}

/* vpe_module_handle_streamoff_event:
 *
 **/
int32_t vpe_module_handle_streamoff_event(mct_module_t* module,
  mct_event_t* event)
{
  int rc;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n",
      __func__, __LINE__, module, event);
    return -EINVAL;
  }
  mct_stream_info_t *streaminfo =
    (mct_stream_info_t *)event->u.ctrl_event.control_event_data;
  uint32_t identity = event->identity;
  CDBG_HIGH("%s:%d, info: doing stream-off for identity 0x%x",
    __func__, __LINE__, identity);

  vpe_module_ctrl_t* ctrl = (vpe_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  vpe_module_session_params_t* session_params = NULL;
  vpe_module_stream_params_t*  stream_params = NULL;
  vpe_module_get_params_for_identity(ctrl, identity,
    &session_params, &stream_params);
  if(!stream_params) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  /* change the state of this stream to OFF, this will prevent
     any incoming buffers to be added to the processing queue  */
  PTHREAD_MUTEX_LOCK(&(stream_params->mutex));
  stream_params->is_stream_on = FALSE;
  PTHREAD_MUTEX_UNLOCK(&(stream_params->mutex));

  /* send stream_off to downstream. This blocking call ensures
     downstream modules are streamed off and no acks pending from them */
  rc = vpe_module_send_event_downstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  CDBG("%s:%d, info: downstream stream-off done.", __func__, __LINE__);

  /* invalidate any remaining entries in queue corresponding to
     this identity. This will also send/update corresponding ACKs */
  CDBG("%s:%d, info: invalidating queue.", __func__, __LINE__);
  rc = vpe_module_invalidate_queue(ctrl, identity);
    if(rc < 0) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }

  /* process hardware command for stream off, this ensures
     hardware is done with this identity */
  vpe_hardware_cmd_t cmd;
  cmd.type = VPE_HW_CMD_STREAMOFF;
  cmd.u.streamoff_identity = streaminfo->identity;
  rc = vpe_hardware_process_command(ctrl->vpehw, cmd);
  if(rc < 0) {
    CDBG_ERROR("%s:%d: hw streamoff failed\n", __func__, __LINE__);
    return -rc;
  }
  CDBG_HIGH("%s:%d, info: stream-off done for identity 0x%x",
    __func__, __LINE__, identity);
  return 0;
}

/* vpe_module_post_msg_to_thread:
 *
 * @module: vpe module pointer
 * @msg: message to be posted for thread
 * Description:
 *  Writes message to the pipe for which the vpe_thread is listening to.
 *
 **/
int32_t vpe_module_post_msg_to_thread(mct_module_t *module,
  vpe_thread_msg_t msg)
{
  int32_t rc;
  if(!module) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EINVAL;
  }
  CDBG_LOW("%s:%d, msg.type=%d", __func__, __LINE__, msg.type);
  vpe_module_ctrl_t *ctrl = (vpe_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  rc = write(ctrl->pfd[WRITE_FD], &msg, sizeof(vpe_thread_msg_t));
  if(rc < 0) {
    CDBG_ERROR("%s:%d, write() failed\n", __func__, __LINE__);
    return -EIO;
  }
  return 0;
}

/* vpe_module_enq_event:
 *
 * @module: vpe module pointer
 * @event:  vpe_event to be queued
 * @prio:   priority of the event(realtime/offline)
 *
 * Description:
 *  Enqueues a vpe_event into realtime or offline queue based on the
 *  priority.
 *
 **/
int32_t vpe_module_enq_event(mct_module_t* module,
  vpe_module_event_t* vpe_event, vpe_priority_t prio)
{
  if(!module || !vpe_event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p", __func__, __LINE__,
      module, vpe_event);
    return -EINVAL;
  }
  vpe_module_ctrl_t *ctrl = (vpe_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);

  CDBG_LOW("%s:%d, prio=%d", __func__, __LINE__, prio);
  switch (prio) {
  case VPE_PRIORITY_REALTIME:
    PTHREAD_MUTEX_LOCK(&(ctrl->realtime_queue.mutex));
    mct_queue_push_tail(ctrl->realtime_queue.q, vpe_event);
    CDBG_LOW("%s:%d, real-time queue size = %d", __func__, __LINE__,
      ctrl->realtime_queue.q->length);
    PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
    break;
  case VPE_PRIORITY_OFFLINE:
    PTHREAD_MUTEX_LOCK(&(ctrl->offline_queue.mutex));
    mct_queue_push_tail(ctrl->offline_queue.q, vpe_event);
    CDBG_LOW("%s:%d, offline queue size = %d", __func__, __LINE__,
      ctrl->offline_queue.q->length);
    PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
    break;
  default:
    CDBG_ERROR("%s:%d, failed, bad prio value=%d", __func__, __LINE__, prio);
    return -EINVAL;
  }
  return 0;
}

int32_t vpe_module_send_event_downstream(mct_module_t* module,
   mct_event_t* event)
{
  boolean ret;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p", __func__, __LINE__,
                module, event);
    return -EINVAL;
  }
  uint32_t identity = event->identity;

  /* TODO: Because module type is not identity based in mct_module_t we can
     either privately use it or depend on the port's peer existence to
     determine whether to forward event downstream or not. So temporarily
     disabling this condition check as because video stream comes after
     preview and could potentially modify module->type to sink so that
     downstream module will not receive events and hence ack is not reflected
     back. Non-existence of a port with match identity means downstream module
     is not available and is just an indication of this module is sink or
     peerless */
#if 0
  /* forward the event downstream only if we are not sink/peerless module */
  if(module->type == MCT_MODULE_FLAG_SINK ||
     module->type == MCT_MODULE_FLAG_PEERLESS) {
    /* CDBG("%s:%d, info: module is sink/peerless, event not"
         "sent for identity=0x%x", __func__, __LINE__, identity);
    */
    return 0;
  }
#endif
  /* find corresponding source port */
  mct_port_t* port = vpe_module_find_port_with_identity(module, MCT_PORT_SRC,
                       identity);
  if(!port) {
    CDBG_LOW("%s:%d, no source port found.with identity=0x%x",
      __func__, __LINE__, identity);
    return 0;
  }
  /* if port has a peer, post event to the downstream peer */
  if(MCT_PORT_PEER(port) == NULL) {
    CDBG_ERROR("%s:%d, failed, no downstream peer found.", __func__, __LINE__);
    return -EINVAL;
  }
  ret = mct_port_send_event_to_peer(port, event);
  if(ret == FALSE) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  return 0;
}

/* vpe_module_send_event_upstream:
 *
 * Description:
 *  Sends event to the upstream peer based on the event identity.
 *
 **/
int32_t vpe_module_send_event_upstream(mct_module_t* module,
   mct_event_t* event)
{
  boolean ret;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p", __func__, __LINE__,
                module, event);
    return -EINVAL;
  }
  uint32_t identity = event->identity;
  /* find corresponding sink port */
  mct_port_t* port = vpe_module_find_port_with_identity(module, MCT_PORT_SINK,
                       identity);
  if(!port) {
    CDBG_ERROR("%s:%d, failed, no sink port found.with identity=0x%x",
      __func__, __LINE__, identity);
    return -EINVAL;
  }
  /* if port has a peer, post event to the upstream peer */
  if(!MCT_PORT_PEER(port)) {
    CDBG_ERROR("%s:%d, failed, no upstream peer found.", __func__, __LINE__);
    return -EINVAL;
  }
  ret = mct_port_send_event_to_peer(port, event);
  if(ret == FALSE) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EFAULT;
  }
  return 0;
}

/* vpe_module_invalidate_queue:
 *
 **/
int32_t vpe_module_invalidate_queue(vpe_module_ctrl_t* ctrl,
  uint32_t identity)
{
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  /* TODO: currently only realtime q used, to be extended for offline case */
  vpe_module_event_queue_t *queue;
  queue = &(ctrl->realtime_queue);
  void*  input[2];
  input[0] = (uint32_t*)ctrl;
  input[1] = &identity;
  PTHREAD_MUTEX_LOCK(&(queue->mutex));
  mct_queue_traverse(queue->q, vpe_module_invalidate_q_traverse_func,
    input);
  PTHREAD_MUTEX_UNLOCK(&(queue->mutex));
  return 0;
}

/* vpe_module_send_buf_divert_ack:
 *
 *  Sends a buf_divert_ack to upstream module.
 *
 **/
static int32_t vpe_module_send_buf_divert_ack(vpe_module_ctrl_t *ctrl,
  isp_buf_divert_ack_t isp_ack)
{
  mct_event_t event;
  int rc;
  memset(&event, 0x00, sizeof(mct_event_t));
  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = isp_ack.identity;
  event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT_ACK;
  event.u.module_event.module_event_data = &isp_ack;
  CDBG_LOW("%s:%d, sending isp ack with identity=0x%x, is_buf_dirty=%d, "
           "buf_idx=%d", __func__, __LINE__, isp_ack.identity,
           isp_ack.is_buf_dirty, isp_ack.buf_idx);
  rc = vpe_module_send_event_upstream(ctrl->p_module, &event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  return 0;
}

/* vpe_module_do_ack:
 *
 *  Decrements the refcount of the ACK which is stored in the ack_list,
 *  correspoding to the key. If the refcount becomes 0, a buf_divert_ack
 *  is sent upstream. At this time the ack entry is removed from list.
 *
 **/
int32_t vpe_module_do_ack(vpe_module_ctrl_t *ctrl,
  vpe_module_ack_key_t key)
{
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EINVAL;
  }
  /* find corresponding ack from the list. If the all references
     to that ack are done, send the ack and remove the entry from the list */
  vpe_module_ack_t *vpe_ack;
  CDBG("%s:%d, buf_idx=%d, identity=0x%x", __func__, __LINE__,
    key.buf_idx, key.identity);
  PTHREAD_MUTEX_LOCK(&(ctrl->ack_list.mutex));
  vpe_ack = vpe_module_find_ack_from_list(ctrl, key);
  if(!vpe_ack) {
    CDBG_ERROR("%s:%d, failed, ack not found in list, for buf_idx=%d, "
      "identity=0x%x", __func__, __LINE__, key.buf_idx, key.identity);
    PTHREAD_MUTEX_UNLOCK(&(ctrl->ack_list.mutex));
    return -EFAULT;
  }
  vpe_ack->ref_count--;
  CDBG("%s:%d, vpe_ack->ref_count=%d\n", __func__, __LINE__,
    vpe_ack->ref_count);
  struct timeval tv;
  if(vpe_ack->ref_count == 0) {
    ctrl->ack_list.list = mct_list_remove(ctrl->ack_list.list, vpe_ack);
    ctrl->ack_list.size--;
    /* unlock before sending event to prevent any deadlock */
    PTHREAD_MUTEX_UNLOCK(&(ctrl->ack_list.mutex));
    gettimeofday(&(vpe_ack->out_time), NULL);
    CDBG_LOW("%s:%d, in_time=%ld.%ld us, out_time=%ld.%ld us, ",
      __func__, __LINE__, vpe_ack->in_time.tv_sec, vpe_ack->in_time.tv_usec,
      vpe_ack->out_time.tv_sec, vpe_ack->out_time.tv_usec);
    CDBG_LOW("%s:%d, holding time = %6ld us, ", __func__, __LINE__,
      (vpe_ack->out_time.tv_sec - vpe_ack->in_time.tv_sec)*1000000L +
      (vpe_ack->out_time.tv_usec - vpe_ack->in_time.tv_usec));
    vpe_module_send_buf_divert_ack(ctrl, vpe_ack->isp_buf_divert_ack);
    gettimeofday(&tv, NULL);
    CDBG_LOW("%s:%d, upstream event time = %6ld us, ", __func__, __LINE__,
      (tv.tv_sec - vpe_ack->out_time.tv_sec)*1000000L +
      (tv.tv_usec - vpe_ack->out_time.tv_usec));
    free(vpe_ack);
  } else {
    PTHREAD_MUTEX_UNLOCK(&(ctrl->ack_list.mutex));
  }
  return 0;
}

/* vpe_module_handle_ack_from_downstream:
 *
 *  Handles the buf_divert_ack event coming from downstream module.
 *  Corresponding ACK stored in ack_list is updated and/or released
 *  accordingly.
 *
 */
static int32_t vpe_module_handle_ack_from_downstream(mct_module_t* module,
  mct_event_t* event)
{
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  vpe_module_ctrl_t* ctrl = (vpe_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  isp_buf_divert_ack_t* isp_buf_ack =
    (isp_buf_divert_ack_t*)(event->u.module_event.module_event_data);

  vpe_module_ack_key_t key;
  key.identity = isp_buf_ack->identity;
  key.buf_idx = isp_buf_ack->buf_idx;
  CDBG("%s:%d, doing ack for divert_done ack from downstream",
    __func__, __LINE__);
  vpe_module_do_ack(ctrl, key);
  return 0;
}

/* vpe_module_put_new_ack_in_list:
 *
 * Description:
 *   Adds a new ACK in the ack_list with the given params.
 **/
int32_t vpe_module_put_new_ack_in_list(vpe_module_ctrl_t *ctrl,
  vpe_module_ack_key_t key, int32_t buf_dirty, int32_t ref_count)
{
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  /* this memory will be freed by thread when ack is removed from list */
  vpe_module_ack_t *vpe_ack =
    (vpe_module_ack_t *) malloc (sizeof(vpe_module_ack_t));
  if(!vpe_ack) {
    CDBG_ERROR("%s:%d, malloc failed\n", __func__, __LINE__);
    return -ENOMEM;
  }
  memset(vpe_ack, 0x00, sizeof(vpe_module_ack_t));
  vpe_ack->isp_buf_divert_ack.identity = key.identity;
  vpe_ack->isp_buf_divert_ack.buf_idx = key.buf_idx;
  vpe_ack->isp_buf_divert_ack.is_buf_dirty = buf_dirty;
  vpe_ack->ref_count = ref_count;
  CDBG("%s:%d, adding ack in list, identity=0x%x", __func__, __LINE__,
    vpe_ack->isp_buf_divert_ack.identity);
  CDBG("%s:%d, buf_idx=%d, ref_count=%d", __func__, __LINE__,
    vpe_ack->isp_buf_divert_ack.buf_idx, vpe_ack->ref_count);
  PTHREAD_MUTEX_LOCK(&(ctrl->ack_list.mutex));
  gettimeofday(&(vpe_ack->in_time), NULL);
  ctrl->ack_list.list = mct_list_append(ctrl->ack_list.list,
                          vpe_ack, NULL, NULL);
  ctrl->ack_list.size++;
  PTHREAD_MUTEX_UNLOCK(&(ctrl->ack_list.mutex));
  return 0;
}

int32_t vpe_module_process_downstream_event(mct_module_t* module,
  mct_event_t* event)
{
  boolean ret;
  int rc;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  uint32_t identity = event->identity;
  vpe_module_ctrl_t *ctrl = (vpe_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  /* handle events based on type, if not handled, forward it downstream */
  switch(event->type) {
  case MCT_EVENT_MODULE_EVENT: {
    switch(event->u.module_event.type) {
    case MCT_EVENT_MODULE_BUF_DIVERT:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_BUF_DIVERT: identity=0x%x", __func__,
        __LINE__, identity);
      rc = vpe_module_handle_buf_divert_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_ISP_OUTPUT_DIM:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_ISP_OUTPUT_DIM: identity=0x%x", __func__,
        __LINE__, identity);
      rc = vpe_module_handle_isp_out_dim_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_STREAM_CROP:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_STREAM_CROP: identity=0x%x", __func__,
        __LINE__, identity);
      rc = vpe_module_handle_stream_crop_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_STATS_DIS_UPDATE:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_STATS_DIS_UPDATE: identity=0x%x", __func__,
        __LINE__, identity);
      rc = vpe_module_handle_dis_update_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_SET_STREAM_CONFIG:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_SET_STREAM_CONFIG: identity=0x%x", __func__,
        __LINE__, identity);
      rc = vpe_module_handle_stream_cfg_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_PPROC_DIVERT_INFO:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_PPROC_DIVERT_INFO: identity=0x%x", __func__,
        __LINE__, identity);
      rc = vpe_module_handle_div_info_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    default:
      rc = vpe_module_send_event_downstream(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed, module_event_type=%d, identity=0x%x",
          __func__, __LINE__, event->u.module_event.type, identity);
        return -EFAULT;
      }
      break;
    }
    break;
  }
  case MCT_EVENT_CONTROL_CMD: {
    switch(event->u.ctrl_event.type) {
    case MCT_EVENT_CONTROL_STREAMON: {
      rc = vpe_module_handle_streamon_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, streamon failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    }
    case MCT_EVENT_CONTROL_STREAMOFF: {
      rc = vpe_module_handle_streamoff_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, streamoff failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    }
    case MCT_EVENT_CONTROL_SET_PARM: {
      rc = vpe_module_handle_set_parm_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    }
    default:
      rc = vpe_module_send_event_downstream(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed, control_event_type=%d, identity=0x%x",
          __func__, __LINE__, event->u.ctrl_event.type, identity);
        return -EFAULT;
      }
      break;
    }
    break;
  }
  default:
    CDBG_ERROR("%s:%d, failed, bad event type=%d, identity=0x%x",
      __func__, __LINE__, event->type, identity);
    return -EFAULT;
  }
  return 0;
}

int32_t vpe_module_process_upstream_event(mct_module_t* module,
  mct_event_t *event)
{
  int rc;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  uint32_t identity = event->identity;
  CDBG_LOW("%s:%d: identity=0x%x, event->type=%d", __func__, __LINE__,
       identity, event->type);
  /* todo : event handling */
  switch(event->type) {
  case MCT_EVENT_MODULE_EVENT: {
    switch(event->u.module_event.type) {
    case MCT_EVENT_MODULE_BUF_DIVERT_ACK:
      CDBG("%s:%d: MCT_EVENT_MODULE_BUF_DIVERT_ACK: identity=0x%x", __func__,
        __LINE__, identity);
      rc = vpe_module_handle_ack_from_downstream(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    default:
      break;
    }
    break;
  }
  default:
    /* all upstream events are module events */
    break;
  }
  /* forward the event upstream if we are not source/peerless module */
  if((mct_module_find_type(module, event->identity)) !=
      MCT_MODULE_FLAG_SOURCE &&
     (mct_module_find_type(module, event->identity)) !=
      MCT_MODULE_FLAG_PEERLESS) {
    rc = vpe_module_send_event_upstream(module, event);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
      return rc;
    }
  }
  return 0;
}

/* vpe_module_notify_add_stream:
 *
 * creates and initializes the stream-specific paramater structures when a
 * stream is reserved in port
 **/
int32_t vpe_module_notify_add_stream(mct_module_t* module, mct_port_t* port,
  mct_stream_info_t* stream_info)
{
  if(!module || !stream_info || !port) {
    CDBG_ERROR("%s:%d, failed, module=%p, port=%p, stream_info=%p\n", __func__,
      __LINE__, module, port, stream_info);
    return -EINVAL;
  }
  vpe_module_ctrl_t *ctrl = (vpe_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed, module=%p\n", __func__, __LINE__, module);
    return -EINVAL;
  }
  uint32_t identity = stream_info->identity;
  /* create stream specific params structure */
  uint32_t session_id;
  int i,j;
  boolean success = FALSE;
  vpe_hardware_params_t *hw_params;
  session_id = VPE_GET_SESSION_ID(identity);
  CDBG("%s:%d: identity=0x%x\n", __func__, __LINE__, identity);

  /* find if a stream is already added on this port. If yes, we need to link
     that stream with this. */
  vpe_module_session_params_t *linked_session_params = NULL;
  vpe_module_stream_params_t *linked_stream_params = NULL;
  uint32_t linked_identity;
  int32_t found = vpe_port_get_linked_identity(port, identity,
    &linked_identity);
  if (found > 0) {
    CDBG("%s:%d, found linked identity=0x%x", __func__, __LINE__,
      linked_identity);
    vpe_module_get_params_for_identity(ctrl,linked_identity,
      &linked_session_params, &linked_stream_params);
    if (!linked_stream_params) {
      CDBG_ERROR("%s:%d, failed, module=%p\n", __func__, __LINE__, module);
      return -EINVAL;
    }
  }

  for(i=0; i < VPE_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i]) {
      if(ctrl->session_params[i]->session_id == session_id) {
        for(j=0; j < VPE_MODULE_MAX_STREAMS; j++) {
          if(ctrl->session_params[i]->stream_params[j] == NULL) {
            ctrl->session_params[i]->stream_params[j] =
              (vpe_module_stream_params_t *)
                 malloc (sizeof(vpe_module_stream_params_t));
            memset(ctrl->session_params[i]->stream_params[j], 0x00,
              sizeof(vpe_module_stream_params_t));
            ctrl->session_params[i]->stream_params[j]->identity = identity;

            /* assign priority */
            if(stream_info->streaming_mode == CAM_STREAMING_MODE_CONTINUOUS) {
              ctrl->session_params[i]->stream_params[j]->priority =
                VPE_PRIORITY_REALTIME;
            } else {
              ctrl->session_params[i]->stream_params[j]->priority =
                VPE_PRIORITY_OFFLINE;
            }
            /* hfr_skip_required in only in preview stream */
            ctrl->session_params[i]->stream_params[j]->hfr_skip_count = 0;
            ctrl->session_params[i]->stream_params[j]->hfr_skip_required =
              (stream_info->stream_type == CAM_STREAM_TYPE_PREVIEW) ?
                TRUE : FALSE;

            /* init divert information */
            ctrl->session_params[i]->stream_params[j]->
              div_info.divert_flags = 0;
            ctrl->session_params[i]->stream_params[j]->
              div_info.num_passes = 0; // TODO: review

            /* assign stream type */
            ctrl->session_params[i]->stream_params[j]->stream_type =
              stream_info->stream_type;
            hw_params = &ctrl->session_params[i]->stream_params[j]->hw_params;
            /* output dimensions */
            hw_params->output_info.width = stream_info->dim.width;
            hw_params->output_info.height = stream_info->dim.height;
            hw_params->output_info.stride =
              stream_info->buf_planes.plane_info.mp[0].stride;
            hw_params->output_info.scanline =
              stream_info->buf_planes.plane_info.mp[0].scanline;
            /* format info */
            if (stream_info->fmt == CAM_FORMAT_YUV_420_NV12) {
              hw_params->output_info.plane_fmt = VPE_PARAM_PLANE_CBCR;
            } else if (stream_info->fmt == CAM_FORMAT_YUV_420_NV21) {
              hw_params->output_info.plane_fmt = VPE_PARAM_PLANE_CRCB;
            } else {
              CDBG_ERROR("%s:%d, failed. Format not supported\n", __func__,
                __LINE__);
              return -EINVAL;
            }
            /* set linked stream */
            if (linked_stream_params) {
              ctrl->session_params[i]->stream_params[j]->linked_stream_params =
                linked_stream_params;
              linked_stream_params->linked_stream_params =
                ctrl->session_params[i]->stream_params[j];
            } else {
              ctrl->session_params[i]->stream_params[j]->linked_stream_params =
                NULL;
            }
            /* initialize the mutex for stream_params */
            pthread_mutex_init(
              &(ctrl->session_params[i]->stream_params[j]->mutex), NULL);
            ctrl->session_params[i]->stream_count++;
            success = TRUE;
            vpe_module_dump_stream_params(
              ctrl->session_params[i]->stream_params[j], __func__, __LINE__);
            break;
          }
        }
      }
    }
    if(success == TRUE) {
      break;
    }
  }
  if(success == FALSE) {
    CDBG_ERROR("%s:%d, failed, identity=0x%x", __func__, __LINE__, identity);
    return -EFAULT;
  }
  CDBG_HIGH("%s:%d, info: success, identity=0x%x", __func__, __LINE__, identity);
  return 0;
}

/* vpe_module_notify_remove_stream:
 *
 *  destroys stream-specific data structures when a stream is unreserved
 *  in port
 **/
int32_t vpe_module_notify_remove_stream(mct_module_t* module, uint32_t identity)
{
  if(!module) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  vpe_module_ctrl_t *ctrl = (vpe_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  CDBG("%s:%d: identity=0x%x\n", __func__, __LINE__, identity);
  /* destroy stream specific params structure */
  uint32_t session_id;
  int i,j;
  boolean success = FALSE;
  session_id = VPE_GET_SESSION_ID(identity);
  for(i=0; i < VPE_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i]) {
      if(ctrl->session_params[i]->session_id == session_id) {
        for(j=0; j < VPE_MODULE_MAX_STREAMS; j++) {
          if(ctrl->session_params[i]->stream_params[j]) {
            if(ctrl->session_params[i]->stream_params[j]->identity ==
                identity) {
              /* remove linked params */
              if (ctrl->session_params[i]->stream_params[j]->
                  linked_stream_params) {
                ctrl->session_params[i]->stream_params[j]->
                  linked_stream_params->linked_stream_params = NULL;
                ctrl->session_params[i]->stream_params[j]->
                  linked_stream_params = NULL;
              }
              pthread_mutex_destroy(
                &(ctrl->session_params[i]->stream_params[j]->mutex));
              free(ctrl->session_params[i]->stream_params[j]);
              ctrl->session_params[i]->stream_params[j] = NULL;
              ctrl->session_params[i]->stream_count--;
              success = TRUE;
              break;
            }
          }
        }
      }
    }
    if(success == TRUE) {
      break;
    }
  }
  if(success == FALSE) {
    CDBG_ERROR("%s:%d, failed, identity=0x%x", __func__, __LINE__, identity);
    return -EFAULT;
  }
  return 0;
}
