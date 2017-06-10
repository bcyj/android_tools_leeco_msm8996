/* is_port.c
 *
 * Copyright (c) 2013 - 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "mct_port.h"
#include "mct_stream.h"
#include "modules.h"
#include "stats_module.h"
#include "is.h"
#include "is_thread.h"
#include "is_port.h"
#include "aec.h"
#include "camera_dbg.h"
#include "c2dExt.h"
/* This should be declared in sensor_lib.h */
void poke_gyro_sample(uint64_t t, int32_t gx, int32_t gy, int32_t gz);

#define IS_VIDEO_STREAM_RUNNING (private->video_reserved_id & 0xFFFF)
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

#define SEC_TO_USEC     (1000000L)


/** is_port_init_thread:
 *    @port: IS port
 *
 *  Returns TRUE IS thread was successfuly started.
 **/
static boolean is_port_init_thread(mct_port_t *port)
{
  boolean rc = FALSE;
  is_port_private_t *private;

  private = (is_port_private_t *)port->port_private;
  private->thread_data = is_thread_init();
  CDBG("%s private->thread_data: %p", __func__, private->thread_data);
  if (private->thread_data != NULL) {
    private->thread_data->is_port = port;
    rc = TRUE;
  } else {
    CDBG_ERROR("%s private->thread_data is NULL", __func__);
  }
  return rc;
}


/** is_port_start_thread:
 *    @port: IS port
 *
 *  Returns TRUE IS thread was successfuly started.
 **/
static boolean is_port_start_thread(mct_port_t *port)
{
  boolean rc = FALSE;
  is_port_private_t *private;

  private = (is_port_private_t *)port->port_private;
  if (private->thread_data != NULL) {
    CDBG("%s: Start IS thread", __func__);
    rc = is_thread_start(private->thread_data);
    if (rc == FALSE) {
      is_thread_deinit(private->thread_data);
      private->thread_data = NULL;
    }
  } else {
    rc = FALSE;
  }
  return rc;
}


/** is_port_handle_stream_config_event:
 *    @private: private port data
 *    @mod_evt: module event
 **/
static boolean is_port_handle_stream_config_event(is_port_private_t *private,
  mct_event_module_t *mod_event)
{
  boolean rc = TRUE;
  sensor_out_info_t *sensor_info =
    (sensor_out_info_t *)mod_event->module_event_data;

  CDBG_HIGH("%s: w = %u, h = %u, ma = %u, p = %d", __func__,
    sensor_info->dim_output.width, sensor_info->dim_output.height,
    sensor_info->sensor_mount_angle, sensor_info->position);

  is_thread_msg_t *msg = (is_thread_msg_t *)
    malloc(sizeof(is_thread_msg_t));

  if (msg != NULL ) {
    memset(msg, 0, sizeof(is_thread_msg_t));
    msg->type = MSG_IS_SET;
    msg->u.is_set_parm.type = IS_SET_PARAM_STREAM_CONFIG;
    msg->u.is_set_parm.u.is_sensor_info.sensor_mount_angle =
      sensor_info->sensor_mount_angle;
    msg->u.is_set_parm.u.is_sensor_info.camera_position =
      sensor_info->position;
    is_thread_en_q_msg(private->thread_data, msg);
  } else {
    CDBG_ERROR("%s: malloc failed!", __func__);
    rc = FALSE;
  }

  return rc;
}


/** is_port_handle_set_is_enable:
 *    @private: private port data
 *    @ctrl_evt: control event
 *
 *  Returns TRUE if event was successfuly queued to the IS thread for
 *  processing.
 **/
static boolean is_port_handle_set_is_enable(is_port_private_t *private,
  mct_event_control_t *ctrl_event)
{
  boolean rc = TRUE;
  stats_set_params_type *stats_parm = ctrl_event->control_event_data;

  CDBG_HIGH("%s: IS enable = %d", __func__, stats_parm->u.is_param.u.is_enable);
  is_thread_msg_t *msg = (is_thread_msg_t *)
    malloc(sizeof(is_thread_msg_t));

  if (msg != NULL ) {
    memset(msg, 0, sizeof(is_thread_msg_t));
    msg->type = MSG_IS_SET;
    msg->u.is_set_parm.type = IS_SET_PARAM_IS_ENABLE;
    msg->u.is_set_parm.u.is_enable = stats_parm->u.is_param.u.is_enable;
    is_thread_en_q_msg(private->thread_data, msg);
  } else {
    CDBG_ERROR("%s: malloc failed!", __func__);
    rc = FALSE;
  }

  return rc;
}


/** is_port_handle_stream_event:
 *    @private: private port data
 *    @event: event
 *
 *  Returns TRUE if event was successfuly queued to the IS thread for
 *  processing.
 **/
static boolean is_port_handle_stream_event(is_port_private_t *private,
  mct_event_t *event)
{
  boolean rc = TRUE;
  mct_event_control_t *control = &event->u.ctrl_event;

  if (control->type == MCT_EVENT_CONTROL_STREAMON) {
    CDBG_HIGH("%s: MCT_EVENT_CONTROL_STREAMON, identity = 0x%x", __func__,
      event->identity);
  } else {
    CDBG_HIGH("%s: MCT_EVENT_CONTROL_STREAMOFF, identity = 0x%x", __func__,
      event->identity);
  }

  is_thread_msg_t *msg = (is_thread_msg_t *)
    malloc(sizeof(is_thread_msg_t));

  if (msg != NULL ) {
    memset(msg, 0, sizeof(is_thread_msg_t));
    msg->type = MSG_IS_PROCESS;
    msg->u.is_process_parm.type = IS_PROCESS_STREAM_EVENT;
    if (event->identity == private->video_reserved_id) {
      if (control->type == MCT_EVENT_CONTROL_STREAMON) {
        msg->u.is_process_parm.u.stream_event_data.stream_event =
          IS_VIDEO_STREAM_ON;
      } else {
        msg->u.is_process_parm.u.stream_event_data.stream_event =
          IS_VIDEO_STREAM_OFF;
      }
    } else {
      msg->u.is_process_parm.u.stream_event_data.stream_event =
        IS_OTHER_STREAM_ON_OFF;
    }
    msg->u.is_process_parm.u.stream_event_data.is_info = &private->is_info;
    is_thread_en_q_msg(private->thread_data, msg);
  } else {
    CDBG_ERROR("%s: malloc failed!", __func__);
    rc = FALSE;
  }

  return rc;
}


/** is_port_handle_stats_event:
 *    @port: IS port
 *    @event: event
 *
 *  Returns TRUE if event was successfully handled.
 **/
static boolean is_port_handle_stats_event(mct_port_t *port, mct_event_t *event)
{
  boolean rc = TRUE;
  is_port_private_t *private = (is_port_private_t *)port->port_private;
  mct_event_stats_isp_t *stats_event =
    (mct_event_stats_isp_t *)event->u.module_event.module_event_data;

  if (private->video_stream_on) {
    if ((stats_event->stats_mask & (1 << MSM_ISP_STATS_RS)) &&
        (stats_event->stats_mask & (1 << MSM_ISP_STATS_CS))) {
      private->RSCS_stats_ready = TRUE;
      if (!private->is_info.is_inited) {
        private->is_info.num_row_sum = ((q3a_rs_stats_t *)
          stats_event->stats_data[MSM_ISP_STATS_RS].stats_buf)->num_row_sum;
        private->is_info.num_col_sum = ((q3a_cs_stats_t *)
          stats_event->stats_data[MSM_ISP_STATS_CS].stats_buf)->num_col_sum;
      }
    }

    if (private->RSCS_stats_ready) {
      is_thread_msg_t *msg;

      private->is_info.timestamp = stats_event->timestamp;
      msg = (is_thread_msg_t *)malloc(sizeof(is_thread_msg_t));
      if (msg != NULL ) {
        memset(msg, 0, sizeof(is_thread_msg_t));
        msg->type = MSG_IS_PROCESS;
        msg->u.is_process_parm.type = IS_PROCESS_RS_CS_STATS;
        msg->u.is_process_parm.u.stats_data.run_is = 1;
        msg->u.is_process_parm.u.stats_data.frame_id = stats_event->frame_id;
        msg->u.is_process_parm.u.stats_data.identity = event->identity;
        msg->u.is_process_parm.u.stats_data.port = port;
        msg->u.is_process_parm.u.stats_data.is_info = &private->is_info;
        memcpy(&msg->u.is_process_parm.u.stats_data.rs_cs_data.rs_stats,
          stats_event->stats_data[MSM_ISP_STATS_RS].stats_buf,
          sizeof(q3a_rs_stats_t));
        memcpy(&msg->u.is_process_parm.u.stats_data.rs_cs_data.cs_stats,
          stats_event->stats_data[MSM_ISP_STATS_CS].stats_buf,
          sizeof(q3a_cs_stats_t));
        is_thread_en_q_msg(private->thread_data, msg);
      } else {
        CDBG_ERROR("%s: malloc failed!", __func__);
        rc = FALSE;
      }
      private->RSCS_stats_ready = FALSE;
    }
  } else if (private->stream_type == CAM_STREAM_TYPE_VIDEO) {
    if ((stats_event->stats_mask & (1 << MSM_ISP_STATS_RS)) &&
        (stats_event->stats_mask & (1 << MSM_ISP_STATS_CS))) {
      private->RSCS_stats_ready = TRUE;
    }

    if (private->RSCS_stats_ready) {
      is_thread_msg_t *msg = (is_thread_msg_t *)
        malloc(sizeof(is_thread_msg_t));

      if (msg != NULL ) {
        memset(msg, 0, sizeof(is_thread_msg_t));
        msg->type = MSG_IS_PROCESS;
        msg->u.is_process_parm.type = IS_PROCESS_RS_CS_STATS;
        msg->u.is_process_parm.u.stats_data.run_is = 0;
        msg->u.is_process_parm.u.stats_data.frame_id = stats_event->frame_id;
        msg->u.is_process_parm.u.stats_data.identity = event->identity;
        msg->u.is_process_parm.u.stats_data.port = port;
        msg->u.is_process_parm.u.stats_data.is_info = &private->is_info;
        is_thread_en_q_msg(private->thread_data, msg);
      } else {
        CDBG_ERROR("%s: malloc failed!", __func__);
        rc = FALSE;
      }
      private->RSCS_stats_ready = FALSE;
    }
  }
  return rc;
}


/** is_port_handle_gyro_stats_event:
 *    @port: IS port
 *    @event: event
 *
 *  Returns TRUE if event was successfully handled.
 **/
static boolean is_port_handle_gyro_stats_event(mct_port_t *port,
  mct_event_t *event)
{
  boolean rc = TRUE;
  is_port_private_t *private = (is_port_private_t *)port->port_private;
  mct_event_gyro_stats_t *gyro_stats_event =
    (mct_event_gyro_stats_t *)event->u.module_event.module_event_data;

  is_thread_msg_t *msg = (is_thread_msg_t *)
    malloc(sizeof(is_thread_msg_t));

  CDBG("%s: gyro frame id = %u", __func__, gyro_stats_event->is_gyro_data.frame_id);
  if (msg != NULL ) {
    memset(msg, 0, sizeof(is_thread_msg_t));
    msg->type = MSG_IS_PROCESS;
    msg->u.is_process_parm.type = IS_PROCESS_GYRO_STATS;
    msg->u.is_process_parm.u.gyro_data.frame_id =
      gyro_stats_event->is_gyro_data.frame_id;
    msg->u.is_process_parm.u.gyro_data.is_info = &private->is_info;
    memcpy(&msg->u.is_process_parm.u.gyro_data.gyro_data,
      &gyro_stats_event->is_gyro_data, sizeof(mct_event_gyro_data_t));
    is_thread_en_q_msg(private->thread_data, msg);
  } else {
    CDBG_ERROR("%s: malloc failed!", __func__);
    rc = FALSE;
  }

  return rc;
}


/** is_port_handle_dis_config_event:
 *    @private: private port data
 *    @mod_evt: module event
 **/
static boolean is_port_handle_dis_config_event(is_port_private_t *private,
  mct_event_module_t *mod_event)
{
  boolean rc = TRUE;
  isp_dis_config_info_t *dis_config;

  dis_config = (isp_dis_config_info_t *)mod_event->module_event_data;
  CDBG_HIGH("%s: MCT_EVENT_MODULE_ISP_DIS_CONFIG, sid = %u, strid = %x, "
    "vid = %x, col_num = %u, row_num = %u, w = %u, h = %u", __func__,
    dis_config->session_id, dis_config->stream_id, private->video_reserved_id,
    private->reserved_id, dis_config->col_num, dis_config->row_num,
    dis_config->width, dis_config->height);

  is_thread_msg_t *msg = (is_thread_msg_t *)
    malloc(sizeof(is_thread_msg_t));

  if (msg != NULL ) {
    memset(msg, 0, sizeof(is_thread_msg_t));
    msg->type = MSG_IS_SET;
    msg->u.is_set_parm.type = IS_SET_PARAM_DIS_CONFIG;
    msg->u.is_set_parm.u.is_config_info.width = dis_config->width;
    msg->u.is_set_parm.u.is_config_info.height = dis_config->height;
    is_thread_en_q_msg(private->thread_data, msg);
  } else {
    CDBG_ERROR("%s: malloc failed!", __func__);
    rc = FALSE;
  }

  return rc;
}


/** is_port_handle_output_dim_event:
 *    @private: private port data
 *    @mod_evt: module event
 **/
static boolean is_port_handle_output_dim_event(is_port_private_t *private,
  mct_event_module_t *mod_event)
{
  boolean rc = TRUE;
  mct_stream_info_t *stream_info = NULL;

  stream_info = (mct_stream_info_t *)mod_event->module_event_data;
  CDBG_HIGH("%s: MCT_EVENT_MODULE_ISP_OUTPUT_DIM, steam_type = %d, w = %d, "
    "h = %d, IS mode = %d", __func__, stream_info->stream_type,
    stream_info->dim.width, stream_info->dim.height, stream_info->is_type);

  is_thread_msg_t *msg = (is_thread_msg_t *)
    malloc(sizeof(is_thread_msg_t));

  if (msg != NULL ) {
    memset(msg, 0, sizeof(is_thread_msg_t));
    msg->type = MSG_IS_SET;
    msg->u.is_set_parm.type = IS_SET_PARAM_OUTPUT_DIM;
    msg->u.is_set_parm.u.is_output_dim_info.is_mode = stream_info->is_type;
    msg->u.is_set_parm.u.is_output_dim_info.vfe_width = stream_info->dim.width;
    msg->u.is_set_parm.u.is_output_dim_info.vfe_height =
      stream_info->dim.height;
    is_thread_en_q_msg(private->thread_data, msg);
  } else {
    CDBG_ERROR("%s: malloc failed!", __func__);
    rc = FALSE;
  }

  return rc;
}


/** is_port_send_is_update:
 *    @port: IS port
 *    @private: private port data
 **/
static void is_port_send_is_update(mct_port_t *port,
  is_port_private_t *private)
{
  mct_event_t is_update_event;
  is_update_t is_update;

  /* Sanity check */
  /* is_enabled is reset to 0 when IS initialization fails.  By checking this
     flag for 0, IS won't send DIS_UPDATE event when it is not operational. */
  if (private->is_output.x < 0 || private->is_output.y < 0 ||
    private->is_info.is_enabled == 0) {
    return;
  }

  is_update.id = private->video_reserved_id;
  is_update.x = private->is_output.x;
  is_update.y = private->is_output.y;
  is_update.width = private->is_info.is_width;
  is_update.height = private->is_info.is_height;
  is_update.frame_id = private->is_output.frame_id;
  if (private->is_info.is_mode == IS_TYPE_EIS_2_0) {
    is_update.use_3d = 1;
    memcpy(is_update.transform_matrix, private->is_output.transform_matrix,
      sizeof(is_update.transform_matrix));
    is_update.transform_type = private->is_info.transform_type;
    if (!private->video_stream_on) {
      is_update.width = private->is_info.preview_width;
      is_update.height = private->is_info.preview_height;
    }
  }
  else {
    is_update.use_3d = 0;
  }

  CDBG("%s: fid = %d, x = %d, y = %d, w = %d, h = %d, tt = %u, "
    "tm = %f %f %f %f %f %f %f %f %f", __func__,
    is_update.frame_id, is_update.x, is_update.y,
    is_update.width, is_update.height, is_update.transform_type,
    is_update.transform_matrix[0], is_update.transform_matrix[1],
    is_update.transform_matrix[2], is_update.transform_matrix[3],
    is_update.transform_matrix[4], is_update.transform_matrix[5],
    is_update.transform_matrix[6], is_update.transform_matrix[7],
    is_update.transform_matrix[8]);

  is_update_event.type = MCT_EVENT_MODULE_EVENT;
  is_update_event.identity = private->video_reserved_id;
  is_update_event.direction = MCT_EVENT_UPSTREAM;
  is_update_event.u.module_event.type = MCT_EVENT_MODULE_STATS_DIS_UPDATE;
  is_update_event.u.module_event.module_event_data = &is_update;
  mct_port_send_event_to_peer(port, &is_update_event);
}


/** is_port_callback:
 *    @port: IS port
 *    @output: Output from processing IS event
 **/
static void is_port_callback(mct_port_t *port,
  is_process_output_t *output)
{
  is_port_private_t *private = port->port_private;

  CDBG("%s: IS process ouput type = %d", __func__, output->type);
  switch (output->type) {
  case IS_PROCESS_OUTPUT_RS_CS_STATS:
    is_port_send_is_update(port, port->port_private);
    break;

  case IS_PROCESS_OUTPUT_GYRO_STATS:
    is_port_send_is_update(port, port->port_private);
    break;

  case IS_PROCESS_OUTPUT_STREAM_EVENT:
    if (output->is_stream_event == IS_VIDEO_STREAM_ON) {
      private->video_stream_on = 1;
    } else if (output->is_stream_event == IS_VIDEO_STREAM_OFF) {
      private->video_stream_on = 0;
    }
    if (private->video_stream_on == 0) {
      /* Default offsets to half margin for cropping at center during camcorder
         preview no recording. */
      if (private->is_info.is_mode != IS_TYPE_EIS_2_0) {
        private->is_output.x =
          (private->is_info.vfe_width - private->is_info.is_width) / 2;
        private->is_output.y =
          (private->is_info.vfe_height - private->is_info.is_height) / 2;
      } else {
        private->is_output.x = 0;
        private->is_output.y = 0;
      }

      private->is_output.transform_matrix[0] = 0.877193;
      private->is_output.transform_matrix[1] = 0.0;
      private->is_output.transform_matrix[2] = 0.0;
      private->is_output.transform_matrix[3] = 0.0;
      private->is_output.transform_matrix[4] = 0.877193;
      private->is_output.transform_matrix[5] = 0.0;
      private->is_output.transform_matrix[6] = 0.0;
      private->is_output.transform_matrix[7] = 0.0;
      private->is_output.transform_matrix[8] = 1.0;
    }
    break;

  default:
    break;
  }
}


/** is_port_event:
 *    @port: IS port
 *    @event: event
 *
 *  This function handles events for the IS port.
 *
 *  Returns TRUE on successful event processing.
 **/
static boolean is_port_event(mct_port_t *port, mct_event_t *event)
{
  boolean rc = TRUE;
  is_port_private_t *private;

  /* sanity check */
  if (!port || !event)
    return FALSE;

  private = (is_port_private_t *)(port->port_private);
  if (!private)
    return FALSE;

  /* sanity check: ensure event is meant for port with same identity*/
  if ((private->reserved_id & 0xFFFF0000) !=
      (event->identity & 0xFFFF0000))
  {
    return FALSE;
  }

  switch (MCT_EVENT_DIRECTION(event)) {
  case MCT_EVENT_DOWNSTREAM: {
    switch (event->type) {
    case MCT_EVENT_CONTROL_CMD: {
      mct_event_control_t *control = &event->u.ctrl_event;

      //CDBG("%s: Control event type %d", __func__, control->type);
      switch (control->type) {
      case MCT_EVENT_CONTROL_STREAMON:
        if (private->thread_data) {
          rc = is_port_handle_stream_event(private, event);
        }
        break;

      case MCT_EVENT_CONTROL_STREAMOFF:
        if (private->thread_data) {
          rc = is_port_handle_stream_event(private, event);
        }
        break;

      case MCT_EVENT_CONTROL_SET_PARM: {
        stats_set_params_type *stats_parm = control->control_event_data;
        if (private->thread_data &&
          stats_parm->param_type == STATS_SET_IS_PARAM &&
          stats_parm->u.is_param.type == IS_SET_PARAM_IS_ENABLE) {
            rc = is_port_handle_set_is_enable(private, control);
        }
      }
        break;

      default:
        break;
      }
    } /* case MCT_EVENT_CONTROL_CMD */
      break;

    case MCT_EVENT_MODULE_EVENT: {
      mct_event_module_t *mod_event = &event->u.module_event;

      switch (mod_event->type) {
      case MCT_EVENT_MODULE_STATS_DATA:
        if (private->is_info.is_enabled && (IS_VIDEO_STREAM_RUNNING)) {
          rc = is_port_handle_stats_event(port, event);
        }
        break;

      case MCT_EVENT_MODULE_STATS_GYRO_STATS:
        if (private->is_info.is_inited &&
          private->is_info.is_mode != IS_TYPE_DIS &&
          (IS_VIDEO_STREAM_RUNNING)) {
          rc = is_port_handle_gyro_stats_event(port, event);
        }
        break;

      case MCT_EVENT_MODULE_SET_STREAM_CONFIG: {
          rc = is_port_handle_stream_config_event(private, mod_event);
      }
        break;

      case MCT_EVENT_MODULE_ISP_DIS_CONFIG: {
        if (private->thread_data) {
          rc = is_port_handle_dis_config_event(private, mod_event);
        }
      }
        break;

      case MCT_EVENT_MODULE_MODE_CHANGE: {
        private->stream_type = ((stats_mode_change_event_data *)
          (mod_event->module_event_data))->stream_type;
      }
        break;

      case MCT_EVENT_MODULE_START_STOP_STATS_THREADS: {
        uint8_t *start_flag = (uint8_t*)(mod_event->module_event_data);
        CDBG("%s MCT_EVENT_MODULE_START_STOP_STATS_THREADS start_flag: %d",
          __func__,*start_flag);
        if (*start_flag) {
          if (is_port_start_thread(port) == FALSE) {
            CDBG_ERROR("%s: is thread start failed", __func__);
            rc = FALSE;
          }
        } else {
          if (private->thread_data) {
            is_thread_stop(private->thread_data);
          }
        }
      }
        break;

      case MCT_EVENT_MODULE_ISP_OUTPUT_DIM: {
        if (private->thread_data) {
          rc = is_port_handle_output_dim_event(private, mod_event);
        }
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


/** is_port_ext_link:
 *    @identity: session id | stream id
 *    @port: IS port
 *    @peer: For IS sink port, peer is most likely stats port
 *
 *  Sets IS port's external peer port.
 *
 *  Returns TRUE on success.
 **/
static boolean is_port_ext_link(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  boolean rc = FALSE, thread_init = FALSE;
  is_port_private_t *private;
  mct_event_t event;

  CDBG("%s: Enter", __func__);
  if (strcmp(MCT_OBJECT_NAME(port), "is_sink"))
    return FALSE;

  private = (is_port_private_t *)port->port_private;
  if (!private)
    return FALSE;

  MCT_OBJECT_LOCK(port);
  switch (private->state) {
  case IS_PORT_STATE_RESERVED:
    CDBG("%s: IS_PORT_STATE_RESERVED", __func__);
    if ((private->reserved_id & 0xFFFF0000) != (identity & 0xFFFF0000)) {
      break;
    }
  /* Fall through */
  case IS_PORT_STATE_UNLINKED:
    CDBG("%s: IS_PORT_STATE_UNLINKED", __func__);
    if ((private->reserved_id & 0xFFFF0000) != (identity & 0xFFFF0000)) {
      break;
    }

  case IS_PORT_STATE_CREATED:
    if ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000)) {
      thread_init = TRUE;
    }
    rc = TRUE;
    break;

  case IS_PORT_STATE_LINKED:
    CDBG("%s: IS_PORT_STATE_LINKED", __func__);
    if ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000)) {
      rc = TRUE;
    }
    break;

  default:
    break;
  }

  if (rc == TRUE) {
    /* If IS module requires a thread and the port state above warrants one,
       create the thread here */
    if (thread_init == TRUE) {
      if (is_port_init_thread(port) == FALSE) {
        rc = FALSE;
        goto init_thread_fail;
      }
    }
    private->state = IS_PORT_STATE_LINKED;
    MCT_PORT_PEER(port) = peer;
    MCT_OBJECT_REFCOUNT(port) += 1;
  }

init_thread_fail:
  MCT_OBJECT_UNLOCK(port);
  CDBG("%s: rc=%d", __func__, rc);
  return rc;
}


/** is_port_unlink:
 *  @identity: session id | stream id
 *  @port: IS port
 *  @peer: IS port's peer port (probably stats port)
 *
 *  This funtion unlinks the IS port from its peer.
 **/
static void is_port_unlink(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  is_port_private_t *private;

  if (!port || !peer || MCT_PORT_PEER(port) != peer)
    return;

  private = (is_port_private_t *)port->port_private;
  if (!private)
    return;

  CDBG("%s: port state = %d, identity = 0x%x", __func__,
    private->state, identity);
  MCT_OBJECT_LOCK(port);
  if (private->state == IS_PORT_STATE_LINKED &&
      (private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000)) {
    MCT_OBJECT_REFCOUNT(port) -= 1;
    if (!MCT_OBJECT_REFCOUNT(port)) {
      private->state = IS_PORT_STATE_UNLINKED;
      CDBG("%s: Stop IS thread, video_reserved_id = %x", __func__,
        private->video_reserved_id);
      is_thread_deinit(private->thread_data);
      private->thread_data = NULL;
      MCT_PORT_PEER(port) = NULL;
    }
  }
  MCT_OBJECT_UNLOCK(port);

  return;
}


/** is_port_set_caps:
 *    @port: port object whose caps are to be set
 *    @caps: this port's capability.
 *
 *  Function overwrites a ports capability.
 *
 *  Returns TRUE if it is valid source port.
 **/
static boolean is_port_set_caps(mct_port_t *port,
  mct_port_caps_t *caps)
{
  CDBG("%s: Enter", __func__);
  if (strcmp(MCT_PORT_NAME(port), "is_sink"))
    return FALSE;

  port->caps = *caps;
  return TRUE;
}


/** is_port_check_caps_reserve:
 *    @port: this interface module's port
 *    @peer_caps: the capability of peer port which wants to match
 *                interface port
 *    @stream_info: stream information
 *
 *  Returns TRUE on success.
 **/
static boolean is_port_check_caps_reserve(mct_port_t *port, void *caps,
  void *stream_info)
{
  boolean rc = FALSE;
  mct_port_caps_t *port_caps;
  is_port_private_t *private;
  mct_stream_info_t *strm_info = (mct_stream_info_t *)stream_info;

  CDBG("%s: Enter", __func__);
  MCT_OBJECT_LOCK(port);
  if (!port || !caps || !strm_info ||
      strcmp(MCT_OBJECT_NAME(port), "is_sink")) {
    CDBG("%s: Exit unsucessful", __func__);
    goto reserve_done;
  }

  port_caps = (mct_port_caps_t *)caps;
  if (port_caps->port_caps_type != MCT_PORT_CAPS_STATS) {
    rc = FALSE;
    goto reserve_done;
  }

  private = (is_port_private_t *)port->port_private;
  CDBG("%s: port state = %d, identity = 0x%x, stream_type = %d", __func__,
    private->state, strm_info->identity, strm_info->stream_type);
  switch (private->state) {
  case IS_PORT_STATE_LINKED:
  if ((private->reserved_id & 0xFFFF0000) ==
      (strm_info->identity & 0xFFFF0000)) {
    if (strm_info->stream_type == CAM_STREAM_TYPE_VIDEO) {
      private->video_reserved_id = strm_info->identity;
      CDBG("%s: video reserved_id = 0x%x", __func__, private->video_reserved_id);
      CDBG("%s:%d w = %lu, h = %lu", __func__, __LINE__, strm_info->dim.width,
        strm_info->dim.height);
    } else if (strm_info->stream_type == CAM_STREAM_TYPE_PREVIEW) {
      private->is_info.preview_width = strm_info->dim.width;
      private->is_info.preview_height = strm_info->dim.height;
      private->reserved_id = strm_info->identity;
      CDBG("%s:%d w = %lu, h = %lu", __func__, __LINE__,
        private->is_info.preview_width, private->is_info.preview_height);
    }
    rc = TRUE;
  }
  break;

  case IS_PORT_STATE_CREATED:
  case IS_PORT_STATE_UNRESERVED:
    if (strm_info->stream_type == CAM_STREAM_TYPE_VIDEO) {
      private->video_reserved_id = strm_info->identity;
      CDBG("%s: reserved_id = 0x%x", __func__, private->video_reserved_id);
      CDBG("%s:%d w = %lu, h = %lu", __func__, __LINE__, strm_info->dim.width,
        strm_info->dim.height);
    }
    private->reserved_id = strm_info->identity;
    private->stream_type = strm_info->stream_type;
    private->state       = IS_PORT_STATE_RESERVED;
    rc = TRUE;
    break;

  case IS_PORT_STATE_RESERVED:
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


/** is_port_check_caps_unreserve:
 *    @port: this port object to remove the session/stream
 *    @identity: session+stream identity
 *
 *  This function frees the identity from port's children list.
 *
 *  Returns FALSE if the identity does not exist.
 **/
static boolean is_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  boolean rc = FALSE;
  is_port_private_t *private;

  if (!port || strcmp(MCT_OBJECT_NAME(port), "is_sink"))
    return FALSE;

  CDBG("%s: E, identity = 0x%x", __func__, identity);
  private = (is_port_private_t *)port->port_private;
  if (!private)
    return FALSE;

  CDBG("%s: port state = %d, identity = 0x%x", __func__,
    private->state, identity);
  if (private->state == IS_PORT_STATE_UNRESERVED)
    return TRUE;

  MCT_OBJECT_LOCK(port);
  if ((private->state == IS_PORT_STATE_UNLINKED ||
       private->state == IS_PORT_STATE_RESERVED) &&
      ((private->reserved_id & 0xFFFF0000) == (identity & 0xFFFF0000))) {

    if (!MCT_OBJECT_REFCOUNT(port)) {
      private->state = IS_PORT_STATE_UNRESERVED;
      private->video_reserved_id = (private->video_reserved_id & 0xFFFF0000);
      private->reserved_id = (private->reserved_id & 0xFFFF0000);
    }
    rc = TRUE;
  }

unreserve_done:
  MCT_OBJECT_UNLOCK(port);
  return rc;
}


/** is_port_init:
 *    @port: IS port
 *    @session_id: session id
 *
 *  This function initializes the IS port's internal variables.
 *
 *  Returns TRUE on success.
 **/
boolean is_port_init(mct_port_t *port, unsigned int session_id)
{
  mct_port_caps_t caps;
  is_port_private_t *private;

  if (port == NULL || strcmp(MCT_OBJECT_NAME(port), "is_sink"))
    return FALSE;

  private = (void *)malloc(sizeof(is_port_private_t));
  if (!private)
    return FALSE;

  memset(private, 0, sizeof(is_port_private_t));
  private->set_parameters = is_set_parameters;
  private->process = is_process;
  private->callback = is_port_callback;
  private->is_process_output.is_output = &private->is_output;
  private->video_reserved_id = session_id;
  private->reserved_id = session_id;
  private->state = IS_PORT_STATE_CREATED;
  private->is_info.transform_type = C2D_LENSCORRECT_PERSPECTIVE |
    C2D_LENSCORRECT_BILINEAR | C2D_LENSCORRECT_ORIGIN_IN_MIDDLE |
    C2D_LENSCORRECT_SOURCE_RECT;
  port->port_private = private;
  port->direction = MCT_PORT_SINK;
  caps.port_caps_type = MCT_PORT_CAPS_STATS;
  caps.u.stats.flag   = MCT_PORT_CAP_STATS_CS_RS;

  mct_port_set_event_func(port, is_port_event);
  /* Accept default int_link function */
  mct_port_set_ext_link_func(port, is_port_ext_link);
  mct_port_set_unlink_func(port, is_port_unlink);
  mct_port_set_set_caps_func(port, is_port_set_caps);
  mct_port_set_check_caps_reserve_func(port, is_port_check_caps_reserve);
  mct_port_set_check_caps_unreserve_func(port, is_port_check_caps_unreserve);

  if (port->set_caps) {
    port->set_caps(port, &caps);
  }
  return TRUE;
}


/** is_port_deinit:
 *    @port: IS port
 *
 * This function frees the IS port's memory.
 **/
void is_port_deinit(mct_port_t *port)
{
  if (!port || strcmp(MCT_OBJECT_NAME(port), "is_sink"))
    return;

  if (port->port_private)
    free(port->port_private);
}


/** is_port_find_identity:
 *    @port: IS port
 *    @identity: session id | stream id
 *
 * This function checks for the port with a given session.
 *
 * Returns TRUE if the port is found.
 **/
boolean is_port_find_identity(mct_port_t *port, unsigned int identity)
{
  is_port_private_t *private;

  if (!port)
      return FALSE;

  if (strcmp(MCT_OBJECT_NAME(port), "is_sink"))
    return FALSE;

  private = port->port_private;

  if (private) {
    return ((private->reserved_id & 0xFFFF0000) ==
            (identity & 0xFFFF0000) ? TRUE : FALSE);
  }

  return FALSE;
}

