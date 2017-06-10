/* gyro_port.c
 *
 * Copyright (c) 2013 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
*/

#include "mct_port.h"
#include "mct_stream.h"
#include "modules.h"
#include "stats_module.h"
#include "stats_event.h"
#include "dsps_hw_interface.h"
#include "aec.h"
#include "camera_dbg.h"


#if 0
#undef CDBG
#define CDBG ALOGE
#endif

#undef CDBG_HIGH
#if 1
#define CDBG_HIGH ALOGE
#else
#define CDBG_HIGH
#endif

#undef CDBG_LOW
#if 0
#define CDBG_LOW ALOGE
#else
#define CDBG_LOW
#endif

#define SEC_TO_USEC     (1000000L)
#define Q8              (0x00000100)
#define BUF_SIZE        (2)


typedef enum {
  GYRO_PORT_STATE_CREATED = 1,
  GYRO_PORT_STATE_RESERVED,
  GYRO_PORT_STATE_LINKED,
  GYRO_PORT_STATE_UNLINKED,
  GYRO_PORT_STATE_UNRESERVED
} gyro_port_state_t;


/** _gyro_port_private_t:
 *    @reserved_id: session id | stream id
 *    @state: state of gyro port
 *    @sof: start of frame timestamp in microseconds.
 *    @frame_time: time when the last line of the sensor is read in
 *                 microseconds.  Does not include vertical blanking time
 *    @last_time: end time of the last gyro sample interval in microseconds
 *    @exposure_time: exposure time in seconds
 *    @gyro_stats_ready: indicates whether gyro data is avaliable for broadcast
 *    @gyro_stats: MCT_EVENT_MODULE_STATS_GYRO_STATS event body
 *    @dsps_handle: DSPS handle for access to gyro data
 *
 *  This structure defines gyro port's private variables
 **/
typedef struct _gyro_port_private {
  unsigned int reserved_id;
  gyro_port_state_t state;
  unsigned int frame_id[BUF_SIZE];
  unsigned long long sof[BUF_SIZE];
  unsigned int sof_event_identity;
  unsigned long long frame_time;
  unsigned long long last_time;
  float exposure_time[BUF_SIZE];
  unsigned int gyro_stats_ready;
  mct_event_gyro_stats_t gyro_stats;
  void *dsps_handle;
} gyro_port_private_t;


/** gyro_port_handle_sof_event:
 *    @port: gyro port
 *    @event: event object
 *    @private: gyro port private variables
 *
 * This function handles the SOF event.
 **/
static void gyro_port_handle_sof_event(mct_port_t *port, mct_event_t *event,
  gyro_port_private_t *private)
{
  mct_bus_msg_isp_sof_t *sof_event =
    (mct_bus_msg_isp_sof_t *)event->u.module_event.module_event_data;
  mct_event_t fps_event;
  unsigned long fps;
  unsigned long long sof, eof;
  dsps_set_data_t dsps_set;
  dsps_get_data_t dsps_get;
  struct timeval tv;
  int i;

  if (gettimeofday(&tv, NULL) == 0) {
    CDBG("%s, time = %llu, fid = %u, eid = %u", __func__,
      (((int64_t)tv.tv_sec * 1000000) + tv.tv_usec), sof_event->frame_id,
      event->identity);
  }

  /* Get FPS */
  fps_event.type = MCT_EVENT_MODULE_EVENT;
  fps_event.identity = event->identity;
  fps_event.direction = MCT_EVENT_UPSTREAM;
  fps_event.u.module_event.type = MCT_EVENT_MODULE_3A_GET_CUR_FPS;
  fps_event.u.module_event.module_event_data = &fps;
  mct_port_send_event_to_peer(port, &fps_event);
  fps /= Q8;

  i = sof_event->frame_id % BUF_SIZE;
  private->frame_id[i] = sof_event->frame_id;
  private->sof_event_identity = event->identity;

  /* Calculate time interval */
  sof = (unsigned long long)sof_event->timestamp.tv_sec * SEC_TO_USEC
      + (unsigned long long)sof_event->timestamp.tv_usec;
  eof = sof + private->frame_time;
  dsps_set.data.t_start = private->last_time ?
    private->last_time : sof - 15000;
  /* Below is the gyro interval request end time for EIS 2.0 */
  dsps_set.data.t_end = sof + private->frame_time -
   (double)private->exposure_time[i] * SEC_TO_USEC / 2;
  /* Below is the gyro interval request end time for EIS 1.0 */
  /* dsps_set.data.t_end = (sof + eof) / 2 -
    (long)((double)private->exposure_time[i] * SEC_TO_USEC / 2) + 5000; */
  private->last_time = dsps_set.data.t_end;
  private->sof[i] = sof;
  CDBG("%s: sof = %llu, frame_time = %llu, exp/2 = %f, fps = %lu",
    __func__, sof, private->frame_time,
    (double)private->exposure_time[i] * SEC_TO_USEC / 2, fps);

  CDBG("%s: Post for gyro interval %llu, %llu, %llu", __func__,
    dsps_set.data.t_start, dsps_set.data.t_end,
    dsps_set.data.t_end - dsps_set.data.t_start);

  /* Request gyro samples from t_start to t_end */
  dsps_set.msg_type = DSPS_GET_REPORT;
  dsps_set.data.id = sof_event->frame_id;
  dsps_proc_set_params(private->dsps_handle, &dsps_set);
}


/** gyro_port_callback
 *    @port: gyro port
 *    @frame_id: frame_id (LSB)
 *
 *  This is the callback function given to the DSPS layer.  It gets called when
 *  DSPS layer receives gyro samples from sensors.
 **/
static void gyro_port_callback(void *port, unsigned int frame_id)
{
  gyro_port_private_t *private = ((mct_port_t *)port)->port_private;
  dsps_get_data_t dsps_get;

  CDBG("%s: frame id = %u", __func__, frame_id);
  dsps_get.type = GET_DATA_FRAME;
  dsps_get.format = TYPE_Q16;
  dsps_get.id = (unsigned short)frame_id;

  if (dsps_get_params(private->dsps_handle, &dsps_get, 1) == 0) {
    mct_event_gyro_stats_t gyro_stats;
    mct_event_t gyro_stats_event;
    unsigned int i, j;

    i = frame_id % BUF_SIZE;
    if ((private->frame_id[i] & 0x0FF) == frame_id) {
      gyro_stats.is_gyro_data.sof = private->sof[i];
      gyro_stats.is_gyro_data.frame_time = private->frame_time;
      gyro_stats.is_gyro_data.exposure_time = private->exposure_time[i];

      gyro_stats.is_gyro_data.frame_id = private->frame_id[i];

      gyro_stats.is_gyro_data.sample_len = dsps_get.output_data.sample_len;
      for (j = 0; j < dsps_get.output_data.sample_len; j++) {
        gyro_stats.is_gyro_data.sample[j] = dsps_get.output_data.sample[j];
        CDBG_LOW("%s: gyro sample[%d], %llu, %d, %d, %d", __func__, j,
          gyro_stats.is_gyro_data.sample[j].timestamp,
          gyro_stats.is_gyro_data.sample[j].value[0],
          gyro_stats.is_gyro_data.sample[j].value[1],
          gyro_stats.is_gyro_data.sample[j].value[2]);
      }

      if (dsps_get.output_data.sample_len > 2) {
        gyro_stats.q16_angle[0] = dsps_get.output_data.sample[1].value[0];
        gyro_stats.q16_angle[1] = dsps_get.output_data.sample[1].value[1];
        gyro_stats.q16_angle[2] = dsps_get.output_data.sample[1].value[2];
        gyro_stats.ts = dsps_get.output_data.sample[1].timestamp;
      }

      gyro_stats_event.type = MCT_EVENT_MODULE_EVENT;
      gyro_stats_event.identity = private->sof_event_identity;
      gyro_stats_event.direction = MCT_EVENT_UPSTREAM;
      gyro_stats_event.u.module_event.type = MCT_EVENT_MODULE_STATS_GYRO_STATS;
      gyro_stats_event.u.module_event.module_event_data = &gyro_stats;
      mct_port_send_event_to_peer(port, &gyro_stats_event);
    } else {
      ALOGE("%s: gyro_port queue overflow", __func__);
    }
  }
  else {
    ALOGE("%s: Error fetching gyro data", __func__);
  }
}


/** gyro_port_event:
 *    @port: gyro port
 *    @event: event
 *
 *  This function handles events for the gyro port.
 *
 *  Returns TRUE on success.
 **/
static boolean gyro_port_event(mct_port_t *port, mct_event_t *event)
{
  boolean rc = TRUE;
  gyro_port_private_t *private;

  /* sanity check */
  if (!port || !event)
    return FALSE;

  private = (gyro_port_private_t *)(port->port_private);
  if (!private)
    return FALSE;

  /* sanity check: ensure event is meant for port with same identity*/
  if ((private->reserved_id & 0xFFFF0000) != (event->identity & 0xFFFF0000))
  {
    return FALSE;
  }

  switch (MCT_EVENT_DIRECTION(event)) {
  case MCT_EVENT_DOWNSTREAM: {
    switch (event->type) {
    case MCT_EVENT_CONTROL_CMD: {
      mct_event_control_t *control = &event->u.ctrl_event;

      CDBG("%s: Control event type %d", __func__, control->type);
      switch (control->type) {
      case MCT_EVENT_CONTROL_STREAMON:
        private->last_time = 0;
        break;

      case MCT_EVENT_CONTROL_STREAMOFF:
        break;

      case MCT_EVENT_CONTROL_SET_PARM:
        break;

      default:
        break;
      }
    } /* case MCT_EVENT_CONTROL_CMD */
      break;

    case MCT_EVENT_MODULE_EVENT: {
      mct_event_module_t *mod_event = &event->u.module_event;

      CDBG_LOW("%s: Module event type %d", __func__, mod_event->type);
      switch (mod_event->type) {
      case MCT_EVENT_MODULE_SOF_NOTIFY:
        gyro_port_handle_sof_event(port, event, private);
        break;

      case MCT_EVENT_MODULE_GET_GYRO_DATA: {
        mct_event_gyro_data_t *gyro_data_event =
          (mct_event_gyro_data_t *)mod_event->module_event_data;
        dsps_get_data_t dsps_get;

        CDBG("%s: Got MCT_EVENT_MODULE_GET_GYRO_DATA %u", __func__,
          gyro_data_event->frame_id);
        /* Call dsps_get_params */
        dsps_get.type = GET_DATA_FRAME;
        dsps_get.format = TYPE_Q16;
        dsps_get.id = (unsigned short)gyro_data_event->frame_id;

        if (dsps_get_params(private->dsps_handle, &dsps_get, 1) == 0) {
          unsigned int i;

          gyro_data_event->ready = 1;
          gyro_data_event->sample_len = dsps_get.output_data.sample_len;
          for (i = 0; i < dsps_get.output_data.sample_len; i++) {
            gyro_data_event->sample[i] = dsps_get.output_data.sample[i];
          }
          i = gyro_data_event->frame_id % BUF_SIZE;
          gyro_data_event->sof = private->sof[i];
          gyro_data_event->frame_time = private->frame_time;
          gyro_data_event->exposure_time = private->exposure_time[i];

          if (dsps_get.output_data.sample_len > 2) {
            private->gyro_stats.q16_angle[0] =
              dsps_get.output_data.sample[1].value[0];
            private->gyro_stats.q16_angle[1] =
              dsps_get.output_data.sample[1].value[1];
            private->gyro_stats.q16_angle[2] =
              dsps_get.output_data.sample[1].value[2];
            private->gyro_stats.ts = dsps_get.output_data.sample[1].timestamp;
            private->gyro_stats_ready = 1;
          }
        }
        else {
          gyro_data_event->ready = 0;
          gyro_data_event->sample_len = 0;
        }
      }
        break;

      case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {
        aec_update_t *aec_update_data;
        int i;

        aec_update_data = (aec_update_t *)mod_event->module_event_data;
        i = aec_update_data->frame_id % BUF_SIZE;
        CDBG("%s: fid = %u, exp time = %.6f", __func__,
          aec_update_data->frame_id, aec_update_data->exp_time);
        private->exposure_time[i] = aec_update_data->exp_time;
      }
        break;

      case MCT_EVENT_MODULE_SET_STREAM_CONFIG: {
        sensor_out_info_t *sensor_info;

        CDBG("%s: Got MCT_EVENT_MODULE_SET_STREAM_CONFIG", __func__);
        sensor_info = (sensor_out_info_t *)mod_event->module_event_data;
        private->frame_time = (unsigned long long)
          ((sensor_info->ll_pck * sensor_info->dim_output.height) /
           (sensor_info->vt_pixel_clk / SEC_TO_USEC));
      }
        break;

      default:
        break;
      }
    } /* case MCT_EVENT_MODULE_EVENT */
      break;

    default:
      break;
    } /* switch (event->type) */

  } /* case MCT_EVENT_TYPE_DOWNSTREAM */
      break;

  default:
    break;
  } /* switch (MCT_EVENT_DIRECTION(event)) */

  return rc;
}


/** gyro_port_ext_link:
 *    @identity: session id | stream id
 *    @port: gyro port
 *    @peer: For gyro sink port, peer is most likely stats port
 *
 *  Sets gyro port's external peer port.
 *
 *  Returns TRUE on success.
 **/
static boolean gyro_port_ext_link(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  boolean rc = FALSE;
  gyro_port_private_t *private;
  mct_event_t event;

  CDBG("%s: Enter", __func__);
  if (strcmp(MCT_OBJECT_NAME(port), "gyro_sink"))
    return FALSE;

  private = (gyro_port_private_t *)port->port_private;
  if (!private)
    return FALSE;

  MCT_OBJECT_LOCK(port);
  switch (private->state) {
  case GYRO_PORT_STATE_RESERVED:
  case GYRO_PORT_STATE_UNLINKED:
    if ((private->reserved_id & 0xFFFF0000) != (identity & 0xFFFF0000)) {
      break;
    }
  case GYRO_PORT_STATE_CREATED:
    /* If gyro module requires a thread, indicate here. */
    rc = TRUE;
    break;

  case GYRO_PORT_STATE_LINKED:
    if ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000)) {
      rc = TRUE;
    }
    break;

  default:
    break;
  }

  if (rc == TRUE) {
    /* If gyro module requires a thread and the port state above warrants one,
       create the thread here */
    private->state = GYRO_PORT_STATE_LINKED;
    MCT_PORT_PEER(port) = peer;
    MCT_OBJECT_REFCOUNT(port) += 1;
  }

  MCT_OBJECT_UNLOCK(port);
  CDBG("%s: rc=%d", __func__, rc);
  return rc;
}


/** gyro_port_unlink:
 *  @identity: session id | stream id
 *  @port: gyro port
 *  @peer: gyro port's peer port (probably stats port)
 *
 *  This funtion unlinks the gyro port from its peer.
 **/
static void gyro_port_unlink(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  gyro_port_private_t *private;

  if (!port || !peer || MCT_PORT_PEER(port) != peer)
    return;

  private = (gyro_port_private_t *)port->port_private;
  if (!private)
    return;

  MCT_OBJECT_LOCK(port);
  if (private->state == GYRO_PORT_STATE_LINKED &&
      (private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000)) {

    /* Take care of stopping gyro thread (if any) here */

    MCT_OBJECT_REFCOUNT(port) -= 1;
    if (!MCT_OBJECT_REFCOUNT(port)) {
      private->state = GYRO_PORT_STATE_UNLINKED;
      MCT_PORT_PEER(port) = NULL;
    }
  }
  MCT_OBJECT_UNLOCK(port);

  return;
}


/** gyro_port_set_caps:
 *    @port: port object whose caps are to be set
 *    @caps: this port's capability.
 *
 *  Function overwrites a ports capability.
 *
 *  Returns TRUE if it is valid source port.
 **/
static boolean gyro_port_set_caps(mct_port_t *port,
  mct_port_caps_t *caps)
{
  CDBG("%s: Enter", __func__);
  if (strcmp(MCT_PORT_NAME(port), "gyro_sink"))
    return FALSE;

  port->caps = *caps;
  return TRUE;
}


/** gyro_port_check_caps_reserve:
 *    @port: this interface module's port
 *    @peer_caps: the capability of peer port which wants to match
 *                interface port
 *    @stream_info: stream information
 *
 *  Returns TRUE on success.
 **/
static boolean gyro_port_check_caps_reserve(mct_port_t *port, void *caps,
  void *stream_info)
{
  boolean rc = FALSE;
  mct_port_caps_t *port_caps;
  gyro_port_private_t *private;
  mct_stream_info_t *strm_info = (mct_stream_info_t *)stream_info;

  CDBG("%s: Enter", __func__);
  MCT_OBJECT_LOCK(port);
  if (!port || !caps || !strm_info ||
      strcmp(MCT_OBJECT_NAME(port), "gyro_sink")) {
    CDBG("%s: Exit unsucessful", __func__);
    goto reserve_done;
  }

  port_caps = (mct_port_caps_t *)caps;
  /* Change later to SOF? */
  if (port_caps->port_caps_type != MCT_PORT_CAPS_STATS) {
    rc = FALSE;
    goto reserve_done;
  }

  private = (gyro_port_private_t *)port->port_private;
  if (!private){
      rc = FALSE;
      goto reserve_done;
  }

  switch (private->state) {
  case GYRO_PORT_STATE_LINKED:
  if ((private->reserved_id & 0xFFFF0000) ==
      (strm_info->identity & 0xFFFF0000))
    rc = TRUE;
  break;

  case GYRO_PORT_STATE_CREATED:
  case GYRO_PORT_STATE_UNRESERVED:
    private->reserved_id = strm_info->identity;
    private->state = GYRO_PORT_STATE_RESERVED;
    rc = TRUE;
    break;

  case GYRO_PORT_STATE_RESERVED:
    if ((private->reserved_id & 0xFFFF0000) ==
        (strm_info->identity & 0xFFFF0000))
      rc = TRUE;
    break;

  default:
    break;
  }

reserve_done:
  MCT_OBJECT_UNLOCK(port);
  return rc;
}


/** gyro_port_check_caps_unreserve:
 *    @port: this port object to remove the session/stream
 *    @identity: session+stream identity
 *
 *  This function frees the identity from port's children list.
 *
 *  Returns FALSE if the identity does not exist.
 **/
static boolean gyro_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  boolean rc = FALSE;
  gyro_port_private_t *private;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "gyro_sink"))
    return FALSE;

  private = (gyro_port_private_t *)port->port_private;
  if (!private)
    return FALSE;

  if (private->state == GYRO_PORT_STATE_UNRESERVED)
    return TRUE;

  MCT_OBJECT_LOCK(port);
  if ((private->state == GYRO_PORT_STATE_UNLINKED ||
       private->state == GYRO_PORT_STATE_RESERVED) &&
      ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000))) {

    if (!MCT_OBJECT_REFCOUNT(port)) {
      private->state = GYRO_PORT_STATE_UNRESERVED;
      private->reserved_id = (private->reserved_id & 0xFFFF0000);
    }
    rc = TRUE;
  }

unreserve_done:
  MCT_OBJECT_UNLOCK(port);
  return rc;
}


/** gyro_port_init:
 *    @port: gyro port
 *    @session_id: session id
 *    @dsps_handle: DSPS handle
 *
 *  This function initializes the gyro port's internal variables.
 *
 *  Returns TRUE on success.
 **/
boolean gyro_port_init(mct_port_t *port, unsigned int session_id)
{
  mct_port_caps_t caps;
  gyro_port_private_t *private;

  if (port == NULL || strcmp(MCT_OBJECT_NAME(port), "gyro_sink")) {
    return FALSE;
  }

  private = (void *)malloc(sizeof(gyro_port_private_t));
  if (!private) {
    return FALSE;
  }

  memset(private, 0, sizeof(gyro_port_private_t));
  private->reserved_id = session_id;
  private->state = GYRO_PORT_STATE_CREATED;
  port->port_private = private;
  port->direction = MCT_PORT_SINK;
  caps.port_caps_type = MCT_PORT_CAPS_STATS;
  caps.u.stats.flag = MCT_PORT_CAP_STATS_GYRO;

  mct_port_set_event_func(port, gyro_port_event);
  CDBG("%s: gyro_port_event addr = %p", __func__, gyro_port_event);
  /* Accept default int_link function */
  mct_port_set_ext_link_func(port, gyro_port_ext_link);
  mct_port_set_unlink_func(port, gyro_port_unlink);
  mct_port_set_set_caps_func(port, gyro_port_set_caps);
  mct_port_set_check_caps_reserve_func(port, gyro_port_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port, gyro_port_check_caps_unreserve);

  if (port->set_caps) {
    port->set_caps(port, &caps);
  }

  CDBG_HIGH("%s: Init DSPS", __func__);
  private->dsps_handle = dsps_proc_init(port, gyro_port_callback);
  if (private->dsps_handle != NULL) {
    dsps_set_data_t dsps_set_data;

    CDBG_HIGH("%s: DSPS inited", __func__);
    dsps_set_data.msg_type = DSPS_ENABLE_REQ;
    dsps_set_data.sensor_feature.mode = ENABLE_ALL;
    if (dsps_proc_set_params(private->dsps_handle, &dsps_set_data) != 0) {
      CDBG_HIGH("%s: DSPS enable req failed", __func__);
      dsps_proc_deinit(private->dsps_handle);
      CDBG_HIGH("%s: DSPS deinited", __func__);
      private->dsps_handle = NULL;
    }
  }

  return TRUE;
}


/** gyro_port_deinit:
 *    @port: gyro port
 *
 * This function frees the gyro port's memory.
 **/
void gyro_port_deinit(mct_port_t *port)
{
  gyro_port_private_t *private;
  if (!port || strcmp(MCT_OBJECT_NAME(port), "gyro_sink")) {
    return;
  }

  private = port->port_private;
  CDBG_HIGH("%s: Deinit DSPS", __func__);
  if (private->dsps_handle != NULL) {
    dsps_set_data_t dsps_set_data;

    dsps_set_data.msg_type = DSPS_DISABLE_REQ;
    dsps_set_data.sensor_feature.mode = 0;
    dsps_proc_set_params(private->dsps_handle, &dsps_set_data);
    dsps_proc_deinit(private->dsps_handle);
    CDBG_HIGH("%s: DSPS deinited", __func__);
  }

  if (port->port_private) {
    free(port->port_private);
  }
}


/** gyro_port_find_identity:
 *    @port: gyro port
 *    @identity: session id | stream id
 *
 * This function checks for the port with a given session.
 *
 * Returns TRUE if the port is found.
 **/
boolean gyro_port_find_identity(mct_port_t *port, unsigned int identity)
{
  gyro_port_private_t *private;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "gyro_sink"))
    return TRUE;

  private = port->port_private;

  if (private) {
    return ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000) ?
            TRUE : FALSE);
  }

  return FALSE;
}

