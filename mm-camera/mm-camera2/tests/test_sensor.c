/*============================================================================

  Copyright (c) 2012 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "mct_module.h"
#include "module_sensor.h"
#include "camera_dbg.h"
#include "mct_stream.h"
#include "mct_pipeline.h"

#undef CDBG
#define CDBG ALOGE
#undef CDBG_ERROR
#define CDBG_ERROR ALOGE

int main(int argc, char * argv[])
{
  mct_module_t *s_module = NULL;
  mct_pipeline_cap_t query_buf;
  mct_event_t event;
  uint32_t i = 0;
  cam_stream_info_t stream_info;
  mct_port_t *s_port = NULL;
  uint32_t identity = 0;
  CDBG("%s:%d\n", __func__, __LINE__);
  /* Call sensor mct module init */
  s_module = module_sensor_init("sensor");
  if (!s_module) {
      CDBG_ERROR("%s:%d s_module cannot be initialized!!",__func__, __LINE__);
      return 0;
  }
  /* Call query capabilities */
  s_module->query_mod(s_module, &query_buf, 1);
  CDBG("%s:%d caps: mode %d position %d mount angle %d\n", __func__, __LINE__,
    query_buf.sensor_cap.modes_supported, query_buf.sensor_cap.position, query_buf.sensor_cap.sensor_mount_angle);
  CDBG("%s:%d caps: focal length %f hor view angle %f ver view angle %f\n",
    __func__, __LINE__, query_buf.sensor_cap.focal_length,
    query_buf.sensor_cap.hor_view_angle, query_buf.sensor_cap.ver_view_angle);
  CDBG("%s:%d\n", __func__, __LINE__);

  identity = pack_identity(1, 0);
  /* Call set mod */
  s_module->set_mod(s_module, MCT_MODULE_FLAG_SOURCE, identity);

  CDBG("%s:%d process_event %p\n", __func__, __LINE__, s_module->process_event);

#if 0
  /* Call Add stream */
  event.sessionIdx = 1;
  event.streamIdx = 0;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event_ctrl.type = MCT_EVENT_ADD_STREAM;
  event.data = (void *)&event_ctrl;
  s_module->process_event(s_module, &event);
  CDBG("%s:%d\n", __func__, __LINE__);
#endif

  CDBG("%s:%d\n", __func__, __LINE__);
  s_port = MCT_PORT_CAST(MCT_MODULE_SRCPORTS(s_module)->data);
  CDBG("%s:%d s_port %p\n", __func__, __LINE__, s_port);
  s_port->ext_link(identity, s_port, NULL);
  CDBG("%s:%d\n", __func__, __LINE__);
  /* Call stream on */
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  stream_info.stream_type = CAM_STREAM_TYPE_PREVIEW;
  stream_info.dim.width = 1984;
  stream_info.dim.height = 1508;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_STREAMON;
  event.u.ctrl_event.control_event_data = (void *)&stream_info;
  s_module->process_event(s_module, &event);
  sleep(5);

  CDBG("%s:%d\n", __func__, __LINE__);
  /* Call stream off */
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_STREAMOFF;
  s_module->process_event(s_module, &event);
  CDBG("%s:%d\n", __func__, __LINE__);
  s_port->un_link(identity, s_port, NULL);
  CDBG("%s:%d\n", __func__, __LINE__);
#if 0
  /* Call remove stream */
  event.sessionIdx = 1;
  event.streamIdx = 0;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event_ctrl.type = MCT_EVENT_REMOVE_STREAM;
  event.data = (void *)&event_ctrl;
  s_module->process_event(s_module, &event);
  CDBG("%s:%d\n", __func__, __LINE__);
#endif
  /* Free sensor mct module */
  module_sensor_deinit(s_module);

  CDBG("%s:%d\n", __func__, __LINE__);
  return 0;
}

