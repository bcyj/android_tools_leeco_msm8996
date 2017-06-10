/* afd_port.c
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "afd_port.h"
#include "afd_thread.h"
#include "stats_port.h"
#include "stats_module.h"
#include "modules.h"


/** afd_port_callback
 *
 **/
static void afd_port_callback(afd_output_data_t *output, void *p)
{
  mct_port_t         *port    = (mct_port_t *)p;
  afd_port_private_t *private = NULL;
  mct_event_t        event;
  stats_update_t     stats_upate;

  if (!output || !port) {
    return;
  }
  private = (afd_port_private_t *)(port->port_private);
  if (!private)
      return;

  switch (output->afd_atb) {
    case AFD_TYPE_60HZ:
      stats_upate.afd_update.afd_atb =
        AFD_TBL_60HZ;
      break;
    case AFD_TYPE_50HZ:
      stats_upate.afd_update.afd_atb =
        AFD_TBL_50HZ;
    break;
    default:
      stats_upate.afd_update.afd_atb =
        AFD_TBL_OFF;
  }
  stats_upate.afd_update.afd_enable    = output->afd_enable;
  stats_upate.afd_update.afd_exec_once = output->afd_exec_once;
  stats_upate.afd_update.afd_monitor = output->afd_monitor;
  stats_upate.flag       = STATS_UPDATE_AFD;
  /* pack into an mct_event object*/
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = private->reserved_id;
  event.type     = MCT_EVENT_MODULE_EVENT;

  event.u.module_event.type              = MCT_EVENT_MODULE_STATS_AFD_UPDATE;
  event.u.module_event.module_event_data = (void *)(&stats_upate);
  MCT_PORT_EVENT_FUNC(port)(port, &event);
  return;
}

/** afd_port_start_threads
 *    @port: pointer to afd port
 *  Launch afd thread
 **/
static boolean afd_port_init_threads(mct_port_t *port)
{
  boolean     rc = TRUE;
  afd_port_private_t *private = port->port_private;

  private->thread_data = afd_thread_init();
  CDBG("%s private->thread_data: %p", __func__, private->thread_data);
  if (private->thread_data == NULL) {
    CDBG_ERROR("%s private->thread_data is NULL", __func__);
    rc = FALSE;
  }
  return rc;
}

/** afd_port_start_threads
 *    @port: pointer to afd port
 *  Launch afd thread
 **/
static boolean afd_port_start_threads(mct_port_t *port)
{
  boolean     rc = FALSE;
  afd_port_private_t *private = port->port_private;

  if (private->thread_data != NULL) {
    rc = afd_thread_start(port);
    if (rc == FALSE) {
      afd_thread_deinit(port);
    }
  }
  CDBG("%s: Start afd thread", __func__);
  return rc;
}

/** afd_port_check_session_id
 *    @d1: session+stream identity
 *    @d2: session+stream identity
 *
 *  To find out if both identities are matching;
 *  Return TRUE if matches.
 **/
static boolean afd_port_check_session_id(void *d1, void *d2)
{
  unsigned int v1, v2;
  v1 = *((unsigned int *)d1);
  v2 = *((unsigned int *)d2);

  return (((v1 & 0xFFFF0000) ==
    (v2 & 0xFFFF0000)) ? TRUE : FALSE);
}

/** afd_port_proc_downstream_event:
 *    @port : afd port pointer.
 *    @event: mct event for afd module
 *
 *  Send downstream event to afd algorithm
 **/
static boolean afd_port_proc_downstream_event(mct_port_t *port, mct_event_t *event)
{
  boolean rc = TRUE;

  afd_port_private_t *private = (afd_port_private_t *)(port->port_private);
  mct_event_module_t *mod_evt = &(event->u.module_event);

  switch (mod_evt->type) {
  case MCT_EVENT_MODULE_SET_RELOAD_CHROMATIX:
  case MCT_EVENT_MODULE_SET_CHROMATIX_PTR: {
    modulesChromatix_t *mod_chrom =
      (modulesChromatix_t *)mod_evt->module_event_data;
    afd_thread_msg_t *afd_msg   =
        (afd_thread_msg_t *)malloc(sizeof(afd_thread_msg_t));
    if (afd_msg == NULL)
      return FALSE;
    memset(afd_msg, 0, sizeof(afd_thread_msg_t));

    afd_msg->type = MSG_AFD_SET;

    afd_msg->u.afd_set_parm.type = AFD_SET_PARAM_INIT_CHROMATIX;
    afd_msg->u.afd_set_parm.u.init_param.chromatix =  mod_chrom->chromatixPtr;
    rc = afd_thread_en_q_msg(private->thread_data, afd_msg);
  } /* case MCT_EVENT_MODULE_SET_CHROMATIX_PTR */
    break;
  case MCT_EVENT_MODULE_START_STOP_STATS_THREADS: {
    uint8_t *start_flag = (uint8_t*)(mod_evt->module_event_data);
    CDBG("%s MCT_EVENT_MODULE_START_STOP_STATS_THREADS start_flag: %d",
      __func__,*start_flag);

    if (*start_flag) {
      if (afd_port_start_threads(port) == FALSE) {
        CDBG("%s: afd thread start failed", __func__);
        rc = FALSE;
      }
    } else {
      if (private->thread_data) {
        afd_thread_stop(private->thread_data);
      }
    }
  }
    break;
  case MCT_EVENT_MODULE_SET_STREAM_CONFIG: {
    afd_thread_msg_t *afd_msg   =
      (afd_thread_msg_t *)malloc(sizeof(afd_thread_msg_t));
    if (afd_msg == NULL)
      return FALSE;

    sensor_out_info_t * sensor_info = (sensor_out_info_t *)(mod_evt->module_event_data);
    memset(afd_msg, 0 , sizeof(afd_thread_msg_t));
    afd_msg->type = MSG_AFD_SET;
    afd_msg->u.afd_set_parm.type = AFD_SET_SENSOR_PARAM;
    afd_msg->u.afd_set_parm.u.aec_af_data.max_preview_fps =
      sensor_info->max_fps * 0x00000100;
    afd_msg->u.afd_set_parm.u.aec_af_data.preview_fps =
      sensor_info->max_fps * 0x00000100;
    afd_msg->u.afd_set_parm.u.aec_af_data.preview_linesPerFrame =
      sensor_info->fl_lines;
    afd_msg->u.afd_set_parm.u.aec_af_data.sen_dim_height =
      sensor_info->dim_output.height;
    rc = afd_thread_en_q_msg(private->thread_data, afd_msg);
  }/* MCT_EVENT_MODULE_SET_STREAM_CONFIG*/
  break;

  case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {
    afd_thread_msg_t *afd_msg   =
        (afd_thread_msg_t *)malloc(sizeof(afd_thread_msg_t));
    if (afd_msg == NULL)
      return FALSE;
    memset(afd_msg, 0, sizeof(afd_thread_msg_t));

    stats_update_t *stats_update =
      (stats_update_t *)mod_evt->module_event_data;

    if (!stats_update || stats_update->flag != STATS_UPDATE_AEC) {
      rc = FALSE;
      free(afd_msg);
      break;
    }

    afd_msg->type = MSG_AFD_SET;
    afd_msg->u.afd_set_parm.type = AFD_SET_AEC_PARAM;
    afd_msg->u.afd_set_parm.u.aec_af_data.aec_settled = stats_update->aec_update.settled;
    afd_msg->u.afd_set_parm.u.aec_af_data.band_50hz_gap = stats_update->aec_update.band_50hz_gap;
    afd_msg->u.afd_set_parm.u.aec_af_data.cur_line_cnt = stats_update->aec_update.linecount;
    afd_msg->u.afd_set_parm.u.aec_af_data.max_line_cnt = stats_update->aec_update.max_line_cnt;
    switch (stats_update->aec_update.cur_atb) {
      case   STATS_PROC_ANTIBANDING_OFF:
        afd_msg->u.afd_set_parm.u.aec_af_data.aec_atb =
          AFD_TBL_OFF;
        break;
      case STATS_PROC_ANTIBANDING_60HZ:
        afd_msg->u.afd_set_parm.u.aec_af_data.aec_atb =
          AFD_TBL_60HZ;
        break;
      case STATS_PROC_ANTIBANDING_50HZ:
        afd_msg->u.afd_set_parm.u.aec_af_data.aec_atb =
          AFD_TBL_50HZ;
        break;
      default:
        afd_msg->u.afd_set_parm.u.aec_af_data.aec_atb =
          AFD_TBL_OFF;
    }
    rc = afd_thread_en_q_msg(private->thread_data, afd_msg);
  }
    break;

  case MCT_EVENT_MODULE_STATS_AF_UPDATE: {
    #if 0
    stats_update_t *stats_update =
      (stats_update_t *)mod_evt->module_event_data;
    afd_thread_msg_t *afd_msg   =
        (afd_thread_msg_t *)malloc(sizeof(afd_thread_msg_t));
    if (afd_msg == NULL)
      return FALSE;

    if (!stats_update || stats_update->flag != STATS_UPDATE_AF) {
      rc = FALSE;
      free(afd_msg);
      break;
    }

    afd_msg->type = MSG_AFD_SET;
    afd_msg->u.afd_set_parm.type = AFD_SET_AF_PARAM;
    FIXMEEEE
    afd_msg->u.afd_set_parm.u.aec_af_data.af_active = stats_update->af_update.af_active;
    afd_msg->u.afd_set_parm.u.aec_af_data.cont_af_enabled = stats_update->af_update.cont_af_enabled;
    rc = afd_thread_en_q_msg(private->thread_data, afd_msg);
    #endif
  }
    break;
  case MCT_EVENT_MODULE_STATS_DATA: {
    mct_event_stats_isp_t *stats_event ;
    stats_event =(mct_event_stats_isp_t *)(mod_evt->module_event_data);
    if (stats_event) {
      if(!(stats_event->stats_mask & (1 << MSM_ISP_STATS_RS))) {
        return TRUE;
      }
      afd_thread_msg_t *afd_msg   =
        (afd_thread_msg_t *)malloc(sizeof(afd_thread_msg_t));
      if (afd_msg == NULL)
        return FALSE;
      memset(afd_msg, 0 , sizeof(afd_thread_msg_t));
      stats_t * afd_stats = (stats_t *)calloc(1, sizeof(stats_t));
      if(afd_stats == NULL) {
        free(afd_msg);
        break;
      }
      afd_msg->u.stats = afd_stats;
      afd_msg->type = MSG_AFD_STATS;
      afd_stats->stats_type_mask |= STATS_RS;
      memcpy(&afd_stats->yuv_stats.q3a_rs_stats, stats_event->stats_data[MSM_ISP_STATS_RS].stats_buf,
        sizeof(q3a_rs_stats_t));
      rc = afd_thread_en_q_msg(private->thread_data, afd_msg);
    } /* if(stats_event)*/

  } /* MCT_EVENT_MODULE_STATS_DATA*/
  break;
  default:
    break;
  } /* switch (mod_evt->type) */

  return rc;
}
/** afd_port_proc_fill_antibanding_parm
 *
 **/
static boolean afd_port_proc_fill_antibanding_parm(afd_thread_msg_t *afd_msg,
  cam_antibanding_mode_type mod)
{
  int rc = TRUE;
  switch(mod){
    case  CAM_ANTIBANDING_MODE_OFF:
      afd_msg->u.afd_set_parm.u.set_enable.afd_enable = FALSE;
      break;
    case CAM_ANTIBANDING_MODE_60HZ:
      afd_msg->u.afd_set_parm.u.set_enable.afd_enable = FALSE;
      afd_msg->u.afd_set_parm.u.set_enable.afd_mode = AFD_TYPE_60HZ;
      break;
    case CAM_ANTIBANDING_MODE_50HZ:
      afd_msg->u.afd_set_parm.u.set_enable.afd_enable = FALSE;
      afd_msg->u.afd_set_parm.u.set_enable.afd_mode = AFD_TYPE_50HZ;
      break;
    case CAM_ANTIBANDING_MODE_AUTO:
      afd_msg->u.afd_set_parm.u.set_enable.afd_enable = TRUE;
      afd_msg->u.afd_set_parm.u.set_enable.afd_mode = AFD_TYPE_AUTO;
      break;
    case CAM_ANTIBANDING_MODE_AUTO_50HZ:
      afd_msg->u.afd_set_parm.u.set_enable.afd_enable = TRUE;
      afd_msg->u.afd_set_parm.u.set_enable.afd_mode = AFD_TYPE_AUTO_50HZ;
      break;
    case CAM_ANTIBANDING_MODE_AUTO_60HZ:
      afd_msg->u.afd_set_parm.u.set_enable.afd_enable = TRUE;
      afd_msg->u.afd_set_parm.u.set_enable.afd_mode = AFD_TYPE_AUTO_60HZ;
      break;
    default:
      rc = FALSE;
    } /* switch(mod)*/
  return rc;
}
/** afd_port_proc_downstream_ctrl
 *
 **/
static boolean afd_port_proc_downstream_ctrl(mct_port_t *port, mct_event_t *event)
{
  boolean rc = TRUE;

  afd_port_private_t  *private  = (afd_port_private_t *)(port->port_private);
  mct_event_control_t *mod_ctrl = &(event->u.ctrl_event);
  switch (mod_ctrl->type) {
  case MCT_EVENT_CONTROL_SET_PARM: {
     stats_set_params_type *stat_parm = (stats_set_params_type *)mod_ctrl->control_event_data;
     afd_thread_msg_t *afd_msg =
       (afd_thread_msg_t *)malloc(sizeof(afd_thread_msg_t));
     if (afd_msg == NULL)
       return FALSE;
     memset(afd_msg, 0, sizeof(afd_thread_msg_t));

     switch (stat_parm->param_type) {
     case STATS_SET_AFD_PARAM: {
       afd_msg->type                =  MSG_AFD_SET;
       afd_msg->u.afd_set_parm.type =  AFD_SET_ENABLE;
       rc = afd_port_proc_fill_antibanding_parm(afd_msg, stat_parm->u.afd_param);
     }
     break;
     case STATS_SET_COMMON_PARAM: {
       stats_common_set_parameter_t *common_param =
         &(stat_parm->u.common_param);
       if (common_param->type == COMMON_SET_PARAM_STATS_DEBUG_MASK) {
         afd_msg->type = MSG_AFD_SET;
         afd_msg->u.afd_set_parm.type = AFD_SET_STATS_DEBUG_MASK;
         afd_msg->u.afd_set_parm.u.stats_debug_mask = common_param->u.stats_debug_mask;
         rc = TRUE;
       } else {
         rc = FALSE;
       }
     }
     break;
     default:
     break;
     }

     if (rc == TRUE) {
       rc = afd_thread_en_q_msg(private->thread_data, afd_msg);
     } else
       free(afd_msg);
     break;
  } /* case MCT_EVENT_CONTROL_SET_PARM*/
  default:
    break;
  }
  return rc;
}

/** afd_port_event
 *    @port:  this port from where the event should go
 *    @event: event object to send upstream or downstream
 *
 *  Because AFD module works no more than a sink module,
 *  hence its upstream event will need a little bit processing.
 *
 *  Return TRUE for successful event processing.
 **/
static boolean afd_port_event(mct_port_t *port, mct_event_t *event)
{
  boolean rc = FALSE;
  afd_port_private_t *private;
  /* sanity check */
  if (!port || !event)
    return FALSE;

  private = (afd_port_private_t *)(port->port_private);
  if (!private)
    return FALSE;

  /* sanity check: ensure event is meant for port with same identity*/
  if ((private->reserved_id & 0xFFFF0000) != (event->identity & 0xFFFF0000))
    return FALSE;

  switch (MCT_EVENT_DIRECTION(event)) {

  case MCT_EVENT_DOWNSTREAM: {

    switch (event->type) {
    case MCT_EVENT_MODULE_EVENT: {
      rc= afd_port_proc_downstream_event(port, event);
    } /* case MCT_EVENT_MODULE_EVENT */
      break;

    case MCT_EVENT_CONTROL_CMD: {
      rc = afd_port_proc_downstream_ctrl(port,event);
    }
      break;

    default:
      break;
    }
  } /* case MCT_EVENT_TYPE_DOWNSTREAM */
    break;

  case MCT_EVENT_UPSTREAM: {
    mct_port_t *peer = MCT_PORT_PEER(port);
    MCT_PORT_EVENT_FUNC(peer)(peer, event);
  }
    break;

  default:
    rc = FALSE;
    break;
  }
  return rc;
}

/** afd_port_set_caps
 *    @port: port object which the caps to be set;
 *    @caps: this port's capability.
 *
 *  Return TRUE if it is valid soruce port.
 *
 *  Function overwrites a ports capability.
 **/
static boolean afd_port_set_caps(mct_port_t *port,
  mct_port_caps_t *caps)
{
  if (strcmp(MCT_PORT_NAME(port), "afd_sink")){
    ALOGE("%s:%d Error", __func__, __LINE__);
    return FALSE;
  }

  port->caps = *caps;
  return TRUE;
}

/** afd_port_check_caps_reserve
 *    @port: this interface module's port;
 *    @peer_caps: the capability of peer port which wants to match
 *                interterface port;
 *    @stream_info:
 *
 *  Stats modules are pure s/w software modules, and every port can
 *  support one identity. If the identity is different, support
 *  can be provided via create a new port. Regardless source or
 *  sink port, once capabilities are matched,
 *  - If this port has not been used, it can be supported;
 *  - If the requested stream is in existing identity, return
 *    failure
 *  - If the requested stream belongs to a different session, the port
 *  can not be used.
 **/
static boolean afd_port_check_caps_reserve(mct_port_t *port, void *caps,
  void *s_info)
{
  boolean            rc = FALSE;
  mct_port_caps_t    *port_caps;
  afd_port_private_t *private;
  mct_stream_info_t *stream_info =
    (mct_stream_info_t *)s_info;
  MCT_OBJECT_LOCK(port);
  if (!port || !caps || !stream_info ||
      strcmp(MCT_OBJECT_NAME(port), "afd_sink")) {
    rc = FALSE;
    goto reserve_done;
  }

  port_caps = (mct_port_caps_t *)caps;
  if (port_caps->port_caps_type != MCT_PORT_CAPS_STATS) {
    rc = FALSE;
    goto reserve_done;
  }

  private = (afd_port_private_t *)port->port_private;
  switch (private->state) {
  case AFD_PORT_STATE_LINKED: {
    if ((private->reserved_id & 0xFFFF0000) ==
      (stream_info->identity & 0xFFFF0000))
      rc = TRUE;
  }
    break;

  case AFD_PORT_STATE_CREATED:
  case AFD_PORT_STATE_UNRESERVED: {

    private->reserved_id = stream_info->identity;
    private->stream_type = stream_info->stream_type;
    private->state       = AFD_PORT_STATE_RESERVED;
    rc = TRUE;
  }
    break;

  case AFD_PORT_STATE_RESERVED:
    if ((private->reserved_id & 0xFFFF0000) ==
      (stream_info->identity & 0xFFFF0000))
      rc = TRUE;
    break;

  default:
    rc = FALSE;
    break;
  }

reserve_done:
  MCT_OBJECT_UNLOCK(port);
  return rc;
}

/** afd_port_check_caps_unreserve
 *    @port: this port object to remove the session/stream;
 *    @identity: session+stream identity.
 *
 *    Return FALSE if the identity is not existing.
 *
 *  This function frees the identity from port's children list.
 **/
static boolean afd_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  afd_port_private_t *private;
  boolean rc = FALSE;
  if (!port || strcmp(MCT_OBJECT_NAME(port), "afd_sink"))
    return FALSE;

  private = (afd_port_private_t *)port->port_private;
  if (!private)
    return FALSE;

  MCT_OBJECT_LOCK(port);
  if ((private->state == AFD_PORT_STATE_UNLINKED   ||
    private->state == AFD_PORT_STATE_RESERVED ||
    private->state == AFD_PORT_STATE_LINKED) &&
    ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000))) {

    if (!MCT_OBJECT_REFCOUNT(port)) {
      private->state       = AFD_PORT_STATE_UNRESERVED;
      private->reserved_id = (private->reserved_id & 0xFFFF0000);
    }
    rc                   = TRUE;
  } else {
    rc = FALSE;
  }
  MCT_OBJECT_UNLOCK(port);
  return rc;
}

/** afd_port_ext_link
 *    @identity:  Identity of session/stream
 *    @port: SINK of AFD ports
 *    @peer: For AFD sink- peer is STATS sink port
 *
 *  Set AFD port's external peer port, which is STATS module's
 *  sink port.
 *
 *  Return TRUE on success.
 **/
static boolean afd_port_ext_link(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  boolean rc = FALSE, thread_init = FALSE;
  afd_port_private_t  *private;
  mct_event_t         event;
  if (strcmp(MCT_OBJECT_NAME(port), "afd_sink")){
    return FALSE;
  }

  private = (afd_port_private_t *)port->port_private;
  if (!private) {
    return FALSE;
  }

  MCT_OBJECT_LOCK(port);
  switch (private->state) {
  case AFD_PORT_STATE_RESERVED:
  case AFD_PORT_STATE_UNLINKED:
    if ((private->reserved_id & 0xFFFF0000) != (identity & 0xFFFF0000)) {
      break;
    }
  /* Fall through */
  case AFD_PORT_STATE_CREATED:
    thread_init = TRUE;
    rc = TRUE;
    break;

  case AFD_PORT_STATE_LINKED:
    if ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000)) {
      rc = TRUE;
      thread_init = FALSE;
    }
    break;

  default:
    break;
  }

  if (rc == TRUE) {

    if (thread_init == TRUE) {
      if (afd_port_init_threads(port) == FALSE) {
        rc = FALSE;
        goto afd_ext_link_done;
      }
    }

    private->state = AFD_PORT_STATE_LINKED;
    MCT_PORT_PEER(port) = peer;
    MCT_OBJECT_REFCOUNT(port) += 1;
  }

afd_ext_link_done:
  MCT_OBJECT_UNLOCK(port);
  return rc;
}

/** afd_port_ext_unlink
 *
 *  @identity: Identity of session/stream
 *  @port: afd module's sink port
 *  @peer: peer of stats sink port
 *
 * This funtion unlink the peer ports of stats sink, src ports
 * and its peer submodule's port
 *
 **/
static void afd_port_ext_unlink(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  afd_port_private_t *private;
  if (!port || !peer || MCT_PORT_PEER(port) != peer)
    return;

  private = (afd_port_private_t *)port->port_private;
  if (!private)
    return;

  MCT_OBJECT_LOCK(port);
  if (private->state == AFD_PORT_STATE_LINKED &&
    ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000))){
    MCT_OBJECT_REFCOUNT(port) -= 1;
    if (!MCT_OBJECT_REFCOUNT(port)){
      CDBG("%s: afd_data=%p", __func__, private->thread_data);
      private->state = AFD_PORT_STATE_UNLINKED;
      afd_thread_deinit(port);
      MCT_PORT_PEER(port) = NULL;
    }
  }

  MCT_OBJECT_UNLOCK(port);
  return;
}

/** afd_port_find_identity
 *
 **/
boolean afd_port_find_identity(mct_port_t *port, unsigned int identity)
{
  afd_port_private_t *private;

  if (!port)
      return FALSE;

  if (strcmp(MCT_OBJECT_NAME(port), "afd_sink"))
    return FALSE;

  private = port->port_private;

  if (private) {
    return ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000) ?
      TRUE : FALSE);
  }

  return FALSE;
}

/** afd_port_deinit
 *    @port: afd sink port
 *
 *  de-initialize one AFD sink port
 *
 *  Return nothing
 **/
void afd_port_deinit(mct_port_t *port)
{
  afd_port_private_t *private;

  if (!port)
    return;

  if(strcmp(MCT_OBJECT_NAME(port), "afd_sink"))
      return;

  private = port->port_private;
  if (private) {
      afd_destroy(private->afd_object.afd);
      free(port->port_private);
  }
}

/** afd_port_init
 *    @port: port object to be initialized
 *
 *  Port initialization, use this function to overwrite
 *  default port methods and install capabilities. Stats
 *  module should have ONLY sink port.
 *
 *  Return TRUE on success.
 **/
boolean afd_port_init(mct_port_t *port, unsigned int identity)
{
  mct_port_caps_t    caps;
  afd_port_private_t *private;
  mct_list_t         *list;

  private = malloc(sizeof(afd_port_private_t));
  if (private == NULL)
    return FALSE;
  memset(private, 0, sizeof(afd_port_private_t));

  /* initialize AFD object */
  AFD_INITIALIZE_LOCK(&private->afd_object);
  private->afd_object.set_parameters = afd_set_parameters;
  private->afd_object.get_parameters = afd_get_parameters;
  private->afd_object.process = afd_process;
  private->afd_object.afd_cb = afd_port_callback;
  private->afd_object.afd = afd_init();
  if (private->afd_object.afd == NULL) {
    free(private);
    return FALSE;
  }

  private->reserved_id = identity;
  private->state       = AFD_PORT_STATE_CREATED;

  port->port_private  = private;
  port->direction     = MCT_PORT_SINK;
  caps.port_caps_type = MCT_PORT_CAPS_STATS;
  caps.u.stats.flag   = MCT_PORT_CAP_STATS_CS_RS;

  mct_port_set_event_func(port, afd_port_event);
  mct_port_set_set_caps_func(port, afd_port_set_caps);
  mct_port_set_ext_link_func(port, afd_port_ext_link);
  mct_port_set_unlink_func(port, afd_port_ext_unlink);
  mct_port_set_check_caps_reserve_func(port, afd_port_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port, afd_port_check_caps_unreserve);

  if (port->set_caps)
    port->set_caps(port, &caps);
  return TRUE;
}
