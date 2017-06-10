/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef CPP_PORT_H
#define CPP_PORT_H

#include "modules.h"
#include "chromatix.h"
#include "chromatix_common.h"
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

#define CPP_MAX_STREAMS_PER_PORT  2
#define CPP_MAX_DIVERT_INFO_PER_PORT  4

typedef enum {
  CPP_PORT_STATE_UNRESERVED,
  CPP_PORT_STATE_RESERVED,
  CPP_PORT_STATE_LINKED,
  CPP_PORT_STATE_STREAMING
} cpp_port_state_t;

typedef enum {
  CPP_PORT_TYPE_INVALID,
  CPP_PORT_TYPE_STREAMING,
  CPP_PORT_TYPE_BURST
} cpp_port_type_t;

typedef struct _cpp_port_stream_data_t {
  cpp_port_state_t port_state;
  uint32_t identity;
  cam_streaming_mode_t streaming_mode;
} cpp_port_stream_data_t;

typedef struct _cpp_divert_info_t {
  uint32_t identity[CPP_MAX_STREAMS_PER_PORT];
  pproc_divert_info_t config[CPP_MAX_DIVERT_INFO_PER_PORT];
} cpp_divert_info_t;

typedef struct _cpp_port_data_t {
  cpp_port_type_t port_type;
  cpp_port_stream_data_t stream_data[CPP_MAX_STREAMS_PER_PORT];
  int32_t num_streams;
  uint32_t session_id;
  cpp_divert_info_t cpp_divert_info;
} cpp_port_data_t;


mct_port_t *cpp_port_create(const char* name, mct_port_direction_t dir);

static boolean cpp_port_check_caps_reserve(mct_port_t *port, void *peer_caps,
  void *info);
static boolean cpp_port_check_caps_unreserve(mct_port_t *port,
  uint32_t identity);
static boolean cpp_port_ext_link_func(uint32_t identity,
  mct_port_t* port, mct_port_t *peer);
static void cpp_port_ext_unlink_func(unsigned int identity,
  mct_port_t *port, mct_port_t *peer);
static boolean cpp_port_event_func(mct_port_t *port, mct_event_t *event);
#endif
