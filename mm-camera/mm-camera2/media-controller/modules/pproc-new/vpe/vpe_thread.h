/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef VPE_THREAD_H
#define VPE_THREAD_H

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
#include "vpe_hardware.h"
#include "vpe_module.h"

typedef struct _vpe_module_ctrl_t vpe_module_ctrl_t;

typedef enum {
  VPE_THREAD_MSG_NEW_EVENT_IN_Q,
  VPE_THREAD_MSG_ABORT
} vpe_thread_msg_type_t;

typedef struct _vpe_thread_msg_t {
  vpe_thread_msg_type_t type;
  void *data;
} vpe_thread_msg_t;

int32_t vpe_thread_create(mct_module_t *module);
int32_t vpe_thread_process_pipe_message(vpe_module_ctrl_t *ctrl,
  vpe_thread_msg_t msg);
static void vpe_thread_fatal_exit(vpe_module_ctrl_t *ctrl, boolean post_to_bus);
static int32_t vpe_thread_process_hardware_event(vpe_module_ctrl_t *ctrl);
#endif
