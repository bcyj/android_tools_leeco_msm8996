/* stats_port.c
  *
 * Copyright (c) 2013 - 2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "modules.h"
#include "stats_module.h"
#include "stats_port.h"
#include "q3a_module.h"
#include "mct_controller.h"

#include "cam_intf.h"
#include "cam_types.h"
#include "aec.h"
#include "awb.h"
#include "af.h"
#include "3AStatsDataTypes.h"
#include <fcntl.h>
#include <stdlib.h>


#if 0
#undef CDBG
#define CDBG CDBG_ERROR
#endif

// MACRO defines for stats skip features
#define ENABLE_STATS_PORT_SKIP_MODE   0     // SET this value to 1 will turn on stats_skip featurei
#ifdef FEATURE_SKIP_STATS
#define ENABLE_STATS_PORT_SKIP_MODE   1    //Enable stats skip for 8x10 target
#endif
#define STATS_PORT_SKIP_STATS_MAX_FPS 30    // if fps exceed will trigger skip stats
#define STATS_PORT_SKIP_STATS_MIN_FPS 20    // if fps below this value won't skip stats
#define Q8                            0x00000100
#define EXIF_DEBUG_MASK_STATS        (0x10000 << 5)
#define CPU_CLOCK_TRESHOLD           (1700000) /* CPU clock rate below which target will be treated as Low-Tier*/

/** stats_port_state_t
 *
 *  This enumeration represents the states of port
 *
 **/
typedef enum {
  STATS_PORT_STATE_CREATED,
  STATS_PORT_STATE_RESERVED,
  STATS_PORT_STATE_LINKED,
  STATS_PORT_STATE_UNLINKED,
  STATS_PORT_STATE_UNRESERVED
} stats_port_state_t;

/** stats_port_reserved_streams
 *    @identity:       the identity of current stream
 *    @stream_type:    the type of stream, SNAPTSHOT, VIDEO, RAW, etc
 *    @streaming_mode: the mode of streaming, BURST, CONTINUOUS, etc
 *    @used_flag:      indicates if the current instance was used or not
 *
 *  the structure used to keep the information of current streams in the session
 **/
typedef struct {
  unsigned int          identity;
  cam_stream_type_t     stream_type;
  cam_streaming_mode_t  streaming_mode;
  boolean               used_flag;
} stats_port_reserved_streams;

/** stats_port_event_t
 *    @cap_flag: indicate the type of capability
 *    @event:    the mct event
 *
 *  the event structure used by stats module internally
 **/
typedef struct {
  unsigned int cap_flag;
  mct_event_t  *event;
} stats_port_event_t;

/** stats_port_setparm_ctrl_t
 *    @has_chromatix_set: indicates if chromatix has been set
 *    @stream_on_counter:  the counter of streams in current session
 *    @evt:               the event
 *    @is_initialised:    indicates if the current instance has been initialized
 *
 *  the parameter structure for ctrl event
 **/
typedef struct {
  boolean             has_chromatix_set;
  int32_t             stream_on_counter;
  stats_port_event_t  evt[CAM_INTF_PARM_MAX];
  boolean             is_initialised[CAM_INTF_PARM_MAX];
} stats_port_setparm_ctrl_t;

/** stats_port_private_t
 *    @reserved_id:          the identity
 *    @stream_type:          the type of current stream
 *    @state:                the state of current stream
 *    @streams_info:         the structure of streams info
 *    @video_stream_cnt:     the counter for video stream
 *    @snapshot_stream_cnt:  the counter for snapshot stream
 *    @preview_stream_cnt:   the counter for preview stream
 *    @sub_ports:            the list of subport
 *    @snap_stream_id:       the id for snapshot stream
 *    @parm_ctrl:            the control parameter
 *    @max_sensor_fps        for skip stats feature, the max fps
 *                           values saved from sensor, need to
 *                           use when calculate current actual
 *                           fps
 *    @skip_stats_mode       for skip stats feature, flag, if
 *                           it's ture, change to skip stats
 *                           mode, false will not skip stats
 *    @current_fps           for skip stats feature, fps value will be
 *                           saved in this variable to decide how many stats
 *                           frame are needed to be skipped
 *                           multiple stats for one frame id
 *    @fake_trigger_id       keep internal trigger_id for HAL1->HAL3 translation
 *    @legacy_hal_cmd        new HAL3 does not use commands, only set params;
 *                           this will tell if we are dealing with a legacy
 *                           commands or not
 *
 *  Structures for port private data.
 *  First 3 members of this structure should be same for all the sub-modules
 *  of stats. Else we might run into wrong stream types
 **/
typedef struct _stats_port_private {
  unsigned int                 reserved_id;
  cam_stream_type_t            stream_type;
  stats_port_state_t           state;
  stats_port_reserved_streams  streams_info[MAX_NUM_STREAMS];
  unsigned int                 video_stream_cnt;
  unsigned int                 snapshot_stream_cnt;
  unsigned int                 preview_stream_cnt;
  mct_list_t                   *sub_ports;
  unsigned int                 snap_stream_id;
  stats_port_setparm_ctrl_t    parm_ctrl;

  float                        max_sensor_fps;
  boolean                      skip_stats_mode;
  int32_t                      current_fps;
  int                          fake_trigger_id;
  boolean                      legacy_hal_cmd;
  mct_stream_info_t            preview_stream_info;
  uint32_t                     stats_debug_mask;
  int32_t                      bg_stats_buffer_size;
  int32_t                      bhist_stats_buffer_size;
  BayerGridStatsType           bg_stats_debug_Data;
  BayerHistogramStatsType      hist_stats_debug_Data;
} stats_port_private_t;


/** stats_port_sub_link_t
 *    @id:   identity of current stream
 *    @peer: the peer module currently connected to
 *
 *  Structure used to link port with external peer
 **/
typedef struct {
  unsigned int  id;
  mct_port_t    *peer;
} stats_port_sub_link_t;

/** stats_port_caps_reserve_t
 *    @caps:        the capability of the port
 *    @stream_info: the information of current stream
 *
 *  Structure used to reserve port's capability
 **/
typedef struct {
  mct_port_caps_t      caps;
  mct_stream_info_t    *stream_info;
} stats_port_caps_reserve_t;



/* Internal function prototypes. */
static void copy_stats_buffer_to_debug_data(
  mct_event_t *event, mct_port_t *port);
static void send_stats_buffer_to_debug_data(mct_port_t *port);
boolean is_stats_buffer_debug_data_enable(mct_port_t *port);


/** stats_port_set_stats_skip_mode
 *    @port:  the port instance
 *    @event: the event to be processed
 *
 *  get the fps value from AEC_UPDATE event and determine if
 *  shall change the stats_skip_mode. We use a MAX_FPS & MIN_FPS
 *  to stablize the skip_mode so that it would not change on
 *  small FPS changes
 *
 *  Return void
 **/
static void stats_port_set_stats_skip_mode(mct_port_t *port,
  mct_event_t *event)
{
  if (!ENABLE_STATS_PORT_SKIP_MODE) {
    return;
  }
  stats_update_t *stats_update = (stats_update_t *)event->u.module_event.module_event_data;
  stats_port_private_t *private = (stats_port_private_t *)port->port_private;

  int32_t current_fps = (int32_t)MIN(1/stats_update->aec_update.exp_time, private->max_sensor_fps);
  private->current_fps = current_fps;

  if (private->skip_stats_mode == TRUE && current_fps < STATS_PORT_SKIP_STATS_MIN_FPS) {
    private->skip_stats_mode = FALSE;
  } else if (private->skip_stats_mode == FALSE && current_fps > STATS_PORT_SKIP_STATS_MAX_FPS) {
    private->skip_stats_mode = TRUE;
  }

  CDBG("%s exp_time %f sensor_fps %f current_fps %d skip_mode %d", __func__,
    stats_update->aec_update.exp_time, private->max_sensor_fps, current_fps,
    private->skip_stats_mode);

  return;
}

/** stats_port_set_stats_skip_mode
 *    @port:  the port instance
 *    @event: the event to be processed
 *
 *  The no of stats frame that need to be ignored will be calculated
 *  depending on the current fps.
 *
 *  Return TRUE if skip current stats, FALSE to not skip
 **/
static boolean stats_port_if_skip_current_stats(mct_port_t *port, mct_event_t *event)
{
  if (!ENABLE_STATS_PORT_SKIP_MODE) {
    return FALSE;
  }

  stats_port_private_t *private = (stats_port_private_t *)port->port_private;
  int32_t current_frame_id = ((mct_event_stats_isp_t *)(event->u.module_event.
                            module_event_data))->frame_id;
  uint8_t index = 0;
  boolean rc = FALSE;

  if (private->skip_stats_mode) {
    index = (private->current_fps / STATS_PORT_SKIP_STATS_MAX_FPS) + 0.5;
    if (current_frame_id % index) {
      rc = TRUE;
    }
    CDBG("%s skip_next_stats %d current_frame_id %d, current_fps: %d",
      __func__, rc, current_frame_id, private->current_fps);
  }
  return rc;
}

/** stats_port_check_id
 *    @data1: port1's id
 *    @data2: port2's id
 *
 *  Check if two ports have same identity.
 *
 *  Return TRUE if the two ports have same id.
 **/
static boolean stats_port_check_id(void *data1, void *data2)
{
  return ((*((unsigned int *)data1) == *((unsigned int *)data2)) ?
    TRUE : FALSE);
}

/** stats_port_handle_enable_meta_channel_event
 *    @port:  the port instance
 *    @event: the event to be processed
 *
 *  Handle meta channel event
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean stats_port_handle_enable_meta_channel_event(mct_port_t *port,
  mct_event_t *event)
{
  boolean              rc = TRUE;
  stats_port_private_t *private = port->port_private;
  mct_event_t          stats_event;
  uint32_t             ch_idx;

  meta_channel_buf_divert_request_t divert_request;

  divert_request.meta_idx_mask =
    *((uint32_t *)event->u.module_event.module_event_data);

  CDBG("%s:%d E", __func__, __LINE__);

  stats_event.direction = MCT_EVENT_UPSTREAM;
  stats_event.identity = event->identity;
  stats_event.type = MCT_EVENT_MODULE_EVENT;
  stats_event.u.module_event.type = MCT_EVENT_MODULE_META_CHANNEL_DIVERT;
  stats_event.u.module_event.module_event_data =
    (void *)(&(divert_request));
  CDBG("%s: Send event to ISP to divert the meta channel!", __func__);
  mct_port_send_event_to_peer(port, &stats_event);

  return rc;
}


/** stats_port_send_event_downstream
 *    @data:      a port object of mct_port_t where the event to send from;
 *    @user_data: object of stats_port_private_t which contains capability
 *                to match and event object.
 *
 *  This function should be called by stats src ports to
 *  redirect any event to its other src ports. eg Redirect Gyro
 *  event to Q3a stats src port. It will also redirect the event
 *  to the event originator.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean stats_port_send_event_downstream(void *data, void *user_data)
{
  mct_port_t         *port = (mct_port_t *)data;
  stats_port_event_t *port_event = (stats_port_event_t *)user_data;
  if(!port || !port_event->event){
    return FALSE;
  }
  /* send to peer with same capability and not to the event originator*/
  if (MCT_PORT_EVENT_FUNC(port)) {
    MCT_PORT_EVENT_FUNC(port)(port, port_event->event);
  }

  return TRUE;
}

/** stats_port_resend_event_downstream
 *    @data:      a port object of mct_port_t where the event to send from;
 *    @user_data: object of stats_port_private_t which contains capability
 *                to match and event object.
 *
 *  Return: Boolean
 *
 *  This function should be called by stats src ports to
 *  redirect any event to its other src ports. eg Redirect Gyro
 *  event to Q3a stats src port. It will ensure it doesn't redirect
 *  the Q3 events back to the Q3A port, because Q3A port handles this
 *  redirection internally.
 **/
static boolean stats_port_redirect_event_downstream(void *data, void *user_data)
{
  mct_port_t         *port = (mct_port_t *)data;
  stats_port_event_t *port_event = (stats_port_event_t *)user_data;

  if((port->caps.u.stats.flag & MCT_PORT_CAP_STATS_Q3A) &&
    ((port_event->cap_flag & MCT_PORT_CAP_STATS_AEC) ||
    (port_event->cap_flag & MCT_PORT_CAP_STATS_AWB) ||
    (port_event->cap_flag & MCT_PORT_CAP_STATS_AF))) {
    /* The event originated from one of the Q3A subports, we don't want to
     * send it to the to the Q3A port again. */
    CDBG("%s: port name: %s, port caps: %d, need %d, caps TYPE: %d,"
      " port_event->cap_flag: %d, origin: %d", __func__, MCT_PORT_NAME(port),
      port->caps.u.stats.flag, MCT_PORT_CAP_STATS_Q3A,
      port->caps.port_caps_type, port_event->cap_flag, port_event->cap_flag);

    return TRUE;
  }

  if (MCT_PORT_EVENT_FUNC(port)) {
    MCT_PORT_EVENT_FUNC(port)(port, port_event->event);
  }

  return TRUE;
}

/** stats_port_check_session_id
 *    @d1: session+stream identity
 *    @d2: session+stream identity
 *
 *  To find out if both identities are matching;
 *
 *  Return TRUE if matches.
 **/
static boolean stats_port_check_session_id(void *d1, void *d2)
{
  unsigned int v1, v2;
  v1 = *((unsigned int *)d1);
  v2 = *((unsigned int *)d2);

  return  ((v1 & 0xFFFF0000) == (v2 & 0xFFFF0000) ?
           TRUE : FALSE);
}

/** stats_port_proc_eztune_set_parm
 *    @cap_flag:         the destination port's cap flag;
 *    @ezetune_cmd_data: the command data for the eztune
 *    @stats_parm:       the parameters to be sent to the Q3A module
 *
 *  Translate the HAL eztune cmd param to Stats set param
 *
 *  Return TRUE on success, FALSE on unknown eztune cmd.
 **/
static boolean stats_port_proc_eztune_set_parm(unsigned int *cap_flag,
  cam_eztune_cmd_data_t *ezetune_cmd_data, stats_set_params_type *stats_parm)
{
  boolean rc = TRUE;

  switch (ezetune_cmd_data->cmd) {
  case CAM_EZTUNE_CMD_STATUS: {
    *cap_flag =
      MCT_PORT_CAP_STATS_AWB | MCT_PORT_CAP_STATS_AEC | MCT_PORT_CAP_STATS_AF;
    stats_parm->param_type = STATS_SET_Q3A_PARAM;
    stats_parm->u.q3a_param.type = Q3A_ALL_SET_PARAM;
    stats_parm->u.q3a_param.u.q3a_all_param.type = Q3A_ALL_SET_EZTUNE_RUNNIG;
    stats_parm->u.q3a_param.u.q3a_all_param.u.ez_runnig =
      ezetune_cmd_data->u.running;
  }
    break;
  case CAM_EZTUNE_CMD_AEC_FORCE_LINECOUNT: {
    aec_ez_force_linecount_t *aec_ez_force_linecount =
      &stats_parm->u.q3a_param.u.aec_param.u.ez_force_linecount;
    *cap_flag = MCT_PORT_CAP_STATS_AEC;
    stats_parm->param_type = STATS_SET_Q3A_PARAM;
    stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
    stats_parm->u.q3a_param.u.aec_param.type =
      AEC_SET_PARAM_EZ_FORCE_LINECOUNT;
    aec_ez_force_linecount_t *force_info = &ezetune_cmd_data->u.ez_force_param;
    aec_ez_force_linecount->forced = force_info->forced;
    aec_ez_force_linecount->force_linecount_value =
      force_info->force_linecount_value;
  }
    break;
  case CAM_EZTUNE_CMD_AEC_FORCE_GAIN: {
    aec_ez_force_gain_t *aec_ez_force_gain =
      &stats_parm->u.q3a_param.u.aec_param.u.ez_force_gain;
    *cap_flag = MCT_PORT_CAP_STATS_AEC;
    stats_parm->param_type = STATS_SET_Q3A_PARAM;
    stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
    stats_parm->u.q3a_param.u.aec_param.type =
      AEC_SET_PARAM_EZ_FORCE_GAIN;
    aec_ez_force_gain_t *force_info = &ezetune_cmd_data->u.ez_force_param;
    aec_ez_force_gain->forced = force_info->forced;
    aec_ez_force_gain->force_gain_value = force_info->force_gain_value;
  }
    break;
  case CAM_EZTUNE_CMD_AEC_FORCE_EXP: {
    aec_ez_force_exp_t *aec_ez_force_exp =
      &stats_parm->u.q3a_param.u.aec_param.u.ez_force_exp;
    *cap_flag = MCT_PORT_CAP_STATS_AEC;
    stats_parm->param_type = STATS_SET_Q3A_PARAM;
    stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
    stats_parm->u.q3a_param.u.aec_param.type = AEC_SET_PARAM_EZ_FORCE_EXP;
    aec_ez_force_exp_t *force_info = &ezetune_cmd_data->u.ez_force_param;
    aec_ez_force_exp->forced = force_info->forced;
    aec_ez_force_exp->force_exp_value = force_info->force_exp_value;
  }
    break;
  case CAM_EZTUNE_CMD_AEC_FORCE_SNAP_LC: {
    aec_ez_force_snap_linecount_t *aec_ez_force_snap_linecount =
      &stats_parm->u.q3a_param.u.aec_param.u.ez_force_snap_linecount;
    *cap_flag = MCT_PORT_CAP_STATS_AEC;
    stats_parm->param_type = STATS_SET_Q3A_PARAM;
    stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
    stats_parm->u.q3a_param.u.aec_param.type =
      AEC_SET_PARAM_EZ_FORCE_SNAP_LINECOUNT;
    aec_ez_force_snap_linecount_t *force_info =
      &ezetune_cmd_data->u.ez_force_param;
    aec_ez_force_snap_linecount->forced = force_info->forced;
    aec_ez_force_snap_linecount->force_snap_linecount_value =
      force_info->force_snap_linecount_value;
  }
    break;
  case CAM_EZTUNE_CMD_AEC_FORCE_SNAP_GAIN: {
    aec_ez_force_snap_gain_t *aec_ez_force_snap_gain =
      &stats_parm->u.q3a_param.u.aec_param.u.ez_force_snap_gain;
    *cap_flag = MCT_PORT_CAP_STATS_AEC;
    stats_parm->param_type = STATS_SET_Q3A_PARAM;
    stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
    stats_parm->u.q3a_param.u.aec_param.type =
      AEC_SET_PARAM_EZ_FORCE_SNAP_GAIN;
    aec_ez_force_snap_gain_t *force_info = &ezetune_cmd_data->u.ez_force_param;
    aec_ez_force_snap_gain->forced = force_info->forced;
    aec_ez_force_snap_gain->force_snap_gain_value =
      force_info->force_snap_gain_value;
  }
    break;
  case CAM_EZTUNE_CMD_AEC_FORCE_SNAP_EXP: {
    aec_ez_force_snap_exp_t *aec_ez_force_snap_exp =
      &stats_parm->u.q3a_param.u.aec_param.u.ez_force_snap_exp;
    *cap_flag = MCT_PORT_CAP_STATS_AEC;
    stats_parm->param_type = STATS_SET_Q3A_PARAM;
    stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
    stats_parm->u.q3a_param.u.aec_param.type =
      AEC_SET_PARAM_EZ_FORCE_SNAP_EXP;
    aec_ez_force_snap_exp_t *force_info = &ezetune_cmd_data->u.ez_force_param;
    aec_ez_force_snap_exp->forced = force_info->forced;
    aec_ez_force_snap_exp->force_snap_exp_value =
      force_info->force_snap_exp_value;
  }
    break;
  case CAM_EZTUNE_CMD_AEC_ENABLE: {
    *cap_flag = MCT_PORT_CAP_STATS_AEC;
    stats_parm->param_type = STATS_SET_Q3A_PARAM;
    stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
    stats_parm->u.q3a_param.u.aec_param.type = AEC_SET_PARAM_ENABLE;
    stats_parm->u.q3a_param.u.aec_param.u.aec_enable =
      ezetune_cmd_data->u.aec_enable;
  }
    break;
  case CAM_EZTUNE_CMD_AWB_MODE: {
    *cap_flag = MCT_PORT_CAP_STATS_AWB;
    stats_parm->param_type = STATS_SET_Q3A_PARAM;
    stats_parm->u.q3a_param.type = Q3A_SET_AWB_PARAM;
    stats_parm->u.q3a_param.u.awb_param.type = AWB_SET_PARAM_WHITE_BALANCE;
    stats_parm->u.q3a_param.u.awb_param.u.awb_current_wb =
      ezetune_cmd_data->u.awb_mode;
  }
    break;
  case CAM_EZTUNE_CMD_AWB_ENABLE: {
    *cap_flag = MCT_PORT_CAP_STATS_AWB;
    stats_parm->param_type = STATS_SET_Q3A_PARAM;
    stats_parm->u.q3a_param.type = Q3A_SET_AWB_PARAM;
    stats_parm->u.q3a_param.u.awb_param.type = AWB_SET_PARAM_ENABLE;
    stats_parm->u.q3a_param.u.awb_param.u.awb_enable =
      ezetune_cmd_data->u.awb_enable;
  }
    break;
  case CAM_EZTUNE_CMD_AF_ENABLE: {
    *cap_flag = MCT_PORT_CAP_STATS_AF;
    stats_parm->param_type = STATS_SET_Q3A_PARAM;
    stats_parm->u.q3a_param.type = Q3A_SET_AF_PARAM;
    stats_parm->u.q3a_param.u.af_param.type = AF_SET_PARAM_EZ_ENABLE;
    stats_parm->u.q3a_param.u.af_param.u.af_ez_enable =
      ezetune_cmd_data->u.af_enable;
  }
    break;

  default: {
    /* Return FALSE if case of unknown or unhandled eztune cmd */
    rc = FALSE;
  }
    break;
  }

  return rc;
}

/** stats_port_filter_set_param
 *    @param: the set parameter to be checked if it needs to be filtered
 *
 *  Filter some set parameters because some HAL1 commands are now translated
 *  into HAL3 set params
 *
 *  Return TRUE if the parameter should not be filtered, FALSE if it should.
 **/
static boolean stats_port_filter_set_param(int param)
{
  cam_intf_parm_type_t cam_param = (cam_intf_parm_type_t)param;
  boolean rc = TRUE;

  switch (cam_param) {
  case CAM_INTF_META_AF_TRIGGER:
  case CAM_INTF_META_AEC_PRECAPTURE_TRIGGER:
  case CAM_INTF_PARM_MANUAL_FOCUS_POS:
    rc = FALSE;
    break;
  default:
    break;
  }

  return rc;
}

#define INFO_PATH "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq"
boolean stats_port_is_low_tier_target(void) {
  boolean retVal = FALSE;
  FILE    *fp;
  char    buf[100];
  size_t  read_bytes;
  int32_t cpuMaxFreq;

  fp = fopen(INFO_PATH, "r");
  if (fp == NULL) {
    CDBG_ERROR("%s: Cannot determine CPU max frequency.", __func__);
    return FALSE;
  }

  read_bytes = fread(buf, 1, 99, fp);
  if (read_bytes <= 99) {
    buf[read_bytes] = 0;
  } else {
    buf[99] = 0;
  }
  fclose(fp);

  cpuMaxFreq = atoi(buf);
  CDBG("%s: max CPU clock: %d, threshold: %d, low tier: %d" , __func__,
    cpuMaxFreq, CPU_CLOCK_TRESHOLD, cpuMaxFreq <= CPU_CLOCK_TRESHOLD);

  if (cpuMaxFreq <= CPU_CLOCK_TRESHOLD) {
    retVal = TRUE;
  }

  return retVal;
}


/** stats_port_proc_set_parm
 *    @port:       a port instance where the event to send from;
 *    @event:      the event to be processed
 *    @sent_done:  a flag to indicate if the event is processed
 *
 *  Translate the HAL set param to Stats set param
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean stats_port_proc_downstream_set_parm(mct_port_t *port,
  mct_event_t *event, boolean *sent_done) {
  boolean rc = TRUE;
  *sent_done = FALSE;

  boolean                  send_internal = FALSE;
  mct_event_control_parm_t *ui_parm =
    (mct_event_control_parm_t *)event->u.ctrl_event.control_event_data;
  stats_port_private_t     *private;
  stats_port_event_t       port_event;

  private = (stats_port_private_t *)(port->port_private);
  stats_port_setparm_ctrl_t *parm_ctrl =
    (stats_port_setparm_ctrl_t *)&private->parm_ctrl;
  if (event->type == MCT_EVENT_CONTROL_CMD &&
    event->u.ctrl_event.type == MCT_EVENT_CONTROL_SET_PARM) {
    mct_event_t new_event;
    port_event.event = &new_event;
    stats_set_params_type *stats_parm = malloc(sizeof(stats_set_params_type));
    q3a_set_params_type *q3a_param;
    if (stats_parm != NULL) {
      new_event.direction = MCT_EVENT_DOWNSTREAM;
      new_event.timestamp = event->timestamp;
      new_event.identity = event->identity;
      new_event.type = event->type;
      new_event.u.ctrl_event.type = event->u.ctrl_event.type;
      new_event.u.ctrl_event.control_event_data = stats_parm;
      *sent_done  = TRUE;
      switch (ui_parm->type) {
      case CAM_INTF_PARM_ZSL_MODE: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AEC;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
        stats_parm->u.q3a_param.u.aec_param.type = AEC_SET_PARAM_ZSL_OP;
        stats_parm->u.q3a_param.u.aec_param.u.zsl_op =
          *((int32_t *)ui_parm->parm_data);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_BRIGHTNESS: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AEC;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
        stats_parm->u.q3a_param.u.aec_param.type = AEC_SET_PARAM_BRIGHTNESS_LVL;
        stats_parm->u.q3a_param.u.aec_param.u.brightness =
          *((int32_t *)ui_parm->parm_data);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_WHITE_BALANCE: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AWB;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AWB_PARAM;
        stats_parm->u.q3a_param.u.awb_param.type = AWB_SET_PARAM_WHITE_BALANCE;
        stats_parm->u.q3a_param.u.awb_param.u.awb_current_wb =
          *((int32_t *)ui_parm->parm_data);
        send_internal = TRUE;
        CDBG("%s wb mode: %d", __func__,
          stats_parm->u.q3a_param.u.awb_param.u.awb_current_wb);
      }
        break;
      case CAM_INTF_PARM_WB_MANUAL: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AWB;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AWB_PARAM;
        stats_parm->u.q3a_param.u.awb_param.type = AWB_SET_PARAM_MANUAL_WB;
        stats_parm->u.q3a_param.u.awb_param.u.manual_wb_params =
          *((manual_wb_parm_t *)ui_parm->parm_data);
        send_internal = TRUE;
        CDBG("%s wb manual mode type: %d", __func__,
          stats_parm->u.q3a_param.u.awb_param.u.manual_wb_params.type);
      }
        break;
      case CAM_INTF_PARM_ISO: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AEC;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
        stats_parm->u.q3a_param.u.aec_param.type = AEC_SET_PARAM_ISO_MODE;
        stats_parm->u.q3a_param.u.aec_param.u.iso =
          *((uint32_t *)ui_parm->parm_data);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_EXPOSURE_TIME: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AEC;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
        stats_parm->u.q3a_param.u.aec_param.type = AEC_SET_PARAM_EXP_TIME;
        stats_parm->u.q3a_param.u.aec_param.u.manual_exposure_time =
          *((uint64_t *)ui_parm->parm_data);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_ANTIBANDING: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AFD;
        stats_parm->param_type = STATS_SET_AFD_PARAM;
        stats_parm->u.afd_param =
          *((cam_antibanding_mode_type *)ui_parm->parm_data);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_FPS_RANGE: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AEC;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
        stats_parm->u.q3a_param.u.aec_param.type = AEC_SET_PARAM_FPS;
        if (CAM_STREAM_TYPE_VIDEO != private->stream_type) {
          stats_parm->u.q3a_param.u.aec_param.u.fps.max_fps =
            ((cam_fps_range_t *)ui_parm->parm_data)->max_fps * 256;
          stats_parm->u.q3a_param.u.aec_param.u.fps.min_fps =
            ((cam_fps_range_t *)ui_parm->parm_data)->min_fps * 256;
        } else {
          stats_parm->u.q3a_param.u.aec_param.u.fps.max_fps =
            ((cam_fps_range_t *)ui_parm->parm_data)->video_max_fps * 256;
          stats_parm->u.q3a_param.u.aec_param.u.fps.min_fps =
            ((cam_fps_range_t *)ui_parm->parm_data)->video_min_fps * 256;
        }
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_EXPOSURE_COMPENSATION: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AEC;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
        stats_parm->u.q3a_param.u.aec_param.type =
          AEC_SET_PARAM_EXP_COMPENSATION;
        stats_parm->u.q3a_param.u.aec_param.u.exp_comp =
          *((int32_t *)ui_parm->parm_data);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_LED_MODE: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AEC;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
        stats_parm->u.q3a_param.u.aec_param.type = AEC_SET_PARAM_LED_MODE;
        stats_parm->u.q3a_param.u.aec_param.u.led_mode =
          *((int32_t *)ui_parm->parm_data);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_AEC_ALGO_TYPE: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AEC;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
        stats_parm->u.q3a_param.u.aec_param.type = AEC_SET_PARAM_METERING_MODE;
        stats_parm->u.q3a_param.u.aec_param.u.aec_metering =
          *((int32_t *)ui_parm->parm_data);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_FOCUS_ALGO_TYPE: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AF;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AF_PARAM;
        stats_parm->u.q3a_param.u.af_param.type = AF_SET_PARAM_METERING_MODE;
        stats_parm->u.q3a_param.u.af_param.u.af_metering_mode =
          *((int32_t *)ui_parm->parm_data);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_AEC_ROI: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AEC;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
        stats_parm->u.q3a_param.u.aec_param.type = AEC_SET_PARAM_ROI;
        cam_set_aec_roi_t roi_info =
          *((cam_set_aec_roi_t *)ui_parm->parm_data);
        stats_parm->u.q3a_param.u.aec_param.u.aec_roi.enable =
          roi_info.aec_roi_enable;
        /* Only send a single touch ROI */
        stats_parm->u.q3a_param.u.aec_param.u.aec_roi.r[0].x =
          roi_info.cam_aec_roi_position.coordinate[0].x;
        stats_parm->u.q3a_param.u.aec_param.u.aec_roi.r[0].y =
          roi_info.cam_aec_roi_position.coordinate[0].y;
        stats_parm->u.q3a_param.u.aec_param.u.aec_roi.r[0].dx = 0;
        stats_parm->u.q3a_param.u.aec_param.u.aec_roi.r[0].dy = 0;
        stats_parm->u.q3a_param.u.aec_param.u.aec_roi.num_regions = 1;
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_AF_ROI: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AF;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AF_PARAM;
        stats_parm->u.q3a_param.u.af_param.type = AF_SET_PARAM_ROI;
        stats_parm->u.q3a_param.u.af_param.current_frame_id =
            event->u.ctrl_event.current_frame_id;
        cam_roi_info_t cam_roi_info =
          *((cam_roi_info_t *)ui_parm->parm_data);
        int i;


        stats_parm->u.q3a_param.u.af_param.u.af_roi_info.roi_updated = TRUE;

        stats_parm->u.q3a_param.u.af_param.u.af_roi_info.num_roi =
            cam_roi_info.num_roi;
        stats_parm->u.q3a_param.u.af_param.u.af_roi_info.type =
            AF_ROI_TYPE_TOUCH;
        stats_parm->u.q3a_param.u.af_param.u.af_roi_info.frm_id =
            cam_roi_info.frm_id;
        for(i = 0; i < cam_roi_info.num_roi; i++) {
          stats_parm->u.q3a_param.u.af_param.u.af_roi_info.roi[i].x =
              cam_roi_info.roi[i].left;
          stats_parm->u.q3a_param.u.af_param.u.af_roi_info.roi[i].y =
              cam_roi_info.roi[i].top;
          stats_parm->u.q3a_param.u.af_param.u.af_roi_info.roi[i].dx =
              cam_roi_info.roi[i].width;
          stats_parm->u.q3a_param.u.af_param.u.af_roi_info.roi[i].dy =
              cam_roi_info.roi[i].height;
          stats_parm->u.q3a_param.u.af_param.u.af_roi_info.weight[i] = 0;
        }
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_FOCUS_MODE: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AF;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AF_PARAM;
        stats_parm->u.q3a_param.u.af_param.type = AF_SET_PARAM_FOCUS_MODE;
        stats_parm->u.q3a_param.u.af_param.u.af_mode =
          *((int32_t *)ui_parm->parm_data);

        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_MANUAL_FOCUS_POS: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AF;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AF_PARAM;
        stats_parm->u.q3a_param.u.af_param.type =
          AF_SET_PARAM_FOCUS_MANUAL_POSITION;
        stats_parm->u.q3a_param.u.af_param.u.af_manual_focus_info =
          *((af_input_manual_focus_t *)ui_parm->parm_data);

        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_BESTSHOT_MODE: {
        port_event.cap_flag = (MCT_PORT_CAP_STATS_AF |
          MCT_PORT_CAP_STATS_AEC |
          MCT_PORT_CAP_STATS_AWB |
          MCT_PORT_CAP_STATS_ASD);
        stats_parm->param_type = STATS_SET_COMMON_PARAM;
        stats_parm->u.common_param.type = COMMON_SET_PARAM_BESTSHOT;
        stats_parm->u.common_param.u.bestshot_mode =
          *((int32_t *)ui_parm->parm_data);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_AEC_LOCK: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AEC;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
        stats_parm->u.q3a_param.u.aec_param.type =  AEC_SET_PARAM_LOCK;
        stats_parm->u.q3a_param.u.aec_param.u.aec_lock =
          *((int32_t *)ui_parm->parm_data);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_AWB_LOCK: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AWB;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AWB_PARAM;
        stats_parm->u.q3a_param.u.awb_param.type = AWB_SET_PARAM_LOCK;
        stats_parm->u.q3a_param.u.awb_param.u.awb_lock =
          *((int32_t *)ui_parm->parm_data);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_EZTUNE_CMD: {
        cam_eztune_cmd_data_t *ezetune_cmd_data =
          (cam_eztune_cmd_data_t *)ui_parm->parm_data;

        stats_port_proc_eztune_set_parm(&port_event.cap_flag, ezetune_cmd_data,
          stats_parm);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_REDEYE_REDUCTION: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AEC;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
        stats_parm->u.q3a_param.u.aec_param.type =
          AEC_SET_PARAM_REDEYE_REDUCTION_MODE;
        stats_parm->u.q3a_param.u.aec_param.u.redeye_mode =
          *((int32_t *)ui_parm->parm_data);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_ASD_ENABLE: {
        /* Do nothing */
      }
        break;
      case CAM_INTF_PARM_DIS_ENABLE: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_IS;
        stats_parm->param_type = STATS_SET_IS_PARAM;
        stats_parm->u.is_param.type = IS_SET_PARAM_IS_ENABLE;
        stats_parm->u.is_param.u.is_enable = *((int32_t *)ui_parm->parm_data);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_HDR: {
        cam_exp_bracketing_t exp;
        port_event.cap_flag = MCT_PORT_CAP_STATS_AEC;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
        stats_parm->u.q3a_param.u.aec_param.type = AEC_SET_PARAM_BRACKET;
        exp = *((cam_exp_bracketing_t *)ui_parm->parm_data);
        if(exp.mode == CAM_EXP_BRACKETING_OFF)
          stats_parm->u.q3a_param.u.aec_param.u.aec_bracket[0] = '\0';
        else
          strlcpy(stats_parm->u.q3a_param.u.aec_param.u.aec_bracket,
            exp.values, MAX_EXP_BRACKETING_LENGTH);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_FLASH_BRACKETING: {
        cam_flash_bracketing_t *p_flashBracket =
          (cam_flash_bracketing_t *)ui_parm->parm_data;
        port_event.cap_flag = MCT_PORT_CAP_STATS_AEC;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
        stats_parm->u.q3a_param.u.aec_param.type =  AEC_SET_PARAM_FLASH_BRACKET;
        stats_parm->u.q3a_param.u.aec_param.u.chromaflash_enable =
          p_flashBracket->enable;
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_SENSOR_HDR:
      case CAM_INTF_PARM_VIDEO_HDR: {
        port_event.cap_flag = (MCT_PORT_CAP_STATS_AEC |
          MCT_PORT_CAP_STATS_AWB);
        stats_parm->param_type = STATS_SET_COMMON_PARAM;
        stats_parm->u.common_param.type = COMMON_SET_PARAM_VIDEO_HDR;
        stats_parm->u.common_param.u.video_hdr =
          *((int32_t *)ui_parm->parm_data);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_STATS_DEBUG_MASK: {
        int32_t data = *((int32_t *)ui_parm->parm_data);
        CDBG("%s STATS_DEBUG_MASK %d", __func__, data);
        port_event.cap_flag = (MCT_PORT_CAP_STATS_AEC |
          MCT_PORT_CAP_STATS_AWB | MCT_PORT_CAP_STATS_AF);

        stats_parm->param_type = STATS_SET_COMMON_PARAM;
        stats_parm->u.common_param.type = COMMON_SET_PARAM_STATS_DEBUG_MASK;
        stats_parm->u.common_param.u.stats_debug_mask =
          *((int32_t *)ui_parm->parm_data);
        send_internal = TRUE;
        private->stats_debug_mask = stats_parm->u.common_param.u.stats_debug_mask;
      }
        break;
      case CAM_INTF_PARM_ALGO_OPTIMIZATIONS_MASK: {
        port_event.cap_flag = (MCT_PORT_CAP_STATS_AEC |
          MCT_PORT_CAP_STATS_AWB);
        stats_parm->param_type = STATS_SET_COMMON_PARAM;
        stats_parm->u.common_param.type = COMMON_SET_PARAM_ALGO_OPTIMIZATIONS_MASK;
        stats_parm->u.common_param.u.algo_opt_mask =
          *((uint32_t *)ui_parm->parm_data);
        if (stats_port_is_low_tier_target() &&
          (!(0x80000000 & stats_parm->u.common_param.u.algo_opt_mask))) {
          /* Dont enable optimizations for low-tier target if msb of mask is set */
          stats_parm->u.common_param.u.algo_opt_mask |= (STATS_MASK_AEC|STATS_MASK_AWB);
          CDBG_ERROR("%s Enable AEC & AWB subsampling optimizations", __func__);
        }
        CDBG("%s Algo opt enable %u", __func__, stats_parm->u.common_param.u.algo_opt_mask);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_META_AF_TRIGGER: { // HAL3
        port_event.cap_flag = MCT_PORT_CAP_STATS_AF;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AF_PARAM;
        stats_parm->u.q3a_param.u.af_param.current_frame_id =
          event->u.ctrl_event.current_frame_id;
        cam_trigger_t af_trigger =
          *((cam_trigger_t *)ui_parm->parm_data);
        stats_parm->u.q3a_param.u.af_param.u.af_trigger_id =
          af_trigger.trigger_id;

        if (private->legacy_hal_cmd == TRUE) {
          /* For AF it does not matter how the command is issued, so just
           * reset the flag for the current HAL command */
          private->legacy_hal_cmd = FALSE;
        }

        send_internal = TRUE;
        if (af_trigger.trigger == CAM_AF_TRIGGER_START) {
          stats_parm->u.q3a_param.u.af_param.type =
            AF_SET_PARAM_START;
          CDBG("%s:%d CAM_AF_TRIGGER_START", __func__, __LINE__);
        } else if (af_trigger.trigger == CAM_AF_TRIGGER_CANCEL) {
          CDBG("%s:%d CAM_AF_TRIGGER_CANCEL", __func__, __LINE__);
          stats_parm->u.q3a_param.u.af_param.type = AF_SET_PARAM_CANCEL_FOCUS;
        } else {
          send_internal = FALSE;
        }
      }
        break;
      case CAM_INTF_META_AEC_PRECAPTURE_TRIGGER: { //HAL3
        port_event.cap_flag = MCT_PORT_CAP_STATS_AEC;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
        if (private->legacy_hal_cmd == TRUE) {
          /* HAL1 to HAL3 style */
          stats_parm->u.q3a_param.u.aec_param.type =
            AEC_SET_PARAM_PREP_FOR_SNAPSHOT_LEGACY;
          /* Reset the flag, we need it only for the current HAL command. */
          private->legacy_hal_cmd = FALSE;
        } else {
          stats_parm->u.q3a_param.u.aec_param.type =
            AEC_SET_PARAM_PREPARE_FOR_SNAPSHOT;
        }
        cam_trigger_t recvd_aec_trigger =
          *((cam_trigger_t *)ui_parm->parm_data);
        stats_parm->u.q3a_param.u.aec_param.u.aec_trigger.trigger =
          recvd_aec_trigger.trigger;
        stats_parm->u.q3a_param.u.aec_param.u.aec_trigger.trigger_id =
          recvd_aec_trigger.trigger_id;
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_META_CAPTURE_INTENT:{
        port_event.cap_flag = (MCT_PORT_CAP_STATS_AF |
          MCT_PORT_CAP_STATS_AEC |
          MCT_PORT_CAP_STATS_AWB |
          MCT_PORT_CAP_STATS_ASD);
        stats_parm->param_type = STATS_SET_COMMON_PARAM;
        stats_parm->u.common_param.type = COMMON_SET_CAPTURE_INTENT;
        stats_parm->u.common_param.u.capture_type =
          *((int32_t *)ui_parm->parm_data);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_META_SCALER_CROP_REGION: {
        port_event.cap_flag = (MCT_PORT_CAP_STATS_AF |
          MCT_PORT_CAP_STATS_AEC |
          MCT_PORT_CAP_STATS_AWB |
          MCT_PORT_CAP_STATS_ASD);
        stats_parm->param_type = STATS_SET_COMMON_PARAM;
        stats_parm->u.common_param.type = COMMON_SET_CROP_REGION;
        stats_parm->u.common_param.u.crop_region =
          *((cam_crop_region_t *)ui_parm->parm_data);
        send_internal = TRUE;
      }
        break;
      case CAM_INTF_PARM_LONGSHOT_ENABLE: {
        port_event.cap_flag = MCT_PORT_CAP_STATS_AEC;
        stats_parm->param_type = STATS_SET_Q3A_PARAM;
        stats_parm->u.q3a_param.type = Q3A_SET_AEC_PARAM;
        stats_parm->u.q3a_param.u.aec_param.type =
          AEC_SET_PARAM_LONGSHOT_MODE;
        stats_parm->u.q3a_param.u.aec_param.u.longshot_mode =
          *((int8_t *)ui_parm->parm_data);
        send_internal = TRUE;
      }
      break;
      default: {
      }
        break;
      }

      if (send_internal) {
        int     idx = ui_parm->type;
        boolean filter_ok;

        /* Filter some set parameters for HAL1 */
        filter_ok = stats_port_filter_set_param(idx);

        MCT_OBJECT_LOCK(port);
        if (!parm_ctrl->is_initialised[idx] && filter_ok) {
          parm_ctrl->evt[idx] = port_event;
          parm_ctrl->evt[idx].event = malloc(sizeof(mct_event_t));
          memcpy(parm_ctrl->evt[idx].event, port_event.event,
            sizeof(mct_event_t));
          parm_ctrl->evt[idx].event->u.ctrl_event.control_event_data =
            (void *)malloc(sizeof(stats_set_params_type));
        }

        if (filter_ok) {
          memcpy((stats_set_params_type *)parm_ctrl->evt[idx].event->
            u.ctrl_event.control_event_data,
            stats_parm, sizeof(stats_set_params_type));
          parm_ctrl->is_initialised[idx] = TRUE;
        }

        if (parm_ctrl->has_chromatix_set) {
          rc = mct_list_traverse((mct_list_t *)private->sub_ports,
            stats_port_send_event_downstream, &port_event);
        }
        MCT_OBJECT_UNLOCK(port);

      }
      free(stats_parm);
    }
  }
  return rc;
}

/** stats_port_transform_af_cmd_to_set_parm
 *    @port:       a port instance where the event to send from;
 *    @event:      the event to be processed
 *    @sent_done:  a flag to indicate if the event is processed
 *
 *  Translate the HAL set param to Stats set param
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean stats_port_transform_af_cmd_to_set_parm(mct_port_t *port,
  mct_event_t *event, boolean *sent_done)
{
  boolean                  rc = TRUE;
  mct_event_control_parm_t fake_ui_parm;
  cam_trigger_t            fake_trigger;
  stats_port_private_t     *private;

  private = (stats_port_private_t *)(port->port_private);
  stats_port_setparm_ctrl_t *parm_ctrl =
    (stats_port_setparm_ctrl_t *)&private->parm_ctrl;

  if (event->u.ctrl_event.type == MCT_EVENT_CONTROL_DO_AF) {
    fake_trigger.trigger = CAM_AF_TRIGGER_START;
    CDBG("%s: Do_AF call -> CAM_AF_TRIGGER_START", __func__);
  } else if (event->u.ctrl_event.type == MCT_EVENT_CONTROL_CANCEL_AF) {
    fake_trigger.trigger = CAM_AF_TRIGGER_CANCEL;
    CDBG("%s: Cancel_AF call -> CAM_AF_TRIGGER_CANCEL", __func__);
  } else {
    *sent_done = FALSE;
    return rc;
  }

  fake_trigger.trigger_id = ++private->fake_trigger_id;
  fake_ui_parm.type = CAM_INTF_META_AF_TRIGGER;
  fake_ui_parm.parm_data = &fake_trigger;

  /* change the type of the event */
  event->u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  /* Set the event data pointer */
  event->u.ctrl_event.control_event_data = &fake_ui_parm;
  /* Set the flag to distinguish the way we should handle the set parameter */
  private->legacy_hal_cmd = TRUE;
  /* Send the faked set_param event downstream */
  CDBG("%s: Will send Do_AF call -> CAM_AF_TRIGGER_START", __func__);
  rc = stats_port_proc_downstream_set_parm(port, event, sent_done);

  return rc;
}

/** stats_port_transform_prepsnap_cmd_to_set_parm
 *    @port:       a port instance where the event to send from;
 *    @event:      the event to be processed
 *    @sent_done:  a flag to indicate if the event is processed
 *
 *  Translate the HAL prepare snapshot cmd param to Stats set param
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean stats_port_transform_prepsnap_cmd_to_set_parm(mct_port_t *port,
  mct_event_t *event, boolean *sent_done)
{
  boolean                  rc = TRUE;
  mct_event_control_parm_t fake_ui_parm;
  cam_trigger_t            fake_trigger;
  stats_port_private_t     *private;

  private = (stats_port_private_t *)(port->port_private);
  stats_port_setparm_ctrl_t *parm_ctrl =
    (stats_port_setparm_ctrl_t *)&private->parm_ctrl;

  fake_ui_parm.parm_data = &fake_trigger;

  /* change the type of the event */
  event->u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  /* Set the event data pointer */
  event->u.ctrl_event.control_event_data = &fake_ui_parm;

  /* Now send a precapture trigger */
  fake_ui_parm.type = CAM_INTF_META_AEC_PRECAPTURE_TRIGGER;
  fake_trigger.trigger = CAM_AEC_TRIGGER_START;
  fake_trigger.trigger_id = ++private->fake_trigger_id;

  /* Send the faked set_param event downstream */
  CDBG("%s Transform prepare snapshot into precapture trigger", __func__);
  /* Set the flag to distinguish the way we should handle the set parameter */
  private->legacy_hal_cmd = TRUE;
  rc = stats_port_proc_downstream_set_parm(port, event, sent_done);

  return rc;
}

//MCT_EVENT_CONTROL_PREPARE_SNAPSHOT

/** stats_port_event
 *    @port:  this port from where the event should go
 *    @event: event object to send upstream or downstream
 *
 *  Because stats interface module works no more than a event pass through
 *  module, hence its downstream event handling should be fairly
 *  straight-forward, but upstream event will need a little
 *  bit processing.
 *
 *  Return TRUE for successful event processing.
 **/
static boolean stats_port_event(mct_port_t *port, mct_event_t *event)
{
  boolean              rc = TRUE;
  stats_port_private_t *private;
  stats_port_event_t   port_event;

  CDBG("%s: port=%p, event=%p", __func__, port, event);
  /* sanity check */
  if (!port || !event) {
    return FALSE;
  }

  private = (stats_port_private_t *)(port->port_private);
  if (!private) {
    CDBG_ERROR("%s: Private port is NULL", __func__);
    return FALSE;
  }

  /* sanity check: ensure event is meant for port with same identity*/

  if (mct_list_find_custom(MCT_OBJECT_CHILDREN(port), &(event->identity),
    stats_port_check_session_id) == NULL) {

    CDBG_ERROR("%s: sanity fail event id=%d", __func__, event->identity);
    return FALSE;
  }

  /* Save the event for later use */
  port_event.event = event;

  switch (MCT_EVENT_DIRECTION(event)) {
  case MCT_EVENT_UPSTREAM: {
    /* The upstream events could come from Q3A, DIS, AFD or sensor
     * module.
     *
     * Need to check event and see if it has to redirect the event
     * to downstream, for example GYRO event needs to redirect to
     * Q3A and DIS modules */
    mct_event_direction redirect;
    mct_list_t *list;

    /* Currently we are just sending module event, instead of going
       one level down to stats event */

    /* check to see if need to redirect this event to sub-modules */
    CDBG("%s: Received event type=%d", __func__, event->u.module_event.type);
    switch (event->u.module_event.type) {
    case MCT_EVENT_MODULE_IMGLIB_AF_CONFIG: {
        event->identity = private->preview_stream_info.identity;
        redirect = MCT_EVENT_UPSTREAM;
        CDBG("%s: Received IMGLIB_AF_CONFIG, identity=0x%x", __func__, event->identity);
      }
      break;
    case MCT_EVENT_MODULE_STATS_GYRO_STATS: {
      /* GRYO STATS should be redirected downstream to both Q3A
       * and EIS modules */
      port_event.cap_flag = MCT_PORT_CAP_STATS_GYRO;
      redirect = MCT_EVENT_DOWNSTREAM;
    }
      break;

    case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {
      port_event.cap_flag = MCT_PORT_CAP_STATS_AEC;
      if (private->parm_ctrl.stream_on_counter > 0) {
        /* AEC UPDATE should be redirected downstream to ASD module and
         * also sent to upsteam */
        redirect = MCT_EVENT_BOTH;
      } else {
        // don't send stats to UPSTREAM if all of the streams were off
        CDBG_ERROR("%s stop sending AEC_UPDATE", __func__);
        redirect = MCT_EVENT_DOWNSTREAM;
      }
      stats_port_set_stats_skip_mode(port, event);
    }
      break;

    case MCT_EVENT_MODULE_STATS_AWB_UPDATE: {
      port_event.cap_flag = MCT_PORT_CAP_STATS_AWB;
      if (private->parm_ctrl.stream_on_counter > 0) {
        /* AWB UPDATE should be redirected downstream to ASD module and
         * also sent to upsteam */
        redirect = MCT_EVENT_BOTH;
      } else {
        // don't send stats to UPSTREAM if all of the streams were off
        CDBG_ERROR("%s stop sending AWB_UPDATE", __func__);
        redirect = MCT_EVENT_DOWNSTREAM;
      }
    }
      break;
    case MCT_EVENT_MODULE_STATS_AF_UPDATE: {
      port_event.cap_flag = MCT_PORT_CAP_STATS_AF;
      /* AWB UPDATE should be redirected downstream to ASD module and
       * also sent to upsteam */
      redirect = MCT_EVENT_BOTH;
    }
      break;

    case MCT_EVENT_MODULE_STATS_ASD_UPDATE: {
      /* ASD event should be redirected downstream to Q3A module and
       * also sent to upstream */
      port_event.cap_flag = MCT_PORT_CAP_STATS_ASD;
      redirect = MCT_EVENT_BOTH;
    }
      break;

    case MCT_EVENT_MODULE_STATS_AFD_UPDATE: {
      /* AFD event should be redirected downstream to Q3A module and
       * used by AEC */
      port_event.cap_flag = MCT_PORT_CAP_STATS_AFD;
      redirect = MCT_EVENT_DOWNSTREAM;
    }
      break;

    case MCT_EVENT_MODULE_STATS_POST_TO_BUS: {
      mct_module_t *module =
        MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);

      if (!module) {
        CDBG_ERROR("%s: Failure getting module info!", __func__);
      } else {
        CDBG("%s: Posting message to bus for module: %s", __func__,
          module->object.name);
        /* These are bus messages that need to be post to bus. No need
           to forward this message upstream or downstream */
        if (TRUE != mct_module_post_bus_msg(module,
          (mct_bus_msg_t *)event->u.module_event.module_event_data)) {
          CDBG("%s: Failure posting to the bus!", __func__);
        }
      }
      redirect = MCT_EVENT_NONE;
    }
      break;

    case MCT_EVENT_MODULE_GET_GYRO_DATA: {
      port_event.cap_flag = 0;
      redirect = MCT_EVENT_DOWNSTREAM;
    }
      break;

    default: {
      redirect = MCT_EVENT_UPSTREAM;
    }
      break;
    }

    /* always forward the event to upstream(sink port) first */
    if (redirect == MCT_EVENT_UPSTREAM || redirect == MCT_EVENT_BOTH) {
      rc = mct_port_send_event_to_peer(port, event);
    }

    if (redirect == MCT_EVENT_DOWNSTREAM || redirect == MCT_EVENT_BOTH) {
      /* redirect the event to sub-modules' ports */
      event->direction = MCT_EVENT_DOWNSTREAM;

      rc = mct_list_traverse((mct_list_t *)private->sub_ports,
        stats_port_redirect_event_downstream, &port_event);
    }
  } /* case MCT_EVENT_TYPE_UPSTREAM */
    break;

  case MCT_EVENT_DOWNSTREAM: {
    /* In case of sink port, no need to peek into the event,
     * instead just simply forward the event and let downstream
     * modules process them and take action accordingly */
    boolean sent = 0;
    if (event->type == MCT_EVENT_CONTROL_CMD &&
      event->u.ctrl_event.type == MCT_EVENT_CONTROL_SET_PARM) {
      rc = stats_port_proc_downstream_set_parm(port, event, &sent);
    } else if (event->type == MCT_EVENT_CONTROL_CMD &&
      (event->u.ctrl_event.type == MCT_EVENT_CONTROL_DO_AF ||
      event->u.ctrl_event.type == MCT_EVENT_CONTROL_CANCEL_AF)) {
      CDBG("%s: Sending do_AF/cancel_AF call", __func__);
      rc = stats_port_transform_af_cmd_to_set_parm(port, event, &sent);
    } else if (event->type == MCT_EVENT_CONTROL_CMD &&
      event->u.ctrl_event.type == MCT_EVENT_CONTROL_PREPARE_SNAPSHOT) {
      CDBG("%s: Sending Prep snapshot call", __func__);
      rc = stats_port_transform_prepsnap_cmd_to_set_parm(port, event, &sent);
    } else if (event->type == MCT_EVENT_MODULE_EVENT &&
      ((event->u.module_event.type == MCT_EVENT_MODULE_SET_CHROMATIX_PTR) ||
      (event->u.module_event.type == MCT_EVENT_MODULE_SET_RELOAD_CHROMATIX) ||
      (event->u.module_event.type == MCT_EVENT_MODULE_SET_AF_TUNE_PTR) ||
      (event->u.module_event.type == MCT_EVENT_MODULE_SET_RELOAD_AFTUNE))){

      int idx;
      stats_port_setparm_ctrl_t *parm_ctrl =
        (stats_port_setparm_ctrl_t *)&private->parm_ctrl;

      if (CAM_STREAM_TYPE_RAW != private->stream_type) {
        rc = mct_list_traverse((mct_list_t *)private->sub_ports,
          stats_port_send_event_downstream, &port_event);
      }

      MCT_OBJECT_LOCK(port);
      for (idx = 0; idx < CAM_INTF_PARM_MAX; idx++) {
        if (parm_ctrl->is_initialised[idx] == TRUE) {
          rc = mct_list_traverse((mct_list_t *)private->sub_ports,
            stats_port_send_event_downstream, &parm_ctrl->evt[idx]);
        }
      } /* end of for */
      parm_ctrl->has_chromatix_set = TRUE;
      MCT_OBJECT_UNLOCK(port);

      sent = TRUE;
    } else if (event->type == MCT_EVENT_MODULE_EVENT &&
      event->u.module_event.type == MCT_EVENT_MODULE_SENSOR_META_CONFIG) {
      rc = stats_port_handle_enable_meta_channel_event(port, event);
      sent = TRUE;
    } else if (event->type == MCT_EVENT_CONTROL_CMD &&
      event->u.ctrl_event.type == MCT_EVENT_CONTROL_STREAMOFF) {
      stats_port_setparm_ctrl_t *parm_ctrl =
        (stats_port_setparm_ctrl_t *)&private->parm_ctrl;
      CDBG("%s: STREAMOFF event received!", __func__);
      if (!(MCT_OBJECT_REFCOUNT(port) == 3 &&
        private->snap_stream_id == (event->identity & 0x0000FFFF))) {
        MCT_OBJECT_LOCK(port);
      }
      parm_ctrl->stream_on_counter--;
      if (0 == parm_ctrl->stream_on_counter) {
        parm_ctrl->has_chromatix_set = FALSE;
      }
      MCT_OBJECT_UNLOCK(port);
    } else if (event->type == MCT_EVENT_CONTROL_CMD &&
        event->u.ctrl_event.type == MCT_EVENT_CONTROL_STREAMON) {
      stats_port_setparm_ctrl_t *parm_ctrl =
        (stats_port_setparm_ctrl_t *)&private->parm_ctrl;
      CDBG("%s: STREAMON event received!", __func__);

      MCT_OBJECT_LOCK(port);
      parm_ctrl->stream_on_counter++;
      MCT_OBJECT_UNLOCK(port);
    } else if (event->type == MCT_EVENT_MODULE_EVENT &&
        event->u.module_event.type == MCT_EVENT_MODULE_SET_STREAM_CONFIG) {

       // save the sensor fps, used in skip stats feature
       sensor_out_info_t *sensor_info = (sensor_out_info_t *)(event->u.module_event.module_event_data);
       private->max_sensor_fps = sensor_info->max_fps;
       CDBG("%s max_sensor_fps %f", __func__, sensor_info->max_fps);
    } else if (event->type == MCT_EVENT_MODULE_EVENT &&
        event->u.module_event.type == MCT_EVENT_MODULE_STATS_DATA){

      if (is_stats_buffer_debug_data_enable(port)) {

        /* Copy stats buffers to debug data structures. */
        copy_stats_buffer_to_debug_data(event, port);
      } else {
        CDBG("debug data not enabled");
      }
      // if turns out we shall skip the current stats, then just set the sent flag to true, pretend it has already sent to the sub-modules
      CDBG("%s frame id %d", __func__, ((mct_event_stats_isp_t *)(event->u.module_event.module_event_data))->frame_id);
      if (stats_port_if_skip_current_stats(port, event)) {
        CDBG("%s skip current stats", __func__);
        sent = TRUE;
      }
    } else if (event->type == MCT_EVENT_MODULE_EVENT &&
      event->u.module_event.type == MCT_EVENT_MODULE_SOF_NOTIFY){
      if (is_stats_buffer_debug_data_enable(port)) {

        /* send stats buffers to exif debug data */
        send_stats_buffer_to_debug_data(port);
      } else {
        CDBG("debug data not enabled");
      }
    }
    CDBG("%s: down event:", __func__);
    if (!sent) {
      /* We don't want to handle these events for the SNAPSHOT stream */
      if (event->type == MCT_EVENT_MODULE_EVENT &&
        ((event->u.module_event.type == MCT_EVENT_MODULE_STREAM_CROP) ||
        (event->u.module_event.type == MCT_EVENT_MODULE_ISP_OUTPUT_DIM))) {
        int i;
        boolean case_break = FALSE;

        for (i = 0; i < MAX_NUM_STREAMS; i++) {
          if (private->streams_info[i].used_flag == TRUE &&
            private->streams_info[i].identity == event->identity &&
            (private->streams_info[i].stream_type == CAM_STREAM_TYPE_SNAPSHOT ||
             private->streams_info[i].stream_type == CAM_STREAM_TYPE_RAW)) {
            case_break = TRUE;
            /* prevent reporting an error to the caller */
            rc = TRUE;
            break;
          }
        }
        if (case_break) {
          CDBG("%s: SNAPSHOT stream event: %d - DISCARD", __func__,
            event->u.module_event.type);
          break;
        }
      }
      rc = mct_list_traverse((mct_list_t *)private->sub_ports,
        stats_port_send_event_downstream, &port_event);

    }
  } /* case MCT_EVENT_TYPE_DOWNSTREAM */
    break;

  default: {
    CDBG_ERROR("%s: Unknown event", __func__);
    rc = FALSE;
  }
    break;
  }

  return rc;
}

static boolean stats_port_start_stop_stats_thread(
mct_port_t *port, uint8_t start_flag)
{
  boolean                rc = TRUE;
  stats_port_private_t   *private = (stats_port_private_t *)port->port_private;
  stats_port_event_t     port_event;
  mct_event_t            event;

  /* This event handling in each submodule should be a blocking call */
  CDBG("%s: start_flag: %d", __func__, start_flag);
  port_event.event = &event;
  port_event.cap_flag = (MCT_PORT_CAP_STATS_AF |
    MCT_PORT_CAP_STATS_AEC |
    MCT_PORT_CAP_STATS_AWB |
    MCT_PORT_CAP_STATS_IS  |
    MCT_PORT_CAP_STATS_AFD |
    MCT_PORT_CAP_STATS_ASD);
  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = private->reserved_id;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_START_STOP_STATS_THREADS;
  event.u.module_event.module_event_data = (void *)(&start_flag);
  rc = mct_list_traverse((mct_list_t *)private->sub_ports,
     stats_port_send_event_downstream, &port_event);

 return rc;
}


/** stats_port_set_caps
 *    @port: port object which the caps to be set
 *    @caps: this port's capability
 *
 *  Function overwrites a ports capability.
 *
 *  Return TRUE if it is valid source port.
 **/
static boolean stats_port_set_caps(mct_port_t *port, mct_port_caps_t *caps)
{
  if (strcmp(MCT_PORT_NAME(port), "stats_sink")) {
    return FALSE;
  }

  port->caps = *caps;
  return TRUE;
}

/** stats_port_sub_caps_reserve
 *    @data:      mct_port_t object
 *    @user_data: stats_port_caps_reserve_t object
 *
 *  To reserve port's capability
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean stats_port_sub_caps_reserve(void *data, void *user_data)
{
  boolean                   rc = FALSE;
  mct_port_t                *port = (mct_port_t *)data;
  stats_port_caps_reserve_t *reserve = (stats_port_caps_reserve_t *)user_data;

  if (port->check_caps_reserve) {
    rc = port->check_caps_reserve(port, &reserve->caps, reserve->stream_info);
  }

  return rc;
}

/** stats_port_sub_unreserve
 *    @data:      mct_port_t object
 *    @user_data: identity
 *
 *  To unreserve port's capability
 *
 *  Return TRUE.
 **/
static boolean stats_port_sub_unreserve(void *data, void *user_data)
{
  mct_port_t *port = (mct_port_t *)data;
  unsigned int id = *((unsigned int *)user_data);

  if (port->check_caps_unreserve) {
    port->check_caps_unreserve(port, id);
  }

  return TRUE;
}

/** stats_port_add_reserved_stream
 *    @private:     Private info of the stats port.
 *    @stream_info: Stream info of the reserving stream.
 *
 * This function adds the stream to the stream list whenever a new stream
 * is reserved.
 *
 * Return FALSE if adding the stream fails.
 **/

static boolean stats_port_add_reserved_stream(stats_port_private_t *private,
  mct_stream_info_t *stream_info)
{
  boolean rc = FALSE;
  int     i = 0;

  CDBG("%s, Adding stream to reserved list, identity=%d", __func__,
    stream_info->identity);

  for (i = 0; i < MAX_NUM_STREAMS; i++) {
    if ((private->streams_info[i].used_flag == TRUE) &&
      (private->streams_info[i].identity == stream_info->identity)) {
      CDBG_ERROR("%s, Stream with identity=%d already reserved", __func__,
        stream_info->identity);
      rc = TRUE;
      return rc;
    }
  }

  for (i = 0; i < MAX_NUM_STREAMS; i++) {
    if (private->streams_info[i].used_flag == FALSE) {
      private->streams_info[i].identity = stream_info->identity;
      private->streams_info[i].stream_type = stream_info->stream_type;
      private->streams_info[i].streaming_mode = stream_info->streaming_mode;
      private->streams_info[i].used_flag = TRUE;

      if (stream_info->stream_type == CAM_STREAM_TYPE_VIDEO) {
        private->video_stream_cnt++;
        private->stream_type = CAM_STREAM_TYPE_VIDEO;
        private->reserved_id = stream_info->identity;
      } else if (stream_info->stream_type == CAM_STREAM_TYPE_PREVIEW) {
        private->preview_stream_cnt++;
        if (!private->video_stream_cnt) {
          private->stream_type = CAM_STREAM_TYPE_PREVIEW;
          private->reserved_id = stream_info->identity;
        }
      } else if (stream_info->stream_type == CAM_STREAM_TYPE_SNAPSHOT ||
                 stream_info->stream_type == CAM_STREAM_TYPE_RAW) {
        private->snapshot_stream_cnt++;
        if (!private->video_stream_cnt && (!private->preview_stream_cnt ||
          (private->preview_stream_cnt && stream_info->streaming_mode))) {
          private->stream_type = CAM_STREAM_TYPE_SNAPSHOT;
          private->reserved_id = stream_info->identity;
        }
      } else {
        CDBG_ERROR("%s, Invalid stream type=%d", __func__,
          stream_info->stream_type);
        rc = FALSE;
        break;
      }
      rc = TRUE;
      break;
    }
  }

  if (!rc) {
    CDBG_ERROR("%s, Adding stream with identity=%d, to reserved list failed",
      __func__, stream_info->identity);
  }

  return rc;
}

/** stats_port_delete_reserved_stream
 *    @private: Private info of the stats port.
 *    @identity: session+stream identity.
 *
 * This function deletes the stream from the stream list whenever the stream
 * is unreserved.
 *
 * Return FALSE if deleting the stream fails.
 **/
static boolean stats_port_delete_reserved_stream(stats_port_private_t *private,
  unsigned int identity)
{
  boolean rc = FALSE;
  int     i = 0;
  int     j = 0;

  CDBG("%s, Deleting stream from reserved list, identity=%d", __func__,
    identity);

  for (i = 0; i < MAX_NUM_STREAMS; i++) {
    if (identity == private->streams_info[i].identity) {
      mct_list_traverse(private->sub_ports, stats_port_sub_unreserve,
        &identity);
      private->streams_info[i].identity  = 0;
      private->streams_info[i].used_flag = FALSE;

      if (private->streams_info[i].stream_type == CAM_STREAM_TYPE_VIDEO) {
        private->video_stream_cnt--;
      } else if (private->streams_info[i].stream_type ==
        CAM_STREAM_TYPE_SNAPSHOT) {
        private->snapshot_stream_cnt--;
      } else if (private->streams_info[i].stream_type ==
        CAM_STREAM_TYPE_PREVIEW) {
        private->preview_stream_cnt--;
      }

      private->streams_info[i].stream_type = CAM_STREAM_TYPE_MAX;
      private->streams_info[i].streaming_mode = 0;

      if (private->video_stream_cnt) {
        for (j = MAX_NUM_STREAMS - 1; j >= 0; j--) {
          if (private->streams_info[j].used_flag &&
            private->streams_info[j].stream_type == CAM_STREAM_TYPE_VIDEO) {

            private->stream_type = CAM_STREAM_TYPE_VIDEO;
            private->reserved_id = private->streams_info[j].identity;

            rc = TRUE;
            return rc;
          }
        }
      }
      if (private->snapshot_stream_cnt) {
        for (j = MAX_NUM_STREAMS - 1; j >= 0; j--) {
          if (private->streams_info[j].used_flag &&
            (private->streams_info[j].stream_type == CAM_STREAM_TYPE_SNAPSHOT ||
            private->streams_info[i].stream_type == CAM_STREAM_TYPE_RAW)) {

            private->stream_type = CAM_STREAM_TYPE_SNAPSHOT;
            private->reserved_id = private->streams_info[j].identity;

            rc = TRUE;
            return rc;
          }
        }
      }
      if (private->preview_stream_cnt) {
        for (j = MAX_NUM_STREAMS - 1; j >= 0; j--) {
          if (private->streams_info[j].used_flag &&
            private->streams_info[j].stream_type == CAM_STREAM_TYPE_PREVIEW) {

            private->stream_type = CAM_STREAM_TYPE_PREVIEW;
            private->reserved_id = private->streams_info[j].identity;

            rc = TRUE;
            return rc;
          }
        }
      }

      private->reserved_id = (private->reserved_id & 0xFFFF0000);

      rc = TRUE;
      break;
    }
  }

  if (!rc) {
    CDBG_ERROR("%s, Stream with identity=%d, not found in the reserved list",
      __func__, identity);
  }

  return rc;
}


/** stats_port_check_caps_reserve
 *    @port: this interface module's port;
 *    @peer_caps: the capability of peer port which wants to match
 *                interface port;
 *    @info:
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
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean stats_port_check_caps_reserve(mct_port_t *port, void *caps,
  void *info)
{
  boolean              rc = FALSE;
  mct_port_caps_t      *port_caps;
  stats_port_private_t *private;
  cam_stream_type_t    old_stream_type;
  mct_event_t          cmd_event;
  mct_event_module_t   event_data;
  stats_port_event_t   stats_event;
  mct_stream_info_t    *stream_info = info;

  memset(&(event_data), 0 , sizeof(mct_event_module_t));
  MCT_OBJECT_LOCK(port);
  if (!port || !caps || !stream_info ||
    strcmp(MCT_OBJECT_NAME(port), "stats_sink")) {

    CDBG_ERROR("%s: Invalid parameters!\n", __func__);
    rc = FALSE;
    goto reserve_done;
  }

  port_caps = (mct_port_caps_t *)caps;

  if (port_caps->port_caps_type != MCT_PORT_CAPS_STATS) {
    CDBG_ERROR("%s: Invalid Port capability type!", __func__);
    rc = FALSE;
    goto reserve_done;
  }

  private = (stats_port_private_t *)port->port_private;
  old_stream_type = private->stream_type;

  if (!stats_port_check_session_id(&(private->reserved_id),
    &(stream_info->identity))) {
    CDBG("%s, session id not match.", __func__);
    rc = FALSE;
    goto reserve_done;
  }

  CDBG("%s: state %d\n", __func__, private->state);
  /* Hack: Keep preview stream info for AFS */
  if (stream_info->stream_type == CAM_STREAM_TYPE_PREVIEW)
    private->preview_stream_info = *stream_info;
  switch (private->state) {
  case STATS_PORT_STATE_LINKED: {
    if (mct_list_find_custom(MCT_OBJECT_CHILDREN(port),
      &(stream_info->identity), stats_port_check_session_id) != NULL) {
      stats_port_caps_reserve_t reserve;

      reserve.caps = *port_caps;
      reserve.stream_info = stream_info;
      CDBG("%s: Calling stats_port_sub_caps_reserve", __func__);
      if (mct_list_traverse(private->sub_ports, stats_port_sub_caps_reserve,
        &reserve) == TRUE) {
        rc = stats_port_add_reserved_stream(private, stream_info);
      }
    }
  }
    break;

  case STATS_PORT_STATE_CREATED:
  case STATS_PORT_STATE_UNRESERVED: {
    stats_port_caps_reserve_t reserve;

    reserve.caps = *port_caps;
    reserve.stream_info = stream_info;

    if (mct_list_traverse(private->sub_ports, stats_port_sub_caps_reserve,
      &reserve) == TRUE) {
      private->reserved_id = stream_info->identity;
      private->state       = STATS_PORT_STATE_RESERVED;
      private->stream_type = stream_info->stream_type;
      rc = stats_port_add_reserved_stream(private, stream_info);
    }
  }
    break;

  case STATS_PORT_STATE_RESERVED: {
    if ((private->reserved_id & 0xFFFF0000) ==
      (stream_info->identity & 0xFFFF0000)) {
      rc = TRUE;
    }
  }
    break;

  default: {
    rc = FALSE;
  }
    break;
  }

  MCT_OBJECT_UNLOCK(port);

  if (stream_info->stream_type == CAM_STREAM_TYPE_PREVIEW) {
    unsigned int identity;
    identity = (stream_info->identity & 0x0000FFFF);
    cmd_event.type = MCT_EVENT_MODULE_EVENT;
    cmd_event.identity = private->reserved_id;
    cmd_event.direction = MCT_EVENT_DOWNSTREAM;
    cmd_event.u.module_event = event_data;
    cmd_event.u.module_event.type = MCT_EVENT_MODULE_PREVIEW_STREAM_ID;
    cmd_event.u.module_event.module_event_data = (void *)stream_info;
    stats_event.event = &cmd_event;
    rc = mct_list_traverse((mct_list_t *)private->sub_ports,
      stats_port_send_event_downstream, &stats_event);
  }

  if (old_stream_type != private->stream_type) {
    CDBG("%s: Changing stream from %d to %d", __func__, old_stream_type,
      private->stream_type);

     /* Store event data into struct */
     stats_mode_change_event_data module_event_data;
     module_event_data.reserved_id  = private->reserved_id;
     module_event_data.stream_type   = private->stream_type;

    cmd_event.type = MCT_EVENT_MODULE_EVENT;
    cmd_event.identity = private->reserved_id;
    cmd_event.direction = MCT_EVENT_DOWNSTREAM;
    cmd_event.u.module_event = event_data;
    cmd_event.u.module_event.type = MCT_EVENT_MODULE_MODE_CHANGE;
    cmd_event.u.module_event.module_event_data = (void *) &module_event_data;

    stats_event.cap_flag = 0;
    stats_event.event = &cmd_event;

    /* Send event to the stats sub-ports to change stream */
    rc = mct_list_traverse((mct_list_t *)private->sub_ports,
      stats_port_send_event_downstream, &stats_event);
  }
  if(stream_info->stream_type == CAM_STREAM_TYPE_SNAPSHOT ||
     stream_info->stream_type == CAM_STREAM_TYPE_RAW) {
    private->snap_stream_id = (stream_info->identity & 0x0000FFFF);
  }

  return rc;

reserve_done:
  MCT_OBJECT_UNLOCK(port);
  return rc;
}

/** module_stats_check_caps_unreserve
 *    @port: this port object to remove the session/stream;
 *    @identity: session+stream identity.
 *
 *  This function frees the identity from port's children list.
 *
 *  Return FALSE if the identity is not existing.
 **/
boolean stats_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  stats_port_private_t *private;
  boolean              rc = FALSE;
  cam_stream_type_t    old_stream_type;
  mct_event_t          cmd_event;
  mct_event_module_t   event_data;
  stats_port_event_t   stats_event;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "stats_sink")) {
    return FALSE;
  }

  private = (stats_port_private_t *)port->port_private;
  if (!private) {
    return FALSE;
  }

  old_stream_type = private->stream_type;

  if (private->state == STATS_PORT_STATE_UNRESERVED) {
    return TRUE;
  }

  if ((private->state == STATS_PORT_STATE_UNLINKED ||
    private->state == STATS_PORT_STATE_RESERVED) &&
    ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000))) {

    MCT_OBJECT_LOCK(port);
    mct_list_traverse(private->sub_ports, stats_port_sub_unreserve, &identity);
    rc = stats_port_delete_reserved_stream(private, identity);
    private->state       = STATS_PORT_STATE_UNRESERVED;
    MCT_OBJECT_UNLOCK(port);
  } else if (MCT_OBJECT_REFCOUNT(port)) {
    /* This case is raised in case of multiple streams when the state is
     * already unreserved. Delete stream and unreserve them with the sub-ports
     * till all streams are done with. */
    if ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000)) {
      MCT_OBJECT_LOCK(port);
      rc = stats_port_delete_reserved_stream(private, identity);
      MCT_OBJECT_UNLOCK(port);
    }
  }

  if (old_stream_type != private->stream_type) {
    CDBG("%s: Changing stream from %d to %d", __func__, old_stream_type,
      private->stream_type);

     /* Store event data into struct */
     stats_mode_change_event_data module_event_data;
     module_event_data.reserved_id  = private->reserved_id;
     module_event_data.stream_type   = private->stream_type;

    cmd_event.type = MCT_EVENT_MODULE_EVENT;
    cmd_event.identity = private->reserved_id;
    cmd_event.direction = MCT_EVENT_DOWNSTREAM;
    cmd_event.u.module_event = event_data;
    cmd_event.u.module_event.type = MCT_EVENT_MODULE_MODE_CHANGE;
    cmd_event.u.module_event.module_event_data = (void *)& module_event_data;

    stats_event.cap_flag = 0;
    stats_event.event = &cmd_event;

    /* Send event to the stats sub-ports to change stream */
    rc = mct_list_traverse((mct_list_t *)private->sub_ports,
      stats_port_send_event_downstream, &stats_event);
  }

  return rc;
}

/** stats_port_sub_ports_ext_link
 *    @data:      mct_port_t object
 *    @user_data: stats_port_sub_link_t object
 *
 *  Call the extlink function of a stats subport.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean stats_port_sub_ports_ext_link(void *data, void *user_data)
{
  boolean rc = TRUE;
  mct_port_t            *port = (mct_port_t *)data;
  stats_port_sub_link_t *ext  = (stats_port_sub_link_t *)user_data;

  if (MCT_PORT_EXTLINKFUNC(port)) {
    rc = MCT_PORT_EXTLINKFUNC(port)(ext->id, port, ext->peer);
  } else {
    rc = FALSE;
  }

  return rc;
}

/** stats_port_ext_link
 *    @identity:  Identity of session/stream
 *    @port: SRC/SINK of stats ports
 *    @peer: For stats sink- peer is most likely isp port
 *           For src module - peer is submodules sink.
 *
 *  Set stats port's external peer port.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
static boolean stats_port_ext_link(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  boolean               rc = FALSE;
  stats_port_private_t  *private;
  stats_port_sub_link_t ext_link;

  CDBG("%s: E", __func__);
  if (strcmp(MCT_OBJECT_NAME(port), "stats_sink")) {
    CDBG_ERROR("%s: Stats port name does not match!", __func__);
    return FALSE;
  }

  private = (stats_port_private_t *)port->port_private;
  if (!private) {
    CDBG_ERROR("%s: Private port NULL!", __func__);
    return FALSE;
  }

  CDBG("%s: state %d\n", __func__, private->state);
  MCT_OBJECT_LOCK(port);
  switch (private->state) {
  case STATS_PORT_STATE_RESERVED: {
    if ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000)) {
      CDBG("%s: Identity matches reserved_id!", __func__);
      rc = TRUE;
    }
  }
    break;

  case STATS_PORT_STATE_CREATED:
  case STATS_PORT_STATE_UNLINKED: {
    rc = TRUE;
  }
    break;

  case STATS_PORT_STATE_LINKED: {
    if (mct_list_find_custom(MCT_OBJECT_CHILDREN(port), &identity,
      stats_port_check_session_id) != NULL) {
      rc = TRUE;
    }
  }
    break;

  default: {
    rc = FALSE;
  }
    break;
  }

  if (rc == TRUE) {
    ext_link.id   = identity;
    ext_link.peer = port;
    CDBG("%s: Invoke sub-ports ext link", __func__);
    /* Invoke sub ports' ext link  */
    rc = mct_list_traverse(private->sub_ports, stats_port_sub_ports_ext_link,
      &ext_link);
    if (rc == TRUE) {
      private->state = STATS_PORT_STATE_LINKED;
      MCT_PORT_PEER(port) = peer;
      MCT_OBJECT_REFCOUNT(port) += 1;
      if (1 == MCT_OBJECT_REFCOUNT(port)) {
        /* Send event to start all 3A threads from here after linking first stream */
        stats_port_start_stop_stats_thread(port, TRUE);
      }
    }
  }
  MCT_OBJECT_UNLOCK(port);

  CDBG("%s:X rc=%d", __func__, rc);
  return rc;
}

/** stats_port_sub_unlink
 *    @data: mct_port_t object
 *    @user_data: stats_port_sub_link_t object
 *
 *  Unlink stats subport.
 *
 *  Return TRUE.
 **/
static boolean stats_port_sub_unlink(void *data, void *user_data)
{
  mct_port_t *port = (mct_port_t *)data;
  stats_port_sub_link_t *sub_link = (stats_port_sub_link_t *)user_data;

  if (port->un_link) {
    port->un_link(sub_link->id, port, sub_link->peer);
  }

  return TRUE;
}

/** stats_port_unlink
 *
 *    @identity: Identity of session/stream
 *    @port:     stats sink port (called from mct)
 *               stats src port called from stats sink port.
 *    @peer:     peer of stats sink port
 *
 * This funtion unlink the peer ports of stats sink, src ports
 * and its peer submodule's port
 *
 * Return void.
 **/
static void stats_port_unlink(unsigned int identity, mct_port_t *port,
  mct_port_t *peer)
{
  stats_port_private_t  *private;
  stats_port_sub_link_t sub_link;

  if (!port || !peer) {
    return;
  }

  private = (stats_port_private_t *)port->port_private;
  if (!private) {
    return;
  }

  MCT_OBJECT_LOCK(port);
  if (private->state == STATS_PORT_STATE_LINKED &&
    mct_list_find_custom(MCT_OBJECT_CHILDREN(port), &identity,
      stats_port_check_id) != NULL) {

    sub_link.id   = identity;
    sub_link.peer = port;
    if (1 == MCT_OBJECT_REFCOUNT(port)) {
      /* Send event to stop all stats threads from here before unlinking last stream */
      stats_port_start_stop_stats_thread(port, FALSE);
    }
    CDBG("%s: Invoke sub-ports ext un link", __func__);
    mct_list_traverse(private->sub_ports, stats_port_sub_unlink, &sub_link);

    MCT_OBJECT_REFCOUNT(port) -= 1;
    if (!MCT_OBJECT_REFCOUNT(port)) {
      private->state = STATS_PORT_STATE_UNLINKED;
      MCT_PORT_PEER(port) = NULL;
    }
  }
  MCT_OBJECT_UNLOCK(port);

  return;
}

/** stats_port_check_port
 *    @port:     the port to be checked
 *    @identity: the identity to be checked against the port
 *
 *  Check if the port matches the given identity.
 *
 *  Return TRUE if the identity matches, otherwise return FALSE.
 **/
boolean stats_port_check_port(mct_port_t *port, unsigned int identity)
{
  stats_port_private_t *private;

  CDBG("%s: E", __func__);

  if (!port){
    CDBG_ERROR("%s: port NULL",__func__);
    return FALSE;
  }
  if (((private = port->port_private) == NULL) ||
    strcmp(MCT_OBJECT_NAME(port), "stats_sink")) {
   CDBG("%s: E private=%p, name =%s", __func__, private,
      MCT_OBJECT_NAME(port));
    return FALSE;
  }

  return ((private->reserved_id & 0xFFFF0000) ==
    (identity & 0xFFFF0000) ? TRUE : FALSE);
}

/** stats_port_deinit
 *    @port: the port to be destroyed
 *
 *  Deinit & destroy port instance
 *
 *  Return void.
 **/
void stats_port_deinit(mct_port_t *port)
{
  stats_port_private_t *private;
  int                  i;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "stats_sink")) {
    return;
  }

  private = port->port_private;
  if (private) {
    stats_port_setparm_ctrl_t *parm_ctrl =
      (stats_port_setparm_ctrl_t *)&private->parm_ctrl;
    for (i = 0; i < CAM_INTF_PARM_MAX; i++) {
      if (parm_ctrl->evt[i].event) {
        if (parm_ctrl->evt[i].event->u.ctrl_event.control_event_data) {
          free(parm_ctrl->evt[i].event->u.ctrl_event.control_event_data);
        }
        free(parm_ctrl->evt[i].event);
      }
    }

    /* Release the sub ports list */
    mct_list_free_list(private->sub_ports);

    free(private);
    private = NULL;
  }
}

/** stats_port_init
 *    @port:      port object to be initialized
 *    @identity:  the identity of port
 *    @sub_ports: the sub ports list
 *
 *  Port initialization, use this function to overwrite
 *  default port methods and install capabilities. Stats
 *  module should have ONLY sink port.
 *
 *  Return TRUE on success, FALSE on failure.
 **/
boolean stats_port_init(mct_port_t *port, unsigned int identity,
  mct_list_t *sub_ports)
{
  mct_port_caps_t      caps;
  stats_port_private_t *private;

  private = malloc(sizeof(stats_port_private_t));
  if (private == NULL) {
    return FALSE;
  }
  memset(private, 0, sizeof(stats_port_private_t));

  private->state       = STATS_PORT_STATE_CREATED;
  private->sub_ports   = sub_ports;
  private->reserved_id = identity;

  port->port_private   = private;
  port->direction      = MCT_PORT_SINK;

  caps.port_caps_type  = MCT_PORT_CAPS_STATS;
  caps.u.stats.flag    = (MCT_PORT_CAP_STATS_Q3A | MCT_PORT_CAP_STATS_CS_RS |
    MCT_PORT_CAP_STATS_HIST);

  mct_port_set_event_func(port, stats_port_event);
  mct_port_set_set_caps_func(port, stats_port_set_caps);
  mct_port_set_ext_link_func(port, stats_port_ext_link);
  mct_port_set_unlink_func(port, stats_port_unlink);
  mct_port_set_check_caps_reserve_func(port, stats_port_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port, stats_port_check_caps_unreserve);

  if (port->set_caps) {
    port->set_caps(port, &caps);
  }

  return TRUE;
}

/* This function simply returns the TRUE or FALSE, based on debug mask */
boolean is_stats_buffer_debug_data_enable(mct_port_t *port)
{
  boolean exif_dbg_enable = 0;

  if (!port) {
    CDBG_ERROR("%s Null pointer", __func__);
    return 0;
  }
  stats_port_private_t *private = (stats_port_private_t *)port->port_private;
  if (private->stats_debug_mask & EXIF_DEBUG_MASK_STATS) {
    exif_dbg_enable = 1;
  }
  return (exif_dbg_enable);
}


/* Function to copy BG stats and Hist stats into debug data structure. */
static void copy_stats_buffer_to_debug_data(mct_event_t *event,
  mct_port_t *port)
{
  if (!event || !port) {
    CDBG_ERROR("%s Null pointer", __func__);
    return;
  }
  stats_port_private_t *private = (stats_port_private_t *)port->port_private;
  const mct_event_stats_isp_t *const stats_event =
    (mct_event_stats_isp_t *)event->u.module_event.module_event_data;

  if (stats_event->stats_mask & (1 << MSM_ISP_STATS_BG)) {
    uint32 index = 0;
    const q3a_bg_stats_t* const p_bg_stats =
      (q3a_bg_stats_t*)stats_event->stats_data[MSM_ISP_STATS_BG].stats_buf;
    BayerGridStatsType* const pBgDebugStats = &(private->bg_stats_debug_Data);
    const int32 debug_data_size = sizeof(private->bg_stats_debug_Data.redChannelSum);
    const int32 q3a_data_size = sizeof(p_bg_stats->bg_r_sum);
    const int32 copy_size = (q3a_data_size > debug_data_size) ?
      debug_data_size : q3a_data_size;

    /* Horizontal and vertical regions */
    pBgDebugStats->bgStatsNumHorizontalRegions =
      (uint16)p_bg_stats->bg_region_h_num;
    pBgDebugStats->bgStatsNumVerticalRegions =
      (uint16)p_bg_stats->bg_region_v_num;

    /* Instead of loop, used memcopy for better performance. */
    /* Copy BG stats sum array. */
    memcpy(pBgDebugStats->redChannelSum, p_bg_stats->bg_r_sum, copy_size);
    memcpy(pBgDebugStats->grChannelSum, p_bg_stats->bg_gr_sum, copy_size);
    memcpy(pBgDebugStats->gbChannelSum, p_bg_stats->bg_gb_sum, copy_size);
    memcpy(pBgDebugStats->blueChannelSum, p_bg_stats->bg_b_sum, copy_size);

    /* Copy BG stats count array. */
    for (index = 0;
      (index < BAYER_GRID_NUM_REGIONS) && (index < MAX_BG_STATS_NUM);
      index++) {
      pBgDebugStats->redChannelCount[index] = (uint16)p_bg_stats->bg_r_num[index];
      pBgDebugStats->grChannelCount[index] = (uint16)p_bg_stats->bg_gr_num[index];
      pBgDebugStats->gbChannelCount[index] = (uint16)p_bg_stats->bg_gb_num[index];
      pBgDebugStats->blueChannelCount[index] = (uint16)p_bg_stats->bg_b_num[index];
    }
    private->bg_stats_buffer_size = sizeof(private->bg_stats_debug_Data);
  }
  if (stats_event->stats_mask & (1 << MSM_ISP_STATS_BHIST)) {
    const q3a_bhist_stats_t* const p_hist_stats =
      (q3a_bhist_stats_t*)stats_event->stats_data[MSM_ISP_STATS_BHIST].stats_buf;
    BayerHistogramStatsType* const pHistDebugStats = &(private->hist_stats_debug_Data);
    const int32 debug_data_size = sizeof(private->hist_stats_debug_Data.grChannel);
    const int32 q3a_data_size = sizeof(p_hist_stats->bayer_gr_hist);
    const int32 copy_size = (q3a_data_size > debug_data_size) ?
      debug_data_size : q3a_data_size;

    /* Instead of loop, used memcopy for better performance. */
    /* Copy Gr histogram stats. */
    memcpy(pHistDebugStats->grChannel, p_hist_stats->bayer_gr_hist, copy_size);
    private->bhist_stats_buffer_size = sizeof(private->hist_stats_debug_Data);
  }
}

static void send_stats_buffer_to_debug_data(mct_port_t *port)
{
  mct_event_t event;
  mct_bus_msg_t bus_msg;
  cam_stats_buffer_exif_debug_t stats_buffer_info;
  stats_port_private_t *private;
  int size = 0, total_stats_buff_size = 0;

  if (!port) {
    CDBG_ERROR("%s: input error", __func__);
    return;
  }
  private = (stats_port_private_t *)(port->port_private);
  total_stats_buff_size = private->bg_stats_buffer_size +
    private->bhist_stats_buffer_size;

  if (STATS_BUFFER_DEBUG_DATA_SIZE < total_stats_buff_size ||
    0 == total_stats_buff_size) {
    CDBG_ERROR("%s Stats buffer debug data send error. buffer size mismatch"
      "buffer size: %d, BG stats size: %d, bhist stats size %d", __func__,
      STATS_BUFFER_DEBUG_DATA_SIZE, private->bg_stats_buffer_size,
      private->bhist_stats_buffer_size);
    return;
  }
  bus_msg.sessionid = (private->reserved_id >> 16);
  bus_msg.type = MCT_BUS_MSG_STATS_EXIF_DEBUG_INFO;
  bus_msg.msg = (void *)&stats_buffer_info;
  size = (int)sizeof(cam_stats_buffer_exif_debug_t);
  bus_msg.size = size;
  memset(&stats_buffer_info, 0, size);
  stats_buffer_info.bg_stats_buffer_size =
    private->bg_stats_buffer_size;
  stats_buffer_info.bhist_stats_buffer_size =
    private->bhist_stats_buffer_size;
  CDBG("%s Stats buffer debug data size. bg: %d, bhist: %d", __func__,
    private->bg_stats_buffer_size,
    private->bhist_stats_buffer_size);

  /* Copy the bg stats debug data if data size is valid */
  if (private->bg_stats_buffer_size) {
    memcpy(&(stats_buffer_info.stats_buffer_private_debug_data[0]),
      &(private->bg_stats_debug_Data), private->bg_stats_buffer_size);
  }

  /* Copy the bhist stats debug data if data size is valid */
  if (private->bhist_stats_buffer_size) {
    memcpy(&(stats_buffer_info.stats_buffer_private_debug_data[private->bg_stats_buffer_size]),
      &(private->hist_stats_debug_Data), private->bhist_stats_buffer_size);
  }
  event.direction = MCT_EVENT_UPSTREAM;
  event.identity = private->reserved_id;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_POST_TO_BUS;
  event.u.module_event.module_event_data = (void *)(&bus_msg);
  MCT_PORT_EVENT_FUNC(port)(port, &event);
}

