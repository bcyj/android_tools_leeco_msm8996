/* af_port.c
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include "af_module.h"
#include "af_port.h"
#include "cam_intf.h"
#include "cam_types.h"
#include "af.h"
#include "af_fdprio.h"
#include "q3a_thread.h"
#include "q3a_port.h"
#include "mct_stream.h"
#include "mct_module.h"
#include "stats_event.h"

#define USE_HAL_3    0

#ifdef DBG_AF_PORT
#undef CDBG
#define CDBG CDBG_ERROR
#endif

/** _af_pre_init_updates:
 *    @is_stream_info_updated: TRUE if stream info update
 *     received before AF init.
 *    @stream_info: saved isp dim output.
 *
 *  Any updates received before AF is initiliazed will be saved
 *  locally and then updated to AF algorithm once we get tuning
 *  pointer update.
 */
typedef struct _af_pre_init_updates {
  boolean is_stream_info_updated;
  mct_stream_info_t stream_info;

} af_pre_init_updates_t;


/** af_port_private:
 *  private data structure of AF port
 *  @reserved_id: session index
 *  @stream_info: stream parameters
 *  @state: AF port state
 *  @af_object: parameters specific to AF object
 *  @thread_data: AF thread information
 *  @af_initialized: flag to indicate if AF has been initialized
 *                 and it's ok to forward events to AF algorithm
 *
 **/
typedef struct _af_port_private {
  unsigned int      reserved_id;
  mct_stream_info_t stream_info;
  af_port_state_t   state;
  af_object_t       af_object;
  q3a_thread_data_t *thread_data;
  boolean           af_initialized;
  af_fdprio_t       fd_prio_data;
  cam_focus_mode_type prev_af_mode;
  cam_focus_mode_type af_mode;
  af_status_type    af_status;
  boolean           af_trigger_called;
  af_port_state_trans_t af_trans;
  uint32_t sof_id;
  cam_focus_distances_info_t af_focus_distance; // save the focus distance info and send to HAL
  int32_t           af_focus_pos;
  boolean           af_led_assist; //if LED assisted AF is running
  af_run_mode_type  run_type;
  af_isp_up_event_t isp_status;
  af_stream_crop_t  stream_crop;
  af_input_from_isp_t isp_info;
  af_preview_size_t preview_size;
  boolean force_af_update;
  af_sw_stats_t     *p_sw_stats;
  af_input_from_aec_t latest_aec_info;
  char                af_debug_data_array[AF_DEBUG_DATA_SIZE];
  uint32_t            af_debug_data_size;
  af_pre_init_updates_t pre_init_updates;
  boolean               reconfigure_ISP_pending;
} af_port_private_t;

/* Static Function Definitions */
static boolean af_port_handle_isp_output_dim_event(af_port_private_t *af_port,
  mct_stream_info_t *stream_info);

/** af_port_map_input_roi_to_camif: Map input ROI to camif size.
 *
 *  @af_internal: internal af data structure
 *
 **/
static boolean af_port_map_input_roi_to_camif(
  af_port_private_t *af_port,
  af_roi_t *roi) {
  int roi_x, roi_y, roi_x2, roi_y2;
  double h_scale_ratio, v_scale_ratio;

  af_stream_crop_t stream_crop_zero;
  memset(&stream_crop_zero, 0, sizeof(af_stream_crop_t));
  if (!memcmp(&af_port->stream_crop, &stream_crop_zero,
      sizeof(af_stream_crop_t))) {
    CDBG("%s: Crop data is not valid, ", __func__);
    return FALSE;
  }

  CDBG("%s: Input AF ROI: (%d, %d, %d, %d) Preview-size: %d x %d", __func__,
    roi->x, roi->y, roi->dx, roi->dy,
    af_port->preview_size.width, af_port->preview_size.height);
  if (!roi->dx || !roi->dy) {
    memset(roi, 0, sizeof(af_roi_t));
    return TRUE;
  }

  CDBG("%s: VFE-MAP: (%d, %d, %d %d) PP-CROP: (%d, %d, %d, %d) ISP-DIM: %dx%d",
    __func__, af_port->stream_crop.vfe_map_x, af_port->stream_crop.vfe_map_y,
    af_port->stream_crop.vfe_map_width, af_port->stream_crop.vfe_map_height,
    af_port->stream_crop.pp_x, af_port->stream_crop.pp_y,
    af_port->stream_crop.pp_crop_out_x, af_port->stream_crop.pp_y,
    af_port->isp_info.width, af_port->isp_info.height);

  if (!af_port->stream_crop.pp_crop_out_x ||
    !af_port->stream_crop.pp_crop_out_y) {
    h_scale_ratio = 1;
    v_scale_ratio = 1;
    af_port->stream_crop.pp_x = 0;
    af_port->stream_crop.pp_y = 0;
    af_port->stream_crop.pp_crop_out_x = af_port->preview_size.width;
    af_port->stream_crop.pp_crop_out_y = af_port->preview_size.height;
  } else {
    h_scale_ratio = (double)(af_port->preview_size.width) /
      af_port->stream_crop.pp_crop_out_x;
    v_scale_ratio = (double)(af_port->preview_size.height) /
      af_port->stream_crop.pp_crop_out_y;
  }

  // Reverse calculation for cpp output to vfe output
  roi_x = roi->x / h_scale_ratio;
  roi_y = roi->y / v_scale_ratio;
  roi_x += af_port->stream_crop.pp_x;
  roi_y += af_port->stream_crop.pp_y;

  roi_x2 = (roi->dx + roi->x) / h_scale_ratio;
  roi_y2 = (roi->dy + roi->y) / v_scale_ratio;
  roi_x2 += af_port->stream_crop.pp_x;
  roi_y2 += af_port->stream_crop.pp_y;

  // Reverse calculation for vfe output to camif output
  if (!af_port->isp_info.width || !af_port->isp_info.height) {
    CDBG("%s: Invalid ISP output", __func__);
    return FALSE;
  }

  roi_x = ((roi_x * af_port->stream_crop.vfe_map_width) /
    af_port->isp_info.width) + af_port->stream_crop.vfe_map_x;
  roi_y = ((roi_y * af_port->stream_crop.vfe_map_height) /
    af_port->isp_info.height) + af_port->stream_crop.vfe_map_y;

  roi_x2 = ((roi_x2 * af_port->stream_crop.vfe_map_width) /
    af_port->isp_info.width) + af_port->stream_crop.vfe_map_x;
  roi_y2 = ((roi_y2 * af_port->stream_crop.vfe_map_height) /
    af_port->isp_info.height) + af_port->stream_crop.vfe_map_y;

  roi->x = roi_x;
  roi->y = roi_y;
  roi->dx = roi_x2 - roi_x;
  roi->dy = roi_y2 - roi_y;
  CDBG("%s:Mapped ROI: (%d, %d, %d %d)", __func__,
    roi->x, roi->y, roi->dx, roi->dy);

  return TRUE;
} /* af_recalc_stats_config_roi_info: */

/** af_port_handle_lock_caf_event:
 *    @af_port:  private AF port data
 *    @set_parm: a message to populate
 *    @lock:     lock or unlock
 *
 * Handle continuous autofocus lock call from application.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_lock_caf_event(af_port_private_t * af_port,
  af_set_parameter_t * set_parm, boolean lock)
{
  boolean rc = TRUE;

  /* Update parameter message to be sent */
  set_parm->type = AF_SET_PARAM_LOCK_CAF;
  set_parm->u.af_lock_caf = lock;

  CDBG("%s: Lock/unlock (%d) CAF!", __func__, lock);
  return rc;
} /* af_port_handle_lock_caf_event */

/** af_port_lock_caf
 * Lock unlock CAF
 *
 * @af_port: private port info
 *
 * @lock: lock caf if TRUE, unlock if false
 *
 * Return:
 **/
static void af_port_lock_caf(af_port_private_t *af_port,
  boolean lock) {
  boolean rc = TRUE;
  af_set_parameter_t *set_parm;

  if (!af_port) {
    CDBG_ERROR("%s: Invalid parameters!", __func__);
    return;
  }
  CDBG("%s: Lock (%d) CAF!", __func__, lock);

  /* Allocate memory to create AF message */
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));
  if (af_msg == NULL) {
    CDBG_ERROR("%s: Memory allocation failure!", __func__);
    return;
  }
  /* populate af message to post to thread */
  memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
  af_msg->type = MSG_AF_SET;

  set_parm = &af_msg->u.af_set_parm;

  rc = af_port_handle_lock_caf_event(af_port, set_parm, lock);

  /* Enqueue the message to the AF thread */
  q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
} /* af_port_lock_caf */

/** af_port_handle_auto_mode_state_update
 * Handle AF state changes and action for focus mode Auto.
 *
 * @af_port: private port number
 *
 * @trans_reason: reason of transition
 *
 * @state (out): new state transitioned to
 * Return: True - Success  False - failure
 **/
static boolean af_port_handle_auto_mode_state_update(
  af_port_private_t *af_port,
  af_port_state_transition_type cause,
  cam_af_state_t *state)
{
  boolean rc = TRUE;
  cam_af_state_t new_state = af_port->af_trans.af_state;
  CDBG("%s: Handle Auto Mode AF state update!", __func__);
  CDBG("%s: Input-state: %d Transition-Cause: %d", __func__,
    af_port->af_trans.af_state, cause);

  switch (af_port->af_trans.af_state) {
  case CAM_AF_STATE_INACTIVE:
    if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Start AF sweep. Lens now moving */
      new_state = CAM_AF_STATE_ACTIVE_SCAN;
    }
    break;
  case CAM_AF_STATE_ACTIVE_SCAN:
    if (cause == AF_PORT_TRANS_CAUSE_AF_DONE_FOCUSED) {
      /* If AF successful lens now locked */
      new_state = CAM_AF_STATE_FOCUSED_LOCKED;
    } else if (cause == AF_PORT_TRANS_CAUSE_AF_DONE_UNFOCUSED) {
      /* AF unfocused. Lens now locked */
      new_state = CAM_AF_STATE_NOT_FOCUSED_LOCKED;
    } else if (cause == AF_PORT_TRANS_CAUSE_CANCEL) {
      /* Cancel/reset AF. Lens now locked */
      new_state = CAM_AF_STATE_INACTIVE;
    }
    break;
  case CAM_AF_STATE_FOCUSED_LOCKED:
    if (cause == AF_PORT_TRANS_CAUSE_CANCEL) {
      /* Cancel/reset AF */
      new_state = CAM_AF_STATE_INACTIVE;
    } else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Start new sweep. Lens now moving */
      new_state = CAM_AF_STATE_ACTIVE_SCAN;
    }
    break;
  case CAM_AF_STATE_NOT_FOCUSED_LOCKED:
    if (cause == AF_PORT_TRANS_CAUSE_CANCEL) {
      /* Cancel/reset AF */
      new_state = CAM_AF_STATE_INACTIVE;
    } else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Start new sweep. Lens now moving */
      new_state = CAM_AF_STATE_ACTIVE_SCAN;
    }
    break;
  default:
    CDBG_ERROR("%s:%d: Error: Undefined AF state!", __func__, __LINE__);
    break;
  }

  /* New state */
  CDBG("%s: Old state: %d New AF state: %d", __func__,
    af_port->af_trans.af_state, new_state);
  *state = new_state;
  return rc;
} /* af_port_handle_auto_mode_state_update */

/** af_port_handle_caf_cam_mode_state_update
 * Handle AF state changes and action for focus mode Continuous
 * Picture.
 *
 * @af_port: private port number
 *
 * @trans_reason: reason of transition
 *
 * @state (out): new state transitioned to
 * Return: True - Success  False - failure
 **/
static boolean af_port_handle_caf_cam_mode_state_update(
  af_port_private_t *af_port,
  af_port_state_transition_type cause,
  cam_af_state_t *state)
{
  boolean rc = TRUE;
  cam_af_state_t new_state = af_port->af_trans.af_state;
  CDBG("%s: Handle Continuous Picture Mode AF state update!", __func__);
  CDBG("%s: Input-state: %d Transition-Cause: %d", __func__,
    af_port->af_trans.af_state, cause);
  switch (af_port->af_trans.af_state) {
  case CAM_AF_STATE_INACTIVE:
    if (cause == AF_PORT_TRANS_CAUSE_SCANNING) {
      /* Start AF scan. Lens now moving */
      new_state = CAM_AF_STATE_PASSIVE_SCAN;
    } else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Query AF state. Lens now locked. */
      new_state = CAM_AF_STATE_NOT_FOCUSED_LOCKED;
      af_port_lock_caf(af_port, TRUE);
      af_port->force_af_update = TRUE;
    }
    break;
  case CAM_AF_STATE_PASSIVE_SCAN:
    if ((cause == AF_PORT_TRANS_CAUSE_AF_DONE_FOCUSED) ||
      (cause == AF_PORT_TRANS_CAUSE_AF_DONE_UNFOCUSED)) {
      /* AF scanning done. Lens now locked */
      if (af_port->af_trigger_called) {
        CDBG("%s: AF trigger had been called before!", __func__);
        new_state = (af_port->af_status == AF_STATUS_FOCUSED) ?
          CAM_AF_STATE_FOCUSED_LOCKED : CAM_AF_STATE_NOT_FOCUSED_LOCKED;
        af_port->af_trigger_called = FALSE;
        af_port_lock_caf(af_port, TRUE);
      } else {
        new_state = (cause == AF_PORT_TRANS_CAUSE_AF_DONE_FOCUSED)
          ? CAM_AF_STATE_PASSIVE_FOCUSED : CAM_AF_STATE_PASSIVE_UNFOCUSED;
      }
    } else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Eventual transition if focus good/not good. Lock Lens */
      /* We are still scanning. We'll wait till scanning complete comes back. */
      CDBG("%s: We are still scanning. Wait till AF is done.", __func__);
      af_port->af_trigger_called = TRUE;
    } else if (cause == AF_PORT_TRANS_CAUSE_CANCEL) {
      /* Reset lens position. Lens now locked. */
      new_state = CAM_AF_STATE_INACTIVE;
    }
    break;
  case CAM_AF_STATE_PASSIVE_FOCUSED:
    if (cause == AF_PORT_TRANS_CAUSE_SCANNING)
      /* Start AF scan. Lens now moving */
      new_state = CAM_AF_STATE_PASSIVE_SCAN;
    else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Immediate transition if focus good/not good. Lock Lens */
      new_state = CAM_AF_STATE_FOCUSED_LOCKED;
      af_port_lock_caf(af_port, TRUE);
      af_port->force_af_update = TRUE;
    } else if (cause == AF_PORT_TRANS_CAUSE_AF_DONE_UNFOCUSED) {
      new_state = CAM_AF_STATE_PASSIVE_UNFOCUSED;
    }
    break;
  case CAM_AF_STATE_PASSIVE_UNFOCUSED:
    if (cause == AF_PORT_TRANS_CAUSE_SCANNING)
      /* Start AF scan. Lens now moving */
      new_state = CAM_AF_STATE_PASSIVE_SCAN;
    else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Immediate transition if focus not good. Lock Lens */
      new_state = CAM_AF_STATE_NOT_FOCUSED_LOCKED;
      af_port_lock_caf(af_port, TRUE);
      af_port->force_af_update = TRUE;
    }
    break;

  case CAM_AF_STATE_FOCUSED_LOCKED:
    if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Do nothing */
      new_state = CAM_AF_STATE_FOCUSED_LOCKED;
      af_port->force_af_update = TRUE;
    } else if (cause == AF_PORT_TRANS_CAUSE_CANCEL) {
      /* Restart AF scan */
      new_state = CAM_AF_STATE_PASSIVE_FOCUSED;
      af_port_lock_caf(af_port, FALSE);
      af_port->force_af_update = TRUE;
    }
    break;
  case CAM_AF_STATE_NOT_FOCUSED_LOCKED:
    if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Do nothing */
      new_state = CAM_AF_STATE_NOT_FOCUSED_LOCKED;
      af_port->force_af_update = TRUE;
    } else if (cause == AF_PORT_TRANS_CAUSE_CANCEL) {
      /* Restart AF scan */
      new_state = CAM_AF_STATE_PASSIVE_UNFOCUSED;
      af_port_lock_caf(af_port, FALSE);
      af_port->force_af_update = TRUE;
    }
    break;
  default:
    CDBG_ERROR("%s:%d: Error: Undefined AF state!", __func__, __LINE__);
    break;
  }

  /* New state */
  CDBG("%s: New AF state: %d", __func__, new_state);
  *state = new_state;
  return rc;
} /* af_port_handle_caf_cam_mode_state_update */

/** af_port_handle_caf_video_mode_state_update
 * Handle AF state changes and action for focus mode Continuous
 * Video.
 *
 * @af_port: private port number
 *
 * @trans_reason: reason of transition
 *
 * @state (out): new state transitioned to
 * Return: True - Success  False - failure
 **/
static boolean af_port_handle_caf_video_mode_state_update(
  af_port_private_t *af_port,
  af_port_state_transition_type cause,
  cam_af_state_t *state)
{
  boolean rc = TRUE;
  cam_af_state_t new_state = af_port->af_trans.af_state;
  CDBG("%s: Handle Continuous Picture Mode AF state update!", __func__);

  switch (af_port->af_trans.af_state) {
  case CAM_AF_STATE_INACTIVE:
    if (cause == AF_PORT_TRANS_CAUSE_SCANNING) {
      /* Start AF scan. Lens now moving */
      new_state = CAM_AF_STATE_PASSIVE_SCAN;
    } else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Query AF state. Lens now locked. */
      new_state = CAM_AF_STATE_NOT_FOCUSED_LOCKED;
      af_port_lock_caf(af_port, TRUE);
      af_port->force_af_update = TRUE;
    }
    break;
  case CAM_AF_STATE_PASSIVE_SCAN:
    if ((cause == AF_PORT_TRANS_CAUSE_AF_DONE_FOCUSED) ||
      (cause == AF_PORT_TRANS_CAUSE_AF_DONE_UNFOCUSED)) {
      /* AF scanning done. Lens now locked */
      new_state = (cause == AF_PORT_TRANS_CAUSE_AF_DONE_FOCUSED)
        ? CAM_AF_STATE_PASSIVE_FOCUSED : CAM_AF_STATE_PASSIVE_UNFOCUSED;
    } else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER){
      /* Immediate transition if focus good/not good. Lock Lens */
      new_state = (af_port->af_status == AF_STATUS_FOCUSED) ?
        CAM_AF_STATE_FOCUSED_LOCKED : CAM_AF_STATE_NOT_FOCUSED_LOCKED;
      af_port_lock_caf(af_port, TRUE);
    } else if (cause == AF_PORT_TRANS_CAUSE_CANCEL) {
      /* Reset lens position. Lens now locked. */
      new_state = CAM_AF_STATE_INACTIVE;
    }
    break;
  case CAM_AF_STATE_PASSIVE_FOCUSED:
    if (cause == AF_PORT_TRANS_CAUSE_SCANNING) {
      /* Start AF scan. Lens now moving */
      new_state = CAM_AF_STATE_PASSIVE_SCAN;
    } else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Immediate transition if focus good/not good. Lock Lens */
      new_state = CAM_AF_STATE_FOCUSED_LOCKED;
      af_port_lock_caf(af_port, TRUE);
    } else if (cause == AF_PORT_TRANS_CAUSE_AF_DONE_UNFOCUSED) {
      new_state = CAM_AF_STATE_PASSIVE_UNFOCUSED;
    }
    break;
  case CAM_AF_STATE_PASSIVE_UNFOCUSED:
    if (cause == AF_PORT_TRANS_CAUSE_SCANNING) {
      /* Start AF scan. Lens now moving */
      new_state = CAM_AF_STATE_PASSIVE_SCAN;
    } else if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Immediate transition if focus good/not good. Lock Lens */
      new_state = CAM_AF_STATE_NOT_FOCUSED_LOCKED;
      af_port_lock_caf(af_port, TRUE);
    }
    break;
  case CAM_AF_STATE_FOCUSED_LOCKED:
    if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Do nothing */
      new_state = CAM_AF_STATE_FOCUSED_LOCKED;
    } else if (cause == AF_PORT_TRANS_CAUSE_CANCEL) {
      /* Restart AF scan */
      new_state = CAM_AF_STATE_PASSIVE_FOCUSED;
      af_port_lock_caf(af_port, FALSE);
    }
    break;
  case CAM_AF_STATE_NOT_FOCUSED_LOCKED:
    if (cause == AF_PORT_TRANS_CAUSE_TRIGGER) {
      /* Do nothing */
      new_state = CAM_AF_STATE_NOT_FOCUSED_LOCKED;
    } else if (cause == AF_PORT_TRANS_CAUSE_CANCEL) {
      /* Restart AF scan */
      new_state = CAM_AF_STATE_PASSIVE_UNFOCUSED;
      af_port_lock_caf(af_port, FALSE);
    }
    break;
  default:
    CDBG_ERROR("%s:%d: Error: Undefined AF state!", __func__, __LINE__);
    break;
  }

  /* New state */
  CDBG("%s: New AF state: %d", __func__, new_state);
  *state = new_state;
  return rc;
} /* af_port_handle_caf_video_mode_state_update */

/** af_port_update_af_state
 * Update AF state to HAL only if it changes
 *
 * @port: port info
 *
 * @trans_reason: reason of transition
 *
 * Return: True - Success  False - failure
 **/
static boolean af_port_update_af_state(mct_port_t  *port,
  af_port_state_transition_type cause) {
  boolean rc = TRUE;
  mct_event_t event;
  mct_bus_msg_t bus_msg;
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  cam_af_state_t new_state = af_port->af_trans.af_state;

  CDBG("%s: AF Current State: %d Trans. Cause: %d",
    __func__, af_port->af_trans.af_state, cause);
  af_port->af_trans.cause = cause;

  /* If transition cause is AF mode change, we'll set the
     state to INACTIVE irrespective of other conditions */
  if (cause == AF_PORT_TRANS_CAUSE_MODE_CHANGE) {
    CDBG("%s: AF mode changed. Set AF state to Inactive!", __func__);
    if (af_port->prev_af_mode == CAM_FOCUS_MODE_AUTO &&
      (af_port->af_mode == CAM_FOCUS_MODE_CONTINOUS_PICTURE ||
      af_port->af_mode == CAM_FOCUS_MODE_CONTINOUS_VIDEO))
      new_state = CAM_AF_STATE_PASSIVE_FOCUSED;
    else
    new_state = CAM_AF_STATE_INACTIVE;
  } else {
    /* If current mode is CONTINOUS_PICTURE */
    if (af_port->af_mode == CAM_FOCUS_MODE_CONTINOUS_PICTURE) {
      CDBG("%s: Current AF mode is: %d", __func__, af_port->af_mode);
      af_port_handle_caf_cam_mode_state_update(af_port, cause, &new_state);
    } else if (af_port->af_mode == CAM_FOCUS_MODE_CONTINOUS_VIDEO) {
      CDBG("%s: Current AF mode is: %d", __func__, af_port->af_mode);
      af_port_handle_caf_video_mode_state_update(af_port, cause, &new_state);
    } else if ((af_port->af_mode == CAM_FOCUS_MODE_AUTO) ||
      (af_port->af_mode == CAM_FOCUS_MODE_MACRO)) {
      CDBG("%s: Current AF mode is: %d", __func__, af_port->af_mode);
      af_port_handle_auto_mode_state_update(af_port, cause, &new_state);
    }
    /* For all other modes, we keep AF State INACTIVE */
    else {
      new_state = CAM_AF_STATE_INACTIVE;
    }
  }
  af_port->af_trans.af_state = new_state;

  return rc;
}

/** af_send_bus_msg:
 * Post bus message
 *
 * @port: port info
 * @bus_msg_type: message type
 * @payload: message payload
 *
 * @status: focus status
 *
 * Return: TRUE: Success  FALSE: Failure
 **/
static boolean af_send_bus_msg(mct_port_t *port,
  mct_bus_msg_type_t bus_msg_type, void *payload,
  int size, int sof_id)
{
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  mct_event_t event;
  mct_bus_msg_t bus_msg;
  boolean rc = TRUE;

  bus_msg.sessionid = (af_port->reserved_id >> 16);
  bus_msg.type = bus_msg_type;
  bus_msg.msg = payload;
  bus_msg.size = size;

  /* pack into an mct_event object*/
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = af_port->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)(&bus_msg);
  event.u.module_event.current_frame_id = sof_id;

  rc = MCT_PORT_EVENT_FUNC(port)(port, &event);

  return rc;
}

/** af_port_process_status_update:
 *    @port:   port info
 *    @status: focus status
 *
 * Send AF status update to upper layer. Also send focus distance update.
 *
 * Return: TRUE: Success  FALSE: Failure
 **/
static boolean af_port_process_status_update(
  mct_port_t  *port,
  af_output_data_t *af_out) {
  boolean rc = TRUE;

  af_port_state_transition_type cause;
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  af_status_t *af_status = &af_out->focus_status;
  CDBG("%s: Send AF Status Update!", __func__);
  af_port->af_status = af_status->status;

  /* update AF status */
  switch (af_status->status) {
  case AF_STATUS_FOCUSED:
    cause = AF_PORT_TRANS_CAUSE_AF_DONE_FOCUSED;
    break;
  case AF_STATUS_UNKNOWN:
    cause = AF_PORT_TRANS_CAUSE_AF_DONE_UNFOCUSED;
    break;
  case AF_STATUS_FOCUSING:
    cause = AF_PORT_TRANS_CAUSE_SCANNING;
    break;
  default:
    cause = AF_PORT_TRANS_CAUSE_AF_DONE_UNFOCUSED;
    break;
  }

  /* transition to new AF state */
  CDBG("%s: Current trigger_id: %d", __func__, af_port->af_trans.trigger_id);
  CDBG("%s: Transition to new AF state. cause: %d", __func__, cause);
  af_port_update_af_state(port, cause);

  /* save the focus distance and send it out later
   * it needs to be sent out on each SOF
   */
  af_port->af_focus_distance.focus_distance[CAM_FOCUS_DISTANCE_NEAR_INDEX] =
    af_status->f_distance.focus_distance[FOCUS_DISTANCE_NEAR_INDEX];
  af_port->af_focus_distance.focus_distance[CAM_FOCUS_DISTANCE_OPTIMAL_INDEX] =
    af_status->f_distance.focus_distance[FOCUS_DISTANCE_OPTIMAL_INDEX];
  af_port->af_focus_distance.focus_distance[CAM_FOCUS_DISTANCE_FAR_INDEX] =
    af_status->f_distance.focus_distance[FOCUS_DISTANCE_FAR_INDEX];
  af_port->af_focus_pos = af_status->focus_pos;
  return rc;
} /* af_port_process_status_update */



/** af_port_send_legacy_status_update:
 *    @port:   port info
 *
 * Send AF status update to upper layer. Also send focus distance update.
 *
 * Return: TRUE: Success  FALSE: Failure
 **/
static boolean af_port_send_legacy_status_update(mct_port_t  *port) {
  boolean                 rc = TRUE;
  mct_event_t             event;
  mct_bus_msg_t           bus_msg;
  mct_bus_msg_af_status_t af_msg;
  af_port_private_t       *af_port = (af_port_private_t *)(port->port_private);

  memset(&af_msg, 0, sizeof(mct_bus_msg_af_status_t));

  CDBG_ERROR("%s: Send AF Status Update! af_state=%d", __func__,af_port->af_trans.af_state);

  /* update AF status */
  switch (af_port->af_trans.af_state) {
  case CAM_AF_STATE_PASSIVE_FOCUSED: {
    af_msg.focus_state = CAM_AF_PASSIVE_FOCUSED;
  }
    break;

  case CAM_AF_STATE_FOCUSED_LOCKED: {
    af_msg.focus_state = CAM_AF_FOCUSED;
  }
    break;

  case CAM_AF_STATE_NOT_FOCUSED_LOCKED: {
    af_msg.focus_state = CAM_AF_NOT_FOCUSED;
  }
    break;

  case CAM_AF_STATE_PASSIVE_UNFOCUSED: {
    af_msg.focus_state = CAM_AF_PASSIVE_UNFOCUSED;
  }
    break;

  case CAM_AF_STATE_PASSIVE_SCAN: {
    af_msg.focus_state = CAM_AF_PASSIVE_SCANNING;
  }
    break;

  case CAM_AF_STATE_ACTIVE_SCAN: {
    af_msg.focus_state = CAM_AF_SCANNING;
  }
    break;

  case CAM_AF_STATE_INACTIVE: {
    af_msg.focus_state = CAM_AF_INACTIVE;
  }
    break;

  default: {
    af_msg.focus_state = CAM_AF_NOT_FOCUSED;
  }
    break;
  }

  /* update focus distance */
  af_msg.f_distance.focus_distance[CAM_FOCUS_DISTANCE_NEAR_INDEX] =
    af_port->af_focus_distance.focus_distance[FOCUS_DISTANCE_NEAR_INDEX];
  af_msg.f_distance.focus_distance[CAM_FOCUS_DISTANCE_OPTIMAL_INDEX] =
    af_port->af_focus_distance.focus_distance[FOCUS_DISTANCE_OPTIMAL_INDEX];
  af_msg.f_distance.focus_distance[CAM_FOCUS_DISTANCE_FAR_INDEX] =
    af_port->af_focus_distance.focus_distance[FOCUS_DISTANCE_FAR_INDEX];

  af_msg.focus_pos = af_port->af_focus_pos;

  rc = af_send_bus_msg(port, MCT_BUS_MSG_Q3A_AF_STATUS, &af_msg,
    sizeof(mct_bus_msg_af_status_t), af_port->sof_id);

  if (rc == TRUE)
    CDBG_ERROR("%s: Send AF Status update to af port: %d", __func__,
      af_msg.focus_state);
  else
    CDBG_ERROR("%s: Send AF Status update to af port: %d failed!", __func__,
      af_msg.focus_state);

  return rc;
} /* af_port_send_legacy_status_update */

#if 0
static boolean af_port_send_af_lens_state(
  mct_port_t  *port) {

  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  boolean rc = TRUE;
  cam_af_lens_state_t lens_state = CAM_AF_LENS_STATE_STATIONARY;

  switch (af_port->af_trans.af_state) {
  case CAM_AF_STATE_INACTIVE:
  case CAM_AF_STATE_PASSIVE_FOCUSED:
  case CAM_AF_STATE_FOCUSED_LOCKED:
  case CAM_AF_STATE_NOT_FOCUSED_LOCKED:
    lens_state = CAM_AF_LENS_STATE_STATIONARY;
    break;
  case CAM_AF_STATE_PASSIVE_SCAN:
  case CAM_AF_STATE_ACTIVE_SCAN:
    lens_state = CAM_AF_LENS_STATE_MOVING;
    break;
  default:
    break;
  }

  rc = af_send_bus_msg(port, MCT_BUS_MSG_SET_AF_LENS_STATE, &lens_state,
    sizeof(cam_af_lens_state_t), AF_SEND_IMMEDIATE);

  return rc;
}
#endif
/** af_port_send_roi_update:
 * Send AF roi to upper layer.
 *
 * @port: port info
 *
 * @status: focus status
 *
 * Return: TRUE: Success  FALSE: Failure
 **/
static boolean af_port_send_roi_update(
  mct_port_t  *port,
  af_output_data_t *af_out) {
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  boolean rc = TRUE;
  cam_area_t roi_msg;

  CDBG("%s: Send AF ROI Update!", __func__);
  af_roi_info_t *af_roi_info = &af_out->roi_info;
  roi_msg.rect.left   = af_roi_info->roi[0].x;
  roi_msg.rect.top    = af_roi_info->roi[0].y;
  roi_msg.rect.width  = af_roi_info->roi[0].dx;
  roi_msg.rect.height = af_roi_info->roi[0].dy;
  roi_msg.weight      = af_roi_info->weight[0];
  rc = af_send_bus_msg(port, MCT_BUS_MSG_SET_AF_ROI, &roi_msg,
    sizeof(cam_area_t), af_port->sof_id);

  return rc;
}

/** af_port_send_stats_config:
 *    @port:   port info
 *    @af_out: output af data
 *
 * Send AF stats configuration info to ISP.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_send_stats_config(mct_port_t *port,
  af_output_data_t *af_out)
{
  stats_config_t    stats_config;
  af_config_t       af_conf;
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);
  mct_event_t       event;
  boolean           rc = TRUE;
  int i;
  memset(&stats_config, 0, sizeof(stats_config_t));

  af_conf.stream_id = af_port->reserved_id;
  af_conf.roi.left =
    af_out->af_stats_config.region.x;
  af_conf.roi.top =
    af_out->af_stats_config.region.y;
  af_conf.roi.width =
    af_out->af_stats_config.region.dx;
  af_conf.roi.height =
    af_out->af_stats_config.region.dy;

  af_conf.grid_info.h_num = af_out->af_stats_config.rgn_h_num;
  af_conf.grid_info.v_num = af_out->af_stats_config.rgn_v_num;
  af_conf.r_min = af_out->af_stats_config.r_min;
  af_conf.b_min = af_out->af_stats_config.b_min;
  af_conf.gr_min = af_out->af_stats_config.gr_min;
  af_conf.gb_min = af_out->af_stats_config.gb_min;

  memcpy(&af_conf.hpf, &af_out->af_stats_config.hpf,
    sizeof(int) * MAX_HPF_COEFF);

  CDBG("%s: Sending AF stats config to ISP!"
    "roi.left: %d top: %d width: %d height: %d h_num: %d v_num: %d"
    "r_min: %d b_min: %d gr_min: %d gb_min: %d",
    __func__, af_conf.roi.left, af_conf.roi.top,
    af_conf.roi.width, af_conf.roi.height, af_conf.grid_info.h_num,
    af_conf.grid_info.v_num, af_conf.r_min, af_conf.b_min,
    af_conf.gr_min, af_conf.gb_min);
  CDBG("%s: HPF:", __func__);
  for (i = 0; i < MAX_HPF_COEFF; i++) {
    CDBG("hpf[%d]: %d", i, af_conf.hpf[i]);
  }

  /* update stats_config data structure */
  stats_config.stats_mask = (1 << MSM_ISP_STATS_BF);
  stats_config.af_config = &af_conf;

  if (af_port->isp_status.is_isp_ready == TRUE) {
    /* pack into an mct_event object*/
    event.direction = MCT_EVENT_UPSTREAM;
    event.identity = af_port->reserved_id;
    event.type = MCT_EVENT_MODULE_EVENT;
    event.u.module_event.type = MCT_EVENT_MODULE_STATS_CONFIG_UPDATE;
    event.u.module_event.module_event_data = (void *)(&stats_config);
    event.u.module_event.current_frame_id = af_port->sof_id;

    CDBG("%s: send AF_CONFIG data to ISP, port =%p, event =%p",
      __func__, port, &event);
    rc = MCT_PORT_EVENT_FUNC(port)(port, &event);
  } else {
    CDBG("%s: ISP not UP yet! Save the config data for later!", __func__);
    af_port->isp_status.need_to_send_af_config = TRUE;
    af_port->isp_status.config = af_conf;
  }

  return rc;
} /* af_port_send_stats_config */

/** af_port_send_af_info_to_metadata
 *  update af peak_location_index to metadata
 *  only AF_STATUS_FOCUSED case, the value is valid
 *  other cases the value will reset to zero
 **/
static void af_port_send_af_info_to_metadata(
  mct_port_t  *port,
  af_output_data_t *output) {
  mct_event_t             event;
  mct_bus_msg_t           bus_msg;
  af_output_eztune_data_t af_info;
  af_port_private_t       *private;
  int size;
  if (!output || !port) {
    ALOGE("%s: input error", __func__);
    return;
  }
  private = (af_port_private_t *)(port->port_private);
  bus_msg.sessionid = (private->reserved_id >> 16);
  bus_msg.type = MCT_BUS_MSG_AF_EZTUNING_INFO;
  bus_msg.msg = (void *)&af_info;
  size = (int)sizeof(af_output_eztune_data_t);
  bus_msg.size = size;

  memcpy(&af_info, &output->eztune.eztune_data, size);

  CDBG("%s: peak_location_index:%d", __func__, af_info.peak_location_index);
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = private->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)(&bus_msg);
  MCT_PORT_EVENT_FUNC(port)(port, &event);
}

/** af_port_handle_set_focus_mode_to_hal_type:

 * Handle AF mode event to HAL enums.
 *
 * @port: port info
 *
 * @set_parm: a message to populate
 *
 * mode: focus mode to be set
 *
 * Return: TRUE: Success  FALSE: Failure
 **/
uint8_t af_port_handle_set_focus_mode_to_hal_type(af_port_private_t *af_port,
  af_mode_type mode)
{
  uint8_t hal_mode;

  switch (mode) {
  case AF_MODE_AUTO:
    hal_mode = CAM_FOCUS_MODE_AUTO;
    break;
  case AF_MODE_INFINITY:
    hal_mode = CAM_FOCUS_MODE_INFINITY;
    break;
  case AF_MODE_MACRO:
    hal_mode = CAM_FOCUS_MODE_MACRO;
    break;
  case AF_MODE_CAF:
    if(af_port->stream_info.stream_type == CAM_STREAM_TYPE_VIDEO)
      hal_mode = CAM_FOCUS_MODE_CONTINOUS_VIDEO;
    else
      hal_mode = CAM_FOCUS_MODE_CONTINOUS_PICTURE;
    break;
  case AF_MODE_NOT_SUPPORTED:
    hal_mode = CAM_FOCUS_MODE_FIXED;
    break;
  default:
    hal_mode =  CAM_FOCUS_MODE_EDOF;
    break;
  }

  return hal_mode;
}

/** af_port_send_af_mobicat_info_to_metadata
 *  update af peak_location_index to metadata
 *  only AF_STATUS_FOCUSED case, the value is valid
 *  other cases the value will reset to zero
 **/
static void af_port_send_af_mobicat_info_to_metadata(
  mct_port_t  *port,
  af_output_data_t *output)
{
  mct_event_t             event;
  mct_bus_msg_t           bus_msg;
  af_output_mobicat_data_t af_info;
  af_port_private_t       *private;
  int size;
  if (!output || !port) {
    ALOGE("%s: input error", __func__);
    return;
  }
  private = (af_port_private_t *)(port->port_private);
  bus_msg.sessionid = (private->reserved_id >> 16);
  bus_msg.type = MCT_BUS_MSG_AF_MOBICAT_INFO;
  bus_msg.msg = (void *)&af_info;
  size = (int)sizeof(af_output_mobicat_data_t);
  bus_msg.size = size;

  memcpy(&af_info, &output->mobicat.mobicat_data, size);
  af_info.peak_location_index = output->mobicat.peakpos_index;

  ALOGD("%s: peak_location_index:%d", __func__,af_info.peak_location_index);
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = private->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)(&bus_msg);
  MCT_PORT_EVENT_FUNC(port)(port, &event);
}

/** af_port_send_afs_config:
 *    @port:   port info
 *    @af_out: output af data
 *
 * Send software AF configuration to the AFS module.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_send_afs_config(mct_port_t *port,
  af_output_data_t *af_out)
{
  mct_imglib_af_config_t imglib_af_config;
  af_config_t            af_conf;
  af_port_private_t      *af_port = (af_port_private_t *)(port->port_private);
  mct_event_t            event;
  boolean                rc = TRUE;

  /* Copy the ASF configuration info from the algorithm output */
  imglib_af_config.enable     = af_out->swaf_config.enable;
  imglib_af_config.frame_id   = af_out->swaf_config.frame_id;
  imglib_af_config.coeff_len  = af_out->swaf_config.coeff_len;
  imglib_af_config.roi.left   = af_out->swaf_config.roi.x;
  imglib_af_config.roi.top    = af_out->swaf_config.roi.y;
  imglib_af_config.roi.width  = af_out->swaf_config.roi.dx;
  imglib_af_config.roi.height = af_out->swaf_config.roi.dy;
  imglib_af_config.filter_type = af_out->swaf_config.sw_filter_type;
  imglib_af_config.FV_min     = af_out->swaf_config.fv_min;

  memcpy(&imglib_af_config.coeffa, &af_out->swaf_config.coeffa,
    sizeof(double) * MAX_SWAF_COEFFA_NUM);
  memcpy(&imglib_af_config.coeffb, &af_out->swaf_config.coeffb,
    sizeof(double) * MAX_SWAF_COEFFB_NUM);
  memcpy(&imglib_af_config.coeff_fir, &af_out->swaf_config.coeff_fir,
    sizeof(int) * MAX_SWAF_COEFFFIR_NUM);

  /* pack into an mct_event object*/
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = af_port->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_IMGLIB_AF_CONFIG;
  event.u.module_event.module_event_data = (void *)(&imglib_af_config);

  CDBG("%s: Send software AF CONFIG data to SAF, port =%p, event =%p",
    __func__, port, &event);

  rc = MCT_PORT_EVENT_FUNC(port)(port, &event);
  /* Allocate memory to create AF message. we'll post it to AF thread.*/
    q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
      malloc(sizeof(q3a_thread_af_msg_t));
    if (af_msg == NULL) {
      return FALSE;
    }
    memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
    af_msg->u.af_set_parm.u.p_sw_stats = af_port->p_sw_stats;
    af_msg->type = MSG_AF_SET;
    af_msg->u.af_set_parm.type = AF_SET_PARAM_SW_STATS_POINTER;
    CDBG("%s: Set sw stats backup pointer:%p", __func__, af_port->p_sw_stats);
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  return rc;
} /* af_port_send_afs_config */


/** af_port_send_exif_debug_data:
 *    @port:   TODO
 *    @stats_update_t: TODO
 *
 * TODO description
 *
 * Return nothing
 **/
static void af_port_send_exif_debug_data(mct_port_t *port)
{
  mct_event_t          event;
  mct_bus_msg_t        bus_msg;
  cam_af_exif_debug_t  *af_info;
  af_port_private_t    *private;
  int                  size;

  if (!port) {
    CDBG_ERROR("%s: input error", __func__);
    return;
  }
  private = (af_port_private_t *)(port->port_private);
  if (private == NULL) {
    return;
  }

  /* Send exif data if data size is valid */
  if (!private->af_debug_data_size) {
    CDBG("af_port: Debug data not available");
    return;
  }
  af_info = (cam_af_exif_debug_t *)malloc(sizeof(cam_af_exif_debug_t));
  if (!af_info) {
    CDBG_ERROR("Failure allocating memory for debug data");
    return;
  }
  bus_msg.sessionid = (private->reserved_id >> 16);
  bus_msg.type = MCT_BUS_MSG_AF_EXIF_DEBUG_INFO;
  bus_msg.msg = (void *)af_info;
  size = (int)sizeof(cam_af_exif_debug_t);
  bus_msg.size = size;
  memset(af_info, 0, size);
  af_info->af_debug_data_size = private->af_debug_data_size;

  CDBG("%s af_debug_data_size: %d", __func__, private->af_debug_data_size);
  memcpy(&(af_info->af_private_debug_data[0]), private->af_debug_data_array,
    private->af_debug_data_size);
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = private->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)(&bus_msg);
  MCT_PORT_EVENT_FUNC(port)(port, &event);
  if (af_info) {
    free(af_info);
  }
}


/** af_port_send_ISP_reconfig_msg:
 *    @af_port:   private port data
 *
 *  Send a message to the AF algorithm to request ISP AF stats reconfiguration
 *  after each streamon.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_send_ISP_reconfig_msg(af_port_private_t *af_port) {
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));
  if (af_msg == NULL) {
    CDBG_ERROR("%s: Not Enough memory.", __func__);
    return FALSE;
  }

  af_msg->type = MSG_AF_SET;
  af_msg->u.af_set_parm.type = AF_SET_PARAM_RECONFIG_ISP;

  return q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
}

/** af_port_callback: Callback function once AF module has some
 *  updates to send.
 *
 * Callback function once AF module has some updates to send.
 *
 * Return void
 **/
static void af_port_callback(af_output_data_t *af_out, void *p)
{
  mct_port_t        *port = (mct_port_t *)p;
  af_port_private_t *private = NULL;
  mct_event_t       event;
  stats_update_t    stats_update;
  af_update_t       af_update;
  boolean           send_update = FALSE;
  boolean           legacy_sent = FALSE;
  boolean           rc;
  af_port_state_transition_type cause;

  if (!af_out || !port) {
    CDBG_ERROR("%s: input error", __func__);
    return;
  }

  private = (af_port_private_t *)(port->port_private);
  if (!private) {
    return;
  }
  CDBG("%s: AF port callback", __func__);
  /* First reset AF update data structure */
  memset(&af_update, 0, sizeof(af_update_t));

  /* We'll process the output only if output has been updated
   * Should we keep the following order when we process the output?
   * Need to check.
   **/
  if (af_out->sof_id) {
    private->sof_id = af_out->sof_id;
  }

  if (af_out->result) {
    if (af_out->type & AF_OUTPUT_STATUS) {
      /* Update AF status event.
       * For AF status, we directly post it to bus
       **/
      af_port_process_status_update(port, af_out);
      legacy_sent = TRUE;
    }

    /* Update AF ROI event. */
    if (af_out->type & AF_OUTPUT_STATS_CONFIG) {
      /* Send event to ISP to configure new AF ROI */
      CDBG("%s: Send event to ISP to configure new ROI", __func__);
      af_port_send_stats_config(port, af_out);
    }

    if (af_out->type & AF_OUTPUT_SWAF_CONFIG) {
      /* Send event to AFS module to configure frame assisted AF */
      CDBG("%s: Send event to AFS module to configure frame assisted AF",
        __func__);
      af_port_send_afs_config(port, af_out);
    }

    if (af_out->type & AF_OUTPUT_STOP_AF) {
      /* stop AF. */
      CDBG("%s: Send Stop AF to ISP", __func__);
      af_update.stop_af = af_out->stop_af;
      send_update = TRUE;
    }

    if (af_out->type & AF_OUTPUT_CHECK_LED) {
      /* Send event to AEC to turn ON LED if required */
      CDBG("%s: Send check LED to AEC!", __func__);
      af_update.check_led = af_out->check_led;
      send_update = TRUE;
    }
    if (af_out->type & AF_OUTPUT_UPDATE_FOCUS_POS){
      cam_focus_pos_info_t cur_pos;
      cur_pos.scale = af_out->cur_lens_pos.scale;
      cur_pos.diopter = af_out->cur_lens_pos.diopter;
      CDBG("%s: Send lens position data to HAL:scale:%d;diopter: %f",
        __func__,cur_pos.scale,cur_pos.diopter);
      af_send_bus_msg(port, MCT_BUS_MSG_UPDATE_AF_FOCUS_POS,
        &cur_pos,sizeof(cam_focus_pos_info_t),private->sof_id);
    }
  }
  af_send_bus_msg(port, MCT_BUS_MSG_SET_AF_STATE, &private->af_trans.af_state,
    sizeof(cam_af_state_t), private->sof_id);

   if (legacy_sent == TRUE) {
    /* Avoid double sending the update */
    private->force_af_update = FALSE;
    af_port_send_legacy_status_update(port);
   } else if (legacy_sent == FALSE && private->force_af_update) {
    private->force_af_update = FALSE;
    af_port_send_legacy_status_update(port);
  }

  if (!private->af_initialized) {
    uint8_t focus_mode = CAM_FOCUS_MODE_FIXED;
    af_send_bus_msg(port, MCT_BUS_MSG_SET_AF_MODE, &focus_mode,
      sizeof(uint8_t), private->sof_id);
  } else if (af_out->focus_mode_info.mode) {
    uint8_t focus_mode;
    focus_mode = af_port_handle_set_focus_mode_to_hal_type(private,
      af_out->focus_mode_info.mode);
    af_send_bus_msg(port, MCT_BUS_MSG_SET_AF_MODE, &focus_mode,
      sizeof(uint8_t), private->sof_id);
  }

  af_send_bus_msg(port, MCT_BUS_MSG_SET_AF_TRIGGER_ID,
    &(private->af_trans.trigger_id), sizeof(int32_t), private->sof_id);
  af_port_send_roi_update(port, af_out);
  af_send_bus_msg(port, MCT_BUS_MSG_SET_AF_FOCUS_INFO,
    &(private->af_focus_distance), sizeof(cam_focus_distances_info_t),
    private->sof_id);

  if(af_out->type & AF_OUTPUT_EZ_METADATA) {
    if (af_out->eztune.eztune_data.ez_running == TRUE) {
      CDBG("%s: Send update af metadata!", __func__);
      af_port_send_af_info_to_metadata(port, af_out);
    }
  }

  if(af_out->type & AF_OUTPUT_MOBICAT_METADATA) {
    af_port_send_af_mobicat_info_to_metadata(port, af_out);
  }

  if (af_out->type & AF_OUTPUT_RESET_AEC) {
    CDBG("%s: Send update AEC reset!", __func__);
    af_send_bus_msg(port, MCT_BUS_MSG_SET_AEC_RESET,
      NULL, 0, private->sof_id);
  }
  if (af_out->type & AF_OUTPUT_UPDATE_EVENT) {

    /* Save the debug data in private data struct to be sent out later */
    CDBG("%s: Save debug data in port private", __func__);
    private->af_debug_data_size = af_out->af_debug_data_size;
    if (af_out->af_debug_data_size) {
      memcpy(private->af_debug_data_array, af_out->af_debug_data_array,
        af_out->af_debug_data_size);
    }
  }
  if (send_update) {
    CDBG("%s: Creating output msg to send!", __func__);
    stats_update.flag = STATS_UPDATE_AF;
    stats_update.af_update = af_update;

    /* pack into an mct_event object*/
    event.direction = MCT_EVENT_UPSTREAM;
    event.identity = private->reserved_id;
    event.type = MCT_EVENT_MODULE_EVENT;
    event.u.module_event.current_frame_id = private->sof_id;
    event.u.module_event.type = MCT_EVENT_MODULE_STATS_AF_UPDATE;
    event.u.module_event.module_event_data = (void *)(&stats_update);

    CDBG("%s: send MCT_EVENT_MODULE_STATS_AF_UPDATE to af port,",
      " port =%p, event =%p", __func__, port, &event);
    rc = MCT_PORT_EVENT_FUNC(port)(port, &event);
    if(rc == FALSE) {
      CDBG("%s: Msg not sent!", __func__);
    }
  }
  CDBG("%s: x", __func__);
}


/** af_port_send_move_lens_cmd:
 *    @output: output AF data
 *    @p:      AF port
 *
 * Callback function that will ask the actuator to move the lens.
 *
 * Return the result of the command: TRUE if the lens is moved, FALSE if not
 **/
static boolean af_port_send_move_lens_cmd(void *output, int32_t p)
{
  mct_port_t        *port = (mct_port_t *)p;
  af_port_private_t *private = NULL;
  stats_update_t    stats_update;
  af_update_t       af_update;
  mct_event_t       event;
  af_output_data_t  *af_out = (af_output_data_t *)output;
  boolean           rc;

  private = (af_port_private_t *)(port->port_private);
  if (!private) {
    CDBG_ERROR("%s: Pointer to port's private data is NULL!", __func__);
    return FALSE;
  }

  memset(&af_update, 0, sizeof(af_update_t));
  af_update.stats_frm_id = af_out->frame_id;

  if (af_out->type & AF_OUTPUT_RESET_LENS) {
    /* Ask sensor module to reset the lens */
    CDBG("%s: Asking actuator to reset the lens!", __func__);
    af_update.reset_lens = af_out->reset_lens.reset_lens;
    af_update.reset_pos = af_out->reset_lens.reset_pos;
  } else if (af_out->type & AF_OUTPUT_MOVE_LENS) {
    /* If we have to move the lens, ask sensor module */
    if (af_out->move_lens.use_dac_value == FALSE) {
      af_update.use_dac_value = FALSE;
      af_update.move_lens = af_out->move_lens.move_lens;
      af_update.direction = af_out->move_lens.direction;
      af_update.num_of_steps = af_out->move_lens.num_of_steps;
      CDBG("%s: Asking actuator to move the lens! flag: %d, steps: %d, dir: %d",
        __func__,
        af_update.move_lens,
        af_update.num_of_steps,
        af_update.direction);
    } else {
      int i;
      CDBG("%s: Use DAC. Num of interval: %d", __func__,
        af_out->move_lens.num_of_interval);
      af_update.move_lens = af_out->move_lens.move_lens;
      af_update.use_dac_value = TRUE;
      af_update.num_of_interval = af_out->move_lens.num_of_interval;
      for (i = 0; i < af_update.num_of_interval; i++) {
        af_update.pos[i] = af_out->move_lens.pos[i];
        af_update.delay[i] = af_out->move_lens.delay[i];
        CDBG("%s: pos[%d] = %d delay[%d] = %d", __func__,
          i, af_update.pos[i], i, af_update.delay[i]);
      }
    }
  }

  CDBG("%s: Creating output msg to send!", __func__);
  stats_update.flag = STATS_UPDATE_AF;
  stats_update.af_update = af_update;

  /* pack into an mct_event object*/
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = private->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_AF_UPDATE;
  event.u.module_event.module_event_data = (void *)(&stats_update);
  event.u.module_event.current_frame_id = private->sof_id;

  CDBG("%s: send AF_UPDATE to af port, port = %p, event = %p",
    __func__, port, &event);
  rc = MCT_PORT_EVENT_FUNC(port)(port, &event);

  return rc;
}

/** af_port_set_af_data:
 *    @af_port: private port data
 *
 * Set/reset some AF internal parameters to default values
 * and reset the lens to known default position.
 *
 * Return void
 **/
static void af_port_set_af_data(af_port_private_t *af_port)
{
  CDBG("%s: Set AF data!", __func__);

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));

  if (af_msg != NULL) {
    memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
    af_msg->type = MSG_AF_SET;
    af_msg->u.af_set_parm.type = AF_SET_PARAM_INIT;

    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }
}

/** af_port_handle_capture_intent:
 **/
static void af_port_handle_capture_intent(af_port_private_t *af_port,
  cam_intent_t capture_intent) {
  CDBG("%s: Handle Capture Intent: %d!", __func__, capture_intent);
  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));

  if (af_msg != NULL) {
    memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
    af_msg->type = MSG_AF_SET;
    af_msg->u.af_set_parm.type = AF_SET_PARAM_RUN_MODE;

    /* Update which mode is this - Preview/Video/Snapshot */
    switch (capture_intent) {
    case CAM_INTENT_VIDEO_RECORD:
      af_msg->u.af_set_parm.u.af_run_mode = AF_RUN_MODE_VIDEO;
      break;
    case CAM_INTENT_PREVIEW:
    case CAM_INTENT_ZERO_SHUTTER_LAG:
      af_msg->u.af_set_parm.u.af_run_mode = AF_RUN_MODE_CAMERA;
      break;
    case CAM_INTENT_VIDEO_SNAPSHOT:
    case CAM_INTENT_STILL_CAPTURE:
      af_msg->u.af_set_parm.u.af_run_mode = AF_RUN_MODE_SNAPSHOT;
      break;
    case CAM_INTENT_CUSTOM:
    default:
      af_msg->u.af_set_parm.u.af_run_mode = AF_RUN_MODE_CAMERA;
      break;
    }
    CDBG("%s: Updating AF Run type: %d", __func__,
      af_msg->u.af_set_parm.u.af_run_mode);
    af_port->run_type = af_msg->u.af_set_parm.u.af_run_mode;
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }
} /* af_port_handle_capture_intent */

/** af_port_handle_set_crop_region_event:
 **/
static void af_port_handle_set_crop_region_event(af_port_private_t *af_port,
  cam_crop_region_t *crop_region) {
  af_crop_info_t crop_info;

  CDBG("%s: Handle Set Crop Region Event!", __func__);
  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));

  if (af_msg != NULL) {
    memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
    af_msg->type = MSG_AF_SET;
    af_msg->u.af_set_parm.type = AF_SET_PARAM_CROP_REGION;

    crop_info.x = crop_region->left;
    crop_info.y = crop_region->top;
    crop_info.dx = crop_region->width;
    crop_info.dy = crop_region->height;
    CDBG("%s: Input Crop Info: x; %d y: %d dx: %d dy: %d", __func__,
      crop_info.x, crop_info.y, crop_info.dx, crop_info.dy);

    af_msg->u.af_set_parm.u.crop_info = crop_info;
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }
} /* af_port_handle_set_crop_region_event */


/** af_port_handle_stream_mode_update:
 *    @af_port: private port data
 *
 * Update the camera operation mode
 *
 * Return void
 **/
static void af_port_handle_stream_mode_update(af_port_private_t *af_port) {
  CDBG("%s: Update Stream Mode!", __func__);

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));

  if (af_msg != NULL ) {
    memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
    af_msg->type = MSG_AF_SET;
    af_msg->u.af_set_parm.type = AF_SET_PARAM_RUN_MODE;

    /* Update which mode is this - Preview/Video/Snapshot */
    switch (af_port->stream_info.stream_type) {
    case CAM_STREAM_TYPE_VIDEO: {
      af_msg->u.af_set_parm.u.af_run_mode = AF_RUN_MODE_VIDEO;
    }
      break;

    case CAM_STREAM_TYPE_PREVIEW: {
      af_msg->u.af_set_parm.u.af_run_mode = AF_RUN_MODE_CAMERA;
    }
      break;

    case CAM_STREAM_TYPE_RAW:
    case CAM_STREAM_TYPE_SNAPSHOT: {
      af_msg->u.af_set_parm.u.af_run_mode = AF_RUN_MODE_SNAPSHOT;
    }
      break;

    default: {
      af_msg->u.af_set_parm.u.af_run_mode = AF_RUN_MODE_CAMERA;
    }
      break;
    }
    CDBG("%s: Updating stream mode to: %d", __func__,
      af_msg->u.af_set_parm.u.af_run_mode);
    af_port->run_type = af_msg->u.af_set_parm.u.af_run_mode;
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }
} /* af_port_handle_stream_mode_update */

/** af_port_handle_af_tuning_update:
 *    @af_port: private port data
 *    @mod_evt: module event containing tuning header pointer
 *
 * Update the af tuning header if it has changed.
 *
 * Return nothing
 **/
static void af_port_handle_af_tuning_update(af_port_private_t * af_port,
  mct_event_module_t *mod_evt)
{
  af_algo_tune_parms_t *tuning_info;
  CDBG("%s: Handle af_tuning header update event!", __func__);
  tuning_info = (af_algo_tune_parms_t *)mod_evt->module_event_data;

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));

  if (af_msg != NULL) {
    memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
    af_msg->type = MSG_AF_SET;
    af_msg->u.af_set_parm.type = AF_SET_PARAM_UPDATE_TUNING_HDR;
    af_msg->u.af_set_parm.u.af_init_param.tuning_info = tuning_info;

    af_msg->u.af_set_parm.u.af_init_param.op_mode = af_port->run_type;

    af_msg->u.af_set_parm.u.af_init_param.preview_size.width =
      af_port->stream_info.dim.width;
    af_msg->u.af_set_parm.u.af_init_param.preview_size.height =
      af_port->stream_info.dim.height;
    af_msg->u.af_set_parm.u.af_init_param.af_tuning_params.\
      af_caf_trigger_after_taf = AF_CAF_TRIGGER_AFTER_TAF;

    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

    /* Also update tuning header for fd_prio AF */
    af_port->fd_prio_data.tuning_info = tuning_info;

    /* And save preview_size */
    af_port->preview_size.width = af_port->stream_info.dim.width;
    af_port->preview_size.height = af_port->stream_info.dim.height;

    CDBG("%s: Preview Size: %dx%d", __func__, af_port->preview_size.width,
      af_port->preview_size.height);

    /* Also update any pre-init module updates we've received so far */
    if (af_port->pre_init_updates.is_stream_info_updated == TRUE) {
      CDBG("%s: Update saved ISP info!", __func__);
      af_port_handle_isp_output_dim_event(af_port,
        &af_port->pre_init_updates.stream_info);
      af_port->pre_init_updates.is_stream_info_updated == FALSE;
    }

    /* Now we can start sending events to AF algorithm to process */
    af_port->af_initialized = TRUE;
  }
} /* af_port_handle_af_tuning_update */

/** af_port_handle_chromatix_update_evt:
 *    @af_port: private port data
 *    @mod_evt: module event containing chromatix pointer
 *
 * Update the chromatix pointer if the chromatix is reloaded.
 *
 * Return void
 **/
static void af_port_handle_chromatix_update_evt(af_port_private_t *af_port,
  mct_event_module_t *mod_evt)
{
  modulesChromatix_t *mod_chrom =
    (modulesChromatix_t *)mod_evt->module_event_data;

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));

  if (af_msg != NULL ) {
    memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
    af_msg->type = MSG_AF_SET;
    af_msg->u.af_set_parm.type = AF_SET_PARAM_INIT_CHROMATIX_PTR;
    af_msg->u.af_set_parm.u.af_init_param.chromatix =
      mod_chrom->chromatixPtr;
    af_msg->u.af_set_parm.u.af_init_param.comm_chromatix =
      mod_chrom->chromatixComPtr;
    af_msg->u.af_set_parm.u.af_init_param.preview_size.width =
      af_port->stream_info.dim.width;
    af_msg->u.af_set_parm.u.af_init_param.preview_size.height =
      af_port->stream_info.dim.height;
    af_msg->u.af_set_parm.u.af_init_param.af_tuning_params.\
      af_caf_trigger_after_taf = AF_CAF_TRIGGER_AFTER_TAF;
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }
}

/** af_port_handle_aec_update:
 *    @af_port: private port data
 *    @mod_evt: module event structure
 *
 * AEC has some updates. Save ones AF needs.
 *
 * Return void
 **/
static void af_port_handle_aec_update(af_port_private_t * af_port,
  mct_event_module_t * mod_evt) {
  af_input_from_aec_t *aec_info = NULL;
  stats_update_t *stats_update = (stats_update_t *)mod_evt->module_event_data;

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));
  if (af_msg != NULL) {
    memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
    af_msg->type = MSG_AF_SET;
    /* Update message contents - first set param type */
    af_msg->u.af_set_parm.type = AF_SET_PARAM_UPDATE_AEC_INFO;
    /* Copy aec data now */
    aec_info = &(af_msg->u.af_set_parm.u.aec_info);

    aec_info->aec_settled = stats_update->aec_update.settled;
    aec_info->cur_luma = stats_update->aec_update.cur_luma;
    aec_info->target_luma = stats_update->aec_update.target_luma;
    aec_info->luma_settled_cnt = stats_update->aec_update.luma_settled_cnt;
    aec_info->cur_real_gain = stats_update->aec_update.real_gain;
    aec_info->exp_index = stats_update->aec_update.exp_index;
    aec_info->lux_idx = stats_update->aec_update.lux_idx;
    aec_info->exp_tbl_val = stats_update->aec_update.exp_tbl_val;
    aec_info->comp_luma = stats_update->aec_update.comp_luma;
    aec_info->pixels_per_region = stats_update->aec_update.pixelsPerRegion;
    aec_info->num_regions = stats_update->aec_update.numRegions;
    aec_info->exp_time = stats_update->aec_update.exp_time;
    aec_info->preview_fps = stats_update->aec_update.preview_fps;
    aec_info->preview_linesPerFrame =
      stats_update->aec_update.preview_linesPerFrame;
    aec_info->linecnt = stats_update->aec_update.linecount;
    aec_info->target_luma = stats_update->aec_update.target_luma;
    if (stats_update->aec_update.SY) {
      memcpy(aec_info->SY, stats_update->aec_update.SY,
        sizeof(unsigned long) * MAX_YUV_STATS_NUM);
      aec_info->Av_af = stats_update->aec_update.Av;
      aec_info->Tv_af = stats_update->aec_update.Tv;
      aec_info->Bv_af = stats_update->aec_update.Bv;
      aec_info->Sv_af = stats_update->aec_update.Sv;
      aec_info->Ev_af = stats_update->aec_update.Ev;
      CDBG("%s: preview_fps %f, exp time %f",__func__,
        (float)aec_info->preview_fps/256.0,aec_info->exp_time);
      CDBG("%s: Bv_af: %f", __func__, aec_info->Bv_af);
      af_port->latest_aec_info = *aec_info;
      q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
    }
  }
} /* af_port_handle_aec_update */

/** af_port_handle_gyro_update:
 *    @af_port: private port data
 *    @mod_evt: module event structure
 *
 * Update gyro data.
 *
 * Return void.
 **/
static void af_port_handle_gyro_update(af_port_private_t * af_port,
  mct_event_module_t * mod_evt)
{
  af_input_from_gyro_t   *gyro_info = NULL;
  mct_event_gyro_stats_t *gyro_update =
    (mct_event_gyro_stats_t *)mod_evt->module_event_data;
  int                    i = 0;

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));
  if (af_msg != NULL) {
    memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
    af_msg->type = MSG_AF_SET;
    /* Update message contents - first set param type*/
    af_msg->u.af_set_parm.type = AF_SET_PARAM_UPDATE_GYRO_INFO;
    /* Copy gyro data now */
    gyro_info = &(af_msg->u.af_set_parm.u.gyro_info);

    gyro_info->q16_ready = TRUE;
    gyro_info->float_ready = TRUE;

    for (i = 0; i < 3; i++)
    {
      gyro_info->q16[i] = (long) gyro_update->q16_angle[i];
      gyro_info->flt[i] = (float)gyro_update->q16_angle[i] / (1 << 16);
      CDBG("%s: i: %d q16: %ld flt: %f", __func__, i,
        gyro_info->q16[i], gyro_info->flt[i]);
    }

    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }
} /* af_port_handle_gyro_update */

/** af_port_handle_sensor_update:
 *    @af_port: private port data
 *    @mod_evt: module event structure
 *
 * Sensor has some updates. Save once AF needs.
 *
 * Return void.
 **/
static void af_port_handle_sensor_update(af_port_private_t * af_port,
  mct_event_module_t * mod_evt)
{
  af_input_from_sensor_t *sensor_info = NULL;
  sensor_out_info_t      *sensor_update =
    (sensor_out_info_t *)(mod_evt->module_event_data);

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));
  if (af_msg != NULL) {
    memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
    af_msg->type = MSG_AF_SET;
    /* Update message contents - first set param type */
    af_msg->u.af_set_parm.type = AF_SET_PARAM_UPDATE_SENSOR_INFO;

    /* Copy af data now */
    sensor_info = &(af_msg->u.af_set_parm.u.sensor_info);

    sensor_info->af_not_supported =
      !(sensor_update->af_lens_info.af_supported);
    //sensor_info->preview_fps = ;
    sensor_info->max_preview_fps = sensor_update->max_fps;
    sensor_info->actuator_info.focal_length =
      sensor_update->af_lens_info.focal_length;
    sensor_info->actuator_info.af_f_num = sensor_update->af_lens_info.f_number;
    sensor_info->actuator_info.af_f_pix = sensor_update->af_lens_info.pix_size;
    sensor_info->actuator_info.af_total_f_dist =
      sensor_update->af_lens_info.total_f_dist;
    sensor_info->actuator_info.hor_view_angle =
      sensor_update->af_lens_info.hor_view_angle;
    sensor_info->actuator_info.ver_view_angle =
      sensor_update->af_lens_info.ver_view_angle;
    sensor_info->actuator_info.um_per_dac = sensor_update->af_lens_info.um_per_dac;
    sensor_info->actuator_info.dac_offset = sensor_update->af_lens_info.dac_offset;
    sensor_info->sensor_res_height =
      sensor_update->request_crop.last_line -
      sensor_update->request_crop.first_line + 1;
    sensor_info->sensor_res_width =
      sensor_update->request_crop.last_pixel -
      sensor_update->request_crop.first_pixel + 1;

    CDBG("%s: Sensor Res width: %d height: %d", __func__,
      sensor_info->sensor_res_width, sensor_info->sensor_res_height);

    /* Update CAMIF size for FD-AF too */
    af_port->fd_prio_data.camif_height = sensor_info->sensor_res_height;
    af_port->fd_prio_data.camif_width = sensor_info->sensor_res_width;

    /* Also if AF isn't supported */
    if (sensor_info->af_not_supported == TRUE) {
      af_port->af_initialized = FALSE;
    }
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }
} /* af_port_handle_sensor_update */

/** af_port_handle_fd_event:
 *    @af_port:   private port data
 *    @mod_event: module event
 *
 * Handle new face detection ROIs
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_fd_event(af_port_private_t *af_port,
  mct_event_module_t *mod_evt) {
  boolean rc = TRUE;
  mct_face_info_t *face_info = NULL;
  face_info = (mct_face_info_t *)mod_evt->module_event_data;

  if (!face_info) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }

  af_port->fd_prio_data.pface_info = face_info;
  af_port->fd_prio_data.current_frame_id = face_info->frame_id;
  af_fdprio_process(&af_port->fd_prio_data, AF_FDPRIO_CMD_PROC_FD_ROI);

  return rc;
} /* af_port_handle_fd_event */

/** af_port_handle_sof_event
 * Send Sof id
 *
 * @af_port: private port number
 *
 * @mod_event: module event
 *
 **/
static boolean af_port_handle_sof_event(mct_port_t *port,
  mct_event_module_t *mod_evt) {
  boolean rc = TRUE;
  mct_bus_msg_isp_sof_t *sof_event;
  af_port_private_t *af_port =
    (af_port_private_t *)(port->port_private);
  sof_event = (mct_bus_msg_isp_sof_t *)(mod_evt->module_event_data);

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));
  if (af_msg != NULL) {
    memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
    af_msg->type = MSG_AF_SET;
    /* Update message contents - first set param type */
    af_msg->u.af_set_parm.type = AF_SET_PARAM_SOF;
    af_msg->u.af_set_parm.u.af_set_sof_id = sof_event->frame_id;
    rc &= q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }

  CDBG("%s: Send exif info update with debug data", __func__);
  af_port_send_exif_debug_data(port);
  return rc;
} /* af_port_handle_sof_event */

/** af_port_handle_stats_event
 *    @af_port:   private port data
 *    @mod_event: module event
 *
 * Once we receive AF stats, we need to ask af algorithm to process it.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_stats_event(af_port_private_t * af_port,
  mct_event_module_t * mod_evt)
{
  boolean               rc = TRUE;
  mct_event_stats_isp_t *stats_event =
    (mct_event_stats_isp_t *)mod_evt->module_event_data;

  if (stats_event) {
    /* First check if these are AF stats. No need to allocate and free
     * memory if we are going to ignore this event.
     */
    if ((stats_event->stats_mask & (1 << MSM_ISP_STATS_AF)) ||
      (stats_event->stats_mask & (1 << MSM_ISP_STATS_BF))) {
      /* Allocate memory to create AF message. we'll post it to AF thread.*/
      q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
        malloc(sizeof(q3a_thread_af_msg_t));
      if (af_msg == NULL) {
        CDBG_ERROR("%s Error allocating memory.", __func__);
        return FALSE;
      }
      memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));

      stats_af_t *af_stats = (stats_af_t *)malloc(sizeof(stats_af_t));
      if (af_stats == NULL) {
        CDBG_ERROR("%s Error allocating memory.", __func__);
        free(af_msg);
        return FALSE;
      }
      /* No need to memset to 0 the allocated buffer. It will be set by
       * copying the stats into it. The rest of the fields are also
       * set appropriately.
       */
      af_stats->frame_id = stats_event->frame_id;
      /* Copy field by field - the two structures are NOT of the same type */
      af_stats->time_stamp.time_stamp_sec = stats_event->timestamp.tv_sec;
      af_stats->time_stamp.time_stamp_us  = stats_event->timestamp.tv_usec;

      CDBG("%s: Stats mask: %x", __func__, stats_event->stats_mask);
      /* If it is YUV stats */
      if (stats_event->stats_mask & (1 << MSM_ISP_STATS_AF)) {
        af_msg->type = MSG_AF_STATS;
        af_stats->stats_type_mask = STATS_AF;
        CDBG("%s: stats_buf: %p", __func__,
          stats_event->stats_data[MSM_ISP_STATS_AF].stats_buf);
        memcpy(&(af_stats->u.q3a_af_stats),
          stats_event->stats_data[MSM_ISP_STATS_AF].stats_buf,
          sizeof(q3a_af_stats_t));
      /* If it is bayer stats */
      } else if (stats_event->stats_mask & (1 << MSM_ISP_STATS_BF)) {
        af_msg->type = MSG_BF_STATS;
        af_stats->stats_type_mask = STATS_BF;
        CDBG("%s: stats_buf: %p", __func__,
          stats_event->stats_data[MSM_ISP_STATS_BF].stats_buf);
        memcpy(&(af_stats->u.q3a_bf_stats),
          stats_event->stats_data[MSM_ISP_STATS_BF].stats_buf,
          sizeof(q3a_bf_stats_t));
        /* Need to call this function for every frame */
        af_port->fd_prio_data.current_frame_id = stats_event->frame_id;
        if (af_port->af_initialized) {
          af_fdprio_process(&af_port->fd_prio_data, AF_FDPRIO_CMD_PROC_COUNTERS);
        }
      }
      af_msg->u.stats = af_stats;
      /* Enqueue the message to the AF thread */
      rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
      /* If the AF thread is inactive, it will not enqueue our
       * message and instead will free it. Then we need to manually
       * free the payload */
      if (rc == FALSE) {
        free(af_stats);
      }
    }
  }

  return rc;
} /* af_port_handle_stats_event */

/** af_port_handle_stream_crop_event:
 *    @af_port:   private port data
 *    @mod_event: module event
 *
 * When we receive the crop info, we need to save it to be able
 * to calculate the right ROI when the HAL sends the command to
 * set the ROI.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_stream_crop_event(af_port_private_t * af_port,
  mct_event_module_t * mod_evt)
{
  boolean                   rc = TRUE;
  mct_bus_msg_stream_crop_t *stream_crop;

  stream_crop = (mct_bus_msg_stream_crop_t *)mod_evt->module_event_data;
  if (!stream_crop) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));
  if (af_msg == NULL) {
    return FALSE;
  }
  memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));

  af_msg->u.af_set_parm.u.stream_crop.pp_x           = stream_crop->x;
  af_msg->u.af_set_parm.u.stream_crop.pp_y           = stream_crop->y;
  af_msg->u.af_set_parm.u.stream_crop.pp_crop_out_x  = stream_crop->crop_out_x;
  af_msg->u.af_set_parm.u.stream_crop.pp_crop_out_y  = stream_crop->crop_out_y;
  af_msg->u.af_set_parm.u.stream_crop.vfe_map_x      = stream_crop->x_map;
  af_msg->u.af_set_parm.u.stream_crop.vfe_map_y      = stream_crop->y_map;
  af_msg->u.af_set_parm.u.stream_crop.vfe_map_width  = stream_crop->width_map;
  af_msg->u.af_set_parm.u.stream_crop.vfe_map_height = stream_crop->height_map;
  af_msg->type = MSG_AF_SET;
  af_msg->u.af_set_parm.type = AF_SET_PARAM_STREAM_CROP_INFO;

  /* Also save the crop info locally */
  memcpy(&af_port->stream_crop, &af_msg->u.af_set_parm.u.stream_crop,
    sizeof(af_stream_crop_t));
  CDBG("%s: pp_x: %d pp_y: %d pp_crop_out_x: %d pp_crop_out_y: %d"
    "vfe_map_x: %d vfe_map_y: %d vfe_map_width: %d vfe_map_height: %d",
    __func__, stream_crop->x, stream_crop->y, stream_crop->crop_out_x,
    stream_crop->crop_out_y, stream_crop->x_map, stream_crop->y_map,
    stream_crop->width_map, stream_crop->height_map);
  rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

  if (af_port->reconfigure_ISP_pending == TRUE) {
    /* Keep trying if failure occurs while sending the message. */
    if (af_port_send_ISP_reconfig_msg(af_port) == TRUE) {
      af_port->reconfigure_ISP_pending = FALSE;
    }
  }

  return rc;
} /* af_port_handle_stream_crop_event */

/** af_port_handle_isp_output_dim_event:
 *    @af_port:   private port data
 *    @mod_event: module event
 *
 * When we receive the isp dim info, we need to save it to be able
 * to calculate the right ROI when the HAL sends the command to
 * set the ROI.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_isp_output_dim_event(af_port_private_t *af_port,
  mct_stream_info_t *stream_info) {
  boolean rc = TRUE;
  af_input_from_isp_t *isp_info = NULL;

  if (!stream_info) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));
  if (af_msg == NULL) {
    return FALSE;
  }
  memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));

  isp_info = &(af_msg->u.af_set_parm.u.isp_info);
  isp_info->width = stream_info->dim.width;
  isp_info->height = stream_info->dim.height;

  /* save the isp info here too */
  memcpy(&af_port->isp_info, isp_info, sizeof(af_input_from_isp_t));

  af_msg->type = MSG_AF_SET;
  af_msg->u.af_set_parm.type = AF_SET_PARAM_UPDATE_ISP_INFO;

  rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);


  return rc;
} /* af_port_handle_isp_output_dim_event */

/** af_port_set_move_lens_cb_data:
 *    @af_port: private port data
 *
 * Send the lens move command function to the library along with the port.
 * The port is needed to call the event function.
 *
 * Return void.
 **/
static void af_port_set_move_lens_cb_data(mct_port_t *port)
{
  af_port_private_t *af_port = (af_port_private_t *)(port->port_private);

  CDBG("%s: Set move lens CB data!", __func__);
  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));

  if (af_msg != NULL) {
    memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
    af_msg->type = MSG_AF_SET;
    af_msg->u.af_set_parm.type = AF_SET_PARAM_MOVE_LENS_CB;

    af_msg->u.af_set_parm.u.move_lens_cb_info.move_lens_cb =
      af_port_send_move_lens_cmd;
    af_msg->u.af_set_parm.u.move_lens_cb_info.object_id = (int32_t)port;

    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  }
} /* af_port_set_move_lens_cb_data */

/** af_port_handle_imglib_output:
 *    @af_port:   private port data
 *    @mod_event: module event
 *
 * Handle new face detection ROIs
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_imglib_output(af_port_private_t * af_port,
  mct_event_module_t * mod_evt)
{
  boolean                rc = TRUE;
  mct_imglib_af_output_t *imglib_af_output;
  af_sw_stats_t          *sw_stats;

  imglib_af_output = (mct_imglib_af_output_t *)mod_evt->module_event_data;
  if (!imglib_af_output) {
    CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
    return FALSE;
  }

  /* Copy stats from imglib to AF Port */
  af_port->p_sw_stats->frame_id = imglib_af_output->frame_id;
  af_port->p_sw_stats->fV = imglib_af_output->fV;
  af_port->p_sw_stats->pending = FALSE;
  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));
  if (af_msg == NULL) {
    return FALSE;
  }
  memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
  /* Pack imglib stats to Core */
  sw_stats = &(af_msg->u.af_set_parm.u.sw_stats);
  sw_stats->frame_id = imglib_af_output->frame_id;
  sw_stats->fV = imglib_af_output->fV;
  sw_stats->pending = FALSE;

  af_msg->type = MSG_AF_SET;
  af_msg->u.af_set_parm.type = AF_SET_PARAM_IMGLIB_OUTPUT;

  rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

  return rc;
} /* af_port_handle_imglib_output */

/** af_port_handle_isp_stats_info:
 *    @af_port: private port data
 *    @mod_evt: module event structure
 *
 * Send AF stats capabilities to the library.
 *
 * Return void.
 **/
static void af_port_handle_isp_stats_info(af_port_private_t * af_port,
  mct_event_module_t * mod_evt)
{
  mct_stats_info_t *stats_info =
    (mct_stats_info_t *)mod_evt->module_event_data;
  int kernel_size = 0;

  /* Inform AF algorithm about how big is the kernel size */
  /* This is till the time when there is a better mechanism
     to handle multiple AF kernel sizes */

  /* Allocate memory to create AF message. we'll post it to AF thread.*/
  CDBG("%s: HPF kernel size: %d", __func__, stats_info->kernel_size);
  if (stats_info->kernel_size == MCT_EVENT_STATS_HPF_LEGACY) {
    kernel_size = AF_STATS_HPF_LEGACY;
  } else if (stats_info->kernel_size == MCT_EVENT_STATS_HPF_2X13) {
    kernel_size = AF_STATS_HPF_2x13;
  } else {
    kernel_size = AF_STATS_HPF_2x5;
  }

  q3a_thread_af_msg_t *af_msg2 = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));
  if (af_msg2 != NULL) {
    memset(af_msg2, 0, sizeof(q3a_thread_af_msg_t));
    af_msg2->type = MSG_AF_SET;
    /* Update message contents - first set param type*/
    af_msg2->u.af_set_parm.type = AF_SET_PARAM_UPDATE_KERNEL_SIZE;
    af_msg2->u.af_set_parm.u.kernel_size = kernel_size;
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg2);
  }

} /* af_port_handle_stats_info */

/** af_port_handle_module_event:
 *    @port:    AF port data
 *    @mod_evt: module event
 *
 * Handle module event received at AF port.
 *
 * Return void
 **/
static void af_port_handle_module_event(mct_port_t *port,
  mct_event_module_t *mod_evt)
{
  af_port_private_t *af_port =
    (af_port_private_t *)(port->port_private);

  CDBG("%s: Handle AF module event of type: %d", __func__, mod_evt->type);
  switch (mod_evt->type) {
    /* Event to update chromatix pointer */
  case MCT_EVENT_MODULE_SET_RELOAD_CHROMATIX:
  case MCT_EVENT_MODULE_SET_CHROMATIX_PTR: {
    CDBG("%s: Set chromatix update", __func__);
    /* Depending upon whether we use chromatix or af tuning header
     * we'll use one or the other for initialization
     **/
    if (af_port->af_initialized) {
      af_port_handle_chromatix_update_evt(af_port, mod_evt);
    }
  }
    break;

    /* Event to set af tuning header pointer */
  case MCT_EVENT_MODULE_SET_RELOAD_AFTUNE:
  case MCT_EVENT_MODULE_SET_AF_TUNE_PTR: {
    CDBG("%s: Set AF tuning header", __func__);
    af_port_handle_af_tuning_update(af_port, mod_evt);
    af_port_set_move_lens_cb_data(port);
    af_port_set_af_data(af_port);
  }
    break;

    /* Event to query thread object */
  case MCT_EVENT_MODULE_STATS_GET_THREAD_OBJECT: {
    CDBG("%s: Get thread object", __func__);
    q3a_thread_af_data_t *data =
      (q3a_thread_af_data_t *)(mod_evt->module_event_data);
    af_port->thread_data = data->thread_data;
    af_port->fd_prio_data.thread_data = data->thread_data;
    data->af_port = port;
    data->af_cb = af_port_callback;
    data->af_obj = &(af_port->af_object);
  }
    break;

  case MCT_EVENT_MODULE_SET_STREAM_CONFIG: {
    /* Update sensor info */
    CDBG("%s: Update sensor info!", __func__);
    af_port_handle_sensor_update(af_port, mod_evt);
    af_port_handle_stream_mode_update(af_port);

  }
    break;

  /* Event when new AF stats is available */
  case MCT_EVENT_MODULE_STATS_DATA: {
    if (af_port->af_initialized) {
      CDBG("%s: Handle stats event", __func__);
      af_port_handle_stats_event(af_port, mod_evt);
    }
  } /* case MCT_EVENT_MODULE_STATS_DATA */
    break;

  /* Event when AEC has some update to share*/
  case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {
    if (af_port->af_initialized) {
      CDBG("%s: Handle aec update", __func__);
      af_port_handle_aec_update(af_port, mod_evt);
    }
  }
    break;

  /* Event on every frame containing the crop info */
  case MCT_EVENT_MODULE_STREAM_CROP: {
    if (af_port->af_initialized) {
      CDBG("%s: Handle stream crop event", __func__);
      af_port_handle_stream_crop_event(af_port, mod_evt);
    }
  }
    break;

  case MCT_EVENT_MODULE_ISP_OUTPUT_DIM: {
    CDBG("%s: Event MCT_EVENT_MODULE_ISP_OUTPUT_DIM received!", __func__);
    mct_stream_info_t *stream_info =
      (mct_stream_info_t *)mod_evt->module_event_data;

    if (af_port->af_initialized) {
      CDBG("%s: Handle ISP output dim event", __func__);
      af_port_handle_isp_output_dim_event(af_port, stream_info);
    } else {
      /* save the stream info locally */
      CDBG("%s: AF Not initialized. Save locally!", __func__);
      af_port->pre_init_updates.is_stream_info_updated = TRUE;
      memcpy(&af_port->pre_init_updates.stream_info, stream_info,
        sizeof(mct_stream_info_t));
    }
  }
    break;

  case MCT_EVENT_MODULE_MODE_CHANGE: {
    /* Stream mode has changed */
    af_port->stream_info.stream_type =
      ((stats_mode_change_event_data*)(mod_evt->module_event_data))->stream_type;
    af_port->reserved_id =
      ((stats_mode_change_event_data*)(mod_evt->module_event_data))->reserved_id;
  }
    break;

  case MCT_EVENT_MODULE_PREVIEW_STREAM_ID: {
    mct_stream_info_t  *stream_info =
          (mct_stream_info_t *)(mod_evt->module_event_data);
    af_port->stream_info.dim.width = stream_info->dim.width;
    af_port->stream_info.dim.height = stream_info->dim.height;
  }
    break;

  case MCT_EVENT_MODULE_FACE_INFO: {
    if (af_port->af_initialized) {
      CDBG("%s: New Face ROI received", __func__);
      af_port_handle_fd_event(af_port, mod_evt);
    }
  }
    break;
  case MCT_EVENT_MODULE_SOF_NOTIFY: {
    if (af_port->af_initialized) {
      if (!af_port_handle_sof_event(port, mod_evt)) {
        CDBG_ERROR("%s Error in posting SoF mesg to AF port", __func__);
      }
    }
  } /*MCT_EVENT_MODULE_SOF_NOTIFY*/
    break;

  case MCT_EVENT_MODULE_ISP_STREAMON_DONE: {
    stats_config_t stats_config;
    mct_event_t event;
    memset(&stats_config, 0, sizeof(stats_config_t));
    CDBG("%s: ISP STREAMON received!", __func__);
    af_port->isp_status.is_isp_ready = TRUE;
    /* Check if we need have pending stats config request to send */
    if (af_port->isp_status.need_to_send_af_config) {
      CDBG("%s: Resend last saved AF config!", __func__);
      stats_config.stats_mask = (1 << MSM_ISP_STATS_BF);
      stats_config.af_config = &af_port->isp_status.config;

      CDBG("%s: Resending AF stats config to ISP! roi_type: %d"
        "roi.left: %d top: %d width: %d height: %d", __func__,
        stats_config.af_config->roi_type, stats_config.af_config->roi.left,
        stats_config.af_config->roi.top, stats_config.af_config->roi.width,
        stats_config.af_config->roi.height);

      /* Pack into an mct_event object */
      event.direction = MCT_EVENT_UPSTREAM;
      event.identity = af_port->reserved_id;
      event.type = MCT_EVENT_MODULE_EVENT;
      event.u.module_event.type =
        MCT_EVENT_MODULE_STATS_CONFIG_UPDATE;
      event.u.module_event.module_event_data =
        (void *)(&stats_config);

      CDBG("%s: send AF_CONFIG data to ISP, port =%p, event =%p",
        __func__, port, &event);
      MCT_PORT_EVENT_FUNC(port)(port, &event);
      af_port->isp_status.need_to_send_af_config = FALSE;
    }
  }
  break;

  case MCT_EVENT_MODULE_STATS_GYRO_STATS: {
    if (af_port->af_initialized) {
      CDBG("%s: Handle gyro data update!", __func__);
      af_port_handle_gyro_update(af_port, mod_evt);
    }
  }
    break;

  case MCT_EVENT_MODULE_IMGLIB_AF_OUTPUT: {
    if (af_port->af_initialized) {
      CDBG("%s: Handle imglib output data update!", __func__);
      af_port_handle_imglib_output(af_port, mod_evt);
    }
  }
    break;

  case MCT_EVENT_MODULE_GET_AF_SW_STATS_FILTER_TYPE: {
    boolean rc = TRUE;
    CDBG("%s: Get AFS filter type", __func__);
    /* Allocate memory to create AF message. we'll post it to AF thread.*/
    q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
      malloc(sizeof(q3a_thread_af_msg_t));
    if (af_msg == NULL) {
      return FALSE;
    }
    memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));

    af_msg->type = MSG_AF_GET;
    af_msg->u.af_get_parm.type = AF_GET_PARAM_SW_STATS_FILTER_TYPE;
    af_msg->sync_flag = TRUE;
    rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
    if (rc) {
      af_sw_filter_type *sw_stats_type =
            (af_sw_filter_type *)mod_evt->module_event_data;
          *sw_stats_type = af_msg->u.af_get_parm.u.af_sw_stats_filter_type;
    }
    free(af_msg);
  }
    break;

  case MCT_EVENT_MODULE_HFR_MODE_NOTIFY: {
    int32_t *hfr_mode = (int32_t *)(mod_evt->module_event_data);
    boolean rc = TRUE;
    CDBG_ERROR("%s: HFR Mode: %d!", __func__, *hfr_mode);

    /* Allocate memory to create AF message. we'll post it to AF thread.*/
    q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
      malloc(sizeof(q3a_thread_af_msg_t));
    if (af_msg == NULL) {
      return FALSE;
    }
    memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));

    af_msg->type = MSG_AF_SET;
    af_msg->u.af_set_parm.type = AF_SET_PARAM_HFR_MODE;
    af_msg->u.af_set_parm.u.hfr_mode = *hfr_mode;
    af_msg->sync_flag = TRUE;
    rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
    free(af_msg);
  }
    break;
  case MCT_EVENT_MODULE_GET_AEC_LUX_INDEX: {
    float *lux_index = (float *)mod_evt->module_event_data;
    *lux_index = af_port->latest_aec_info.lux_idx;
  }
    break;
  case MCT_EVENT_MODULE_ISP_STATS_INFO: {
    CDBG("%s: Handle ISP stats info event!", __func__);
    af_port_handle_isp_stats_info(af_port, mod_evt);
  }
    break;

  default: {
    CDBG("%s: Default. no action!", __func__);
  }
    break;
  }
} /* af_port_handle_module_event */

/** af_port_handle_set_roi_evt:
 *   @af_port:   private AF port structure
 *   @set_parm:  a message to populate
 *   @roi_input: new ROI information
 *
 * Handle set parameter event to set AF ROI.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_set_roi_evt(
  af_port_private_t *af_port,
  af_set_parameter_t *set_parm,
  af_roi_info_t *roi_input) {
  af_roi_info_t temp_roi;
  boolean rc = FALSE;
  CDBG("%s: Set ROI params: frm_id: %d num_roi: %d roi_update: %d", __func__,
    roi_input->frm_id, roi_input->num_roi, roi_input->roi_updated);

  /* Copy roi data into temp before making adjustment for preview to
   * camif conversion. this will avoid overwriting event data. */
  memcpy(&temp_roi, roi_input, sizeof(af_roi_info_t));

  /* set updated flag to false to avoid duplicate roi calls.
   * that comes because of issue in stats_port where it sends
   * multiple event for same event during stream on. */
  if (roi_input->roi_updated == true) {
    roi_input->roi_updated = false;
  } else {
    CDBG("%s: InValid ROI. return!", __func__);
    return FALSE;
  }
  /* For HAL1 we'll need to map the ROI to CAMIF size. For HAL3
     Applicaiton will give ROI in terms of CAMIF size */
#if !USE_HAL_3

  if (roi_input->num_roi > 0) {
    memcpy(&temp_roi.roi[0], &roi_input->roi[0], sizeof(af_roi_t));
    rc = af_port_map_input_roi_to_camif(af_port, &temp_roi.roi[0]);

    if (rc == FALSE) {
      CDBG("%s: Failure mapping ROI. return!", __func__);
      return FALSE;
    }
  }
#endif
  /* Update parameter message to be sent */
  set_parm->type = AF_SET_PARAM_ROI;
  memcpy(&set_parm->u.af_roi_info, &temp_roi, sizeof(af_roi_info_t));

  return TRUE;
} /* af_port_handle_set_roi_evt */

/** af_port_handle_set_focus_mode_evt:
 *   @af_port:  private AF port data
 *   @set_parm: a message to populate
 *   @mode:     focus mode to be set
 *
 * Handle set parameter event to set focus mode.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_set_focus_mode_evt(mct_port_t *port,
  af_set_parameter_t * set_parm, int mode)
{
  boolean rc = TRUE;
  boolean mode_change = FALSE;
  af_port_private_t *af_port =
    (af_port_private_t *)(port->port_private);

  CDBG("%s: Set Focus mode: %d old_mode: %d", __func__, mode, af_port->af_mode);
  if ((cam_focus_mode_type)mode != af_port->af_mode) {
    mode_change = TRUE;
  }

  /* Update parameter message to be sent */
  set_parm->type = AF_SET_PARAM_FOCUS_MODE;

  /* We need to translate Android focus_mode Macro to the one
     AF algorithm understands.*/
  switch (mode) {
  case CAM_FOCUS_MODE_AUTO: {
    set_parm->u.af_mode = AF_MODE_AUTO;
  }
    break;

  case CAM_FOCUS_MODE_INFINITY: {
    set_parm->u.af_mode = AF_MODE_INFINITY;
  }
    break;

  case CAM_FOCUS_MODE_MACRO: {
    set_parm->u.af_mode = AF_MODE_MACRO;
  }
    break;

  case CAM_FOCUS_MODE_CONTINOUS_VIDEO:
  case CAM_FOCUS_MODE_CONTINOUS_PICTURE: {
    set_parm->u.af_mode = AF_MODE_CAF;
  }
    break;
  case CAM_FOCUS_MODE_MANUAL: {
    set_parm->u.af_mode = AF_MODE_MANUAL;
  }
    break;

  case CAM_FOCUS_MODE_FIXED:
  case CAM_FOCUS_MODE_EDOF:
  default: {
    set_parm->u.af_mode = AF_MODE_NOT_SUPPORTED;
    rc = FALSE;
  }
    break;
  }

  if (mode_change) {
    CDBG("%s: Focus mode has changed! Update!", __func__);
    af_port->prev_af_mode = af_port->af_mode;
    af_port->af_mode = mode;
    af_port_update_af_state(port, AF_PORT_TRANS_CAUSE_MODE_CHANGE);
  }
  return rc;
} /* af_port_handle_set_focus_mode_evt */

/** af_port_handle_set_focus_manual_pos_evt:
 *   @af_port:  private AF port data
 *   @set_parm: a message to populate
 *   @pos:      manual focus pos to be set
 *
 * Handle set parameter event to set focus mode.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_set_focus_manual_pos_evt(
  af_port_private_t * af_port,
  af_set_parameter_t * set_parm, af_input_manual_focus_t * manual_pos_info)
{
  boolean rc = TRUE;
  /*All modes except diopter; use ratio for log*/
  if(manual_pos_info->flag != AF_MANUAL_FOCUS_MODE_DIOPTER)
    CDBG("%s: Set manual focus position: %ld Type: %d", __func__,
      manual_pos_info->af_manual_lens_position_ratio, (int)manual_pos_info->flag);
  else /*diopter mode */
    CDBG("%s: Set manual focus diopter position: %f Type: %d", __func__,
      manual_pos_info->af_manual_diopter, (int)manual_pos_info->flag);

  /* Update parameter message to be sent */
  set_parm->type = AF_SET_PARAM_FOCUS_MANUAL_POSITION;
  set_parm->u.af_manual_focus_info = *manual_pos_info;
  return rc;
}


/** af_port_handle_set_metering_mode_evt:
 *    @af_port:  private AF port data
 *    @set_parm: a message to populate
 *    @mode:     metering mode to be set
 *
 * Handle set parameter event to set the metering mode.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_set_metering_mode_evt(af_port_private_t * af_port,
  af_set_parameter_t * set_parm, int mode)
{
  boolean rc = TRUE;

  CDBG("%s: Set Metering mode: %d", __func__, mode);

  /* Update parameter message to be sent */
  set_parm->type = AF_SET_PARAM_METERING_MODE;

  /* We need to translate Android metering mode Macro to the one
   * AF algorithm understands.
   **/
  switch (mode) {
  case CAM_FOCUS_ALGO_AUTO: {
    set_parm->u.af_metering_mode = AF_METER_AUTO;
  }
    break;

  case CAM_FOCUS_ALGO_SPOT: {
    set_parm->u.af_metering_mode = AF_METER_SPOT;
  }
    break;

  case CAM_FOCUS_ALGO_CENTER_WEIGHTED: {
    set_parm->u.af_metering_mode = AF_METER_CTR_WEIGHTED;
  }
    break;

  case CAM_FOCUS_ALGO_AVERAGE: {
    set_parm->u.af_metering_mode = AF_METER_AVERAGE;
  }
    break;

  default: {
    set_parm->u.af_metering_mode = AF_METER_AUTO;
    rc = FALSE;
  }
    break;
  }
  return rc;
} /* af_port_handle_set_metering_mode_evt */

/** af_port_handle_set_bestshot_mode_evt:
 *    @af_port:  AF port data structure
 *    @set_parm: a message to populate
 *    @mode:     scene mode to be set
 *
 * Handle set parameter event to set the bestshot mode.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_set_bestshot_mode_evt(
  af_port_private_t * af_port,
  af_set_parameter_t * set_parm,
  int mode)
{
  boolean rc = TRUE;
  CDBG("%s: Set Bestshot mode: %d", __func__, mode);

  /* Update parameter message to be sent */
  set_parm->type = AF_SET_PARAM_BESTSHOT;

  /* We need to translate Android bestshot mode Macro to the one
   * AF algorithm understands.
   **/
  switch (mode) {
  case CAM_SCENE_MODE_OFF: {
    set_parm->u.af_bestshot_mode = AF_SCENE_MODE_OFF;
  }
    break;

  case CAM_SCENE_MODE_AUTO: {
    set_parm->u.af_bestshot_mode = AF_SCENE_MODE_AUTO;
  }
    break;

  case CAM_SCENE_MODE_LANDSCAPE: {
    set_parm->u.af_bestshot_mode = AF_SCENE_MODE_LANDSCAPE;
  }
    break;

  case CAM_SCENE_MODE_SNOW: {
    set_parm->u.af_bestshot_mode = AF_SCENE_MODE_SNOW;
  }
    break;

  case CAM_SCENE_MODE_BEACH: {
    set_parm->u.af_bestshot_mode = AF_SCENE_MODE_BEACH;
  }
    break;

  case CAM_SCENE_MODE_SUNSET: {
    set_parm->u.af_bestshot_mode = AF_SCENE_MODE_SUNSET;
  }
    break;

  case CAM_SCENE_MODE_NIGHT: {
    set_parm->u.af_bestshot_mode = AF_SCENE_MODE_NIGHT;
  }
    break;

  case CAM_SCENE_MODE_PORTRAIT: {
    set_parm->u.af_bestshot_mode = AF_SCENE_MODE_PORTRAIT;
  }
    break;

  case CAM_SCENE_MODE_BACKLIGHT: {
    set_parm->u.af_bestshot_mode = AF_SCENE_MODE_BACKLIGHT;
  }
    break;

  case CAM_SCENE_MODE_SPORTS: {
    set_parm->u.af_bestshot_mode = AF_SCENE_MODE_SPORTS;
  }
    break;

  case CAM_SCENE_MODE_ANTISHAKE: {
    set_parm->u.af_bestshot_mode = AF_SCENE_MODE_ANTISHAKE;
  }
    break;

  case CAM_SCENE_MODE_FLOWERS: {
    set_parm->u.af_bestshot_mode = AF_SCENE_MODE_FLOWERS;
  }
    break;

  case CAM_SCENE_MODE_CANDLELIGHT: {
    set_parm->u.af_bestshot_mode = AF_SCENE_MODE_CANDLELIGHT;
  }
    break;

  case CAM_SCENE_MODE_FIREWORKS: {
    set_parm->u.af_bestshot_mode = AF_SCENE_MODE_FIREWORKS;
  }
    break;

  case CAM_SCENE_MODE_PARTY: {
    set_parm->u.af_bestshot_mode = AF_SCENE_MODE_PARTY;
  }
    break;

  case CAM_SCENE_MODE_NIGHT_PORTRAIT: {
    set_parm->u.af_bestshot_mode = AF_SCENE_MODE_NIGHT_PORTRAIT;
  }
    break;

  case CAM_SCENE_MODE_THEATRE: {
    set_parm->u.af_bestshot_mode = AF_SCENE_MODE_THEATRE;
  }
    break;

  case CAM_SCENE_MODE_ACTION: {
    set_parm->u.af_bestshot_mode = AF_SCENE_MODE_ACTION;
  }
    break;

  case CAM_SCENE_MODE_AR: {
    set_parm->u.af_bestshot_mode = AF_SCENE_MODE_AR;
  }
    break;

  default: {
    rc = FALSE;
  }
    break;
  }

  return rc;
} /* af_port_handle_set_bestshot_mode_evt */

/** af_port_handle_wait_for_aec_est
 * Tell the AF to wait for the AEC to complete estimation when LED is ON.
 *   @af_port: private AF port data
 *   @set_parm:        a message to populate
 *   @wait:            wait or don't wait
 *
 * Return: TRUE: Success  FALSE: Failure
 **/
static boolean af_port_handle_wait_for_aec_est(
  af_port_private_t * af_port,
  af_set_parameter_t * set_parm,
  boolean wait)
{
  boolean rc = TRUE;

  /* Update parameter message to be sent */
  set_parm->type = AF_SET_PARAM_WAIT_FOR_AEC_EST;
  set_parm->u.af_wait_for_aec_est = wait;

  af_port->af_led_assist = wait;

  return rc;
} /* af_port_handle_wait_for_aec_est */


/** af_port_handle_ez_enable_af_event
 * handle eztuing enable call from application.
 *
 * @af_port: private AF port data
 *
 * @set_parm: a message to populate
 *
 * @enable: enable or unenable
 *
 * Return: TRUE: Success  FALSE: Failure
 **/
static boolean af_port_handle_ez_enable_af_event(
  af_port_private_t * af_port,
  af_set_parameter_t * set_parm,
  boolean enable)
{
  boolean rc = TRUE;

  /* Update parameter message to be sent to alg */
  set_parm->type = AF_SET_PARAM_EZ_ENABLE;
  set_parm->u.af_ez_enable = enable;

  CDBG("%s: enable/unenable (%d) AF!", __func__, enable);
  return rc;
} /* af_port_handle_ez_enable_af_event */

/** af_port_handle_do_af_event
 *    @af_port:  private port data
 *
 * handle autofocus call from application.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_do_af_event(af_port_private_t *af_port)
{
  boolean rc = TRUE;

  CDBG("%s: Handle do_af event !!!", __func__);

  /* Allocate memory to create AF message. we'll post it to AF thread. */
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));
  if (af_msg == NULL) {
    CDBG_ERROR("%s Not enough memory.", __func__);
    return FALSE;
  }
  /* populate af message to post to thread */
  memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
  af_msg->type = MSG_AF_START;
  af_msg->u.af_set_parm.type = AF_SET_PARAM_START;

  /* Enqueue the message to the AF thread */
  rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

  return rc;
} /* af_port_handle_do_af_event */

/** af_port_send_reset_caf_event
 * When LED assisted AF is running in CAF mode, reset internal CAF state machine
 * to make it start from scratch.
 *
 * @af_port: private port number
 *
 * Return: TRUE: Success  FALSE: Failure
 **/
static boolean af_port_send_reset_caf_event(af_port_private_t * af_port)
{
  boolean rc = TRUE;

  CDBG("%s: Send reset cmd to CAF !!!", __func__);

  /* Allocate memory to create AF message. we'll post it to AF thread. */
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));
  if (af_msg == NULL) {
    CDBG_ERROR("%s Not enough memory.", __func__);
    return FALSE;
  }
  /* populate af message to post to thread */
  memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
  af_msg->type = MSG_AF_SET;
  af_msg->u.af_set_parm.type = AF_SET_PARAM_RESET_CAF;

  /* Enqueue the message to the AF thread */
  rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

  return rc;
} /* af_port_handle_reset_caf_event */

/** af_port_handle_cancel_af_event:
 *    @af_port:  private port data
 *
 * handle autofocus cancel call from application.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_cancel_af_event(af_port_private_t *af_port)
{
  boolean rc = TRUE;

  CDBG("%s: Handle cancel AF event!!!", __func__);

  /* Allocate memory to create AF message. we'll post it to AF thread. */
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));
  if (af_msg == NULL) {
    CDBG_ERROR("%s Not enough memory.", __func__);
    return FALSE;
  }
  /* populate af message to post to thread */
  memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
  af_msg->type = MSG_AF_CANCEL;
  af_msg->u.af_set_parm.type = AF_SET_PARAM_CANCEL_FOCUS;

  /* Enqueue the message to the AF thread */
  rc = q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

  return rc;
} /* af_port_handle_cancel_af_event */

/** af_port_handle_set_parm_event
 * Handle AF related set parameter calls from upper layer.
 *
 * Handle AF related set parameter calls from upper layer.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_set_parm_event(mct_port_t *port,
  af_set_parameter_t * parm)
{
  boolean            rc = TRUE;
  boolean            sent = FALSE;
  af_set_parameter_t *set_parm;
  af_port_private_t  *af_port = (af_port_private_t *)(port->port_private);

  if (!parm || !af_port) {
    CDBG_ERROR("%s: Invalid parameters!", __func__);
    return FALSE;
  }

  CDBG("%s: Handle set param event of type: %d", __func__,
    parm->type);

  /* Allocate memory to create AF message. we'll post it to AF thread. */
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));
  if (af_msg == NULL) {
    CDBG_ERROR("%s: Memory allocation failure!", __func__);
    return FALSE;
  }
  /* populate af message to post to thread */
  memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
  af_msg->type = MSG_AF_SET;

  set_parm = &af_msg->u.af_set_parm;

  switch (parm->type) {
  case AF_SET_PARAM_ROI: {
    rc = af_port_handle_set_roi_evt(af_port, set_parm, &parm->u.af_roi_info);
  }
    break;

  case AF_SET_PARAM_FOCUS_MODE: {
    CDBG("%s Set focus mode %d", __func__, (int)parm->u.af_mode);
    rc = af_port_handle_set_focus_mode_evt(port, set_parm,
      (int)parm->u.af_mode);
  }
    break;

  case AF_SET_PARAM_FOCUS_MANUAL_POSITION: {
    rc = af_port_handle_set_focus_manual_pos_evt(af_port, set_parm,
      (af_input_manual_focus_t *)&parm->u.af_manual_focus_info);
  }
    break;

  case AF_SET_PARAM_METERING_MODE: {
    rc = af_port_handle_set_metering_mode_evt(af_port, set_parm,
      (int)parm->u.af_metering_mode);
  }
    break;

  case AF_SET_PARAM_BESTSHOT: {
    rc = af_port_handle_set_bestshot_mode_evt(af_port, set_parm,
      (int)parm->u.af_bestshot_mode);
  }
    break;

  case AF_SET_PARAM_SUPER_EVT: {
    /* Update parameter message to be sent */
    set_parm->type = parm->type;
    set_parm->u.af_set_parm_id = parm->u.af_set_parm_id;
  }
    break;

  case AF_SET_PARAM_META_MODE: {
  /* Update parameter message to be sent */
    set_parm->type = parm->type;
    set_parm->u.af_set_meta_mode = parm->u.af_set_meta_mode;
  }
    break;

  case AF_SET_PARAM_LOCK_CAF: {
    rc = af_port_handle_lock_caf_event(af_port, set_parm,
      (int)parm->u.af_lock_caf);
  }
    break;

  case AF_SET_PARAM_STATS_DEBUG_MASK: {
    set_parm->type = parm->type;
    set_parm->u.stats_debug_mask = parm->u.stats_debug_mask;
  }
    break;

  case AF_SET_PARAM_EZ_ENABLE: {
    rc = af_port_handle_ez_enable_af_event(af_port, set_parm,
      (int)parm->u.af_ez_enable);
  }
    break;

  case AF_SET_PARAM_WAIT_FOR_AEC_EST: {
    /* prioritize the massage in order to not skip stats
     *  after the AEC reports it is ready */
    af_msg->is_priority = TRUE;
    af_port_handle_wait_for_aec_est(af_port, set_parm,
      (int)parm->u.af_wait_for_aec_est);
  }
    break;

  case AF_SET_PARAM_START: { // HAL3
    /* Store trigger id */
    af_port->af_trans.trigger_id = (parm->u.af_trigger_id) ?
      parm->u.af_trigger_id : 0;
    CDBG("%s: TRIGGER_START - Trigger-ID received: %d", __func__,
      af_port->af_trans.trigger_id);
    /* Make do_af call */
    /* If it's CAF mode we don't need to explicitly call auto-focus.
       We just check the current CAF status and based on that make decision. */
    CDBG("%s: Current focus mode: %d, af_led_assist: %d", __func__,
      af_port->af_mode, af_port->af_led_assist);
    if ((af_port->af_mode != CAM_FOCUS_MODE_CONTINOUS_PICTURE) &&
      (af_port->af_mode != CAM_FOCUS_MODE_CONTINOUS_VIDEO)) {
      rc = af_port_handle_do_af_event(af_port);
      if (rc) {
        sent = TRUE;
      }
    } else if (af_port->af_led_assist == TRUE) {
      /** We are in continuous mode, so fool the HAL it is scanning.
       *  It is actually active scan, but we fool the HAL to believe
       *  it is passive and it will wait until focusing is completed
       **/
      af_port->af_trans.af_state = CAM_AF_STATE_PASSIVE_SCAN;
      CDBG("Replace current AF state with CAM_AF_STATE_PASSIVE_SCAN");
      rc = af_port_send_reset_caf_event(af_port);
    }

    /* Update AF transition */
    if (rc) {
      af_port_update_af_state(port, AF_PORT_TRANS_CAUSE_TRIGGER);
    }
  }
    break;

  case AF_SET_PARAM_CANCEL_FOCUS: {// HAL3
    /* Store trigger id */
    af_port->af_trans.trigger_id = (parm->u.af_trigger_id) ?
      parm->u.af_trigger_id : 0;
    CDBG("%s: TRIGGER_CANCEL: Trigger-ID received: %d", __func__,
      af_port->af_trans.trigger_id);
    af_port->af_trigger_called = FALSE;
    //Do not need to do cancel if not locked or scanning
    if (af_port->af_trans.af_state != CAM_AF_STATE_PASSIVE_FOCUSED &&
      af_port->af_trans.af_state != CAM_AF_STATE_PASSIVE_UNFOCUSED &&
      af_port->af_trans.af_state != CAM_AF_STATE_INACTIVE) {
      rc = af_port_handle_cancel_af_event(af_port);
      /* change AF state */
      if (rc) {
        sent = TRUE;
        af_port_update_af_state(port, AF_PORT_TRANS_CAUSE_CANCEL);
      }
    }
  }
    break;

  default: {
    free(af_msg);
    return FALSE;
  }
    break;
  }
  /* Enqueue the message to the AF thread */
  if (sent == FALSE) {
    q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);
  } else {
    free(af_msg);
  }

  return TRUE;
} /* af_port_handle_set_parm_event */

/** af_port_handle_set_allparm_event
 * Handle Q3A related set parameter calls from upper layer.
 *
 * Handle Q3A related set parameter calls from upper layer an translate it
 * to AF set parameter.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_handle_set_allparm_event(af_port_private_t * af_port,
  q3a_all_set_param_type * parm)
{
  boolean            rc = TRUE;
  af_set_parameter_t *set_parm;

  if (!parm || !af_port) {
    CDBG_ERROR("%s: Invalid parameters!", __func__);
    return FALSE;
  }

  CDBG("%s: Handle set param event of type: %d", __func__,
    parm->type);

  /* Allocate memory to create AF message. we'll post it to AF thread. */
  q3a_thread_af_msg_t *af_msg = (q3a_thread_af_msg_t *)
    malloc(sizeof(q3a_thread_af_msg_t));
  if (af_msg == NULL) {
    CDBG_ERROR("%s: Memory allocation failure!", __func__);
    return FALSE;
  }
  /* populate af message to post to thread */
  memset(af_msg, 0, sizeof(q3a_thread_af_msg_t));
  af_msg->type = MSG_AF_SET;

  set_parm = &af_msg->u.af_set_parm;

  switch (parm->type) {
  case Q3A_ALL_SET_EZTUNE_RUNNIG: {
    af_msg->u.af_set_parm.type = AF_SET_PARAM_EZ_TUNE_RUNNING;
    af_msg->u.af_set_parm.u.ez_running = parm->u.ez_runnig;
  }
    break;

  default: {
    free(af_msg);
    return FALSE;
  }
    break;
  }
  /* Enqueue the message to the AF thread */
  q3a_af_thread_en_q_msg(af_port->thread_data, af_msg);

  return TRUE;
} /* af_port_handle_set_allparm_event */

/** af_port_handle_control_event:
 *    @af_port: private AF port data
 *    @mod_evt: module event received
 *
 * Handle control events received at AF port.
 *
 * Return void
 **/
static void af_port_handle_control_event(mct_port_t * port,
  mct_event_control_t * ctrl_evt) {
  af_port_private_t *af_port =
    (af_port_private_t *)(port->port_private);

  if (!af_port->af_initialized) {
    CDBG("%s: AF not supported (yet)!", __func__);
    return;
  }
  CDBG("%s: Handle Control event of type: %d", __func__,
    ctrl_evt->type);
  switch (ctrl_evt->type) {
  case MCT_EVENT_CONTROL_SET_PARM: {
    stats_set_params_type *stat_parm =
      (stats_set_params_type *)ctrl_evt->control_event_data;
    if (stat_parm->param_type == STATS_SET_Q3A_PARAM) {
      q3a_set_params_type  *q3a_param = &(stat_parm->u.q3a_param);
      if (q3a_param->type == Q3A_SET_AF_PARAM) {
        af_port_handle_set_parm_event(port, &q3a_param->u.af_param);
      } else if (q3a_param->type == Q3A_ALL_SET_PARAM) {
        af_port_handle_set_allparm_event(af_port, &q3a_param->u.q3a_all_param);
      }
    }
    /* If it's common params shared by many modules */
    else if (stat_parm->param_type == STATS_SET_COMMON_PARAM) {
      stats_common_set_parameter_t *common_param =
        &(stat_parm->u.common_param);
      af_set_parameter_t af_param;
      if (common_param->type == COMMON_SET_PARAM_BESTSHOT) {
        af_param.type = AF_SET_PARAM_BESTSHOT;
        af_param.u.af_bestshot_mode = common_param->u.bestshot_mode;
        af_port_handle_set_parm_event(port, &af_param);
      } else if (common_param->type == COMMON_SET_PARAM_STATS_DEBUG_MASK) {
        af_param.type = AF_SET_PARAM_STATS_DEBUG_MASK;
        af_param.u.stats_debug_mask = common_param->u.stats_debug_mask;
        af_port_handle_set_parm_event(port, &af_param);
      } else if (common_param->type == COMMON_SET_PARAM_SUPER_EVT) {
        af_param.type = AF_SET_PARAM_SUPER_EVT;
        af_param.u.af_set_parm_id = common_param->u.current_frame_id;
        af_port_handle_set_parm_event(port, &af_param);
      } else if (common_param->type == COMMON_SET_PARAM_META_MODE) {
        af_param.type = AF_SET_PARAM_META_MODE;
        af_param.u.af_set_meta_mode = common_param->u.meta_mode;
        af_port_handle_set_parm_event(port, &af_param);
      } else if (common_param->type == COMMON_SET_CAPTURE_INTENT) {
        af_port_handle_capture_intent(af_port, common_param->u.capture_type);
      } else if (common_param->type == COMMON_SET_CROP_REGION) {
        af_port_handle_set_crop_region_event(af_port,
          &common_param->u.crop_region);
      }
    }
  }
    break;

  case MCT_EVENT_CONTROL_DO_AF: {
    af_port_handle_do_af_event(af_port);
    CDBG("%s: DO AF CMD!", __func__);
  }
    break;

  case MCT_EVENT_CONTROL_CANCEL_AF: {
    af_port_handle_cancel_af_event(af_port);
    CDBG("%s: CANCEL AF CMD!", __func__);
  }
    break;

  case MCT_EVENT_CONTROL_STREAMON: {
    CDBG("%s: STREAMON received!", __func__);
    af_port->reconfigure_ISP_pending = TRUE;
  }
  break;

  case MCT_EVENT_CONTROL_STREAMOFF: {
    CDBG("%s: STREAMOFF received!", __func__);
    /* reset ISP ON info */
    memset(&af_port->isp_status, 0, sizeof(af_isp_up_event_t));
  }
  break;

  default: {
  }
    break;
  }
} /* af_port_handle_control_event */

/** af_port_check_session
 *    @data1: port's existing identity;
 *    @data2: new identity to compare.
 *
 * Check if two session index in the identities are equal
 *
 * Return TRUE if two session index in the identities are equal, FALSE if not.
 **/
static boolean af_port_check_session(void *data1, void *data2)
{
  return (((*(unsigned int *)data1) & 0xFFFF0000) ==
    ((*(unsigned int *)data2) & 0xFFFF0000) ?
    TRUE : FALSE);
}

/** af_port_check_port_availability:
 *    @port:    private AF port data
 *    @session: Pointer to the session ID
 *
 * Check if the AF port is available for the session
 *
 * Return TRUE on success, otherwise FALSE
 **/
boolean af_port_check_port_availability(mct_port_t * port,
  unsigned int *session)
{
  if (port->port_private == NULL) {
    return TRUE;
  }

  if (mct_list_find_custom(MCT_OBJECT_CHILDREN(port), session,
    af_port_check_session) != NULL) {
    return TRUE;
  }

  return FALSE;
}

/** af_port_event:
 *    @port:  port of AF module to handle event
 *    @event: event to be handled
 *
 * af sink module's event processing function. Received events could be:
 * AEC/AWB/AF Bayer stats;
 * Gyro sensor stats;
 * Information request event from other module(s);
 * Information update event from other module(s);
 * It ONLY takes MCT_EVENT_DOWNSTREAM event.
 *
 * Return TRUE if the event is processed successfully, FALSE on failure.
 **/
static boolean af_port_event(mct_port_t * port, mct_event_t * event)
{
  boolean           rc = TRUE;
  af_port_private_t *private;

  CDBG("%s: Received AF port event type: %d, dir: %d", __func__,
    event->type, MCT_EVENT_DIRECTION(event));

  /* sanity check */
  if (!port || !event) {
    CDBG_ERROR("%s: NULL parameter", __func__);
    return FALSE;
  }

  private = (af_port_private_t *)(port->port_private);
  if (!private){
    CDBG_ERROR("%s: NULL private port", __func__);
    return FALSE;
  }

  /* sanity check: ensure event is meant for port with same identity*/
  if ((private->reserved_id & 0xFFFF0000) != (event->identity & 0xFFFF0000)) {
    CDBG("%s:Identity does not match", __func__);
    return FALSE;
  }

  switch (MCT_EVENT_DIRECTION(event)) {
  case MCT_EVENT_DOWNSTREAM: {
    switch (event->type) {
    case MCT_EVENT_MODULE_EVENT: {
      mct_event_module_t *mod_evt =
        (mct_event_module_t *)&(event->u.module_event);
      af_port_handle_module_event(port, mod_evt);
    }
      break;

    case MCT_EVENT_CONTROL_CMD: {
      mct_event_control_t *ctrl_evt =
        (mct_event_control_t *)&(event->u.ctrl_event);
      af_port_handle_control_event(port, ctrl_evt);
    }
      break;

    default:
      break;
    }
  } /* case MCT_EVENT_DOWNSTREAM */
    break;

  case MCT_EVENT_UPSTREAM: {
    CDBG("%s: up event send to peer", __func__);
    mct_port_t *peer = MCT_PORT_PEER(port);
    MCT_PORT_EVENT_FUNC(peer)(peer, event);
    CDBG("%s: up event sent", __func__);
  }
    break;

  default: {
    rc = FALSE;
  }
    break;
  }
  CDBG("%s: X rc: %d", __func__, rc);
  return rc;
}

/** af_port_ext_link:
 *    @identity: session id + stream id
 *    @port:     af module's sink port
 *    @peer:     q3a module's sink port
 *
 * link AF port to Q3A port
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_ext_link(unsigned int identity,  mct_port_t *port,
  mct_port_t *peer)
{
  boolean           rc = FALSE;
  af_port_private_t *private;

  CDBG("%s: Link AF port to q3a", __func__);
  /* af sink port's external peer is always q3a module's sink port */
  if (!port || !peer ||
    strcmp(MCT_OBJECT_NAME(port), "af_sink") ||
    strcmp(MCT_OBJECT_NAME(peer), "q3a_sink")) {
    CDBG("%s: NULL parameters", __func__);
    return FALSE;
  }

  private = (af_port_private_t *)port->port_private;
  if (!private){
    CDBG("%s: private port NULL", __func__);
    return FALSE;
  }

  MCT_OBJECT_LOCK(port);
  switch (private->state) {
  case AF_PORT_STATE_RESERVED:
  /* Fall through, no break */
  case AF_PORT_STATE_UNLINKED:
  /* Fall through, no break */
  case AF_PORT_STATE_LINKED: {
    if ((private->reserved_id & 0xFFFF0000) != (identity & 0xFFFF0000)) {
      CDBG("%s: AF Port Identity does not match!", __func__);
      break;
    }
  }
  /* Fall through, no break */
  case AF_PORT_STATE_CREATED: {
    rc = TRUE;
  }
    break;

  default: {
  }
    break;
  }
  CDBG("%s:%d rc=%d", __func__, __LINE__, rc);
  if (rc == TRUE) {
    private->state = AF_PORT_STATE_LINKED;
    MCT_PORT_PEER(port) = peer;
    MCT_OBJECT_REFCOUNT(port) += 1;
  }
  MCT_OBJECT_UNLOCK(port);

  CDBG("%s: %d: X rc = %d port_state: %d", __func__, __LINE__, rc,
    private->state);
  return rc;
}

/** af_port_ext_unlink:
 *    @identity: session id + stream id
 *    @port:     af module's sink port
 *    @peer:     q3a module's sink port
 *
 * Unlink AF port from Q3A port
 *
 * Return TRUE on success, FALSE on failure.
 **/
static void af_port_ext_unlink(unsigned int identity, mct_port_t *port,
  mct_port_t *peer)
{
  af_port_private_t *private;

  if (!port || !peer || MCT_PORT_PEER(port) != peer) {
    return;
  }

  private = (af_port_private_t *)port->port_private;
  if (!private) {
    return;
  }

  MCT_OBJECT_LOCK(port);
  if (private->state == AF_PORT_STATE_LINKED &&
    (private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000)) {

    MCT_OBJECT_REFCOUNT(port) -= 1;
    if (!MCT_OBJECT_REFCOUNT(port)) {
      private->state = AF_PORT_STATE_UNLINKED;
      MCT_PORT_PEER(port) = NULL;
    }
  }
  MCT_OBJECT_UNLOCK(port);

  return;
}

/** af_port_set_caps
 *    @port: af module's sink port
 *    @caps: pointer to be set to point to the port's capabilities
 *
 * set caps of AF module's sink module
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_set_caps(mct_port_t *port, mct_port_caps_t *caps)
{
  if (strcmp(MCT_PORT_NAME(port), "af_sink")) {
    return FALSE;
  }

  port->caps = *caps;
  return TRUE;
}

/** af_port_check_caps_reserve
 *    @port:        af module's sink port
 *    @caps:        port's capabilities
 *    @stream_info: the stream info
 *
 *  AF sink port can ONLY be re-used by ONE session. If this port
 *  has been in use, AF module has to add an extra port to support
 *  any new session(via module_af_request_new_port).
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_check_caps_reserve(mct_port_t *port, void *caps,
  void *stream_nfo)
{
  boolean           rc = FALSE;
  mct_port_caps_t   *port_caps;
  af_port_private_t *private;
  mct_stream_info_t *stream_info = (mct_stream_info_t *)stream_nfo;

  CDBG("%s: AF port check if reserved!", __func__);
  MCT_OBJECT_LOCK(port);
  if (!port || !caps || !stream_info ||
    strcmp(MCT_OBJECT_NAME(port), "af_sink")) {
    CDBG_ERROR("%s: NULL info", __func__);
    rc = FALSE;
    goto reserve_done;
  }

  port_caps = (mct_port_caps_t *)caps;
  if (port_caps->port_caps_type != MCT_PORT_CAPS_STATS) {
    CDBG_ERROR("%s: Wrong port caps type", __func__);
    rc = FALSE;
    goto reserve_done;
  }

  private = (af_port_private_t *)port->port_private;
  switch (private->state) {
  case AF_PORT_STATE_LINKED: {
    CDBG("%s: AF Port state linked", __func__);
    if ((private->reserved_id & 0xFFFF0000) ==
      (stream_info->identity & 0xFFFF0000))
    rc = TRUE;
  }
    break;

  case AF_PORT_STATE_CREATED:
  case AF_PORT_STATE_UNRESERVED: {
    CDBG("%s: AF Port state created/unreseved", __func__);
    private->reserved_id = stream_info->identity;
    private->stream_info = *stream_info;
    private->state       = AF_PORT_STATE_RESERVED;
    rc = TRUE;
  }
    break;

  case AF_PORT_STATE_RESERVED: {
    CDBG("%s: AF Port state reserved", __func__);
    if ((private->reserved_id & 0xFFFF0000) ==
      (stream_info->identity & 0xFFFF0000))
    rc = TRUE;
  }
    break;

  default: {
    rc = FALSE;
  }
    break;
  }
  CDBG("%s: AF port state: %d", __func__, private->state);
reserve_done:
  MCT_OBJECT_UNLOCK(port);
  return rc;
}

/** af_port_check_caps_unreserve:
 *    @port:     af module's sink port
 *    @identity: session id + stream id
 *
 * Unreserves the AF port.
 *
 * Return TRUE on success, FALSE on failure.
 **/
static boolean af_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  af_port_private_t *private;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "af_sink")) {
    return FALSE;
  }

  private = (af_port_private_t *)port->port_private;
  if (!private) {
    return FALSE;
  }

  MCT_OBJECT_LOCK(port);
  if (((private->state == AF_PORT_STATE_UNLINKED)   ||
    (private->state == AF_PORT_STATE_LINKED) ||
    (private->state == AF_PORT_STATE_RESERVED)) &&
    ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000))) {

    if (!MCT_OBJECT_REFCOUNT(port)) {
      private->state       = AF_PORT_STATE_UNRESERVED;
      private->reserved_id = (private->reserved_id & 0xFFFF0000);
    }
  }
  MCT_OBJECT_UNLOCK(port);

  return TRUE;
}

/** af_port_find_identity:
 *    @port:     af port to be checked
 *    @identity: identity to be compared with
 *
 * Compare af port's session info with identity
 *
 * Return TRUE if equal, FALSE if not.
 **/
boolean af_port_find_identity(mct_port_t *port, unsigned int identity)
{
  af_port_private_t *private;

  if ( !port || strcmp(MCT_OBJECT_NAME(port), "af_sink")) {
    return FALSE;
  }

  private = port->port_private;

  if (private) {
    return ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000) ?
      TRUE : FALSE);
  }

  return FALSE;
}

/** af_port_deinit
 *    @port: af module's sink port to be deinited
 *
 * deinit af module's port
 *
 * Return void
 **/
void af_port_deinit(mct_port_t *port)
{
  af_port_private_t *private;

  if (!port) {
    return;
  }

  if(strcmp(MCT_OBJECT_NAME(port), "af_sink")) {
    return;
  }

  private = (af_port_private_t *)port->port_private;
  if (!private) {
    return;
  }

  if (private->p_sw_stats) {
    free(private->p_sw_stats);
    private->p_sw_stats = NULL;
  }

  AF_DESTROY_LOCK(&private->af_object);
  af_destroy(private->af_object.af);
  free(private);
}

/** af_port_init:
 *    @port:       af's sink port to be initialized
 *    @session_id: session id to be set into af's sink port
 *
 *  af port initialization entry point. Because AF module/port is
 *  pure software object, defer af_port_init when session starts.
 *
 * Return TRUE on success, FALSE on failure.
 **/
boolean af_port_init(mct_port_t *port, unsigned int *session_id)
{
  boolean           rc = TRUE;
  mct_port_caps_t   caps;
  unsigned int      *session;
  mct_list_t        *list;
  af_port_private_t *private;

  CDBG("%s: Init AF port", __func__);

  if (!port || strcmp(MCT_OBJECT_NAME(port), "af_sink")) {
    CDBG("%s: Port name does not match", __func__);
    return FALSE;
  }

  CDBG("%s: Allocate memory to store private port data ", __func__);
  private = (void *)calloc(1, sizeof(af_port_private_t));
  if (!private) {
    CDBG_ERROR("%s: Failure allocating memory for port data ", __func__);
    return FALSE;
  }

  /* initialize AF object */
  AF_INITIALIZE_LOCK(&(private->af_object));
  CDBG("%s: Init AF!", __func__);
  private->af_object.af = af_init(&(private->af_object.af_ops));

  if (private->af_object.af == NULL) {
    CDBG_ERROR("%s: NULL AF object!", __func__);
    free(private);
    return FALSE;
  }

  private->p_sw_stats = (af_sw_stats_t *)malloc(sizeof(af_sw_stats_t));
  private->reserved_id = *session_id;
  private->state       = AF_PORT_STATE_CREATED;
  private->af_initialized = FALSE;
  private->af_mode     = CAM_FOCUS_MODE_MAX;
  private->af_status   = AF_STATUS_INIT;
  port->port_private   = private;
  port->direction      = MCT_PORT_SINK;
  caps.port_caps_type  = MCT_PORT_CAPS_STATS;
  caps.u.stats.flag    = MCT_PORT_CAP_STATS_AF;

  memset(&private->isp_status, 0, sizeof(af_isp_up_event_t));
  memset(&private->af_trans, 0, sizeof(af_port_state_trans_t));
  private->af_trans.af_state = CAM_AF_STATE_INACTIVE;

  af_fdprio_process(&private->fd_prio_data, AF_FDPRIO_CMD_INIT);

  /* this is sink port of af module */
  mct_port_set_event_func(port, af_port_event);
  mct_port_set_ext_link_func(port, af_port_ext_link);
  mct_port_set_unlink_func(port, af_port_ext_unlink);
  mct_port_set_set_caps_func(port, af_port_set_caps);
  mct_port_set_check_caps_reserve_func(port, af_port_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port, af_port_check_caps_unreserve);

  if (port->set_caps) {
    port->set_caps(port, &caps);
  }

  return TRUE;
}
