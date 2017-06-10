/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef VPE_PORT_H
#define VPE_PORT_H

#include "vpe_module.h"

#define VPE_MAX_STREAMS_PER_PORT  2

typedef enum {
  VPE_PORT_STATE_UNRESERVED,
  VPE_PORT_STATE_RESERVED,
  VPE_PORT_STATE_LINKED,
  VPE_PORT_STATE_STREAMING
} vpe_port_state_t;

typedef enum {
  VPE_PORT_TYPE_INVALID,
  VPE_PORT_TYPE_STREAMING,
  VPE_PORT_TYPE_BURST
} vpe_port_type_t;

typedef struct _vpe_port_stream_data_t {
  vpe_port_state_t port_state;
  uint32_t identity;
  cam_streaming_mode_t streaming_mode;
} vpe_port_stream_data_t;


typedef struct _vpe_port_data_t {
  vpe_port_type_t port_type;
  vpe_port_stream_data_t stream_data[VPE_MAX_STREAMS_PER_PORT];
  int32_t num_streams;
  uint32_t session_id;
} vpe_port_data_t;


mct_port_t *vpe_port_create(const char* name, mct_port_direction_t dir);

static boolean vpe_port_check_caps_reserve(mct_port_t *port, void *peer_caps,
  void *info);
static boolean vpe_port_check_caps_unreserve(mct_port_t *port,
  uint32_t identity);
static boolean vpe_port_ext_link_func(uint32_t identity,
  mct_port_t* port, mct_port_t *peer);
static void vpe_port_ext_unlink_func(unsigned int identity,
  mct_port_t *port, mct_port_t *peer);
static boolean vpe_port_event_func(mct_port_t *port, mct_event_t *event);
int32_t vpe_port_get_linked_identity(mct_port_t *port, uint32_t identity,
  uint32_t *linked_identity);
#endif
