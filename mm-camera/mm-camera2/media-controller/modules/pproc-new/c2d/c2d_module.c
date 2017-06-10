/*============================================================================

  Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "c2d_module.h"
#include "c2d_port.h"
#include "c2d_log.h"
#include "c2d_module_events.h"
#include "pp_buf_mgr.h"
#include <cutils/properties.h>
#include "server_debug.h"

#define C2D_NUM_SINK_PORTS    8
#define C2D_NUM_SOURCE_PORTS  8

volatile uint32_t gCamC2dLogLevel = 0;
char c2d_prop[PROPERTY_VALUE_MAX];

/** c2d_module_init:
 *  Args:
 *    @name: module name
 *  Return:
 *    - mct_module_t pointer corresponding to c2d on SUCCESS
 *    - NULL in case of FAILURE or if C2D hardware does not
 *      exist
 **/
mct_module_t *c2d_module_init(const char *name)
{
  mct_module_t *module;
  c2d_module_ctrl_t* ctrl;
  CDBG_ERROR("%s:%d name=%s", __func__, __LINE__, name);
  module = mct_module_create(name);
  if(!module) {
    CDBG_ERROR("%s:%d failed.", __func__, __LINE__);
    return NULL;
  }
  ctrl = c2d_module_create_c2d_ctrl();
  if(!ctrl) {
    CDBG_ERROR("%s:%d failed", __func__, __LINE__);
    goto error_cleanup_module;
  }
  MCT_OBJECT_PRIVATE(module) = ctrl;
  ctrl->p_module = module;
  module->set_mod = c2d_module_set_mod;
  module->query_mod = c2d_module_query_mod;
  module->start_session = c2d_module_start_session;
  module->stop_session = c2d_module_stop_session;

  mct_port_t* port;
  int i;
  /* Create default ports */
  for(i=0; i < C2D_NUM_SOURCE_PORTS; i++) {
    port = c2d_port_create("c2d-src", MCT_PORT_SRC);
    if (NULL==port) {
      goto error_cleanup_module;
    }
    module->srcports = mct_list_append(module->srcports, port, NULL, NULL);
    module->numsrcports++;
    MCT_PORT_PARENT(port) = mct_list_append(MCT_PORT_PARENT(port), module,
                              NULL, NULL);
  }
  for(i=0; i < C2D_NUM_SINK_PORTS; i++) {
    port = c2d_port_create("c2d-sink", MCT_PORT_SINK);
    if (NULL==port) {
      goto error_cleanup_module;
    }
    module->sinkports = mct_list_append(module->sinkports, port, NULL, NULL);
    module->numsinkports++;
    MCT_PORT_PARENT(port) = mct_list_append(MCT_PORT_PARENT(port), module,
                              NULL, NULL);
  }
  CDBG_HIGH("%s:%d: info: C2D module_init successful", __func__, __LINE__);
  return module;

error_cleanup_module:
  mct_module_destroy(module);
  return NULL;
}

/** get_c2d_loglevel:
 *
 *  Args:
 *  Return:
 *    void
 **/

void get_c2d_loglevel()
{
  uint32_t temp;
  uint32_t log_level;
  uint32_t debug_mask;
  memset(c2d_prop, 0, sizeof(c2d_prop));
  /**  Higher 4 bits : Value of Debug log level (Default level is 1 to print all CDBG_HIGH)
       Lower 28 bits : Control mode for sub module logging(Only 3 sub modules in PPROC )
       0x1 for PPROC
       0x10 for C2D
       0x100 for CPP  */
  property_get("persist.camera.pproc.debug.mask", c2d_prop, "268435463"); // 0x10000007=268435463
  temp = atoi(c2d_prop);
  log_level = ((temp >> 28) & 0xF);
  debug_mask = (temp & PPROC_DEBUG_MASK_C2D);
  if (debug_mask > 0)
      gCamC2dLogLevel = log_level;
  else
      gCamC2dLogLevel = 0; // Debug logs are not required if debug_mask is zero
}

/** c2d_module_free_port
 *    @data: port object to free
 *    @user_data: should be NULL
 *
 *  To free a sink or source port.
 *
 *  Return TRUE on success.
 **/
static boolean c2d_module_free_port(void *data, void *user_data)
{
  CDBG("%s:%d] E\n", __func__, __LINE__);
  mct_port_t *port = MCT_PORT_CAST(data);
  mct_module_t *module = (mct_module_t *)user_data;

  if (!port) {
    CDBG_ERROR("%s:%d] error because list data is null\n", __func__,
      __LINE__);
    return FALSE;
  }
  if (!module ) {
    CDBG_ERROR("%s:%d] error because module is null\n", __func__,
      __LINE__);
    return FALSE;
  }

  if (strncmp(MCT_OBJECT_NAME(port), "c2d-src", strlen("c2d-src")) &&
      strncmp(MCT_OBJECT_NAME(port), "c2d-sink", strlen("c2d-sink"))) {
     CDBG_ERROR("%s:%d] error because port is invalid\n", __func__, __LINE__);
     return FALSE;
  }

  switch (MCT_PORT_DIRECTION(port)) {
    case MCT_PORT_SRC: {
      module->srcports = mct_list_remove(module->srcports, port);
      module->numsrcports--;
      break;
    }
    case MCT_PORT_SINK: {
      module->sinkports = mct_list_remove(module->sinkports, port);
      module->numsinkports--;
      break;
    }
    default:
      break;
  }

  MCT_PORT_PARENT(port) = mct_list_remove(MCT_PORT_PARENT(port), module);
  c2d_port_destroy(port);

  CDBG("%s:%d] X\n", __func__, __LINE__);
  return TRUE;
}


/** c2d_module_deinit:
 *
 *  Args:
 *    @module: pointer to c2d mct module
 *  Return:
 *    void
 **/
void c2d_module_deinit(mct_module_t *module)
{
  CDBG("%s E",__func__);
  c2d_module_ctrl_t *ctrl;
  int i = 0;
  int numsrcports = 0;
  int numsinkports = 0;
  if (!module || strcmp(MCT_OBJECT_NAME(module), "c2d")) {
    CDBG_ERROR("%s, Invalid module",__func__);
    return;
  }

  ctrl = MCT_OBJECT_PRIVATE(module);

  numsinkports = module->numsinkports;
  numsrcports = module->numsrcports;
  for (i = 0; i < numsinkports; i++) {
    c2d_module_free_port(MCT_MODULE_SINKPORTS(module)->data,module);
  }
  for (i = 0; i < numsrcports; i++) {
    c2d_module_free_port(MCT_MODULE_SRCPORTS(module)->data,module);
  }

  mct_list_free_list(MCT_MODULE_SRCPORTS(module));
  mct_list_free_list(MCT_MODULE_SINKPORTS(module));

  c2d_module_destroy_c2d_ctrl(ctrl);

  mct_module_destroy(module);
}

static c2d_module_ctrl_t* c2d_module_create_c2d_ctrl(void)
{
  c2d_module_ctrl_t *ctrl = NULL;
  mct_queue_t *q;
  int rc;
  ctrl = (c2d_module_ctrl_t *) malloc(sizeof(c2d_module_ctrl_t));
  if(!ctrl) {
    CDBG_ERROR("%s:%d, malloc failed", __func__, __LINE__);
    return NULL;
  }
  memset(ctrl, 0x00, sizeof(c2d_module_ctrl_t));

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

  /* Create PIPE for communication with c2d_thread */
  rc = pipe(ctrl->pfd);
  if ((ctrl->pfd[0]) >= MAX_FD_PER_PROCESS) {
    dump_list_of_daemon_fd();
    ctrl->pfd[0] = -1;
    rc = -1;
  }
  if(rc < 0) {
    CDBG_ERROR("%s:%d, pipe() failed", __func__, __LINE__);
    goto error_pipe;
  }
  pthread_cond_init(&(ctrl->th_start_cond), NULL);
  ctrl->session_count = 0;

  /* Create the C2D hardware instance */
  ctrl->c2dhw = c2d_hardware_create();
  if(ctrl->c2dhw == NULL) {
    CDBG_ERROR("%s:%d, failed, cannnot create c2d hardware instance\n",
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

static int32_t c2d_module_destroy_c2d_ctrl(c2d_module_ctrl_t *ctrl)
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
  c2d_hardware_destroy(ctrl->c2dhw);
  free(ctrl);
  return 0;
}

void c2d_module_set_mod(mct_module_t *module, unsigned int module_type,
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

boolean c2d_module_query_mod(mct_module_t *module, void *buf,
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

  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);

  /* TODO: Fill pp cap according to C2D HW caps*/
  c2d_hardware_cmd_t cmd;
  cmd.type = C2D_HW_CMD_GET_CAPABILITIES;
  rc = c2d_hardware_process_command(ctrl->c2dhw, cmd);
  if(rc < 0) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return FALSE;
  }
  /* TODO: Need a linking function to fill pp cap based on HW caps? */
  if (pp_cap->height_padding < CAM_PAD_TO_32) {
    pp_cap->height_padding = CAM_PAD_TO_32;
  }
  if (pp_cap->plane_padding < CAM_PAD_TO_32) {
    pp_cap->plane_padding = CAM_PAD_TO_32;
  }
  if (pp_cap->width_padding < CAM_PAD_TO_32) {
    pp_cap->width_padding = CAM_PAD_TO_32;
  }
  pp_cap->min_num_pp_bufs += MODULE_C2D_MIN_NUM_PP_BUFS;
  pp_cap->feature_mask |= (CAM_QCOM_FEATURE_CROP | CAM_QCOM_FEATURE_FLIP|
    CAM_QCOM_FEATURE_SCALE | CAM_QCOM_FEATURE_ROTATION);
  return TRUE;
}

boolean c2d_module_start_session(mct_module_t *module, unsigned int sessionid)
{
  int32_t rc;
  int32_t ion_fd;
  get_c2d_loglevel(); //dynamic logging level
  CDBG_HIGH("%s:%d, info: starting session %d", __func__, __LINE__, sessionid);
  if(!module) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return FALSE;
  }
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return FALSE;
  }
  if(ctrl->session_count >= C2D_MODULE_MAX_SESSIONS) {
    CDBG_ERROR("%s:%d, failed, too many sessions, count=%d",
      __func__, __LINE__, ctrl->session_count);
    return FALSE;
  }

  ion_fd = open("/dev/ion", O_RDWR|O_DSYNC);
  if (ion_fd >= MAX_FD_PER_PROCESS) {
    dump_list_of_daemon_fd();
    ion_fd = -1;
  }
  if (ion_fd < 0) {
    CDBG_ERROR("%s:%d, Cannot open ION ",__func__,__LINE__);
    ion_fd = 0;
  }

  /* create a new session specific params structure */
  int i;
  for(i=0; i < C2D_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i] == NULL) {
      ctrl->session_params[i] =
        (c2d_module_session_params_t*)
           malloc(sizeof(c2d_module_session_params_t));
      memset(ctrl->session_params[i], 0x00,
        sizeof(c2d_module_session_params_t));
      ctrl->session_params[i]->session_id = sessionid;
      ctrl->session_params[i]->frame_hold.is_frame_hold = FALSE;
      ctrl->session_params[i]->dis_hold.is_valid = FALSE;
      ctrl->session_params[i]->fps_range.max_fps = 30.0f;
      ctrl->session_params[i]->fps_range.min_fps = 30.0f;
      ctrl->session_params[i]->fps_range.video_max_fps= 30.0f;
      ctrl->session_params[i]->fps_range.video_min_fps= 30.0f;
      ctrl->session_params[i]->ion_fd = ion_fd;
      pthread_mutex_init(&(ctrl->session_params[i]->dis_mutex), NULL);
      break;
    }
  }

  /* start the thread only when first session starts */
  if(ctrl->session_count == 0) {
    /* spawn the c2d thread */
    rc = c2d_thread_create(module);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, c2d_thread_create() failed", __func__, __LINE__);
      return FALSE;
    }
    CDBG("%s:%d, info: c2d_thread created.", __func__, __LINE__);
    /* Create buffer manager instance */
    ctrl->buf_mgr = pp_buf_mgr_open();
    if (!ctrl->buf_mgr) {
      CDBG_ERROR("%s:%d, pp_buf_mgr_open() failed", __func__, __LINE__);
      return FALSE;
    }
    /* Create c2d instance */
    ctrl->c2d = pproc_library_init();
    if (!ctrl->c2d) {
      CDBG_ERROR("%s:%d, pproc_library_init() failed", __func__, __LINE__);
      return FALSE;
    }
    /* Open c2d instance */
    rc = ctrl->c2d->func_tbl->open(&ctrl->c2d_ctrl);
    if (rc < 0) {
      CDBG_ERROR("%s:%d, ctrl->c2d->func_tbl->open() failed", __func__,
        __LINE__);
      return FALSE;
    }
  }
  ctrl->session_count++;
  CDBG_HIGH("%s:%d, info: session %d started.", __func__, __LINE__, sessionid);
  return TRUE;
}

boolean c2d_module_stop_session(mct_module_t *module, unsigned int sessionid)
{
  int32_t rc;
  if(!module) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return FALSE;
  }
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return FALSE;
  }
  CDBG_HIGH("%s:%d, info: stopping session %d ...", __func__, __LINE__, sessionid);
  ctrl->session_count--;
  /* stop the thread only when last session terminates */
  if(ctrl->session_count == 0) {
    /* stop the C2D thread */
    CDBG("%s:%d, info: stopping c2d_thread...", __func__, __LINE__);
    c2d_thread_msg_t msg;
    msg.type = C2D_THREAD_MSG_ABORT;
    rc = c2d_module_post_msg_to_thread(module, msg);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, c2d_module_post_msg_to_thread() failed",
        __func__, __LINE__);
      return FALSE;
    }
    /* wait for thread completion */
    pthread_join(ctrl->c2d_thread, NULL);
    /* close the c2d hardware */
    CDBG("%s:%d, closing c2d subdev...", __func__, __LINE__);
    /* Close buffer manager instance */
    pp_buf_mgr_close(ctrl->buf_mgr);
    /* Close c2d instance */
    rc = ctrl->c2d->func_tbl->close(ctrl->c2d_ctrl);
    if (rc < 0) {
      CDBG_ERROR("%s:%d, ctrl->c2d->func_tbl->close() failed", __func__,
        __LINE__);
      return FALSE;
    }
  }
  /* remove the session specific params */
  int i;
  for(i=0; i < C2D_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i]) {
      if(ctrl->session_params[i]->session_id == sessionid) {
        pthread_mutex_destroy(
                &(ctrl->session_params[i]->dis_mutex));
        if (ctrl->session_params[i]->ion_fd) {
          close(ctrl->session_params[i]->ion_fd);
        }
        free(ctrl->session_params[i]);
        ctrl->session_params[i] = NULL;
        break;
      }
    }
  }
  CDBG_HIGH("%s:%d, info: session %d stopped.", __func__, __LINE__, sessionid);
  return TRUE;
}

/* c2d_module_post_msg_to_thread:
 *
 * @module: c2d module pointer
 * @msg: message to be posted for thread
 * Description:
 *  Writes message to the pipe for which the c2d_thread is listening to.
 *
 **/
int32_t c2d_module_post_msg_to_thread(mct_module_t *module,
  c2d_thread_msg_t msg)
{
  int32_t rc;
  if(!module) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EINVAL;
  }
  CDBG_LOW("%s:%d, msg.type=%d", __func__, __LINE__, msg.type);
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  rc = write(ctrl->pfd[WRITE_FD], &msg, sizeof(c2d_thread_msg_t));
  if(rc < 0) {
    CDBG_ERROR("%s:%d, write() failed\n", __func__, __LINE__);
    return -EIO;
  }
  return 0;
}

/* c2d_module_enq_event:
 *
 * @module: c2d module pointer
 * @event:  c2d_event to be queued
 * @prio:   priority of the event(realtime/offline)
 *
 * Description:
 *  Enqueues a c2d_event into realtime or offline queue based on the
 *  priority.
 *
 **/
int32_t c2d_module_enq_event(mct_module_t* module,
  c2d_module_event_t* c2d_event, c2d_priority_t prio)
{
  if(!module || !c2d_event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p", __func__, __LINE__,
      module, c2d_event);
    return -EINVAL;
  }
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);

  CDBG_LOW("%s:%d, prio=%d", __func__, __LINE__, prio);
  switch (prio) {
  case C2D_PRIORITY_REALTIME:
    PTHREAD_MUTEX_LOCK(&(ctrl->realtime_queue.mutex));
    mct_queue_push_tail(ctrl->realtime_queue.q, c2d_event);
    CDBG_LOW("%s:%d, real-time queue size = %d", __func__, __LINE__,
      ctrl->realtime_queue.q->length);
    PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
    break;
  case C2D_PRIORITY_OFFLINE:
    PTHREAD_MUTEX_LOCK(&(ctrl->offline_queue.mutex));
    mct_queue_push_tail(ctrl->offline_queue.q, c2d_event);
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

int32_t c2d_module_send_event_downstream(mct_module_t* module,
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
  mct_port_t* port = c2d_module_find_port_with_identity(module, MCT_PORT_SRC,
                       identity);
  if(!port) {
    CDBG_LOW("%s:%d, info: no source port found.with identity=0x%x",
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

/* c2d_module_send_event_upstream:
 *
 * Description:
 *  Sends event to the upstream peer based on the event identity.
 *
 **/
int32_t c2d_module_send_event_upstream(mct_module_t* module,
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
  mct_port_t* port = c2d_module_find_port_with_identity(module, MCT_PORT_SINK,
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

/* c2d_module_invalidate_queue:
 *
 **/
int32_t c2d_module_invalidate_queue(c2d_module_ctrl_t* ctrl,
  uint32_t identity)
{
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  /* TODO: currently only realtime q used, to be extended for offline case */
  c2d_module_event_queue_t *queue;
  queue = &(ctrl->realtime_queue);
  void*  input[2];
  input[0] = (uint32_t*)ctrl;
  input[1] = &identity;
  PTHREAD_MUTEX_LOCK(&(queue->mutex));
  mct_queue_traverse(queue->q, c2d_module_invalidate_q_traverse_func,
    input);
  PTHREAD_MUTEX_UNLOCK(&(queue->mutex));
  return 0;
}

/* c2d_module_send_buf_divert_ack:
 *
 *  Sends a buf_divert_ack to upstream module.
 *
 **/
static int32_t c2d_module_send_buf_divert_ack(c2d_module_ctrl_t *ctrl,
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
  rc = c2d_module_send_event_upstream(ctrl->p_module, &event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  return 0;
}

/* c2d_module_do_ack:
 *
 *  Decrements the refcount of the ACK which is stored in the ack_list,
 *  correspoding to the key. If the refcount becomes 0, a buf_divert_ack
 *  is sent upstream. At this time the ack entry is removed from list.
 *
 **/
int32_t c2d_module_do_ack(c2d_module_ctrl_t *ctrl,
  c2d_module_ack_key_t key)
{
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EINVAL;
  }
  /* find corresponding ack from the list. If the all references
     to that ack are done, send the ack and remove the entry from the list */
  c2d_module_ack_t *c2d_ack;
  CDBG("%s:%d, buf_idx=%d, identity=0x%x", __func__, __LINE__,
    key.buf_idx, key.identity);
  PTHREAD_MUTEX_LOCK(&(ctrl->ack_list.mutex));
  c2d_ack = c2d_module_find_ack_from_list(ctrl, key);
  if(!c2d_ack) {
    CDBG_ERROR("%s:%d, failed, ack not found in list, for buf_idx=%d, "
      "identity=0x%x", __func__, __LINE__, key.buf_idx, key.identity);
    PTHREAD_MUTEX_UNLOCK(&(ctrl->ack_list.mutex));
    return -EFAULT;
  }
  c2d_ack->ref_count--;
  CDBG("%s:%d, c2d_ack->ref_count=%d\n", __func__, __LINE__,
    c2d_ack->ref_count);
  struct timeval tv;
  if(c2d_ack->ref_count == 0) {
    ctrl->ack_list.list = mct_list_remove(ctrl->ack_list.list, c2d_ack);
    ctrl->ack_list.size--;
    /* unlock before sending event to prevent any deadlock */
    PTHREAD_MUTEX_UNLOCK(&(ctrl->ack_list.mutex));
    gettimeofday(&(c2d_ack->out_time), NULL);
    CDBG_LOW("%s:%d, in_time=%ld.%ld us, out_time=%ld.%ld us, ",
      __func__, __LINE__, c2d_ack->in_time.tv_sec, c2d_ack->in_time.tv_usec,
      c2d_ack->out_time.tv_sec, c2d_ack->out_time.tv_usec);
    CDBG("%s:%d, holding time = %6ld us,frame_id=%d,buf_idx=%d,identity=%d",
      __func__, __LINE__,
      (c2d_ack->out_time.tv_sec - c2d_ack->in_time.tv_sec)*1000000L +
      (c2d_ack->out_time.tv_usec - c2d_ack->in_time.tv_usec),
      c2d_ack->isp_buf_divert_ack.frame_id,
      c2d_ack->isp_buf_divert_ack.buf_idx,
      key.identity);
    c2d_module_send_buf_divert_ack(ctrl, c2d_ack->isp_buf_divert_ack);
    gettimeofday(&tv, NULL);
    CDBG_LOW("%s:%d, upstream event time = %6ld us, ", __func__, __LINE__,
      (tv.tv_sec - c2d_ack->out_time.tv_sec)*1000000L +
      (tv.tv_usec - c2d_ack->out_time.tv_usec));
    free(c2d_ack);
  } else {
    PTHREAD_MUTEX_UNLOCK(&(ctrl->ack_list.mutex));
  }
  return 0;
}

/* c2d_module_handle_ack_from_downstream:
 *
 *  Handles the buf_divert_ack event coming from downstream module.
 *  Corresponding ACK stored in ack_list is updated and/or released
 *  accordingly.
 *
 */
static int32_t c2d_module_handle_ack_from_downstream(mct_module_t* module,
  mct_event_t* event)
{
  int32_t                      rc = 0;
  c2d_module_stream_params_t  *stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  boolean                      bool_ret = 0;

  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EINVAL;
  }

  c2d_module_get_params_for_identity(ctrl, event->identity, &session_params,
     &stream_params);
  if (!session_params || !stream_params) {
    CDBG_ERROR("%s:%d: failed params %p %p\n", __func__, __LINE__,
      session_params, stream_params);
    return -EFAULT;
  }

  isp_buf_divert_ack_t* isp_buf_ack =
    (isp_buf_divert_ack_t*)(event->u.module_event.module_event_data);

  if (((stream_params->stream_info->is_type == IS_TYPE_EIS_2_0 ||
    stream_params->interleaved) &&
    !stream_params->single_module) ||
    stream_params->hw_params.processed_divert) {
  /* Put acked buffer */
    bool_ret = pp_buf_mgr_put_buf(ctrl->buf_mgr,isp_buf_ack->identity,
      isp_buf_ack->buf_idx, isp_buf_ack->frame_id, isp_buf_ack->timestamp);
    if (bool_ret == FALSE) {
      CDBG_ERROR("%s:%d pp_buf_mgr_put_buf idx %d frame id %d\n", __func__,
        __LINE__, isp_buf_ack->buf_idx, isp_buf_ack->frame_id);
      rc = -EINVAL;
    }
  } else {
    c2d_module_ack_key_t key;
    key.identity = isp_buf_ack->identity;
    key.buf_idx = isp_buf_ack->buf_idx;
    CDBG("%s:%d, doing ack for divert_done ack from downstream",
      __func__, __LINE__);
    c2d_module_do_ack(ctrl, key);
  }

  return rc;
}

/* c2d_module_put_new_ack_in_list:
 *
 * Description:
 *   Adds a new ACK in the ack_list with the given params.
 **/
int32_t c2d_module_put_new_ack_in_list(c2d_module_ctrl_t *ctrl,
  c2d_module_ack_key_t key, int32_t buf_dirty, int32_t ref_count,
  isp_buf_divert_t *isp_buf)
{
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  /* this memory will be freed by thread when ack is removed from list */
  c2d_module_ack_t *c2d_ack =
    (c2d_module_ack_t *) malloc (sizeof(c2d_module_ack_t));
  if(!c2d_ack) {
    CDBG_ERROR("%s:%d, malloc failed\n", __func__, __LINE__);
    return -ENOMEM;
  }
  memset(c2d_ack, 0x00, sizeof(c2d_module_ack_t));
  c2d_ack->isp_buf_divert_ack.identity = isp_buf->identity;
  c2d_ack->isp_buf_divert_ack.buf_idx = key.buf_idx;
  c2d_ack->isp_buf_divert_ack.is_buf_dirty = buf_dirty;
  c2d_ack->isp_buf_divert_ack.channel_id = key.channel_id;
  c2d_ack->isp_buf_divert_ack.frame_id = isp_buf->buffer.sequence;
  c2d_ack->isp_buf_divert_ack.timestamp = isp_buf->buffer.timestamp;
  c2d_ack->isp_buf_divert_ack.meta_data = key.meta_data;
  c2d_ack->ref_count = ref_count;
  c2d_ack->isp_buf_divert_ack.handle = isp_buf->handle;
  c2d_ack->isp_buf_divert_ack.output_format = isp_buf->output_format;
  c2d_ack->isp_buf_divert_ack.input_intf = isp_buf->input_intf;
  c2d_ack->isp_buf_divert_ack.is_skip_pproc = isp_buf->is_skip_pproc;
  CDBG("%s:%d, adding ack in list, identity=0x%x", __func__, __LINE__,
    c2d_ack->isp_buf_divert_ack.identity);
  CDBG("%s:%d, buf_idx=%d, ref_count=%d", __func__, __LINE__,
    c2d_ack->isp_buf_divert_ack.buf_idx, c2d_ack->ref_count);
  PTHREAD_MUTEX_LOCK(&(ctrl->ack_list.mutex));
  gettimeofday(&(c2d_ack->in_time), NULL);
  ctrl->ack_list.list = mct_list_append(ctrl->ack_list.list,
                          c2d_ack, NULL, NULL);
  ctrl->ack_list.size++;
  PTHREAD_MUTEX_UNLOCK(&(ctrl->ack_list.mutex));
  return 0;
}

int32_t c2d_module_process_downstream_event(mct_module_t* module,
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
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  /* handle events based on type, if not handled, forward it downstream */
  switch(event->type) {
  case MCT_EVENT_MODULE_EVENT: {
    switch(event->u.module_event.type) {
    case MCT_EVENT_MODULE_BUF_DIVERT:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_BUF_DIVERT: identity=0x%x", __func__,
        __LINE__, identity);
      rc = c2d_module_handle_buf_divert_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_ISP_OUTPUT_DIM:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_ISP_OUTPUT_DIM: identity=0x%x", __func__,
        __LINE__, identity);
      rc = c2d_module_handle_isp_out_dim_event(module, event);
      if (rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        //isp output dim should be passed to downstream no matter what
        rc = c2d_module_send_event_downstream(module, event);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_STREAM_CROP:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_STREAM_CROP: identity=0x%x", __func__,
        __LINE__, identity);
      rc = c2d_module_handle_stream_crop_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_STATS_DIS_UPDATE:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_STATS_DIS_UPDATE: identity=0x%x", __func__,
        __LINE__, identity);
      rc = c2d_module_handle_dis_update_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_SET_STREAM_CONFIG:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_SET_STREAM_CONFIG: identity=0x%x", __func__,
        __LINE__, identity);
      rc = c2d_module_handle_stream_cfg_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_SENSOR_UPDATE_FPS:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_SENSOR_UPDATE_FPS: identity=0x%x", __func__,
        __LINE__, identity);
      rc = c2d_module_handle_fps_update_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_PPROC_DIVERT_INFO:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_PPROC_DIVERT_INFO: identity=0x%x", __func__,
        __LINE__, identity);
      rc = c2d_module_handle_div_info_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    default:
      rc = c2d_module_send_event_downstream(module, event);
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
      rc = c2d_module_handle_streamon_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, streamon failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    }
    case MCT_EVENT_CONTROL_STREAMOFF: {
      rc = c2d_module_handle_streamoff_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, streamoff failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    }
    case MCT_EVENT_CONTROL_SET_PARM: {
      rc = c2d_module_handle_set_parm_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    }
    default:
      rc = c2d_module_send_event_downstream(module, event);
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

int32_t c2d_module_process_upstream_event(mct_module_t* module,
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
    case MCT_EVENT_MODULE_BUF_DIVERT_ACK: {
      CDBG("%s:%d: MCT_EVENT_MODULE_BUF_DIVERT_ACK: identity=0x%x", __func__,
        __LINE__, identity);
      c2d_module_stream_params_t  *stream_params = NULL;
      c2d_module_session_params_t *session_params = NULL;
      c2d_module_ctrl_t* ctrl = (c2d_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
      if(!ctrl) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return -EINVAL;
      }

      c2d_module_get_params_for_identity(ctrl, event->identity, &session_params,
         &stream_params);
      if (!session_params || !stream_params) {
        CDBG_ERROR("%s:%d: failed params %p %p\n", __func__, __LINE__,
          session_params, stream_params);
        return -EFAULT;
      }

      rc = c2d_module_handle_ack_from_downstream(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
      }
      if ((stream_params->stream_info->is_type == IS_TYPE_EIS_2_0 ||
        stream_params->interleaved) &&
        !stream_params->single_module) {
        return rc;
      }
      break;
    }
    default:
      break;
    }
    break;
  }
  default:
    /* all upstream events are module events */
    break;
  }
  rc = c2d_module_send_event_upstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return rc;
  }
  return 0;
}

/* c2d_module_get_input_format
 *
 *  @ peer_port_caps - port capabilities of the peer module
 *  @ stream_info - stream info snet during link modules
 *
 *  Return the format of the input frame, based on the peer source port format
 *
 *  Return value is input frame format flag.
 */
cam_format_t c2d_module_get_input_format(mct_port_caps_t *peer_port_caps,
  mct_stream_info_t* stream_info) {

  if (peer_port_caps->u.frame.format_flag & MCT_PORT_CAP_FORMAT_YCBYCR) {
    return CAM_FORMAT_YUV_RAW_8BIT_YUYV;
  } else if (peer_port_caps->u.frame.format_flag & MCT_PORT_CAP_FORMAT_YCRYCB) {
    return CAM_FORMAT_YUV_RAW_8BIT_YVYU;
  } else if (peer_port_caps->u.frame.format_flag & MCT_PORT_CAP_FORMAT_CBYCRY) {
    return CAM_FORMAT_YUV_RAW_8BIT_UYVY;
  } else if (peer_port_caps->u.frame.format_flag & MCT_PORT_CAP_FORMAT_CRYCBY) {
    return CAM_FORMAT_YUV_RAW_8BIT_VYUY;
  } else {
    return stream_info->fmt;
  }
}

/* c2d_module_notify_add_stream:
 *
 * creates and initializes the stream-specific paramater structures when a
 * stream is reserved in port
 **/
int32_t c2d_module_notify_add_stream(mct_module_t* module, mct_port_t* port,
  mct_stream_info_t* stream_info, void *peer_caps)
{
  int rc;
  cam_pp_feature_config_t *pp_config;
  if(!module || !stream_info || !port || !peer_caps) {
    CDBG_ERROR("%s:%d, failed, module=%p, port=%p, stream_info=%p peers %p\n", __func__,
      __LINE__, module, port, stream_info, peer_caps);
    return -EINVAL;
  }
  mct_port_caps_t *peer_port_caps = (mct_port_caps_t *)peer_caps;
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed, module=%p\n", __func__, __LINE__, module);
    return -EINVAL;
  }
  uint32_t identity = stream_info->identity;
  /* create stream specific params structure */
  uint32_t session_id;
  int i,j;
  boolean success = FALSE;
  c2d_hardware_params_t *hw_params;
  session_id = C2D_GET_SESSION_ID(identity);
  CDBG("%s:%d: identity=0x%x, stream type=%d\n",
    __func__, __LINE__, identity,stream_info->stream_type);

  /* find if a stream is already added on this port. If yes, we need to link
     that stream with this. (only for continuous streams)*/
  c2d_module_session_params_t *linked_session_params = NULL;
  c2d_module_stream_params_t *linked_stream_params = NULL;
  uint32_t linked_identity;
  int32_t found = c2d_port_get_linked_identity(port, identity,
    &linked_identity);
  if (found > 0) {
    CDBG("%s:%d, found linked identity=0x%x", __func__, __LINE__,
      linked_identity);
    c2d_module_get_params_for_identity(ctrl,linked_identity,
      &linked_session_params, &linked_stream_params);
    if (!linked_stream_params) {
      CDBG_ERROR("%s:%d, failed, module=%p\n", __func__, __LINE__, module);
      return -EINVAL;
    }
  }

  for(i=0; i < C2D_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i]) {
      if(ctrl->session_params[i]->session_id == session_id) {
        for(j=0; j < C2D_MODULE_MAX_STREAMS; j++) {
          if(ctrl->session_params[i]->stream_params[j] == NULL) {
            ctrl->session_params[i]->stream_params[j] =
              (c2d_module_stream_params_t *)
                 malloc (sizeof(c2d_module_stream_params_t));
            memset(ctrl->session_params[i]->stream_params[j], 0x00,
              sizeof(c2d_module_stream_params_t));
            ctrl->session_params[i]->stream_params[j]->identity = identity;

            /* assign priority */
            if(stream_info->streaming_mode == CAM_STREAMING_MODE_CONTINUOUS) {
              ctrl->session_params[i]->stream_params[j]->priority =
                C2D_PRIORITY_REALTIME;
            } else {
              ctrl->session_params[i]->stream_params[j]->priority =
                C2D_PRIORITY_REALTIME;
            }
            /* initialize input/output fps values */
            /* initialize input/output fps values */
            if(stream_info->stream_type == CAM_STREAM_TYPE_VIDEO)
            {
                ctrl->session_params[i]->stream_params[j]->
                  hfr_skip_info.input_fps =
                  ctrl->session_params[i]->fps_range.video_max_fps;
                ctrl->session_params[i]->stream_params[j]->
                  hfr_skip_info.output_fps =
                  ctrl->session_params[i]->fps_range.video_max_fps;
            }
            else
            {
                ctrl->session_params[i]->stream_params[j]->
                  hfr_skip_info.input_fps =
                  ctrl->session_params[i]->fps_range.max_fps;
                ctrl->session_params[i]->stream_params[j]->
                  hfr_skip_info.output_fps =
                  ctrl->session_params[i]->fps_range.max_fps;
            }

            CDBG("%s:%d input_fps=%.2f, output_fps %f identity=0x%x",
              __func__, __LINE__,
              ctrl->session_params[i]->stream_params[j]->hfr_skip_info.input_fps,
              ctrl->session_params[i]->stream_params[j]->hfr_skip_info.output_fps,
              ctrl->session_params[i]->stream_params[j]->identity);

            ctrl->session_params[i]->stream_params[j]->
              hfr_skip_info.skip_count = 0;
            /* hfr_skip_required in only in preview stream */
            ctrl->session_params[i]->stream_params[j]->
              hfr_skip_info.skip_required =
                ((stream_info->stream_type == CAM_STREAM_TYPE_PREVIEW) ||
                (stream_info->stream_type == CAM_STREAM_TYPE_VIDEO))?
                  TRUE : FALSE;

            /* init divert information */
            ctrl->session_params[i]->stream_params[j]->div_config = NULL;

            /* assign stream type */
            ctrl->session_params[i]->stream_params[j]->stream_type =
              stream_info->stream_type;
            /* set interleaved */
            if (peer_port_caps->u.frame.format_flag & MCT_PORT_CAP_INTERLEAVED) {
              ctrl->session_params[i]->stream_params[j]->interleaved = 1;
            } else {
              ctrl->session_params[i]->stream_params[j]->interleaved = 0;
            }

            if (MCT_MODULE_FLAG_SINK == mct_module_find_type(module,identity)) {
              ctrl->session_params[i]->stream_params[j]->single_module = TRUE;
            }
            else {
              ctrl->session_params[i]->stream_params[j]->single_module = FALSE;
            }

            hw_params = &ctrl->session_params[i]->stream_params[j]->hw_params;
            /* output dimensions */
            hw_params->output_info.width = stream_info->dim.width;
            hw_params->output_info.height = stream_info->dim.height;
            if ((stream_info->is_type == IS_TYPE_EIS_2_0 ||
                ctrl->session_params[i]->stream_params[j]->interleaved) &&
                !ctrl->session_params[i]->stream_params[j]->single_module) {
              hw_params->output_info.stride =
                  stream_info->dim.width;
              hw_params->output_info.scanline =
                  stream_info->dim.height;
            } else {
              hw_params->output_info.stride =
                  stream_info->buf_planes.plane_info.mp[0].stride;
              hw_params->output_info.scanline =
                  stream_info->buf_planes.plane_info.mp[0].scanline;
            }

            /* rotation/flip */
            if (stream_info->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
                pp_config = &stream_info->reprocess_config.pp_feature_config;
            } else {
                pp_config = &stream_info->pp_config;
            }
            hw_params->mirror = pp_config->flip;
             CDBG("%s:%d, Rotation=%d", __func__, __LINE__,
              pp_config->rotation);
            if (pp_config->rotation == ROTATE_0) {
                hw_params->rotation = 0;
            } else if (pp_config->rotation == ROTATE_90) {
                hw_params->rotation = 1;
            } else if (pp_config->rotation == ROTATE_180) {
                 hw_params->rotation = 2;
            } else if (pp_config->rotation == ROTATE_270) {
                 hw_params->rotation = 3;
            }

            if ((stream_info->is_type == IS_TYPE_EIS_2_0 ||
              ctrl->session_params[i]->stream_params[j]->interleaved) &&
              !ctrl->session_params[i]->stream_params[j]->single_module) {
              if ((hw_params->rotation == 1) || (hw_params->rotation == 3)) {
                uint32_t temp_width;
                temp_width = hw_params->output_info.width;
                hw_params->output_info.width = hw_params->output_info.height;
                hw_params->output_info.height = temp_width;
                temp_width = hw_params->output_info.stride;
                hw_params->output_info.stride = hw_params->output_info.scanline;
                hw_params->output_info.scanline = temp_width;
              }
              hw_params->rotation = 0;
            }
            /* format info */
            if (stream_info->fmt == CAM_FORMAT_YUV_420_NV12 ||
                stream_info->fmt == CAM_FORMAT_YUV_420_NV12_VENUS) {
              hw_params->output_info.c2d_plane_fmt = C2D_PARAM_PLANE_CBCR;
            } else if (stream_info->fmt == CAM_FORMAT_YUV_420_NV21) {
              hw_params->output_info.c2d_plane_fmt = C2D_PARAM_PLANE_CRCB;
            } else if (stream_info->fmt == CAM_FORMAT_YUV_422_NV16) {
              hw_params->output_info.c2d_plane_fmt = C2D_PARAM_PLANE_CBCR422;
            } else if (stream_info->fmt == CAM_FORMAT_YUV_422_NV61) {
              hw_params->output_info.c2d_plane_fmt = C2D_PARAM_PLANE_CRCB422;
            } else if (stream_info->fmt == CAM_FORMAT_YUV_420_YV12) {
              hw_params->output_info.c2d_plane_fmt = C2D_PARAM_PLANE_CRCB420;
            } else {
              CDBG_ERROR("%s:%d, failed. Format not supported\n", __func__,
                __LINE__);
              return -EINVAL;
            }
            hw_params->output_info.cam_fmt = stream_info->fmt;
            c2d_divert_info_t *div_config = NULL;
           if (!linked_stream_params) {
             ctrl->session_params[i]->stream_params[j]->c2d_input_lib_params.format =
                 c2d_module_get_input_format(peer_port_caps,stream_info);
             ctrl->session_params[i]->stream_params[j]->c2d_input_lib_params.surface_bit = C2D_SOURCE;
             rc = ctrl->c2d->func_tbl->process(ctrl->c2d_ctrl,
             PPROC_IFACE_CREATE_SURFACE,
             &ctrl->session_params[i]->stream_params[j]->c2d_input_lib_params);
             if (rc < 0) {
               CDBG_ERROR("%s:%d failed", __func__, __LINE__);
               return rc;
             }
           } else {
             if ((linked_stream_params->stream_info->dim.width *
                 linked_stream_params->stream_info->dim.height <
                 stream_info->dim.width* stream_info->dim.height)) {
                 rc = ctrl->c2d->func_tbl->process(ctrl->c2d_ctrl,
                 PPROC_IFACE_DESTROY_SURFACE,
                 &linked_stream_params->c2d_input_lib_params.id);

                 ctrl->session_params[i]->stream_params[j]->c2d_input_lib_params.format =
                     c2d_module_get_input_format(peer_port_caps,stream_info);
                 ctrl->session_params[i]->stream_params[j]->c2d_input_lib_params.surface_bit = C2D_SOURCE;
                 rc = ctrl->c2d->func_tbl->process(ctrl->c2d_ctrl,
                 PPROC_IFACE_CREATE_SURFACE,
                 &ctrl->session_params[i]->stream_params[j]->c2d_input_lib_params);
                 if (rc < 0) {
                   CDBG_ERROR("%s:%d failed", __func__, __LINE__);
                   return rc;
                 }
                 memcpy(
                   &linked_stream_params->c2d_input_lib_params,
                   &ctrl->session_params[i]->stream_params[j]->c2d_input_lib_params,
                   sizeof (c2d_libparams));
            } else {
                memcpy(
                 &ctrl->session_params[i]->stream_params[j]->c2d_input_lib_params,
                 &linked_stream_params->c2d_input_lib_params,
                 sizeof (c2d_libparams));
             }
          }
          if (stream_info->is_type == IS_TYPE_EIS_2_0) {
            ctrl->session_params[i]->stream_params[j]->c2d_output_lib_params.format =
              ctrl->session_params[i]->stream_params[j]->c2d_input_lib_params.format;
          } else {
            ctrl->session_params[i]->stream_params[j]->c2d_output_lib_params.format = stream_info->fmt;
          }
          ctrl->session_params[i]->stream_params[j]->c2d_output_lib_params.surface_bit = C2D_TARGET;
          rc = ctrl->c2d->func_tbl->process(ctrl->c2d_ctrl,
            PPROC_IFACE_CREATE_SURFACE,
            &ctrl->session_params[i]->stream_params[j]->c2d_output_lib_params);
           if (rc < 0) {
             CDBG_ERROR("%s:%d failed", __func__, __LINE__);
             return rc;
           }
            /* set linked stream */
            ctrl->session_params[i]->stream_params[j]->
              linked_stream_params = NULL;
            if (linked_stream_params) {
              ctrl->session_params[i]->stream_params[j]->
                linked_stream_params = linked_stream_params;
              linked_stream_params->linked_stream_params =
                ctrl->session_params[i]->stream_params[j];
              div_config = linked_stream_params->div_config;
            } else {
              div_config = malloc(sizeof(c2d_divert_info_t));
              if(!div_config) {
                CDBG_ERROR("%s:%d, malloc failed\n", __func__, __LINE__);
                return -ENOMEM;
              }
              memset(div_config, 0, sizeof(c2d_divert_info_t));
              div_config->identity[0] = PPROC_INVALID_IDENTITY;
              div_config->identity[1] = PPROC_INVALID_IDENTITY;
            }

            /* Now attach the identity to divert config table */
            c2d_module_set_divert_cfg_identity(PPROC_INVALID_IDENTITY,
              identity, div_config);
            ctrl->session_params[i]->stream_params[j]->div_config =
              div_config;
            /* initialize the mutex for stream_params */
            pthread_mutex_init(
              &(ctrl->session_params[i]->stream_params[j]->mutex), NULL);
            /* Initialize stream info */
            ctrl->session_params[i]->stream_params[j]->stream_info =
              stream_info;
            ctrl->session_params[i]->stream_count++;
            success = TRUE;
            c2d_module_dump_stream_params(
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

/* c2d_module_notify_remove_stream:
 *
 *  destroys stream-specific data structures when a stream is unreserved
 *  in port
 **/
int32_t c2d_module_notify_remove_stream(mct_module_t* module, uint32_t identity)
{
  if(!module) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  CDBG("%s:%d: identity=0x%x\n", __func__, __LINE__, identity);
  /* destroy stream specific params structure */
  uint32_t session_id;
  int i,j;
  boolean success = FALSE;
  session_id = C2D_GET_SESSION_ID(identity);
  for(i=0; i < C2D_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i]) {
      if(ctrl->session_params[i]->session_id == session_id) {
        for(j=0; j < C2D_MODULE_MAX_STREAMS; j++) {
          if(ctrl->session_params[i]->stream_params[j]) {
            if(ctrl->session_params[i]->stream_params[j]->identity ==
                identity) {
              /* Now detach the identity from divert config table */
              c2d_module_set_divert_cfg_identity(identity,
                PPROC_INVALID_IDENTITY,
                ctrl->session_params[i]->stream_params[j]->div_config);
            if (!ctrl->session_params[i]->stream_params[j]->linked_stream_params)
               ctrl->c2d->func_tbl->process(ctrl->c2d_ctrl,
               PPROC_IFACE_DESTROY_SURFACE,
               &ctrl->session_params[i]->stream_params[j]->c2d_input_lib_params.id);

            ctrl->c2d->func_tbl->process(ctrl->c2d_ctrl,
                PPROC_IFACE_DESTROY_SURFACE,
                &ctrl->session_params[i]->stream_params[j]->c2d_output_lib_params.id);
              /* remove linked params */
              if (ctrl->session_params[i]->stream_params[j]->
                  linked_stream_params) {
                ctrl->session_params[i]->stream_params[j]->
                  linked_stream_params->linked_stream_params = NULL;
                ctrl->session_params[i]->stream_params[j]->
                  linked_stream_params = NULL;
              } else {
                free(ctrl->session_params[i]->stream_params[j]->div_config);
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
