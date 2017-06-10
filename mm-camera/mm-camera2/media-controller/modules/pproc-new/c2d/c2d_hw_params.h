/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef C2D_HW_PARAMS_H
#define C2D_HW_PARAMS_H

#include <media/msmb_camera.h>
#include <media/msmb_pproc.h>
#include "cam_types.h"
#include "mtype.h"
#include "modules.h"
#include "chromatix.h"
#include "chromatix_common.h"

#define PAD_TO_32(a)               (((a)+31)&~31)
#define PAD_TO_2(a)               (((a)+1)&~1)

#define C2D_PARAM_ASF_F_KERNEL_MAX          16
#define C2D_PARAM_ASF_LUT_MAX               24
#define C2D_PARAM_ASF_LUT3_MAX              12

#define ROTATION_BUFFER_SIZE 16384
#define LINE_BUFFER_SIZE 512
#define MAL_SIZE 32

typedef enum {
  C2D_ROT_0,
  C2D_ROT_0_H_FLIP,
  C2D_ROT_0_V_FLIP,
  C2D_ROT_0_HV_FLIP,
  C2D_ROT_90,
  C2D_ROT_90_H_FLIP,
  C2D_ROT_90_V_FLIP,
  C2D_ROT_90_HV_FLIP,
} c2d_rot_cfg;

typedef enum {
  C2D_PLANE_CBCR,
  C2D_PLANE_CRCB,
  C2D_PLANE_Y,
  C2D_PLANE_CB,
  C2D_PLANE_CR,
} c2d_plane_fmt;

struct c2d_crop_info {
  uint32_t enable;
  uint16_t first_pixel;
  uint16_t last_pixel;
  uint16_t first_line;
  uint16_t last_line;
};

struct c2d_plane_info_t {
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
  c2d_plane_fmt input_plane_fmt;
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
};

struct c2d_frame_info_t {
  int32_t frame_id;
  struct timeval timestamp;
  uint32_t identity;
  uint32_t buff_index;
  uint32_t native_buff;
  void    *cookie;
  struct c2d_plane_info_t plane_info[MAX_PLANES];
  int num_planes;
  c2d_plane_fmt out_plane_fmt;
  c2d_plane_fmt in_plane_fmt;
  double noise_profile[3][4];
  double weight[3][4];
  double denoise_ratio[3][4];
  double edge_softness[3][4];
  double sharpness_ratio;
};

/* ---------------------------- new params --------------------------------- */

#define C2D_DENOISE_NUM_PLANES    3
#define C2D_DENOISE_NUM_PROFILES  4

typedef enum {
  C2D_PARAM_ASF_OFF,
  C2D_PARAM_ASF_DUAL_FILTER,
  C2D_PARAM_ASF_EMBOSS,
  C2D_PARAM_ASF_SKETCH,
  C2D_PARAM_ASF_NEON,
} c2d_params_asf_mode_t;

typedef struct _c2d_params_scale_info_t {
  double h_scale_ratio;
  double v_scale_ratio;
} c2d_params_scale_info_t;

typedef struct _c2d_params_denoise_info_t {
  double noise_profile;
  double weight;
  double denoise_ratio;
  double edge_softness;
} c2d_params_denoise_info_t;

typedef struct _c2d_params_crop_window_t {
  uint32_t x, y, dx, dy;
} c2d_params_crop_window_t;

typedef struct _c2d_params_is_crop_window_t {
  int use_3d;
  uint32_t x, y, dx, dy;
  float transform_matrix[9];
  unsigned int transform_type;
} c2d_params_is_crop_window_t;

typedef struct _c2d_params_crop_info_t {
  c2d_params_crop_window_t stream_crop;
  c2d_params_is_crop_window_t is_crop;
  uint32_t process_window_first_pixel;
  uint32_t process_window_first_line;
  uint32_t process_window_width;
  uint32_t process_window_height;
} c2d_params_crop_info_t;

typedef enum {
  C2D_PARAM_PLANE_CBCR,
  C2D_PARAM_PLANE_CRCB,
  C2D_PARAM_PLANE_Y,
  C2D_PARAM_PLANE_CB,
  C2D_PARAM_PLANE_CR,
  C2D_PARAM_PLANE_CBCR422,
  C2D_PARAM_PLANE_CRCB422,
  C2D_PARAM_PLANE_CRCB420,
  C2D_PARAM_PLANE_YCBYCR422,
  C2D_PARAM_PLANE_YCRYCB422,
  C2D_PARAM_PLANE_CBYCRY422,
  C2D_PARAM_PLANE_CRYCBY422,
} c2d_params_plane_fmt_t;

typedef struct {
  int fd;
  unsigned long vaddr;
  uint32_t index;
  uint32_t offset;
  uint8_t native_buff;
  uint8_t processed_divert;
} c2d_hardware_buffer_info_t;

typedef struct {
  uint32_t identity;
  uint32_t num_buffs;
  c2d_hardware_buffer_info_t *buffer_info;
} c2d_hardware_stream_buff_info_t;

typedef struct _c2d_params_dim_info_t {
  int32_t                width;
  int32_t                height;
  int32_t                stride;
  int32_t                scanline;
  cam_format_t           cam_fmt;
  c2d_params_plane_fmt_t c2d_plane_fmt;
} c2d_params_dim_info_t;

typedef struct _c2d_params_asf_info_t {
  float sp;
  uint8_t neg_abs_y1;
  uint8_t dyna_clamp_en;
  uint8_t sp_eff_en;
  int16_t clamp_h_ul;
  int16_t clamp_h_ll;
  int16_t clamp_v_ul;
  int16_t clamp_v_ll;
  float clamp_scale_max;
  float clamp_scale_min;
  uint16_t clamp_offset_max;
  uint16_t clamp_offset_min;
  uint32_t nz_flag;
  float sobel_h_coeff[C2D_PARAM_ASF_F_KERNEL_MAX];
  float sobel_v_coeff[C2D_PARAM_ASF_F_KERNEL_MAX];
  float hpf_h_coeff[C2D_PARAM_ASF_F_KERNEL_MAX];
  float hpf_v_coeff[C2D_PARAM_ASF_F_KERNEL_MAX];
  float lpf_coeff[C2D_PARAM_ASF_F_KERNEL_MAX];
  float lut1[C2D_PARAM_ASF_LUT_MAX];
  float lut2[C2D_PARAM_ASF_LUT_MAX];
  float lut3[C2D_PARAM_ASF_LUT3_MAX];
} c2d_params_asf_info_t;

typedef struct _c2d_params_aec_trigger_params {
  float lowlight_trigger_start;
  float lowlight_trigger_end;
  float brightlight_trigger_start;
  float brightlight_trigger_end;
  float aec_trigger_input;
} c2d_params_aec_trigger_t;

typedef enum _c2d_params_asf_region {
  C2D_PARAM_ASF_LOW_LIGHT,
  C2D_PARAM_ASF_LOW_LIGHT_INTERPOLATE,
  C2D_PARAM_ASF_NORMAL_LIGHT,
  C2D_PARAM_ASF_BRIGHT_LIGHT_INTERPOLATE,
  C2D_PARAM_ASF_BRIGHT_LIGHT,
  C2D_PARAM_ASF_MAX_LIGHT
} c2d_params_asf_region_t;

typedef struct _c2d_params_aec_trigger_info_t {
  float lux_idx;
  float gain;
} c2d_params_aec_trigger_info_t;

typedef struct _c2d_hardware_params_t {
  /* TODO: Remove frameid, identity and timestamp */
  uint32_t frame_id;
  uint32_t identity;
  struct timeval timestamp;
  //c2d_params_asf_mode_t     asf_mode;
  //c2d_params_asf_info_t     asf_info;
  c2d_params_scale_info_t   scale_info;
  uint32_t                  rotation;
  uint32_t                  mirror;
  c2d_params_crop_info_t    crop_info;
  c2d_params_dim_info_t     input_info;
  c2d_params_dim_info_t     output_info;
  //boolean                   denoise_enable;
  c2d_params_denoise_info_t
    denoise_info[C2D_DENOISE_NUM_PLANES][C2D_DENOISE_NUM_PROFILES];
  void                     *cookie;
  c2d_hardware_buffer_info_t input_buffer_info;
  boolean                   processed_divert;
} c2d_hardware_params_t;

#endif
