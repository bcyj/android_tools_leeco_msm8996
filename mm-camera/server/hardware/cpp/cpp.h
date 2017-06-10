/* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential. */

#ifndef __CPP_H
#define __CPP_H

typedef enum {
  CPP_STATUS_OK = 0,
  CPP_STATUS_NOT_SUPPORTED = 1,
  CPP_STATUS_DEV_ERROR = 2,
  CPP_STATUS_NO_MEM = 3,
  CPP_STATUS_CLIENT_QUEUE_FULL = 4,
} CPP_STATUS;

struct cpp_frame_info_t {
  /*input*/
  int src_width;
  int src_height;
  int src_stride;
  int dst_width;
  int dst_height;
  int dst_stride;
  int rotate;
  int mirror;
  int prescale_padding;
  int postscale_padding;
  double h_scale_ratio;
  double v_scale_ratio;
  int horizontal_scale_ratio;
  int vertical_scale_ratio;
  double h_scale_initial_phase;
  double v_scale_initial_phase;
  long long horizontal_scale_initial_phase;
  long long vertical_scale_initial_phase;
  int line_buffer_size;
  int maximum_dst_stripe_height;
  int mal_byte_size;
  int bytes_per_pixel;
  unsigned int source_address;
  unsigned int destination_address;
  /*derived*/
  int frame_width_mcus; /*The number of stripes*/
  int frame_height_mcus; /*The number of blocks in the vertical direction*/
  /*dst_height rounded up to the rotation_block_height*/
  int dst_height_block_aligned;
  /*common stripe width
    a stripe that is on the left or right boundary can have a smaller width*/
  int dst_block_width;
  /*common stripe height,
    a block that is on the top or bottom can have a smaller height*/
  int dst_block_height;
  int *stripe_block_width;/*actual block width in the horizontal direction*/
  int *stripe_block_height;/*actual block height in the vertical direction*/
  /*The initial phase of the topleft corner fetch block
    assuming each dst block has the same width dst_block_width*/
  long long horizontal_scale_block_initial_phase;
  /*The initial phase of the topleft corner fetch block
    assuming each dst block has the same height dst_block_height*/
  long long vertical_scale_block_initial_phase;
};

typedef struct {
  struct cam_list list;
  int32_t frame_id;
  uint32_t client_id;
  enum msm_cpp_frame_type frame_type;
  struct cpp_frame_info_t frame_info;
} cpp_process_queue_t;

CPP_STATUS cpp_get_instance(uint32_t *cpp_client_inst_id);
CPP_STATUS cpp_free_instance(uint32_t cpp_client_inst_id);
CPP_STATUS cpp_process_frame(cpp_process_queue_t *new_frame);
CPP_STATUS cpp_client_frame_finish(uint32_t client_id, uint32_t frame_id);

void cpp_prepare_frame_info(struct cpp_frame_info_t *in_info,
                            struct msm_cpp_frame_info_t *out_info);
#endif
