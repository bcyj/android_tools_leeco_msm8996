/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef __LIVE_SNAPSHOT_H__
#define __LIVE_SNAPSHOT_H__

#include "tgtcommon.h"
#include "sensor_interface.h"

typedef struct {
  uint32_t buf_size;
  uint8_t  *buf;
  int32_t  fd;
  struct ion_allocation_data ion_alloc;
  struct ion_fd_data fd_data;
} liveshot_buffer_alloc_t;

typedef struct {
  struct v4l2_buffer v4l2_buf;
  struct v4l2_plane planes[VIDEO_MAX_PLANES];
  uint8_t num_planes;
} liveshot_frame_t;

typedef struct {
  char node_name[MAX_DEV_NAME_LEN];
  int main_node_fd;
  cam_format_t main_format;
  uint32_t main_image_mode;
  struct v4l2_format main_fmt;
  int buf_count;
  liveshot_buffer_alloc_t main_buf[2];
  liveshot_frame_t main_frames[2];
} liveshot_buffer_t;

typedef struct {
  uint32_t total_mctl_frames;
  liveshot_buffer_t buffer_info;
} liveshot_ctrl_t;

void liveshot_init(liveshot_ctrl_t* liveshot_ctrl, char *node_name);
int32_t prepare_liveshot(void *ctrlblk);
int32_t destroy_liveshot(void *ctrlblk);

#endif /*__LIVE_SNAPSHOT_H__*/
