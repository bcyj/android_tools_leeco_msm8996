/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef C2D_PORT_H
#define C2D_PORT_H

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

#define C2D_MAX_STREAMS_PER_PORT  2
#define C2D_MAX_DIVERT_INFO_PER_PORT  4

typedef enum {
  C2D_PORT_STATE_UNRESERVED,
  C2D_PORT_STATE_RESERVED,
  C2D_PORT_STATE_LINKED,
  C2D_PORT_STATE_STREAMING
} c2d_port_state_t;

typedef enum {
  C2D_PORT_TYPE_INVALID,
  C2D_PORT_TYPE_STREAMING,
  C2D_PORT_TYPE_BURST
} c2d_port_type_t;

typedef struct _c2d_port_stream_data_t {
  c2d_port_state_t port_state;
  uint32_t identity;
  cam_streaming_mode_t streaming_mode;
} c2d_port_stream_data_t;

typedef struct _c2d_divert_info_t {
  uint32_t identity[C2D_MAX_STREAMS_PER_PORT];
  pproc_divert_info_t config[C2D_MAX_DIVERT_INFO_PER_PORT];
} c2d_divert_info_t;

typedef struct _c2d_port_data_t {
  c2d_port_type_t port_type;
  c2d_port_stream_data_t stream_data[C2D_MAX_STREAMS_PER_PORT];
  int32_t num_streams;
  uint32_t session_id;
  c2d_divert_info_t c2d_divert_info;
} c2d_port_data_t;


mct_port_t *c2d_port_create(const char* name, mct_port_direction_t dir);

static boolean c2d_port_check_caps_reserve(mct_port_t *port, void *peer_caps,
  void *info);
static boolean c2d_port_check_caps_unreserve(mct_port_t *port,
  uint32_t identity);
static boolean c2d_port_ext_link_func(uint32_t identity,
  mct_port_t* port, mct_port_t *peer);
static void c2d_port_ext_unlink_func(unsigned int identity,
  mct_port_t *port, mct_port_t *peer);
static boolean c2d_port_event_func(mct_port_t *port, mct_event_t *event);
int32_t c2d_port_get_linked_identity(mct_port_t *port, uint32_t identity,
  uint32_t *linked_identity);
#endif
