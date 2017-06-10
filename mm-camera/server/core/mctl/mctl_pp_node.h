/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __MCTL_PP_NODE_H__
#define __MCTL_PP_NODE_H__

#include "camera.h"

#define MCTL_PP_NODE_MAX_NUM_BUF      16
#define MCTL_PP_VIDEO_BUFFER_CNT     4

typedef struct {
  uint32_t buf_size;
  uint8_t  *buf;
  int32_t  fd;
  struct ion_allocation_data ion_alloc;
  struct ion_fd_data fd_data;
} mctl_pp_node_buffer_alloc_t;

typedef struct {
    struct v4l2_buffer v4l2_buf;
    struct v4l2_plane planes[VIDEO_MAX_PLANES];
    uint8_t num_planes;
}mctl_pp_node_frame_t;

typedef struct {
  int fd;
  cam_stream_info_def_t strm_info;
  struct v4l2_format fmt;
  int buf_count;
  mctl_pp_node_buffer_alloc_t buf_alloc[MCTL_PP_NODE_MAX_NUM_BUF];
  mctl_pp_node_frame_t frames[MCTL_PP_NODE_MAX_NUM_BUF];
  int acquired_for_rdi;
}mctl_pp_node_obj_t;

#endif
