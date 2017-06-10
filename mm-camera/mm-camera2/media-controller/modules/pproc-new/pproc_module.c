/* pproc_module.c
 *
 * Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "camera_dbg.h"
#include "cam_intf.h"
#include "cam_types.h"
#include "modules.h"
#include "mct_list.h"
#include "mct_module.h"
#include "mct_port.h"
#include "eztune_diagnostics.h"
#include "pproc_module.h"
#include "pproc_port.h"
#include "mct_controller.h"
//#include "module_cac.h"
#include "c2d_module.h"
#include "cpp_module.h"
#include "vpe_module.h"
#include "wnr_module.h"
#include "module_imglib.h"
#include <cutils/properties.h>

#if 0
#define DEBUG_PPROC_MODULE
#ifdef DEBUG_PPROC_MODULE
#undef CDBG
#define CDBG CDBG_ERROR
#endif
#endif
#undef PPROC_OFFLINE_USE_V4L2
//#define PPROC_OFFLINE_USE_V4L2

/** _pproc_module_type:
 *    @identity:    identity (sessionid + streamid).
 *    @module_type: module type corresponding to this identity
 *
 * Store the module type per identity
 **/
typedef struct _pproc_module_type {
  unsigned int      identity;
  mct_module_type_t module_type;
} pproc_module_type_t;

/** _pproc_module_private:
 *    @cpp_module:       cpp submodule
 *    @vpe_module:       vpe submodule
 *    @c2d_module:       c2d submodule
 *    @cac_module:       cac submodule
 *    @wnr_module:       wnr submodule
 *    @llvd_module:      LLVD submodule
 *    @module_type_list: list to hold identity(stream) and
 *                       corresponding module type
 *
 * private object structure for pproc module
 **/
typedef struct _pproc_module_private {
  mct_module_t *cpp_module;
  mct_module_t *vpe_module;
  mct_module_t *c2d_module;
  mct_module_t *cac_module;
  mct_module_t *wnr_module;
  mct_module_t *hdr_module;
  mct_module_t *llvd_module;
  mct_list_t   *module_type_list;
} pproc_module_private_t;

volatile uint32_t gCamPprocLogLevel = 0;
char pproc_prop[PROPERTY_VALUE_MAX];

/** pproc_module_util_check_stream
 *    @d1: mct_stream_t* pointer to the stream being checked
 *    @d2: uint32_t* pointer to identity
 *
 *  Check if the stream matches stream index or stream type.
 *
 *  Return: TRUE if stream matches.
 **/
boolean pproc_module_util_check_stream(void *d1, void *d2)
{
  boolean ret_val = FALSE;
  mct_stream_t *stream = (mct_stream_t *)d1;
  uint32_t *id = (uint32_t *)d2;

  if (stream && id && stream->streaminfo.identity == *id)
    ret_val = TRUE;

  return ret_val;
}

/** pproc_module_util_find_parent
 *    @identity: required identity
 *    @module: module, whichs parents will be serached
 *
 * Finds module parent (stream) with specified identity
 *
 * Returns Pointer to stream handler in case of cucess
 *   or NULL in case of failure
 **/
mct_stream_t* pproc_module_util_find_parent(uint32_t identity,
  mct_module_t* module)
{
  mct_stream_t* ret_val = NULL;
  mct_list_t *find_list = NULL;

  if (module && MCT_MODULE_PARENT(module)) {
    find_list = mct_list_find_custom(MCT_MODULE_PARENT(module),
      &identity, pproc_module_util_check_stream);

    if (find_list)
      ret_val = find_list->data;
  }

  return ret_val;
}

/** pproc_module_get_sub_mod
 *    @module: this pproc module object
 *    @name:   sub-module's name
 *
 *  To get a sub-module object from pproc module.
 *
 *  Return sub-module object if it is exists. Otherwise NULL.
 **/
mct_module_t* pproc_module_get_sub_mod(mct_module_t *module, const char *name)
{
  pproc_module_private_t *mod_private;

  CDBG("%s:%d] E\n", __func__, __LINE__);
  if (module == NULL) {
    CDBG_ERROR("%s:%d] error module: %p\n", __func__, __LINE__, module);
    return NULL;
  }

  if (strcmp(MCT_OBJECT_NAME(module), "pproc")) {
    CDBG_ERROR("%s:%d] error invalid module\n", __func__, __LINE__);
    return NULL;
  }

  mod_private = MCT_OBJECT_PRIVATE(module);

  if (!strcmp(name, "cpp")) {
    if (mod_private->cpp_module)
      return mod_private->cpp_module;
  } else if (!strcmp(name, "vpe")) {
    if (mod_private->vpe_module)
      return mod_private->vpe_module;
  } else if (!strcmp(name, "c2d")) {
    if (mod_private->c2d_module)
      return mod_private->c2d_module;
  } else if (!strcmp(name, "cac")) {
    if (mod_private->cac_module)
      return mod_private->cac_module;
  } else if (!strcmp(name, "wnr")) {
    if (mod_private->wnr_module)
      return mod_private->wnr_module;
  } else if (!strcmp(name, "hdr")) {
    if (mod_private->hdr_module)
      return mod_private->hdr_module;
  } else if (!strcmp(name, "llvd")) {
    if (mod_private->llvd_module)
      return mod_private->llvd_module;
  }

  CDBG("%s:%d] X\n", __func__, __LINE__);
  return NULL;
}

/** pproc_module_free_port
 *    @data: port object to free
 *    @user_data: should be NULL
 *
 *  To free a sink or source port.
 *
 *  Return TRUE on success.
 **/
static boolean pproc_module_free_port(void *data, void *user_data)
{
  boolean ret = FALSE;
  mct_port_t *port = MCT_PORT_CAST(data);
  mct_module_t *module = (mct_module_t *)user_data;

  CDBG("%s:%d] E\n", __func__, __LINE__);
  if (!port) {
    CDBG_ERROR("%s:%d] error because list data is null\n", __func__,
      __LINE__);
    return FALSE;
  }

  if (strncmp(MCT_OBJECT_NAME(port), "pproc_sink", strlen("pproc_sink")) &&
      strncmp(MCT_OBJECT_NAME(port), "pproc_source", strlen("pproc_source"))) {
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

  ret = mct_object_unparent(MCT_OBJECT_CAST(port), MCT_OBJECT_CAST(module));
  if (FALSE == ret) {
    CDBG_ERROR("%s: Can not unparent port %s from module %s \n",
      __func__, MCT_OBJECT_NAME(port), MCT_OBJECT_NAME(module));
  }

  pproc_port_deinit(port);

  CDBG("%s:%d] X\n", __func__, __LINE__);
  return TRUE;
}

/** pproc_module_free_type_list
 *    @data: pproc_module_type_t object to free
 *    @user_data: should be NULL
 *
 *  To free module private's module_type_list element.
 *
 *  Return TRUE on success.
 **/
static boolean pproc_module_free_type_list(void *data, void *user_data)
{
  pproc_module_type_t *type = (pproc_module_type_t *)data;

  CDBG("%s:%d] E\n", __func__, __LINE__);
  if (!type) {
    CDBG_ERROR("%s:%d] error type: %p\n", __func__, __LINE__, type);
    return FALSE;
  }

  free(type);

  CDBG("%s:%d] X\n", __func__, __LINE__);
  return TRUE;
}

/** pproc_module_check_session
 *    @data1: pproc_module_type_t object from module private
 *    @data2: session id object
 *
 *  To check if session id has alreday been existing.
 *
 *  Return TRUE if the session id is existing.
 **/
static boolean pproc_module_check_session(void *data1, void *data2)
{
  pproc_module_type_t *type = (pproc_module_type_t *)data1;
  unsigned int        *sessionid = (unsigned int *)data2;

  return (((type->identity & 0xFFFF0000) >> 16) == *sessionid ?
    TRUE: FALSE);
}

/** pproc_module_check_identity
 *    @data1: pproc_module_type_t object from module private
 *    @data2: identity object
 *
 *  To check if identity has alreday been existing.
 *
 *  Return TRUE if the identity is existing.
 **/
static boolean pproc_module_check_identity(void *data1, void *data2)
{
  pproc_module_type_t *type = (pproc_module_type_t *)data1;
  unsigned int        *identity = (unsigned int *)data2;

  return ((type->identity == *identity) ?
    TRUE: FALSE);
}

/** pproc_module_reserve_compatible_port
 *    @data1: submods port
 *    @data2: stream attributes used to reserve this port;
 *
 *  To reserve port on module in stream.
 *
 *  Reserve status from submod
 **/
static boolean pproc_module_reserve_compatible_port(void *data1, void *data2)
{
  mct_port_t        *port = (mct_port_t *)data1;
  mct_stream_info_t *stream_info = (mct_stream_info_t *)data2;
  mct_port_caps_t    peer_caps;

  if (!port || !stream_info) {
    CDBG_ERROR("%s:%d] error port: %p stream_info: %p\n", __func__, __LINE__,
      port, stream_info);
    return FALSE;
  }

  if (port->peer != NULL) {
    return FALSE;
  }

  return TRUE;
}

/** pproc_module_offline_streamon
 *    @module: pproc module
 *    @event: mct event to be handled
 *    @data: stream info
 *
 *  Handle stream on event for offline stream. Request pproc's
 *  own sink port that internally requests' sub module's sink
 *  port and link with pproc's sink port.
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_offline_streamon(mct_module_t *module,
  mct_event_t *event, void *data)
{
  boolean              rc = TRUE;
  mct_list_t          *lport = NULL;
  mct_port_t          *port = NULL;

  /* Validate input pameters */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "pproc") || !event || !data) {
    CDBG_ERROR("%s:%d failed: data %p\n", __func__, __LINE__, data);
    rc = FALSE;
    goto ERROR;
  }

  /* Find pproc port for this identity */
  lport = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), &event->identity,
    pproc_port_check_identity_in_port);

  if (!lport) {
    CDBG_ERROR("%s:%d failed: to caps reserve\n", __func__, __LINE__);
    rc = FALSE;
    goto ERROR;
  }

  /* Extract pproc port from mct list */
  port = (mct_port_t *)lport->data;
  if (!port) {
    CDBG_ERROR("%s:%d failed: reserved port NULL", __func__, __LINE__);
    rc = FALSE;
    goto ERROR;
  }

  /* Call stream on event on pproc port to get forward to sub module */
  rc = port->event_func(port, event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed: to stream on sub module\n", __func__, __LINE__);
    rc = FALSE;
  }

ERROR:
  return rc;
}

/** pproc_module_send_output_dim_event
 *    @module: pproc module
 *
 *  Handle pproc_module_send_output_dim_event. Sends
 *  preview dimensions to the next module.
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_output_dim_event(mct_module_t *module,
  mct_port_t *port, unsigned int identity, cam_dimension_t *input_dim,
  cam_mp_len_offset_t *input_mp, cam_format_t input_fmt)
{
  boolean           rc = TRUE;
  mct_event_t       event;
  mct_stream_info_t stream_info;

  /* Validate input parameters */
  if (!module || !port || !input_dim) {
    CDBG_ERROR("%s:%d failed: module %p, port %p, input_dim %p\n", __func__,
      __LINE__, module, port, input_dim);
    rc = FALSE;
    goto ERROR;
  }

  /* Fill stream_info parameters */
  stream_info.dim.width = input_dim->width;
  stream_info.dim.height = input_dim->height;
  memcpy(stream_info.buf_planes.plane_info.mp,
    input_mp,sizeof(stream_info.buf_planes.plane_info.mp));
  stream_info.fmt = input_fmt;

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_OUTPUT_DIM;
  event.u.module_event.module_event_data = (void *)&stream_info;

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed: send output dim event\n", __func__, __LINE__);
  }

ERROR:
  return rc;
}

/** pproc_module_extract_crop_params
 *    @module: pproc module
 *
 *  Handle pproc_module_extract_crop_params
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_extract_crop_params(cam_metadata_info_t *metadata,
  mct_stream_info_t *input_stream_info, unsigned int identity,
  uint32_t input_stream_id, mct_bus_msg_stream_crop_t *stream_crop)
{
  boolean rc = TRUE;
  cam_crop_data_t *crop_data = NULL;
  uint8_t num_of_streams = 0, index = 0;
  crop_data = &metadata->crop_data;

  /* Validate input parameters */
  if (!metadata || !input_stream_info || !stream_crop) {
    CDBG_ERROR("%s:%d failed: metadata %p input stream %p stream_crop %p\n",
      __func__, __LINE__, metadata, input_stream_info, stream_crop);
    rc = FALSE;
    goto ERROR;
  }

  memset(stream_crop, 0, sizeof(mct_bus_msg_stream_crop_t));
  stream_crop->session_id = PPROC_GET_SESSION_ID(identity);
  stream_crop->stream_id = PPROC_GET_STREAM_ID(identity);
  stream_crop->crop_out_x = input_stream_info->dim.width;
  stream_crop->crop_out_y = input_stream_info->dim.height;

  if (metadata->is_crop_valid == FALSE) {
    CDBG("%s:%d No crop parameters\n", __func__, __LINE__);
    rc = TRUE;
    goto ERROR;
  }

  num_of_streams = crop_data->num_of_streams;
  for (index = 0; index < num_of_streams; index++) {
    if (crop_data->crop_info[index].stream_id == input_stream_id) {
      break;
    }
  }
  if ((index < num_of_streams) &&
    (crop_data->crop_info[index].crop.width > 0) &&
    (crop_data->crop_info[index].crop.width <=
    input_stream_info->dim.width) &&
    (crop_data->crop_info[index].crop.height > 0) &&
    (crop_data->crop_info[index].crop.height <=
    input_stream_info->dim.height)) {
    /* Update stream crop with input crop parameters present in stream info */
    stream_crop->x = crop_data->crop_info[index].crop.left;
    stream_crop->y = crop_data->crop_info[index].crop.top;
    stream_crop->crop_out_x = crop_data->crop_info[index].crop.width;
    stream_crop->crop_out_y = crop_data->crop_info[index].crop.height;
  }
  CDBG("%s:%d crop %d %d %d %d\n", __func__, __LINE__,
    stream_crop->x, stream_crop->y, stream_crop->crop_out_x,
    stream_crop->crop_out_y);

ERROR:
  return rc;
}


/** pproc_module_send_stream_crop_event
 *    @module: pproc module
 *
 *  Handle pproc_module_send_stream_crop_event
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_stream_crop_event(mct_port_t *port,
  unsigned int identity, mct_bus_msg_stream_crop_t *stream_crop)
{
  boolean     rc = TRUE;
  mct_event_t event;

  /* Validate input parameters */
  if (!port || !stream_crop) {
    CDBG_ERROR("%s:%d failed: port %p stream_crop %p\n", __func__, __LINE__,
      port, stream_crop);
    rc = FALSE;
    goto ERROR;
  }

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_STREAM_CROP;
  event.u.module_event.module_event_data = (void *)stream_crop;

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed: send output dim event\n", __func__, __LINE__);
  }

ERROR:
  return rc;
}

/** pproc_module_send_aec_update_event
 *    @module: pproc module
 *
 *  Handle pproc_module_send_aec_update_event
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_aec_update_event(mct_port_t *port,
  unsigned int identity, stats_update_t *stats_update)
{
  boolean     rc = TRUE;
  mct_event_t event;

  /* Validate input parameters */
  if (!port || !stats_update) {
    CDBG_ERROR("%s:%d failed: port %p stats_update %p\n", __func__, __LINE__,
      port, stats_update);
    rc = FALSE;
    goto ERROR;
  }

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_AEC_UPDATE;
  event.u.module_event.module_event_data = (void *)stats_update;

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed: send aec update event\n", __func__,
      __LINE__);
  }

ERROR:
  return rc;
}

/** pproc_module_send_gamma_update_event
 *    @module: pproc module
 *
 *  Handle pproc_module_send_gamma_update_event
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_gamma_update_event(mct_port_t *port,
  unsigned int identity, uint16_t *p_gamma)
{
  boolean     rc = TRUE;
  mct_event_t event;

  /* Validate input parameters */
  if (!port || !p_gamma) {
    CDBG_ERROR("%s:%d failed: port %p p_gamma %p\n", __func__, __LINE__,
      port, p_gamma);
    rc = FALSE;
    goto ERROR;
  }

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_GAMMA_UPDATE;
  event.u.module_event.module_event_data = (void *)p_gamma;

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed: send gamma update event\n", __func__,
      __LINE__);
  }

ERROR:
  return rc;
}

/** pproc_module_send_awb_update_event
 *    @module: pproc module
 *
 *  Handle pproc_module_send_awb_update_event
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_awb_update_event(mct_port_t *port,
  unsigned int identity, awb_update_t *awb_update)
{
  boolean     rc = TRUE;
  mct_event_t event;

  /* Validate input parameters */
  if (!port || !awb_update) {
    CDBG_ERROR("%s:%d failed: port %p awb_update %p\n", __func__, __LINE__,
      port, awb_update);
    rc = FALSE;
    goto ERROR;
  }

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_AWB_UPDATE;
  event.u.module_event.module_event_data = (void *)awb_update;

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed: send awb update event\n", __func__,
      __LINE__);
  }

ERROR:
  return rc;
}

/** pproc_module_send_set_chromatix_event
 *    @module: pproc module
 *
 *  Handle pproc_module_send_set_chromatix_event
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_set_chromatix_event(mct_port_t *port,
  unsigned int identity, modulesChromatix_t *module_chromatix)
{
  boolean     rc = TRUE;
  mct_event_t event;

  /* Validate input parameters */
  if (!port || !module_chromatix) {
    CDBG_ERROR("%s:%d failed: port %p chromatix %p\n", __func__, __LINE__,
      port, module_chromatix);
    rc = FALSE;
    goto ERROR;
  }

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_SET_CHROMATIX_PTR;
  event.u.module_event.module_event_data = (void *)module_chromatix;

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed: send set chromatix event\n", __func__,
      __LINE__);
  }

ERROR:
  return rc;
}

/** pproc_module_send_fd_update_event
 *    @port: mct port
 *    @identity: stream identity
 *    @faces_data: fd info
 *
 *  Handle pproc_module_send_fd_update_event. Sends the FD
 *  info to next module
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_fd_update_event(mct_port_t *port,
  unsigned int identity, cam_face_detection_data_t *faces_data)
{
  boolean     rc = TRUE;
  mct_event_t event;

  /* Validate input parameters */
  if (!port || !faces_data) {
    CDBG_ERROR("%s:%d failed: port %p faces_data %p\n", __func__, __LINE__,
      port, faces_data);
    rc = FALSE;
    goto ERROR;
  }

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_FACE_INFO;
  event.u.module_event.module_event_data = (void *)faces_data;

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed: send fd update event\n", __func__,
      __LINE__);
  }

ERROR:
  return rc;
}

/** pproc_module_send_buf_divert_event
 *    @module: pproc module
 *
 *  Handle pproc_module_send_buf_divert_event
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_buf_divert_event(mct_module_t *module,
  mct_port_t *port, unsigned identity, mct_stream_map_buf_t *buf_holder,
  cam_stream_parm_buffer_t *parm_buf)
{
  boolean           rc = TRUE;
  int32_t           i = 0;
  isp_buf_divert_t  isp_buf;
  struct v4l2_plane plane[VIDEO_MAX_PLANES];
  mct_event_t       event;

  CDBG_LOW("%s:%d +: module %p port %p buf_holder %p\n",
    __func__, __LINE__, module, port, buf_holder);

  /* Validate input parameters */
  if (!module || !port || !buf_holder || !parm_buf) {
    CDBG_ERROR("%s:%d failed: module %p port %p buf_holder %p\n", __func__,
      __LINE__, module, port, buf_holder);
    rc = FALSE;
    goto ERROR;
  }

  memset(&isp_buf, 0, sizeof(isp_buf_divert_t));
  memset(plane, 0, sizeof(plane));
  isp_buf.is_uv_subsampled = parm_buf->reprocess.frame_pp_config.uv_upsample;
  isp_buf.is_skip_pproc = FALSE;
#ifdef PPROC_OFFLINE_USE_V4L2
  /* TODO use v4l2 buffer later */
  isp_buf.native_buf = FALSE;
  isp_buf.buffer.sequence = parm_buf->reprocess.frame_idx;
  isp_buf.buffer.m.planes = plane;
  CDBG_LOW("%s:%d,isp_buf.native_buf: %d\n", __func__,__LINE__, isp_buf.native_buf);
  for (i = 0; i < buf_holder->num_planes; i++) {
    isp_buf.buffer.m.planes[i].m.userptr = buf_holder->buf_planes[i].fd;
    isp_buf.buffer.m.planes[i].data_offset = buf_holder->buf_planes[i].offset;
    isp_buf.buffer.m.planes[i].length = buf_holder->buf_planes[i].size;
  }

  isp_buf.buffer.length = buf_holder->num_planes;
  isp_buf.buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  isp_buf.buffer.index = buf_holder->buf_index;
  isp_buf.buffer.memory = V4L2_MEMORY_USERPTR;

  /* Fill timestamp */
  gettimeofday(&isp_buf.buffer.timestamp, NULL);

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_STREAM_CROP;
  event.u.module_event.module_event_data = (void *)&isp_buf;
#else
  /* Use native buffer for now */
  isp_buf.native_buf = TRUE;
  CDBG_LOW("%s:%d,isp_buf.native_buf: %d\n", __func__, __LINE__,
    isp_buf.native_buf);
  isp_buf.fd = buf_holder->buf_planes[0].fd;
  isp_buf.vaddr = buf_holder->buf_planes[0].buf;
  isp_buf.buffer.sequence = parm_buf->reprocess.frame_idx;

  isp_buf.buffer.length = buf_holder->num_planes;
  isp_buf.buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  isp_buf.buffer.index = buf_holder->buf_index;
  isp_buf.buffer.memory = V4L2_MEMORY_USERPTR;

  /* Fill timestamp */
  gettimeofday(&isp_buf.buffer.timestamp, NULL);

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
  event.u.module_event.module_event_data = (void *)&isp_buf;
#endif

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed: send output dim event\n", __func__, __LINE__);
  }

ERROR:
  return rc;
}

static boolean pproc_module_find_stream_by_streamid(void *data1, void *data2)
{
  mct_stream_t *stream = (void *)data1;
  uint32_t *input_stream_id = (uint32_t *)data2;
  if (!stream) {
    CDBG_ERROR("%s:%d failed stream %p", __func__, __LINE__,stream);
    return FALSE;
  }
  if (!input_stream_id) {
    CDBG_ERROR("%s:%d failed input_stream_id %p", __func__, __LINE__,input_stream_id);
    return FALSE;
  }
  if ((stream->streaminfo.identity & 0xFFFF) == *input_stream_id) {
    return TRUE;
  }
  return FALSE;
}

static boolean pproc_module_find_stream_by_sessionid(void *data1, void *data2)
{
  mct_stream_t *stream = (void *)data1;
  uint32_t *input_session_id = (uint32_t *)data2;

  if (!stream) {
    CDBG_ERROR("%s:%d failed stream %p", __func__, __LINE__,stream);
    return FALSE;
  }
  if (!input_session_id) {
    CDBG_ERROR("%s:%d failed input_session_id %p", __func__, __LINE__,input_session_id);
    return FALSE;
  }
  if (((stream->streaminfo.identity & 0xFFFF0000) >> 16) == *input_session_id) {
    return TRUE;
  }
  return FALSE;
}

static boolean pproc_module_find_online_input_buffer(void *data1, void *data2)
{
  mct_stream_map_buf_t *buf_holder = (mct_stream_map_buf_t *)data1;
  uint32_t *buf_index = (uint32_t *)data2;
  if (!buf_holder || !buf_index) {
    CDBG_ERROR("%s:%d failed buf_holder %p buf_index %p\n", __func__, __LINE__,
      buf_holder, buf_index);
    return FALSE;
  }
  if ((buf_holder->buf_index == *buf_index) &&
      (buf_holder->buf_type == CAM_MAPPING_BUF_TYPE_STREAM_BUF)) {
    return TRUE;
  }
  return FALSE;
}

static void *pproc_module_get_online_input_buffer(mct_module_t *module,
  mct_stream_info_t *stream_info, cam_stream_parm_buffer_t *parm_buf,
  uint32_t identity)
{
  uint32_t session_id = identity >> 16;
  uint32_t stream_id = identity & 0xFFFF;
  mct_pipeline_t *pipeline = NULL;
  mct_stream_t *stream = NULL;
  mct_list_t *stream_list = NULL, *buf_list = NULL;

  /* Validate input params */
  if (!module || !stream_info) {
    CDBG_ERROR("%s:%d failed module %p stream_info %p\n", __func__, __LINE__,
      module, stream_info);
    return NULL;
  }
  CDBG_LOW("%s: %d + module = %p stream_info = %p stream_info.type = %d"
    " identity = %d", __func__, __LINE__, module, stream_info,
    stream_info->stream_type, identity);
  /* Get pproc module's parent - stream */
  stream_list = mct_list_find_custom(MCT_MODULE_PARENT(module), &session_id,
    pproc_module_find_stream_by_sessionid);
  if (!stream_list) {
    CDBG_ERROR("%s:%d failed stream_list %p\n", __func__, __LINE__, stream_list);
    return NULL;
  }
  if (!stream_list->data) {
    CDBG_ERROR("%s:%d failed stream_list->data %p", __func__, __LINE__,
      stream_list->data);
    return FALSE;
  }
  stream = (mct_stream_t *)stream_list->data;
  /* Get stream's parent - pipeline */
  pipeline = MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
  if (!pipeline) {
    CDBG_ERROR("%s:%d failed pipeline %p\n", __func__, __LINE__, pipeline);
    return NULL;
  }
  stream_list = mct_list_find_custom(MCT_PIPELINE_CHILDREN(pipeline),
    &stream_info->reprocess_config.online.input_stream_id,
    pproc_module_find_stream_by_streamid);
  if (!stream_list) {
    CDBG_ERROR("%s:%d failed stream_list %p", __func__, __LINE__, stream_list);
    return FALSE;
  }
  if (!stream_list->data) {
    CDBG_ERROR("%s:%d failed stream_list->data %p", __func__, __LINE__,
      stream_list->data);
    return FALSE;
  }
  stream = (mct_stream_t *)stream_list->data;

  if (!stream->streaminfo.img_buffer_list) {
    CDBG_ERROR("%s:%d failed img_buffer_list = NULL", __func__, __LINE__);
    return FALSE;
  }

  buf_list = mct_list_find_custom(stream->streaminfo.img_buffer_list,
    &parm_buf->reprocess.buf_index, pproc_module_find_online_input_buffer);
  if (!buf_list) {
    CDBG_ERROR("%s:%d failed buf_list %p", __func__, __LINE__, buf_list);
    return FALSE;
  }
  if (!buf_list->data) {
    CDBG_ERROR("%s:%d failed buf_list->data %p", __func__, __LINE__,
      buf_list->data);
    return FALSE;
  }
  return buf_list->data;
}

static boolean pproc_module_find_offline_input_buffer(void *data1, void *data2)
{
  mct_stream_map_buf_t *buf_holder = (mct_stream_map_buf_t *)data1;
  uint32_t *buf_index = (uint32_t *)data2;
  if (!buf_holder || !buf_index) {
    CDBG_ERROR("%s:%d failed buf_holder %p buf_index %p\n", __func__, __LINE__,
      buf_holder, buf_index);
    return FALSE;
  }
  if ((buf_holder->buf_index == *buf_index) &&
      (buf_holder->buf_type == CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF)) {
    return TRUE;
  }
  return FALSE;
}

static void *pproc_module_get_offline_input_buffer(mct_module_t *module,
  cam_stream_parm_buffer_t *parm_buf, uint32_t identity)
{
  uint32_t session_id = identity >> 16;
  uint32_t stream_id = identity & 0xFFFF;
  mct_pipeline_t *pipeline = NULL;
  mct_stream_t *stream = NULL;
  mct_list_t *stream_list = NULL, *buf_list = NULL;
  /* Validate input params */
  if (!module) {
    CDBG_ERROR("%s:%d failed module %p\n", __func__, __LINE__, module);
    return NULL;
  }
  /* Get pproc module's parent - stream */
  stream_list = mct_list_find_custom(MCT_MODULE_PARENT(module), &session_id,
    pproc_module_find_stream_by_sessionid);
  if (!stream_list) {
    CDBG_ERROR("%s:%d failed stream_list %p\n", __func__, __LINE__, stream_list);
    return NULL;
  }
  if (!stream_list->data) {
    CDBG_ERROR("%s:%d failed stream_list->data %p", __func__, __LINE__,
      stream_list->data);
    return FALSE;
  }
  stream = (mct_stream_t *)stream_list->data;
  /* Get stream's parent - pipeline */
  pipeline = MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
  if (!pipeline) {
    CDBG_ERROR("%s:%d failed pipeline %p\n", __func__, __LINE__, pipeline);
    return NULL;
  }
  stream_list = mct_list_find_custom(MCT_PIPELINE_CHILDREN(pipeline),
    &stream_id, pproc_module_find_stream_by_streamid);
  if (!stream_list) {
    CDBG_ERROR("%s:%d failed stream_list %p", __func__, __LINE__, stream_list);
    return FALSE;
  }
  if (!stream_list->data) {
    CDBG_ERROR("%s:%d failed stream_list->data %p", __func__, __LINE__,
      stream_list->data);
    return FALSE;
  }
  stream = (mct_stream_t *)stream_list->data;
  buf_list = mct_list_find_custom(stream->streaminfo.img_buffer_list,
    &parm_buf->reprocess.buf_index, pproc_module_find_offline_input_buffer);
  if (!buf_list) {
    CDBG_ERROR("%s:%d failed buf_list %p", __func__, __LINE__, buf_list);
    return FALSE;
  }
  if (!buf_list->data) {
    CDBG_ERROR("%s:%d failed buf_list->data %p", __func__, __LINE__,
      buf_list->data);
    return FALSE;
  }
  return buf_list->data;
}

/** pproc_module_send_set_param_event
 *    @module: pproc module
 *
 *  Handle pproc_module_send_set_param_event
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_set_param_event(mct_port_t *port,
  unsigned int identity, cam_pp_feature_config_t *pp_feature_config,
  cam_intf_parm_type_t type)
{
  boolean                  rc = TRUE;
  mct_event_t              event;
  mct_event_control_parm_t event_parm;
  cam_stream_parm_buffer_t stream_param;

  memset(&event, 0, sizeof(mct_event_t));
  memset(&event_parm, 0, sizeof(mct_event_control_parm_t));

  /* Validate input parameters */
  if (!port || !pp_feature_config) {
    CDBG_ERROR("%s:%d failed: port %p pp_feature_config %p\n", __func__,
      __LINE__, port, pp_feature_config);
    return FALSE;
  }

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_CONTROL_CMD;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.timestamp = 0;
  event.u.module_event.type = MCT_EVENT_CONTROL_SET_PARM;
  event.u.ctrl_event.control_event_data = &event_parm;
  event_parm.type = type;
  switch (type) {
  case CAM_INTF_PARM_WAVELET_DENOISE:
    event_parm.parm_data = &pp_feature_config->denoise2d;
    break;
  case CAM_INTF_PARM_SHARPNESS:
    event_parm.parm_data = &pp_feature_config->sharpness;
    break;
  case CAM_INTF_PARM_ROTATION:
    event_parm.parm_data = &pp_feature_config->rotation;
    break;
  case CAM_INTF_PARM_EFFECT:
    event_parm.parm_data = &pp_feature_config->effect;
    break;
  case CAM_INTF_PARM_STREAM_FLIP:
    memset(&stream_param, 0, sizeof(cam_stream_parm_buffer_t));
    stream_param.type = type;
    stream_param.flipInfo.flip_mask = pp_feature_config->flip;
    event.u.module_event.type = MCT_EVENT_CONTROL_PARM_STREAM_BUF;
    event.u.ctrl_event.control_event_data = &stream_param;
    break;
  default:
    CDBG_ERROR("%s:%d] error invalid type:%d\n", __func__, __LINE__, type);
    rc = FALSE;
    break;
  }

  if (rc == TRUE) {
     rc = port->event_func(port, &event);
     if (rc == FALSE) {
       CDBG_ERROR("%s:%d failed: send denoise setparam event\n", __func__,
         __LINE__);
     }
  }

  return rc;
}

/** pproc_module_send_parm_stream_buf_event
 *    @port: pproc port
 *    @identity: identity
 *    @stream_info: stream info
 *
 *  Handle pproc_module_send_parm_stream_buf_event
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_parm_stream_buf_event(mct_port_t *port,
  unsigned int identity, mct_stream_info_t *stream_info)
{
  boolean                  rc = TRUE;
  mct_event_t              cmd_event;
  mct_event_control_t      event_data;

  event_data.type = MCT_EVENT_CONTROL_PARM_STREAM_BUF;
  event_data.control_event_data = (void *)&stream_info->parm_buf;

  cmd_event.type = MCT_EVENT_CONTROL_CMD;
  cmd_event.identity = identity;
  cmd_event.direction = MCT_EVENT_DOWNSTREAM;
  cmd_event.timestamp = 0;
  cmd_event.u.ctrl_event = event_data;

   rc = port->event_func(port, &cmd_event);
   if (rc == FALSE) {
     CDBG_ERROR("%s:%d failed: send stream buf event\n", __func__,
       __LINE__);
   }

  return rc;
}

/** pproc_module_handle_reprocess_online
 *    @module: pproc module
 *
 *  Handle pproc_module_handle_reprocess_online
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_handle_reprocess_online(mct_module_t *module,
  mct_port_t *port, mct_stream_info_t *stream_info,
  cam_stream_parm_buffer_t *parm_buf, unsigned int identity)
{
  boolean                           rc = TRUE;
  uint32_t                          session_id = 0;
  uint32_t                          stream_id = 0;
  cam_metadata_info_t              *metadata = NULL;
  mct_stream_info_t                *input_stream_info = NULL;
  mct_bus_msg_stream_crop_t         stream_crop;
  mct_stream_map_buf_t             *buf_holder = NULL;
  cam_pp_feature_config_t          *pp_feature_config;
  modulesChromatix_t                module_chromatix;
  mct_stream_session_metadata_info *priv_metadata;
  stats_get_data_t                 *stats_get = NULL;
  stats_update_t                    stats_update;
  aec_update_t                     *aec_update = NULL;
  awb_update_t                      awb_update;
  uint16_t                          gamma_update[64];
  cam_face_detection_data_t faces_data;

  /* Validate input parameters */
  if (!module || !port || !stream_info || !parm_buf) {
    CDBG_ERROR("%s:%d failed: module %p port %p stream_info %p parm_buf %p\n",
      __func__, __LINE__, module, port, stream_info, parm_buf);
    rc = FALSE;
    goto ERROR;
  }

  session_id = PPROC_GET_SESSION_ID(identity);
  stream_id = PPROC_GET_STREAM_ID(identity);

  /* Get input stream info */
  input_stream_info =
    (mct_stream_info_t *)mct_module_get_stream_info(module, session_id,
    stream_info->reprocess_config.online.input_stream_id);
  if (!input_stream_info) {
    CDBG_ERROR("%s:%d stream_info NULL\n", __func__, __LINE__);
    rc = FALSE;
    goto ERROR;
  }

  /* Validate number of planes */
  if (input_stream_info->buf_planes.plane_info.num_planes == 0) {
    CDBG_ERROR("%s:%d input stream info num planes 0\n", __func__, __LINE__);
    rc = FALSE;
    goto ERROR;
  }

  /* Pass output dim event */
  rc = pproc_module_send_output_dim_event(module, port, identity,
    &input_stream_info->dim,
    input_stream_info->buf_planes.plane_info.mp,
    input_stream_info->fmt);

  /* TODO: when HAL implements single place holder for all the pp_config
     we will switch to using this. For now use the other pp_feature_config
     pp_feature_config = &input_stream_info->pp_config; */
  pp_feature_config = &stream_info->reprocess_config.pp_feature_config;

  /* check for CROP in feature mask */
  /* TODO: enable this only when HAL enables it */
  //if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_CROP) {
    /* Check if metadata is present */
    if (parm_buf->reprocess.meta_present == 1) {
      /* Extract meta buffer */
      metadata = (cam_metadata_info_t *)mct_module_get_buffer_ptr(
        parm_buf->reprocess.meta_buf_index, module, session_id,
        parm_buf->reprocess.meta_stream_handle);
      if (!metadata) {
        CDBG_ERROR("%s:%d failed: metadata NULL\n", __func__, __LINE__);
        rc = FALSE;
        goto ERROR;
      }

      /* Extract crop params from meta data */
      rc = pproc_module_extract_crop_params(metadata, input_stream_info,
        identity, stream_info->reprocess_config.online.input_stream_id,
        &stream_crop);
      if (rc == FALSE) {
        CDBG_ERROR("%s:%d failed: to extract crop params\n", __func__,
          __LINE__);
        rc = FALSE;
        goto ERROR;
      }

      rc = pproc_module_send_stream_crop_event(port, identity, &stream_crop);
      if (rc == FALSE) {
        CDBG_ERROR("%s:%d failed: send stream crop event\n", __func__,
          __LINE__);
        rc = FALSE;
        goto ERROR;
      }

      /* Extract chromatix params from meta data */
      memset(&module_chromatix, 0, sizeof(modulesChromatix_t));
      priv_metadata =
        (mct_stream_session_metadata_info *)metadata->private_metadata;
      module_chromatix.chromatixComPtr =
        priv_metadata->sensor_data.common_chromatix_ptr;
      module_chromatix.chromatixPtr = priv_metadata->sensor_data.chromatix_ptr;
      /* Send chromatix pointer downstream */
      rc = pproc_module_send_set_chromatix_event(port, identity,
        &module_chromatix);
      if (rc == FALSE) {
        CDBG_ERROR("%s:%d failed: send stream crop event\n", __func__,
          __LINE__);
        rc = FALSE;
        goto ERROR;
      }

      /* Extract aec update from meta data */
      stats_get =
        (stats_get_data_t *)&priv_metadata->stats_aec_data.private_data;
      memset(&stats_update, 0, sizeof(stats_update_t));
      stats_update.flag = stats_get->flag;
      aec_update = &stats_update.aec_update;
      if (stats_update.flag) {
        aec_update->lux_idx = stats_get->aec_get.lux_idx;
        aec_update->real_gain = stats_get->aec_get.real_gain[0];
        CDBG_ERROR("%s:%d ###AEC update %f %f,identity:0x%x", __func__,
          __LINE__, aec_update->lux_idx, aec_update->real_gain,identity);
        /* Send AEC_UPDATE event */
        rc = pproc_module_send_aec_update_event(port, identity,
          &stats_update);
        if (rc == FALSE) {
          CDBG_ERROR("%s:%d failed: send aec update event\n", __func__,
            __LINE__);
          rc = FALSE;
          goto ERROR;
        }
      }
      /* Extract gamma update from meta data and send downstream */
      memcpy(&gamma_update, &priv_metadata->isp_gamma_data,
        (sizeof(uint16_t) * 64));
      rc = pproc_module_send_gamma_update_event(port, identity,
        (uint16_t *)&gamma_update[0]);
      if (rc == FALSE) {
        CDBG_ERROR("%s:%d failed: send gamma update event\n", __func__,
          __LINE__);
        rc = FALSE;
        goto ERROR;
      }

      /* Extract awb update from meta data and send downstream */
      memcpy(&awb_update, &priv_metadata->isp_stats_awb_data,
        sizeof(awb_update_t));
      rc = pproc_module_send_awb_update_event(port, identity, &awb_update);
      if (rc == FALSE) {
        CDBG_ERROR("%s:%d failed: send awb update event\n", __func__,
          __LINE__);
        rc = FALSE;
        goto ERROR;
      }
    }
  //}

  /* check for denoise in feature mask */
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_DENOISE2D) {
    rc = pproc_module_send_set_param_event(port, identity, pp_feature_config,
      CAM_INTF_PARM_WAVELET_DENOISE);
    if (rc == FALSE) {
      CDBG_ERROR("%s:%d failed: send stream crop event\n", __func__, __LINE__);
      goto ERROR;
    }
  }

  CDBG_HIGH("%s:%d input buf index %d input stream id %d"
    "stream type %d\n", __func__, __LINE__, parm_buf->reprocess.buf_index,
    stream_info->reprocess_config.online.input_stream_id,
    stream_info->reprocess_config.online.input_stream_type);

  /* check for effect in feature mask */
  if(pp_feature_config->feature_mask & CAM_QCOM_FEATURE_EFFECT) {
    rc = pproc_module_send_set_param_event(port, identity, pp_feature_config,
      CAM_INTF_PARM_EFFECT);
    if (rc == FALSE) {
     CDBG_ERROR("%s:%d failed: send stream effect event\n", __func__,
       __LINE__);
     goto ERROR;
    }
  }
  /* check for sharpness in feature mask */
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_SHARPNESS) {
    rc = pproc_module_send_set_param_event(port, identity, pp_feature_config,
      CAM_INTF_PARM_SHARPNESS);
    if (rc == FALSE) {
     CDBG_ERROR("%s:%d failed: send stream crop event\n", __func__,
       __LINE__);
     goto ERROR;
    }
  }

  /* check for flip in feature mask */
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_FLIP) {
    rc = pproc_module_send_set_param_event(port, identity, pp_feature_config,
       CAM_INTF_PARM_STREAM_FLIP);
    if (rc == FALSE) {
       CDBG_ERROR("%s:%d failed: send stream flip event\n", __func__,
         __LINE__);
       goto ERROR;
    }
  }

  /* check for rotation in feature mask */
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_ROTATION) {
    rc = pproc_module_send_set_param_event(port, identity, pp_feature_config,
       CAM_INTF_PARM_ROTATION);
    if (rc == FALSE) {
       CDBG_ERROR("%s:%d failed: send stream rotation event\n", __func__,
         __LINE__);
       goto ERROR;
    }
  }
#ifdef USE_PREVIEW_FD_INFO_FOR_TP
  /* check if trueportrait is enabled */
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_TRUEPORTRAIT) {
    memcpy(&faces_data, &metadata->faces_data,
      sizeof(cam_face_detection_data_t));

    rc = pproc_module_send_fd_update_event(port, identity, &faces_data);
    if (rc == FALSE) {
      CDBG_ERROR("%s:%d failed: send fd event\n", __func__,
        __LINE__);
      rc = FALSE;
      goto ERROR;
    }
  }
#endif
  /* Pick Input buffer from different stream */
  buf_holder = pproc_module_get_online_input_buffer(module, stream_info,
    parm_buf, identity);
  if (!buf_holder) {
    CDBG_ERROR("%s:%d failed: buf_holder NULL\n", __func__, __LINE__);
    rc = FALSE;
    goto ERROR;
  }
  CDBG_HIGH("%s:%d input buf fd %d type %d size %d\n", __func__, __LINE__,
    buf_holder->buf_planes[0].fd, buf_holder->buf_type, buf_holder->buf_size);
  /* Send buf divert downstream */
  rc = pproc_module_send_buf_divert_event(module, port, identity, buf_holder,
    parm_buf);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed: send buf divert event\n", __func__, __LINE__);
    rc = FALSE;
    goto ERROR;
  }

ERROR:
  return rc;
}

/**
* Function: pproc_module_get_meta_buffer
*
* Description: Function used as callback to find
*   metadata buffer wiht corresponding index
*
* Input parameters:
*   @data - MCT stream buffer list
*   @user_data - Pointer to searched buffer index
*
* Return values:
*     true/false
*
* Notes: none
**/
static boolean pproc_module_get_meta_buffer(void *data, void *user_data)
{
  mct_stream_map_buf_t *buf = (mct_stream_map_buf_t *)data;
  uint8_t *buf_index = (uint8_t *)user_data;

  if (!buf || !buf_index) {
    CDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  CDBG("%s:%d] buf type %d buff index %d search index %d",
      __func__, __LINE__, buf->buf_type, buf->buf_index, *buf_index);

  /* For face detection is used stream buff type */
  if (buf->buf_type != CAM_MAPPING_BUF_TYPE_OFFLINE_META_BUF)
    return FALSE;

  return ((uint8_t)buf->buf_index == *buf_index);
}

/** pproc_module_handle_reprocess_offline
 *    @module: pproc module
 *
 *  Handle pproc_module_handle_reprocess_offline
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_handle_reprocess_offline(mct_module_t *module,
  mct_port_t *port, mct_stream_info_t *input_stream_info,
  cam_stream_parm_buffer_t *parm_buf, unsigned int identity)
{
  boolean                      rc = FALSE;
  uint32_t                     session_id = 0;
  uint32_t                     stream_id = 0;
  cam_pp_offline_src_config_t *offline_src_cfg;
  mct_bus_msg_stream_crop_t    stream_crop;
  cam_pp_feature_config_t     *pp_feature_config;
  mct_stream_map_buf_t        *buf_holder = NULL;
  cam_crop_param_t            *crop = NULL;

  /* Validate input parameters */
  if (!module || !input_stream_info || !parm_buf) {
    CDBG_ERROR("%s:%d failed: module %p input_stream_info %p parm_buf %p\n",
      __func__, __LINE__, module, input_stream_info, parm_buf);
    goto ERROR;
  }

  session_id = PPROC_GET_SESSION_ID(identity);
  stream_id = PPROC_GET_STREAM_ID(identity);

  /* Validate number of planes */
  if (input_stream_info->buf_planes.plane_info.num_planes == 0) {
    CDBG_ERROR("%s:%d input stream info num planes 0\n", __func__, __LINE__);
    rc = FALSE;
    goto ERROR;
  }

  /* Pass output dim event */
  offline_src_cfg = &input_stream_info->reprocess_config.offline;
  rc = pproc_module_send_output_dim_event(module, port, identity,
    &offline_src_cfg->input_dim,
    offline_src_cfg->input_buf_planes.plane_info.mp,
    offline_src_cfg->input_fmt);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed: send stream crop event\n", __func__, __LINE__);
    goto ERROR;
  }

  /* TODO: when HAL implements single place holder for all the pp_config
     we will switch to using this. For now use the other pp_feature_config
     pp_feature_config = &input_stream_info->pp_config; */
  pp_feature_config = &input_stream_info->reprocess_config.pp_feature_config;

  crop = &parm_buf->reprocess.frame_pp_config.crop;
  /* check for CROP in feature mask */
  if (crop->crop_enabled == 1) {
    memset(&stream_crop, 0, sizeof(mct_bus_msg_stream_crop_t));
    stream_crop.x = crop->input_crop.left;
    stream_crop.y = crop->input_crop.top;
    stream_crop.crop_out_x = crop->input_crop.width;
    stream_crop.crop_out_y = crop->input_crop.height;
    rc = pproc_module_send_stream_crop_event(port, identity, &stream_crop);
    if (rc == FALSE) {
      CDBG_ERROR("%s:%d failed: send stream crop event\n", __func__, __LINE__);
      goto ERROR;
    }
  }

   if (parm_buf->reprocess.meta_present == 1) {
     /* Extract meta buffer */
     mct_list_t  *temp_list = NULL;
     cam_metadata_info_t  *metadata = NULL;

     temp_list = mct_list_find_custom(input_stream_info->img_buffer_list,
       &parm_buf->reprocess.meta_buf_index,pproc_module_get_meta_buffer);
     if (temp_list && temp_list->data) {
       mct_stream_map_buf_t *buff_holder = temp_list->data;
       metadata = buff_holder->buf_planes[0].buf;
     }
     if (metadata) {
       modulesChromatix_t                module_chromatix;
       mct_stream_session_metadata_info *priv_metadata;
       stats_get_data_t                 *stats_get = NULL;
       stats_update_t                    stats_update;
       aec_update_t                     *aec_update = NULL;
       awb_update_t                      awb_update;
       uint16_t                          gamma_update[64];

       /* Extract chromatix params from meta data */
       memset(&module_chromatix, 0, sizeof(modulesChromatix_t));
       priv_metadata =
         (mct_stream_session_metadata_info *)metadata->private_metadata;
       module_chromatix.chromatixComPtr =
         priv_metadata->sensor_data.common_chromatix_ptr;
       module_chromatix.chromatixPtr = priv_metadata->sensor_data.chromatix_ptr;
       /* Send chromatix pointer downstream */
       rc = pproc_module_send_set_chromatix_event(port, identity,
         &module_chromatix);
       if (rc == FALSE) {
         CDBG_ERROR("%s:%d failed: send stream crop event\n", __func__,
           __LINE__);
       }

       /* Extract aec update from meta data */
       stats_get =
         (stats_get_data_t *)&priv_metadata->stats_aec_data.private_data;
       memset(&stats_update, 0, sizeof(stats_update_t));
       stats_update.flag = STATS_UPDATE_AEC;
       aec_update = &stats_update.aec_update;
       if (stats_get) {
         aec_update->lux_idx = stats_get->aec_get.lux_idx;
         aec_update->real_gain = stats_get->aec_get.real_gain[0];
         /* Send AEC_UPDATE event */
         rc = pproc_module_send_aec_update_event(port, identity,
           &stats_update);
         if (rc == FALSE) {
           CDBG_ERROR("%s:%d failed: send aec update event\n", __func__,
              __LINE__);
         }
       }

       /* Extract gamma update from meta data and send downstream */
       memcpy(&gamma_update, &priv_metadata->isp_gamma_data,
         (sizeof(uint16_t) * 64));
       rc = pproc_module_send_gamma_update_event(port, identity,
         (uint16_t *)&gamma_update[0]);
       if (rc == FALSE) {
         CDBG_ERROR("%s:%d failed: send gamma update event\n", __func__,
           __LINE__);
       }

       /* Extract awb update from meta data and send downstream */
       memcpy(&awb_update, &priv_metadata->isp_stats_awb_data,
         sizeof(awb_update_t));
       rc = pproc_module_send_awb_update_event(port, identity, &awb_update);
      if (rc == FALSE) {
         CDBG_ERROR("%s:%d failed: send awb update event\n", __func__,
           __LINE__);
       }
     } else {
     CDBG_ERROR("%s:%d] Metadata buffer idx %d is not available",
       __func__, __LINE__,parm_buf->reprocess.meta_buf_index );
     }
#ifdef USE_PREVIEW_FD_INFO_FOR_TP
    /* check if trueportrait is enabled */
    if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_TRUEPORTRAIT) {
      cam_face_detection_data_t faces_data;
      memcpy(&faces_data, &metadata->faces_data,
        sizeof(cam_face_detection_data_t));

      rc = pproc_module_send_fd_update_event(port, identity, &faces_data);
      if (rc == FALSE) {
        CDBG_ERROR("%s:%d failed: send fd event\n", __func__,
          __LINE__);
        rc = FALSE;
        goto ERROR;
      }
    }
#endif
  }
  /* check for denoise in feature mask */
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_DENOISE2D) {
    rc = pproc_module_send_set_param_event(port, identity, pp_feature_config,
      CAM_INTF_PARM_WAVELET_DENOISE);
    if (rc == FALSE) {
      CDBG_ERROR("%s:%d failed: send stream crop event\n", __func__, __LINE__);
      goto ERROR;
    }
  }

  /* check for effect in feature mask */
  if(pp_feature_config->feature_mask & CAM_QCOM_FEATURE_EFFECT) {
    rc = pproc_module_send_set_param_event(port, identity, pp_feature_config,
      CAM_INTF_PARM_EFFECT);
    if (rc == FALSE) {
     CDBG_ERROR("%s:%d failed: send stream effect event\n", __func__,
       __LINE__);
     goto ERROR;
    }
  }

  /* check for sharpness in feature mask */
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_SHARPNESS) {
    rc = pproc_module_send_set_param_event(port, identity, pp_feature_config,
       CAM_INTF_PARM_SHARPNESS);
    if (rc == FALSE) {
      CDBG_ERROR("%s:%d failed: send stream crop event\n", __func__,
       __LINE__);
      goto ERROR;
    }
  }

  /* check for flip in feature mask */
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_FLIP) {
    rc = pproc_module_send_set_param_event(port, identity, pp_feature_config,
       CAM_INTF_PARM_STREAM_FLIP);
    if (rc == FALSE) {
       CDBG_ERROR("%s:%d failed: send stream flip event\n", __func__,
         __LINE__);
       goto ERROR;
    }
  }

  /* check for rotation in feature mask */
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_ROTATION) {
    rc = pproc_module_send_set_param_event(port, identity, pp_feature_config,
      CAM_INTF_PARM_ROTATION);
    if (rc == FALSE) {
      CDBG_ERROR("%s:%d failed: send stream rotation event\n", __func__,
       __LINE__);
      goto ERROR;
    }
  }
  /* Pick Input buffer from different stream */
  buf_holder = pproc_module_get_offline_input_buffer(module, parm_buf,
    identity);
  if (!buf_holder) {
    CDBG_ERROR("%s:%d failed: buf_holder NULL\n", __func__, __LINE__);
    rc = FALSE;
    goto ERROR;
  }
  CDBG_HIGH("%s:%d input buf fd %d type %d size %d\n", __func__, __LINE__,
    buf_holder->buf_planes[0].fd, buf_holder->buf_type, buf_holder->buf_size);

  /* Send buf divert downstream */
  rc = pproc_module_send_buf_divert_event(module, port, identity, buf_holder,
    parm_buf);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed: send buf divert event\n", __func__, __LINE__);
    goto ERROR;
  }

  return TRUE;

ERROR:
  return rc;
}

/** pproc_module_offline_stream_param_buf
 *    @module: pproc module
 *    @event: mct event to be handled
 *    @data: stream info
 *
 *  Handle stream param buf event for offline stream. Use input
 *  buffer index sent as part of this call for performing post
 *  processing. Use crop information, chromatix pointers and AEC
 *  trigger from stream info and meta data respectively.
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_offline_stream_param_buf(mct_module_t *module,
  mct_event_t *event, void *data)
{
  boolean                     rc = TRUE;
  uint32_t                    i = 0;
  mct_port_t                 *port = NULL;
  mct_stream_info_t          *stream_info = NULL;
  cam_stream_parm_buffer_t   *parm_buf = NULL;

  /* Validate input pameters */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "pproc") || !event || !data) {
    CDBG_ERROR("%s:%d failed: data %p\n", __func__, __LINE__, data);
    rc = FALSE;
    goto ERROR;
  }

  parm_buf = (cam_stream_parm_buffer_t *)data;

  /* Validate parm buf type */
  if (parm_buf->type != CAM_STREAM_PARAM_TYPE_DO_REPROCESS
    && parm_buf->type != CAM_STREAM_PARAM_TYPE_GET_OUTPUT_CROP
    && parm_buf->type != CAM_STREAM_PARAM_TYPE_GET_IMG_PROP) {

    CDBG_ERROR("%s:%d failed: parm_buf.type is invalid\n", __func__, __LINE__);
    rc = TRUE;
    goto ERROR;
  }

  /* Get reserved pproc sink port for this identity */
  port = pproc_port_get_reserved_port(module, event->identity);
  if (!port) {
    CDBG_ERROR("%s:%d failed: reserved port NULL", __func__, __LINE__);
    rc = FALSE;
    goto ERROR;
  }

  /* Get stream info for pproc sink port */
  stream_info = pproc_port_get_attached_stream_info(port, event->identity);
  if (!stream_info) {
    CDBG_ERROR("%s:%d failed: stream info NULL\n", __func__, __LINE__);
    rc = FALSE;
    goto ERROR;
  }

  /* There is possibility for downstream modules (like sw-wnr) to expect this event */
  rc = port->event_func(port, event);
  if (FALSE == rc) {
    CDBG_ERROR("%s:%d] Error sending stream param buf event downstream\n", __func__, __LINE__);
    goto ERROR;
  }

  if (CAM_STREAM_PARAM_TYPE_DO_REPROCESS == parm_buf->type) {
    /* Find the type of reprocess, offline / online */
    if (stream_info->reprocess_config.pp_type == CAM_ONLINE_REPROCESS_TYPE) {
      rc = pproc_module_handle_reprocess_online(module, port, stream_info,
        parm_buf, event->identity);
      if (rc == FALSE) {
        CDBG_ERROR("%s:%d failed: handle reprocess online\n", __func__,
          __LINE__);
        rc = FALSE;
        goto ERROR;
      }
    } else if (stream_info->reprocess_config.pp_type ==
      CAM_OFFLINE_REPROCESS_TYPE) {
      rc = pproc_module_handle_reprocess_offline(module, port, stream_info,
        parm_buf, event->identity);
      if (rc == FALSE) {
        CDBG_ERROR("%s:%d failed: handle reprocess offline\n", __func__,
          __LINE__);
        rc = FALSE;
        goto ERROR;
      }
    }
  }

ERROR:
  return rc;
}

/** pproc_module_offline_streamoff
 *    @module: pproc module
 *    @event: mct event to be handled
 *    @data: stream info
 *
 *  Handle stream off event for offline stream.
 *
 *  Return: TRUE TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_offline_streamoff(mct_module_t *module,
  mct_event_t *event, void *data)
{
  boolean     rc = TRUE;
  mct_list_t *lport = NULL;
  mct_port_t *port = NULL;

  CDBG("%s:%d data %p\n", __func__, __LINE__, data);
  /* Validate input pameters */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "pproc") || !event || !data) {
    CDBG_ERROR("%s:%d failed: data %p\n", __func__, __LINE__, data);
    rc = FALSE;
    goto ERROR;
  }

  /* Find pproc port for this identity */
  lport = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), &event->identity,
    pproc_port_check_identity_in_port);
  if (!lport) {
    CDBG_ERROR("%s:%d failed: to find pproc port\n", __func__, __LINE__);
    rc = FALSE;
    goto ERROR;
  }

  /* Extract pproc port from mct list */
  port = (mct_port_t *)lport->data;
  if (!port) {
    CDBG_ERROR("%s:%d failed: reserved port NULL", __func__, __LINE__);
    rc = FALSE;
    goto ERROR;
  }

  /* Call streamoff event on pproc port to get forwarded to sub module */
  rc = port->event_func(port, event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed: to stream off sub module\n", __func__, __LINE__);
    rc = FALSE;
  }

ERROR:
  return rc;
}

/** pproc_module_offline_deletestream
 *    @module: pproc module
 *    @event: mct event to be handled
 *    @data: stream info
 *
 *  Call unlink and caps unreserve on submodule's sink port that is used for
 *  offline stream.
 *
 *  Return: TRUE TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_offline_deletestream(mct_module_t *module,
  mct_event_t *event, void *data)
{
  boolean     rc = TRUE;
  mct_list_t *lport = NULL;
  mct_port_t *port = NULL;

  CDBG("%s:%d data %p\n", __func__, __LINE__, data);
  /* Validate input pameters */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "pproc") || !event || !data) {
    CDBG_ERROR("%s:%d failed: data %p\n", __func__, __LINE__, data);
    rc = FALSE;
    goto ERROR;
  }

  /* Find pproc port for this identity */
  lport = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), &event->identity,
    pproc_port_check_identity_in_port);
  if (!lport) {
    CDBG_ERROR("%s:%d failed: to find pproc port\n", __func__, __LINE__);
    rc = FALSE;
    goto ERROR;
  }

  /* Extract pproc port from mct list */
  port = (mct_port_t *)lport->data;
  if (!port) {
    CDBG_ERROR("%s:%d failed: reserved port NULL", __func__, __LINE__);
    rc = FALSE;
    goto ERROR;
  }

  rc = port->check_caps_unreserve(port, event->identity);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d failed: to caps unreserve\n", __func__, __LINE__);
    rc = FALSE;
  }
  mct_port_remove_child(event->identity, port);
ERROR:
  return rc;
}

/** pproc_module_process_event
 *    @module: pproc module
 *    @event: mct event to be handled
 *
 *  Handle event set on this module. As per current
 *  architecture, his path is exercised only for reprocessing
 *
 *  Return: TRUE if event is handled successfully
 *          FALSE otherwise **/
static boolean pproc_module_process_event(mct_module_t *module,
  mct_event_t *event)
{
  boolean              rc = TRUE;
  mct_event_control_t *ctrl_event = NULL;

  /* Validate input parameters */
  if (!module || !event || strcmp(MCT_OBJECT_NAME(module), "pproc")) {
    CDBG_ERROR("%s:%d falied: module %p event %p\n", __func__, __LINE__, module,
      event);
    rc = FALSE;
    goto ERROR;
  }

  /* Check whether event's type is MCT_EVENT_CONTROL_CMD */
  if (event->type != MCT_EVENT_CONTROL_CMD) {
    CDBG_ERROR("%s:%d failed invalid event type %d\n", __func__, __LINE__,
      event->type);
    rc = FALSE;
    goto ERROR;
  }

  ctrl_event = &event->u.ctrl_event;
  CDBG_LOW("event %d", ctrl_event->type);
  switch (ctrl_event->type) {
  case MCT_EVENT_CONTROL_STREAMON:
    rc = pproc_module_offline_streamon(module, event,
      ctrl_event->control_event_data);
    break;
  case MCT_EVENT_CONTROL_PARM_STREAM_BUF:
    rc = pproc_module_offline_stream_param_buf(module, event,
      ctrl_event->control_event_data);
    break;
  case MCT_EVENT_CONTROL_STREAMOFF:
    rc = pproc_module_offline_streamoff(module, event,
      ctrl_event->control_event_data);
    break;
  case MCT_EVENT_CONTROL_DEL_OFFLINE_STREAM:
    rc = pproc_module_offline_deletestream(module, event,
      ctrl_event->control_event_data);
    break;
  default:
    CDBG("%s:%d invalid control event type %d\n", __func__, __LINE__,
      ctrl_event->type);
    rc = FALSE;
    break;
  }

ERROR:
  return rc;
}

/** get_pproc_loglevel:
 *
 *  Args:
 *  Return:
 *    void
 **/

void get_pproc_loglevel()
{
  uint32_t temp;
  uint32_t log_level;
  uint32_t debug_mask;
  memset(pproc_prop, 0, sizeof(pproc_prop));
  /**  Higher 4 bits : Value of Debug log level (Default level is 1 to print all CDBG_HIGH)
       Lower 28 bits : Control mode for sub module logging(Only 3 sub modules in PPROC )
       0x1 for PPROC
       0x10 for C2D
       0x100 for CPP  */
  property_get("persist.camera.pproc.debug.mask", pproc_prop, "268435463"); // 0x10000007=268435463
  temp = atoi(pproc_prop);
  log_level = ((temp >> 28) & 0xF);
  debug_mask = (temp & PPROC_DEBUG_MASK_PPROC);
  if (debug_mask > 0)
      gCamPprocLogLevel = log_level;
  else
      gCamPprocLogLevel = 0; // Debug logs are not required if debug_mask is zero
}

/** pproc_module_start_session
 *    @module:   pproc module
 *    @identity: stream|session identity
 *
 *  Call submodule start session function.
 *
 *  Return TRUE on success.
 **/
static boolean pproc_module_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  pproc_module_private_t  *mod_private;
  boolean                  rc = FALSE;

  get_pproc_loglevel(); //dynamic logging
  CDBG("%s:%d] E\n", __func__, __LINE__);
  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "pproc")) {
    CDBG_ERROR("%s:%d] error module: %p\n", __func__, __LINE__, module);
    return rc;
  }

  MCT_OBJECT_LOCK(module);
  mod_private = MCT_OBJECT_PRIVATE(module);
  if (!mod_private) {
    CDBG_ERROR("%s:%d] error module private is NULL\n", __func__, __LINE__);
    goto start_session_done;
  }

#if 0
  /* TODO: set_mod() is not called during removing the stream */
  /* check to see if the session has already been existing */
  if (mct_list_find_custom(mod_private->module_type_list,
        &sessionid, pproc_module_check_session) != NULL) {
    rc = TRUE;
    goto start_session_done;
  }
#endif

  if (mod_private->cpp_module) {
    if (mod_private->cpp_module->start_session(mod_private->cpp_module,
          sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in cpp start session\n", __func__, __LINE__);
      goto start_session_done;
    }
  }

  if (mod_private->vpe_module) {
    if (mod_private->vpe_module->start_session(mod_private->vpe_module,
          sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in vpe start session\n", __func__, __LINE__);
      goto start_session_done;
    }
  }

  if (mod_private->c2d_module) {
    if (mod_private->c2d_module->start_session(mod_private->c2d_module,
          sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in c2d start session\n", __func__, __LINE__);
      goto start_session_done;
    }
  }

  if (mod_private->cac_module) {
    if (mod_private->cac_module->start_session(mod_private->cac_module,
          sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in cac start session\n", __func__, __LINE__);
      goto start_session_done;
    }
  }

  if (mod_private->wnr_module) {
    if (mod_private->wnr_module->start_session(mod_private->wnr_module,
          sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in wnr start session\n", __func__, __LINE__);
      goto start_session_done;
    }
  }

  if (mod_private->hdr_module) {
    if (mod_private->hdr_module->start_session(mod_private->hdr_module,
          sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in hdr start session\n", __func__, __LINE__);
      goto start_session_done;
    }
  }

  if (mod_private->llvd_module) {
    if (mod_private->llvd_module->start_session(mod_private->llvd_module,
          sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in llvd start session\n", __func__, __LINE__);
      goto start_session_done;
    }
  }

  rc = TRUE;

start_session_done:
  MCT_OBJECT_UNLOCK(module);
  CDBG("%s:%d] X\n", __func__, __LINE__);
  return rc;
}

/** pproc_module_stop_session
 *    @module: PPROC module
 *    @identity: stream|session identity
 *
 *  Call submodule stop session function
 *
 *  Return TRUE on success.
 **/
static boolean pproc_module_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  pproc_module_private_t  *mod_private;
  boolean                  rc = FALSE;

  CDBG("%s:%d] E\n", __func__, __LINE__);
  /* Sanity check */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "pproc")) {
    CDBG_ERROR("%s:%d] error module: %p\n", __func__, __LINE__, module);
    return rc;
  }

  MCT_OBJECT_LOCK(module);
  mod_private = MCT_OBJECT_PRIVATE(module);
  if (!mod_private) {
    CDBG_ERROR("%s:%d] error module private is NULL\n", __func__, __LINE__);
    goto stop_session_done;
  }

#if 0
  /* TODO: set_mod() is not called during removing the stream */
  /* check to see if the session has already been existing */
  if (mct_list_find_custom(mod_private->module_type_list,
        &sessionid, pproc_module_check_session) != NULL) {
    rc = TRUE;
    goto stop_session_done;
  }
#endif

  if (mod_private->cpp_module) {
    if (mod_private->cpp_module->stop_session(mod_private->cpp_module,
          sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in cpp stop session\n", __func__, __LINE__);
      goto stop_session_done;
    }
  }

  if (mod_private->vpe_module) {
    if (mod_private->vpe_module->stop_session(mod_private->vpe_module,
          sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in vpe stop session\n", __func__, __LINE__);
      goto stop_session_done;
    }
  }

  if (mod_private->c2d_module) {
    if (mod_private->c2d_module->stop_session(mod_private->c2d_module,
          sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in c2d stop session\n", __func__, __LINE__);
      goto stop_session_done;
    }
  }

  if (mod_private->cac_module) {
    if (mod_private->cac_module->stop_session(mod_private->cac_module,
          sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in cac stop session\n", __func__, __LINE__);
      goto stop_session_done;
    }
  }

  if (mod_private->wnr_module) {
    if (mod_private->wnr_module->stop_session(mod_private->wnr_module,
          sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in wnr stop session\n", __func__, __LINE__);
      goto stop_session_done;
    }
  }

  if (mod_private->hdr_module) {
    if (mod_private->hdr_module->stop_session(mod_private->hdr_module,
          sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in hdr stop session\n", __func__, __LINE__);
      goto stop_session_done;
    }
  }

  if (mod_private->llvd_module) {
    if (mod_private->llvd_module->stop_session(mod_private->llvd_module,
          sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in llvd stop session\n", __func__, __LINE__);
      goto stop_session_done;
    }
  }

  rc = TRUE;

stop_session_done:
  MCT_OBJECT_UNLOCK(module);
  CDBG("%s:%d] X\n", __func__, __LINE__);
  return rc;
}

/** pproc_module_request_new_port
 *    @stream_info:
 *    @direction:
 *    @module:
 *    @peer_caps:
 *
 *    Create new port dynamically due to lack of support from
 *    existing ports.
 **/
static mct_port_t *pproc_module_request_new_port(void *stream_info,
  mct_port_direction_t direction, mct_module_t *module, void *peer_caps)
{
  /* TODO:
   *
   *   This function may be triggered after LINK fails due to lack of port
   *   support or when pproc is a source port but MCT needs a sink port to
   *   connect to it and communicate through this port.
   *
   *   Dependes on the nature of stream_info and checking with sub-modules,
   *   i.e, CPP/VPE/C2D to see whether or not new port can be created. If
   *   yes, create a corresponding new port.
   */
  return NULL;
}

/** pproc_module_set_mod
*     @module:      pproc module itself ("pproc")
 *    @module_type: module type to set
 *    @identity:    stream|session identity
 *
 *  Set this module the type for the stream(from identity).
 *
 *  Return TRUE on success
 **/
static void pproc_module_set_mod(mct_module_t *module,
  unsigned int module_type, unsigned int identity)
{
  mct_module_type_t       type = (mct_module_type_t)module_type;
  pproc_module_private_t *mod_private;
  pproc_module_type_t    *pproc_type;
  mct_list_t             *list;

  CDBG("%s:%d] E\n", __func__, __LINE__);
  if (!module || strcmp(MCT_OBJECT_NAME(module), "pproc")) {
    CDBG_ERROR("%s:%d] error module: %p\n", __func__, __LINE__, module);
    return;
  }

  MCT_OBJECT_LOCK(module);
  mod_private = (pproc_module_private_t *)MCT_OBJECT_PRIVATE(module);

  /* If set_mod for same identity then remove the existing entry */
  list = mct_list_find_custom(mod_private->module_type_list, &identity,
    pproc_module_check_identity);

  if (list) {
    pproc_type = list->data;
    mod_private->module_type_list =
      mct_list_remove(mod_private->module_type_list, pproc_type);
    free(pproc_type);
  }

  if (MCT_MODULE_FLAG_INVALID == module_type) {
    MCT_OBJECT_UNLOCK(module);
    return;
  }

  pproc_type = malloc(sizeof(pproc_module_type_t));
  if (!pproc_type) {
    CDBG_ERROR("%s:%d] error in memory allocation\n", __func__, __LINE__);
    MCT_OBJECT_UNLOCK(module);
    return;
  }
  pproc_type->identity = identity;
  pproc_type->module_type = module_type;

  list = mct_list_append(mod_private->module_type_list, pproc_type, NULL, NULL);
  if (!list) {
    CDBG_ERROR("%s:%d] error appending in module type list\n", __func__,
      __LINE__);
    free(pproc_type);
    MCT_OBJECT_UNLOCK(module);
    return;
  }

  mod_private->module_type_list = list;
  mct_module_add_type(module, module_type, identity);
  MCT_OBJECT_UNLOCK(module);

  CDBG("%s:%d] X\n", __func__, __LINE__);
  return;
}

/** pproc_module_query_mod
 *     @module:     pproc module itself ("pproc")
 *     @query_buf:  media controller's query information buffer
 *     @sessionid:  session and stream identity
 *
 *  pproc module's capability is based on the sub-modules and
 *  independant of the session id. This function is used to
 *  query submodule capability by simply pass the query buffer
 *  to sub-modules so that the can be filled out accordingly.
 *
 *  Return Sub-module's query result
 **/
static boolean pproc_module_query_mod(mct_module_t *module,
  void *buf, unsigned int sessionid)
{
  pproc_module_private_t *mod_private;
  boolean                 rc = FALSE;
  mct_pipeline_cap_t     *query_buf = (mct_pipeline_cap_t *)buf;

  CDBG("%s:%d] E\n", __func__, __LINE__);
  if (!module || !query_buf || strcmp(MCT_OBJECT_NAME(module), "pproc")) {
    CDBG_ERROR("%s:%d] error module: %p query_buf: %p\n", __func__, __LINE__,
      module, query_buf);
    return rc;
  }

  MCT_OBJECT_LOCK(module);
  mod_private = (pproc_module_private_t *)MCT_OBJECT_PRIVATE(module);

  /* TODO: We should probably independently store the sub-mods caps and give
     the aggregate caps to media-controller */

  memset(&query_buf->pp_cap, 0, sizeof(mct_pipeline_pp_cap_t));
  if (mod_private->cpp_module && mod_private->cpp_module->query_mod) {
    if (mod_private->cpp_module->query_mod(mod_private->cpp_module, query_buf,
      sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in cpp query mod\n", __func__, __LINE__);
      goto query_mod_done;
    }
  }

  if (mod_private->vpe_module && mod_private->vpe_module->query_mod) {
    if (mod_private->vpe_module->query_mod(mod_private->vpe_module, query_buf,
      sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in vpe query mod\n", __func__, __LINE__);
      goto query_mod_done;
    }
  }

  if (mod_private->c2d_module && mod_private->c2d_module->query_mod) {
    if (mod_private->c2d_module->query_mod(mod_private->c2d_module, query_buf,
      sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in c2d query mod\n", __func__, __LINE__);
      goto query_mod_done;
    }
  }

  if (mod_private->cac_module && mod_private->cac_module->query_mod) {
    if (mod_private->cac_module->query_mod(mod_private->cac_module, query_buf,
      sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in cac query mod\n", __func__, __LINE__);
      goto query_mod_done;
    }
  }

  if (mod_private->wnr_module && mod_private->wnr_module->query_mod) {
    if (mod_private->wnr_module->query_mod(mod_private->wnr_module, query_buf,
      sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in wnr query mod\n", __func__, __LINE__);
      goto query_mod_done;
    }
  }

  if (mod_private->hdr_module && mod_private->hdr_module->query_mod) {
    if (mod_private->hdr_module->query_mod(mod_private->hdr_module, query_buf,
      sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in hdr query mod\n", __func__, __LINE__);
      goto query_mod_done;
    }
  }

  if (mod_private->llvd_module && mod_private->llvd_module->query_mod) {
    if (mod_private->llvd_module->query_mod(mod_private->llvd_module, query_buf,
      sessionid) == FALSE) {
      CDBG_ERROR("%s:%d] error in llvd query mod\n", __func__, __LINE__);
      goto query_mod_done;
    }
  }

  rc = TRUE;

query_mod_done:
  MCT_OBJECT_UNLOCK(module);
  CDBG("%s:%d] feature_mask 0x%x X\n", __func__, __LINE__,
    query_buf->pp_cap.feature_mask);
  return rc;
}

/** pproc_module_deinit
 *    @mod: PPROC module object
 *
 *  Function for MCT to deinit PPROC module. This will remove
 *  all the ports of this module. Also call deinit for the
 *  submods.
 *
 *  Return: NULL
 **/
void pproc_module_deinit(mct_module_t *module)
{
  pproc_module_private_t *mod_private;
  int i = 0;
  int numsrcports = 0;
  int numsinkports = 0;

  CDBG("%s:%d] E\n", __func__, __LINE__);
  if (!module || strcmp(MCT_OBJECT_NAME(module), "pproc"))
    return;

  mod_private = (pproc_module_private_t *)MCT_OBJECT_PRIVATE(module);

#ifdef CAMERA_FEATURE_CPP
  if (mod_private->cpp_module)
    cpp_module_deinit(mod_private->cpp_module);
#endif

#ifdef CAMERA_FEATURE_VPE
  if (mod_private->vpe_module)
    vpe_module_deinit(mod_private->vpe_module);
#endif

  if (mod_private->c2d_module)
    c2d_module_deinit(mod_private->c2d_module);

  if (mod_private->cac_module)
    module_cac_deinit(mod_private->cac_module);

#ifdef CAMERA_FEATURE_WNR_SW
  if (mod_private->wnr_module)
    module_wnr_deinit(mod_private->wnr_module);
  if (mod_private->hdr_module)
    module_hdr_deinit(mod_private->hdr_module);
#else
  if (mod_private->wnr_module)
    wnr_module_deinit(mod_private->wnr_module);
#endif

  if (mod_private->llvd_module)
    module_llvd_deinit(mod_private->llvd_module);

  numsrcports = module->numsrcports;
  numsinkports = module->numsinkports;

  for (i = 0; i < numsinkports; i++) {
    pproc_module_free_port(MCT_MODULE_SINKPORTS(module)->data,module);
  }
  for (i = 0; i < numsrcports; i++) {
   pproc_module_free_port(MCT_MODULE_SRCPORTS(module)->data,module);
  }

  mct_list_free_list(MCT_MODULE_SRCPORTS(module));
  mct_list_free_list(MCT_MODULE_SINKPORTS(module));

  /* TODO: Modules children is port which is deleted above ! Is there a
     consideration to make the streams as children of PPROC */
  //mct_list_free_list(MCT_MODULE_CHILDREN(module));

  mct_list_free_all(mod_private->module_type_list, pproc_module_free_type_list);
  free(mod_private);
  mct_module_destroy(module);

  CDBG("%s:%d] X\n", __func__, __LINE__);
  return;
}

/** pproc_module_create_default_ports
 *    @pproc: this pproc module object
 *
 *  By default, create 2 sink ports and 2 source ports, because considering
 *  extreme use case - live snapshot, in which there are 2 streaming inputs
 *  (with one buffer) goes into one port and 1 streaming input
 *  goes into another port per session.
 *
 *  These ports should map to internal CPP/VPE/C2D sub-modules'
 *  correspoinding ports.
 *
 *  Return TRUE on success.
 **/
static boolean pproc_module_create_default_ports(mct_module_t *module)
{
  boolean     rc = TRUE;
  mct_port_t *port;

  CDBG("%s:%d] E\n", __func__, __LINE__);
  port = pproc_port_init("pproc_sink_1");
  if (port) {
    if (mct_module_add_port(module, port) == FALSE) {
      CDBG_ERROR("%s:%d] sink_1 add failed\n", __func__, __LINE__);
      goto create_error;
    }
  } else {
    CDBG_ERROR("%s:%d] sink_1 init failed\n", __func__, __LINE__);
    goto create_error;
  }

  port = pproc_port_init("pproc_sink_2");
  if (port) {
    if (mct_module_add_port(module, port) == FALSE) {
      CDBG_ERROR("%s:%d] sink_2 add failed\n", __func__, __LINE__);
      goto create_error;
    }
  } else {
    CDBG_ERROR("%s:%d] sink_2 init failed\n", __func__, __LINE__);
    goto create_error;
  }

  port = pproc_port_init("pproc_sink_3");
  if (port) {
    if (mct_module_add_port(module, port) == FALSE) {
      CDBG_ERROR("%s:%d] sink_3 add failed\n", __func__, __LINE__);
      goto create_error;
    }
  } else {
    CDBG_ERROR("%s:%d] sink_3 init failed\n", __func__, __LINE__);
    goto create_error;
  }

  port = pproc_port_init("pproc_sink_4");
  if (port) {
    if (mct_module_add_port(module, port) == FALSE) {
      CDBG_ERROR("%s:%d] sink_4 add failed\n", __func__, __LINE__);
      goto create_error;
    }
  } else {
    CDBG_ERROR("%s:%d] sink_4 init failed\n", __func__, __LINE__);
    goto create_error;
  }

  port = pproc_port_init("pproc_sink_5");
  if (port) {
    if (mct_module_add_port(module, port) == FALSE) {
      CDBG_ERROR("%s:%d] sink_5 add failed\n", __func__, __LINE__);
      goto create_error;
    }
  } else {
    CDBG_ERROR("%s:%d] sink_5 init failed\n", __func__, __LINE__);
    goto create_error;
  }

  port = pproc_port_init("pproc_sink_6");
  if (port) {
    if (mct_module_add_port(module, port) == FALSE) {
      CDBG_ERROR("%s:%d] sink_6 add failed\n", __func__, __LINE__);
      goto create_error;
    }
  } else {
    CDBG_ERROR("%s:%d] sink_6 init failed\n", __func__, __LINE__);
    goto create_error;
  }

  port = pproc_port_init("pproc_sink_7");
  if (port) {
    if (mct_module_add_port(module, port) == FALSE) {
      CDBG_ERROR("%s:%d] sink_7 add failed\n", __func__, __LINE__);
      goto create_error;
    }
  } else {
    CDBG_ERROR("%s:%d] sink_7 init failed\n", __func__, __LINE__);
    goto create_error;
  }

  port = pproc_port_init("pproc_sink_8");
  if (port) {
    if (mct_module_add_port(module, port) == FALSE) {
      CDBG_ERROR("%s:%d] sink_8 add failed\n", __func__, __LINE__);
      goto create_error;
    }
  } else {
    CDBG_ERROR("%s:%d] sink_8 init failed\n", __func__, __LINE__);
    goto create_error;
  }

  port = pproc_port_init("pproc_source_1");
  if (port) {
    if (mct_module_add_port(module, port) == FALSE) {
      CDBG_ERROR("%s:%d] source_1 add failed\n", __func__, __LINE__);
      goto create_error;
    }
  } else {
    CDBG_ERROR("%s:%d] source_1 init failed\n", __func__, __LINE__);
    goto create_error;
  }

  port = pproc_port_init("pproc_source_2");
  if (port) {
    if (mct_module_add_port(module, port) == FALSE) {
      CDBG_ERROR("%s:%d] source_2 add failed\n", __func__, __LINE__);
      goto create_error;
    }
  } else {
    CDBG_ERROR("%s:%d] source_2 init failed\n", __func__, __LINE__);
    goto create_error;
  }

  port = pproc_port_init("pproc_source_3");
  if (port) {
    if (mct_module_add_port(module, port) == FALSE) {
      CDBG_ERROR("%s:%d] source_3 add failed\n", __func__, __LINE__);
      goto create_error;
    }
  } else {
    CDBG_ERROR("%s:%d] source_3 init failed\n", __func__, __LINE__);
    goto create_error;
  }

  port = pproc_port_init("pproc_source_4");
  if (port) {
    if (mct_module_add_port(module, port) == FALSE) {
      CDBG_ERROR("%s:%d] source_4 add failed\n", __func__, __LINE__);
      goto create_error;
    }
  } else {
    CDBG_ERROR("%s:%d] source_4 init failed\n", __func__, __LINE__);
    goto create_error;
  }

  port = pproc_port_init("pproc_source_5");
  if (port) {
    if (mct_module_add_port(module, port) == FALSE) {
      CDBG_ERROR("%s:%d] source_5 add failed\n", __func__, __LINE__);
      goto create_error;
    }
  } else {
    CDBG_ERROR("%s:%d] source_5 init failed\n", __func__, __LINE__);
    goto create_error;
  }

  port = pproc_port_init("pproc_source_6");
  if (port) {
    if (mct_module_add_port(module, port) == FALSE) {
      CDBG_ERROR("%s:%d] source_6 add failed\n", __func__, __LINE__);
      goto create_error;
    }
  } else {
    CDBG_ERROR("%s:%d] source_6 init failed\n", __func__, __LINE__);
    goto create_error;
  }

  port = pproc_port_init("pproc_source_7");
  if (port) {
    if (mct_module_add_port(module, port) == FALSE) {
      CDBG_ERROR("%s:%d] source_7 add failed\n", __func__, __LINE__);
      goto create_error;
    }
  } else {
    CDBG_ERROR("%s:%d] source_7 init failed\n", __func__, __LINE__);
    goto create_error;
  }

  port = pproc_port_init("pproc_source_8");
  if (port) {
    if (mct_module_add_port(module, port) == FALSE) {
      CDBG_ERROR("%s:%d] source_8 add failed\n", __func__, __LINE__);
      goto create_error;
    }
  } else {
    CDBG_ERROR("%s:%d] source_8 init failed\n", __func__, __LINE__);
    goto create_error;
  }

  CDBG("%s:%d] X\n", __func__, __LINE__);
  return TRUE;

create_error:
  mct_list_free_all(MCT_MODULE_SRCPORTS(module), pproc_module_free_port);
  mct_list_free_all(MCT_MODULE_SINKPORTS(module),pproc_module_free_port);
  CDBG("%s:%d] X\n", __func__, __LINE__);
  return FALSE;
}

/** pproc_module_init:
 *    @name: name of this pproc interface module("pproc").
 *
 *  pproc interface module initializtion entry point, it only
 *  creates pproc module and initialize its sub-modules(cpp/vpe,
 *  c2d, s/w scaling modules etc.). pproc should also initialize
 *  its sink and source ports which map to its sub-modules ports.
 *
 *  Return: pproc module object if success
 **/
mct_module_t* pproc_module_init(const char *name)
{
  int                     i;
  mct_module_t           *pproc;
  mct_port_t             *port;
  pproc_module_private_t *mod_private;

  CDBG("%s:%d] E\n", __func__, __LINE__);
  if (strcmp(name, "pproc")) {
    CDBG_ERROR("%s:%d] invalid module name\n", __func__, __LINE__);
    return NULL;
  }

  pproc = mct_module_create("pproc");
  if (!pproc) {
    CDBG_ERROR("%s:%d] error module create failed\n", __func__, __LINE__);
    return NULL;
  }
  mod_private = malloc(sizeof(pproc_module_private_t));
  if (mod_private == NULL) {
    CDBG_ERROR("%s:%d] error because private data alloc failed\n", __func__,
      __LINE__);
    goto private_error;
  }
  memset(mod_private, 0 ,sizeof(pproc_module_private_t));
  /* TODO: Add version or caps based information to build topology */
#ifdef CAMERA_FEATURE_CPP
  mod_private->cpp_module = cpp_module_init("cpp");
#endif
#ifdef CAMERA_FEATURE_VPE
  mod_private->vpe_module = vpe_module_init("vpe");
#endif
  mod_private->c2d_module = c2d_module_init("c2d");
#if CAMERA_FEATURE_CAC
  mod_private->cac_module = module_cac_init("cac");
  if (NULL!=mod_private->cac_module) {
    /* populate the parent pointer */
    module_cac_set_parent(mod_private->cac_module, pproc);
  }
  else{
    CDBG_ERROR("%s:%d] cac module create failed\n", __func__, __LINE__);
  }
#else
#ifdef CAMERA_FEATURE_WNR_SW
  mod_private->wnr_module = module_wnr_init("wnr");
  if (NULL != mod_private->wnr_module) {
    /* populate the parent pointer */
    module_wnr_set_parent(mod_private->wnr_module, pproc);
  }
  else {
    CDBG_ERROR("%s:%d] wnr module create failed\n", __func__, __LINE__);
  }

  mod_private->hdr_module = module_hdr_init("hdr");
  if (NULL == mod_private->hdr_module) {
    CDBG_ERROR("%s:%d] hdr module create failed\n", __func__, __LINE__);
  }
#endif
#endif

  mod_private->llvd_module = module_llvd_init("llvd");
  if (NULL != mod_private->llvd_module) {
    /* populate the parent pointer */
    module_llvd_set_parent(mod_private->llvd_module, pproc);
  } else{
    CDBG_ERROR("%s:%d] llvd module create failed\n", __func__, __LINE__);
  }

  MCT_OBJECT_PRIVATE(pproc) = mod_private;

  /* create pproc's stream/capture sinkports and source ports */
  if (pproc_module_create_default_ports(pproc) == FALSE) {
    CDBG_ERROR("%s:%d] error in default port creation\n", __func__,
      __LINE__);
    goto port_create_error;
  }

  mct_module_set_set_mod_func(pproc, pproc_module_set_mod);
  mct_module_set_query_mod_func(pproc, pproc_module_query_mod);
  mct_module_set_start_session_func(pproc, pproc_module_start_session);
  mct_module_set_stop_session_func(pproc, pproc_module_stop_session);
  mct_module_set_request_new_port_func(pproc, pproc_module_request_new_port);
  mct_module_set_process_event_func(pproc, pproc_module_process_event);

  CDBG("%s:%d] X\n", __func__, __LINE__);
  return pproc;

port_create_error:
  pproc_module_deinit(pproc);
  mod_private = NULL;
  pproc = NULL;
sub_modue_error:
  if(mod_private)
    free(mod_private);
private_error:
  if(pproc)
    mct_module_destroy(pproc);
  CDBG("%s:%d] X\n", __func__, __LINE__);
  return NULL;
}
