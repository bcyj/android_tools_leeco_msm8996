/* q3a_port.c
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 **/
#include "q3a_port.h"
#include "aec_port.h"
#include "awb_port.h"
#include "af_port.h"
#include "q3a_thread.h"
#include "stats_module.h"

#include "aec.h"
#include "awb.h"
#include "af.h"

#if Q3A_PORT_DEBUG
#undef CDBG
#define CDBG CDBG_ERROR
#endif

/* To preserve the aec and af state after TAF till scene does not change */
#define TAF_SAVE_AEC_AF 0

#define IS_Q3A_PORT_IDENTITY(port_private, identity) \
  ((port_private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000))

/* Time to wait after LED estimation sequence (AEC+AF) is complete - in ms*/
#define Q3A_PORT_LED_WAIT_TIME          3000

/* Definitions of local functions */
static boolean q3a_port_process_event_for_led_af(mct_port_t *port,
  mct_event_t *event);

/** q3a_port_start_threads
 *    @port:     module's port
 *    @identity: stream/session identity
 *    @type:     the type of the thread - AF or AECAWB
 *
 *  This function will start the thread handlers of the q3a subports.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean q3a_port_start_threads(mct_port_t *port,
  unsigned int identity, q3a_thread_type_t type)
{
  mct_event_t        event;
  q3a_port_private_t *private = port->port_private;

  switch (type) {
  case Q3A_THREAD_AECAWB: {
    q3a_thread_aecawb_data_t *aecawb_data;
    aecawb_data = private->aecawb_data;
    if (aecawb_data != NULL) {
      event.type      = MCT_EVENT_MODULE_EVENT;
      event.identity  = identity;
      event.direction = MCT_EVENT_DOWNSTREAM;

      event.u.module_event.type = MCT_EVENT_MODULE_STATS_GET_THREAD_OBJECT;
      event.u.module_event.module_event_data = (void *)aecawb_data;

      if (MCT_PORT_EVENT_FUNC(private->aec_port)(private->aec_port, &event) ==
        FALSE ||
        MCT_PORT_EVENT_FUNC(private->awb_port)(private->awb_port, &event) ==
        FALSE) {
        CDBG_ERROR("%s: NOT Start AECAWB thread", __func__);
        q3a_thread_aecawb_deinit(aecawb_data);
        return FALSE;
      }

      CDBG("%s: Start AEAWB thread", __func__);
      q3a_thread_aecawb_start(aecawb_data);
    }
  } /* case Q3A_THREAD_AECAWB */
    break;

  case Q3A_THREAD_AF: {
    q3a_thread_af_data_t *af_data;
    af_data = private->af_data;
    if (af_data != NULL) {
      event.type       = MCT_EVENT_MODULE_EVENT;
      event.identity   = identity;
      event.direction  = MCT_EVENT_DOWNSTREAM;

      event.u.module_event.type = MCT_EVENT_MODULE_STATS_GET_THREAD_OBJECT;
      event.u.module_event.module_event_data = (void *)af_data;

      if (MCT_PORT_EVENT_FUNC(private->af_port)(private->af_port, &event) ==
        FALSE) {
        CDBG_ERROR("%s: AF event handler returned FALSE", __func__);
        q3a_thread_af_deinit(af_data);
        return FALSE;
      }
      CDBG("%s: Start AF thread", __func__);
      q3a_thread_af_start(af_data);
    }
  } /* case Q3A_THREAD_AF */
    break;

  default: {
    CDBG_ERROR("%s: Invalid start thread type", __func__);
    return FALSE;
  } /* default */
    break;
  } /* switch (type) */

  return TRUE;
} /* q3a_port_start_threads */

/** q3a_port_skip_bad_frame
 *  Inform HAL to skip current frame to avoid flicker
 **/
static void q3a_port_skip_bad_frame(
  mct_port_t  *port, uint32_t start_idx, uint32_t skip_cnt)
{
  mct_event_t event;
  mct_bus_msg_t bus_msg;
  q3a_port_private_t *private;
  cam_frame_idx_range_t range;
  int size;
  if (!port || !skip_cnt) {
    CDBG_ERROR("%s: input error, port: %p, skip_cnt: %d",
      __func__, port, skip_cnt);
    return;
  }
  CDBG("%s start_idx: %d, skip_count: %d", __func__,
    start_idx, skip_cnt);
  private = port->port_private;
  range.min_frame_idx = start_idx;
  range.max_frame_idx = range.min_frame_idx + skip_cnt - 1;
  bus_msg.sessionid = (private->reserved_id >> 16);
  bus_msg.type = MCT_BUS_MSG_FRAME_INVALID;
  bus_msg.msg = &range;
  size = (int)sizeof(cam_frame_idx_range_t);
  bus_msg.size = size;
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = private->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)(&bus_msg);
  MCT_PORT_EVENT_FUNC(port)(port, &event);
}


/** q3a_port_send_event_to_aec_port
  *    @port:  module's port
  *    @event: the event object to be sent downstream
  *
  *  This function will send an event to the AEC port.
  *
  *  Return TRUE on success, FALSE on failure.
  **/
static boolean q3a_port_send_event_to_aec_port(mct_port_t *port,
  mct_event_t *event)
{
  boolean rc = FALSE;
  q3a_port_private_t *private = port->port_private;

  CDBG("%s:%d E", __func__, __LINE__);

  if (MCT_PORT_EVENT_FUNC(private->aec_port)) {
    rc = MCT_PORT_EVENT_FUNC(private->aec_port)(private->aec_port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: AEC event handler returned failure!", __func__);
    }
  }

  CDBG("%s:%d X", __func__, __LINE__);
  return rc;
}

/** q3a_port_send_event_to_af_port
 *    @event: the event object to be sent downstream
 *
 *  This function will send an event to the AF port.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean q3a_port_send_event_to_af_port(mct_port_t *port,
  mct_event_t *event)
{
  boolean rc = FALSE;
  q3a_port_private_t *private = port->port_private;

  CDBG("%s:%d E", __func__, __LINE__);

  if (MCT_PORT_EVENT_FUNC(private->af_port)) {
    rc = MCT_PORT_EVENT_FUNC(private->af_port)(private->af_port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: AF event handler returned failure!", __func__);
    }
  }

  CDBG("%s:%d X", __func__, __LINE__);
  return rc;
}

/** q3a_port_send_event_downstream
 *    @port:  this is be the module's port
 *    @event: the event object to be sent downstream
 *
 *  This function will send the received event downstream to the
 *  q3a submodules (AF, AWB and AEC).
 *
 *  Return TRUE on success, FALSE on event handler failure.
 **/
static boolean q3a_port_send_event_downstream(mct_port_t *port,
  mct_event_t *event)
{
  boolean rc = FALSE;
  q3a_port_private_t *private = port->port_private;

  CDBG("%s:%d E", __func__, __LINE__);

  if (MCT_PORT_EVENT_FUNC(private->aec_port)) {
    rc = MCT_PORT_EVENT_FUNC(private->aec_port)(private->aec_port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: AEC event handler returned failure!", __func__);
      goto send_done;
    }
  }

  if (MCT_PORT_EVENT_FUNC(private->awb_port)) {
    rc = MCT_PORT_EVENT_FUNC(private->awb_port)(private->awb_port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: AWB event handler returned failure!", __func__);
      goto send_done;
    }
  }

  if (MCT_PORT_EVENT_FUNC(private->af_port)) {
    rc = MCT_PORT_EVENT_FUNC(private->af_port)(private->af_port, event);
    if (rc == FALSE) {
      CDBG_ERROR("%s: AF event handler returned failure!", __func__);
      goto send_done;
    }
  }

send_done:
  CDBG("%s:%d X rc=%d", __func__, __LINE__, rc);
  return rc;
} /* q3a_port_send_event_downstream */

/** q3a_port_proc_upstream_mod_event
 *    @port:  this is be the module's port
 *    @event: the event object to be sent upstream
 *
 *  This function will send the received event upstream and will propagate
 *  it to the other q3a submodules.
 *
 *  Return TRUE on success, FALSE on event handler failure.
 **/
static boolean q3a_port_proc_upstream_mod_event(mct_port_t *port,
  mct_event_t *event)
{
  boolean            rc = TRUE;
  q3a_port_private_t *private = port->port_private;

  CDBG("%s: E stats->type=%d", __func__,event->u.module_event.type);
  /* Always send UPDATE or REQUEST upstream first */
  mct_port_send_event_to_peer(port, event);

  /* Check to see if need to redirect this event to sub-ports */
  event->direction = MCT_EVENT_DOWNSTREAM;
  switch (event->u.module_event.type) {
  case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {
    CDBG("%s: to AWB, awb_port=%p, func=%p, event=%p", __func__,
      private->awb_port, MCT_PORT_EVENT_FUNC(private->awb_port), event);
    if (MCT_PORT_EVENT_FUNC(private->awb_port)) {
      MCT_PORT_EVENT_FUNC(private->awb_port)(private->awb_port, event);
    }

    if (MCT_PORT_EVENT_FUNC(private->af_port)) {
      MCT_PORT_EVENT_FUNC(private->af_port)(private->af_port, event);
    }
  }
    break;

  case MCT_EVENT_MODULE_FAST_AEC_CONVERGE_ACK: {
    CDBG("%s: to AWB, awb_port=%p, func=%p, event=%p", __func__,
         private->awb_port, MCT_PORT_EVENT_FUNC(private->awb_port), event);
    if (MCT_PORT_EVENT_FUNC(private->awb_port))
      MCT_PORT_EVENT_FUNC(private->awb_port)(private->awb_port, event);

    if (MCT_PORT_EVENT_FUNC(private->af_port))
      MCT_PORT_EVENT_FUNC(private->af_port)(private->af_port, event);
  }
    break;

  case MCT_EVENT_MODULE_STATS_AWB_UPDATE: {
    if (MCT_PORT_EVENT_FUNC(private->aec_port)) {
      MCT_PORT_EVENT_FUNC(private->aec_port)(private->aec_port, event);
    }

    if (MCT_PORT_EVENT_FUNC(private->af_port)) {
      MCT_PORT_EVENT_FUNC(private->af_port)(private->af_port, event);
    }
  }
    break;

  case MCT_EVENT_MODULE_STATS_AF_UPDATE: {
    if (MCT_PORT_EVENT_FUNC(private->aec_port)) {
      MCT_PORT_EVENT_FUNC(private->aec_port)(private->aec_port, event);
    }

    if (MCT_PORT_EVENT_FUNC(private->awb_port)) {
      MCT_PORT_EVENT_FUNC(private->awb_port)(private->awb_port, event);
    }
  }
    break;

  default: {
    rc = FALSE;
  }
    break;
  }

  CDBG("%s: X", __func__);
  return rc;
} /* q3a_port_proc_upstream_mod_event */

/** q3a_port_is_led_needed
 *    @port:  this is the module's port
 *
 *  This function will check if the LED is needed to assist the AF
 *
 *  Return TRUE if LED is needed, FALSE if not.
 **/
static boolean q3a_port_is_led_needed(mct_port_t *port)
{
  q3a_port_private_t *private = port->port_private;

  return private->af_led_data.led_af_needed;
}

/** q3a_port_update_led_af_to_sensor
  *   @port:  this is the module's port
  *   @type:  this is the led af type
  *
  * This function will send the led af type to sensor
  *
  *  Return TRUE on success, FALSE on failure.
 **/
static boolean q3a_port_update_led_af_to_sensor(mct_port_t *port, boolean flag)
{
  boolean rc = TRUE;
  q3a_port_private_t *private = port->port_private;
  mct_event_t q3a_event;
  int led_af_flag = flag;
  q3a_event.direction           = MCT_EVENT_UPSTREAM;
  q3a_event.identity            = private->reserved_id;
  q3a_event.type                = MCT_EVENT_MODULE_EVENT;
  q3a_event.u.module_event.type = MCT_EVENT_MODULE_LED_AF_UPDATE;
  q3a_event.u.module_event.current_frame_id = private->cur_sof_id;
  q3a_event.u.module_event.module_event_data = &led_af_flag;
  rc = mct_port_send_event_to_peer(port, &q3a_event);
  return rc;
}

/** q3a_port_af_wait_for_aec_update
 *    @port: this is the module's port
 *    @wait: if AF should wait
 *
 *  This function will send event to the AF telling it to wait or to stop
 *  waiting for the AEC to converge.
 *
 *  Return TRUE on success, FALSE on event handler failure.
 **/
static boolean q3a_port_af_wait_for_aec_update(mct_port_t *port, boolean wait)
{
  boolean                rc = TRUE;
  q3a_port_private_t     *private = port->port_private;
  stats_set_params_type  stats_set_param;
    mct_event_t          event;

  CDBG("%s: Wait/Don't wait for AEC: %d", __func__, wait);
  event.u.ctrl_event.control_event_data =
    (stats_set_params_type*)&stats_set_param;

  event.identity  = private->reserved_id;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  stats_set_param.param_type = STATS_SET_Q3A_PARAM;
  stats_set_param.u.q3a_param.type = Q3A_SET_AF_PARAM;
  stats_set_param.u.q3a_param.u.af_param.type = AF_SET_PARAM_WAIT_FOR_AEC_EST;
  stats_set_param.u.q3a_param.u.af_param.u.af_wait_for_aec_est = wait;

  rc = q3a_port_send_event_to_af_port(port, &event);

  return rc;
}

/** q3a_port_request_aec_est_for_af
  *    @port: this is the module's port
  *    @mode: if AEC should do a LED estimation helping the AF in a very low
  *           lighting condition
  *
  *  This function will send event to the AEC to do LED estimation for AF.
  *
  *  Return TRUE on success, FALSE on event handler failure.
 **/
static boolean q3a_port_request_aec_est_for_af(mct_port_t *port, boolean mode)
{
  boolean               rc = TRUE;
  q3a_port_private_t    *private = port->port_private;
  stats_set_params_type stats_set_param;
  mct_event_t           event;

  CDBG("%s: Request AEC for AF: %d", __func__, mode);
  event.u.ctrl_event.control_event_data =
    (stats_set_params_type*)&stats_set_param;

  event.identity  = private->reserved_id;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  stats_set_param.param_type = STATS_SET_Q3A_PARAM;
  stats_set_param.u.q3a_param.type = Q3A_SET_AEC_PARAM;
  stats_set_param.u.q3a_param.u.aec_param.type =
    AEC_SET_PARAM_DO_LED_EST_FOR_AF;
  stats_set_param.u.q3a_param.u.aec_param.u.est_for_af = mode;

  rc = q3a_port_send_event_to_aec_port(port, &event);

  return rc;
}


static boolean q3a_port_request_aec_roi_off(mct_port_t *port)
{
  boolean                 rc = TRUE;
  q3a_port_private_t      *private = port->port_private;
  stats_set_params_type   stats_set_param;
  mct_event_t             event;

  CDBG("%s: called", __func__);
  event.u.ctrl_event.control_event_data =
    (stats_set_params_type*)&stats_set_param;
  event.identity = private->reserved_id;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  stats_set_param.param_type = STATS_SET_Q3A_PARAM;
  stats_set_param.u.q3a_param.type = Q3A_SET_AEC_PARAM;
  stats_set_param.u.q3a_param.u.aec_param.type =
    AEC_SET_PARAM_ROI;
  stats_set_param.u.q3a_param.u.aec_param.u.aec_roi.enable = FALSE;

  rc = q3a_port_send_event_to_aec_port(port, &event);

  return rc;
}
/** q3a_port_request_led_off
 *    @port:      this is the module's port
 *    @cancel_op: if this is a cancel AF operation
 *
 *  This function will return the saved AEC update information to the sensor.
 *  It the operation is not a cancel AF, it will send the stored estimated
 *  values. If the operation is cancel, it will send the stored values before
 *  the LED was turned on.
 *
 *  Return TRUE on success, FALSE on event handler failure.
 **/
static boolean q3a_port_request_led_off(mct_port_t *port,
  boolean cancel_op)
{
  boolean            rc = TRUE;
  q3a_port_private_t *private = port->port_private;

  if (cancel_op == TRUE) {
    /* Cancel operation. Restore last no_led values to sensor and tell it
     * to turn OFF the LED.
     */
    CDBG("%s: Send AEC estimation cancel to Sensor on next real update!",
      __func__);
    private->af_led_data.send_stored_no_led_data = TRUE;

    private->af_led_data.aec_no_led_data.aec_update.real_gain =
      private->af_led_data.aec_no_led_data.aec_update.led_off_gain;
    private->af_led_data.aec_no_led_data.aec_update.linecount =
      private->af_led_data.aec_no_led_data.aec_update.led_off_linecnt;

    CDBG("%s:%d Restoring: est_state:%d, gain:%f, lc:%d!", __func__, __LINE__,
    private->af_led_data.aec_no_led_data.aec_update.est_state,
    private->af_led_data.aec_no_led_data.aec_update.real_gain,
    private->af_led_data.aec_no_led_data.aec_update.linecount);
  } else {
    CDBG("%s: Send AEC estimation done to Sensor on next real update!",
      __func__);
    private->af_led_data.send_stored_update_data = TRUE;

    private->af_led_data.aec_update_data.aec_update.real_gain =
      private->af_led_data.aec_update_data.aec_update.led_off_gain;
    private->af_led_data.aec_update_data.aec_update.linecount =
      private->af_led_data.aec_update_data.aec_update.led_off_linecnt;

    CDBG("%s:%d Restoring: est_state:%d, gain:%f, lc:%d!", __func__, __LINE__,
    private->af_led_data.aec_update_data.aec_update.est_state,
    private->af_led_data.aec_update_data.aec_update.real_gain,
    private->af_led_data.aec_update_data.aec_update.linecount);
  }

  private->af_led_data.led_status = 0;

  return rc;
}

/** q3a_port_start_timer
 *    @port: this is the module's port
 *
 *  Once LED sequence is complete, we
 *  wait for sometime before resetting our state to idle.
 *  This helps to avoid the duplicate LED sequence pre-capture
 *  comes right after auto-focus.
 *
 *  Return nothing
 **/
static void q3a_port_start_timer(mct_port_t *port)
{
  q3a_port_private_t *private;
  int frames_to_wait = 0;

  private = (q3a_port_private_t *)(port->port_private);
  frames_to_wait =
    private->af_led_data.preview_fps * Q3A_PORT_LED_WAIT_TIME / 1000;
  pthread_mutex_lock(&private->af_led_data.timer_lock);
  private->af_led_data.led_wait_count = frames_to_wait;
  pthread_mutex_unlock(&private->af_led_data.timer_lock);
} /* q3a_port_start_timer */

/** q3a_port_check_timer
 *    @port: this is the module's port
 *
 *  Check if our timer is still on. If
 *  we have timed out move to IDLE state.
 *
 *  Return nothing
 **/
static void q3a_port_check_timer_and_update(mct_port_t *port)
{
  q3a_port_private_t *private = (q3a_port_private_t *)(port->port_private);
  boolean            send_event = FALSE;
  mct_event_t        q3a_event;

  pthread_mutex_lock(&private->af_led_data.timer_lock);
  if (private->af_led_data.led_wait_count > 0) {
    private->af_led_data.led_wait_count -= 1;
    CDBG("%s: Frames to wait before resetting the LED state: %d", __func__,
      private->af_led_data.led_wait_count);
    send_event =  FALSE;
  } else if (private->af_led_data.led_wait_count == 0) {
    private->af_led_data.led_wait_count = -1;
    send_event = TRUE;
  } else {
    send_event = FALSE;
  }
  pthread_mutex_unlock(&private->af_led_data.timer_lock);

  if (send_event) {
    /* Request Sensor to reset their LED state too */
    CDBG("%s: Timeout! Request to reset LED state (evid: %d)!",
      __func__, MCT_EVENT_MODULE_LED_STATE_TIMEOUT);
    q3a_event.direction           = MCT_EVENT_UPSTREAM;
    q3a_event.identity            = private->reserved_id;
    q3a_event.type                = MCT_EVENT_MODULE_EVENT;
    q3a_event.u.module_event.type = MCT_EVENT_MODULE_LED_STATE_TIMEOUT;
    q3a_event.u.module_event.current_frame_id = private->cur_sof_id;
    mct_port_send_event_to_peer(port, &q3a_event);

    /* Send downstream too */
    q3a_event.direction           = MCT_EVENT_DOWNSTREAM;
    q3a_port_send_event_downstream(port, &q3a_event);

    /* If we are in AF_DONE state now, time to move to IDLE state */
    pthread_mutex_lock(&private->af_led_data.state_lock);
    private->af_led_data.state = AF_WITH_LED_STATE_IDLE;
    pthread_mutex_unlock(&private->af_led_data.state_lock);
  }
} /* q3a_port_check_timer_and_update */

/** q3a_port_stop_timer
 *    @port: this is the module's port
 *
 *  Reset the timer
 *
 *  Return nothing
 **/
static void q3a_port_stop_timer(mct_port_t *port)
{
  q3a_port_private_t *private = (q3a_port_private_t *)(port->port_private);

  pthread_mutex_lock(&private->af_led_data.timer_lock);
  private->af_led_data.led_wait_count = 0;
  pthread_mutex_unlock(&private->af_led_data.timer_lock);

  q3a_port_check_timer_and_update(port);
} /* q3a_port_stop_timer */

/** q3a_port_process_event_for_led_af
 *    @port:  this is be the module's port
 *    @event: the event object to be sent downstream
 *
 *  This is the main function to handle the AF+LED feature. It will check
 *  if the LED is needed for the AF to focus and will start a sequence of
 *  events tracking current states to manage the feature.
 *
 *  Return TRUE if the event is sent downstream, FALSE if not.
 **/
static boolean q3a_port_process_event_for_led_af(mct_port_t *port,
  mct_event_t *event)
{
  boolean            rc = FALSE;
  q3a_port_private_t *private;

  private = (q3a_port_private_t *)(port->port_private);

  switch (MCT_EVENT_DIRECTION(event)) {
  /* In the upstream we track the Q3A submodules' status */
  case MCT_EVENT_UPSTREAM: {
    switch (event->type) {
    case MCT_EVENT_MODULE_EVENT: {
      switch(event->u.module_event.type) {
      case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {
        stats_update_t *stats_update =
          (stats_update_t *)event->u.module_event.module_event_data;
#if TAF_SAVE_AEC_AF
        if(private->aec_roi_enable) {
          if (stats_update->aec_update.luma_settled_cnt > 3){
            private->aec_settled = TRUE;
          }
          if (stats_update->aec_update.luma_settled_cnt == 0
            && private->aec_settled == TRUE){
            private->aec_ocsillate_cnt++;
            if (private->aec_ocsillate_cnt == 5){
              private->aec_roi_enable = FALSE;
              private->aec_settled = FALSE;
              q3a_port_request_aec_roi_off(port);
            }
          } else {
            private->aec_ocsillate_cnt=0;
          }
        }
#endif
        private->af_led_data.preview_fps =
          stats_update->aec_update.preview_fps;
        CDBG("%s: Preview-FPS: %d", __func__, private->af_led_data.preview_fps);
        if (stats_update->aec_update.est_state == AEC_EST_DONE_FOR_AF) {

          pthread_mutex_lock(&private->af_led_data.state_lock);
          if (private->af_led_data.state == AF_WITH_LED_STATE_AEC_RUNNING) {
            q3a_port_af_wait_for_aec_update(port, FALSE);
            private->af_led_data.state = AF_WITH_LED_STATE_AF_RUNNING;
          }
          pthread_mutex_unlock(&private->af_led_data.state_lock);

          private->af_led_data.aec_update_data = *stats_update;
          private->af_led_data.aec_update_data.aec_update.est_state =
            AEC_EST_DONE;

          CDBG("%s:%d Let AF start!", __func__, __LINE__);
        } else if (stats_update->aec_update.est_state == AEC_EST_OFF) {
          /* copy last AEC data when the LED is OFF */
          private->af_led_data.aec_no_led_data = *stats_update;
          private->af_led_data.aec_no_led_data.aec_update.est_state =
            AEC_EST_DONE;
        }

        private->af_led_data.led_af_needed = stats_update->aec_update.led_needed;
        private->af_led_data.flash_needed = stats_update->aec_update.flash_needed;

        /* if we are about to send the the EST_DONE event, send it here,
         * but get the current sof_id and stats_frm_id. This will sync sending
         * the update on the SOF with the right IDs.
         */
        if (private->af_led_data.send_stored_update_data == TRUE) {
          CDBG("%s:%d Restore AEC update data with LED estimation!", __func__,
            __LINE__);
          private->af_led_data.send_stored_update_data = FALSE;

          private->af_led_data.aec_update_data.aec_update.sof_id =
            stats_update->aec_update.sof_id;
          private->af_led_data.aec_update_data.aec_update.stats_frm_id =
            stats_update->aec_update.stats_frm_id;
          /* switch the pointer of the event data to point to our saved data */
          memcpy(&private->af_led_data.aec_output_data,
            &private->af_led_data.aec_update_data, sizeof(stats_update_t));
          event->u.module_event.module_event_data =
            &private->af_led_data.aec_output_data;
        } else if (private->af_led_data.send_stored_no_led_data == TRUE) {
          CDBG("%s:%d Restore AEC update data with NO LED estimation!",
            __func__, __LINE__);
          private->af_led_data.send_stored_no_led_data = FALSE;

          private->af_led_data.aec_no_led_data.aec_update.sof_id =
            stats_update->aec_update.sof_id;
          private->af_led_data.aec_no_led_data.aec_update.stats_frm_id =
            stats_update->aec_update.stats_frm_id;

          /* switch the pointer of the event data to point to our saved data */
          memcpy(&private->af_led_data.aec_output_data,
            &private->af_led_data.aec_no_led_data, sizeof(stats_update_t));
          event->u.module_event.module_event_data =
            &private->af_led_data.aec_output_data;
        }
        if (stats_update->aec_update.est_state == AEC_EST_DONE) {
          /* Block regular aec update for one frame to first send event for skipping preview frames */
          private->af_led_data.send_stored_update_data = TRUE;
          private->af_led_data.aec_update_data = *stats_update;
          q3a_port_skip_bad_frame(port, private->cur_sof_id,
            NUMBER_OF_SKIP_FRAMES_PRE_FLASH_END);
          rc = TRUE;
        }
      }
        break;

      case MCT_EVENT_MODULE_STATS_AF_UPDATE: {
        stats_update_t *stats_update =
          (stats_update_t *)event->u.module_event.module_event_data;

      }
        break;

      case MCT_EVENT_MODULE_STATS_POST_TO_BUS: {
        mct_bus_msg_t *bus_msg =
          (mct_bus_msg_t *)event->u.module_event.module_event_data;
        if (bus_msg->type == MCT_BUS_MSG_SET_AEC_STATE) {
          cam_ae_state_t ae_state = *(cam_ae_state_t *)bus_msg->msg;
        } else if (bus_msg->type == MCT_BUS_MSG_SET_AF_STATE) {
          cam_af_state_t af_state = *(cam_af_state_t *)bus_msg->msg;

          if ((af_state == CAM_AF_STATE_FOCUSED_LOCKED ||
            af_state == CAM_AF_STATE_NOT_FOCUSED_LOCKED ||
            af_state == CAM_AF_STATE_PASSIVE_FOCUSED ||
            af_state == CAM_AF_STATE_PASSIVE_UNFOCUSED) &&
            private->af_led_data.state == AF_WITH_LED_STATE_AF_RUNNING) {
            pthread_mutex_lock(&private->af_led_data.state_lock);
            CDBG("%s:%d AF with LED is DONE: %d", __func__, __LINE__, af_state);
            private->af_led_data.state = AF_WITH_LED_STATE_AF_DONE;
            if (private->af_led_data.led_status == 1) {
              q3a_port_request_aec_est_for_af(port, FALSE);
              q3a_port_request_led_off(port, FALSE);
              q3a_port_skip_bad_frame(port, private->cur_sof_id,
                NUMBER_OF_SKIP_FRAMES_PRE_FLASH_END);
              if(private->af_led_data.prepare_snapshot_trigger == TRUE){
                private->af_led_data.state = AF_WITH_LED_STATE_IDLE;
                private->af_led_data.prepare_snapshot_trigger = FALSE;
              }
            }
            q3a_port_start_timer(port);
            pthread_mutex_unlock(&private->af_led_data.state_lock);
          }
        } else if (bus_msg->type == MCT_BUS_MSG_SET_AEC_RESET &&
          private->aec_roi_enable == TRUE){
          private->aec_roi_enable = FALSE;
          q3a_port_request_aec_roi_off(port);
        }
      }
        break;
      default:
        break;
      }
    }
      break;
    default:
      break;
    }
  }
    break;
  /* Monitor downstream for particular events */
  case MCT_EVENT_DOWNSTREAM: {
    switch (event->type) {
    case MCT_EVENT_MODULE_EVENT: {
      mct_event_module_t *mod_evt = &(event->u.module_event);
      if (mod_evt->type == MCT_EVENT_MODULE_SOF_NOTIFY) {
        mct_bus_msg_isp_sof_t *sof_event;
        sof_event =(mct_bus_msg_isp_sof_t *)(mod_evt->module_event_data);
        private->cur_sof_id = sof_event->frame_id;
        if (private->af_led_data.state == AF_WITH_LED_STATE_AF_DONE) {
          /* update the timer */
          q3a_port_check_timer_and_update(port);
        }
      }
    }
      break;
    case MCT_EVENT_CONTROL_CMD: {
      mct_event_control_t *ctrl_evt =
        (mct_event_control_t *)&(event->u.ctrl_event);

      switch (ctrl_evt->type) {
      case MCT_EVENT_CONTROL_SOF: {
        if (private->af_led_data.state == AF_WITH_LED_STATE_AF_DONE) {
          /* update the timer */
          q3a_port_check_timer_and_update(port);
        }
      }
        break;
      case MCT_EVENT_CONTROL_SET_PARM: {
        stats_set_params_type *stat_parm =
          (stats_set_params_type *)ctrl_evt->control_event_data;
        if (stat_parm->param_type == STATS_SET_Q3A_PARAM) {
          q3a_set_params_type  *q3a_param = &(stat_parm->u.q3a_param);
          if (q3a_param->type == Q3A_SET_AF_PARAM) {
            af_set_parameter_t *parm =
              (af_set_parameter_t *)&q3a_param->u.af_param;

            switch (parm->type) {
            case AF_SET_PARAM_ROI:
              CDBG("%s:%d AF_SET_PARAM_ROI:", __func__, __LINE__);
              break;

            case AF_SET_PARAM_FOCUS_MODE:
              CDBG("%s:%d AF_SET_PARAM_FOCUS_MODE: %d", __func__, __LINE__,
                parm->u.af_mode);
              if (parm->u.af_mode == CAM_FOCUS_MODE_INFINITY ||
                parm->u.af_mode == CAM_FOCUS_MODE_MANUAL ||
                parm->u.af_mode == CAM_FOCUS_MODE_AUTO) {
                private->af_led_data.af_focus_mode_block = TRUE;
              }
              else {
                private->af_led_data.af_focus_mode_block = FALSE;
              }
              break;

            case AF_SET_PARAM_START: {  // HAL3
              CDBG("%s:%d AF_SET_PARAM_START:", __func__, __LINE__);

              if (private->stream_on == FALSE) {
                CDBG("%s:%d Stream is OFF, skipping for AF...", __func__,
                  __LINE__);
                rc = TRUE;
                break;
              }

              /* Ask AEC if LED estimation is needed */
              if (q3a_port_is_led_needed(port) == TRUE) {
                pthread_mutex_lock(&private->af_led_data.state_lock);
                /* If Flash is needed, start AF and tell it to wait for
                 * AC to finish estimation */
                q3a_port_update_led_af_to_sensor(port,TRUE);
                q3a_port_af_wait_for_aec_update(port, TRUE);
                /* Tell the AC to do estimation for AF
                 * (don't tell the sensor to turn OFF the LED) */
                q3a_port_request_aec_est_for_af(port, TRUE);

                /* Request for preview frame skip to HAL */
                q3a_port_skip_bad_frame(port, private->cur_sof_id,
                  NUMBER_OF_SKIP_FRAMES_PRE_FLASH_START);
                private->af_led_data.state = AF_WITH_LED_STATE_AEC_RUNNING;
                private->af_led_data.led_status = 1;
                pthread_mutex_unlock(&private->af_led_data.state_lock);

                CDBG("%s:%d LED_AF_NEEDED", __func__, __LINE__);
              } else {
                pthread_mutex_lock(&private->af_led_data.state_lock);
                private->af_led_data.state = AF_WITH_LED_STATE_IDLE;
                private->af_led_data.led_status = 0;
                pthread_mutex_unlock(&private->af_led_data.state_lock);
                CDBG("%s:%d LED_NOT_NEEDED", __func__, __LINE__);
              }
            }
              break;

            case AF_SET_PARAM_CANCEL_FOCUS: // HAL3
              pthread_mutex_lock(&private->af_led_data.state_lock);
              switch (private->af_led_data.state) {
              case AF_WITH_LED_STATE_IDLE:
                /* Do nothing */
                break;
              case AF_WITH_LED_STATE_AEC_RUNNING:
                /* if the cancel auto focus is in snapshot af, we need
                 * treat it as the same af type of touch af, tell the snesor
                 * port in case the led not turn off.
                 */
                q3a_port_af_wait_for_aec_update(port, FALSE);
                q3a_port_request_aec_est_for_af(port, FALSE);
                q3a_port_request_led_off(port, TRUE);
                break;
              case AF_WITH_LED_STATE_AF_RUNNING:
                q3a_port_request_aec_est_for_af(port, FALSE);
                /* if the cancel auto focus is in snapshot af, we need
                 * treat it as the same af type of touch af, tell the snesor
                 * port in case the led not turn off.
                 */
                q3a_port_request_led_off(port, TRUE);
                break;
              case AF_WITH_LED_STATE_AF_DONE:
                break;
              default:
                break;
              }
              private->af_led_data.state = AF_WITH_LED_STATE_IDLE;
              pthread_mutex_unlock(&private->af_led_data.state_lock);
              /* Just reset the counter.*/
              q3a_port_stop_timer(port);
              CDBG("%s:%d AF_SET_PARAM_CANCEL_FOCUS:", __func__, __LINE__);
              break;

            default:
              break;
            }
          } else if (q3a_param->type == Q3A_SET_AEC_PARAM) {
            aec_set_parameter_t *parm =
              (aec_set_parameter_t *)&q3a_param->u.aec_param;
            CDBG("%s:%d AEC_SET_PARAM: %d, FRAME: %d", __func__, __LINE__,
              parm->type, event->u.ctrl_event.current_frame_id);

            switch (parm->type) {
            case AEC_SET_PARAM_ROI:
              if(parm->u.aec_roi.enable)
                private->aec_roi_enable = TRUE;
                private->aec_settled = FALSE;
              break;

            case AEC_SET_PARAM_PREPARE_FOR_SNAPSHOT:
              CDBG("%s:%d AEC_SET_PARAM_PREPARE_FOR_SNAPSHOT: %d, FRAME: %d, "
                "LED state: %d", __func__, __LINE__,
                parm->type, event->u.ctrl_event.current_frame_id,
                private->af_led_data.state);
              pthread_mutex_lock(&private->af_led_data.state_lock);
              if (private->af_led_data.state != AF_WITH_LED_STATE_IDLE) {
                parm->type = AEC_SET_PARAM_PREP_FOR_SNAPSHOT_NOTIFY;
              }
              pthread_mutex_unlock(&private->af_led_data.state_lock);
              break;
            /* Reuse the defined value to detect HAL1 prep snapshot cmd */
            case AEC_SET_PARAM_PREP_FOR_SNAPSHOT_LEGACY:
              if (private->stream_on == FALSE) {
                CDBG("%s:%d Stream is OFF, skipping for AEC...",
                  __func__, __LINE__);
                rc = TRUE;
                break;
              }

              CDBG("%s:%d LEGACY AEC prepare for SNAPSHOT: %d, FRAME: %d, "
                "LED state: %d", __func__, __LINE__,
                parm->type, event->u.ctrl_event.current_frame_id,
                private->af_led_data.state);

              pthread_mutex_lock(&private->af_led_data.state_lock);
              if (private->af_led_data.state == AF_WITH_LED_STATE_AF_DONE) {
                CDBG("%s Estimation is ready, just update with skip", __func__);
                //notify sensor estimation is done
                q3a_port_request_led_off(port, FALSE);
                private->af_led_data.aec_update_data.aec_update.est_state =
                  AEC_EST_DONE_SKIP;
                /* Reset the state machine to IDLE */
                private->af_led_data.state = AF_WITH_LED_STATE_IDLE;
                /* Skip sending the event downstream to AEC port/algo */
                rc = TRUE;
              } else if (private->af_led_data.state == AF_WITH_LED_STATE_IDLE) {
                CDBG("%s AF+LED sequence in IDLE state.", __func__);
                /* Prevent hanging by sending a normal prep snapshot event.
                 * It will not start the AF. This should not happen - HAL/App is
                 * responsible for the right sequence, but we still need
                 * protection. */
                q3a_port_update_led_af_to_sensor(port,FALSE);
                if (q3a_port_is_led_needed(port) == TRUE &&
                      private->af_led_data.af_scene_mode_block == FALSE &&
                      private->af_led_data.af_focus_mode_block == FALSE &&
                      private->af_supported){
                  parm->type = AF_SET_PARAM_START;
                  stat_parm->param_type = STATS_SET_Q3A_PARAM;
                  stat_parm->u.q3a_param.type = Q3A_SET_AF_PARAM;

                  q3a_port_af_wait_for_aec_update(port, TRUE);
                  /* Tell the AC to do estimation for AF
                              * (don't tell the sensor to turn OFF the LED) */
                  q3a_port_request_aec_est_for_af(port, TRUE);
                  private->af_led_data.state = AF_WITH_LED_STATE_AEC_RUNNING;
                  private->af_led_data.led_status = 1;
                  private->af_led_data.prepare_snapshot_trigger = TRUE;

                  /* Request for preview frame skip to HAL */
                  q3a_port_skip_bad_frame(port, private->cur_sof_id,
                    NUMBER_OF_SKIP_FRAMES_PRE_FLASH_START);
                } else {
                  /* To handle case when flash is forced on in bright light condition.
                     Frame skip will be required when subject is too close */
                  if (private->af_led_data.flash_needed) {
                    /* Request for preview frame skip to HAL */
                    q3a_port_skip_bad_frame(port, private->cur_sof_id,
                      NUMBER_OF_SKIP_FRAMES_PRE_FLASH_START);
                  }
                  parm->type = AEC_SET_PARAM_PREPARE_FOR_SNAPSHOT;
                }
               pthread_mutex_unlock(&private->af_led_data.state_lock);
              } else {
                /* This is a precapture command sent during estimation for AF */
                CDBG("%s Precapture command sent during estimation for AF",
                  __func__);
                q3a_port_update_led_af_to_sensor(port,FALSE);
                /* Skip sending the event downstream to AEC port/algo */
                rc = TRUE;
              }
              pthread_mutex_unlock(&private->af_led_data.state_lock);

              break;
            default:
              break;
            }
          }
        } else if (stat_parm->param_type == STATS_SET_COMMON_PARAM) {
          stats_common_set_parameter_t *common_param =
          &(stat_parm->u.common_param);
          if(common_param->type == COMMON_SET_CAPTURE_INTENT &&
            common_param->u.capture_type == STATS_INTENT_STILL_CAPTURE) {
            CDBG("%s Still capture ongoing!", __func__);
            pthread_mutex_lock(&private->af_led_data.state_lock);
            if (private->af_led_data.state == AF_WITH_LED_STATE_AF_DONE) {
              private->af_led_data.state = AF_WITH_LED_STATE_IDLE;
            }
            pthread_mutex_unlock(&private->af_led_data.state_lock);
          }
		  else if(common_param->type == COMMON_SET_PARAM_BESTSHOT){
            if(common_param->u.bestshot_mode == CAM_SCENE_MODE_SUNSET ||
                 common_param->u.bestshot_mode == CAM_SCENE_MODE_LANDSCAPE)
              private->af_led_data.af_scene_mode_block = TRUE;
            else
              private->af_led_data.af_scene_mode_block = FALSE;
          }
        }
      }
        break;
      case MCT_EVENT_CONTROL_STREAMON: {
        mct_stream_info_t *stream_info = (mct_stream_info_t*)event->u.ctrl_event.control_event_data;
        cam_stream_type_t stream_type = stream_info->stream_type;
        if (stream_type == CAM_STREAM_TYPE_PREVIEW) {
          private->stream_on = TRUE;
        }
      }
        break;
      case MCT_EVENT_CONTROL_STREAMOFF: {
        mct_stream_info_t *stream_info = (mct_stream_info_t*)event->u.ctrl_event.control_event_data;
        cam_stream_type_t stream_type = stream_info->stream_type;
        if (stream_type == CAM_STREAM_TYPE_PREVIEW) {
          private->stream_on = FALSE;
        }
      }
        break;
      default:
        break;
      }
    }
      break;
    default:
      break;
    }
      break;
  }
    break;
  default: {
  }
    break;
  } /* switch (MCT_EVENT_DIRECTION(event)) */

  return rc;
}

/** q3a_port_event
 *    @port:  this port from where the event should go
 *    @event: event object to send upstream or downstream
 *
 *  Because stats interface module works only as an event pass through module,
 *  hence its downstream event handling should be fairly straightforward,
 *  but upstream event will need a little bit processing.
 *
 *  Return TRUE for successful event processing, FALSE on failure.
 **/
static boolean q3a_port_event(mct_port_t *port, mct_event_t *event)
{
  boolean            rc = TRUE;
  q3a_port_private_t *private;

  /* Sanity check */
  if (!port || !event) {
    return FALSE;
  }

  private = (q3a_port_private_t *)(port->port_private);
  if (!private) {
    return FALSE;
  }

  /* Sanity check: ensure event is meant for port with the same identity */
  if (!IS_Q3A_PORT_IDENTITY(private, event->identity)) {
    CDBG_ERROR("%s IDENTYTY does not match!", __func__);
    return FALSE;
  }

  switch (MCT_EVENT_DIRECTION(event)) {
  case MCT_EVENT_UPSTREAM: {
    /* Process Q3A events to handle Q3A statuses and functionalities */
    if ((rc = q3a_port_process_event_for_led_af(port, event)) == TRUE) {
      break;
    }
    /* The upstream events could come from AEC, AWB or AF module.
     *
     * Need to check event and see if it has to redirect the event
     * to downstream.
     */
    rc = q3a_port_proc_upstream_mod_event(port,event);
  } /* case MCT_EVENT_UPSTREAM */
    break;

  case MCT_EVENT_DOWNSTREAM: {
    if (event->type == MCT_EVENT_MODULE_EVENT &&
      event->u.module_event.type == MCT_EVENT_MODULE_SET_STREAM_CONFIG) {

      // Save the sensor af support info, used in led af
      sensor_out_info_t *sensor_info =
        (sensor_out_info_t *)(event->u.module_event.module_event_data);
      private->af_supported = sensor_info->af_lens_info.af_supported;
      CDBG("%s AF supported =%d", __func__,private->af_supported);
    }
    if (event->type == MCT_EVENT_MODULE_EVENT &&
      event->u.module_event.type == MCT_EVENT_MODULE_START_STOP_STATS_THREADS) {

      uint8_t *start_flag = (uint8_t*)(event->u.module_event.module_event_data);
      CDBG("%s MCT_EVENT_MODULE_START_STOP_STATS_THREADS start_flag: %d",
        __func__,*start_flag);

      if (*start_flag) {
        CDBG("%s: Starting AEC/AWB thread!", __func__);
        if (q3a_port_start_threads(port, event->identity,
          Q3A_THREAD_AECAWB) == FALSE) {
          rc = FALSE;
          CDBG_ERROR("%s: aec thread failed", __func__);
        }
        CDBG("%s: Starting AF thread!", __func__);
        if (q3a_port_start_threads(port, event->identity,
          Q3A_THREAD_AF) == FALSE) {
          CDBG_ERROR("%s: Starting AF thread failed!", __func__);
          rc = FALSE;
        }
      } else {
        CDBG("%s: aecawb_data=%p, af_data: %p", __func__,
          private->aecawb_data, private->af_data);
        if (private->aecawb_data) {
          /* Stop AECAWB thread */
          q3a_thread_aecawb_stop(private->aecawb_data);
        }
        if (private->af_data) {
          /* Stop AF thread */
          q3a_thread_af_stop(private->af_data);
        }
      }
    }
    if(MCT_EVENT_MODULE_EVENT == event->type
      && event->u.module_event.type == MCT_EVENT_MODULE_PREVIEW_STREAM_ID) {

        mct_stream_info_t  *stream_info =
          (mct_stream_info_t *)(event->u.module_event.module_event_data);
        private->preview_stream_id =
          (stream_info->identity & 0x0000FFFF);
        if ((rc = q3a_port_send_event_downstream(port, event)) == FALSE) {
          CDBG("Send downstream event failed.");
        }
        break;
    }
    if(MCT_EVENT_MODULE_EVENT == event->type
      && event->u.module_event.type == MCT_EVENT_MODULE_STREAM_CROP ) {
        mct_bus_msg_stream_crop_t *stream_crop = NULL;
        stream_crop =
          (mct_bus_msg_stream_crop_t *)event->u.module_event.module_event_data;
        if(stream_crop->stream_id != private->preview_stream_id)
          break;
    }
    if (private->state == Q3A_PORT_STATE_LINKED ||
      (MCT_EVENT_MODULE_EVENT == event->type
        && event->u.module_event.type == MCT_EVENT_MODULE_MODE_CHANGE)) {

      /* This event is received when the stream type changes.
       * Should pass on the event to the corresponding sub-ports
       * through downstream.
       */
      if(MCT_EVENT_MODULE_EVENT == event->type
        && event->u.module_event.type == MCT_EVENT_MODULE_MODE_CHANGE) {
        private->stream_type =
          ((stats_mode_change_event_data*)
          (event->u.module_event.module_event_data))->stream_type;
        private->reserved_id =
          ((stats_mode_change_event_data*)
          (event->u.module_event.module_event_data))->reserved_id;
      }

      /* Process Q3A events to handle Q3A statuses and functionalities */
      if ((rc = q3a_port_process_event_for_led_af(port, event)) == TRUE) {
        /* Downstream event processed, no need to process again. */
        break;
      }

      if ((rc = q3a_port_send_event_downstream(port, event)) == FALSE) {
        CDBG("Send downstream event failed.");
        break;
      }
    }
  } /* case MCT_EVENT_TYPE_DOWNSTREAM */
    break;

  default: {
  }
    break;
  } /* switch (MCT_EVENT_DIRECTION(event)) */

  CDBG("%s: X rc =%d", __func__, rc);
  return rc;
} /* q3a_port_event */

/** q3a_port_set_caps
 *    @port: port object which the caps to be set;
 *    @caps: this port's capabilities.
 *
 *  Function overwrites a ports capability.
 *
 *  Return TRUE if it is valid source port.
 **/
static boolean q3a_port_set_caps(mct_port_t *port, mct_port_caps_t *caps)
{
  if (strcmp(MCT_PORT_NAME(port), "q3a_sink")) {
    CDBG_ERROR("%s: Port name mismatch: %s != q3a_sink",
      __func__, MCT_PORT_NAME(port));
    return FALSE;
  }

  port->caps = *caps;
  return TRUE;
} /* q3a_port_set_caps */

/** q3a_port_check_caps_reserve
 *    @port:        this interface module's port;
 *    @peer_caps:   the capability of peer port which wants to match
 *                  interface port;
 *    @stream_info: the info for this stream
 *
 *  Stats modules are pure software modules, and every port can
 *  support one identity. If the identity is different, support
 *  can be provided via creating a new port. Regardless source or
 *  sink port, once capabilities are matched,
 *  - If this port has not been used, it can be supported;
 *  - If the requested stream is in existing identity, return failure
 *  - If the requested stream belongs to a different session, the port
 *    can not be used.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean q3a_port_check_caps_reserve(mct_port_t *port, void *caps,
  void *stream_nfo)
{
  boolean            rc = FALSE;
  mct_port_caps_t    *port_caps;
  q3a_port_private_t *private;
  mct_stream_info_t  *stream_info = stream_nfo;

  MCT_OBJECT_LOCK(port);
  if (!port || !caps || !stream_info ||
    strcmp(MCT_OBJECT_NAME(port), "q3a_sink")) {
    CDBG_ERROR("%s: Invalid parameters!", __func__);
    rc = FALSE;
    goto reserve_done;
  }

  port_caps = (mct_port_caps_t *)caps;
  if (port_caps->port_caps_type != MCT_PORT_CAPS_STATS) {
    rc = FALSE;
    goto reserve_done;
  }

  private = (q3a_port_private_t *)port->port_private;

  CDBG("%s: state %d", __func__, private->state);
  switch (private->state) {
    case Q3A_PORT_STATE_LINKED: {
    if (IS_Q3A_PORT_IDENTITY(private, stream_info->identity)) {
      rc = TRUE;
    }
  }
    break;

  case Q3A_PORT_STATE_CREATED:
  case Q3A_PORT_STATE_UNRESERVED: {

    if (private->aec_port->check_caps_reserve(private->aec_port,
      caps, stream_info) == FALSE) {
      rc = FALSE;
      break;
    }

    if (private->awb_port->check_caps_reserve(private->awb_port,
      caps, stream_info) == FALSE) {
      rc = FALSE;
      break;
    }

    if (private->af_port->check_caps_reserve(private->af_port,
      caps, stream_info) == FALSE) {
      rc = FALSE;
      break;
    }

    private->reserved_id = stream_info->identity;
    private->state       = Q3A_PORT_STATE_RESERVED;
    private->stream_type = stream_info->stream_type;
    rc = TRUE;
  }
    break;

  case Q3A_PORT_STATE_RESERVED: {
    if (IS_Q3A_PORT_IDENTITY(private, stream_info->identity)) {
      rc = TRUE;
    }
  }
    break;

  default: {
    rc = FALSE;
  }
    break;
  }

reserve_done:
  MCT_OBJECT_UNLOCK(port);
  return rc;
} /* q3a_port_check_caps_reserve */

/** module_stats_check_caps_unreserve
 *    @port:     this port object to remove the session/stream;
 *    @identity: session+stream identity.
 *
 *  This function frees the identity from port's children list.
 *
 *  Return FALSE if the identity is not existing, else return is TRUE
 **/
static boolean q3a_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  boolean            rc = FALSE;
  q3a_port_private_t *private;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "q3a_sink")) {
    return FALSE;
  }

  private = (q3a_port_private_t *)port->port_private;
  if (!private) {
    return FALSE;
  }

  if (private->state == Q3A_PORT_STATE_UNRESERVED) {
    return TRUE;
  }

  MCT_OBJECT_LOCK(port);
  if ((private->state == Q3A_PORT_STATE_UNLINKED ||
    private->state == Q3A_PORT_STATE_RESERVED) &&
    (IS_Q3A_PORT_IDENTITY(private, identity))) {

    if (private->aec_port->check_caps_unreserve(private->aec_port,
      identity) == FALSE) {
      rc = FALSE;
      goto unreserve_done;
    }

    if (private->awb_port->check_caps_unreserve(private->awb_port,
      identity) == FALSE) {
      rc = FALSE;
      goto unreserve_done;
    }

    if (private->af_port->check_caps_unreserve(private->af_port,
      identity) == FALSE) {
      rc =  FALSE;
      goto unreserve_done;
    }

    if (!MCT_OBJECT_REFCOUNT(port)) {
      private->state       = Q3A_PORT_STATE_UNRESERVED;
      private->reserved_id = (private->reserved_id & 0xFFFF0000);
    }
    rc = TRUE;
  }

unreserve_done:
  MCT_OBJECT_UNLOCK(port);
  return rc;
} /* q3a_port_check_caps_unreserve */

/** q3a_port_ext_link
 *    @identity:  Identity of session/stream
 *    @port:      SRC/SINK of stats ports
 *    @peer:      For stats sink - peer is most likely isp port
 *                For src module - peer is submodules sink.
 *
 *  Set stats port's external peer port.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean q3a_port_ext_link(unsigned int identity, mct_port_t *port,
  mct_port_t *peer)
{
  boolean            rc = FALSE;
  boolean            thread_init = FALSE;
  q3a_port_private_t *private;
  mct_event_t        event;

  CDBG("%s:%d", __func__, __LINE__);
  if (strcmp(MCT_OBJECT_NAME(port), "q3a_sink")) {
    CDBG_ERROR("%s: Q3A port name does not match!", __func__);
    return FALSE;
  }

  private = (q3a_port_private_t *)port->port_private;
  if (!private) {
    CDBG_ERROR("%s: Private port is NULL!", __func__);
    return FALSE;
  }

  MCT_OBJECT_LOCK(port);
  switch (private->state) {
  case Q3A_PORT_STATE_RESERVED:
  case Q3A_PORT_STATE_UNLINKED: {
    if (!IS_Q3A_PORT_IDENTITY(private, identity)) {
      break;
    }
  }
  /* Fall through, no break */
  case Q3A_PORT_STATE_CREATED: {
    CDBG("%s:%d", __func__, __LINE__);
    thread_init = TRUE;
    rc = TRUE;
  }
    break;
  case Q3A_PORT_STATE_LINKED: {
    CDBG("%s:%d", __func__, __LINE__);
    if (IS_Q3A_PORT_IDENTITY(private, identity)) {
      rc = TRUE;
      thread_init = FALSE;
    }
  }
    break;

  default: {
  }
    break;
  }

  if (rc == TRUE) {
    /* Invoke sub ports' ext link */
    CDBG("%s: Invoke sub-ports ext link", __func__);
    if (MCT_PORT_EXTLINKFUNC(private->aec_port)) {
      if (MCT_PORT_EXTLINKFUNC(private->aec_port)(identity,
        private->aec_port, port) == FALSE) {
        rc = FALSE;
        CDBG_ERROR("%s: AEC external link failed!", __func__);
        goto aec_link_fail;
      }
    }

    if (MCT_PORT_EXTLINKFUNC(private->awb_port)) {
      if (MCT_PORT_EXTLINKFUNC(private->awb_port)(identity,
        private->awb_port, port) == FALSE) {
        rc = FALSE;
        CDBG_ERROR("%s: AWB external link failed!", __func__);
        goto awb_link_fail;
      }
    }

    if (MCT_PORT_EXTLINKFUNC(private->af_port)) {
      if (MCT_PORT_EXTLINKFUNC(private->af_port)(identity,
        private->af_port, port) == FALSE){
        rc = FALSE;
        CDBG_ERROR("%s: AF external link failed!", __func__);
        goto af_link_fail;
      }
    }

    if (thread_init == TRUE) {
      private->aecawb_data = q3a_thread_aecawb_init();
      CDBG("%s aecawb data: %p", __func__, private->aecawb_data);
      if (NULL == private->aecawb_data) {
        CDBG_ERROR("%s: aecawb init failed", __func__);
        goto init_thread_fail;
      }
      private->af_data = q3a_thread_af_init();
      CDBG("%s af data: %p", __func__, private->af_data);
      if (NULL == private->af_data) {
        CDBG_ERROR("%s: af init failed", __func__);
        goto init_thread_fail;
      }
    }

    private->state = Q3A_PORT_STATE_LINKED;
    MCT_PORT_PEER(port) = peer;
    MCT_OBJECT_REFCOUNT(port) += 1;
  }

  MCT_OBJECT_UNLOCK(port);
  CDBG("%s:%d rc=%d", __func__, __LINE__, rc);

  return rc;

init_thread_fail:
  MCT_PORT_EXTUNLINKFUNC(private->af_port)(identity, private->af_port, port);
af_link_fail:
  MCT_PORT_EXTUNLINKFUNC(private->awb_port)(identity, private->awb_port, port);
awb_link_fail:
  MCT_PORT_EXTUNLINKFUNC(private->aec_port)(identity, private->aec_port, port);
aec_link_fail:
  MCT_OBJECT_UNLOCK(port);
  return rc;
} /* q3a_port_ext_link */

/** q3a_port_unlink
 *    @identity: Identity of session/stream
 *    @port:     q3a module's sink port
 *    @peer:     peer of stats sink port
 *
 * This function unlink the peer ports of stats sink, src ports
 * and its peer submodule's port
 *
 * Return void
 **/
static void q3a_port_unlink(unsigned int identity, mct_port_t *port,
  mct_port_t *peer)
{
  q3a_port_private_t *private;

  if (!port || !peer || MCT_PORT_PEER(port) != peer) {
    return;
  }

  private = (q3a_port_private_t *)port->port_private;
  if (!private) {
    return;
  }

  MCT_OBJECT_LOCK(port);
  if (private->state == Q3A_PORT_STATE_LINKED) {
    CDBG("%s: Invoke sub-ports ext un link", __func__);
    if (private->aec_port->un_link)
      private->aec_port->un_link(identity, private->aec_port, port);

    if (private->awb_port->un_link)
      private->awb_port->un_link(identity, private->awb_port, port);

    if (private->af_port->un_link)
      private->af_port->un_link(identity, private->af_port, port);

    MCT_OBJECT_REFCOUNT(port) -= 1;
    if (!MCT_OBJECT_REFCOUNT(port)) {
      CDBG("%s deinit aecawb: %p,  af_data: %p", __func__,
        private->aecawb_data, private->af_data);
      private->state = Q3A_PORT_STATE_UNLINKED;
      q3a_thread_aecawb_deinit(private->aecawb_data);
      q3a_thread_af_deinit(private->af_data);
      MCT_PORT_PEER(port) = NULL;
    }
  }
  MCT_OBJECT_UNLOCK(port);

  return;
} /* q3a_port_unlink */

/** q3a_port_find_identity
 *    @port:     module's port
 *    @identity: stream/session identity
 *
 *  This function will try to find the port's identity
 *
 *  Return TRUE if this is the identity of the Q3A port, FALSE if not.
 **/
boolean q3a_port_find_identity(mct_port_t *port, unsigned int identity)
{
  q3a_port_private_t *private;

  if ( !port || strcmp(MCT_OBJECT_NAME(port), "q3a_sink")) {
    return FALSE;
  }

  private = port->port_private;
  if (private) {
    return (IS_Q3A_PORT_IDENTITY(private, identity));
  }

  return FALSE;
} /* q3a_port_find_identity */

/** q3a_port_deinit
 *    @port: port object to be deinitialized
 *
 *  This function will free the private port data.
 *
 *  Return void
 **/
void q3a_port_deinit(mct_port_t *port)
{
  q3a_port_private_t *private;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "q3a_sink"))
    return;

  private = port->port_private;
  if (private) {
    pthread_mutex_destroy(&private->af_led_data.state_lock);
    pthread_mutex_destroy(&private->af_led_data.timer_lock);
    free(private);
    private = NULL;
  }
} /* q3a_port_deinit */

/** q3a_port_init
 *    @port: port object to be initialized
 *
 *  Port initialization, use this function to overwrite
 *  default port methods and install capabilities. Stats
 *  module should have ONLY sink port.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
boolean q3a_port_init(mct_port_t *port, mct_port_t *aec_port,
  mct_port_t *awb_port, mct_port_t *af_port, unsigned int identity)
{
  mct_port_caps_t    caps;
  q3a_port_private_t *private;
  mct_list_t         *list;

  private = malloc(sizeof(q3a_port_private_t));
  if (private == NULL) {
    return FALSE;
  }
  memset(private, 0, sizeof(q3a_port_private_t));

  private->reserved_id = identity;
  private->state       = Q3A_PORT_STATE_CREATED;
  private->aec_port    = aec_port;
  private->awb_port    = awb_port;
  private->af_port     = af_port;

  port->port_private  = private;
  port->direction     = MCT_PORT_SINK;
  caps.port_caps_type = MCT_PORT_CAPS_STATS;
  caps.u.stats.flag   = (MCT_PORT_CAP_STATS_Q3A | MCT_PORT_CAP_STATS_CS_RS |
    MCT_PORT_CAP_STATS_HIST);

  memset(&private->af_led_data, 0, sizeof(q3a_port_af_led_t));
  pthread_mutex_init(&private->af_led_data.state_lock, NULL);
  pthread_mutex_init(&private->af_led_data.timer_lock, NULL);

  mct_port_set_event_func(port, q3a_port_event);
  mct_port_set_set_caps_func(port, q3a_port_set_caps);
  mct_port_set_ext_link_func(port, q3a_port_ext_link);
  mct_port_set_unlink_func(port, q3a_port_unlink);
  mct_port_set_check_caps_reserve_func(port, q3a_port_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port, q3a_port_check_caps_unreserve);

  if (port->set_caps) {
    port->set_caps(port, &caps);
  }

  return TRUE;
} /* q3a_port_init */
