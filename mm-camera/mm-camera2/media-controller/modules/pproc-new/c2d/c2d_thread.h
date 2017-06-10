/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef C2D_THREAD_H
#define C2D_THREAD_H

#include "modules.h"
#include "mct_queue.h"
#include "mct_list.h"
#include "media_controller.h"
#include "mct_port.h"
#include "mct_object.h"
#include "cam_types.h"
#include "mct_module.h"
#include "mct_pipeline.h"
#include "mct_stream.h"
#include "camera_dbg.h"
#include "c2d_hardware.h"
#include "c2d_module.h"

typedef struct _c2d_module_ctrl_t c2d_module_ctrl_t;

typedef enum {
  C2D_THREAD_MSG_NEW_EVENT_IN_Q,
  C2D_THREAD_MSG_ABORT
} c2d_thread_msg_type_t;

typedef struct _c2d_thread_msg_t {
  c2d_thread_msg_type_t type;
  void *data;
} c2d_thread_msg_t;

int32_t c2d_thread_create(mct_module_t *module);
int32_t c2d_thread_process_pipe_message(c2d_module_ctrl_t *ctrl,
  c2d_thread_msg_t msg);
static void c2d_thread_fatal_exit(c2d_module_ctrl_t *ctrl, boolean post_to_bus);
static int32_t c2d_thread_process_hardware_event(c2d_module_ctrl_t *ctrl);
#endif
