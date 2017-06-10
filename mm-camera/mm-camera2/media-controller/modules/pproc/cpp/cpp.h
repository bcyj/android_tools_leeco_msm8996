/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef __CPP_H
#define __CPP_H

#include <media/msmb_pproc.h>
#include "cam_list.h"
#include "pproc_interface.h"

#define ROTATION_BUFFER_SIZE 16384
#define LINE_BUFFER_SIZE 512
#define MAL_SIZE 32

#define PAD_TO_32(a)               (((a)+31)&~31)
#define PAD_TO_2(a)               (((a)+1)&~1)

#define MAX_CPP_DEV 2
#define MAX_CPP_CLIENT 4
#define MAX_CPP_CLIENT_QUEUED_FRAME 8

typedef enum {
  CPP_STATUS_OK = 0,
  CPP_STATUS_NOT_SUPPORTED = 1,
  CPP_STATUS_DEV_ERROR = 2,
  CPP_STATUS_NO_MEM = 3,
  CPP_STATUS_CLIENT_QUEUE_FULL = 4,
} CPP_STATUS;

typedef enum {
  ROT_0,
  ROT_0_H_FLIP,
  ROT_0_V_FLIP,
  ROT_0_HV_FLIP,
  ROT_90,
  ROT_90_H_FLIP,
  ROT_90_V_FLIP,
  ROT_90_HV_FLIP,
} cpp_rot_cfg;

typedef enum {
  PLANE_CBCR,
  PLANE_CRCB,
  PLANE_Y,
  PLANE_CB,
  PLANE_CR,
} cpp_plane_fmt;

typedef enum {
  ASF_OFF,
  ASF_DUAL_FILTER,
  ASF_EMBOSS,
  ASF_SKETCH,
  ASF_NEON
} cpp_asf_mode;

struct cpp_bf_info {
  double bilateral_scale[4];
  double noise_threshold[4];
  double weight[4];
};

struct cpp_plane_scale_info {
  uint8_t v_scale_fir_algo;
  uint8_t h_scale_fir_algo;
  uint8_t v_scale_algo;
  uint8_t h_scale_algo;
  uint8_t subsample_en;
  uint8_t upsample_en;
  uint8_t vscale_en;
  uint8_t hscale_en;

  uint32_t phase_h_step;
  uint32_t phase_v_init;
  uint32_t phase_v_step;
};

struct cpp_stripe_scale_info {
  uint16_t block_width;
  uint16_t block_height;
  uint32_t phase_h_init;
};

struct cpp_rotator_info {
  cpp_rot_cfg rot_cfg;
  uint16_t block_width;
  uint16_t block_height;
  uint32_t block_size;
  uint16_t rowIndex0;
  uint16_t rowIndex1;
  uint16_t colIndex0;
  uint16_t colIndex1;
  uint16_t initIndex;
  uint16_t modValue;
};

struct cpp_crop_info {
  uint32_t enable;
  uint16_t first_pixel;
  uint16_t last_pixel;
  uint16_t first_line;
  uint16_t last_line;
};

struct cpp_fe_info {
  uint32_t buffer_ptr;
  uint32_t buffer_width;
  uint32_t buffer_height;
  uint32_t buffer_stride;
  uint16_t block_width;
  uint16_t block_height;
  uint8_t left_pad;
  uint8_t right_pad;
  uint8_t top_pad;
  uint8_t bottom_pad;
};

struct cpp_we_info {
  uint32_t buffer_ptr[4];
  uint32_t buffer_width;
  uint32_t buffer_height;
  uint32_t buffer_stride;
  uint16_t blocks_per_col;
  uint16_t blocks_per_row;
  uint32_t h_step;
  uint32_t v_step;
  uint16_t h_init;
  uint16_t v_init;
};

struct cpp_stripe_info {
  uint32_t dst_addr;
  uint32_t image_stride;
  uint32_t stripe_index;
  uint32_t stripe_width;
  uint32_t stripe_height;
  uint8_t bytes_per_pixel;
  uint8_t rotation;
  uint8_t mirror;
  struct cpp_fe_info fe_info;
  struct cpp_crop_info bf_crop_info;
  struct cpp_stripe_scale_info scale_info;
  struct cpp_crop_info asf_crop_info;
  struct cpp_rotator_info rot_info;
  struct cpp_we_info we_info;
};

struct cpp_plane_info_t {
  /*input*/
  int src_fd;
  int dst_fd;
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
  int maximum_dst_stripe_height;
  cpp_plane_fmt input_plane_fmt;
  unsigned int source_address;
  unsigned int destination_address;
  /*derived*/
  int num_stripes;
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

  uint32_t bf_crop_enable;
  uint32_t asf_crop_enable;
  uint32_t bf_enable;
  struct cpp_plane_scale_info scale_info;
  struct cpp_stripe_info *stripe_info1;
  struct msm_cpp_frame_strip_info *stripe_info;
};

struct cpp_frame_info_t {
  int32_t frame_id;
  //uint32_t in_buff_idx;
  //uint32_t out_buff_idx;
  enum msm_cpp_frame_type frame_type;
  cpp_asf_mode asf_mode;
  struct cpp_asf_info asf_info;
  struct cpp_bf_info bf_info[3];
  struct cpp_plane_info_t plane_info[MAX_PLANES];
  int num_planes;
  cpp_plane_fmt out_plane_fmt;
  cpp_plane_fmt in_plane_fmt;
  double noise_profile[3][4];
  double weight[3][4];
  double denoise_ratio[3][4];
  double edge_softness[3][4];
  double sharpness_ratio;
};

typedef struct {
  struct cam_list list;
  uint32_t client_id;
  struct cpp_frame_info_t frame_info;
  pproc_interface_frame_divert_t divert_frame;
  //pproc_frame_input_params_t frame_params;
} cpp_process_queue_t;

struct cpp_client_t {
  uint32_t client_inst_id;
  struct msm_cpp_frame_info_t *frame;
  uint32_t client_num_queued_frames;
  cpp_process_queue_t frame_done_queue;
  pproc_client_callback_t client_callback;
};

struct cpp_driver_t {
  struct cpp_hw_info hw_info;
  uint8_t ref_count;
  uint8_t num_cpp_devices;
  uint8_t subdev_node_index[MAX_CPP_DEV];
  int32_t cpp_fd[MAX_CPP_DEV];
  pthread_t pid;
  pthread_mutex_t mutex;
  pthread_cond_t cond_v;
  int is_cpp_thread_ready;
  pthread_cond_t cond_stream_off;
  int is_streamoff_pending;
  int flush_queue;
  cpp_process_queue_t process_queue;
  int num_queued_frames;
  cpp_process_queue_t pending_done_queue;
  int num_pending_done;
  uint32_t inst_id;
  uint8_t initialized;

  uint32_t cpp_client_cnt;
  struct cpp_client_t cpp_client_info[MAX_CPP_CLIENT];
};

struct cpp_library_params_t {
  uint32_t client_id;
  struct cpp_driver_t *driver;
};

CPP_STATUS cpp_get_instance(void *id);
CPP_STATUS cpp_free_instance(void *id);
CPP_STATUS cpp_process_frame(cpp_process_queue_t *new_frame);
CPP_STATUS cpp_client_frame_finish(uint32_t client_id, uint32_t frame_id);

void cpp_prepare_frame_info(struct cpp_frame_info_t *frame_info,
                            struct msm_cpp_frame_info_t *out_info);
#endif
