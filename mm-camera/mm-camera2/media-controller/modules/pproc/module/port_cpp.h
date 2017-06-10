/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef __PORT_CPP_H__
#define __PORT_CPP_H__

#include "mct_port.h"
#include "module_pproc_common.h"

typedef struct {
  module_pproc_common_query_caps_t *caps;
} port_cpp_t;

boolean port_cpp_init(mct_port_t *port, mct_port_direction_t direction);

#endif /* __PORT_CPP_H__ */
