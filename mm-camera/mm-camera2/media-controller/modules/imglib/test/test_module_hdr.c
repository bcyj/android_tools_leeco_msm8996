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

extern mct_module_t *module_hdr_init(const char *name);
extern void module_hdr_deinit(mct_module_t *p_mct_mod);

#define IDENTITY 0xABCD
/**
 * Function: main
 *
 * Description: main hdr general module test
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

int main(int argc, char* argv[])
{
  mct_module_t *module;
  mct_list_t *ports;
  mct_list_t *sink_list;
  mct_port_t *sink_port;
  mct_stream_info_t *stream_info;
  mct_event_t event;
  mct_stream_t stream;
  boolean ret;
  int i;
  mct_port_caps_t peer_caps;
  mct_event_control_parm_t event_parm;

  mct_module_init_name_t hdr_module_name = { .name = "hdr", .init_mod =
    module_hdr_init, .deinit_mod = module_hdr_deinit, };
  unsigned int sessionid = 0x1;

//#define MY_TESTING
#ifdef MY_TESTING

  volatile int bp = 1;

  while (bp == 1) {
    usleep(100000); // sleep for 0.1 seconds
  }
#endif // MY_TESTING
  fprintf(stderr, "=======================================================\n");
  fprintf(stderr, " Qualcomm hdr module test \n");
  fprintf(stderr, "=======================================================\n");

  /* Fill stream info with something useful */
  memset(&peer_caps, 0x00, sizeof(peer_caps));
  memset(&event, 0x00, sizeof(event));
  memset(&stream, 0x00, sizeof(stream));

  //Module initialization, which initiates a dummy src port and reserves it
  module = hdr_module_name.init_mod(hdr_module_name.name);
  if (NULL == module) {
    fprintf(stderr, " Module Init failed");
    goto out;
  }

  //start session is dummy, does nothing. Could be removed later
  if (module->start_session(module, sessionid) != TRUE) {
    fprintf(stderr, " Module start session failed");
    goto out;
  }

  /*
   * set a stream as module parent for test purpose
   * Set stream type to snapshot and set some number (its a don't care)
   * as identity
   */
  stream_info = &stream.streaminfo;
  stream_info->stream_type = CAM_STREAM_TYPE_SNAPSHOT;
  stream_info->identity = IDENTITY;
  ret = mct_object_set_parent(MCT_OBJECT_CAST(module),
    MCT_OBJECT_CAST(&stream));
  if (ret == FALSE) {
    fprintf(stderr, "Error Can not add parent \n");
    goto out;
  }

  /*Send strm on event. This creates a sink port which internally connects to
   dummy src port. Also calls STRM on on the newly created port
   */
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_STREAMON;
  event.identity = IDENTITY;
  if (module->process_event(module, &event) != TRUE) {
    fprintf(stderr, " Module STREAM on failed \n");
    goto out;
  }

  sink_list = MCT_MODULE_SINKPORTS(module);
  if (!sink_list) {
    fprintf(stderr, " There are no sink ports \n");
    goto out;
  }
  sink_port = (mct_port_t *)sink_list->data;

  /*
   * Try doing a caps reserve on the sink module
   * This would fail as port was reserved when it was created.
   */

  peer_caps.port_caps_type = MCT_PORT_CAPS_FRAME;
  ret = sink_port->check_caps_reserve(sink_port, &peer_caps, &stream_info);
  if (ret == FALSE) {
    fprintf(stderr, "cap reserve failed \n");
//		goto out;
  }

  //set HDR parameter for the module
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  event.identity = IDENTITY;
  event_parm.type = CAM_INTF_PARM_HDR;
  event.u.ctrl_event.control_event_data = &event_parm;
  if (module->process_event(module, &event) != TRUE) {
    fprintf(stderr, " Module STREAM on failed \n");
    goto out;
  }

  /*
   * Send a gamma update event command to port:
   * Testing the execution path. Probably will fail
   */
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_GAMMA_UPDATE;
  event.identity = IDENTITY;
  if (TRUE != sink_port->event_func(sink_port, &event)) {
    fprintf(stderr, "Gamma update event on sink port failed \n");
//		goto out;
  }

  /*
   * Send a buff divert event command to port:
   * Testing the execution path. Probably will fail
   */
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
  event.identity = IDENTITY;
  if (TRUE != sink_port->event_func(sink_port, &event)) {
    fprintf(stderr, "Buff divert event failed \n");
//		goto out;
  }

  /* This should fail as STRM off should have already unreserved
   * the port */
  ret = sink_port->check_caps_unreserve(sink_port, 0x00);
  if (ret == FALSE) {
    fprintf(stderr, "cap unreserve failed \n");
//		goto out;
  }

  /*
   * Send the stream off even which should result in lib processing
   * abort
   */
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_STREAMOFF;
  if (module->process_event(module, &event) != TRUE) {
    fprintf(stderr, " Module STREAMOFF failed \n");
//		goto out;
  }

  out:
  /*
   * Call session stop, which should result in auto removal of
   * session based ports (only dummy port should remain) and do a deinit
   */
  if (module) {
    module->stop_session(module, sessionid);
    hdr_module_name.deinit_mod(module);
  }

  return 0;
}

