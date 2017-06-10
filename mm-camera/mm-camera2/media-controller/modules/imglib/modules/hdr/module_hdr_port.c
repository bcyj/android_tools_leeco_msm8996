/*============================================================================
 Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
 Qualcomm Technologies Proprietary and Confidential.
 ============================================================================*/

#include "module_hdr_dbg.h"
#include "module_imglib_common.h"
#include "module_hdr_port.h"
#include "module_hdr_lib.h"
#include <media/msmb_generic_buf_mgr.h>
#include "modules.h"

static boolean module_hdr_port_buf_divert_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event);
static boolean module_hdr_port_isp_gamma_update_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event);
static boolean module_hdr_port_up_buf_divert_ack_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event);
static boolean module_hdr_port_qry_dvrt_evnt(mct_port_t *port,
  mct_event_t *event, boolean *forward_event);
static boolean module_hdr_port_isp_output_dim_event(mct_port_t * port,
  mct_event_t * event,boolean * forward_event);
static boolean module_hdr_port_stream_on_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event);
static boolean module_hdr_port_stream_off_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event);
static boolean module_hdr_port_stream_buf_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event);

/** MODULE_HDR_PROPERTY_DUMP_DISABLE
 *
 *  Defines the system property for dump disable
 *
 *  Returns the system property for dump disable
 *
 **/
#define MODULE_HDR_PROPERTY_DUMP_DISABLE "no"

/** MODULE_HDR_PROPERTY_IN_DUMP_ENABLE
 *
 *  Defines the system property for input frame dump enable
 *
 *  Returns the system property for input frame dump enable
 *
 **/
#define MODULE_HDR_PROPERTY_IN_DUMP_ENABLE "in"

/** MODULE_HDR_PROPERTY_OUT_DUMP_ENABLE
 *
 *  Defines the system property for output frame dump enable
 *
 *  Returns the system property for output frame dump enable
 *
 **/
#define MODULE_HDR_PROPERTY_OUT_DUMP_ENABLE "out"

/** MODULE_HDR_PROPERTY_IN_OUT_DUMP_ENABLE
 *
 *  Defines the system property for input and output frame dump enable
 *
 *  Returns the system property for input and output frame dump enable
 *
 **/
#define MODULE_HDR_PROPERTY_IN_OUT_DUMP_ENABLE "in out"

/** module_hdr_port_state_t:
 *    @MODULE_HDR_PORT_STATE_CREATED: port state is just created
 *    @MODULE_HDR_PORT_STATE_RESERVED: port state is reserved
 *    @MODULE_HDR_PORT_STATE_LINKED: port state is linked to peer
 *    @MODULE_HDR_PORT_STATE_COUNT: port state count
 *
 *  This enum defines the state of the port
 **/
typedef enum
{
  MODULE_HDR_PORT_STATE_CREATED,
  MODULE_HDR_PORT_STATE_RESERVED,
  MODULE_HDR_PORT_STATE_LINKED,
  MODULE_HDR_PORT_STATE_COUNT
} module_hdr_port_state_t;

/** HDR_PORT_IN_BUFFS:
 *
 *  This macro defines the number of input buffers for the port
 *    +1 is for the non HDR frame
 **/
#define HDR_PORT_IN_BUFFS (HDR_LIB_IN_BUFFS + 1)

/** module_hdr_port_t:
 *    @reserved_identity: session identity associated with this port
 *    @state: state of the port
 *    @lib_instance: library instance
 *    @out_buff: output buffer handlers
 *    @in_buff: input buffer handlers
 *    @metadata_buff: metadata buffer handler
 *    @session_meta: metadata session data
 *    @is_session_meta_valid: flag indicating whether session metadata is valid
 *    @config_list: list with current frames' configurations
 *    @mutex_config_list: mutex for protecting access to config list
 *    @hdr_burst_counter: hdr burst counter
 *    @subdev_fd: buffer manager file descriptor
 *    @stream_info: reserved stream info
 *    @input_stream_info: input(snapshot, when hdr is pproc sub module) stream info
 *    @hdr_port_in_buffs: the number of expected input buffers
 *    @non_hdr_buf: non HDR buffer handler
 *    @non_hdr_extra_buf_needed: whether extra non-hdr input buffer is needed
 *    @non_hdr_buf_index: non HDR buffer index from input bracketing sequence
 *    @hdr_need_1x: user setting for hdr 1x frame need
 *    @crop_output_queue: queue with output crop settings
 *    @mutex_crop_output_queue: mutex for protecting output crop queue
 *    @dump_input_frame: Flag to indicate whether input frame needs to be dumped
 *    @dump_output_frame: Flag to indicate whether out frame needs to be dumped
 *    @is_srcport_connected: Flag to indicate connection status of src port
 *
 *  This structure defines hdr module port
 **/
typedef struct
{
  uint32_t reserved_identity;
  module_hdr_port_state_t state;
  void* lib_instance;
  module_hdr_buf_t* out_buff[HDR_LIB_OUT_BUFFS + HDR_LIB_INPLACE_BUFFS];
  module_hdr_buf_t* in_buff[HDR_PORT_IN_BUFFS];
  void* metadata_buff;
  mct_stream_session_metadata_info session_meta;
  boolean is_session_meta_valid;
  mct_list_t *config_list;
  pthread_mutex_t mutex_config_list;
  uint32_t hdr_burst_counter;
  int32_t subdev_fd;
  mct_stream_info_t *stream_info;
  mct_stream_info_t input_stream_info;
  uint32_t hdr_port_in_buffs;
  module_hdr_buf_t* non_hdr_buf;
  boolean non_hdr_extra_buf_needed;
  uint32_t non_hdr_buf_index;
  boolean hdr_need_1x;
  mct_queue_t crop_output_queue;
  pthread_mutex_t mutex_crop_output_queue;
  boolean dump_input_frame;
  boolean dump_output_frame;
  boolean stream_on;
  boolean is_srcport_connected;
} module_hdr_port_t;

/** module_frame_config_t:
 *    @src_port_linked: flag indicating whether src port is linked
 *    @out_buff: output buffer handler
 *    @in_buff: input buffer handler
 *    @input_buff_number: number of input buffers
 *
 *  This structure defines hdr configuration for particular frame
 **/
typedef struct
{
  boolean src_port_linked;
  module_hdr_buf_t* out_buff;
  module_hdr_buf_t* in_buff[HDR_PORT_IN_BUFFS];
  uint32_t input_buff_number;
} module_frame_config_t;

/** module_hdr_port_event_func:
 *  @port: the Port to handle the event
 *  @event: the Event to be handld
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle port event
 *
 * Returns TRUE in case of success
 **/
typedef boolean (*module_hdr_port_event_func)(mct_port_t *port,
  mct_event_t *event, boolean *forward_event);

/** module_hdr_port_event_lut_entry_t:
 *    @type: binded command
 *    @event_handler: binded event handler
 *
 *
 *  This structure defines entry in the hdr module LUT for event handlers
 **/
typedef struct
{
  union
  {
    uint32_t type;
    mct_event_module_type_t event_type;
    mct_event_control_type_t cmd_type;
  };
  module_hdr_port_event_func event_handler;
} module_hdr_port_event_lut_entry_t;

/** module_hdr_port_event_lut:
 *
 *  This array defines hdr module LUT for event handlers
 **/
static module_hdr_port_event_lut_entry_t module_hdr_port_event_lut[] = { { {
  MCT_EVENT_MODULE_BUF_DIVERT }, module_hdr_port_buf_divert_event }, { {
  MCT_EVENT_MODULE_ISP_GAMMA_UPDATE }, module_hdr_port_isp_gamma_update_event },
  { { MCT_EVENT_MODULE_QUERY_DIVERT_TYPE }, module_hdr_port_qry_dvrt_evnt },
  { { MCT_EVENT_MODULE_ISP_OUTPUT_DIM }, module_hdr_port_isp_output_dim_event },
  // Dummy entry. It needs to be always at the end of the LUT
  { { MCT_EVENT_MODULE_MAX }, NULL } };

/** module_hdr_port_cmd_lut:
 *
 *  This array defines hdr module LUT for command handlers
 **/
static module_hdr_port_event_lut_entry_t module_hdr_port_cmd_lut[] = { { {
  MCT_EVENT_CONTROL_STREAMON }, module_hdr_port_stream_on_event }, { {
  MCT_EVENT_CONTROL_STREAMOFF }, module_hdr_port_stream_off_event }, { {
  MCT_EVENT_CONTROL_PARM_STREAM_BUF }, module_hdr_port_stream_buf_event },
// Dummy entry. It needs to be always at the end of the LUT
  { { MCT_EVENT_CONTROL_MAX }, NULL } };

/** module_hdr_port_up_lut:
 *
 *  This array defines hdr module LUT for upstream event handlers
 **/
static module_hdr_port_event_lut_entry_t module_hdr_port_up_lut[] = { { {
  MCT_EVENT_MODULE_BUF_DIVERT_ACK }, module_hdr_port_up_buf_divert_ack_event },
// Dummy entry. It needs to be always at the end of the LUT
  { { MCT_EVENT_MODULE_MAX }, NULL } };

/** module_hdr_port_queue_free_func
 *    @data: node data
 *    @user_data: not used. Null pointer
 *
 * Memory release traverse function for flushing queue
 *
 * Returns TRUE
 **/
static boolean module_hdr_port_queue_free_func(void *data, void *user_data)
{
  free(data);

  return TRUE;
}

/** module_hdr_port_validate_port_session_id
 *    @data1: mct_port_t object
 *    @data2: identity to be checked
 *
 *  Checks if this port has already been assigned to specified session id
 *
 *  Return TRUE if the port has the same session id
 **/
boolean module_hdr_port_validate_port_session_id(void *data1, void *data2)
{
  boolean ret_val = FALSE;
  mct_port_t *port = data1;
  uint32_t *session_id = data2;
  module_hdr_port_t *private_data;

  IDBG_MED("%s +", __func__);

  if (port && session_id) {
    if (MODULE_HDR_VALIDATE_NAME(port)) {
      private_data = port->port_private;
      if ((private_data)
        && (IMGLIB_SESSIONID(private_data->reserved_identity) == *session_id))
        ret_val = TRUE;
    }
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_validate_linked_port_identity
 *    @data1: list buf object
 *    @data2: buffer index to be checked
 *
 *  Checks if this buffer is with specified index
 *
 *  Return TRUE if the port has the same identity and is linked
 **/
static boolean module_hdr_port_validate_buff_index(void *data1, void *data2)
{
  mct_stream_map_buf_t *list_buf = (mct_stream_map_buf_t *)data1;
  uint32_t *buf_idx = (uint32_t *)data2;
  boolean ret_val = FALSE;

  if (list_buf && buf_idx) {
    if (list_buf->buf_index == *buf_idx)
      ret_val = TRUE;
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_check_linked_port_identity
 *    @data1: mct_port_t object
 *    @data2: identity to be checked
 *
 *  Checks if this port has already been assigned to specified identity
 *    and the port is linked
 *
 *  Return TRUE if the port has the same identity and is linked
 **/
boolean module_hdr_port_check_linked_port_identity(void *data1, void *data2)
{
  boolean ret_val = FALSE;
  module_hdr_port_t *private_data;
  mct_port_t *port = (mct_port_t *)data1;
  uint32_t *identity = (uint32_t *)data2;

  IDBG_MED("%s +", __func__);

  if (port && identity) {
    if (MODULE_HDR_VALIDATE_NAME(port)) {
      private_data = port->port_private;
      if ((private_data) && (private_data->reserved_identity == *identity)
        && (MODULE_HDR_PORT_STATE_LINKED == private_data->state))
        ret_val = TRUE;
    }
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_find_event_handler
 *    @id: event id
 *    @hdr_handlers_lut: hdr handlers lut
 *
 * Returns event handler binded to corresponded event id
 *
 * Returns event handler or null (if event handler is not found)
 **/
static void *module_hdr_port_find_event_handler(uint32_t id,
  module_hdr_port_event_lut_entry_t hdr_handlers_lut[])
{
  void* ret_val = NULL;
  uint32_t i;

  IDBG_MED("%s +", __func__);

  for (i = 0; hdr_handlers_lut[i].event_handler; i++) {
    if (hdr_handlers_lut[i].type == id) {
      ret_val = hdr_handlers_lut[i].event_handler;
      break;
    }
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_upstream_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be handled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle an upstream event for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_upstream_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  module_hdr_port_event_func event_handler;

  IDBG_MED("%s +", __func__);

  if (port && event) {
    mct_event_module_t *mct_event_module = &event->u.module_event;

    event_handler = module_hdr_port_find_event_handler(mct_event_module->type,
      module_hdr_port_up_lut);
    if (event_handler)
      ret_val = event_handler(port, event, forward_event);
    else
      ret_val = TRUE;
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_downstream_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be handled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle an event for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_downstream_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  module_hdr_port_event_func event_handler;

  IDBG_MED("%s +", __func__);

  if (port && event) {
    mct_event_module_t *mct_event_module = &event->u.module_event;

    event_handler = module_hdr_port_find_event_handler(mct_event_module->type,
      module_hdr_port_event_lut);
    if (event_handler)
      ret_val = event_handler(port, event, forward_event);
    else
      ret_val = TRUE;
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_fill_native_input_buffer:
 *  @buf_addr: virtual address of the native buffer
 *  @img_frame: image frame handle
 *  @buf_divert: buf divert message
 *  @info: stream info configuration
 *  @img_buff: stream buffer map list
 *  @hdr_port_data: hdr port private data
 *
 *  Function to fill image buffer handler descriptor. Takes care of rotation
 *  configuration of stream when HDR is source and has to divert the buffer to the downstream
 *  components. Example usecase: {PPROC <-> WNR <-> HDR <-> C2D} <-> {IMGLIB
 *  without HDR}
 *
 *  Returns TRUE
 *
 **/
static boolean module_hdr_port_fill_native_input_buffer(
   uint8_t *buff_addr,
   img_frame_t* img_frame,
   isp_buf_divert_t *buf_divert,
   mct_stream_info_t* info,
   mct_stream_map_buf_t *img_buf,
   module_hdr_port_t *hdr_port_data)
{
   int i;
   uint32_t offset=0;
   mct_stream_info_t* input_stream_info =  &hdr_port_data->input_stream_info;

   IDBG_MED("%s +", __func__);
   IDBG_MED("%s Rotation Configuration: %d",__func__,
     info->reprocess_config.pp_feature_config.rotation);

   img_frame->info.width = input_stream_info->dim.width;
   img_frame->info.height = input_stream_info->dim.height;
   img_frame->frame[0].plane_cnt = info->buf_planes.plane_info.num_planes;
   img_frame->idx = buf_divert->buffer.index;

   for (i = 0; i < img_frame->frame[0].plane_cnt; i++) {
      img_frame->frame[0].plane[i].fd = buf_divert->fd;
      img_frame->frame[0].plane[i].plane_type = i;
      img_frame->frame[0].plane[i].stride =
        input_stream_info->buf_planes.plane_info.mp[i].stride;
      img_frame->frame[0].plane[i].scanline =
        input_stream_info->buf_planes.plane_info.mp[i].scanline;
      img_frame->frame[0].plane[i].width = input_stream_info->dim.width;
      img_frame->frame[0].plane[i].height = input_stream_info->dim.height;

      if (CAM_FORMAT_YUV_420_NV21 == input_stream_info->fmt) {
        img_frame->frame[0].plane[i].height /= (i + 1);
      }

      img_frame->frame[0].plane[i].length =
        img_frame->frame[0].plane[i].height *
        img_frame->frame[0].plane[i].stride;

      if (img_buf) {
        img_frame->frame[0].plane[i].offset = img_buf->buf_planes[i].offset;
      } else {
        img_frame->frame[0].plane[i].offset = info->buf_planes.plane_info.mp[i].offset;
      }

      img_frame->frame[0].plane[i].addr = (i == 0) ? buff_addr:
         (buff_addr + (input_stream_info->buf_planes.plane_info.mp[0].scanline
            * input_stream_info->buf_planes.plane_info.mp[0].stride));
   }
   img_frame->timestamp = buf_divert->buffer.timestamp.tv_sec *
     1000000 + buf_divert->buffer.timestamp.tv_usec;
   img_frame->frame_cnt = 1;
   img_frame->info.ss = IMG_H2V2;

   img_frame->info.analysis = 0;
   img_frame->idx = buf_divert->buffer.index;
   //Set width to the same as stride before diverting it to the downstream
   //module
   img_frame->info.width = img_frame->frame[0].plane[IC].stride =
                                      img_frame->frame[0].plane[IY].stride;
   IDBG_MED("%s -", __func__);
   return true;
}

/** module_hdr_port_fill_input_buffer:
 *  @buf_addr: virtual address of the native buffer
 *  @img_frame: image frame handle
 *  @buf_divert: buf divert message
 *  @info: stream info configuration
 *  @img_buff: stream buffer map list
 *
 *  Function to fill image buffer handler descriptor. Need not take care of
 *  rotation configuration when HDR is the sink module.
 *  Example usecase: {PPROC <-> WNR <-> C2D} <-> {IMGLIB + HDR}
 *
 *  Returns TRUE
 **/
static boolean module_hdr_port_fill_input_buffer(
  uint8_t *buff_addr, img_frame_t* img_frame, isp_buf_divert_t *buf_divert,
  mct_stream_info_t* info, mct_stream_map_buf_t *img_buf)
{
   int i;
   uint32_t offset=0;

   IDBG_MED("%s +", __func__);
   for (i = 0; i < img_frame->frame[0].plane_cnt; i++) {
      img_frame->frame[0].plane[i].fd = buf_divert->fd;
      img_frame->frame[0].plane[i].plane_type = i;

      img_frame->frame[0].plane[i].stride =
         info->buf_planes.plane_info.mp[i].stride;
      img_frame->frame[0].plane[i].scanline =
         info->buf_planes.plane_info.mp[i].scanline;

      img_frame->frame[0].plane[i].height = info->dim.height;
      if (CAM_FORMAT_YUV_420_NV21 == info->fmt) {
         img_frame->frame[0].plane[i].height /= (i + 1);
      }

      img_frame->frame[0].plane[i].width = info->dim.width;
      img_frame->frame[0].plane[i].length =
      img_frame->frame[0].plane[i].height*img_frame->frame[0].plane[i].stride;
      if (img_buf) {
         img_frame->frame[0].plane[i].offset = img_buf->buf_planes[i].offset;
      } else {
         img_frame->frame[0].plane[i].offset = offset;
         offset += img_frame->frame[0].plane[i].stride *
           img_frame->frame[0].plane[i].scanline;
      }
      img_frame->frame[0].plane[i].addr = buff_addr;
   }
   img_frame->timestamp = buf_divert->buffer.timestamp.tv_sec * 1000000
      + buf_divert->buffer.timestamp.tv_usec;
   img_frame->frame_cnt = 1;
   img_frame->info.width = info->dim.width;
   img_frame->info.height = info->dim.height;
   img_frame->info.ss = IMG_H2V2;
   img_frame->info.analysis = 0;
   img_frame->idx = buf_divert->buffer.index;
   IDBG_MED("%s -", __func__);
   return true;
}

/** module_hdr_port_fill_divert_buffer_handler:
 *  @img_frame: image buffer handler descriptor
 *  @buf_divert: buf divert message
 *  @info: stream info configuration
 *  @is_srcport_connected: source port connection status flag
 *  @hdr_port_data: hdr port private data
 *
 * Function to fill image buffer handler descriptor
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_fill_divert_buffer_handler(
  img_frame_t* img_frame,
  isp_buf_divert_t *buf_divert,
  mct_stream_info_t* info,
  boolean is_srcport_connected,
  module_hdr_port_t *hdr_port_data)
{
  boolean ret_val = FALSE;
  uint8_t *ptr = NULL;
  mct_stream_map_buf_t *img_buf = NULL;
  mct_list_t *img_buf_list = NULL;
  uint32_t i;
  uint32_t offset=0;

  IDBG_MED("%s +", __func__);

  if (CAM_FORMAT_YUV_420_NV21 == info->fmt) {
    /*
     * Get buffer virtual address pointer:
     * If native buffer, get directly from buf_divert->vaddr pointer
     * else get buf index from buf divert and get virtual address by matching
     * the buff divert buffer index to image buffer list in the stream
     * */
    if (buf_divert->native_buf) {
      ptr = buf_divert->vaddr;
    } else {
      img_buf_list = mct_list_find_custom(info->img_buffer_list,
        &buf_divert->buffer.index, module_hdr_port_validate_buff_index);

      if (img_buf_list)
        img_buf = (mct_stream_map_buf_t *)img_buf_list->data;

      if (img_buf) {
        ptr = (uint8_t *)img_buf->buf_planes[0].buf;
      }
    }

    if (ptr) {
      img_frame->frame[0].plane_cnt = info->buf_planes.plane_info.num_planes;
      if (buf_divert->native_buf && is_srcport_connected) {
         ret_val = module_hdr_port_fill_native_input_buffer(ptr,
           img_frame, buf_divert, info, img_buf, hdr_port_data);
      } else {
         ret_val = module_hdr_port_fill_input_buffer(ptr,
           img_frame, buf_divert, info, img_buf);
      }

      IDBG_MED("%s:%d] dim %dx%d frame %p y-stride %d scanline %d", __func__,
         __LINE__, img_frame->info.width, img_frame->info.height,
         img_frame, img_frame->frame[0].plane[IY].stride,
         img_frame->frame[0].plane[IY].scanline);

      IDBG_MED("%s:%d] y wxh %dx%d chroma wxh %dx%d, c-stride %d scanline %d",
          __func__, __LINE__, img_frame->frame[0].plane[IY].width,
          img_frame->frame[0].plane[IY].height,
          img_frame->frame[0].plane[IC].width,
          img_frame->frame[0].plane[IC].height,
          img_frame->frame[0].plane[IC].stride,
          img_frame->frame[0].plane[IC].scanline);
      IDBG_MED("%s:%d] vaddr %p fd %d\n", __func__, __LINE__,
          img_frame->frame[0].plane[0].addr,
          img_frame->frame[0].plane[0].fd);
    } else {
      IDBG_ERROR("Null pointer detected in %s\n", __func__);
    }
  } else {
    IDBG_ERROR("Only NV21 format is supported in hdr lib");
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_fill_buffer_handler:
 *  @img_frame: image buffer handler descriptor
 *  @mct_stream_map_buf: mct stream map buffer
 *  @info: stream info configuration
 *
 * Function to fill image buffer handler descriptor
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_fill_buffer_handler(img_frame_t* img_frame,
  mct_stream_map_buf_t *mct_stream_map_buf, mct_stream_info_t* info)
{
  boolean ret_val = FALSE;
  struct timeval timestamp;
  uint32_t i;

  IDBG_MED("%s +", __func__);

  if (CAM_FORMAT_YUV_420_NV21 == info->fmt) {
    img_frame->frame[0].plane_cnt = mct_stream_map_buf->num_planes;

    for (i = 0; i < mct_stream_map_buf->num_planes; i++) {
      img_frame->frame[0].plane[i].plane_type = i;
      img_frame->frame[0].plane[i].addr = mct_stream_map_buf->buf_planes[i].buf;
      img_frame->frame[0].plane[i].stride =
        mct_stream_map_buf->buf_planes[i].stride;
      img_frame->frame[0].plane[i].fd = mct_stream_map_buf->buf_planes[i].fd;
      img_frame->frame[0].plane[i].height = info->dim.height;
      if (CAM_FORMAT_YUV_420_NV21 == info->fmt)
        img_frame->frame[0].plane[i].height /= (i + 1);
      img_frame->frame[0].plane[i].width = info->dim.width;
      img_frame->frame[0].plane[i].length = img_frame->frame[0].plane[i].height
        * img_frame->frame[0].plane[i].stride;
      img_frame->frame[0].plane[i].offset =
        mct_stream_map_buf->buf_planes[i].offset;
    }

    gettimeofday(&timestamp, NULL);
    img_frame->timestamp = timestamp.tv_sec * 1000000 + timestamp.tv_usec;

    img_frame->frame_cnt = 1;
    img_frame->info.width = info->dim.width;
    img_frame->info.height = info->dim.height;
    img_frame->info.ss = IMG_H2V2;

    img_frame->info.analysis = 0;
    img_frame->idx = mct_stream_map_buf->buf_index;

    ret_val = TRUE;
  } else
    IDBG_ERROR("Only NV21 format supported in hdr lib");

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_find_buff:
 *  @list_data: buffer instance
 *  @user_data: required buffer index
 *
 * Function to get specified buffer
 *
 * Returns Handler to mapped buffer or NULL
 **/
static boolean module_hdr_port_find_buff(void *list_data, void *user_data)
{
  boolean ret_val = FALSE;
  mct_stream_map_buf_t *img_buf = list_data;
  int32_t *buff_index = user_data;

  IDBG_LOW("%s +", __func__);

  if (img_buf && buff_index) {
    if (*buff_index == (int32_t)img_buf->buf_index)
      ret_val = TRUE;
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_LOW("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_find_stream:
 *  @list_data: stream instance
 *  @user_data: required stream id
 *
 * Function to get pecified stream
 *
 * Returns Handler to mapped buffer or NULL
 **/
static boolean module_hdr_port_find_stream(void *list_data, void *user_data)
{
  boolean ret_val = FALSE;
  mct_stream_t *stream = (mct_stream_t *)list_data;
  uint32_t *stream_index = user_data;

  IDBG_LOW("%s +", __func__);

  if (stream && stream_index) {
    if (*stream_index == stream->streamid)
      ret_val = TRUE;
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_LOW("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_release_img_buffer:
 *  @buff: image buffer handler
 *
 * Function to release image buffer
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_release_img_buffer(module_hdr_buf_t **buff)
{
  module_hdr_buf_t *img_buff;

  IDBG_MED("%s +", __func__);

  if (buff && *buff) {
    img_buff = *buff;
    if (img_buff->img_frame) {
      free(img_buff->img_frame);
      img_buff->img_frame = 0;
    }
    free(img_buff);
    *buff = 0;
  }

  IDBG_MED("%s -", __func__);

  return TRUE;
}

/** module_hdr_port_get_diverted_input_buffer:
 *  @port: the Port to handle the event
 *  @event: the Event to be handled
 *  @info: stream info configuration
 *  @is_srcport_connected: source port connection status flag
 * Function to get diverted input buffer for hdr port
 *
 * Returns Handler to mapped buffer or NULL
 **/
static module_hdr_buf_t *module_hdr_port_get_diverted_input_buffer(
  mct_port_t *port, mct_event_t *event,
  mct_stream_info_t* info, boolean is_srcport_connected)
{
  boolean done = FALSE;
  module_hdr_buf_t *ret_val = NULL;
  isp_buf_divert_t *buf_divert;
  mct_stream_map_buf_t *img_buf = NULL;
  module_hdr_port_t *private_data;
  mct_module_t *module;
  mct_stream_info_t* stream_info = NULL;

  IDBG_MED("%s +", __func__);

  buf_divert = event->u.module_event.module_event_data;

  if (buf_divert && port && port->port_private && MCT_PORT_PARENT(port) &&
    (MCT_PORT_PARENT(port) )->data && info) {

    //allocate hdr buff container structure
    ret_val = calloc(1, sizeof(module_hdr_buf_t));

    if (ret_val) {
      ret_val->img_frame = calloc(1, sizeof(img_frame_t));
      if (ret_val->img_frame) {

        private_data = port->port_private;
        ret_val->img_frame->frame_id = buf_divert->buffer.sequence;
        ret_val->channel_id = buf_divert->channel_id;
        ret_val->meta_data = buf_divert->meta_data;
        ret_val->subdev_fd = private_data->subdev_fd;
        ret_val->is_skip_pproc = buf_divert->is_skip_pproc;

        if (buf_divert->native_buf) {
          ret_val->is_native = TRUE;
        } else {
          ret_val->is_native = FALSE;
        }

        ret_val->identity = info->identity;
        stream_info = info;

        IDBG_LOW("%s is_native: %d stream_type: %d fmt: %d",__func__,
          ret_val->is_native, stream_info->stream_type, stream_info->fmt);
        IDBG_LOW("%s width: %d height: %d va: %p fd:%d", __func__,
          stream_info->dim.width, stream_info->dim.height,
          buf_divert->vaddr, buf_divert->fd);
        if (stream_info) {
          done = module_hdr_port_fill_divert_buffer_handler(ret_val->img_frame,
             buf_divert, stream_info, is_srcport_connected, private_data);
          IDBG_HIGH("Input buf with idx %d id %d from stream id 0x%x is found",
            ret_val->img_frame->idx,
            ret_val->img_frame->frame_id,
            ret_val->identity);
        } else {
          IDBG_ERROR("Failed to find input stream info in %s", __func__);
        }
      } else {
        IDBG_ERROR("Failed to allocate memory in %s", __func__);
      }
    } else {
      IDBG_ERROR("Failed to allocate memory in %s", __func__);
    }
  } else {
    IDBG_ERROR("Null pointer detected in %s", __func__);
  }

  if (!done) {
    module_hdr_port_release_img_buffer(&ret_val);
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_get_input_buffer:
 *  @port: the Port to handle the event
 *  @event: the Event to be handled
 *  @info: stream info configuration
 *
 * Function to get input buffer for hdr port
 *
 * Returns Handler to mapped buffer or NULL
 **/
static module_hdr_buf_t *module_hdr_port_get_input_buffer(mct_port_t *port,
  mct_event_t *event, mct_stream_info_t* info)
{
  boolean done = FALSE;
  module_hdr_buf_t *ret_val;
  mct_module_t *module;
  mct_list_t *list;
  mct_stream_info_t* stream_info = NULL;

  IDBG_MED("%s +", __func__);

  ret_val = calloc(1, sizeof(module_hdr_buf_t));

  if (ret_val) {
    ret_val->img_frame = calloc(1, sizeof(img_frame_t));

    if (ret_val->img_frame) {

      if (CAM_ONLINE_REPROCESS_TYPE == info->reprocess_config.pp_type) {

        /* when the reprocess stream is online stream, input data comes a
         * actual post view, snapshot or preview stream and not from the
         * reprocess stream. Reprocess config contains the stream info
         * embedded as reprocess_config.online.input_stream_id
         *
         * For offline stream, get the buffer directly from the stream
         * */

        module = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
        stream_info =
          (mct_stream_info_t *)mct_module_get_stream_info(module,
            IMGLIB_SESSIONID(info->identity),
            info->reprocess_config.online.input_stream_id);

      } else {
        stream_info = info;
      }

      if (stream_info) {
        list = mct_list_find_custom(stream_info->img_buffer_list,
          &info->parm_buf.reprocess.buf_index, module_hdr_port_find_buff);

        if (list && list->data) {
          if (module_hdr_port_fill_buffer_handler(ret_val->img_frame,
            list->data, stream_info)) {

            ret_val->is_native = TRUE;
            ret_val->img_frame->frame_id = info->parm_buf.reprocess.frame_idx;
            ret_val->identity =
              IMGLIB_PACK_IDENTITY(IMGLIB_SESSIONID(info->identity),
                info->reprocess_config.online.input_stream_id);

            done = TRUE;

            IDBG_HIGH("Input buf with index %d id %d stream id 0x%x is found",
              info->parm_buf.reprocess.buf_index,
              ret_val->img_frame->frame_id,
              info->reprocess_config.online.input_stream_id);
          } else
            IDBG_ERROR("Unsupported buffer in %s", __func__);
        }
      }  else
        IDBG_ERROR("Failed to find input stream info in %s", __func__);
    } else
      IDBG_ERROR("Failed to allocate memory in %s", __func__);
  } else
    IDBG_ERROR("Failed to allocate memory in %s", __func__);

  if (!done) {
    module_hdr_port_release_img_buffer(&ret_val);

    if (CAM_ONLINE_REPROCESS_TYPE == info->reprocess_config.pp_type)
      IDBG_ERROR("Failed to find input buf with index %d from stream id %d",
        info->parm_buf.reprocess.buf_index,
        info->reprocess_config.online.input_stream_id);
    else
      IDBG_ERROR("Failed to find input buf with index %d",
        info->parm_buf.reprocess.buf_index);
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_get_output_buffer:
 *  @port: the Port to handle the event
 *  @event: the Event to be handled
 *  @info: stream info configuration
 *  @frame_id: output frame id
 *
 * Function to get buffer for hdr port
 *
 * Returns Handler to mapped buffer or NULL
 **/
static module_hdr_buf_t *module_hdr_port_get_output_buffer(mct_port_t *port,
  mct_event_t *event, mct_stream_info_t* info, uint32_t frame_id)
{
  boolean done = FALSE;
  struct msm_buf_mngr_info buff;
  module_hdr_buf_t* ret_val = NULL;
  mct_list_t* list;
  int32_t ret;
  module_hdr_port_t *private_data;

  IDBG_MED("%s +", __func__);

  if (port && port->port_private) {
    private_data = port->port_private;

    ret_val = calloc(1, sizeof(module_hdr_buf_t));

    if (ret_val) {
      ret_val->img_frame = calloc(1, sizeof(img_frame_t));
      if (ret_val->img_frame) {
        ret_val->subdev_fd = private_data->subdev_fd;
        buff.session_id = IMGLIB_SESSIONID(event->identity);
        buff.stream_id = IMGLIB_STREAMID(event->identity);
        ret = ioctl(ret_val->subdev_fd, VIDIOC_MSM_BUF_MNGR_GET_BUF, &buff);
        if (ret >= 0) {
          list = mct_list_find_custom(info->img_buffer_list, &buff.index,
            module_hdr_port_find_buff);

          if (list && list->data) {
            IDBG_HIGH("Output buf with index %d from stream id %d is found",
              buff.index, buff.stream_id);
            ret_val->img_frame->frame_id = frame_id;
            ret_val->identity = event->identity;
            ret_val->is_native = FALSE;

            if (module_hdr_port_fill_buffer_handler(ret_val->img_frame,
              list->data, info))
              done = TRUE;
            else
              IDBG_ERROR("Unsupported buffer in %s", __func__);
          } else
            ioctl(ret_val->subdev_fd, VIDIOC_MSM_BUF_MNGR_PUT_BUF, &buff);
        } else
          IDBG_ERROR("Failed to get buffer from buffer manager");
      } else
        IDBG_ERROR("Failed to allocate memory in %s", __func__);
    } else
      IDBG_ERROR("Failed to allocate memory in %s", __func__);
  } else
    IDBG_ERROR("Null pointer detected in %s", __func__);

  if (!done) {
    if (ret_val) {
      if (ret_val->img_frame) {
        if (ret_val->subdev_fd >= 0)
          close(ret_val->subdev_fd);
        free(ret_val->img_frame);
      }
      free(ret_val);
      ret_val = 0;
      IDBG_ERROR("Failed to find output buffer from session %d stream %d",
        IMGLIB_SESSIONID(event->identity), IMGLIB_STREAMID(event->identity));
    }
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_get_metadata_buffer:
 *  @port: the Port to handle the event
 *  @event: the Event to be handled
 *  @info: stream info configuration
 *
 * Function to get metadata buffer for hdr port
 *
 * Returns Handler to mapped buffer or NULL
 **/
static void *module_hdr_port_get_metadata_buffer(mct_port_t *port,
  mct_event_t *event, mct_stream_info_t* info)
{
  void* ret_val = NULL;

  IDBG_MED("%s +", __func__);

  if (port && MCT_PORT_PARENT(port) && event && info) {

    if (info->reprocess_config.pp_type == CAM_ONLINE_REPROCESS_TYPE) {
      ret_val = mct_module_get_buffer_ptr(
        info->parm_buf.reprocess.meta_buf_index,
        (MCT_PORT_PARENT(port) )->data,
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

/** module_hdr_port_release_buffer:
 *  @buffer: buffer handler
 *  @identity: stream identity
 *  @buff_done: flag indicating whether to use buff done
 *                                      or just return the buffer
 *
 * Function to release output buffer
 *
 * Returns TRUE out case of success
 **/
static boolean module_hdr_port_release_buffer(module_hdr_buf_t **buffer,
  uint32_t identity, boolean buff_done)
{
  struct msm_buf_mngr_info buff_info;
  module_hdr_buf_t *buff;
  boolean ret_val = FALSE;
  int cmd;
  int ret;

  IDBG_MED("%s +", __func__);

  if (buffer && *buffer && identity) {
    buff = *buffer;
    if (buff->img_frame) {
      if (!buff->is_native) {
        buff_info.index = buff->img_frame->idx;
        buff_info.session_id = IMGLIB_SESSIONID(identity);
        buff_info.stream_id = IMGLIB_STREAMID(identity);
        buff_info.frame_id = buff->img_frame->frame_id;
        buff_info.timestamp.tv_sec = buff->img_frame->timestamp / 1000000;
        buff_info.timestamp.tv_usec = buff->img_frame->timestamp
          - buff_info.timestamp.tv_sec * 1000000;

        IDBG_HIGH("%s buff_done %d", __func__ , buff_done);
        IDBG_HIGH("%s buff_info.index 0x%x", __func__ , buff_info.index);
        IDBG_HIGH("%s buff_info.frame_id %d", __func__ , buff_info.frame_id);
        IDBG_HIGH("%s identity 0x%x", __func__ , identity);

        if (buff_done)
          cmd = VIDIOC_MSM_BUF_MNGR_BUF_DONE;
        else
          cmd = VIDIOC_MSM_BUF_MNGR_PUT_BUF;
        ret = ioctl(buff->subdev_fd, cmd, &buff_info);

        if (ret >= 0)
          ret_val = TRUE;
        else
          IDBG_ERROR("Failed to do buf_done in %s", __func__);
      }

      free(buff->img_frame);
      buff->img_frame = 0;
    }

    free(buff);
    *buffer = 0;
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_release_metadata_buffer:
 *  @metadata_buff: metadata buffer handler
 *
 * Function to release metadata buffer
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_release_metadata_buffer(void *metadata_buff)
{
  IDBG_MED("%s +", __func__);

  // Do nothing

  IDBG_MED("%s -", __func__);

  return TRUE;
}

/** module_hdr_port_check_config_list_frame_id
 *    @data1: module_frame_config_t object
 *    @data2: frame id to be checked
 *
 *  Checks if this frame configuration is for specified frame id
 *
 *  Return TRUE if the port has the same identity and is linked
 **/
static boolean module_hdr_port_check_config_list_frame_id(void *data1,
  void *data2)
{
  boolean ret_val = FALSE;
  module_frame_config_t *frame_config = (module_frame_config_t *)data1;
  uint32_t *id = (uint32_t *)data2;

  IDBG_MED("%s +", __func__);

  if (id && frame_config && frame_config->out_buff
    && (*id == frame_config->out_buff->img_frame->frame_id))
    ret_val = TRUE;

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_forward_event_to_peer
 *    @data: mct_port_t object
 *    @user_data: event to be forwarded
 *
 * Forwards event to peer with same identity
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_forward_event_to_peer(void *data,
  void *user_data)
{
  boolean ret_val = FALSE;
  mct_port_t *port = (mct_port_t *)data;
  mct_port_t *peer;
  mct_event_t *event = user_data;
  module_hdr_port_t *private_data;

  IDBG_MED("%s +", __func__);

  if (port && event && MODULE_HDR_VALIDATE_NAME(port) && port->port_private
    && MCT_PORT_PEER(port) ) {

    peer = MCT_PORT_PEER(port);
    private_data = port->port_private;

    if (private_data->reserved_identity == event->identity) {

      IDBG_MED("%s: event send to peer", __func__);
      ret_val = MCT_PORT_EVENT_FUNC(peer) (peer, event);
      IDBG_MED("%s: event sent ret_val 0x%x", __func__, ret_val);

    } else
      ret_val = TRUE;
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_send_buff
 *    @port: the Port that handles the event
 *    @buff: buffer handler
 *
 * Sends buff divert to peer
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_send_buff(mct_port_t* port,
  module_hdr_buf_t *buff)
{
  boolean ret_val = FALSE;
  module_hdr_port_t *private_data;
  isp_buf_divert_t isp_buf_divert;
  mct_event_t event;

  IDBG_MED("%s +", __func__);

  if (buff && port && port->port_private && MCT_PORT_PARENT(port) ) {
    private_data = port->port_private;

    memset(&isp_buf_divert, 0, sizeof(isp_buf_divert_t));

    if (TRUE == buff->is_native) {
      isp_buf_divert.native_buf = TRUE;
      isp_buf_divert.vaddr = buff->img_frame->frame[0].plane[0].addr;
      isp_buf_divert.fd = buff->img_frame->frame[0].plane[0].fd;
    } else {
      isp_buf_divert.native_buf = FALSE;
      isp_buf_divert.vaddr = NULL;
      isp_buf_divert.fd = buff->subdev_fd;
    }

    isp_buf_divert.buffer.sequence = buff->img_frame->frame_id;
    isp_buf_divert.buffer.length = buff->img_frame->frame[0].plane_cnt;
    isp_buf_divert.buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    isp_buf_divert.buffer.index = buff->img_frame->idx;
    isp_buf_divert.buffer.memory = V4L2_MEMORY_USERPTR;
    gettimeofday(&isp_buf_divert.buffer.timestamp, NULL);

    isp_buf_divert.is_locked = FALSE;
    isp_buf_divert.ack_flag = FALSE;
    isp_buf_divert.is_buf_dirty = FALSE;
    isp_buf_divert.identity = private_data->reserved_identity;
    isp_buf_divert.channel_id = buff->channel_id;
    isp_buf_divert.meta_data = buff->meta_data;
    isp_buf_divert.is_skip_pproc = buff->is_skip_pproc;

    event.identity = isp_buf_divert.identity;
    event.type = MCT_EVENT_MODULE_EVENT;
    event.direction = MCT_EVENT_DOWNSTREAM;
    event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
    event.u.module_event.module_event_data = (void *)&isp_buf_divert;

    IDBG_HIGH("%s isp_buf_divert:native_buf %d fd %d vaddr %p", __func__ ,
      isp_buf_divert.native_buf, isp_buf_divert.fd, isp_buf_divert.vaddr);
    IDBG_HIGH("%s isp_buf_divert.buffer.index 0x%x", __func__ ,
      isp_buf_divert.buffer.index);
    IDBG_HIGH("%s isp_buf_divert.buffer.sequence %d", __func__ ,
      isp_buf_divert.buffer.sequence);
    IDBG_HIGH("%s identity 0x%x", __func__ , isp_buf_divert.identity);

    ret_val = mct_list_traverse(
      MCT_MODULE_SRCPORTS(MCT_PORT_PARENT(port)->data),
      module_hdr_port_forward_event_to_peer, &event);

    if (!ret_val)
      IDBG_ERROR("Cannot send MCT_EVENT_MODULE_BUF_DIVERT in %s\n", __func__);
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_notify_cb
 *    @user_data: user data
 *    @out_buff: output buffer handler
 *    @in_buff: input buffer handler
 *    @out_crop: output crop
 *
 *
 * Module hdr library process done callback
 *
 * Returns Nothing
 **/
static void module_hdr_port_notify_cb(void* user_data,
  module_hdr_buf_t **out_buff, module_hdr_buf_t **in_buff,
  module_hdr_crop_t* out_crop)
{
  boolean found = FALSE;
  module_frame_config_t frame_config;
  mct_list_t *list_match;
  boolean send_frame_to_next_module = FALSE;
  mct_port_t *port = user_data;
  module_hdr_port_t *private_data;
  module_hdr_crop_t* out_crop_node;
  int32_t i;
  uint32_t j;
  boolean ret_val;

  IDBG_MED("%s +", __func__);

  if (out_buff && in_buff && port && port->port_private && out_crop
    && ((module_hdr_port_t *)port->port_private)->lib_instance) {
    private_data = port->port_private;

    if (private_data->hdr_need_1x) {
      i = 2;
    } else {
      i = 1;
    }

    if (pthread_mutex_lock(&private_data->mutex_crop_output_queue))
      CDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);

    while (i--) {
      out_crop_node = calloc(1, sizeof(module_hdr_crop_t));

      if (out_crop_node) {
        memcpy(out_crop_node, out_crop, sizeof(module_hdr_crop_t));

        mct_queue_push_tail(&private_data->crop_output_queue, out_crop_node);

      } else
        IDBG_ERROR("Not enough memory in %s\n", __func__);
    }

    if (pthread_mutex_unlock(&private_data->mutex_crop_output_queue))
          CDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);

    for (i = 0; i < HDR_LIB_OUT_BUFFS + HDR_LIB_INPLACE_BUFFS; i++) {
      //dump output buffer for debug purpose
      if (private_data->dump_output_frame) {
        mod_imglib_dump_frame(private_data->out_buff[i]->img_frame,
          "hdr_output",
          i);
      }

      if (pthread_mutex_lock(&private_data->mutex_config_list))
        IDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);

      list_match = mct_list_find_custom(private_data->config_list,
        &out_buff[i]->img_frame->frame_id, module_hdr_port_check_config_list_frame_id);

      send_frame_to_next_module = FALSE;

      if (list_match && list_match->data) {
        found = TRUE;
        frame_config = *(module_frame_config_t *)list_match->data;

        if (frame_config.src_port_linked)
          send_frame_to_next_module = TRUE;
        else {
          free(list_match->data);
          private_data->config_list = mct_list_remove(private_data->config_list,
            list_match->data);
        }
      } else
        IDBG_ERROR("Memory corruption in %s:%d \n", __func__, __LINE__);

      if (pthread_mutex_unlock(&private_data->mutex_config_list))
        IDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);

      if (i >= HDR_LIB_OUT_BUFFS) {
        if (!module_hdr_lib_get_output_inplace_index(i - HDR_LIB_OUT_BUFFS,
          &j)) {
            IDBG_ERROR("Cannot get index of inplace buffer in %s\n", __func__);
            continue;
        }

        if (IMG_SUCCESS != img_image_copy(out_buff[i]->img_frame,
          private_data->in_buff[j]->img_frame)) {
          IDBG_ERROR("Cannot copy inplace buffer in %s\n", __func__);
        }
      }

      if (send_frame_to_next_module) {
        module_hdr_port_send_buff(port, out_buff[i]);
        module_hdr_port_release_img_buffer(&private_data->out_buff[i]);
      } else {
        module_hdr_port_release_buffer(&private_data->out_buff[i],
          private_data->reserved_identity, TRUE);
      }

    }

    if (private_data->non_hdr_buf) {
      if (IMG_SUCCESS != img_image_copy(private_data->non_hdr_buf->img_frame,
        private_data->in_buff[private_data->non_hdr_buf_index]->img_frame)) {
        IDBG_ERROR("Cannot copy nonHDR frame in %s\n", __func__);
      }

      if (send_frame_to_next_module) {
        module_hdr_port_send_buff(port, private_data->non_hdr_buf);
        module_hdr_port_release_img_buffer(&private_data->non_hdr_buf);
      } else {
        module_hdr_port_release_buffer(&private_data->non_hdr_buf,
          private_data->reserved_identity, TRUE);
      }
    }

    for (j = 0; j < private_data->hdr_port_in_buffs; j++)
      module_hdr_port_release_img_buffer(&private_data->in_buff[j]);

    module_hdr_port_release_metadata_buffer(private_data->metadata_buff);
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);
}

/** module_hdr_port_get_lib_buf_idx:
 *  @port: the Port which received the event
 *  @event: the Event
 *  @info: Stream info structure
 *
 * Determine if this is 0.5x exposure image, 2x exposure image or something else
 * idx 0 corresponds to 0.5x exposure and idx 1 corresponds to 2x exposure image
 * If the idx is something else, the buffer should be released back with no
 * processing
 *
 * Returns idx
 **/
static uint32_t module_hdr_port_get_lib_buf_idx(mct_port_t *port,
  mct_event_t *event, mct_stream_info_t *info)
{
  module_hdr_port_t *private_data;
  uint32_t idx = 0;

  IDBG_MED("%s +", __func__);

  if (port && event && info && port->port_private) {
    private_data = port->port_private;

    idx = private_data->hdr_burst_counter;
    private_data->hdr_burst_counter++;

    IDBG_LOW("%s: ID returned is :%d\n", __func__, idx);
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return idx;
}

/** module_hdr_port_negative_check_linked_peer
 *    @data: mct_port_t object
 *    @user_data: event to be forwarded
 *
 * Check whether peer is linked to the specified identity
 *
 * Returns FLASE in case of success,
 *   otherwise TRUE need to be returned for iterating the next item in the list
 **/
static boolean module_hdr_port_negative_check_linked_peer(void *data,
  void *user_data)
{
  boolean ret_val = FALSE;
  mct_port_t *port = (mct_port_t *)data;
  mct_port_t *peer;
  mct_event_t *event = user_data;
  module_hdr_port_t *private_data;

  IDBG_MED("%s +", __func__);

  if (port && event && MODULE_HDR_VALIDATE_NAME(port) && port->port_private
    && MCT_PORT_PEER(port) ) {

    peer = MCT_PORT_PEER(port);
    private_data = port->port_private;

    if (private_data->reserved_identity == event->identity) {
      ret_val = FALSE;
    } else {
      ret_val = TRUE;
    }
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_isp_output_dim_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be handled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle ISP output dimension event for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_isp_output_dim_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  module_hdr_port_t *private_data;
  IDBG_MED("%s + forward_event %d", __func__, *forward_event);
  if (port && event && port->port_private
    && event->u.module_event.module_event_data) {
    mct_stream_info_t *stream_info =
      (mct_stream_info_t *)(event->u.module_event.module_event_data);
    private_data = port->port_private;
    private_data->input_stream_info.dim.width = stream_info->dim.width;
    private_data->input_stream_info.dim.height = stream_info->dim.height;
    memcpy(private_data->input_stream_info.buf_planes.plane_info.mp,
    stream_info->buf_planes.plane_info.mp,
    sizeof(stream_info->buf_planes.plane_info.mp));
    private_data->input_stream_info.fmt = stream_info->fmt;
    IDBG_MED("%s:%d] MCT_EVENT_MODULE_ISP_OUTPUT_DIM %dx%d",
      __func__, __LINE__,
      private_data->input_stream_info.dim.width,
      private_data->input_stream_info.dim.height);
    ret_val = TRUE;
  } else {
    IDBG_ERROR("Null pointer detected in %s\n", __func__);
  }
  IDBG_MED("%s -", __func__);
  return ret_val;
}

/** module_hdr_port_isp_gamma_update_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be handled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle Gamma update event for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_isp_gamma_update_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  module_hdr_port_t *private_data;
  void *gamma;
  uint32_t gamma_size = sizeof(img_gamma_t);
  mct_stream_session_metadata_info* session_meta;

  IDBG_MED("%s +", __func__);

  if (port && event && port->port_private
    && event->u.module_event.module_event_data) {

    private_data = port->port_private;
    gamma = event->u.module_event.module_event_data;
    session_meta = &private_data->session_meta;

    if (sizeof(session_meta->isp_gamma_data.private_data) >= gamma_size) {

      memcpy(&session_meta->isp_gamma_data.private_data, gamma, gamma_size);
      ret_val = TRUE;
      private_data->is_session_meta_valid = TRUE;

    } else {
      IDBG_ERROR("%s: Private data size need to be increased\n", __func__);
      IDBG_ERROR("session_meta->isp_gamma_data.private_data size %d\n",
        sizeof(session_meta->isp_gamma_data.private_data));
      IDBG_ERROR("gamma_size size %d\n", gamma_size);
    }
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_qry_dvrt_evnt:
 *  @port: the Port to handle the event
 *  @event: the Event to be handled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle query divert type event for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_qry_dvrt_evnt(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;

  IDBG_MED("%s +", __func__);

  if (port && event && event->u.module_event.module_event_data) {
    uint32_t *divert_mask = event->u.module_event.module_event_data;
    *divert_mask |= PPROC_DIVERT_PROCESSED;
    ret_val = TRUE;
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_issourceport_connected:
 *  @mod: module handler
 *  @event: the event to be handled
 *
 * Function to check the connection status of source port
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_issourceport_connected(mct_module_t *mod,
  mct_event_t *event)
{
  mct_list_t *list_match = NULL;
  boolean val = FALSE;
  list_match = mct_list_find_custom(MCT_MODULE_SRCPORTS(mod),
    &event->identity, module_hdr_port_check_linked_port_identity);
  IDBG_LOW("%s list_match = %p", __func__, list_match);
  if (list_match && list_match->data) {
     val = TRUE;
  }
  return val;
}

/** module_hdr_port_stream_on_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be handled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle stream on command for the port.
 * Do nothing for HDR module
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_stream_on_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  module_hdr_port_t *private_data;
  mct_stream_info_t *info;
  mct_module_t *module = NULL;
#ifdef _ANDROID_
  char value[PROPERTY_VALUE_MAX];
#endif

  IDBG_MED("%s +", __func__);

  if (port && event && port->port_private &&
       ((MCT_OBJECT_PARENT(port)->data))) {

    module = MCT_MODULE_CAST(MCT_OBJECT_PARENT(port)->data);

    private_data = port->port_private;

    private_data->hdr_burst_counter = 0;

    private_data->dump_input_frame = FALSE;
    private_data->dump_output_frame = FALSE;

#ifdef _ANDROID_
    property_get("persist.camera.imglib.hdr.dump",
      value,
      MODULE_HDR_PROPERTY_DUMP_DISABLE);

    if (!strncmp(MODULE_HDR_PROPERTY_IN_DUMP_ENABLE,
      value,
      sizeof(MODULE_HDR_PROPERTY_IN_DUMP_ENABLE))) {
        private_data->dump_input_frame = TRUE;
    } else if (!strncmp(MODULE_HDR_PROPERTY_OUT_DUMP_ENABLE,
      value,
      sizeof(MODULE_HDR_PROPERTY_OUT_DUMP_ENABLE))) {
        private_data->dump_output_frame = TRUE;
    } else if (!strncmp(MODULE_HDR_PROPERTY_IN_OUT_DUMP_ENABLE,
      value,
      sizeof(MODULE_HDR_PROPERTY_IN_OUT_DUMP_ENABLE))) {
        private_data->dump_input_frame = TRUE;
        private_data->dump_output_frame = TRUE;
    }
#endif

    private_data->stream_on = TRUE;
    if (module) {
      private_data->is_srcport_connected =
        module_hdr_port_issourceport_connected(module, event);
      ret_val = TRUE;
    } else {
      ret_val = FALSE;
      IDBG_ERROR("Invalid hdr module detected %s\n", __func__);
    }
    IDBG_LOW("%s is_srcport_connected: %d", __func__, private_data->is_srcport_connected);
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_stream_off_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be handled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle stream off command for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_stream_off_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  module_hdr_port_t *private_data;
  uint32_t i, j;
  boolean found, is_connected;
  uint32_t inplace_indexes_count;
  uint32_t inplace_indexes[HDR_LIB_INPLACE_BUFFS];

  IDBG_MED("%s +", __func__);

  if (port && event && port->port_private) {

    private_data = port->port_private;

    private_data->stream_on = FALSE;

    ret_val = module_hdr_lib_abort(private_data->lib_instance);

    IDBG_LOW("%s is_srcport_connected: %d", __func__, private_data->is_srcport_connected);

    if (pthread_mutex_lock(&private_data->mutex_config_list)) {
      IDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);
    }

    mct_list_traverse(private_data->config_list,
      module_hdr_port_queue_free_func, NULL);
    mct_list_free_list(private_data->config_list);
    private_data->config_list = NULL;

    if (pthread_mutex_unlock(&private_data->mutex_config_list)) {
      IDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);
    }

    inplace_indexes_count = 0;
    for (i = 0; i < HDR_LIB_INPLACE_BUFFS; i++) {
      if (!module_hdr_lib_get_output_inplace_index(i,
        &inplace_indexes[inplace_indexes_count])) {
          IDBG_ERROR("Cannot get index of inplace buffer in %s\n", __func__);
          continue;
      }
      inplace_indexes_count++;
    }

    for (i = 0; i < IMGLIB_ARRAY_SIZE(private_data->in_buff); i++) {
      if (private_data->in_buff[i] && !private_data->in_buff[i]->is_native) {
        // Skip inplace non native buffers since they will be released as output
        found = FALSE;
        j = inplace_indexes_count;
        while(j) {
          if (i == inplace_indexes[--j]) {
            found = TRUE;
            break;
          }
        }
        if (found) {
          // Clear handle since it will be released as output
          private_data->in_buff[i] = 0;
          continue;
        }
      }

      module_hdr_port_release_buffer(&private_data->in_buff[i],
        event->identity, FALSE);
    }

    module_hdr_port_release_buffer(&private_data->non_hdr_buf,
      event->identity, FALSE);

    //hdr output buffers will not be obtained when hdr is src component, so need
    //not release them
    if (!private_data->is_srcport_connected) {
       for (i=0; i < IMGLIB_ARRAY_SIZE(private_data->out_buff); i++) {
         module_hdr_port_release_buffer(&private_data->out_buff[i],
         event->identity, FALSE);
      }
    }
  } else {
    IDBG_ERROR("Null pointer detected in %s\n", __func__);
  }
  IDBG_MED("%s -", __func__);
  return ret_val;
}

/** module_hdr_port_send_stream_configuration:
 *  @port: the Port to handle the event
 *  @identity: identity
 *  @pp_feature_config: pp feature config
 *
 * Function to send stream configuration to subsequent modules
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_send_stream_configuration(mct_port_t *port,
  uint32_t identity, cam_pp_feature_config_t *pp_feature_config)
{
  boolean ret_val = FALSE;
  mct_event_t event;
  mct_event_control_parm_t event_parm;

  IDBG_MED("%s +", __func__);

  if (port && identity && pp_feature_config && MCT_PORT_PARENT(port)) {

    memset(&event, 0, sizeof(mct_event_t));
    memset(&event_parm, 0, sizeof(mct_event_control_parm_t));

    event.identity  = identity;
    event.type      = MCT_EVENT_CONTROL_CMD;
    event.direction = MCT_EVENT_DOWNSTREAM;
    event.timestamp = 0;
    event.u.module_event.type = MCT_EVENT_CONTROL_SET_PARM;
    event.u.ctrl_event.control_event_data = &event_parm;
    event_parm.type = CAM_INTF_PARM_WAVELET_DENOISE;
    event_parm.parm_data = &pp_feature_config->denoise2d;

    ret_val = mct_list_traverse(
      MCT_MODULE_SRCPORTS(MCT_PORT_PARENT(port)->data),
      module_hdr_port_forward_event_to_peer, &event);

  } else {
    IDBG_ERROR("Null pointer detected in %s\n", __func__);
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_stream_buf_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be handled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle stream param event for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_stream_buf_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  mct_stream_info_t *info;
  mct_stream_t* stream;
  mct_module_t *module;
  module_hdr_port_t *private_data;
  cam_stream_parm_buffer_t *parm_buf;
  cam_metadata_info_t* cam_metadata_info;
  uint32_t enable_hdr;
  uint32_t hdr_lib_buf_idx;
  int32_t i;
  uint32_t j;
  module_hdr_buf_t* hdr_buf;
  module_hdr_crop_t* out_crop_node;
  mct_list_t *list_match;
  module_frame_config_t *frame_config;
  boolean src_port_linked;

  IDBG_MED("%s +", __func__);

  if (event && event->u.ctrl_event.control_event_data && port
    && port->port_private
    && ((module_hdr_port_t *)port->port_private)->lib_instance
    && MCT_PORT_PARENT(port) && (MCT_PORT_PARENT(port) )->data) {

    private_data = port->port_private;
    parm_buf = event->u.ctrl_event.control_event_data;
    module = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);

    if (CAM_STREAM_PARAM_TYPE_GET_OUTPUT_CROP == parm_buf->type) {

      if (pthread_mutex_lock(&private_data->mutex_crop_output_queue))
        CDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);

      out_crop_node = mct_queue_pop_head(&private_data->crop_output_queue);

      if (pthread_mutex_unlock(&private_data->mutex_crop_output_queue))
        CDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);

      if (out_crop_node) {
        stream = mod_imglib_find_module_parent(event->identity, module);

        if (stream) {
          info = &stream->streaminfo;

          info->parm_buf.outputCrop.num_of_streams = 1;
          info->parm_buf.outputCrop.crop_info[0].stream_id =
            IMGLIB_STREAMID(event->identity);
          info->parm_buf.outputCrop.crop_info[0].crop.left =
            out_crop_node->start_x;
          info->parm_buf.outputCrop.crop_info[0].crop.top =
            out_crop_node->start_y;
          info->parm_buf.outputCrop.crop_info[0].crop.width =
            out_crop_node->width;
          info->parm_buf.outputCrop.crop_info[0].crop.height =
            out_crop_node->height;
          IDBG_MED("%s:%d](left, top, width, height): (%d, %d, %d, %d)",
             __func__, __LINE__,
            info->parm_buf.outputCrop.crop_info[0].crop.left,
            info->parm_buf.outputCrop.crop_info[0].crop.top,
            info->parm_buf.outputCrop.crop_info[0].crop.width,
            info->parm_buf.outputCrop.crop_info[0].crop.height);

          ret_val = TRUE;
        } else
          IDBG_ERROR("Cannot find module parent with identity 0x%x in %s\n",
            event->identity, __func__);

        free(out_crop_node);
      } else
        IDBG_ERROR("Output crop queue empty in %s\n", __func__);

      *forward_event = TRUE;
    } else if ((parm_buf->type == CAM_STREAM_PARAM_TYPE_DO_REPROCESS)
      && (MCT_MODULE_FLAG_SOURCE ==
        mct_module_find_type(module, event->identity))) {

      stream = mod_imglib_find_module_parent(event->identity, module);

      if (stream && private_data->stream_on) {
        info = &stream->streaminfo;

        if (0 == private_data->hdr_burst_counter) {
          private_data->hdr_need_1x =
            info->reprocess_config.pp_feature_config.hdr_param.hdr_need_1x;
          if (private_data->non_hdr_extra_buf_needed &&
            private_data->hdr_need_1x) {
              private_data->hdr_port_in_buffs = HDR_PORT_IN_BUFFS;
          } else {
            private_data->hdr_port_in_buffs = HDR_LIB_IN_BUFFS;
          }
        }

        mod_imglib_dump_stream_info(info);

        //Process only if HDR is enabled in the reprocess param
        enable_hdr =
          info->reprocess_config.pp_feature_config.hdr_param.hdr_enable;

        if (enable_hdr) {
          /* Determine if this is 0.5x exposure image, 2x exposure image or something else
           * idx 0 corresponds to 0.5x exposure and idx 1 corresponds to 2x exposure image
           * If the idx is something else, the buffer should be released back with no
           * processing
           *  */
          hdr_lib_buf_idx = module_hdr_port_get_lib_buf_idx(port, event, info);


          list_match = mct_list_find_custom(
            MCT_MODULE_SRCPORTS(module), &event->identity,
            module_hdr_port_check_linked_port_identity);

          src_port_linked = FALSE;
          if (list_match && list_match->data) {
            src_port_linked = TRUE;

            if (0 == hdr_lib_buf_idx) {
              module_hdr_port_send_stream_configuration(port, event->identity,
                &info->reprocess_config.pp_feature_config);
            }
          }

          if (hdr_lib_buf_idx < private_data->hdr_port_in_buffs) {

            private_data->in_buff[hdr_lib_buf_idx] =
              module_hdr_port_get_input_buffer(port, event, info);
            if (private_data->in_buff[hdr_lib_buf_idx]) {

              void *session_meta = NULL;

              cam_metadata_info = module_hdr_port_get_metadata_buffer(port,
                event, info);

              if (cam_metadata_info) {
                private_data->metadata_buff = cam_metadata_info;

                memcpy(&private_data->session_meta,
                  cam_metadata_info->private_metadata,
                  sizeof(private_data->session_meta));
                session_meta = &private_data->session_meta;

              } else {
                private_data->metadata_buff = NULL;
                session_meta = NULL;
              }

              //Debug dump :dump input buffer
              if (private_data->dump_input_frame) {
                mod_imglib_dump_frame(
                  private_data->in_buff[hdr_lib_buf_idx]->img_frame,
                  "hdr_input",
                  hdr_lib_buf_idx);
              }

              /* if buf idx is not final, return immediately
               * private_data->hdr_port_in_buffs frames are needed */
              ret_val = TRUE;

              if (hdr_lib_buf_idx == private_data->hdr_port_in_buffs - 1) {

                for (i = 0; i < HDR_LIB_OUT_BUFFS+HDR_LIB_INPLACE_BUFFS; i++) {

                  private_data->out_buff[i] = module_hdr_port_get_output_buffer(
                    port, event, info, private_data->in_buff[i]->img_frame->frame_id);
                  if (!private_data->out_buff[i]) {
                    IDBG_ERROR("Cannot get output buffer in %s\n", __func__);
                    ret_val = FALSE;
                    break;
                  }

                  frame_config = calloc(1, sizeof(module_frame_config_t));

                  if (!frame_config) {
                    IDBG_ERROR("Not enough memory in %s\n", __func__);
                    ret_val = FALSE;
                    break;
                  }

                  frame_config->src_port_linked = src_port_linked;
                  frame_config->out_buff = private_data->out_buff[i];

                  if (HDR_LIB_OUT_BUFFS + HDR_LIB_INPLACE_BUFFS - 1 == i) {
                    for (j = 0; j < private_data->hdr_port_in_buffs; j++) {
                      frame_config->in_buff[j] = private_data->in_buff[j];
                    }
                    frame_config->input_buff_number =
                      private_data->hdr_port_in_buffs;
                  } else
                    frame_config->input_buff_number = 0;

                  if (pthread_mutex_lock(&private_data->mutex_config_list))
                    IDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__,
                      __LINE__);

                  private_data->config_list = mct_list_append(
                    private_data->config_list, frame_config, NULL, NULL);

                  if (pthread_mutex_unlock(&private_data->mutex_config_list))
                    IDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__,
                      __LINE__);

                  if (!private_data->config_list) {
                    IDBG_ERROR("Cannot append to list in %s\n", __func__);
                    ret_val = FALSE;
                    break;
                  }
                }

                if (private_data->hdr_need_1x) {
                  private_data->non_hdr_buf = module_hdr_port_get_output_buffer(
                    port, event, info,
                    private_data->in_buff[private_data->hdr_port_in_buffs - 1]->img_frame->frame_id);

                  if (!private_data->non_hdr_buf) {
                    IDBG_ERROR("Cannot get nonHDR buffer in %s\n", __func__);
                  }
                } else {
                  private_data->non_hdr_buf = NULL;
                }

                if (ret_val) {
                  ret_val = module_hdr_lib_process(private_data->lib_instance,
                    private_data->out_buff, private_data->in_buff, session_meta,
                    port, &info->reprocess_config.pp_feature_config.hdr_param,
                    module_hdr_port_notify_cb);
                }
              }

            } else
              IDBG_ERROR("Cannot get input buffer in %s\n", __func__);
          } else
            IDBG_ERROR("Input buffer index is wrong %s\n", __func__);
        } else
          // If HDR is not enabled for reprocess, event should be forwarded to other ports
          ret_val = TRUE;
      } else
        IDBG_ERROR("Cannot find module parent with identity 0x%x in %s\n",
          event->identity, __func__);
    } else
      // Other parameters should be forwarded without returning error
      ret_val = TRUE;
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  if (!ret_val)
    IDBG_ERROR("Cannot process image in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_check_config_list_buf_index
 *    @data1: module_frame_config_t object
 *    @data2: buff index to be checked
 *
 *  Checks if this frame configuration is for specified buff index
 *
 *  Return TRUE if the port has the same identity and is linked
 **/
boolean module_hdr_port_check_config_list_buf_index(void *data1, void *data2)
{
  boolean ret_val = FALSE;
  module_frame_config_t *frame_config = (module_frame_config_t *)data1;
  uint32_t *buff_index = (uint32_t *)data2;

  IDBG_MED("%s +", __func__);

  if (buff_index && frame_config && frame_config->out_buff
    && frame_config->out_buff->img_frame
    && (*buff_index == frame_config->out_buff->img_frame->idx))
    ret_val = TRUE;

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_send_buff_done_ack
 *    @port: the Port that handles the event
 *    @buff: buffer handler
 *    @buff_done: flag indicating whether buff done will be marked by pproc
 *                otherwise put buff will be marked by pproc
 *
 * Sends buff divert ack to peer
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_send_buff_done_ack(mct_port_t* port,
  module_hdr_buf_t *buff, boolean buff_done)
{
  boolean ret_val = FALSE;
  module_hdr_port_t *private_data;
  isp_buf_divert_ack_t isp_buf_divert_ack;
  mct_event_t event;

  IDBG_MED("%s +", __func__);

  if (buff && buff->img_frame && port && port->port_private &&
    MCT_PORT_PEER(port) && MCT_PORT_EVENT_FUNC(MCT_PORT_PEER(port)) ) {

    private_data = port->port_private;

    isp_buf_divert_ack.buf_idx = buff->img_frame->idx;
    isp_buf_divert_ack.is_buf_dirty = !buff_done;
    isp_buf_divert_ack.identity = buff->identity;
    isp_buf_divert_ack.channel_id = buff->channel_id;
    isp_buf_divert_ack.meta_data = buff->meta_data;
    isp_buf_divert_ack.frame_id = buff->img_frame->frame_id;
    isp_buf_divert_ack.timestamp.tv_sec = buff->img_frame->timestamp / 1000000;
    isp_buf_divert_ack.timestamp.tv_usec = buff->img_frame->timestamp
      - isp_buf_divert_ack.timestamp.tv_sec * 1000000;

    IDBG_HIGH("isp_buf_divert_ack.is_buf_dirty %d", isp_buf_divert_ack.is_buf_dirty);
    IDBG_HIGH("isp_buf_divert_ack.identity 0x%x", isp_buf_divert_ack.identity);
    IDBG_HIGH("isp_buf_divert_ack.buf_idx %d", isp_buf_divert_ack.buf_idx);
    IDBG_HIGH("isp_buf_divert_ack.frame_id %d", isp_buf_divert_ack.frame_id);

    event.identity = private_data->reserved_identity;
    event.type = MCT_EVENT_MODULE_EVENT;
    event.direction = MCT_EVENT_UPSTREAM;
    event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT_ACK;
    event.u.module_event.module_event_data = (void *)&isp_buf_divert_ack;

    ret_val = MCT_PORT_EVENT_FUNC(MCT_PORT_PEER(port)) (MCT_PORT_PEER(port),
      &event);
    if (!ret_val)
      IDBG_ERROR("Cannot send MCT_EVENT_MODULE_BUF_DIVERT_ACK in %s\n",
        __func__);
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_invalidate_buffer
 *    @p_frame: image frame pointer
 *
 *  Clean and Invalidates cache before buffer done or
 *  sending it to the next module.
 *
 *  Return 0 incase of success, else -ve value
 **/
int32_t module_hdr_port_invalidate_buffer(img_frame_t *p_frame)
{
  int rc = -1;
  void *v_addr = NULL;
  int32_t fd = -1;
  int32_t buffer_size = 0;
  int32_t ion_fd_handler = -1;

  if (!p_frame) {
   IDBG_ERROR("%s: %d: Invalid input frame", __func__, __LINE__);
   return IMG_ERR_INVALID_INPUT;
  }

  v_addr = IMG_ADDR(p_frame);
  fd = IMG_FD(p_frame);
  buffer_size = IMG_FRAME_LEN(p_frame);
  ion_fd_handler = open("/dev/ion", O_RDWR|O_DSYNC);

  IDBG_LOW("%s:%d] v_addr %p fd %d size %d ion_fd_handler %d", \
    __func__, __LINE__, v_addr, fd, buffer_size, ion_fd_handler);

  if ((ion_fd_handler < 0) || (fd < 0) || !buffer_size || !v_addr) {
    IDBG_ERROR("%s: %d: Invalid input params", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  rc = img_cache_ops_external(v_addr, buffer_size, 0, fd,
    CACHE_CLEAN_INVALIDATE, ion_fd_handler);

  if (rc) {
    IDBG_ERROR("%s:%d] Cache Invalidation Failed\n", __func__, __LINE__);
  } else {
    IDBG_LOW("%s:%d] Cache Invalidation Success\n", __func__, __LINE__);
  }

  if (ion_fd_handler >= 0)
    close(ion_fd_handler);

  return rc;
}


/** module_hdr_port_divert_notify_cb
 *    @user_data: user data
 *    @out_buff: output buffer handler
 *    @in_buff: input buffer handler
 *    @out_crop: output crop
 *
 * Module hdr library process done callback for diverted frame
 *
 * Returns Nothing
 **/
static void module_hdr_port_divert_notify_cb(void* user_data,
  module_hdr_buf_t **out_buff, module_hdr_buf_t **in_buff,
  module_hdr_crop_t* out_crop)
{
  boolean found = FALSE;
  mct_port_t *port = user_data;
  module_hdr_port_t *private_data;
  module_frame_config_t frame_config;
  mct_list_t *list_match;
  boolean send_frame_to_next_module = FALSE;
  int32_t i;
  uint32_t j;
  uint32_t inplace_indexes_count;
  uint32_t inplace_indexes[HDR_LIB_INPLACE_BUFFS];
  module_hdr_crop_t *out_crop_node;
  boolean is_native_ar[HDR_PORT_IN_BUFFS];

  IDBG_MED("%s +", __func__);

  if (out_buff && in_buff && port && port->port_private && out_crop
    && ((module_hdr_port_t *)port->port_private)->lib_instance) {
    private_data = port->port_private;

    if (private_data->hdr_need_1x) {
      i = 2;
    } else {
      i = 1;
    }

    if (pthread_mutex_lock(&private_data->mutex_crop_output_queue))
      CDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);

    while (i--) {
      out_crop_node = calloc(1, sizeof(module_hdr_crop_t));

      if (out_crop_node) {
        memcpy(out_crop_node, out_crop, sizeof(module_hdr_crop_t));

        mct_queue_push_tail(&private_data->crop_output_queue, out_crop_node);

      } else
        IDBG_ERROR("Not enough memory in %s\n", __func__);
    }

    if (pthread_mutex_unlock(&private_data->mutex_crop_output_queue))
      CDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);

    // Storing info of native buffer in an array
    for (j = 0; j < private_data->hdr_port_in_buffs; j++) {

      if (!private_data->in_buff[j]->is_native) {
        is_native_ar[j] = FALSE;
      } else
        is_native_ar[j] = TRUE;
    }

    for (i = 0; i < HDR_LIB_OUT_BUFFS + HDR_LIB_INPLACE_BUFFS; i++) {

      if (pthread_mutex_lock(&private_data->mutex_config_list))
        IDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);

      list_match = mct_list_find_custom(private_data->config_list,
        &out_buff[i]->img_frame->frame_id,
        module_hdr_port_check_config_list_frame_id);

      if (list_match && list_match->data) {
        found = TRUE;
        frame_config = *(module_frame_config_t *)list_match->data;

        if (frame_config.src_port_linked)
          send_frame_to_next_module = TRUE;
        else {
          free(list_match->data);
          private_data->config_list = mct_list_remove(private_data->config_list,
            list_match->data);
        }
        IDBG_LOW("%s:%d,found = %d send_frame_to_next_module = %d"
          " src_linked:%d",__func__, __LINE__, found,
          send_frame_to_next_module, frame_config.src_port_linked);
      } else
        IDBG_ERROR("Memory corruption in %s:%d \n", __func__, __LINE__);

      if (pthread_mutex_unlock(&private_data->mutex_config_list))
        IDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);

      if (found) {

        if (!private_data->non_hdr_buf && private_data->hdr_need_1x) {
          // Use lowest frame id for output buff
          if (private_data->in_buff[private_data->non_hdr_buf_index]->img_frame->frame_id
            < private_data->out_buff[i]->img_frame->frame_id) {
            IMG_SWAP(private_data->out_buff[i]->img_frame->frame_id,
              private_data->in_buff[private_data->non_hdr_buf_index]->img_frame->frame_id);
          }
        }

        // Inplace native buffs need to be copied to output buffer from current
        // stream
        if (i < HDR_LIB_INPLACE_BUFFS) {
          if (!module_hdr_lib_get_output_inplace_index(i, &j)) {
            IDBG_ERROR("Cannot get index of inplace buffer in %s\n",
              __func__);
          }

          // Clean and invalidate cache before sending to next module
          module_hdr_port_invalidate_buffer(
            private_data->in_buff[j]->img_frame);

          //copy input buffers to output buffers only when src port is not
          //linked
          if (private_data->in_buff[j]->is_native && !send_frame_to_next_module) {
            if (IMG_SUCCESS != img_image_copy(
              private_data->out_buff[i]->img_frame,
              private_data->in_buff[j]->img_frame)) {
                IDBG_ERROR("Cannot copy from native buffer in %s\n", __func__);
            }
          }
        }

        if (private_data->dump_output_frame) {
          if (!send_frame_to_next_module) {
             mod_imglib_dump_frame(out_buff[i]->img_frame, "hdr_output", i);
          } else {
             mod_imglib_dump_frame(private_data->in_buff[j]->img_frame, "hdr_output", i);
          }
        }

        if (send_frame_to_next_module) {
          // module_hdr_port_send_buff(port, out_buff[i]);
          // send the hdr input buffer to next module
          module_hdr_port_send_buff(port,private_data->in_buff[j]);
          IDBG_LOW("%s input buffer %d sent to next module, buff_addr %p\n",
            __func__, j, private_data->in_buff[j]);
        } else {
          // Process non hdr frame once for last buffer
          if (HDR_LIB_OUT_BUFFS + HDR_LIB_INPLACE_BUFFS - 1 == i) {
            if (private_data->non_hdr_buf) {
              if (IMG_SUCCESS != img_image_copy(
                private_data->non_hdr_buf->img_frame,
                private_data->in_buff[private_data->non_hdr_buf_index]->img_frame)) {
                IDBG_ERROR("Cannot copy nonHDR frame in %s\n", __func__);
              }
            }
          }

          module_hdr_port_release_buffer(&private_data->out_buff[i],
            private_data->reserved_identity, TRUE);
        }
      } else if (private_data->dump_output_frame) {
        mod_imglib_dump_frame(out_buff[i]->img_frame, "hdr_output", i);
      }
    }

    if (send_frame_to_next_module) {
        if (private_data->hdr_need_1x) {
          if (private_data->non_hdr_buf) {
            module_hdr_port_send_buff(port, private_data->non_hdr_buf);
            IDBG_LOW("%s input buffer %d sent to next module, buff_addr %p\n",
              __func__, private_data->non_hdr_buf_index,
              private_data->in_buff[private_data->non_hdr_buf_index]);
          } else {
            //send non-hdr input buffer to next module
            module_hdr_port_send_buff(port,
              private_data->in_buff[private_data->non_hdr_buf_index]);
          }
        }
       IDBG_LOW("%s hdr_need_1x: %d non_hdr_buf: %p non_hdr_buf_index: %d\n",
         __func__,private_data->hdr_need_1x,private_data->non_hdr_buf,
         private_data->non_hdr_buf_index );
    } else if (found) {
      inplace_indexes_count = 0;
      for (i = 0; i < HDR_LIB_INPLACE_BUFFS; i++) {
        if (!module_hdr_lib_get_output_inplace_index(i,
          &inplace_indexes[inplace_indexes_count])) {
            IDBG_ERROR("Cannot get index of inplace buffer in %s\n", __func__);
            continue;
        }
        inplace_indexes_count++;
      }
      for (j = 0; j < private_data->hdr_port_in_buffs; j++) {

        if (!is_native_ar[j]) {
          if (private_data->non_hdr_buf_index == j) {
              continue;
          }

          // Skip inplace buffers before they are already released as output
          found = FALSE;
          i = inplace_indexes_count;
          while(i) {
            if (j == inplace_indexes[--i]) {
              found = TRUE;
              break;
            }
          }

          if (found) {
            // Clear handle since it is already released as output
            private_data->in_buff[j] = 0;
            continue;
          }
        }
        module_hdr_port_send_buff_done_ack(port, private_data->in_buff[j],
          FALSE);
        module_hdr_port_release_img_buffer(&private_data->in_buff[j]);
      }

      if (private_data->non_hdr_buf) {
        module_hdr_port_release_buffer(&private_data->non_hdr_buf,
          private_data->reserved_identity, TRUE);
      } else {
        module_hdr_port_release_buffer(
          &private_data->in_buff[private_data->non_hdr_buf_index],
          private_data->reserved_identity,
          private_data->hdr_need_1x);
      }
    }
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);
}

/** module_hdr_port_buf_divert_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be handled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle buf divert event for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_buf_divert_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  module_hdr_port_t *private_data = NULL;
  mct_stream_info_t *info;
  mct_stream_t* stream;
  mct_module_t *module;
  mct_list_t *list_match;
  module_frame_config_t *frame_config = NULL;
  uint32_t enable_hdr;
  uint32_t hdr_lib_buf_idx;
  int32_t i;
  uint32_t j;

  IDBG_MED("%s +", __func__);

  if (port && event && port->port_private && MCT_OBJECT_PARENT(port)
    && MCT_OBJECT_PARENT(port) ->data
    && event->u.module_event.module_event_data) {

    private_data = port->port_private;
    module = MCT_MODULE_CAST(MCT_OBJECT_PARENT(port)->data);

    IDBG_LOW("%s is hdr src port connected: %d", __func__, private_data->is_srcport_connected);

    *forward_event = FALSE;

    stream = mod_imglib_find_module_parent(event->identity, module);
    if (stream && private_data->stream_on) {

      info = &stream->streaminfo;

      if (0 == private_data->hdr_burst_counter) {
        private_data->hdr_need_1x =
          info->reprocess_config.pp_feature_config.hdr_param.hdr_need_1x;
        if (private_data->non_hdr_extra_buf_needed &&
          private_data->hdr_need_1x) {
            private_data->hdr_port_in_buffs = HDR_PORT_IN_BUFFS;
        } else {
          private_data->hdr_port_in_buffs = HDR_LIB_IN_BUFFS;
        }
      }

      mod_imglib_dump_stream_info(info);

      //Process only if HDR is enabled in the reprocess param
      enable_hdr =
        info->reprocess_config.pp_feature_config.hdr_param.hdr_enable;

      if (enable_hdr) {

        /* Determine if this is 0.5x exposure image, 2x exposure image or something else
         * idx 0 corresponds to 0.5x exposure and idx 1 corresponds to 2x exposure image
         * If the idx is something else, the buffer should be released back with no
         * processing
         *  */
        hdr_lib_buf_idx = module_hdr_port_get_lib_buf_idx(port, event, info);
        if (hdr_lib_buf_idx < private_data->hdr_port_in_buffs) {

          private_data->in_buff[hdr_lib_buf_idx] =
            module_hdr_port_get_diverted_input_buffer(port, event, info,
              private_data->is_srcport_connected);
          if (private_data->in_buff[hdr_lib_buf_idx]) {
            //Debug dump input buffer
            if (private_data->dump_input_frame) {
              mod_imglib_dump_frame(
                private_data->in_buff[hdr_lib_buf_idx]->img_frame, "hdr_input",
                hdr_lib_buf_idx);
            }

            /* if buf idx is not final, return immediately
             * private_data->hdr_port_in_buffs frames are needed */
            ret_val = TRUE;

            if (hdr_lib_buf_idx == private_data->hdr_port_in_buffs - 1) {
              IDBG_MED("%s HDR Input Buffers Ready, Start Processing Now!!!", __func__);
              for (i = 0; i < HDR_LIB_OUT_BUFFS + HDR_LIB_INPLACE_BUFFS; i++) {

                if (i < HDR_LIB_OUT_BUFFS) {
                  private_data->out_buff[i] = module_hdr_port_get_output_buffer(
                    port, event, info,
                    private_data->in_buff[i]->img_frame->frame_id);

                  if (!private_data->out_buff[i]) {
                    IDBG_ERROR("Cannot get output buffer in %s\n", __func__);
                    ret_val = FALSE;
                    break;
                  }
                } else {
                  if (!module_hdr_lib_get_output_inplace_index(
                    i - HDR_LIB_OUT_BUFFS, &j)) {
                      IDBG_ERROR("Cannot get index of inplace buffer in %s\n",
                        __func__);
                      break;
                  }
                  //obtain the HDR output buffers only when the HDR is sink module
                  //and the input buffer is native
                  if (!private_data->is_srcport_connected &&
                      private_data->in_buff[j]->is_native) {
                    // Output buffer needs to be allocated on current stream
                    private_data->out_buff[i] =
                      module_hdr_port_get_output_buffer(
                        port, event, info,
                        private_data->in_buff[i]->img_frame->frame_id);

                    if (!private_data->out_buff[i]) {
                      IDBG_ERROR("Cannot get output buffer in %s\n", __func__);
                      ret_val = FALSE;
                      break;
                    }

                  } else {
                    // Copy only buffer descriptor
                    private_data->out_buff[i] = private_data->in_buff[j];
                  }
                }

                frame_config = calloc(1, sizeof(module_frame_config_t));

                if (frame_config) {
                    frame_config->src_port_linked =
                      private_data->is_srcport_connected;
                } else {
                  IDBG_ERROR("Not enough memory in %s\n", __func__);
                  ret_val = FALSE;
                  break;
                }

                frame_config->out_buff = private_data->out_buff[i];

                if (HDR_LIB_OUT_BUFFS + HDR_LIB_INPLACE_BUFFS - 1 == i) {
                  for (j = 0; j < private_data->hdr_port_in_buffs; j++)
                    frame_config->in_buff[j] = private_data->in_buff[j];
                  frame_config->input_buff_number = private_data->hdr_port_in_buffs;
                } else
                  frame_config->input_buff_number = 0;

                if (pthread_mutex_lock(&private_data->mutex_config_list))
                  IDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__,
                    __LINE__);

                private_data->config_list = mct_list_append(
                  private_data->config_list, frame_config, NULL, NULL);

                if (pthread_mutex_unlock(&private_data->mutex_config_list))
                  IDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__,
                    __LINE__);

                if (!private_data->config_list) {
                  IDBG_ERROR("Cannot append to list in %s\n", __func__);
                  ret_val = FALSE;
                  break;
                }
              }

              if (ret_val) {

                boolean native_input =
                  private_data->in_buff[private_data->non_hdr_buf_index]->is_native;
                //Obtain the output buffer for non hdr frame only when the hdr is sink module
                if (!private_data->is_srcport_connected &&
                     (private_data->non_hdr_extra_buf_needed || native_input)
                      && private_data->hdr_need_1x) {
                    private_data->non_hdr_buf =
                      module_hdr_port_get_output_buffer(port, event, info,
                        private_data->in_buff[private_data->hdr_port_in_buffs - 1]->img_frame->frame_id);

                    if (!private_data->non_hdr_buf) {
                      IDBG_ERROR("Cannot get nonHDR buffer in %s\n", __func__);
                    }
                } else {
                  private_data->non_hdr_buf = NULL;
                }

                void *session_meta = NULL;
                if (private_data->is_session_meta_valid == TRUE)
                  session_meta = &private_data->session_meta;
                ret_val = module_hdr_lib_process(private_data->lib_instance,
                  private_data->out_buff, private_data->in_buff, session_meta,
                  port, &info->reprocess_config.pp_feature_config.hdr_param,
                  module_hdr_port_divert_notify_cb);
              }
            }
          } else
            IDBG_ERROR("Cannot get diverted buffer in %s\n", __func__);
        } else {
          /*
           * If the buffer was not meant for this module, do nothing and forward event
           */
          *forward_event = TRUE;
          ret_val = TRUE;
        }
      } else {
        // If HDR is disable, just forward the event
        *forward_event = TRUE;
        ret_val = TRUE;
      }
    } else
      IDBG_ERROR("Can't find module parent with id 0x%x in %s\n",
        event->identity, __func__);
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  if (!ret_val && frame_config && private_data) {
    IDBG_ERROR("Cannot process the image in %s\n", __func__);

    if (pthread_mutex_lock(&private_data->mutex_config_list))
      IDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);

    mct_list_traverse(private_data->config_list,
      module_hdr_port_queue_free_func, NULL);
    mct_list_free_list(private_data->config_list);
    private_data->config_list = NULL;

    if (pthread_mutex_unlock(&private_data->mutex_config_list))
      IDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_up_buf_divert_ack_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be handled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle buf divert ack upstream event for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_up_buf_divert_ack_event(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  boolean found = FALSE;
  module_hdr_port_t *private_data;
  mct_list_t *list_match;
  isp_buf_divert_ack_t *buf_divert_ack;
  module_frame_config_t frame_config;
  uint32_t i;

  IDBG_MED("%s +", __func__);

  if (port && event && port->port_private
    && event->u.module_event.module_event_data) {
    buf_divert_ack = event->u.module_event.module_event_data;
    private_data = port->port_private;

    if (pthread_mutex_lock(&private_data->mutex_config_list))
      IDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);

    list_match = mct_list_find_custom(private_data->config_list,
      &buf_divert_ack->buf_idx, module_hdr_port_check_config_list_buf_index);

    if (list_match && list_match->data) {
      found = TRUE;
      *forward_event = FALSE;
      frame_config = *(module_frame_config_t *)list_match->data;

      free(list_match->data);
      private_data->config_list = mct_list_remove(private_data->config_list,
        list_match->data);
    }

    if (pthread_mutex_unlock(&private_data->mutex_config_list))
      IDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);

    if (found) {

      if (!private_data->non_hdr_buf && private_data->hdr_need_1x) {
        // Use lowest frame id for output buff
        if (frame_config.in_buff[private_data->non_hdr_buf_index]->img_frame->frame_id
          < frame_config.out_buff->img_frame->frame_id) {
          IMG_SWAP(frame_config.out_buff->img_frame->frame_id,
            frame_config.in_buff[private_data->non_hdr_buf_index]->img_frame->frame_id);
        }
      }

      module_hdr_port_release_buffer(&frame_config.out_buff,
        private_data->reserved_identity, !buf_divert_ack->is_buf_dirty);

      if (frame_config.input_buff_number) {
        if (private_data->non_hdr_buf) {
          for (i = 0; i < frame_config.input_buff_number; i++) {
            module_hdr_port_send_buff_done_ack(port, frame_config.in_buff[i],
              FALSE);
            module_hdr_port_release_img_buffer(&frame_config.in_buff[i]);
          }

          if (IMG_SUCCESS != img_image_copy(
            private_data->non_hdr_buf->img_frame,
            frame_config.in_buff[private_data->non_hdr_buf_index]->img_frame)) {
            IDBG_ERROR("Cannot copy nonHDR frame in %s\n", __func__);
          }

          module_hdr_port_release_buffer(&private_data->non_hdr_buf,
            private_data->reserved_identity, TRUE);

        } else {

          for (i = 0; i < frame_config.input_buff_number; i++) {
            if (private_data->non_hdr_buf_index == i) {
              continue;
            }
            module_hdr_port_send_buff_done_ack(port, frame_config.in_buff[i],
              FALSE);
            module_hdr_port_release_img_buffer(&frame_config.in_buff[i]);
          }

          module_hdr_port_release_buffer(
            &frame_config.in_buff[private_data->non_hdr_buf_index],
            private_data->reserved_identity,
            private_data->hdr_need_1x);
        }
      }
    }

    ret_val = TRUE;
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_downstream_ctrl:
 *  @port: the Port to handle the event
 *  @event: the Event to be handled
 *  @forward_event: output flag indicating whether
 *                  event need to be forwarded to the peer
 *
 * Function to handle command for the port
 *
 * Returns TRUE in case of success
 **/
static boolean module_hdr_port_downstream_ctrl(mct_port_t *port,
  mct_event_t *event, boolean *forward_event)
{
  boolean ret_val = FALSE;
  module_hdr_port_event_func event_handler;

  IDBG_MED("%s +", __func__);

  if (port && event) {
    mct_event_control_t *mct_event_control = &event->u.ctrl_event;

    event_handler = module_hdr_port_find_event_handler(mct_event_control->type,
      module_hdr_port_cmd_lut);
    if (event_handler)
      ret_val = event_handler(port, event, forward_event);
    else
      ret_val = TRUE;
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_event:
 *  @port: the Port to handle the event
 *  @event: the Event to be handled
 *
 * Function to handle an event for the port
 *
 * Returns: TRUE if success
 */
static boolean module_hdr_port_event(mct_port_t *port, mct_event_t *event)
{
  boolean ret_val = FALSE;
  boolean forward_event = TRUE; // Always need to be initialized to true
  module_hdr_port_t *private_data;

  IDBG_MED("%s +", __func__);

  if (port && event && port->port_private && MODULE_HDR_VALIDATE_NAME(port)) {

    MCT_OBJECT_LOCK(port);

    private_data = port->port_private;
    if (private_data->reserved_identity == event->identity) {

      IDBG_LOW("%s: private_data->reserved_identity=0x%x, event->identity=0x%x",
        __func__, private_data->reserved_identity, event->identity);
      IDBG_LOW("%s: event dir=%d, type=%d", __func__,
        MCT_EVENT_DIRECTION(event), event->type);

      switch (MCT_EVENT_DIRECTION(event) ) {
      case MCT_EVENT_DOWNSTREAM:
        switch (event->type) {
        case MCT_EVENT_MODULE_EVENT:
          ret_val = module_hdr_port_downstream_event(port, event,
            &forward_event);
          break;

        case MCT_EVENT_CONTROL_CMD:
          ret_val = module_hdr_port_downstream_ctrl(port, event,
            &forward_event);
          break;

        default:
          IDBG_ERROR("Event is with wrong type");
          break;
        }

        if (ret_val && forward_event)
          ret_val = mct_list_traverse(
            MCT_MODULE_SRCPORTS(MCT_PORT_PARENT(port)->data),
            module_hdr_port_forward_event_to_peer, event);
        /* case MCT_EVENT_TYPE_DOWNSTREAM */
        break;

      case MCT_EVENT_UPSTREAM:
        ret_val = module_hdr_port_upstream_event(port, event, &forward_event);

        if (ret_val && forward_event)
          ret_val = mct_list_traverse(
            MCT_MODULE_SINKPORTS(MCT_PORT_PARENT(port)->data),
            module_hdr_port_forward_event_to_peer, event);

        break;

      default:
        IDBG_ERROR("Event is with wrong direction");
        break;
      }
    } else
      IDBG_ERROR("Event is meant for port with different identity");

    MCT_OBJECT_UNLOCK(port);

  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_ext_link
 *    @identity: the identity of the stream and session
 *    @port: the port that is linked
 *    @peer: the peer port of the link
 *
 * Function that handles a new external link on the port
 *
 * Returns: TRUE if success
 */
static boolean module_hdr_port_ext_link(uint32_t identity, mct_port_t *port,
  mct_port_t *peer)
{
  boolean ret_val = FALSE;
  module_hdr_port_t *private_data;

  IDBG_MED("%s +", __func__);

  if (port && peer && MODULE_HDR_VALIDATE_NAME(port)
    && (module_hdr_port_t *)port->port_private) {
    private_data = (module_hdr_port_t *)port->port_private;

    MCT_OBJECT_LOCK(port);

    if ((MODULE_HDR_PORT_STATE_RESERVED == private_data->state
      || MODULE_HDR_PORT_STATE_LINKED == private_data->state)
      && (private_data->reserved_identity == identity)) {

      private_data->state = MODULE_HDR_PORT_STATE_LINKED;
      MCT_PORT_PEER(port) = peer;
      MCT_OBJECT_REFCOUNT(port) += 1;

      IDBG_HIGH("Port %s linked to identity 0x%x", MCT_OBJECT_NAME(port),
        identity);

      ret_val = TRUE;
    }
    MCT_OBJECT_UNLOCK(port);
  } else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_unlink
 *    @identity: the sessionid idntity
 *    @port: the port that is linked
 *    @peer: the peer port of the link
 *
 * Function that handles a removing external link on the port
 *
 * Returns: TRUE if success
 */
static void module_hdr_port_unlink(uint32_t identity, mct_port_t *port,
  mct_port_t *peer)
{
  module_hdr_port_t *private_data;

  IDBG_MED("%s +", __func__);

  if (port && peer
    && MCT_PORT_PEER(port) == peer&& (module_hdr_port_t *)port->port_private
    && MODULE_HDR_VALIDATE_NAME(port)) {

    private_data = (module_hdr_port_t *)port->port_private;

    MCT_OBJECT_LOCK(port);
    if (MODULE_HDR_PORT_STATE_LINKED == private_data->state &&
        private_data->reserved_identity == identity) {

      MCT_OBJECT_REFCOUNT(port) -= 1;
      if (!MCT_OBJECT_REFCOUNT(port))
      private_data->state = MODULE_HDR_PORT_STATE_RESERVED;
    }

    IDBG_HIGH("Port %s unliked from identity 0x%x",
        MCT_OBJECT_NAME(port), identity);

    MCT_OBJECT_UNLOCK(port);
  }

  IDBG_MED("%s -", __func__);
}

/** module_hdr_port_set_caps
 *    @port: port with new capabilities
 *    @caps: new capabilities
 *
 * Function that sets the capabilities of the given port
 *
 * Returns: TRUE if success
 */
static boolean module_hdr_port_set_caps(mct_port_t *port, mct_port_caps_t *caps)
{
  boolean ret_val = FALSE;

  IDBG_MED("%s +", __func__);

  if (MODULE_HDR_VALIDATE_NAME(port)) {

    port->caps = *caps;
    ret_val = TRUE;
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_check_caps_reserve
 *    @port: port to be reserved
 *    @peer_caps: the peer port capabilities
 *    @info: media controller stream info
 *
 * Function that checks capabilities match with suggested peer port. If there
 * is a match, the port is reserved for for establishing suggested link
 *
 * Returns: TRUE if success
 */
static boolean module_hdr_port_check_caps_reserve(mct_port_t *port, void *caps,
  void *info)
{
  boolean ret_val = FALSE;
  mct_port_caps_t *peer_caps = caps;
  mct_stream_info_t *stream_info = info;
  module_hdr_port_t *private_data;
  boolean is_valid_sinkport = FALSE;
  boolean is_valid_srcport = FALSE;

  IDBG_MED("%s +", __func__);

  if (port && MODULE_HDR_VALIDATE_NAME(port) && info
    && (module_hdr_port_t *)port->port_private) {
      if (peer_caps && (port->direction == MCT_PORT_SINK)) {
        if (peer_caps->port_caps_type == port->caps.port_caps_type) {
          is_valid_sinkport = TRUE;
        }
      } else if (port->direction == MCT_PORT_SRC) {
        is_valid_srcport = TRUE;
      }
  }

  IDBG_LOW("%s is_valid_sinkport %d is_valid_srcport %d", __func__,
    is_valid_sinkport, is_valid_srcport);

  if (is_valid_sinkport || is_valid_srcport) {

    MCT_OBJECT_LOCK(port);
    private_data = (module_hdr_port_t *)port->port_private;
    if (MODULE_HDR_PORT_STATE_CREATED == private_data->state) {
      private_data->reserved_identity = stream_info->identity;
      private_data->state = MODULE_HDR_PORT_STATE_RESERVED;
      private_data->stream_info = info;

      if (private_data->subdev_fd > 0) {
        close(private_data->subdev_fd);
        private_data->subdev_fd = -1;
      }

      ret_val =
        module_imglib_common_get_bfr_mngr_subdev(&private_data->subdev_fd);

      if (ret_val) {
        IDBG_HIGH("Port %s reserved to identity 0x%x", MCT_OBJECT_NAME(port),
          stream_info->identity);
      }
    }
    MCT_OBJECT_UNLOCK(port);
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_check_caps_unreserve
 *    @port: port whoms capabilities to be checked
 *    @identity: the identity of the stream and session
 *
 * Function that unreserves specified stream and session from specified port
 *
 * Returns: TRUE if success
 */
static boolean module_hdr_port_check_caps_unreserve(mct_port_t *port,
  uint32_t identity)
{
  boolean ret_val = FALSE;
  module_hdr_port_t *private_data;

  IDBG_MED("%s +", __func__);

  if (port && MODULE_HDR_VALIDATE_NAME(port)
    && (module_hdr_port_t *)port->port_private) {

    private_data = (module_hdr_port_t *)port->port_private;

    MCT_OBJECT_LOCK(port);
    if (MODULE_HDR_PORT_STATE_RESERVED == private_data->state
      && private_data->reserved_identity == identity) {

      if (private_data->subdev_fd > 0) {
        close(private_data->subdev_fd);
        private_data->subdev_fd = -1;
      }

      private_data->state = MODULE_HDR_PORT_STATE_CREATED;

      ret_val = TRUE;

      IDBG_HIGH("Port %s unreserved from identity 0x%x", MCT_OBJECT_NAME(port),
        identity);
    }

    MCT_OBJECT_UNLOCK(port);
  }

  IDBG_MED("%s -", __func__);

  return ret_val;
}

/** module_hdr_port_overwrite_port_funcs:
 *    @port: port whoms functions will be overwritten
 *
 * Overwrites default port functions with hdr port specific ones
 *
 * Returns nothing
 **/
static void module_hdr_port_overwrite_port_funcs(mct_port_t *port)
{
  IDBG_MED("%s + ", __func__);

  if (port) {
    mct_port_set_event_func(port, module_hdr_port_event);
    mct_port_set_ext_link_func(port, module_hdr_port_ext_link);
    mct_port_set_unlink_func(port, module_hdr_port_unlink);
    mct_port_set_set_caps_func(port, module_hdr_port_set_caps);
    mct_port_set_check_caps_reserve_func(port,
      module_hdr_port_check_caps_reserve);
    mct_port_set_check_caps_unreserve_func(port,
      module_hdr_port_check_caps_unreserve);
  } else
    IDBG_ERROR("Null pointer in function %s", __func__);

  IDBG_MED("%s - ", __func__);
}

/** module_hdr_port_deinit
 *    @port: port to be deinitialized
 *
 * Deinitializes port
 *
 * Returns TRUE in case of success
 **/
void module_hdr_port_deinit(mct_port_t *port)
{
  module_hdr_port_t *private_data;

  IDBG_MED("%s +", __func__);

  if (port && port->port_private && MODULE_HDR_VALIDATE_NAME(port)) {

    MCT_OBJECT_LOCK(port);

    private_data = port->port_private;

    if (pthread_mutex_lock(&private_data->mutex_config_list))
      IDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);

    mct_list_traverse(private_data->config_list,
      module_hdr_port_queue_free_func, NULL);
    mct_list_free_list(private_data->config_list);

    if (pthread_mutex_unlock(&private_data->mutex_config_list))
      IDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);

    if (pthread_mutex_destroy(&private_data->mutex_config_list))
      IDBG_ERROR("Cannot destroy mutex\n");

    if (pthread_mutex_lock(&private_data->mutex_crop_output_queue))
      CDBG_ERROR("Cannot lock the mutex in %s:%d \n", __func__, __LINE__);

    if (!MCT_QUEUE_IS_EMPTY(&private_data->crop_output_queue))
      mct_queue_flush(&private_data->crop_output_queue,
        module_hdr_port_queue_free_func);

    if (pthread_mutex_unlock(&private_data->mutex_crop_output_queue))
      CDBG_ERROR("Cannot unlock the mutex in %s:%d \n", __func__, __LINE__);

    if (private_data->subdev_fd > 0) {
      close(private_data->subdev_fd);
      private_data->subdev_fd = -1;
    }

    module_hdr_lib_deinit(private_data->lib_instance);

    free(private_data);

    IDBG_HIGH("Port %s destroyed", MCT_OBJECT_NAME(port));
  }

  IDBG_MED("%s -", __func__);
}

/** module_hdr_port_init:
 *    @port: port to be initialized
 *    @direction: source / sink
 *    @sessionid: session ID to be associated with this port
 *    @lib_handle: library handle
 *
 *  Port initialization entry point. Becase current module/port is
 *  pure software object, defer this function when session starts.
 **/
boolean module_hdr_port_init(mct_port_t *port, mct_port_direction_t direction,
  uint32_t *sessionid, void* lib_handle)
{
  boolean ret_val = FALSE;
  mct_port_caps_t port_caps;
  uint32_t *session;
  mct_list_t *list;
  module_hdr_port_t *private_data;
  mct_pipeline_imaging_cap_t caps;
  uint32_t i;

  IDBG_MED("%s +", __func__);

  if (port)
    if (MODULE_HDR_VALIDATE_NAME(port)) {

      private_data = (void *)calloc(1, sizeof(module_hdr_port_t));
      if (private_data) {
        private_data->lib_instance = module_hdr_lib_init(lib_handle);
        if (private_data->lib_instance) {
          private_data->reserved_identity = (*sessionid) << 16;
          private_data->state = MODULE_HDR_PORT_STATE_CREATED;

          if (module_hdr_lib_query_mod(&caps)) {
            private_data->non_hdr_extra_buf_needed = TRUE;
            private_data->non_hdr_buf_index = HDR_PORT_IN_BUFFS - 1;
            for (i=0; i<caps.hdr_bracketing_setting.num_frames; i++) {
              if (!caps.hdr_bracketing_setting.exp_val.values[i]) {
                private_data->non_hdr_extra_buf_needed = FALSE;
                private_data->non_hdr_buf_index = i;
                break;
              }
            }

            if (!pthread_mutex_init(&private_data->mutex_config_list, NULL)) {

              port->port_private = private_data;
              port->direction = direction;
              port_caps.port_caps_type = MCT_PORT_CAPS_FRAME;

              module_hdr_port_overwrite_port_funcs(port);

              if (port->set_caps)
                port->set_caps(port, &port_caps);

              IDBG_HIGH("Port %s initialized", MCT_OBJECT_NAME(port));

              ret_val = TRUE;
            } else
              IDBG_ERROR("Cannot initialize mutex in %s", __func__);
          } else {
            IDBG_ERROR("Library capabilities cannot be queried");
          }
        } else {
          IDBG_ERROR("Port private data cannot be initialized");
        }

        if (!ret_val)
          free(private_data);
      }
    } else {
      IDBG_ERROR("Requested port name is %s\n", MCT_OBJECT_NAME(port));
      IDBG_ERROR("Port name needs to start with %s\n", MODULE_HDR_NAME);
    }
  else
    IDBG_ERROR("Null pointer detected in %s\n", __func__);

  IDBG_MED("%s -", __func__);

  return ret_val;
}
