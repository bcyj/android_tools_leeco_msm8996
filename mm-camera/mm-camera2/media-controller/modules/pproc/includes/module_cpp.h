/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __MODULE_CPP_H__
#define __MODULE_CPP_H__

#include "modules.h"
#include "mct_list.h"

#define MAX_CPP_DEV 2
#define MAX_SUBDEV_SIZE 32

mct_module_t *module_cpp_init(const char *name);
void module_cpp_deinit(mct_module_t *module);
boolean module_cpp_fill_input_params(mct_module_t *module,
  pproc_frame_input_params_t *frame_params, void *buffer,
  module_pproc_common_frame_params_t *prt_frame_params,
  uint32_t event_identity, mct_port_t *port);

#endif
