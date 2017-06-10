/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef __VPE_HW_PARAMS_H__
#define __VPE_HW_PARAMS_H__

#include <media/msmb_pproc.h>
#include "vpe_util.h"
#include "vpe_reg_pack.h"
#include "mtype.h"

typedef enum camera_rotation_type {
  ROT_NONE               = 0,
  ROT_CLOCKWISE_90       = 1,
  ROT_CLOCKWISE_180      = 6,
  ROT_CLOCKWISE_270      = 7,
} camera_rotation_type;

/**
 * The format of the following structure must match what is expected
 * by VIDIOC_MSM_VPE_TRANSACTION_SETUP in the kernel (see
 * drivers/media/platform/msm/camera_v2/pproc/vpe/msm_vpe.h). Since
 * all of structures within this structure only contain word-aligned
 * data types, we don't have to worry about the compiler adding extra
 * padding. At least that's what we like to tell ourselves...
 */
struct vpe_transaction_setup_t {
    mm_vpe_scale_coef_cfg_type      scaler_cfg_0;
    mm_vpe_scale_coef_cfg_type      scaler_cfg_1;
    mm_vpe_scale_coef_cfg_type      scaler_cfg_2;
    mm_vpe_scale_coef_cfg_type      scaler_cfg_3;
    mm_vpe_input_plane_config_type  input_hw_cfg;
    mm_vpe_output_plane_config_type output_hw_cfg;
    mm_vpe_op_mode_packed           hw_op_mode;
};

#define PAD_TO_32(a)               (((a)+31)&~31)
#define PAD_TO_2(a)               (((a)+1)&~1)

typedef enum {
  VPE_ROT_0,
  VPE_ROT_0_H_FLIP,
  VPE_ROT_0_V_FLIP,
  VPE_ROT_0_HV_FLIP,
  VPE_ROT_90,
  VPE_ROT_90_H_FLIP,
  VPE_ROT_90_V_FLIP,
  VPE_ROT_90_HV_FLIP,
} vpe_rot_cfg;

typedef enum {
  VPE_PLANE_CBCR,
  VPE_PLANE_CRCB,
  VPE_PLANE_Y,
  VPE_PLANE_CB,
  VPE_PLANE_CR,
} vpe_plane_fmt;

struct vpe_bf_info {
  double bilateral_scale[4];
  double noise_threshold[4];
  double weight[4];
};

struct vpe_plane_scale_info {
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

struct vpe_stripe_scale_info {
  uint16_t block_width;
  uint16_t block_height;
  uint32_t phase_h_init;
};

struct vpe_rotator_info {
  vpe_rot_cfg rot_cfg;
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

struct vpe_crop_info {
  uint32_t enable;
  uint16_t first_pixel;
  uint16_t last_pixel;
  uint16_t first_line;
  uint16_t last_line;
};

struct vpe_fe_info {
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

struct vpe_we_info {
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

struct vpe_stripe_info {
  uint32_t dst_addr;
  uint32_t image_stride;
  uint32_t stripe_index;
  uint32_t stripe_width;
  uint32_t stripe_height;
  uint8_t bytes_per_pixel;
  uint8_t rotation;
  uint8_t mirror;
  struct vpe_fe_info fe_info;
  struct vpe_crop_info bf_crop_info;
  struct vpe_stripe_scale_info scale_info;
  struct vpe_crop_info asf_crop_info;
  struct vpe_rotator_info rot_info;
  struct vpe_we_info we_info;
};

struct vpe_plane_info_t {
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
  vpe_plane_fmt input_plane_fmt;
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
  struct vpe_plane_scale_info scale_info;
  struct vpe_stripe_info *stripe_info1;
  struct msm_vpe_frame_strip_info *stripe_info;
};

struct vpe_frame_info_t {
  int32_t frame_id;
  struct timeval timestamp;
  uint32_t identity;
  uint32_t buff_index;
  uint32_t native_buff;
  void    *cookie;
  enum msm_vpe_frame_type frame_type;
  /* struct vpe_asf_info asf_info; */
  /* struct vpe_bf_info bf_info[3]; */
  struct vpe_plane_info_t plane_info[MAX_PLANES];
  int num_planes;
  vpe_plane_fmt out_plane_fmt;
  vpe_plane_fmt in_plane_fmt;
  /* double noise_profile[3][4]; */
  double weight[3][4];
  /* double denoise_ratio[3][4]; */
  /* double edge_softness[3][4]; */
  /* double sharpness_ratio; */
};

typedef struct _vpe_params_crop_window_t {
  uint32_t x, y, dx, dy;
} vpe_params_crop_window_t;

typedef struct _vpe_params_crop_info_t {
  vpe_params_crop_window_t stream_crop;
  vpe_params_crop_window_t is_crop;
  uint32_t process_window_first_pixel;
  uint32_t process_window_first_line;
  uint32_t process_window_width;
  uint32_t process_window_height;
} vpe_params_crop_info_t;

typedef struct _vpe_params_scale_info_t {
  double h_scale_ratio;
  double v_scale_ratio;
} vpe_params_scale_info_t;

typedef enum {
  VPE_PARAM_PLANE_CBCR,
  VPE_PARAM_PLANE_CRCB,
  VPE_PARAM_PLANE_Y,
  VPE_PARAM_PLANE_CB,
  VPE_PARAM_PLANE_CR,
} vpe_params_plane_fmt_t;

typedef struct {
  int fd;
  uint32_t index;
  uint32_t offset;
  uint8_t native_buff;
  uint8_t processed_divert;
} vpe_hardware_buffer_info_t;

typedef struct {
  uint32_t identity;
  uint32_t num_buffs;
  vpe_hardware_buffer_info_t *buffer_info;
} vpe_hardware_stream_buff_info_t;

typedef struct _vpe_params_dim_info_t {
  int32_t                width;
  int32_t                height;
  int32_t                stride;
  int32_t                scanline;
  vpe_params_plane_fmt_t  plane_fmt;
} vpe_params_dim_info_t;

typedef struct _vpe_hardware_params_t {
  /* TODO: Remove frameid, identity and timestamp */
  uint32_t frame_id;
  uint32_t identity;
  struct timeval timestamp;
  /* double                    sharpness_level; */
  /* vpe_params_asf_mode_t     asf_mode; */
  /* vpe_params_asf_info_t     asf_info; */
  vpe_params_scale_info_t   scale_info;
  uint32_t                  rotation;
  uint32_t                  mirror;
  vpe_params_crop_info_t    crop_info;
  vpe_params_dim_info_t     input_info;
  vpe_params_dim_info_t     output_info;
  boolean                   denoise_enable;
  /* vpe_params_denoise_info_t */
  /*   denoise_info[CPP_DENOISE_NUM_PLANES][CPP_DENOISE_NUM_PROFILES]; */
  void                     *cookie;
  vpe_hardware_buffer_info_t buffer_info;
} vpe_hardware_params_t;

void vpe_params_prepare_frame_info(struct vpe_frame_info_t *frame_info,
                                   struct msm_vpe_frame_info_t *out_info);
void vpe_params_create_frame_info(vpe_hardware_params_t *hw_params,
                                  struct vpe_frame_info_t *frame_info);
boolean vpe_hardware_validate_params(vpe_hardware_params_t *hw_params);
void vpe_config_pipeline(struct vpe_transaction_setup_t *setup,
                         struct vpe_frame_info_t *frame_info,
                         struct msm_vpe_frame_info_t *msm_frame_info);

#endif /* __VPE_HW_PARAMS_H__ */
