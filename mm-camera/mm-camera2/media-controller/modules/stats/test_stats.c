/* q3a_port.c
 *
 * Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "mct_module.h"
#include "mct_stream.h"
#include "mct_pipeline.h"
#include "camera_dbg.h"

int main(int argc, char * argv[])
{
  mct_module_t *stats_module = NULL;
  mct_pipeline_cap_t query_buf;
  mct_event_t event;
  uint32_t i = 0;
  cam_stream_info_t stream_info;
  mct_port_t *stats_port = NULL;
  uint32_t identity = 0;

  /* Call mct module init */
  stats_module = stats_module_init("stats");
  if (!stats_module) {
    CDBG("%s: stats_module is Null\n", __func__);
    return -1;
  }
  /* Pack identity Session = 1 stream = 0*/
  identity = pack_identity(1, 0);

  /* Call set mod */
  stats_module->set_mod(stats_module, MCT_MODULE_FLAG_SINK, identity);

  if (!stats_module->process_event) {
    CDBG("%s: process event is Null\n", __func__);
    return -1;
  }

  stats_port = MCT_PORT_CAST(MCT_MODULE_SRCPORTS(stats_module)->data);
  if (!stats_port) {
    CDBG("%s: stats_port is Null\n", __func__);
    return -1;
  }

  stats_port->ext_link(identity, stats_port, NULL);

  /* Call Set parameter */
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD; /* From MCT */
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  stream_info.stream_type = CAM_STREAM_TYPE_PREVIEW;
  event.u.ctrl_event.control_event_data = (void *)&stream_info;
  stats_module->process_event(stats_module, &event);

  /* Send STATS event */
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_MODULE_EVENT; /* From ISP module */
  event.u.ctrl_event.type = MCT_EVENT_MODULE_STATS_DATA;
  stats_module->process_event(stats_module, &event);

  stats_port->un_link(identity, stats_port, NULL);

  /* Deinit stats mct module */
  stats_module_deinit(stats_module);

  return 0;
}


