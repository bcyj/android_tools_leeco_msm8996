/* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential. */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <time.h>
#include "camera.h"
#include "cam_mmap.h"
#include "cam_list.h"
#include "camera_dbg.h"
#include "cpp.h"

struct cpp_frame_info_t frame_1080p_r0_y = {
  .src_width = 1920,
  .src_height = 1080,
  .src_stride = 1920,
  .dst_width = 1920,
  .dst_height = 1080,
  .dst_stride = 1920,
  .rotate = 0,
  .mirror = 0,
  .prescale_padding = 22,
  .postscale_padding = 4,
  .h_scale_ratio = 1,
  .v_scale_ratio = 1,
  .h_scale_initial_phase = 0,
  .v_scale_initial_phase = 0,
  .line_buffer_size = 512,
  .mal_byte_size = 32,
  .maximum_dst_stripe_height = 1080,
  .bytes_per_pixel = 1,
  .source_address = 0,
  .destination_address = 0,
};

typedef struct {
  void *ptr;
  CPP_STATUS (*cpp_get_instance)(uint32_t *cpp_client_inst_id);
  CPP_STATUS (*cpp_free_instance)(uint32_t cpp_client_inst_id);
  CPP_STATUS (*cpp_process_frame)(cpp_process_queue_t *new_frame);
  CPP_STATUS (*cpp_client_frame_finish)(uint32_t client_id, uint32_t frame_id);
  void (*cpp_prepare_frame_info)(struct cpp_frame_info_t *in_info,
                                 struct msm_cpp_frame_info_t *out_info);
} cpp_lib_t;

int main(int argc, char **argv)
{
  int rc = 0, dev_fd = 0;
  int i;
  uint32_t client_id = 0;
  cpp_process_queue_t *new_frame;
  struct msm_cpp_frame_info_t *frame;
  cpp_lib_t cpp_lib;
  CDBG("%s E\n", __func__);

  cpp_lib.ptr = NULL;
  cpp_lib.ptr = dlopen("libmmcamera_cpp.so", RTLD_NOW);
  if (!cpp_lib.ptr) {
    CDBG_ERROR("%s ERROR: couldn't dlopen libcpp.so: %s", __func__, dlerror());
    return -EINVAL;
  }

  *(void **)&cpp_lib.cpp_get_instance =
  dlsym(cpp_lib.ptr, "cpp_get_instance");
  *(void **)&cpp_lib.cpp_free_instance =
  dlsym(cpp_lib.ptr, "cpp_free_instance");
  *(void **)&cpp_lib.cpp_process_frame =
  dlsym(cpp_lib.ptr, "cpp_process_frame");
  *(void **)&cpp_lib.cpp_client_frame_finish =
  dlsym(cpp_lib.ptr, "cpp_client_frame_finish");


  cpp_lib.cpp_get_instance(&client_id);

  for (i = 0; i < 20; i++) {
    new_frame = (cpp_process_queue_t *)malloc(sizeof(cpp_process_queue_t));
    new_frame->frame_id = i;
    new_frame->client_id = client_id;
    new_frame->frame_type = MSM_CPP_REALTIME_FRAME;
    new_frame->frame_info = frame_1080p_r0_y;
    CDBG("%s: Enqueue frame\n", __func__);
    cpp_lib.cpp_process_frame(new_frame);
    CDBG("%s: Enqueue frame done\n", __func__);
    cpp_lib.cpp_client_frame_finish(client_id, i);
    CDBG("%s: frame %d finish\n", __func__, i);
    free(new_frame);
  }

  cpp_lib.cpp_free_instance(client_id);
  CDBG("%s X\n", __func__);
  return rc;
}
