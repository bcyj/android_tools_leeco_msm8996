/* sensor_util.c
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "sensor_util.h"

/**  sensor_util_unpack_identity:
 *    unpacks the 32-bit identity in session_id and stream_id
 *
 *  Arguments:
 *    @identity: input param
 *    @session_id: output param
 *    @stream_id: output param
 *
 *  Return: TRUE on success
 *          FALSE on failure **/

boolean sensor_util_unpack_identity(unsigned int identity,
  uint32_t *session_id, uint32_t *stream_id)
{
  if (!session_id || !stream_id) {
    SERR("failed session_id %p stream_id %p",
      session_id, stream_id);
    return FALSE;
  }
  *stream_id = (identity & 0x0000FFFF);
  *session_id = ((identity & 0xFFFF0000) >> 16);
  SLOW("session_id %d stream id %d", *session_id, *stream_id);
  return TRUE;
}

/** sensor_util_find_bundle: sensor find bundle
 *
 *  @data1: sensor bundle info
 *  @data2: session id
 *
 *  Return: 1 if session id matches with sensor video node id
 *
 *  This function matches session id with sensor video node
 *  id from sensor info passed to it and returns decision
 **/

boolean sensor_util_find_bundle(void *data1, void *data2)
{
  module_sensor_bundle_info_t *s_bundle = (module_sensor_bundle_info_t *)data1;
  int32_t *session_id = (int32_t *)data2;

  if (!s_bundle || !session_id) {
    SERR("failed data1 %p data2 %p", s_bundle,
      session_id);
    return FALSE;
  }
  if (s_bundle->sensor_info->session_id == *session_id)
    return TRUE;

  return FALSE;
}

/** sensor_util_find_identity:
 *    @data1: sensor bundle info
 *    @data2: session id
 *
 *  Return: 1 if child identity matches with current identity
 *
 *  This function matches session id with sensor video node
 *  id from sensor info passed to it and returns decision
 **/

boolean sensor_util_find_identity(void *data1, void *data2)
{
  uint32_t *child_identity = (uint32_t *)data1;
  uint32_t *cur_identity = (uint32_t *)data2;

  if (!child_identity || !cur_identity) {
    SERR("failed data1 %p data2 %p", data1,
      data2);
    return FALSE;
  }
  if (*child_identity == *cur_identity)
    return TRUE;

  return FALSE;
}

static boolean sensor_util_src_port_identity_find_func(void* data,
  void* user_data)
{
  uint32_t* p_identity = (uint32_t*)user_data;
  mct_port_t* port = (mct_port_t *)data;
  if (!p_identity || !port) {
    SERR("failed");
    return FALSE;
  }
  SLOW("port=%p; required identity=0x%x", port, *p_identity);
  mct_list_t* s_list = mct_list_find_custom(MCT_PORT_CHILDREN(port), p_identity,
     sensor_util_find_identity);
  if (s_list == NULL) {
    SLOW("Cannot find port with identity=0x%x", *p_identity);
    return FALSE;
  }
  return TRUE;
}


/*     --- module_sensor_find_src_port_with_identity ---
 *  Description:
 *    Find the src port in module which supports identity
 *  Arguments:
 *    @module:    sensor module pointer
 *    #identity:  32-bit identity
 *  Returns:
 *    pointer to the corresponding port on success
 *    NULL, on failure or if port is not found
 */
mct_port_t *sensor_util_find_src_port_with_identity(mct_module_t *module,
  uint32_t identity)
{
  mct_port_t* port = NULL;
  if (!module) {
    SERR("failed");
    return NULL;
  }
  mct_list_t* s_list = mct_list_find_custom(MCT_MODULE_SRCPORTS(module),
    &identity, sensor_util_src_port_identity_find_func);
  port = (s_list != NULL) ? s_list->data : NULL;
  return port;
}

static boolean sensor_util_stream_list_print_traverse_func(void* data,
  void* user_data)
{
  if (!data) {
    SERR("failed");
    return FALSE;
  }
  module_sensor_port_stream_info_t* stream_info=
    (module_sensor_port_stream_info_t*) data;
  SLOW("stream:bundle_id=%d", stream_info->bundle_id);
  SLOW("stream:identity=0x%x", stream_info->identity);
  SLOW("stream:dim=%dx%d", stream_info->width, stream_info->height);
  SLOW("---------------------------------------------------------------");
  return TRUE;
}

static boolean sensor_util_bundle_list_print_traverse_func(void* data,
  void* user_data)
{
  if (!data) {
    SERR("failed");
    return FALSE;
  }
  module_sensor_port_bundle_info_t* bundle_info=
    (module_sensor_port_bundle_info_t*) data;
  SLOW("bundle:id=%d, ", bundle_info->bundle_config.bundle_id);
  SLOW("bundle:num_streams=%d",
    bundle_info->bundle_config.num_of_streams);
  int i;
  for (i=0; i< bundle_info->bundle_config.num_of_streams; i++) {
    SLOW("bundle:steram[%d]=%d", i,
      bundle_info->bundle_config.stream_ids[i]);
  }
  SLOW("---------------------------------------------------------------");
  return TRUE;
}

void sensor_util_dump_bundle_and_stream_lists(mct_port_t* port,
  const char *func, int line)
{
  if (!port) {
    SERR("failed");
    return;
  }
  module_sensor_port_data_t *port_data =
     (module_sensor_port_data_t*) port->port_private;

  SLOW("DUMP at function %s() at line %d", func, line);
  SLOW("port=%p, bundle_list=%p, stream_list=%p",
    port, port_data->bundle_list, port_data->stream_list);

  SLOW("---------------------------------------------------------------");
  SLOW("                       Bundle List");
  SLOW("---------------------------------------------------------------");
  mct_list_traverse(port_data->bundle_list,
    sensor_util_bundle_list_print_traverse_func, NULL);
  SLOW("                       Stream List");
  SLOW("---------------------------------------------------------------");
  mct_list_traverse(port_data->stream_list,
    sensor_util_stream_list_print_traverse_func, NULL);
}

static boolean sensor_util_fill_bundle_params(void* data, void* user_data)
{
  module_sensor_port_stream_info_t* stream_info =
    (module_sensor_port_stream_info_t*) data;
  module_sensor_port_bundle_info_t* bundle_info =
     (module_sensor_port_bundle_info_t*) user_data;

  if (!stream_info || !bundle_info) {
    SERR("failed");
    return FALSE;
  }
  uint32_t session_id, stream_id;
  sensor_util_unpack_identity(stream_info->identity, &session_id, &stream_id);
  int i;
  for (i = 0; i < bundle_info->bundle_config.num_of_streams; i++) {
    if (stream_id == (uint32_t)bundle_info->bundle_config.stream_ids[i]) {
      stream_info->bundle_id = bundle_info->bundle_config.bundle_id;
      SLOW("stream=%d, bundle=%d", stream_id,
        bundle_info->bundle_config.bundle_id);
      break;
    }
  }
  return TRUE;
}

static boolean sensor_util_find_exact_bundle(void* list_data, void* user_data)
{
  boolean ret_val = FALSE;
  module_sensor_port_bundle_info_t *bundle_info = list_data;
  cam_bundle_config_t* bundle = user_data;

  if (bundle_info && bundle) {
    if (!memcmp(&(bundle_info->bundle_config), bundle,
      sizeof(cam_bundle_config_t))) {
        ret_val = TRUE;
    }
  } else
    SERR("Null pointer detected in %s\n", __func__);

  return ret_val;
}

int32_t sensor_util_assign_bundle_id(mct_module_t* module,
  uint32_t identity, cam_bundle_config_t* bundle)
{
  mct_port_t *s_port = NULL;
  mct_list_t* list;
  uint32_t    i = 0;

  SHIGH("bundle_id=%d, num_streams=%d", bundle->bundle_id,
    bundle->num_of_streams);
  /* Print all bundle stream id's */
  for (i = 0; i < bundle->num_of_streams; i++) {
    SHIGH("bundle stream id %d", bundle->stream_ids[i]);
  }

  /* find a src port which has the given identity */
  s_port = sensor_util_find_src_port_with_identity(module, identity);
  if (!s_port) {
    SERR("failed: cannot find port with identity=0x%x", identity);
    return -EFAULT;
  }

  /* add bundle info in port private data,
     find the correspoding streams in the port_data,
     assign bundle_id to these streams */
  module_sensor_port_data_t* port_data;
  port_data = s_port->port_private;
  if (!port_data) {
    SERR("failed");
    return -EFAULT;
  }

  list = mct_list_find_custom(port_data->bundle_list, bundle,
    sensor_util_find_exact_bundle);

  if (!list) {
    module_sensor_port_bundle_info_t *bundle_info = NULL;
    bundle_info = (module_sensor_port_bundle_info_t *)malloc(
      sizeof(module_sensor_port_bundle_info_t));
    if (!bundle_info) {
      SERR("bundle_infor allocation failed");
      return -EFAULT;
    }
    memset(bundle_info, 0, sizeof(module_sensor_port_bundle_info_t));
    memcpy(&(bundle_info->bundle_config), bundle, sizeof(cam_bundle_config_t));
    bundle_info->stream_on_count = 0;
    /* assign the bundle id to streams and get max dimensions for a bundle */
    mct_list_traverse(port_data->stream_list, sensor_util_fill_bundle_params,
      bundle_info);
    SLOW("assigned bundle_id to streams");
    port_data->bundle_list = mct_list_append(port_data->bundle_list, bundle_info,
      NULL, NULL);
    SLOW("added bundle to list");
  }

  sensor_util_dump_bundle_and_stream_lists(s_port, __func__, __LINE__);
  return 0;
}

static boolean sensor_util_traverse_port(void *data, void *user_data)
{
  boolean ret = TRUE;
  mct_port_t *s_port = (mct_port_t *)data;
  mct_event_t *event = (mct_event_t *)user_data;
  mct_list_t *s_list = NULL;

  if (!s_port || !event) {
    SERR("failed port %p event %p", s_port, event);
    return FALSE;
  }

  s_list = mct_list_find_custom(MCT_PORT_CHILDREN(s_port), &event->identity,
    sensor_util_find_identity);
  if (!s_list) {
    return TRUE;
  }

  SLOW("s_port=%p event_func=%p", s_port, s_port->event_func);
  ret = s_port->event_func(s_port, event);
  return ret;
}

boolean sensor_util_post_event_on_src_port(mct_module_t *module,
  mct_event_t *event)
{
  boolean ret = TRUE;
  mct_port_t *s_port = NULL;

  if (!module || !event) {
    SERR("failed module %p event %p", module, event);
    return FALSE;
  }

  ret = mct_list_traverse(MCT_MODULE_SRCPORTS(module),
    sensor_util_traverse_port, event);
  return ret;
}

static boolean sensor_util_find_stream(void *data, void *user_data)
{
  module_sensor_port_stream_info_t *stream_info =
    (module_sensor_port_stream_info_t *)data;
  uint32_t *identity = (uint32_t *)user_data;
  if (!data || !user_data) {
    SERR("failed");
    return FALSE;
  }
  SLOW("id1=0x%x, id2=0x%x", stream_info->identity, *identity);
  if (stream_info->identity == *identity) {
    return TRUE;
  }
  return FALSE;
}

int32_t sensor_util_find_bundle_id_for_stream(mct_port_t* port,
  uint32_t identity)
{
  mct_list_t                       *s_list = NULL;
  module_sensor_port_stream_info_t *stream_info = NULL;
  module_sensor_port_data_t        *port_data = NULL;
  if (!port) {
    SERR("failed");
    return -1;
  }
  port_data = (module_sensor_port_data_t *)port->port_private;
  if (!port_data) {
    SERR("failed");
    return -1;
  }
  s_list = mct_list_find_custom(port_data->stream_list, &identity,
    sensor_util_find_stream);
  if (!s_list) {
    SLOW("bundle id not found");
    return -1;
  }
  stream_info = (module_sensor_port_stream_info_t *)s_list->data;
  if (!stream_info) {
    SERR("stream info NULL");
    return -1;
  }
  return stream_info->bundle_id;
}

typedef struct {
  cam_stream_type_t stream_type;
  uint16_t session_id;
  uint16_t stream_id;
} module_sensor_util_stream_info_t;

static boolean sensor_util_find_stream_by_type_and_session(void *data,
  void *user_data)
{
  module_sensor_port_stream_info_t *stream_info =
    (module_sensor_port_stream_info_t *)data;
  module_sensor_util_stream_info_t* search_data =
    (module_sensor_util_stream_info_t *)user_data;
  if (!data || !user_data) {
    SERR("failed");
    return FALSE;
  }
  SLOW("type1=0x%x, type2=0x%x", stream_info->stream_type,
    search_data->stream_type);
  SLOW("identity1=0x%x, session2=0x%x", stream_info->identity,
    search_data->session_id);
  if ((search_data->stream_type == stream_info->stream_type) &&
      (search_data->session_id == SENSOR_SESSIONID(stream_info->identity))) {
    search_data->stream_id = SENSOR_STREAMID(stream_info->identity);
    return TRUE;
  }
  return FALSE;
}

static boolean sensor_util_find_port_by_stream_type_and_session(
  void *data, void *user_data)
{
  mct_port_t                       *port = data;
  module_sensor_util_stream_info_t *search_data = user_data;
  mct_list_t                       *s_list = NULL;
  module_sensor_port_stream_info_t *stream_info = NULL;
  module_sensor_port_data_t        *port_data = NULL;

  if (!port) {
    SERR("failed");
    return FALSE;
  }

  port_data = (module_sensor_port_data_t *)port->port_private;
  if (!port_data) {
    SERR("failed");
    return FALSE;
  }

  s_list = mct_list_find_custom(port_data->stream_list, search_data,
    sensor_util_find_stream_by_type_and_session);
  if (!s_list) {
    SLOW("bundle id not found");
    return FALSE;
  }
  stream_info = (module_sensor_port_stream_info_t *)s_list->data;
  if (!stream_info) {
    SERR("stream info NULL");
    return FALSE;
  }
  return TRUE;
}

uint32_t sensor_util_find_stream_identity_by_stream_type_and_session(
  mct_module_t *module, cam_stream_type_t stream_type, uint16_t session_id)
{
  mct_list_t *s_list = NULL;
  module_sensor_util_stream_info_t  search_data;

  if (!module || !session_id) {
    SERR("failed");
    return 0;
  }

  search_data.session_id = session_id;
  search_data.stream_type = stream_type;
  s_list = mct_list_find_custom(MCT_MODULE_SRCPORTS(module), &search_data,
    sensor_util_find_port_by_stream_type_and_session);

  if (!s_list || !s_list->data) {
    SLOW("port not found");
    return 0;
  }
  return SENSOR_IDENTITY(search_data.session_id, search_data.stream_id);
}

static boolean sensor_util_bundle_id_find_func(void* data, void* user_data)
{
  uint32_t *bundle_id = (uint32_t *)user_data;
  module_sensor_port_bundle_info_t* bundle_info =
    (module_sensor_port_bundle_info_t*)data;
  if (!data || !user_data) {
    SERR("failed");
    return FALSE;
  }
  if (bundle_info->bundle_config.bundle_id == *bundle_id)
    return TRUE;
  else
    return FALSE;
}

module_sensor_port_bundle_info_t* sensor_util_find_bundle_by_id(
  mct_port_t* port, int32_t bundle_id)
{
  module_sensor_port_bundle_info_t *bundle_info = NULL;
  module_sensor_port_data_t        *port_data =
    (module_sensor_port_data_t *)port->port_private;
  mct_list_t *s_list  = mct_list_find_custom(port_data->bundle_list,
    &bundle_id, sensor_util_bundle_id_find_func);
  if (s_list != NULL) {
    bundle_info = s_list->data;
  }
  return bundle_info;
}

void sensor_util_remove_list_entries_by_identity(mct_port_t *port,
  uint32_t identity)
{
  module_sensor_port_data_t        *port_data = NULL;
  module_sensor_port_stream_info_t *stream_info = NULL;
  module_sensor_port_bundle_info_t *bundle_info = NULL;
  mct_list_t                       *tmp = NULL;
  int32_t                           bundle_id = -1;
  SLOW("port=%p, identity=0x%x", port, identity);
  if (!port) {
    SERR("failed");
    return;
  }
  port_data = port->port_private;
  if (!port_data) {
    return;
  }
  tmp = mct_list_find_custom(port_data->stream_list, &identity,
    sensor_util_find_stream);
  if (!tmp) {
    SLOW("identity=0x%x, not found in stream list", identity);
  } else {
    stream_info = tmp->data;
    bundle_id = stream_info->bundle_id;
    port_data->stream_list = mct_list_remove(port_data->stream_list,
      stream_info);
    free(stream_info);
    if (!port_data->stream_list)
      MCT_PORT_PEER(port) = NULL;
  }
  /* remove entry in bundle list, if stream is part of bundle */
  if (bundle_id != -1) {
    bundle_info = sensor_util_find_bundle_by_id(port, bundle_id);
    if (!bundle_info) {
      SLOW("bundle_id=%d, not found in bundle list, \
             might be removed already", bundle_id);
    } else {
      port_data->bundle_list = mct_list_remove(port_data->bundle_list,
        bundle_info);
      free(bundle_info);
    }
  }
}

boolean sensor_util_get_sbundle(mct_module_t *s_module,
  uint32_t identity, sensor_bundle_info_t *bundle_info)
{
  boolean                           ret = FALSE;
  uint32_t                          session_id = 0, stream_id = 0;
  module_sensor_bundle_info_t      *s_bundle = NULL;
  module_sensor_ctrl_t             *module_ctrl = NULL;
  mct_list_t                       *s_list = NULL;

  module_ctrl = (module_sensor_ctrl_t *)s_module->module_private;
  if (!module_ctrl || !bundle_info) {
    SERR("failed module ctrl %p bundle info %p", module_ctrl, bundle_info);
    return FALSE;
  }

  ret = sensor_util_unpack_identity(identity, &session_id, &stream_id);
  if (ret == FALSE) {
    SERR("failed");
    return FALSE;
  }

  s_list = mct_list_find_custom(module_ctrl->sensor_bundle, &session_id,
    sensor_util_find_bundle);
  if (!s_list) {
    SERR("failed");
    return FALSE;
  }

  s_bundle = (module_sensor_bundle_info_t *)s_list->data;
  if (!s_bundle) {
    SERR("failed");
    return FALSE;
  }

  /* Fill bundle info */
  bundle_info->s_bundle = s_bundle;
  bundle_info->session_id = session_id;
  bundle_info->stream_id = stream_id;

  return TRUE;
}

int32_t sensor_util_set_digital_gain_to_isp(mct_module_t* module,
  module_sensor_bundle_info_t* s_bundle, uint32_t identity)
{
  int32_t rc;
  boolean ret = TRUE;
  float digital_gain = 0.0f;
  mct_event_t new_event;
  if (!module || !s_bundle) {
    SERR("failed");
    return FALSE;
  }
  /* get current digital gain from sensor */
  /* TODO: add functionality to check if digital_gain update is required */
  module_sensor_params_t *module_sensor_params =
     s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private, SENSOR_GET_DIGITAL_GAIN,
    &digital_gain);
  if (rc < 0) {
      SERR("failed");
      return FALSE;
  }
  new_event.type = MCT_EVENT_MODULE_EVENT;
  new_event.identity = identity;
  new_event.direction = MCT_EVENT_DOWNSTREAM;
  new_event.u.module_event.type = MCT_EVENT_MODULE_SET_DIGITAL_GAIN;
  new_event.u.module_event.module_event_data = (void *)&digital_gain;
  ret = sensor_util_post_event_on_src_port(module, &new_event);
  if (ret == FALSE) {
    SERR("failed");
  }
  return ret;
}

int32_t sensor_util_set_frame_skip_to_isp(mct_module_t* module,
  uint32_t identity, enum msm_vfe_frame_skip_pattern frame_skip_pattern)
{
  boolean ret = TRUE;
  mct_event_t new_event;
  mct_event_control_parm_t new_param;

  if (!module || !identity) {
    SERR("failed");
    return FALSE;
  }

  new_param.type = CAM_INTF_PARM_FRAMESKIP;
  new_param.parm_data = &frame_skip_pattern;

  new_event.type = MCT_EVENT_CONTROL_CMD;
  new_event.identity = identity;
  new_event.direction = MCT_EVENT_DOWNSTREAM;
  new_event.u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  new_event.u.ctrl_event.control_event_data = (void *)&new_param;

  ret = sensor_util_post_event_on_src_port(module, &new_event);

  if (ret == FALSE) {
    SERR("failed");
  }

  return ret;
}

int32_t sensor_util_is_previous_frame_sent(mct_module_t* module,
  mct_event_t *event)
{
  boolean ret = FALSE;
  uint32_t i;
  mct_bus_msg_isp_sof_t *sof_event;
  mct_event_module_t *event_module;

  if (!module || !event || !event->u.module_event.module_event_data) {
    SERR("failed null pointer");
    return FALSE;
  }

  event_module = &event->u.module_event;

  if ((MCT_EVENT_UPSTREAM != event->direction) ||
    (MCT_EVENT_MODULE_EVENT != event->type) ||
    (MCT_EVENT_MODULE_SOF_NOTIFY != event_module->type)) {
    SERR("failed worng event");
    return FALSE;
  }

  sof_event = (mct_bus_msg_isp_sof_t*) (event_module->module_event_data);

  for (i = 0; i < ARRAY_SIZE(sof_event->prev_sent_streamids); i++) {
    if (0 == sof_event->prev_sent_streamids[i]) {
      // No more valid previous stream id are present
      break;
    }
    if (SENSOR_STREAMID(event->identity) == sof_event->prev_sent_streamids[i]) {
      // Frame was just sent for current stream
      ret = TRUE;
      break;
    }
  }

  return ret;
}

boolean sensor_util_check_format(sensor_src_port_cap_t *caps,
  mct_stream_info_t *stream_info)
{
  boolean ret = FALSE;
  int32_t i = 0;

  /* Validate input parameters */
  if (!caps || !stream_info) {
    SERR("failed: caps %p stream_info %p", caps, stream_info);
    return FALSE;
  }

  SLOW("stream fmt %d", stream_info->fmt);
  for (i = 0; i < caps->num_cid_ch; i++) {
    SLOW("caps fmt %d", caps->sensor_cid_ch[i].fmt);
    /* Check whether incoming format request is compabile to current
       port's supported format */
    switch (stream_info->fmt) {
    case CAM_FORMAT_JPEG:
    case CAM_FORMAT_JPEG_RAW_8BIT:
      if ((caps->sensor_cid_ch[i].fmt == CAM_FORMAT_JPEG) ||
          (caps->sensor_cid_ch[i].fmt == CAM_FORMAT_JPEG_RAW_8BIT)) {
        return TRUE;
      }
      break;
    case CAM_FORMAT_YUV_420_NV12:
    case CAM_FORMAT_YUV_420_NV21:
    case CAM_FORMAT_YUV_420_NV21_ADRENO:
    case CAM_FORMAT_YUV_420_YV12:
    case CAM_FORMAT_YUV_422_NV16:
    case CAM_FORMAT_YUV_422_NV61:
    case CAM_FORMAT_YUV_420_NV12_VENUS:
    case CAM_FORMAT_YUV_RAW_8BIT_YUYV:
    case CAM_FORMAT_YUV_RAW_8BIT_YVYU:
    case CAM_FORMAT_YUV_RAW_8BIT_UYVY:
    case CAM_FORMAT_YUV_RAW_8BIT_VYUY:
      switch (caps->sensor_cid_ch[i].fmt) {
      case CAM_FORMAT_YUV_420_NV12:
      case CAM_FORMAT_YUV_420_NV21:
      case CAM_FORMAT_YUV_420_NV21_ADRENO:
      case CAM_FORMAT_YUV_420_YV12:
      case CAM_FORMAT_YUV_422_NV16:
      case CAM_FORMAT_YUV_422_NV61:
      case CAM_FORMAT_YUV_420_NV12_VENUS:
      case CAM_FORMAT_YUV_RAW_8BIT_YUYV:
      case CAM_FORMAT_YUV_RAW_8BIT_YVYU:
      case CAM_FORMAT_YUV_RAW_8BIT_UYVY:
      case CAM_FORMAT_YUV_RAW_8BIT_VYUY:
      case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG:
      case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG:
      case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB:
      case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR:
      case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG:
      case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG:
      case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB:
      case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR:
      case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG:
      case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG:
      case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB:
      case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GBRG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GRBG:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_RGGB:
      case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_BGGR:
        return TRUE;
      default:
        ret = FALSE;
        break;
      }
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_8BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN8_8BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_8BPP_BGGR:
      switch (caps->sensor_cid_ch[i].fmt) {
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
      case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
        return TRUE;
      default:
        ret = FALSE;
        break;
      }
      break;
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_10BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_10BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_10BPP_BGGR:
      switch (caps->sensor_cid_ch[i].fmt) {
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
      case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
        return TRUE;
      default:
        ret = FALSE;
        break;
      }
      break;
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GBRG:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_GRBG:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_RGGB:
    case CAM_FORMAT_BAYER_QCOM_RAW_12BPP_BGGR:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_QCOM_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_MIPI_12BPP_BGGR:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GBRG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_GRBG:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_RGGB:
    case CAM_FORMAT_BAYER_IDEAL_RAW_PLAIN16_12BPP_BGGR:
      switch (caps->sensor_cid_ch[i].fmt) {
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
      case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
        return TRUE;
      default:
        ret = FALSE;
        break;
      }
      break;
    case CAM_FORMAT_META_RAW_8BIT:
    case CAM_FORMAT_META_RAW_10BIT:
      if (caps->sensor_cid_ch[i].fmt == CAM_FORMAT_META_RAW_8BIT ||
        caps->sensor_cid_ch[i].fmt == CAM_FORMAT_META_RAW_10BIT) {
        return TRUE;
      }
      break;
    default:
      ret = FALSE;
      break;
    }
  }
  return ret;
}

/** sensor_util_post_bus_sensor_params: post sensor params to
 *  bus
 *
 *  @data1: sensor bundle info
 *  @data2: session id
 *
 *  Return: TRUE for sucess and FALSE for failure
 *
 *  This function matches session id with sensor video node
 *  id from sensor info passed to it and returns decision
 **/

boolean sensor_util_post_bus_sensor_params(mct_module_t *s_module,
  module_sensor_bundle_info_t *s_bundle, uint32_t identity)
{
  boolean                ret = TRUE;
  mct_bus_msg_t          bus_msg;

  /* Validate input parameters */
  if (!s_module || !s_bundle || !s_bundle->sensor_info) {
    SERR("failed: s_module %p s_bundle %p", s_module, s_bundle);
    return FALSE;
  }

  SLOW("aperture %f", s_bundle->sensor_params.aperture_value);

  /* Fill bus msg params */
  bus_msg.sessionid = s_bundle->sensor_info->session_id;
  bus_msg.type = MCT_BUS_MSG_SENSOR_INFO;
  bus_msg.msg = &s_bundle->sensor_params;
  ret = mct_module_post_bus_msg(s_module, &bus_msg);
  if (ret == FALSE) {
    SERR("failed");
  }

  return ret;
}

boolean sensor_util_load_liveshot_chromatix(mct_module_t *module,
  mct_port_t *port, mct_event_t *event, module_sensor_bundle_info_t *s_bundle)
{
  boolean                     rc = TRUE;
  module_sensor_params_t      *module_sensor_params = NULL;
  module_sensor_params_t      *chromatix_module_params = NULL;
  modules_liveshot_Chromatix_t module_chromatix;
  sensor_chromatix_params_t   chromatix_params;
  sensor_get_t                sensor_get;
  mct_event_t                 new_event;

  module_sensor_params = s_bundle->module_sensor_params[SUB_MODULE_SENSOR];
  chromatix_module_params =
    s_bundle->module_sensor_params[SUB_MODULE_CHROMATIX];

  rc = module_sensor_params->func_tbl.process(
    module_sensor_params->sub_module_private,
    SENSOR_GET_CUR_CHROMATIX_NAME, &sensor_get);
  if (rc < 0) {
    SERR("failed");
    return FALSE;
  }

  SLOW("%s:%d] liveshot chromatix name %s", __func__, __LINE__,
    sensor_get.chromatix_name.liveshot_chromatix);
  if (!sensor_get.chromatix_name.liveshot_chromatix) {
    SERR("failed live snapshot chromatix %s",
      sensor_get.chromatix_name.liveshot_chromatix);
    return FALSE;
  }
  rc = chromatix_module_params->func_tbl.process(
    chromatix_module_params->sub_module_private,
    CHROMATIX_OPEN_LIVESHOT_LIBRARY, &sensor_get.chromatix_name);
  if (rc < 0) {
    SERR("failed");
    return FALSE;
  }
  rc = chromatix_module_params->func_tbl.process(
    chromatix_module_params->sub_module_private,
    CHROMATIX_GET_PTR, &chromatix_params);
  if (rc < 0) {
    SERR("failed");
    return FALSE;
  }
  if (!chromatix_params.liveshot_chromatix_ptr) {
    SERR("failed liveshot chromatix %p",
      chromatix_params.liveshot_chromatix_ptr);
    return FALSE;
  }

  module_chromatix.liveshot_chromatix_ptr =
    chromatix_params.liveshot_chromatix_ptr;
  /* Send chromatix pointer downstream */
  new_event.type = MCT_EVENT_MODULE_EVENT;
  new_event.identity = event->identity;
  new_event.direction = MCT_EVENT_DOWNSTREAM;
  new_event.u.module_event.type =
    MCT_EVENT_MODULE_SET_LIVESHOT_CHROMATIX_PTR;
  new_event.u.module_event.module_event_data =
    (void *)&module_chromatix;
  rc = sensor_util_post_event_on_src_port(module, &new_event);
  if (rc == FALSE) {
    SERR("failed");
    return FALSE;
  }
  return rc;
}

static boolean sensor_util_find_is_bundle_started(void *data1, void *data2)
{
  module_sensor_port_bundle_info_t *bundle =
    (module_sensor_port_bundle_info_t *)data1;

  if (!bundle) {
    return FALSE;
  }

  /* Find whether all streams in this bundle has started streaming */
  if (bundle->stream_on_count == bundle->bundle_config.num_of_streams) {
    return TRUE;
  }

  return FALSE;
}

boolean sensor_util_find_is_any_bundle_started(mct_port_t *port)
{
  module_sensor_port_data_t *port_data = NULL;
  mct_list_t                *blist = NULL;

  /* Validate input paramters */
  if (!port) {
    SERR("failed: invalid input params %p", port);
    return FALSE;
  }

  /* Extract port private */
  port_data = (module_sensor_port_data_t *)port->port_private;
  if (!port_data) {
    SERR("failed: port private %p", port_data);
    return FALSE;
  }

  /* Check whether there is any bundle for this session where all streams
     present in that bundle has already streamed ON */
  blist = mct_list_find_custom(port_data->bundle_list, NULL,
   sensor_util_find_is_bundle_started);
  if (!blist) {
    /* Either no bundle exist for this session or there is no bundle where all
       streams in that bundle has started streaming */
    return FALSE;
  }

  return TRUE;
}

/** sensor_util_post_downstream_event: post LED state message on
 *  bus
 *
 *  @s_module: mct module handle
 *  @identity: identity of event to be posted downstream
 *  @type: event type
 *  @data: data
 *
 *  Return: TRUE for success and FALSE on failure
 *
 *  This function creates module event and posts downstream
 **/

boolean sensor_util_post_downstream_event(mct_module_t *s_module,
  uint32_t identity, mct_event_module_type_t type, void *data)
{
  boolean ret = TRUE;
  mct_event_t event;

  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = type;
  event.u.module_event.module_event_data = data;
  ret = sensor_util_post_event_on_src_port(s_module, &event);
  if (ret == FALSE) {
     SERR("failed");
   }

  return ret;
}

/** sensor_util_post_led_state_msg: post LED state message on
 *  bus
 *
 *  @s_module: mct module handle
 *  @s_bundle: sensor bundle handle
 *
 *  Return: TRUE for success and FALSE on failure
 *
 *  This function creates LED bus message and posts on bus
 **/

boolean sensor_util_post_led_state_msg(mct_module_t *s_module,
  module_sensor_bundle_info_t *s_bundle, uint32_t identity)
{
  boolean                ret = TRUE;

  /* Validate input parameters */
  if (!s_module || !s_bundle) {
    SERR("failed: s_module %p s_bundle %p", s_module, s_bundle);
    return FALSE;
  }

  /* Post downstream event */
  ret = sensor_util_post_downstream_event(s_module, identity,
    MCT_EVENT_MODULE_SET_FLASH_MODE, &s_bundle->sensor_params.flash_mode);
  if (ret == FALSE) {
    SERR("failed");
  }

  return ret;
}
