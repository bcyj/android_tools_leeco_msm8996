/**********************************************************************
* Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "mct_stream.h"
#include "mct_pipeline.h"
#include "mct_object.h"

#include "mct_stream.h"
#include "mct_pipeline.h"


mct_module_t *module_imglib_init(const char *name);
void module_imglib_deinit(mct_module_t *p_mct_mod);

/**
 * Function: main
 *
 * Description: main imglib general module test
 *
 * Input parameters:
 *   argc - argument count
 *   argv - argument strings
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/

mct_module_t *module_imglib_init(const char *name);
void module_imglib_deinit(mct_module_t *p_mct_mod);

int main(int argc, char* argv[])
{
  mct_module_t *module;
  mct_list_t *ports;
  mct_list_t *sink_list;
  mct_port_t *sink_port;
  mct_stream_info_t stream_info;
  mct_event_t event;
  mct_stream_t stream;
  boolean ret;
  int peer_caps, i;


  mct_module_init_name_t imglib_module_name = {
    .name = "img_lib",
    .init_mod = module_imglib_init,
    .deinit_mod = module_imglib_deinit,
  };
  unsigned int sessionid = 0x1;

  fprintf(stderr, "=======================================================\n");
  fprintf(stderr, " Qualcomm Imglib module test \n");
  fprintf(stderr, "=======================================================\n");


  module = imglib_module_name.init_mod(imglib_module_name.name);
  module->start_session(module, sessionid);

  /* Call cap reserve for the port */
  sink_list = MCT_MODULE_SINKPORTS(module);
  if (!sink_list)
    fprintf(stderr, " There are no sink ports \n");

  sink_port = (mct_port_t *)sink_list->data;

  /* Fill stream info with something useful */
  memset(&stream_info, 0x00, sizeof(stream_info));
  memset(&peer_caps, 0x00, sizeof(peer_caps));
  memset(&event, 0x00, sizeof(event));
  memset(&stream, 0x00, sizeof(stream));

  stream_info.stream_type = CAM_STREAM_TYPE_PREVIEW;
  stream_info.identity = 0x8001;

  sink_port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;

  for (i = 0; i < 1000; i++) {
    ret = sink_port->check_caps_reserve(sink_port, &peer_caps, &stream_info);
    if (ret == FALSE) {
      fprintf(stderr, "Error cap reserve \n");
      goto out;
    }

    event.direction = MCT_EVENT_DOWNSTREAM;
    ret = sink_port->event_func(sink_port, &event);
    if (ret == FALSE) {
      fprintf(stderr, "Event port function \n");
      goto out;
    }

    /* We can skip the link since we already reserve the port */
    ret = sink_port->check_caps_unreserve(sink_port, 0x00);
    if (ret == FALSE) {
      fprintf(stderr, "Error cap unreserve \n");
      goto out;
    }
  }

  ret = mct_object_set_parent(MCT_OBJECT_CAST(module), MCT_OBJECT_CAST(&stream));
  if (ret == FALSE) {
    fprintf(stderr, "Error Can not add parent \n");
    goto out;
  }
  stream.streaminfo.stream_type = CAM_STREAM_TYPE_PREVIEW;

  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_STREAMON;
  module->process_event(module, &event);

  event.u.ctrl_event.type = MCT_EVENT_CONTROL_STREAMOFF;
  module->process_event(module, &event);

out:

  module->stop_session(module, sessionid);
  imglib_module_name.deinit_mod(module);

  return 0;
}

