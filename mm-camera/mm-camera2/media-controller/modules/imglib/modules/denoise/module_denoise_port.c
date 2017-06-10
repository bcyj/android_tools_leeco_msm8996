/*============================================================================
Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include "module_denoise_dbg.h"
#include "module_imglib_common.h"
#include "module_denoise_port.h"
#include "module_denoise_lib.h"
#include <media/msmb_generic_buf_mgr.h>
#include "modules.h"
#include "server_debug.h"

static boolean module_denoise_port_set_chromatix_ptr_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event);
static boolean module_denoise_port_stats_aec_update_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event);
static boolean module_denoise_port_buf_divert_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event);
static boolean module_denoise_port_isp_awb_update_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event);
static boolean module_denoise_port_isp_gamma_update_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event);

static boolean module_denoise_port_stream_on_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event);
static boolean module_denoise_port_stream_off_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event);
static boolean module_denoise_port_set_parm_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event);
static boolean module_denoise_port_parm_stream_buf_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event);

static boolean module_denoise_port_up_buf_divert_ack_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event);

/** module_denoise_port_state_t:
 *    @MODULE_DENOISE_PORT_STATE_CREATED: port state is just created
 *    @MODULE_DENOISE_PORT_STATE_RESERVED: port state is reserved
 *    @MODULE_DENOISE_PORT_STATE_LINKED: port state is linked to peer
 *    @MODULE_DENOISE_PORT_STATE_COUNT: port state count
 *
 *  This enum defines the state of the port
 **/
typedef enum {
  MODULE_DENOISE_PORT_STATE_CREATED,
  MODULE_DENOISE_PORT_STATE_RESERVED,
  MODULE_DENOISE_PORT_STATE_LINKED,
  MODULE_DENOISE_PORT_STATE_COUNT
} module_denoise_port_state_t;

/** module_denoise_port_t:
 *    @reserved_identity: session identity associated with this port
 *    @state: state of the port
 *    @lib_instance: library instance
 *    @out_buff: output buffer handler
 *    @in_buff: input buffer handler
 *    @metadata_buff: metadata buffer handler
 *    @session_meta: metadata session data
 *    @cam_denoise_param: denoise configuration parameters
 *    @config_list: list with current frames' configurations
 *
 *  This structure defines denoise module port
 **/
typedef struct {
  uint32_t reserved_identity;
  module_denoise_port_state_t state;
  void* lib_instance;
  module_denoise_buf_t* out_buff;
  module_denoise_buf_t* in_buff;
  void* metadata_buff;
  mct_stream_session_metadata_info session_meta;
  cam_denoise_param_t cam_denoise_param;
  mct_list_t *config_list;
  pthread_mutex_t mutex_config_list;
} module_denoise_port_t;

/** module_denoise_frame_config_t:
 *    @src_port_linked: flag indicating whether src port is linked
 *    @out_buff: output buffer handler
 *    @in_buff: input buffer handler
 *
 *  This structure defines denoise configuration for particular frame
 **/
typedef struct {
  boolean src_port_linked;
  module_denoise_buf_t* out_buff;
  module_denoise_buf_t* in_buff;
} module_denoise_frame_config_t;

/** module_denoise_port_event_func:
 *  @port: the Port to handle the event
 *  @event: the Event to be hanled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle port event
 *
 * Returns TRUE in case of success
 **/
typedef boolean (*module_denoise_port_event_func)(mct_port_t *port,
  mct_event_t *event, boolean *forward_event);

/** module_denoise_port_event_lut_entry_t:
 *    @type: binded command
 *    @event_handler: binded event handler
 *
 *  This structure defines entry in the denoise module LUT for event handlers
 **/
typedef struct {
  union {
    uint32_t type;
    mct_event_module_type_t event_type;
    mct_event_control_type_t cmd_type;
  };
  module_denoise_port_event_func event_handler;
} module_denoise_port_event_lut_entry_t;

/** module_denoise_port_event_lut:
 *
 *  This array defines denoise module LUT for event handlers
 **/
static module_denoise_port_event_lut_entry_t module_denoise_port_event_lut[] = {
  {{MCT_EVENT_MODULE_SET_CHROMATIX_PTR},
    module_denoise_port_set_chromatix_ptr_event},
  {{MCT_EVENT_MODULE_STATS_AEC_UPDATE},
    module_denoise_port_stats_aec_update_event},
  {{MCT_EVENT_MODULE_BUF_DIVERT},
    module_denoise_port_buf_divert_event},
  {{MCT_EVENT_MODULE_ISP_AWB_UPDATE},
    module_denoise_port_isp_awb_update_event},
  {{MCT_EVENT_MODULE_ISP_GAMMA_UPDATE},
    module_denoise_port_isp_gamma_update_event},
  // Dummy entry. It needs to be always at the end of the LUT
  {{MCT_EVENT_MODULE_MAX}, NULL}
};

/** module_denoise_port_cmd_lut:
 *
 *  This array defines denoise module LUT for command handlers
 **/
static module_denoise_port_event_lut_entry_t module_denoise_port_cmd_lut[] = {
  {{MCT_EVENT_CONTROL_STREAMON}, module_denoise_port_stream_on_event},
  {{MCT_EVENT_CONTROL_STREAMOFF}, module_denoise_port_stream_off_event},
  {{MCT_EVENT_CONTROL_SET_PARM}, module_denoise_port_set_parm_event},
  {{MCT_EVENT_CONTROL_PARM_STREAM_BUF},
    module_denoise_port_parm_stream_buf_event},
  // Dummy entry. It needs to be always at the end of the LUT
  {{MCT_EVENT_CONTROL_MAX}, NULL}
};

/** module_denoise_port_up_lut:
 *
 *  This array defines denoise module LUT for upstream event handlers
 **/
static module_denoise_port_event_lut_entry_t module_denoise_port_up_lut[] = {
  {{MCT_EVENT_MODULE_BUF_DIVERT_ACK},
    module_denoise_port_up_buf_divert_ack_event},
  // Dummy entry. It needs to be always at the end of the LUT
  {{MCT_EVENT_MODULE_MAX}, NULL}
};

/** module_denoise_port_validate_port_session_id
 *    @data1: mct_port_t object
 *    @data2: identity to be checked
 *
 *  Checks if this port has already been assigned to specified session id
 *
*  Return TRUE if the port has the same session id
 **/
boolean module_denoise_port_validate_port_session_id(void *data1,
  void *data2)
{
  boolean ret_val = FALSE;
  mct_port_t *port = data1;
  uint32_t *session_id = data2;
  module_denoise_port_t *private;

  IDBG_MED("%s +", __func__);

  if (port && session_id) {
    if (MODULE_DENOISE_VALIDATE_NAME(port)) {
      private = port->port_private;
      if ((private)
        && (IMGLIB_SESSIONID(private->reserved_identity) == *session_id))
          ret_val = TRUE;
    }
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_validate_linked_port_identity
 *    @data1: mct_port_t object
 *    @data2: identity to be checked
 *
 *  Checks if this port has already been assigned to specified identity
 *    and the port is linked
 *
*  Return TRUE if the port has the same identity and is linked
 **/
static boolean module_denoise_port_validate_linked_port_identity(
  mct_port_t *port, uint32_t identity)
{
  boolean ret_val = FALSE;
  module_denoise_port_t *private;

  IDBG_MED("%s +", __func__);

  if (port) {
    if (MODULE_DENOISE_VALIDATE_NAME(port)) {
      private = port->port_private;
      if ((private)
        && (private->reserved_identity == identity)
        && (MODULE_DENOISE_PORT_STATE_LINKED == private->state))
          ret_val = TRUE;
    }
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_check_linked_port_identity
 *    @data1: mct_port_t object
 *    @data2: identity to be checked
 *
 *  Checks if this port has already been assigned to specified identity
 *    and the port is linked
 *
*  Return TRUE if the port has the same identity and is linked
 **/
boolean module_denoise_port_check_linked_port_identity(void *data1,
  void *data2)
{
  boolean ret_val = FALSE;
  mct_port_t *port = (mct_port_t *)data1;
  uint32_t *id = (uint32_t *)data2;

  IDBG_MED("%s +", __func__);

  if (id)
    ret_val = module_denoise_port_validate_linked_port_identity(port, *id);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_find_event_handler
 *    @id: event id
 *    @denoise_handlers_lut: denoise handlers lut
 *
 * Returns event handler binded to corresponded event id
 *
 * Returns event handler or null (if event handler is not found)
 **/
static void* module_denoise_port_find_event_handler(uint32_t id,
  module_denoise_port_event_lut_entry_t denoise_handlers_lut[])
{
  void* ret_val = NULL;
  uint32_t i;

  IDBG_MED("%s +", __func__);

  for (i=0; denoise_handlers_lut[i].event_handler; i++) {
    if (denoise_handlers_lut[i].type == id) {
      ret_val = denoise_handlers_lut[i].event_handler;
      break;
    }
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_upstream_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be hanled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle an upstream event for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_upstream_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  module_denoise_port_event_func event_handler;

  IDBG_MED("%s +", __func__);

  if (port && event) {
    mct_event_module_t *mct_event_module = &event->u.module_event;

    event_handler =
      module_denoise_port_find_event_handler(mct_event_module->type,
        module_denoise_port_up_lut);
    if (event_handler)
      ret_val = event_handler(port, event, forward_event);
    else
      ret_val = TRUE;
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_downstream_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be hanled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle an event for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_downstream_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  module_denoise_port_event_func event_handler;

  IDBG_MED("%s +", __func__);

  if (port && event) {
    mct_event_module_t *mct_event_module = &event->u.module_event;

    event_handler =
      module_denoise_port_find_event_handler(mct_event_module->type,
        module_denoise_port_event_lut);
    if (event_handler)
      ret_val = event_handler(port, event, forward_event);
    else
      ret_val = TRUE;
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_set_chromatix_ptr_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be hanled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle set chromatix ptr event for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_set_chromatix_ptr_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  module_denoise_port_t *private;

  IDBG_MED("%s +", __func__);

  if (port && event && port->port_private
    && event->u.module_event.module_event_data) {
      private = port->port_private;

      memcpy(&private->session_meta.sensor_data,
        event->u.module_event.module_event_data,
        sizeof(private->session_meta.sensor_data));

    ret_val = TRUE;
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_stats_aec_update_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be hanled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle stats update event for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_stats_aec_update_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  module_denoise_port_t *private;
  stats_update_t *stats_update;
  stats_get_data_t* stats_get_data;

  IDBG_MED("%s +", __func__);

  if (port && event && port->port_private
    && event->u.module_event.module_event_data) {

      private = port->port_private;
      stats_update = event->u.module_event.module_event_data;
      stats_get_data =
        (stats_get_data_t*)&private->session_meta.stats_aec_data.private_data;

      if (stats_update->flag == STATS_UPDATE_AEC) {
        stats_get_data->aec_get.valid_entries = 1;
        stats_get_data->aec_get.real_gain[0] =
          stats_update->aec_update.real_gain;
        stats_get_data->aec_get.lux_idx = stats_update->aec_update.lux_idx;
        stats_get_data->flag = stats_update->flag;
      }

      ret_val = TRUE;
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_isp_awb_update_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be hanled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle AWB update event for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_isp_awb_update_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  module_denoise_port_t *private;
  awb_update_t* awb_update;
  mct_stream_session_metadata_info* session_meta;

  IDBG_MED("%s +", __func__);

  if (port && event && port->port_private
    && event->u.module_event.module_event_data) {

      private = port->port_private;
      awb_update = event->u.module_event.module_event_data;
      session_meta = &private->session_meta;

      if (sizeof(session_meta->isp_stats_awb_data.private_data)
        >= sizeof(awb_update_t)) {

          memcpy(&session_meta->isp_stats_awb_data.private_data,
            awb_update,
            sizeof(awb_update_t));
          ret_val = TRUE;

      } else {
        CDBG_ERROR("%s: Private data size need to be increased\n", __func__);
        CDBG_ERROR("session_meta->isp_stats_awb_data.private_data size %d\n",
          sizeof(session_meta->isp_stats_awb_data.private_data));
        CDBG_ERROR("awb_update_t size %d\n", sizeof(awb_update_t));
      }
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_isp_gamma_update_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be hanled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle Gamma update event for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_isp_gamma_update_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  module_denoise_port_t *private;
  void *gamma;
  uint32_t gamma_size = sizeof(img_gamma_t);
  mct_stream_session_metadata_info* session_meta;

  IDBG_MED("%s +", __func__);

  if (port && event && port->port_private
    && event->u.module_event.module_event_data) {

      private = port->port_private;
      gamma = event->u.module_event.module_event_data;
      session_meta = &private->session_meta;

      if (sizeof(session_meta->isp_gamma_data.private_data)
        >= gamma_size) {

          memcpy(&session_meta->isp_gamma_data.private_data,
            gamma,
            gamma_size);
          ret_val = TRUE;

      } else {
        CDBG_ERROR("%s: Private data size need to be increased\n", __func__);
        CDBG_ERROR("session_meta->isp_gamma_data.private_data size %d\n",
          sizeof(session_meta->isp_gamma_data.private_data));
        CDBG_ERROR("gamma_size size %d\n", gamma_size);
      }
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_stream_on_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be hanled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle stream on command for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_stream_on_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  IDBG_MED("%s +", __func__);

  // Do nothing

  IDBG_MED("%s -", __func__);

  return TRUE;
}

/** module_denoise_port_stream_off_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be hanled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle stream off command for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_stream_off_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  module_denoise_port_t *private;

  IDBG_MED("%s +", __func__);

  if (port && event && port->port_private) {
    private = port->port_private;

    ret_val = module_denoise_lib_abort(private->lib_instance);

    if (pthread_mutex_lock(&private->mutex_config_list))
      CDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);

    mct_list_free_list(private->config_list);

    if (pthread_mutex_unlock(&private->mutex_config_list))
      CDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_set_parm_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be hanled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle set param command for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_set_parm_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  module_denoise_port_t *private;
  mct_event_control_parm_t *event_parm;

  IDBG_MED("%s +", __func__);

  if (port && event && port->port_private
    && event->u.ctrl_event.control_event_data) {
      private = port->port_private;
      event_parm = event->u.ctrl_event.control_event_data;

      if (CAM_INTF_PARM_WAVELET_DENOISE == event_parm->type)
      {
        memcpy(&private->cam_denoise_param,
          event_parm->parm_data,
          sizeof(private->cam_denoise_param));
      }
      ret_val = TRUE;
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_fill_divert_buffer_handler:
 *  @img_frame: image buffer handler descriptor
 *  @buf_divert: buf divert message
 *  @info: stream info configuration
 *
 * Function to fill image buffer handler descriptor
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_fill_divert_buffer_handler(
  img_frame_t* img_frame, isp_buf_divert_t *buf_divert, mct_stream_info_t* info)
{
  boolean ret_val = FALSE;
  uint8_t *ptr;
  struct v4l2_buffer* buffer;
  int i;

  IDBG_MED("%s +", __func__);

  buffer = &buf_divert->buffer;
  if ((CAM_FORMAT_YUV_420_NV21 == info->fmt) ||
    (CAM_FORMAT_YUV_422_NV61 == info->fmt)) {

      img_frame->frame[0].plane_cnt = info->buf_planes.plane_info.num_planes;
      ptr = buf_divert->vaddr;
      for (i=0; i<img_frame->frame[0].plane_cnt; i++) {
        img_frame->frame[0].plane[i].plane_type = i;
        img_frame->frame[0].plane[i].stride =
          info->buf_planes.plane_info.mp[i].stride;
        img_frame->frame[0].plane[i].fd = buf_divert->fd;
        img_frame->frame[0].plane[i].height = info->dim.height;
        if (CAM_FORMAT_YUV_420_NV21 == info->fmt)
          img_frame->frame[0].plane[i].height /= (i + 1);
        img_frame->frame[0].plane[i].width = info->dim.width;
        img_frame->frame[0].plane[i].length =
          img_frame->frame[0].plane[i].height
          * img_frame->frame[0].plane[i].stride;
        img_frame->frame[0].plane[i].offset =
          info->buf_planes.plane_info.mp[i].offset;
        ptr += img_frame->frame[0].plane[i].offset;
        if (i)
          ptr += img_frame->frame[0].plane[1].stride * info->dim.height;
        img_frame->frame[0].plane[i].addr = ptr;
      }

      img_frame->timestamp =
        buf_divert->buffer.timestamp.tv_sec * 1000000
        + buf_divert->buffer.timestamp.tv_usec;

      img_frame->frame_cnt = 1;
      img_frame->info.width = info->dim.width;
      img_frame->info.height = info->dim.height;
      if (CAM_FORMAT_YUV_420_NV21 == info->fmt)
        img_frame->info.ss = IMG_H2V2;
      else
        img_frame->info.ss = IMG_H2V1;
      img_frame->info.analysis = 0;
      img_frame->idx = buf_divert->buffer.index;

      ret_val = TRUE;
  } else
    CDBG_ERROR("Only NV21 and NV61 formats are supported in denoise lib");

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_fill_buffer_handler:
 *  @img_frame: image buffer handler descriptor
 *  @mct_stream_map_buf: mct stream map buffer
 *  @info: stream info configuration
 *
 * Function to fill image buffer handler descriptor
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_fill_buffer_handler(img_frame_t* img_frame,
  mct_stream_map_buf_t *mct_stream_map_buf, mct_stream_info_t* info)
{
  boolean ret_val = FALSE;
  struct timeval timestamp;
  uint32_t i;

  IDBG_MED("%s +", __func__);

  if ((CAM_FORMAT_YUV_420_NV21 == info->fmt) ||
    (CAM_FORMAT_YUV_422_NV61 == info->fmt)) {
      img_frame->frame[0].plane_cnt = mct_stream_map_buf->num_planes;
      for (i=0; i<mct_stream_map_buf->num_planes; i++) {
        img_frame->frame[0].plane[i].plane_type = i;
        img_frame->frame[0].plane[i].addr =
          mct_stream_map_buf->buf_planes[i].buf;
        img_frame->frame[0].plane[i].stride =
          mct_stream_map_buf->buf_planes[i].stride;
        img_frame->frame[0].plane[i].fd = mct_stream_map_buf->buf_planes[i].fd;
        img_frame->frame[0].plane[i].height = info->dim.height;
        if (CAM_FORMAT_YUV_420_NV21 == info->fmt)
          img_frame->frame[0].plane[i].height /= (i + 1);
        img_frame->frame[0].plane[i].width = info->dim.width;
        img_frame->frame[0].plane[i].length =
          img_frame->frame[0].plane[i].height
          * img_frame->frame[0].plane[i].stride;
        img_frame->frame[0].plane[i].offset =
          mct_stream_map_buf->buf_planes[i].offset;
      }

      gettimeofday(&timestamp, NULL);
      img_frame->timestamp = timestamp.tv_sec * 1000000 + timestamp.tv_usec;

      img_frame->frame_cnt = 1;
      img_frame->info.width = info->dim.width;
      img_frame->info.height = info->dim.height;
      if (CAM_FORMAT_YUV_420_NV21 == info->fmt)
        img_frame->info.ss = IMG_H2V2;
      else
        img_frame->info.ss = IMG_H2V1;
      img_frame->info.analysis = 0;
      img_frame->idx = mct_stream_map_buf->buf_index;

      ret_val = TRUE;
  } else
    CDBG_ERROR("Only NV21 and NV61 formats are supported in denoise lib");

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_get_bfr_mngr_subdev:
 *  @buf_mgr_fd: buffer manager file descriptor
 *
 * Function to get buffer manager file descriptor
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_get_bfr_mngr_subdev(int *buf_mgr_fd)
{
  struct media_device_info mdev_info;
  int32_t num_media_devices = 0;
  char dev_name[32];
  char subdev_name[32];
  int32_t dev_fd = 0, ioctl_ret;
  boolean ret_val = FALSE;
  uint32_t i = 0;

  IDBG_MED("%s +", __func__);

  if (buf_mgr_fd) {
    *buf_mgr_fd = -1;

    while (1) {
      int32_t num_entities = 1;
      snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices);
      dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
      if (dev_fd >= MAX_FD_PER_PROCESS) {
        dump_list_of_daemon_fd();
        dev_fd = -1;
        break;
      }
      if (dev_fd < 0) {
        IDBG_MED("Enumerating media devices completed");
        break;
      }
      num_media_devices++;
      ioctl_ret = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
      if (ioctl_ret < 0) {
        IDBG_MED("Enumerating media devices completed");
        close(dev_fd);
        break;
      }

      if (strncmp(mdev_info.model, "msm_config",
        sizeof(mdev_info.model) != 0)) {
          close(dev_fd);
          continue;
      }

      while (1) {
        struct media_entity_desc entity;
        memset(&entity, 0, sizeof(entity));
        entity.id = num_entities++;

        ioctl_ret = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
        if (ioctl_ret < 0) {
          IDBG_MED("Enumerating media entities completed");
          break;
        }

        if (entity.type == MEDIA_ENT_T_V4L2_SUBDEV &&
            entity.group_id == MSM_CAMERA_SUBDEV_BUF_MNGR) {
          snprintf(subdev_name, sizeof(dev_name), "/dev/%s", entity.name);

          *buf_mgr_fd = open(subdev_name, O_RDWR);
          if ((*buf_mgr_fd) >= MAX_FD_PER_PROCESS) {
            dump_list_of_daemon_fd();
            *buf_mgr_fd = -1;
            continue;
          }
          if (*buf_mgr_fd < 0) {
            CDBG_ERROR("Open subdev %s failed", subdev_name);
            continue;
          }
          IDBG_MED("Open subdev %s success", subdev_name);
          ret_val = TRUE;
        }

        if (ret_val)
          break;
      }

      close(dev_fd);

      if (ret_val)
        break;
    }
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_find_buff:
 *  @list_data: buffer instance
 *  @user_data: required buffer index
 *
 * Function to get specified buffer
 *
 * Returns Handler to mapped buffer or NULL
 **/
static boolean module_denoise_port_find_buff(void *list_data,
  void *user_data)
{
  boolean ret_val = FALSE;
  mct_stream_map_buf_t *img_buf = list_data;
  int32_t *buff_index = user_data;

  IDBG_LOW("%s +", __func__);

  if (img_buf && buff_index) {
    if (*buff_index == (int32_t)img_buf->buf_index)
      ret_val = TRUE;
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_LOW("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_find_stream:
 *  @list_data: stream instance
 *  @user_data: required stream id
 *
 * Function to get pecified stream
 *
 * Returns Handler to mapped buffer or NULL
 **/
static boolean module_denoise_port_find_stream(void *list_data,
  void *user_data)
{
  boolean ret_val = FALSE;
  mct_stream_t *stream = (mct_stream_t *)list_data;
  uint32_t *stream_index = user_data;

  IDBG_LOW("%s +", __func__);

  if (stream && stream_index) {
    if (*stream_index == stream->streamid)
      ret_val = TRUE;
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_LOW("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_release_img_buffer:
 *  @img_buff: image buffer handler
 *
 * Function to release image buffer
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_release_img_buffer(
  module_denoise_buf_t *img_buff)
{
  IDBG_MED("%s +", __func__);

  if (img_buff) {
    if (img_buff->img_frame)
      free(img_buff->img_frame);
    free(img_buff);
  }

  IDBG_MED("%s -", __func__);

  return TRUE;
}

/** module_denoise_port_get_diverted_input_buffer:
 *  @port: the Port to handle the event
 *  @event: the Event to be hanled
 *  @info: stream info configuration
 *
 * Function to get diverted input buffer for denoise port
 *
 * Returns Handler to mapped buffer or NULL
 **/
static module_denoise_buf_t *module_denoise_port_get_diverted_input_buffer(
  mct_port_t *port, mct_event_t *event, mct_stream_info_t* info)
{
  boolean done = FALSE;
  module_denoise_buf_t *ret_val = NULL;
  isp_buf_divert_t *buf_divert;

  IDBG_MED("%s +", __func__);

  buf_divert = event->u.module_event.module_event_data;

  if (buf_divert->native_buf && buf_divert->vaddr) {
    ret_val = malloc(sizeof(module_denoise_buf_t));
    if (ret_val) {
      ret_val->img_frame = malloc(sizeof(img_frame_t));
      if (ret_val->img_frame) {

        ret_val->frame_id = buf_divert->buffer.sequence;
        ret_val->subdev_fd = buf_divert->fd;

        done = module_denoise_port_fill_divert_buffer_handler(
          ret_val->img_frame, buf_divert, info);

      } else
        CDBG_ERROR("Failed to allocate memory in %s", __func__);
    } else
      CDBG_ERROR("Failed to allocate memory in %s", __func__);
  } else
    CDBG_ERROR("Buff divert buffer info is wrong in %s", __func__);

  if (!done) {
    module_denoise_port_release_img_buffer(ret_val);
    ret_val = NULL;
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_get_input_buffer:
 *  @port: the Port to handle the event
 *  @event: the Event to be hanled
 *  @info: stream info configuration
 *
 * Function to get input buffer for denoise port
 *
 * Returns Handler to mapped buffer or NULL
 **/
static module_denoise_buf_t *module_denoise_port_get_input_buffer(
  mct_port_t *port, mct_event_t *event, mct_stream_info_t* info)
{
  boolean done = FALSE;
  module_denoise_buf_t *ret_val;
  mct_list_t* list;
  mct_pipeline_t *pipeline;
  mct_stream_t* stream;
  mct_module_t *module;
  mct_stream_info_t* stream_info = NULL;

  IDBG_MED("%s +", __func__);

  ret_val = malloc(sizeof(module_denoise_buf_t));

  if (ret_val) {
    ret_val->img_frame = malloc(sizeof(img_frame_t));

    if (ret_val->img_frame) {

      if (CAM_ONLINE_REPROCESS_TYPE == info->reprocess_config.pp_type) {

        if (MCT_PORT_PARENT(port) && (MCT_PORT_PARENT(port))->data) {
          module = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
          stream = mod_imglib_find_module_parent(event->identity, module);
          if (stream && MCT_STREAM_PARENT(stream)
            && (MCT_STREAM_PARENT(stream))->data) {

            pipeline = (MCT_STREAM_PARENT(stream))->data;
            list = mct_list_find_custom(MCT_PIPELINE_CHILDREN(pipeline),
              &info->reprocess_config.online.input_stream_id,
              module_denoise_port_find_stream);
            if (list && list->data) {
              stream = list->data;
              stream_info = &stream->streaminfo;
            }
          }
        }
      } else
        stream_info = info;

      if (stream_info) {
        list = mct_list_find_custom(stream_info->img_buffer_list,
          &info->parm_buf.reprocess.buf_index,
          module_denoise_port_find_buff);

        if (list && list->data) {
          if (module_denoise_port_fill_buffer_handler(ret_val->img_frame,
            list->data, stream_info)) {

              done = TRUE;

              IDBG_HIGH("Input buf with index %d from stream id %d is found",
                info->parm_buf.reprocess.buf_index,
                info->reprocess_config.online.input_stream_id);
          } else
            CDBG_ERROR("Unsupported buffer in %s", __func__);
        }
      }
    } else
      CDBG_ERROR("Failed to allocate memory in %s", __func__);
  } else
    CDBG_ERROR("Failed to allocate memory in %s", __func__);

  if (!done) {
    module_denoise_port_release_img_buffer(ret_val);
    ret_val = NULL;

    if (CAM_ONLINE_REPROCESS_TYPE == info->reprocess_config.pp_type)
      CDBG_ERROR("Failed to find input buf with index %d from stream id %d",
        info->parm_buf.reprocess.buf_index,
        info->reprocess_config.online.input_stream_id);
    else
      CDBG_ERROR("Failed to find input buf with index %d",
        info->parm_buf.reprocess.buf_index);
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_get_buffer:
 *  @port: the Port to handle the event
 *  @event: the Event to be hanled
 *  @info: stream info configuration
 *
 * Function to get buffer for denoise port
 *
 * Returns Handler to mapped buffer or NULL
 **/
static module_denoise_buf_t *module_denoise_port_get_buffer(
  mct_port_t *port, mct_event_t *event, mct_stream_info_t* info)
{
  boolean done = FALSE;
  struct msm_buf_mngr_info buff;
  module_denoise_buf_t* ret_val;
  mct_list_t* list;
  int32_t ret;

  IDBG_MED("%s +", __func__);

  ret_val = malloc(sizeof(module_denoise_buf_t));

  if (ret_val) {
    ret_val->img_frame = malloc(sizeof(img_frame_t));
    if (ret_val->img_frame) {
      if (module_denoise_port_get_bfr_mngr_subdev(&ret_val->subdev_fd)) {
        buff.session_id = IMGLIB_SESSIONID(event->identity);
        buff.stream_id = IMGLIB_STREAMID(event->identity);
        ret = ioctl(ret_val->subdev_fd, VIDIOC_MSM_BUF_MNGR_GET_BUF, &buff);
        if (ret >= 0) {
          list = mct_list_find_custom(info->img_buffer_list,
            &buff.index, module_denoise_port_find_buff);

          if (list && list->data) {
            IDBG_HIGH("Output buf with index %d from stream id %d is found",
              buff.index,
              buff.stream_id);
            ret_val->img_frame->idx = buff.index;
            ret_val->frame_id = info->parm_buf.reprocess.frame_idx;

            if (module_denoise_port_fill_buffer_handler(ret_val->img_frame,
              list->data, info))
                done = TRUE;
            else
              CDBG_ERROR("Unsupported buffer in %s", __func__);
          } else
            ioctl(ret_val->subdev_fd, VIDIOC_MSM_BUF_MNGR_PUT_BUF, &buff);
        } else
          CDBG_ERROR("Failed to get buffer from buffer manager");
      } else
        CDBG_ERROR("Failed to get file descriptor of buffer manager");
    } else
      CDBG_ERROR("Failed to allocate memory in %s", __func__);
  } else
      CDBG_ERROR("Failed to allocate memory in %s", __func__);

  if (!done) {
    if (ret_val) {
      if (ret_val->img_frame) {
        if (ret_val->subdev_fd >= 0)
          close(ret_val->subdev_fd);
        free(ret_val->img_frame);
      }
      free(ret_val);
      ret_val = 0;
      CDBG_ERROR("Failed to find output buffer from session %d stream %d",
        IMGLIB_SESSIONID(event->identity),
        IMGLIB_STREAMID(event->identity));
    }
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_get_metadata_buffer:
 *  @port: the Port to handle the event
 *  @event: the Event to be hanled
 *  @info: stream info configuration
 *
 * Function to get metadata buffer for denoise port
 *
 * Returns Handler to mapped buffer or NULL
 **/
static void *module_denoise_port_get_metadata_buffer(
  mct_port_t *port, mct_event_t *event, mct_stream_info_t* info)
{
  void* ret_val = NULL;

  IDBG_MED("%s +", __func__);

  if (port && MCT_PORT_PARENT(port) && event && info) {

    if (info->reprocess_config.pp_type == CAM_ONLINE_REPROCESS_TYPE) {
      ret_val = mct_module_get_buffer_ptr(
        info->parm_buf.reprocess.meta_buf_index,
        (MCT_PORT_PARENT(port))->data,
        IMGLIB_SESSIONID(event->identity),
        info->parm_buf.reprocess.meta_stream_handle);
    } else {
      ret_val = module_imglib_common_get_metadata(info,
          info->parm_buf.reprocess.meta_buf_index);
    }
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_release_native_buffer:
 *  @out_buff: output buffer handler
 *  @identity: stream identity
 *
 * Function to release native buffer
 *
 * Returns TRUE out case of success
 **/
static boolean module_denoise_port_release_native_buffer(
  module_denoise_buf_t *out_buff, uint32_t identity)
{
  boolean ret_val = FALSE;
  int ret;

  IDBG_MED("%s +", __func__);

  if (out_buff && identity) {
    if (out_buff->img_frame) {

      free(out_buff->img_frame);
    }

    free(out_buff);
    out_buff = 0;
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_release_buffer:
 *  @buff: buffer handler
 *  @identity: stream identity
 *  @buff_done: flag indicating whether to use buff done
 *                                      or just return the buffer
 *
 * Function to release output buffer
 *
 * Returns TRUE out case of success
 **/
static boolean module_denoise_port_release_buffer(
  module_denoise_buf_t *buff, uint32_t identity, boolean buff_done)
{
  struct msm_buf_mngr_info buff_info;
  boolean ret_val = FALSE;
  int cmd;
  int ret;

  IDBG_MED("%s +", __func__);

  if (buff && identity) {
    if (buff->img_frame) {
      buff_info.index = buff->img_frame->idx;
      buff_info.session_id = IMGLIB_SESSIONID(identity);
      buff_info.stream_id = IMGLIB_STREAMID(identity);
      buff_info.frame_id = buff->frame_id;
      buff_info.timestamp.tv_sec = buff->img_frame->timestamp / 1000000;
      buff_info.timestamp.tv_usec = buff->img_frame->timestamp
        - buff_info.timestamp.tv_sec * 100000;
      if (buff_done)
        cmd = VIDIOC_MSM_BUF_MNGR_BUF_DONE;
      else
        cmd = VIDIOC_MSM_BUF_MNGR_PUT_BUF;
      ret = ioctl(buff->subdev_fd, cmd, &buff_info);

      if (ret >= 0)
        ret_val = TRUE;
      else
        CDBG_ERROR("Failed to do buf_done in %s", __func__);

      close(buff->subdev_fd);

      free(buff->img_frame);
    }

    free(buff);
    buff = 0;
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_release_metadata_buffer:
 *  @metadata_buff: metadata buffer handler
 *
 * Function to release metadata buffer
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_release_metadata_buffer(
  void *metadata_buff)
{
  IDBG_MED("%s +", __func__);

  // Do nothing

  IDBG_MED("%s -", __func__);

  return TRUE;
}

/** module_denoise_port_notify_cb
 *    @user_data: user data
 *    @out_buff: output buffer handler
 *    @in_buff: input buffer handler
 *
 * Module denoise library process done callback
 *
 * Returns Nothing
 **/
static void module_denoise_port_notify_cb (void* user_data,
  module_denoise_buf_t *out_buff, module_denoise_buf_t *in_buff)
{
  mct_port_t *port = user_data;
  module_denoise_port_t *private;

  IDBG_MED("%s +", __func__);

  if (out_buff && in_buff && port && port->port_private
    && ((module_denoise_port_t *)port->port_private)->lib_instance) {
      private = port->port_private;

      module_denoise_port_release_buffer(out_buff, private->reserved_identity,
        TRUE);
      module_denoise_port_release_img_buffer(in_buff);
      module_denoise_port_release_metadata_buffer(private->metadata_buff);
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);
}

/** module_denoise_port_parm_stream_buf_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be hanled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle stream param event for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_parm_stream_buf_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  mct_stream_info_t *info;
  mct_stream_t* stream;
  mct_module_t *module;
  module_denoise_port_t *private;
  cam_stream_parm_buffer_t *parm_buf;
  cam_metadata_info_t* cam_metadata_info;
  cam_denoise_param_t* denoise2d;

  IDBG_MED("%s +", __func__);

  if (event && event->u.ctrl_event.control_event_data
    && port&& port->port_private
    && ((module_denoise_port_t *)port->port_private)->lib_instance
    && MCT_PORT_PARENT(port) && (MCT_PORT_PARENT(port))->data) {

      parm_buf = event->u.ctrl_event.control_event_data;
      module = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
      // This event should be handled only
      // if current module is the first one in the stream
      if ((parm_buf->type == CAM_STREAM_PARAM_TYPE_DO_REPROCESS)
        && ((mct_module_find_type(module, event->identity)) == MCT_MODULE_FLAG_SOURCE)) {

          stream = mod_imglib_find_module_parent(event->identity, module);
          if (stream) {
            info = &stream->streaminfo;
            denoise2d = &info->reprocess_config.pp_feature_config.denoise2d;
            if (denoise2d->denoise_enable) {
              private = port->port_private;

              mod_imglib_dump_stream_info(info);

              private->in_buff = module_denoise_port_get_input_buffer(port,
                event, info);
              if (private->in_buff) {
                private->out_buff = module_denoise_port_get_buffer(port, event,
                  info);
                if (private->out_buff) {
                  cam_metadata_info =
                    module_denoise_port_get_metadata_buffer(port, event, info);
                  if (cam_metadata_info) {
                    private->metadata_buff = cam_metadata_info;
                    memcpy(&private->session_meta,
                      cam_metadata_info->private_metadata,
                      sizeof(private->session_meta));
                    memcpy(&private->cam_denoise_param,
                      denoise2d,
                      sizeof(private->cam_denoise_param));

                    ret_val = module_denoise_lib_process(private->lib_instance,
                      private->out_buff, private->in_buff,
                      &private->session_meta, port,
                      &private->cam_denoise_param,
                      module_denoise_port_notify_cb);
                  } else
                    CDBG_ERROR("Cannot get metadata buffer in %s\n", __func__);
                } else
                  CDBG_ERROR("Cannot get output buffer in %s\n", __func__);
              } else
                CDBG_ERROR("Cannot get input buffer in %s\n", __func__);
            }
          } else
            CDBG_ERROR("Cannot find module parent with identity 0x%x in %s\n",
              event->identity, __func__);
      } else
        // Other parameters should be forwaded without returning error
        ret_val = TRUE;
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  if (!ret_val)
    CDBG_ERROR("Cannot process image in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_check_config_list_frame_id
 *    @data1: module_denoise_frame_config_t object
 *    @data2: frame id to be checked
 *
 *  Checks if this frame configuration is for specified frame id
 *
*  Return TRUE if the port has the same identity and is linked
 **/
boolean module_denoise_port_check_config_list_frame_id(void *data1,
  void *data2)
{
  boolean ret_val = FALSE;
  module_denoise_frame_config_t *frame_config =
    (module_denoise_frame_config_t *)data1;
  uint32_t *id = (uint32_t *)data2;

  IDBG_MED("%s +", __func__);

  if (id && frame_config && frame_config->out_buff
    && (*id == frame_config->out_buff->frame_id))
      ret_val = TRUE;

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_check_config_list_buf_index
 *    @data1: module_denoise_frame_config_t object
 *    @data2: buff index to be checked
 *
 *  Checks if this frame configuration is for specified buff index
 *
*  Return TRUE if the port has the same identity and is linked
 **/
boolean module_denoise_port_check_config_list_buf_index(void *data1,
  void *data2)
{
  boolean ret_val = FALSE;
  module_denoise_frame_config_t *frame_config =
    (module_denoise_frame_config_t *)data1;
  uint32_t *buff_index = (uint32_t *)data2;

  IDBG_MED("%s +", __func__);

  if (buff_index && frame_config && frame_config->in_buff
    && frame_config->in_buff->img_frame
    && (*buff_index == frame_config->out_buff->img_frame->idx))
      ret_val = TRUE;

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_send_buff_done_ack
 *    @port: the Port that handles the event
 *    @buff: buffer handler
 *
 * Sends buff divert ack to peer
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_send_buff_done_ack(mct_port_t* port,
  module_denoise_buf_t *buff)
{
  boolean ret_val = FALSE;
  module_denoise_port_t *private;
  isp_buf_divert_ack_t isp_buf_divert_ack;
  mct_event_t event;

  IDBG_MED("%s +", __func__);

  if (buff && port && port->port_private && MCT_PORT_PEER(port)
    && MCT_PORT_EVENT_FUNC(MCT_PORT_PEER(port))) {
      private = port->port_private;

      isp_buf_divert_ack.buf_idx = buff->frame_id;
      isp_buf_divert_ack.is_buf_dirty = TRUE;
      isp_buf_divert_ack.identity = private->reserved_identity;

      event.identity = isp_buf_divert_ack.identity;
      event.type = MCT_EVENT_MODULE_EVENT;
      event.direction = MCT_EVENT_UPSTREAM;
      event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT_ACK;
      event.u.module_event.module_event_data = (void *)&isp_buf_divert_ack;

      ret_val = MCT_PORT_EVENT_FUNC(MCT_PORT_PEER(port))(MCT_PORT_PEER(port),
        &event);

      if (!ret_val)
        CDBG_ERROR("Cannot send MCT_EVENT_MODULE_BUF_DIVERT_ACK in %s\n",
          __func__);
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_forward_event_to_peer
 *    @data: mct_port_t object
 *    @user_data: event to be forwarded
 *
 * Forwards event to peer with same identity
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_forward_event_to_peer(void *data,
                                                         void *user_data)
{
  boolean ret_val = FALSE;
  mct_port_t *port = (mct_port_t *)data;
  mct_port_t *peer;
  mct_event_t *event = user_data;
  module_denoise_port_t *private;

  IDBG_MED("%s +", __func__);

  if (port && event && MODULE_DENOISE_VALIDATE_NAME(port)
    && port->port_private && MCT_PORT_PEER(port)) {

    peer = MCT_PORT_PEER(port);
    private = port->port_private;

    if (private->reserved_identity == event->identity) {

      IDBG_MED("%s: event send to peer", __func__);
      ret_val = MCT_PORT_EVENT_FUNC(peer)(peer, event);
      IDBG_MED("%s: event sent ret_val 0x%x", __func__, ret_val);

    } else
      ret_val = TRUE;
    }

    IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_send_buff
 *    @port: the Port that handles the event
 *    @buff: buffer handler
 *
 * Sends buff divert to peer
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_send_buff(mct_port_t* port,
  module_denoise_buf_t *buff)
{
  boolean ret_val = FALSE;
  module_denoise_port_t *private;
  isp_buf_divert_t isp_buf_divert;
  mct_event_t event;

  IDBG_MED("%s +", __func__);

  if (buff && port && port->port_private && MCT_PORT_PARENT(port)) {
      private = port->port_private;

      memset(&isp_buf_divert, 0, sizeof(isp_buf_divert_t));

      isp_buf_divert.native_buf = TRUE;
      isp_buf_divert.vaddr = buff->img_frame->frame[0].plane[0].addr;
      isp_buf_divert.fd = buff->subdev_fd;

      isp_buf_divert.buffer.sequence = buff->frame_id;
      isp_buf_divert.buffer.length = buff->img_frame->frame[0].plane_cnt;
      isp_buf_divert.buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
      isp_buf_divert.buffer.index = buff->img_frame->idx;
      isp_buf_divert.buffer.memory = V4L2_MEMORY_USERPTR;
      gettimeofday(&isp_buf_divert.buffer.timestamp, NULL);

      isp_buf_divert.is_locked = FALSE;
      isp_buf_divert.ack_flag = FALSE;
      isp_buf_divert.is_buf_dirty = FALSE;
      isp_buf_divert.identity = private->reserved_identity;

      event.identity = isp_buf_divert.identity;
      event.type = MCT_EVENT_MODULE_EVENT;
      event.direction = MCT_EVENT_DOWNSTREAM;
      event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
      event.u.module_event.module_event_data = (void *)&isp_buf_divert;

      ret_val = mct_list_traverse(
        MCT_MODULE_SRCPORTS(MCT_PORT_PARENT(port)->data),
        module_denoise_port_forward_event_to_peer,
        &event);

      if (!ret_val)
        CDBG_ERROR("Cannot send MCT_EVENT_MODULE_BUF_DIVERT in %s\n",
          __func__);
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_divert_notify_cb
 *    @user_data: user data
 *    @out_buff: output buffer handler
 *    @in_buff: input buffer handler
 *
 * Module denoise library process done callback for diverted frame
 *
 * Returns Nothing
 **/
static void module_denoise_port_divert_notify_cb(void* user_data,
  module_denoise_buf_t *out_buff, module_denoise_buf_t *in_buff)
{
  boolean config_found = FALSE;
  mct_port_t *port = user_data;
  module_denoise_port_t *private;
  module_denoise_frame_config_t frame_config;
  mct_list_t *list_match;
  boolean send_frame_to_next_module = FALSE;

  IDBG_MED("%s +", __func__);

  if (out_buff && in_buff && port && port->port_private
    && ((module_denoise_port_t *)port->port_private)->lib_instance) {
      private = port->port_private;

      if (pthread_mutex_lock(&private->mutex_config_list))
        CDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);

      list_match = mct_list_find_custom(private->config_list,
        &out_buff->frame_id, module_denoise_port_check_config_list_frame_id);

      if (list_match && list_match->data) {
        config_found = TRUE;
        frame_config = *(module_denoise_frame_config_t *)list_match->data;

        if (frame_config.src_port_linked)
          send_frame_to_next_module = TRUE;
        else {
          free(list_match->data);
          private->config_list = mct_list_remove(private->config_list,
            list_match->data);
        }
      } else
        CDBG_ERROR("Memory corruption in %s:%d \n", __func__, __LINE__);

      if (pthread_mutex_unlock(&private->mutex_config_list))
        CDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);

      if (config_found) {
        if (send_frame_to_next_module) {
          module_denoise_port_send_buff(port, out_buff);
        } else {
          module_denoise_port_release_buffer(out_buff,
            private->reserved_identity, TRUE);

          module_denoise_port_send_buff_done_ack(port, in_buff);

          module_denoise_port_release_img_buffer(in_buff);
        }
      }
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);
}

/** module_denoise_port_buf_divert_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be hanled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle buf divert event for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_buf_divert_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  module_denoise_port_t *private = NULL;
  mct_stream_info_t *info;
  mct_stream_t* stream;
  mct_module_t *module;
  mct_list_t *list_match;
  isp_buf_divert_t *buf_divert;
  module_denoise_frame_config_t *frame_config = NULL;

  IDBG_MED("%s +", __func__);

  if (port && event && port->port_private
    && MCT_OBJECT_PARENT(port) && MCT_OBJECT_PARENT(port)->data
    && event->u.module_event.module_event_data) {
      buf_divert = event->u.module_event.module_event_data;
      private = port->port_private;
      module = MCT_MODULE_CAST(MCT_OBJECT_PARENT(port)->data);

      frame_config = malloc(sizeof(module_denoise_frame_config_t));

      if (private->cam_denoise_param.denoise_enable) {
        *forward_event = FALSE;

        if (frame_config) {

          list_match = mct_list_find_custom(MCT_MODULE_SRCPORTS(module),
            &event->identity, module_denoise_port_check_linked_port_identity);

          frame_config->src_port_linked = FALSE;
          if (list_match && list_match->data) {
            frame_config->src_port_linked = TRUE;
          }

          stream = mod_imglib_find_module_parent(event->identity, module);
          if (stream) {

            info = &stream->streaminfo;

            mod_imglib_dump_stream_info(info);

            private->in_buff = module_denoise_port_get_diverted_input_buffer(
              port, event, info);
            if (private->in_buff) {

              private->out_buff = module_denoise_port_get_buffer(port, event,
                info);

              if (private->out_buff) {

                frame_config->out_buff = private->out_buff;
                frame_config->in_buff = private->in_buff;

                if (pthread_mutex_lock(&private->mutex_config_list))
                  CDBG_ERROR("Cannot lock the mutex in %s:%d \n",
                    __func__, __LINE__);

                private->config_list =
                  mct_list_append(private->config_list, frame_config, NULL, NULL);

                if (pthread_mutex_unlock(&private->mutex_config_list))
                  CDBG_ERROR("Cannot unlock the mutex in %s:%d \n",
                    __func__, __LINE__);

                if (private->config_list) {

                  ret_val = module_denoise_lib_process(private->lib_instance,
                    private->out_buff, private->in_buff,
                    &private->session_meta, port,
                    &private->cam_denoise_param,
                    module_denoise_port_divert_notify_cb);

                } else
                  CDBG_ERROR("Cannot append to list in %s\n", __func__);
              } else
                CDBG_ERROR("Cannot get output buffer in %s\n", __func__);
            } else
              CDBG_ERROR("Can't find module parent with id 0x%x in %s\n",
                event->identity, __func__);
          } else
            CDBG_ERROR("Cannot get diverted buffer in %s\n", __func__);
        } else
          CDBG_ERROR("Not enough memory in %s\n", __func__);
      } else
        ret_val = TRUE;
    } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  if (!ret_val && frame_config && private) {
    CDBG_ERROR("Cannot process the image in %s\n", __func__);

    if (pthread_mutex_lock(&private->mutex_config_list))
      CDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);

    private->config_list = mct_list_remove(private->config_list, frame_config);
    free(frame_config);

    if (pthread_mutex_unlock(&private->mutex_config_list))
      CDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_up_buf_divert_ack_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be hanled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle buf divert ack upstream event for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_up_buf_divert_ack_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  boolean config_found = FALSE;
  module_denoise_port_t *private;
  mct_list_t *list_match;
  isp_buf_divert_ack_t *buf_divert_ack;
  module_denoise_frame_config_t frame_config;

  IDBG_MED("%s +", __func__);

  if (port && event && port->port_private
    && event->u.module_event.module_event_data) {
      buf_divert_ack = event->u.module_event.module_event_data;
      private = port->port_private;

      if (pthread_mutex_lock(&private->mutex_config_list))
        CDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);

      list_match = mct_list_find_custom(private->config_list,
        &buf_divert_ack->buf_idx,
        module_denoise_port_check_config_list_buf_index);

      if (list_match && list_match->data) {
        config_found = TRUE;
        *forward_event = FALSE;
        frame_config = *(module_denoise_frame_config_t *)list_match->data;

        free(list_match->data);
        private->config_list = mct_list_remove(private->config_list,
          list_match->data);
      }

      if (pthread_mutex_unlock(&private->mutex_config_list))
        CDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);

      if (config_found) {
        module_denoise_port_release_buffer(frame_config.out_buff,
          private->reserved_identity,
          !buf_divert_ack->is_buf_dirty);

        module_denoise_port_send_buff_done_ack(port, frame_config.in_buff);

        module_denoise_port_release_img_buffer(frame_config.in_buff);
      }

      ret_val = TRUE;
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_downstream_ctrl:
 *  @port: the Port to handle the event
 *  @event: the Event to be hanled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle command for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_denoise_port_downstream_ctrl(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  module_denoise_port_event_func event_handler;

  IDBG_MED("%s +", __func__);

  if (port && event) {
    mct_event_control_t *mct_event_control = &event->u.ctrl_event;

    event_handler =
      module_denoise_port_find_event_handler(mct_event_control->type,
        module_denoise_port_cmd_lut);
    if (event_handler)
      ret_val = event_handler(port, event, forward_event);
    else
      ret_val = TRUE;
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be hanled
 *
 * Function to handle an event for the port
 *
 * Returns: TRUE if success
 */
static boolean module_denoise_port_event(mct_port_t *port, mct_event_t *event)
{
  boolean ret_val = FALSE;
  boolean forward_event = TRUE; // Always need to be initialized to true
  module_denoise_port_t *private;

  IDBG_MED("%s +", __func__);

  if (port && event && port->port_private
    && MODULE_DENOISE_VALIDATE_NAME(port)) {

    private = port->port_private;
    if (private->reserved_identity == event->identity) {

      IDBG_HIGH("%s: private->reserved_identity=0x%x, event->identity=0x%x",
        __func__, private->reserved_identity, event->identity);
      IDBG_HIGH("%s: event dir=%d, type=%d id=%d", __func__,
        MCT_EVENT_DIRECTION(event), event->type,
        ((mct_event_module_t *)&event->u.module_event)->type);

      switch (MCT_EVENT_DIRECTION(event)) {
      case MCT_EVENT_DOWNSTREAM:
        switch (event->type) {
        case MCT_EVENT_MODULE_EVENT:
          ret_val = module_denoise_port_downstream_event(port, event,
            &forward_event);
          break;

        case MCT_EVENT_CONTROL_CMD:
          ret_val = module_denoise_port_downstream_ctrl(port, event,
            &forward_event);
          break;

        default:
          CDBG_ERROR("Event is with wrong type");
          break;
        }

        if (ret_val && forward_event)
          ret_val = mct_list_traverse(
            MCT_MODULE_SRCPORTS(MCT_PORT_PARENT(port)->data),
            module_denoise_port_forward_event_to_peer, event);
        /* case MCT_EVENT_TYPE_DOWNSTREAM */
        break;

      case MCT_EVENT_UPSTREAM:
        ret_val = module_denoise_port_upstream_event(port, event,
          &forward_event);

        if (ret_val && forward_event)
          ret_val = mct_list_traverse(
            MCT_MODULE_SINKPORTS(MCT_PORT_PARENT(port)->data),
            module_denoise_port_forward_event_to_peer, event);

        break;

      default:
        CDBG_ERROR("Event is with wrong direction");
        break;
      }
    } else
      CDBG_ERROR("Event is meant for port with different identity");
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_ext_link
 *    @identity: the identity of the stream and session
 *    @port: the port that is linked
 *    @peer: the peer port of the link
 *
 * Function that handles a new external link on the port
 *
 * Returns: TRUE if success
 */
static boolean module_denoise_port_ext_link(uint32_t identity,
  mct_port_t *port, mct_port_t *peer)
{
  boolean ret_val = FALSE;
  module_denoise_port_t *private;

  IDBG_MED("%s +", __func__);

  if (port && peer && MODULE_DENOISE_VALIDATE_NAME(port)
    && (module_denoise_port_t *)port->port_private) {
    private = (module_denoise_port_t *)port->port_private;

    MCT_OBJECT_LOCK(port);

    if ((MODULE_DENOISE_PORT_STATE_RESERVED == private->state
      || MODULE_DENOISE_PORT_STATE_LINKED == private->state)
      && (private->reserved_identity == identity)) {

      private->state = MODULE_DENOISE_PORT_STATE_LINKED;
      MCT_PORT_PEER(port) = peer;
      MCT_OBJECT_REFCOUNT(port) += 1;

      IDBG_HIGH("Port %s linked to identity 0x%x",
        MCT_OBJECT_NAME(port), identity);

      ret_val = TRUE;
    }
    MCT_OBJECT_UNLOCK(port);
  } else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_unlink
 *    @identity: the sessionid idntity
 *    @port: the port that is linked
 *    @peer: the peer port of the link
 *
 * Function that handles a removing external link on the port
 *
 * Returns: TRUE if success
 */
static void module_denoise_port_unlink(uint32_t identity, mct_port_t *port,
  mct_port_t *peer)
{
  module_denoise_port_t *private;

  IDBG_MED("%s +", __func__);

  if (port && peer && MCT_PORT_PEER(port) == peer
    && (module_denoise_port_t *)port->port_private
    && MODULE_DENOISE_VALIDATE_NAME(port)) {

    private = (module_denoise_port_t *)port->port_private;

    MCT_OBJECT_LOCK(port);
    if (MODULE_DENOISE_PORT_STATE_LINKED == private->state &&
        private->reserved_identity == identity) {

      MCT_OBJECT_REFCOUNT(port) -= 1;
      if (!MCT_OBJECT_REFCOUNT(port))
        private->state = MODULE_DENOISE_PORT_STATE_RESERVED;
    }

    IDBG_HIGH("Port %s unliked from identity 0x%x",
      MCT_OBJECT_NAME(port), identity);

    MCT_OBJECT_UNLOCK(port);
  }

  IDBG_MED("%s -", __func__);
}

/** module_denoise_port_set_caps
 *    @port: port with new capabilites
 *    @caps: new capabilities
 *
 * Function that sets the capabilites of the given port
 *
 * Returns: TRUE if success
 */
static boolean module_denoise_port_set_caps(mct_port_t *port,
  mct_port_caps_t *caps)
{
  boolean ret_val = FALSE;

  IDBG_MED("%s +", __func__);

  if (MODULE_DENOISE_VALIDATE_NAME(port)) {

    port->caps = *caps;
    ret_val = TRUE;
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_check_caps_reserve
 *    @port: port whoms capabilities to be checked
 *    @peer_caps: the peer port capabilities
 *    @info: media controller stream info
 *
 * Function that checks capabilities match with suggested peer port. If there
 * is a match, the port is reserved for for establishing suggested link
 *
 * Returns: TRUE if success
 */
static boolean module_denoise_port_check_caps_reserve(mct_port_t *port,
  void *caps, void *info)
{
  boolean ret_val = FALSE;
  mct_port_caps_t *peer_caps = caps;
  mct_stream_info_t *stream_info = info;
  module_denoise_port_t *private;

  IDBG_MED("%s +", __func__);

  if (port && MODULE_DENOISE_VALIDATE_NAME(port)
    && peer_caps && info && (module_denoise_port_t *)port->port_private) {

    MCT_OBJECT_LOCK(port);

    if (peer_caps->port_caps_type == port->caps.port_caps_type) {

      private = (module_denoise_port_t *)port->port_private;
      if (MODULE_DENOISE_PORT_STATE_CREATED == private->state) {

        private->reserved_identity = stream_info->identity;
        private->state = MODULE_DENOISE_PORT_STATE_RESERVED;

        ret_val = TRUE;

        IDBG_HIGH("Port %s reserved to identity 0x%x",
          MCT_OBJECT_NAME(port), stream_info->identity);
      }
    }

    MCT_OBJECT_UNLOCK(port);
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_check_caps_unreserve
 *    @port: port whoms capabilities to be checked
 *    @identity: the identity of the stream and session
 *
 * Function that unreserves specified stream and session from specified port
 *
 * Returns: TRUE if success
 */
static boolean module_denoise_port_check_caps_unreserve (mct_port_t *port,
  uint32_t identity)
{
  boolean ret_val = FALSE;
  module_denoise_port_t *private;

  IDBG_MED("%s +", __func__);

  if (port && MODULE_DENOISE_VALIDATE_NAME(port)
    && (module_denoise_port_t *)port->port_private) {

    private = (module_denoise_port_t *)port->port_private;

    MCT_OBJECT_LOCK(port);
    if (MODULE_DENOISE_PORT_STATE_RESERVED == private->state
      && private->reserved_identity == identity) {

      private->state = MODULE_DENOISE_PORT_STATE_CREATED;

      ret_val = TRUE;

      IDBG_HIGH("Port %s unreserved from identity 0x%x",
        MCT_OBJECT_NAME(port), identity);
    }

    MCT_OBJECT_UNLOCK(port);
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_denoise_port_overwrite_port_funcs:
 *    @port: port whoms functions will be overwritten
 *
 * Overwrites defualft port funcrtions with denoise port specific ones
 *
 * Returns nothing
 **/
static void module_denoise_port_overwrite_port_funcs(mct_port_t *port)
{
  IDBG_MED("%s + ", __func__);

  if (port) {
    mct_port_set_event_func(port, module_denoise_port_event);
    mct_port_set_ext_link_func(port, module_denoise_port_ext_link);
    mct_port_set_unlink_func(port, module_denoise_port_unlink);
    mct_port_set_set_caps_func(port, module_denoise_port_set_caps);
    mct_port_set_check_caps_reserve_func(port,
      module_denoise_port_check_caps_reserve);
    mct_port_set_check_caps_unreserve_func(port,
      module_denoise_port_check_caps_unreserve);
  } else
    CDBG_ERROR("Null pointer in function %s", __func__);

  IDBG_MED("%s - ", __func__);
}

/** module_denoise_port_deinit
 *    @port: port to be deinitialized
 *
 * Deinitializes port
 *
 * Returns TRUE in case of success
 **/
void module_denoise_port_deinit(mct_port_t *port)
{
  module_denoise_port_t *private;

  IDBG_MED("%s +", __func__);

  if (port && port->port_private && MODULE_DENOISE_VALIDATE_NAME(port)) {

    MCT_OBJECT_LOCK(port);

    private = port->port_private;

    if (pthread_mutex_lock(&private->mutex_config_list))
      CDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);

    mct_list_free_list(private->config_list);

    if (pthread_mutex_unlock(&private->mutex_config_list))
      CDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);

    if (pthread_mutex_destroy(&private->mutex_config_list))
      CDBG_ERROR("Cannot destroy mutex\n");

    module_denoise_lib_deinit(private->lib_instance);

    IDBG_HIGH("Port %s destroyed", MCT_OBJECT_NAME(port));

    if (private)
      free(private);
  }

  IDBG_MED("%s -", __func__);
}

/** module_denoise_port_init:
 *    @port: port to be initialized
 *    @direction: source / sink
 *    @sessionid: session ID to be associated with this port
 *    @lib_handle: library handle
 *
 *  Port initialization entry point. Becase current module/port is
 *  pure software object, defer this function when session starts.
 **/
boolean module_denoise_port_init(mct_port_t *port,
  mct_port_direction_t direction, uint32_t *sessionid, void* lib_handle)
{
  boolean ret_val = FALSE;
  mct_port_caps_t caps;
  uint32_t *session;
  mct_list_t *list;
  module_denoise_port_t *private;

  IDBG_MED("%s +", __func__);

  if (port)
    if (MODULE_DENOISE_VALIDATE_NAME(port)) {

      private = (void *)calloc(1, sizeof(module_denoise_port_t));
      if (private) {
        private->lib_instance = module_denoise_lib_init(lib_handle);
        if (private->lib_instance) {
          private->reserved_identity = (*sessionid) << 16;
          private->state       = MODULE_DENOISE_PORT_STATE_CREATED;

          if (!pthread_mutex_init(&private->mutex_config_list, NULL)) {

            port->port_private   = private;
            port->direction      = direction;
            caps.port_caps_type  = MCT_PORT_CAPS_FRAME;

            module_denoise_port_overwrite_port_funcs(port);

            if (port->set_caps)
              port->set_caps(port, &caps);

            IDBG_HIGH("Port %s initialized", MCT_OBJECT_NAME(port));

            ret_val = TRUE;
          } else
            CDBG_ERROR("Cannot initialize mutex in %s", __func__);
        } else {
          CDBG_ERROR("Port private data cannot be initialized");
        }

        if (!ret_val)
          free(private);
      }
    } else {
      CDBG_ERROR("Requested port name is %s\n", MCT_OBJECT_NAME(port));
      CDBG_ERROR("Port name needs to start with %s\n", MODULE_DENOISE_NAME);
    }
  else
    CDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}
