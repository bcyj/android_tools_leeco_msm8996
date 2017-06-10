/*============================================================================

  Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "eztune_diagnostics.h"
#include "cpp_module.h"
#include "cpp_port.h"
#include "cpp_log.h"
#include "cpp_module_events.h"
#include <cutils/properties.h>
#include "server_debug.h"

#define CPP_PORT_NAME_LEN     32
#define CPP_NUM_SINK_PORTS    8
#define CPP_NUM_SOURCE_PORTS  8
#define MINIMUM_PROCESS_TIME 10.0f
#define PADDING_FACTOR       1.2f
#define MINIMUM_CPP_THROUGHPUT      1.2f
#define MAXIMUM_CPP_THROUGHPUT      2

#ifndef MAX
#define MAX(x,y) (((x)>(y)) ? (x) : (y))
#endif

static const char cpp_sink_port_name[CPP_NUM_SINK_PORTS][CPP_PORT_NAME_LEN] = {
 "cpp_sink_0",
 "cpp_sink_1",
 "cpp_sink_2",
 "cpp_sink_3",
 "cpp_sink_4",
 "cpp_sink_5",
 "cpp_sink_6",
 "cpp_sink_7",
};

static const char cpp_src_port_name[CPP_NUM_SOURCE_PORTS][CPP_PORT_NAME_LEN] = {
 "cpp_src_0",
 "cpp_src_1",
 "cpp_src_2",
 "cpp_src_3",
 "cpp_src_4",
 "cpp_src_5",
 "cpp_src_6",
 "cpp_src_7",
};

volatile uint32_t gCamCppLogLevel = 0;
char cpp_prop[PROPERTY_VALUE_MAX];

/** cpp_module_init:
 *  Args:
 *    @name: module name
 *  Return:
 *    - mct_module_t pointer corresponding to cpp on SUCCESS
 *    - NULL in case of FAILURE or if CPP hardware does not
 *      exist
 **/
mct_module_t *cpp_module_init(const char *name)
{
  mct_module_t *module;
  cpp_module_ctrl_t* ctrl;
  CDBG_HIGH("%s:%d name=%s", __func__, __LINE__, name);
  module = mct_module_create(name);
  if(!module) {
    CDBG_ERROR("%s:%d failed.", __func__, __LINE__);
    return NULL;
  }
  ctrl = cpp_module_create_cpp_ctrl();
  if(!ctrl) {
    CDBG_ERROR("%s:%d failed", __func__, __LINE__);
    goto error_cleanup_module;
  }
  MCT_OBJECT_PRIVATE(module) = ctrl;
  ctrl->p_module = module;
  module->set_mod = cpp_module_set_mod;
  module->query_mod = cpp_module_query_mod;
  module->start_session = cpp_module_start_session;
  module->stop_session = cpp_module_stop_session;

  mct_port_t* port;
  int i;
  /* Create default ports */
  for(i=0; i < CPP_NUM_SOURCE_PORTS; i++) {
    port = cpp_port_create(cpp_src_port_name[i], MCT_PORT_SRC);
    if(!port) {
      CDBG_ERROR("%s:%d failed.", __func__, __LINE__);
      return NULL;
    }
    module->srcports = mct_list_append(module->srcports, port, NULL, NULL);
    module->numsrcports++;
    MCT_PORT_PARENT(port) = mct_list_append(MCT_PORT_PARENT(port), module,
                              NULL, NULL);
  }
  for(i=0; i < CPP_NUM_SINK_PORTS; i++) {
    port = cpp_port_create(cpp_sink_port_name[i], MCT_PORT_SINK);
    if(!port) {
      CDBG_ERROR("%s:%d failed.", __func__, __LINE__);
      return NULL;
    }
    module->sinkports = mct_list_append(module->sinkports, port, NULL, NULL);
    module->numsinkports++;
    MCT_PORT_PARENT(port) = mct_list_append(MCT_PORT_PARENT(port), module,
                              NULL, NULL);
  }
  CDBG_HIGH("%s:%d: info: CPP module_init successful", __func__, __LINE__);
  return module;

error_cleanup_module:
  mct_module_destroy(module);
  return NULL;
}

/** get_cpp_loglevel:
 *
 *  Args:
 *  Return:
 *    void
 **/

void get_cpp_loglevel()
{
  uint32_t temp;
  uint32_t log_level;
  uint32_t debug_mask;
  memset(cpp_prop, 0, sizeof(cpp_prop));
  /**  Higher 4 bits : Value of Debug log level (Default level is 1 to print all CDBG_HIGH)
       Lower 28 bits : Control mode for sub module logging(Only 3 sub modules in PPROC )
       0x1 for PPROC
       0x10 for C2D
       0x100 for CPP  */
  property_get("persist.camera.pproc.debug.mask", cpp_prop, "268435463"); // 0x10000007=268435463
  temp = atoi(cpp_prop);
  log_level = ((temp >> 28) & 0xF);
  debug_mask = (temp & PPROC_DEBUG_MASK_CPP);
  if (debug_mask > 0)
      gCamCppLogLevel = log_level;
  else
      gCamCppLogLevel = 0; // Debug logs are not required if debug_mask is zero
}

/** cpp_module_free_port
 *    @data: port object to free
 *    @user_data: should be NULL
 *
 *  To free a sink or source port.
 *
 *  Return TRUE on success.
 **/
static boolean cpp_module_free_port(void *data, void *user_data)
{
  mct_port_t *port = MCT_PORT_CAST(data);
  mct_module_t *module = (mct_module_t *)user_data;

  CDBG("%s:%d] E\n", __func__, __LINE__);
  if (!port ) {
    CDBG_ERROR("%s:%d] error because list data is null\n", __func__,
      __LINE__);
    return FALSE;
  }
  if (!module ) {
    CDBG_ERROR("%s:%d] error because module is null\n", __func__,
      __LINE__);
    return FALSE;
  }

  if (strncmp(MCT_OBJECT_NAME(port), "cpp_sink", strlen("cpp_sink")) &&
      strncmp(MCT_OBJECT_NAME(port), "cpp_src", strlen("cpp_src"))) {
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
  cpp_port_destroy(port);

  CDBG("%s:%d] X\n", __func__, __LINE__);
  return TRUE;
}


/** cpp_module_deinit:
 *
 *  Args:
 *    @module: pointer to cpp mct module
 *  Return:
 *    void
 **/
void cpp_module_deinit(mct_module_t *module)
{
  int i = 0;
  int numsrcports = 0;
  int numsinkports = 0;
  if (!module || strcmp(MCT_OBJECT_NAME(module), "cpp")) {
    CDBG_ERROR("%s, Invalid module",__func__);
    return;
  }

  cpp_module_ctrl_t *ctrl =
    (cpp_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);

  numsrcports = module->numsrcports;
  numsinkports = module->numsinkports;

  for (i = 0; i < numsinkports; i++) {
    cpp_module_free_port(MCT_MODULE_SINKPORTS(module)->data,module);
  }
  for (i = 0; i < numsrcports; i++) {
    cpp_module_free_port(MCT_MODULE_SRCPORTS(module)->data,module);
  }

  mct_list_free_list(MCT_MODULE_SRCPORTS(module));
  mct_list_free_list(MCT_MODULE_SINKPORTS(module));

  cpp_module_destroy_cpp_ctrl(ctrl);
  mct_module_destroy(module);
}

static cpp_module_ctrl_t* cpp_module_create_cpp_ctrl(void)
{
  cpp_module_ctrl_t *ctrl = NULL;
  mct_queue_t *q;
  int rc;
  ctrl = (cpp_module_ctrl_t *) malloc(sizeof(cpp_module_ctrl_t));
  if(!ctrl) {
    CDBG_ERROR("%s:%d, malloc failed", __func__, __LINE__);
    return NULL;
  }
  memset(ctrl, 0x00, sizeof(cpp_module_ctrl_t));

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

  ctrl->clk_rate_list.list = NULL;
  ctrl->clk_rate_list.size = 0;
  pthread_mutex_init(&(ctrl->clk_rate_list.mutex), NULL);

  /* Create PIPE for communication with cpp_thread */
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

  /* initialize cpp_mutex */
  pthread_mutex_init(&(ctrl->cpp_mutex), NULL);

  /* Create the CPP hardware instance */
  ctrl->cpphw = cpp_hardware_create();
  if(ctrl->cpphw == NULL) {
    CDBG_ERROR("%s:%d, failed, cannnot create cpp hardware instance\n",
      __func__, __LINE__);
    goto error_hw;
  }

  /* open the cpp hardware and load firmware at this time */
  cpp_hardware_open_subdev(ctrl->cpphw);
  cpp_hardware_cmd_t cmd;
  cmd.type = CPP_HW_CMD_LOAD_FIRMWARE;
  cpp_hardware_process_command(ctrl->cpphw, cmd);
  cpp_hardware_close_subdev(ctrl->cpphw);

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

static int32_t cpp_module_destroy_cpp_ctrl(cpp_module_ctrl_t *ctrl)
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
  pthread_mutex_destroy(&(ctrl->clk_rate_list.mutex));
  pthread_mutex_destroy(&(ctrl->cpp_mutex));
  pthread_cond_destroy(&(ctrl->th_start_cond));
  close(ctrl->pfd[READ_FD]);
  close(ctrl->pfd[WRITE_FD]);
  cpp_hardware_destroy(ctrl->cpphw);
  free(ctrl);
  return 0;
}

void cpp_module_set_mod(mct_module_t *module, unsigned int module_type,
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

boolean cpp_module_query_mod(mct_module_t *module, void *buf,
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
  int i = 0;

  cpp_module_ctrl_t *ctrl = (cpp_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);

  /* TODO: Fill pp cap according to CPP HW caps*/
  cpp_hardware_cmd_t cmd;
  cmd.type = CPP_HW_CMD_GET_CAPABILITIES;
  rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
  if(rc < 0) {
    CDBG_ERROR("%s:%d: failed\n", __func__, __LINE__);
    return FALSE;
  }
  /* TODO: Need a linking function to fill pp cap based on HW caps? */
  pp_cap->supported_effects[pp_cap->supported_effects_cnt++] =
    CAM_EFFECT_MODE_OFF;
  pp_cap->supported_effects[pp_cap->supported_effects_cnt++] =
    CAM_EFFECT_MODE_EMBOSS;
  pp_cap->supported_effects[pp_cap->supported_effects_cnt++] =
    CAM_EFFECT_MODE_SKETCH;
  pp_cap->supported_effects[pp_cap->supported_effects_cnt++] =
    CAM_EFFECT_MODE_NEON;
  if (pp_cap->height_padding < CAM_PAD_TO_64) {
    pp_cap->height_padding = CAM_PAD_TO_64;
  }
  if (pp_cap->plane_padding < CAM_PAD_TO_64) {
    pp_cap->plane_padding = CAM_PAD_TO_64;
  }
  if (pp_cap->width_padding < CAM_PAD_TO_64) {
    pp_cap->width_padding = CAM_PAD_TO_64;
  }
  pp_cap->min_num_pp_bufs += MODULE_CPP_MIN_NUM_PP_BUFS;
  if (query_buf->sensor_cap.sensor_format != FORMAT_YCBCR) {
    pp_cap->feature_mask |= CAM_QCOM_FEATURE_SHARPNESS;
    pp_cap->feature_mask |= CAM_QCOM_FEATURE_EFFECT;
  }
  pp_cap->feature_mask |= CAM_QCOM_FEATURE_ROTATION;
#ifndef CAMERA_FEATURE_WNR_SW
  if (query_buf->sensor_cap.sensor_format != FORMAT_YCBCR) {
    pp_cap->feature_mask |= CAM_QCOM_FEATURE_DENOISE2D;
  }
  pp_cap->feature_mask |= (CAM_QCOM_FEATURE_CROP |
    CAM_QCOM_FEATURE_ROTATION |
    CAM_QCOM_FEATURE_FLIP | CAM_QCOM_FEATURE_SCALE);
#else
  pp_cap->feature_mask |= (CAM_QCOM_FEATURE_CROP |
    CAM_QCOM_FEATURE_ROTATION | CAM_QCOM_FEATURE_FLIP | CAM_QCOM_FEATURE_SCALE);
#endif

  for(i = 0; i < CPP_MODULE_MAX_SESSIONS; i++) {
    if (ctrl->session_params[i] &&
      (ctrl->session_params[i]->session_id == sessionid)) {
        ctrl->session_params[i]->sensor_format =
          query_buf->sensor_cap.sensor_format;
        break;
    }
  }

  return TRUE;
}

boolean cpp_module_start_session(mct_module_t *module, unsigned int sessionid)
{
  int32_t rc;
  get_cpp_loglevel(); //dynamic logging level
  CDBG_HIGH("%s:%d, info: starting session %d", __func__, __LINE__, sessionid);
  if(!module) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return FALSE;
  }
  cpp_module_ctrl_t *ctrl = (cpp_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return FALSE;
  }
  if(ctrl->session_count >= CPP_MODULE_MAX_SESSIONS) {
    CDBG_ERROR("%s:%d, failed, too many sessions, count=%d",
      __func__, __LINE__, ctrl->session_count);
    return FALSE;
  }

  /* create a new session specific params structure */
  int i;
  for(i=0; i < CPP_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i] == NULL) {
      ctrl->session_params[i] =
        (cpp_module_session_params_t*)
           malloc(sizeof(cpp_module_session_params_t));
      memset(ctrl->session_params[i], 0x00,
        sizeof(cpp_module_session_params_t));
      ctrl->session_params[i]->session_id = sessionid;
      ctrl->session_params[i]->frame_hold.is_frame_hold = FALSE;
      ctrl->session_params[i]->dis_hold.is_valid = FALSE;
      ctrl->session_params[i]->fps_range.max_fps = 30.0f;
      ctrl->session_params[i]->fps_range.min_fps = 30.0f;
      ctrl->session_params[i]->fps_range.video_max_fps= 30.0f;
      ctrl->session_params[i]->fps_range.video_min_fps= 30.0f;
      ctrl->session_params[i]->diag_params.control_asf7x7.enable = 1;
      ctrl->session_params[i]->diag_params.control_wnr.enable = 1;
      ctrl->session_params[i]->hw_params.sharpness_level = 1.0;
      ctrl->session_params[i]->hw_params.asf_mode =
        CPP_PARAM_ASF_DUAL_FILTER;
      pthread_mutex_init(&(ctrl->session_params[i]->dis_mutex), NULL);
      break;
    }
  }

  /* start the thread only when first session starts */
  if(ctrl->session_count == 0) {
    /* open the cpp hardware */
    rc = cpp_hardware_open_subdev(ctrl->cpphw);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, cpp_thread_create() failed", __func__, __LINE__);
      return FALSE;
    }
    /* spawn the cpp thread */
    rc = cpp_thread_create(module);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, cpp_thread_create() failed", __func__, __LINE__);
      return FALSE;
    }
    CDBG("%s:%d, info: cpp_thread created.", __func__, __LINE__);

    /* set default clock */
    cpp_module_set_clock_freq(ctrl,NULL,0);
  }
  ctrl->session_count++;
  CDBG_HIGH("%s:%d, info: session %d started.", __func__, __LINE__, sessionid);
  return TRUE;
}

boolean cpp_module_stop_session(mct_module_t *module, unsigned int sessionid)
{
  int32_t rc;
  if(!module) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return FALSE;
  }
  cpp_module_ctrl_t *ctrl = (cpp_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return FALSE;
  }
  CDBG_HIGH("%s:%d, info: stopping session %d ...", __func__, __LINE__, sessionid);
  ctrl->session_count--;
  /* stop the thread only when last session terminates */
  if(ctrl->session_count == 0) {
    /* stop the CPP thread */
    CDBG("%s:%d, info: stopping cpp_thread...", __func__, __LINE__);
    cpp_thread_msg_t msg;
    msg.type = CPP_THREAD_MSG_ABORT;
    rc = cpp_module_post_msg_to_thread(module, msg);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, cpp_module_post_msg_to_thread() failed",
        __func__, __LINE__);
      return FALSE;
    }
    /* wait for thread completion */
    pthread_join(ctrl->cpp_thread, NULL);
    /* close the cpp hardware */
    CDBG("%s:%d, closing cpp subdev...", __func__, __LINE__);
    cpp_hardware_close_subdev(ctrl->cpphw);
  }
  /* remove the session specific params */
  int i;
  for(i=0; i < CPP_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i]) {
      if(ctrl->session_params[i]->session_id == sessionid) {
        pthread_mutex_destroy(
                &(ctrl->session_params[i]->dis_mutex));
        free(ctrl->session_params[i]);
        ctrl->session_params[i] = NULL;
        break;
      }
    }
  }
  CDBG_HIGH("%s:%d, info: session %d stopped.", __func__, __LINE__, sessionid);
  return TRUE;
}

/* cpp_module_post_msg_to_thread:
 *
 * @module: cpp module pointer
 * @msg: message to be posted for thread
 * Description:
 *  Writes message to the pipe for which the cpp_thread is listening to.
 *
 **/
int32_t cpp_module_post_msg_to_thread(mct_module_t *module,
  cpp_thread_msg_t msg)
{
  int32_t rc;
  if(!module) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EINVAL;
  }
  CDBG_LOW("%s:%d, msg.type=%d", __func__, __LINE__, msg.type);
  cpp_module_ctrl_t *ctrl = (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);
  rc = write(ctrl->pfd[WRITE_FD], &msg, sizeof(cpp_thread_msg_t));
  if(rc < 0) {
    CDBG_ERROR("%s:%d, write() failed\n", __func__, __LINE__);
    return -EIO;
  }
  return 0;
}

/* cpp_module_enq_event:
 *
 * @module: cpp module pointer
 * @event:  cpp_event to be queued
 * @prio:   priority of the event(realtime/offline)
 *
 * Description:
 *  Enqueues a cpp_event into realtime or offline queue based on the
 *  priority.
 *
 **/
int32_t cpp_module_enq_event(mct_module_t* module,
  cpp_module_event_t* cpp_event, cpp_priority_t prio)
{
  if(!module || !cpp_event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p", __func__, __LINE__,
      module, cpp_event);
    return -EINVAL;
  }
  cpp_module_ctrl_t *ctrl = (cpp_module_ctrl_t *)MCT_OBJECT_PRIVATE(module);

  CDBG_LOW("%s:%d, prio=%d", __func__, __LINE__, prio);
  switch (prio) {
  case CPP_PRIORITY_REALTIME:
    PTHREAD_MUTEX_LOCK(&(ctrl->realtime_queue.mutex));
    mct_queue_push_tail(ctrl->realtime_queue.q, cpp_event);
    CDBG_LOW("%s:%d, real-time queue size = %d", __func__, __LINE__,
      ctrl->realtime_queue.q->length);
    PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
    break;
  case CPP_PRIORITY_OFFLINE:
    PTHREAD_MUTEX_LOCK(&(ctrl->offline_queue.mutex));
    mct_queue_push_tail(ctrl->offline_queue.q, cpp_event);
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

int32_t cpp_module_send_event_downstream(mct_module_t* module,
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
  mct_port_t* port = cpp_module_find_port_with_identity(module, MCT_PORT_SRC,
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

/* cpp_module_send_event_upstream:
 *
 * Description:
 *  Sends event to the upstream peer based on the event identity.
 *
 **/
int32_t cpp_module_send_event_upstream(mct_module_t* module,
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
  mct_port_t* port = cpp_module_find_port_with_identity(module, MCT_PORT_SINK,
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

/** cpp_module_invalidate_and_free_qentry
 *    @queue: queue to invalidate and free entries
 *    @identity: identity to invalidate
 *
 *  Invalidate the queue entries corresponding to
 *  given identity. The invalidated entries are acked
 *  and then freed from the list.
 *
 *  Return: void
 **/
static void cpp_module_invalidate_and_free_qentry(cpp_module_ctrl_t* ctrl,
  cpp_module_event_queue_t *queue, uint32_t identity)
{
  mct_list_t *key_list = NULL;
  void*  input[3];
  input[0] = ctrl;
  input[1] = &identity;
  input[2] = &key_list;
  /* First get all keys correspoding to the identity in key_list. Then traverse
     key_list and release the acks from ack_list. This is to avoid holding queue
     mutex when sending an event upstream to avoid potential deadlocks */
  PTHREAD_MUTEX_LOCK(&(queue->mutex));
  mct_queue_traverse(queue->q, cpp_module_invalidate_q_traverse_func,
    input);
  PTHREAD_MUTEX_UNLOCK(&(queue->mutex));
  /* traverse key list to release acks */
  mct_list_traverse(key_list, cpp_module_release_ack_traverse_func, ctrl);
  /* free the key list */
  mct_list_free_all(key_list, cpp_module_key_list_free_traverse_func);
  return;
}

/* cpp_module_invalidate_queue:
 *
 **/
int32_t cpp_module_invalidate_queue(cpp_module_ctrl_t* ctrl,
  uint32_t identity)
{
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  cpp_module_invalidate_and_free_qentry(ctrl, &ctrl->realtime_queue, identity);
  cpp_module_invalidate_and_free_qentry(ctrl, &ctrl->offline_queue, identity);
  return 0;
}

/* cpp_module_send_buf_divert_ack:
 *
 *  Sends a buf_divert_ack to upstream module.
 *
 **/
static int32_t cpp_module_send_buf_divert_ack(cpp_module_ctrl_t *ctrl,
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
  rc = cpp_module_send_event_upstream(ctrl->p_module, &event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EFAULT;
  }
  return 0;
}

/* cpp_module_do_ack:
 *
 *  Decrements the refcount of the ACK which is stored in the ack_list,
 *  correspoding to the key. If the refcount becomes 0, a buf_divert_ack
 *  is sent upstream. At this time the ack entry is removed from list.
 *
 **/
int32_t cpp_module_do_ack(cpp_module_ctrl_t *ctrl,
  cpp_module_ack_key_t key)
{
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed", __func__, __LINE__);
    return -EINVAL;
  }
  /* find corresponding ack from the list. If the all references
     to that ack are done, send the ack and remove the entry from the list */
  cpp_module_ack_t *cpp_ack;
  CDBG("%s:%d, buf_idx=%d, identity=0x%x", __func__, __LINE__,
    key.buf_idx, key.identity);
  PTHREAD_MUTEX_LOCK(&(ctrl->ack_list.mutex));
  cpp_ack = cpp_module_find_ack_from_list(ctrl, key);
  if(!cpp_ack) {
    CDBG_ERROR("%s:%d, failed, ack not found in list, for buf_idx=%d, "
      "identity=0x%x", __func__, __LINE__, key.buf_idx, key.identity);
    PTHREAD_MUTEX_UNLOCK(&(ctrl->ack_list.mutex));
    return -EFAULT;
  }
  cpp_ack->ref_count--;
  CDBG("%s:%d, cpp_ack->ref_count=%d\n", __func__, __LINE__,
    cpp_ack->ref_count);
  struct timeval tv;
  if(cpp_ack->ref_count == 0) {
    ctrl->ack_list.list = mct_list_remove(ctrl->ack_list.list, cpp_ack);
    ctrl->ack_list.size--;
    /* unlock before sending event to prevent any deadlock */
    PTHREAD_MUTEX_UNLOCK(&(ctrl->ack_list.mutex));
    gettimeofday(&(cpp_ack->out_time), NULL);
    CDBG_LOW("%s:%d, in_time=%ld.%ld us, out_time=%ld.%ld us, ",
      __func__, __LINE__, cpp_ack->in_time.tv_sec, cpp_ack->in_time.tv_usec,
      cpp_ack->out_time.tv_sec, cpp_ack->out_time.tv_usec);
    CDBG("%s:%d, holding time = %6ld us,frame_id=%d,buf_idx=%d,identity=0x%x",
      __func__, __LINE__,
      (cpp_ack->out_time.tv_sec - cpp_ack->in_time.tv_sec)*1000000L +
      (cpp_ack->out_time.tv_usec - cpp_ack->in_time.tv_usec),
      cpp_ack->isp_buf_divert_ack.frame_id,
      cpp_ack->isp_buf_divert_ack.buf_idx,
      key.identity);
    cpp_module_send_buf_divert_ack(ctrl, cpp_ack->isp_buf_divert_ack);
    gettimeofday(&tv, NULL);
    CDBG_LOW("%s:%d, upstream event time = %6ld us, ", __func__, __LINE__,
      (tv.tv_sec - cpp_ack->out_time.tv_sec)*1000000L +
      (tv.tv_usec - cpp_ack->out_time.tv_usec));
    free(cpp_ack);
  } else {
    PTHREAD_MUTEX_UNLOCK(&(ctrl->ack_list.mutex));
  }
  return 0;
}

/* cpp_module_handle_ack_from_downstream:
 *
 *  Handles the buf_divert_ack event coming from downstream module.
 *  Corresponding ACK stored in ack_list is updated and/or released
 *  accordingly.
 *
 */
static int32_t cpp_module_handle_ack_from_downstream(mct_module_t* module,
  mct_event_t* event)
{
  cpp_hardware_cmd_t cmd;
  cpp_hardware_event_data_t event_data;
  int32_t rc = 0;
  if(!module || !event) {
    CDBG_ERROR("%s:%d, failed, module=%p, event=%p\n", __func__, __LINE__,
      module, event);
    return -EINVAL;
  }
  cpp_module_ctrl_t* ctrl = (cpp_module_ctrl_t*) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  /* Among the 2 types of buffer divert event from pproc (unprocessed divert
     and processed divert), we assume only processed divert is asynchronous.
     Hence we expect this ack event only for processed divert in which case
     there will not be any entry in the ack list. We make it a policy that
     if unprocessed divert is needed by downstream module, then it has to be
     synchronous. */
  isp_buf_divert_ack_t* isp_buf_ack =
    (isp_buf_divert_ack_t*)(event->u.module_event.module_event_data);
#if 0
  cpp_module_ack_key_t key;
  key.identity = isp_buf_ack->identity;
  key.buf_idx = isp_buf_ack->buf_idx;
  CDBG("%s:%d, doing ack for divert_done ack from downstream",
    __func__, __LINE__);
  cpp_module_do_ack(ctrl, key);
#endif
  cmd.type = CPP_HW_CMD_QUEUE_BUF;
  cmd.u.event_data = &event_data;
  memset(&event_data, 0, sizeof(event_data));
  event_data.identity = isp_buf_ack->identity;
  event_data.out_buf_idx = isp_buf_ack->buf_idx;
  event_data.timestamp = isp_buf_ack->timestamp;
  event_data.frame_id = isp_buf_ack->frame_id;
  event_data.is_buf_dirty = isp_buf_ack->is_buf_dirty;
  rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
  }
  return 0;
}

/* cpp_module_put_new_ack_in_list:
 *
 * Description:
 *   Adds a new ACK in the ack_list with the given params.
 **/
int32_t cpp_module_put_new_ack_in_list(cpp_module_ctrl_t *ctrl,
  cpp_module_ack_key_t key, int32_t buf_dirty, int32_t ref_count,
  isp_buf_divert_t *isp_buf)
{
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  /* this memory will be freed by thread when ack is removed from list */
  cpp_module_ack_t *cpp_ack =
    (cpp_module_ack_t *) malloc (sizeof(cpp_module_ack_t));
  if(!cpp_ack) {
    CDBG_ERROR("%s:%d, malloc failed\n", __func__, __LINE__);
    return -ENOMEM;
  }
  memset(cpp_ack, 0x00, sizeof(cpp_module_ack_t));
  cpp_ack->isp_buf_divert_ack.identity = key.identity;
  cpp_ack->isp_buf_divert_ack.buf_idx = key.buf_idx;
  cpp_ack->isp_buf_divert_ack.is_buf_dirty = buf_dirty;
  cpp_ack->isp_buf_divert_ack.channel_id = key.channel_id;
  cpp_ack->isp_buf_divert_ack.frame_id = isp_buf->buffer.sequence;
  cpp_ack->isp_buf_divert_ack.timestamp = isp_buf->buffer.timestamp;
  cpp_ack->isp_buf_divert_ack.meta_data = key.meta_data;
  cpp_ack->ref_count = ref_count;
  cpp_ack->isp_buf_divert_ack.handle = isp_buf->handle;
  cpp_ack->isp_buf_divert_ack.output_format = isp_buf->output_format;
  cpp_ack->isp_buf_divert_ack.input_intf = isp_buf->input_intf;
  CDBG("%s:%d, adding ack in list, identity=0x%x", __func__, __LINE__,
    cpp_ack->isp_buf_divert_ack.identity);
  CDBG("%s:%d, buf_idx=%d, ref_count=%d", __func__, __LINE__,
    cpp_ack->isp_buf_divert_ack.buf_idx, cpp_ack->ref_count);
  PTHREAD_MUTEX_LOCK(&(ctrl->ack_list.mutex));
  gettimeofday(&(cpp_ack->in_time), NULL);
  ctrl->ack_list.list = mct_list_append(ctrl->ack_list.list,
                          cpp_ack, NULL, NULL);
  ctrl->ack_list.size++;
  PTHREAD_MUTEX_UNLOCK(&(ctrl->ack_list.mutex));
  return 0;
}

int32_t cpp_module_process_downstream_event(mct_module_t* module,
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
  cpp_module_ctrl_t *ctrl = (cpp_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  /* handle events based on type, if not handled, forward it downstream */
  switch(event->type) {
  case MCT_EVENT_MODULE_EVENT: {
    switch(event->u.module_event.type) {
    case MCT_EVENT_MODULE_BUF_DIVERT:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_BUF_DIVERT: identity=0x%x", __func__,
        __LINE__, identity);
      rc = cpp_module_handle_buf_divert_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_ISP_OUTPUT_DIM:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_ISP_OUTPUT_DIM: identity=0x%x",
               __func__, __LINE__, identity);
      rc = cpp_module_handle_isp_out_dim_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_STATS_AEC_UPDATE:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_STATS_AEC_UPDATE: identity=0x%x",
               __func__, __LINE__, identity);
      rc = cpp_module_handle_aec_update_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_SET_CHROMATIX_PTR:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_SET_CHROMATIX_PTR: identity=0x%x",
               __func__, __LINE__, identity);
      rc = cpp_module_handle_chromatix_ptr_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_STREAM_CROP:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_STREAM_CROP: identity=0x%x", __func__,
        __LINE__, identity);
      rc = cpp_module_handle_stream_crop_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_STATS_DIS_UPDATE:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_STATS_DIS_UPDATE: identity=0x%x",
               __func__, __LINE__, identity);
      rc = cpp_module_handle_dis_update_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_SET_STREAM_CONFIG:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_SET_STREAM_CONFIG: identity=0x%x",
               __func__, __LINE__, identity);
      rc = cpp_module_handle_stream_cfg_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_SENSOR_UPDATE_FPS:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_SENSOR_UPDATE_FPS: identity=0x%x", __func__,
        __LINE__, identity);
      rc = cpp_module_handle_fps_update_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_PPROC_DIVERT_INFO:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_PPROC_DIVERT_INFO: identity=0x%x",
               __func__, __LINE__, identity);
      rc = cpp_module_handle_div_info_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    case MCT_EVENT_MODULE_SET_RELOAD_CHROMATIX: {
      CDBG_ERROR("%s:%d: MCT_EVENT_MODULE_SET_RELOAD_CHROMATIX: identity=0x%x",
               __func__, __LINE__, identity);
      rc = cpp_module_handle_load_chromatix_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    }
    case MCT_EVENT_MODULE_PPROC_SET_OUTPUT_BUFF:
      CDBG_LOW("%s:%d: MCT_EVENT_MODULE_PPROC_SET_OUTPUT_BUFF: identity=0x%x",
        __func__, __LINE__, identity);
      rc = cpp_module_handle_set_output_buff_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    default:
      rc = cpp_module_send_event_downstream(module, event);
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
      rc = cpp_module_handle_streamon_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, streamon failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    }
    case MCT_EVENT_CONTROL_STREAMOFF: {
      rc = cpp_module_handle_streamoff_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, streamoff failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    }
    case MCT_EVENT_CONTROL_SET_PARM: {
      rc = cpp_module_handle_set_parm_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    }
    case MCT_EVENT_CONTROL_PARM_STREAM_BUF: {
      rc = cpp_module_handle_set_stream_parm_event(module, event);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return rc;
      }
      break;
    }
    case MCT_EVENT_CONTROL_UPDATE_BUF_INFO:
      rc = cpp_module_handle_update_buf_info(module,event);
      CDBG_ERROR("%s:%d, Update buf queue: APPEND\n", __func__, __LINE__);
      if(rc < 0) {
        CDBG_ERROR("%s:%d, failed to update buffer info\n", __func__, __LINE__);
        return rc;
      }
      break;
    default:
      rc = cpp_module_send_event_downstream(module, event);
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

int32_t cpp_module_process_upstream_event(mct_module_t* module,
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
      rc = cpp_module_handle_ack_from_downstream(module, event);
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
  rc = cpp_module_send_event_upstream(module, event);
  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return rc;
  }
  return 0;
}

/* cpp_module_set_clock_freq:
 *
 * Check for stream information and select clock frequency.
 **/
int32_t cpp_module_set_clock_freq(cpp_module_ctrl_t *ctrl,
  cpp_module_stream_params_t *stream_params, uint32_t stream_event)
{
  int32_t rc = 0;
  uint32_t i;
  cpp_hardware_cmd_t cmd;
  uint32_t input_dim = 0, output_dim = 0, dim = 0;
  float input_fps;
  float input_format_factor, output_format_factor;
  cpp_module_stream_clk_rate_t *clk_rate_obj = NULL;
  int64_t total_load = 0;
  unsigned long clk_rate = 0, rounded_clk_rate = 0;
  float input_bw = 0.0f, output_bw = 0.0f;

  if (!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EINVAL;
  }

  if ( NULL == stream_params) {
    ctrl->clk_rate = ctrl->cpphw->hwinfo.freq_tbl[0];
    cmd.u.clock_settings.clock_rate = ctrl->cpphw->hwinfo.freq_tbl[0];
    cmd.u.clock_settings.avg = 2 * ctrl->cpphw->hwinfo.freq_tbl[0];
    cmd.u.clock_settings.inst = (6 * cmd.u.clock_settings.avg) / 5;
    cmd.type = CPP_HW_CMD_SET_CLK;
    /* Update the clk rate with the next biggest value */
    rc = cpp_hardware_process_command(ctrl->cpphw, cmd);
    if(rc < 0) {
      CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
      return rc;
    }
    return 0;

  } else if (stream_event){

    switch (stream_params->hw_params.output_info.plane_fmt) {
    case CPP_PARAM_PLANE_CRCB422:
    case CPP_PARAM_PLANE_CBCR422: {
      output_format_factor = 2;
      break;
    }
    default: {
      output_format_factor = 1.5f;
      break;
    }
    }

    switch (stream_params->hw_params.input_info.plane_fmt) {
    case CPP_PARAM_PLANE_CRCB422:
    case CPP_PARAM_PLANE_CBCR422: {
      input_format_factor = 2;
      break;
    }
    default: {
      input_format_factor = 1.5f;
      break;
    }
    }

    if(stream_params->priority == CPP_PRIORITY_REALTIME)
      input_fps = stream_params->hfr_skip_info.input_fps;
    else
      input_fps = MINIMUM_PROCESS_TIME;

    output_dim = stream_params->hw_params.output_info.width *
      stream_params->hw_params.output_info.height;

    input_dim = stream_params->hw_params.input_info.width *
      stream_params->hw_params.input_info.height;
    dim = MAX(input_dim, output_dim);
    if (dim)
      total_load = (float)dim * input_format_factor *
        PADDING_FACTOR * input_fps;
    if (stream_params->hw_params.duplicate_output &&
       stream_params->linked_stream_params &&
       (stream_params->linked_stream_params->is_stream_on == TRUE)) {
       /* if output duplication is enabled, only 50% load is added */
       total_load = (float)total_load * 0.5f;
    }

    PTHREAD_MUTEX_LOCK(&(ctrl->clk_rate_list.mutex));
    clk_rate_obj = cpp_module_find_clk_rate_by_identity(ctrl,
      stream_params->hw_params.identity);
    if (clk_rate_obj) {
        CDBG("%s:%d,updating ident 0x%x from load %ld, to load %ld",
            __func__, __LINE__,
          stream_params->identity, (long)clk_rate_obj->total_load, (long)total_load);
        clk_rate_obj->total_load = total_load;
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->clk_rate_list.mutex));
    if (!clk_rate_obj) {
      clk_rate_obj = malloc(sizeof(cpp_module_stream_clk_rate_t));
      if (NULL == clk_rate_obj) {
        CDBG_ERROR("%s:%d, malloc failed\n", __func__, __LINE__);
        return -ENOMEM;
      }
      clk_rate_obj->identity = stream_params->hw_params.identity;
      clk_rate_obj->total_load = total_load;
      PTHREAD_MUTEX_LOCK(&(ctrl->clk_rate_list.mutex));
      ctrl->clk_rate_list.list = mct_list_append(ctrl->clk_rate_list.list,
        clk_rate_obj, NULL, NULL);
      ctrl->clk_rate_list.size++;
      PTHREAD_MUTEX_UNLOCK(&(ctrl->clk_rate_list.mutex));
    }
  } else {
    PTHREAD_MUTEX_LOCK(&(ctrl->clk_rate_list.mutex));
    clk_rate_obj = cpp_module_find_clk_rate_by_identity(ctrl,
      stream_params->hw_params.identity);
    ctrl->clk_rate_list.list = mct_list_remove(ctrl->clk_rate_list.list,
       clk_rate_obj);
    ctrl->clk_rate_list.size--;
    free(clk_rate_obj);
    PTHREAD_MUTEX_UNLOCK(&(ctrl->clk_rate_list.mutex));
  }
  total_load = 0;
  total_load = cpp_module_get_total_load_by_value(ctrl);
  if (total_load < 0) {
    CDBG_ERROR("Fail to get total load");
    return -EFAULT;
  }
  /* output bandwidth does not need to acomodate for padding factor */
  output_bw = ((float)(total_load) / PADDING_FACTOR);
  /* input bandwidth is same as the max load calculated */
  input_bw = (float)(total_load);
  /* clock rate total load divided by the cpp throughput */
  clk_rate = (float)(total_load) / MINIMUM_CPP_THROUGHPUT;

  CDBG("%s: stream type %d, ident 0x%x load  %lld, width %d height %d clk rate %ld", __func__,
    stream_params->stream_type, stream_params->identity, total_load,
    stream_params->hw_params.output_info.width, stream_params->hw_params.output_info.height, clk_rate);

  cmd.u.clock_settings.avg =
    (output_bw + input_bw);

  for(i = 0; i < ctrl->cpphw->hwinfo.freq_tbl_count; i++) {
    if (clk_rate < ctrl->cpphw->hwinfo.freq_tbl[i]) {
      rounded_clk_rate = ctrl->cpphw->hwinfo.freq_tbl[i];
      break;
    }
  }

  if (i == ctrl->cpphw->hwinfo.freq_tbl_count) {
    rounded_clk_rate =
      ctrl->cpphw->hwinfo.freq_tbl[ctrl->cpphw->hwinfo.freq_tbl_count - 1];
    cmd.u.clock_settings.inst =
        cmd.u.clock_settings.avg;
  } else {
    if(clk_rate != 0) {
      cmd.u.clock_settings.inst =
        ((rounded_clk_rate * cmd.u.clock_settings.avg) / clk_rate) *
        MAXIMUM_CPP_THROUGHPUT;
    } else {
      cmd.u.clock_settings.inst = 0;
    }
  }

  ctrl->clk_rate = rounded_clk_rate;
  cmd.u.clock_settings.clock_rate = ctrl->clk_rate;

  CDBG("%s: stream type %d,  ab %lld, ib %lld clock %ld", __func__,
    stream_params->stream_type, cmd.u.clock_settings.avg,
    cmd.u.clock_settings.inst, cmd.u.clock_settings.clock_rate);

  cmd.type = CPP_HW_CMD_SET_CLK;
  /* The new stream needs higher clock */
  rc = cpp_hardware_process_command(ctrl->cpphw, cmd);

  if(rc < 0) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return rc;
  }

  return rc;
}

/* cpp_module_notify_add_stream:
 *
 * creates and initializes the stream-specific paramater structures when a
 * stream is reserved in port
 **/
int32_t cpp_module_notify_add_stream(mct_module_t* module, mct_port_t* port,
  mct_stream_info_t* stream_info)
{
  if(!module || !stream_info || !port) {
    CDBG_ERROR("%s:%d, failed, module=%p, port=%p, stream_info=%p\n", __func__,
      __LINE__, module, port, stream_info);
    return -EINVAL;
  }
  cpp_module_ctrl_t *ctrl = (cpp_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed, module=%p\n", __func__, __LINE__, module);
    return -EINVAL;
  }
  uint32_t identity = stream_info->identity;
  /* create stream specific params structure */
  uint32_t session_id;
  int i,j;
  boolean success = FALSE;
  cpp_hardware_params_t *hw_params;
  cam_pp_feature_config_t *pp_config;
  session_id = CPP_GET_SESSION_ID(identity);
  CDBG("%s:%d, identity=0x%x\n", __func__, __LINE__, identity);

  /* find if a stream is already added on this port. If yes, we need to link
     that stream with this. (only for continuous streams)*/
  cpp_module_session_params_t *linked_session_params = NULL;
  cpp_module_stream_params_t *linked_stream_params = NULL;
  uint32_t linked_identity;
  int32_t found = cpp_port_get_linked_identity(port, identity,
    &linked_identity);
  if (found > 0) {
    CDBG("%s:%d, found linked identity=0x%x", __func__, __LINE__,
      linked_identity);
    cpp_module_get_params_for_identity(ctrl,linked_identity,
      &linked_session_params, &linked_stream_params);
    if (!linked_stream_params) {
      CDBG_ERROR("%s:%d, failed, module=%p\n", __func__, __LINE__, module);
      return -EINVAL;
    }
  }

  for(i=0; i < CPP_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i]) {
      if(ctrl->session_params[i]->session_id == session_id) {
        for(j=0; j < CPP_MODULE_MAX_STREAMS; j++) {
          if(ctrl->session_params[i]->stream_params[j] == NULL) {
            ctrl->session_params[i]->stream_params[j] =
              (cpp_module_stream_params_t *)
                 malloc (sizeof(cpp_module_stream_params_t));
            if (!ctrl->session_params[i]->stream_params[j]) {
              CDBG_ERROR("%s:%d failed: to malloc\n", __func__, __LINE__);
              return -ENOMEM;
            }
            memset(ctrl->session_params[i]->stream_params[j], 0x00,
              sizeof(cpp_module_stream_params_t));
            ctrl->session_params[i]->stream_params[j]->identity = identity;

            /* assign priority */
            if(stream_info->streaming_mode == CAM_STREAMING_MODE_CONTINUOUS) {
              ctrl->session_params[i]->stream_params[j]->priority =
                CPP_PRIORITY_REALTIME;
            } else {
              ctrl->session_params[i]->stream_params[j]->priority =
                CPP_PRIORITY_OFFLINE;
            }
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
            hw_params = &ctrl->session_params[i]->stream_params[j]->hw_params;
            /* output dimensions */
            hw_params->output_info.width = stream_info->dim.width;
            hw_params->output_info.height = stream_info->dim.height;
            hw_params->output_info.stride =
              stream_info->buf_planes.plane_info.mp[0].stride;
            hw_params->output_info.scanline =
              stream_info->buf_planes.plane_info.mp[0].scanline;
            hw_params->output_info.plane_offsets[0] =
              stream_info->buf_planes.plane_info.mp[1].offset;
            hw_params->output_info.plane_offsets[1] =
              stream_info->buf_planes.plane_info.mp[2].offset;

            hw_params->stream_type = stream_info->stream_type;
            hw_params->ez_tune_asf_enable = 1;
            #ifdef CAMERA_FEATURE_WNR_SW
              hw_params->ez_tune_wnr_enable = 0;
            #else
              hw_params->ez_tune_wnr_enable = 1;
            #endif
            hw_params->diagnostic_enable =
              ctrl->session_params[i]->hw_params.diagnostic_enable;
            /* rotation/flip */
            if (stream_info->stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
                pp_config = &stream_info->reprocess_config.pp_feature_config;
            } else {
                pp_config = &stream_info->pp_config;
            }
            if (pp_config->feature_mask & CAM_QCOM_FEATURE_SHARPNESS) {
              hw_params->asf_mode = CPP_PARAM_ASF_DUAL_FILTER;
              hw_params->sharpness_level = 1.0;
            } else {
              /* Disable ASF */
              hw_params->asf_mode = CPP_PARAM_ASF_OFF;
              hw_params->sharpness_level = 0.0f;
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
            /* format info */
            if (stream_info->fmt == CAM_FORMAT_YUV_420_NV12 ||
                stream_info->fmt == CAM_FORMAT_YUV_420_NV12_VENUS) {
              hw_params->output_info.plane_fmt = CPP_PARAM_PLANE_CBCR;
            } else if (stream_info->fmt == CAM_FORMAT_YUV_420_NV21) {
              hw_params->output_info.plane_fmt = CPP_PARAM_PLANE_CRCB;
            } else if (stream_info->fmt == CAM_FORMAT_YUV_422_NV16) {
              hw_params->output_info.plane_fmt = CPP_PARAM_PLANE_CBCR422;
            } else if (stream_info->fmt == CAM_FORMAT_YUV_422_NV61) {
              hw_params->output_info.plane_fmt = CPP_PARAM_PLANE_CRCB422;
            } else if (stream_info->fmt == CAM_FORMAT_YUV_420_YV12) {
                hw_params->output_info.plane_fmt = CPP_PARAM_PLANE_CRCB420;
            } else {
              CDBG_ERROR("%s:%d, failed. Format not supported\n", __func__,
                __LINE__);
              return -EINVAL;
            }
            cpp_divert_info_t *div_config = NULL;
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
              div_config = malloc(sizeof(cpp_divert_info_t));
              if(!div_config) {
                CDBG_ERROR("%s:%d, malloc failed\n", __func__, __LINE__);
                return -ENOMEM;
              }
              memset(div_config, 0, sizeof(cpp_divert_info_t));
              div_config->identity[0] = PPROC_INVALID_IDENTITY;
              div_config->identity[1] = PPROC_INVALID_IDENTITY;
            }

            /* Now attach the identity to divert config table */
            cpp_module_set_divert_cfg_identity(PPROC_INVALID_IDENTITY,
              identity, div_config);
            ctrl->session_params[i]->stream_params[j]->div_config =
              div_config;

            /* use output-duplication is possible for linked streams */
            cpp_module_set_output_duplication_flag(
              ctrl->session_params[i]->stream_params[j]);

            hw_params->identity = identity;

            /* initialize the mutex for stream_params */
            pthread_mutex_init(
              &(ctrl->session_params[i]->stream_params[j]->mutex), NULL);
            ctrl->session_params[i]->stream_count++;
            success = TRUE;
            cpp_module_dump_stream_params(
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

/* cpp_module_notify_remove_stream:
 *
 *  destroys stream-specific data structures when a stream is unreserved
 *  in port
 **/
int32_t cpp_module_notify_remove_stream(mct_module_t* module, uint32_t identity)
{
  cpp_hardware_cmd_t cmd;
  if(!module) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  cpp_module_ctrl_t *ctrl = (cpp_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(!ctrl) {
    CDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
    return -EINVAL;
  }
  CDBG("%s:%d: identity=0x%x\n", __func__, __LINE__, identity);
  /* destroy stream specific params structure */
  uint32_t session_id;
  int i,j;
  boolean success = FALSE;
  session_id = CPP_GET_SESSION_ID(identity);
  for(i=0; i < CPP_MODULE_MAX_SESSIONS; i++) {
    if(ctrl->session_params[i]) {
      if(ctrl->session_params[i]->session_id == session_id) {
        for(j=0; j < CPP_MODULE_MAX_STREAMS; j++) {
          if(ctrl->session_params[i]->stream_params[j]) {
            if(ctrl->session_params[i]->stream_params[j]->identity ==
                identity) {
               /* Now detach the identity from divert config table */
               cpp_module_set_divert_cfg_identity(identity,
                 PPROC_INVALID_IDENTITY,
                 ctrl->session_params[i]->stream_params[j]->div_config);
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
