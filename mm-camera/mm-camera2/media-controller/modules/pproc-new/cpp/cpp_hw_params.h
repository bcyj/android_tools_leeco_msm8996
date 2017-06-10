
/*============================================================================

  Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#ifndef CPP_HW_PARAMS_H
#define CPP_HW_PARAMS_H

#include <media/msmb_camera.h>
#include <media/msmb_pproc.h>
#include "cam_types.h"
#include "mtype.h"
#include "modules.h"
#include "chromatix.h"
#include "chromatix_common.h"

#define PAD_TO_32(a)               (((a)+31)&~31)
#define PAD_TO_2(a)               (((a)+1)&~1)

#define CPP_PARAM_ASF_F_KERNEL_MAX          16
#define CPP_PARAM_ASF_LUT_MAX               24
#define CPP_PARAM_ASF_LUT3_MAX              12

#define ROTATION_BUFFER_SIZE 16384
#define LINE_BUFFER_SIZE 512
#define MAL_SIZE 32

#define CPP_FW_VERSION_1_2_0  0x10020000
#define CPP_FW_VERSION_1_4_0  0x10040000
#define CPP_FW_VERSION_1_6_0  0x10060000

#define CPP_GET_FW_MAJOR_VERSION(ver) ((ver >> 28) & 0xf)
#define CPP_GET_FW_MINOR_VERSION(ver) ((ver >> 16) & 0xfff)
#define CPP_GET_FW_STEP_VERSION(ver)  (ver & 0xffff)

#define LINEAR_INTERPOLATE(factor, reference_i, reference_iplus1) \
  ((factor * reference_iplus1) + ((1 - factor) * reference_i))

/* forward declaration of cpp_hardware_t structures */
typedef struct _cpp_hardware_t cpp_hardware_t;

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

typedef struct {
  int fd;
  uint32_t index;
  uint32_t offset;
  uint8_t native_buff;
  uint8_t processed_divert;
  uint32_t  identity;
} cpp_hardware_buffer_info_t;

struct cpp_asf_info {
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
  uint32_t nz_flag_f2;
  uint32_t nz_flag_f3_f5;
  float sobel_h_coeff[CPP_PARAM_ASF_F_KERNEL_MAX];
  float sobel_v_coeff[CPP_PARAM_ASF_F_KERNEL_MAX];
  float hpf_h_coeff[CPP_PARAM_ASF_F_KERNEL_MAX];
  float hpf_v_coeff[CPP_PARAM_ASF_F_KERNEL_MAX];
  float lpf_coeff[CPP_PARAM_ASF_F_KERNEL_MAX];
  float lut1[CPP_PARAM_ASF_LUT_MAX];
  float lut2[CPP_PARAM_ASF_LUT_MAX];
  float lut3[CPP_PARAM_ASF_LUT3_MAX];
  uint8_t checksum_en;
};

struct cpp_bf_info {
  double bilateral_scale[4];
  double noise_threshold[5];
  double weight[5];
  double clamp_limit_UL[3];
  double clamp_limit_LL[3];
};

typedef struct _cpp_tnr_info_t {
  double bilateral_scale[4];
  double noise_threshold[5];
  double weight[5];
} cpp_tnr_info_t;

typedef struct _cpp_prescaler_bf_info_t {
  double bilateral_scale;
  double noise_threshold;
  double weight;
} cpp_prescaler_bf_info_t;

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
  struct cpp_fe_info fe_r_info; /* TNR scratch buffer */
  struct cpp_crop_info bf_crop_info;
  struct cpp_crop_info prescaler_bf_crop_info;
  struct cpp_crop_info temporal_bf_crop_info;
  struct cpp_stripe_scale_info scale_info;
  struct cpp_crop_info asf_crop_info;
  struct cpp_crop_info next_state_bf_crop_info;
  struct cpp_rotator_info rot_info;
  struct cpp_we_info we_info;
  struct cpp_we_info we_r_info; /* TNR scratch buffer */
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
  int scanline;
  uint32_t temporal_stride;
  int rotate;
  int mirror;
  int prescale_padding;
  int postscale_padding;
  uint32_t state_padding;
  uint32_t spatial_denoise_padding;
  uint32_t prescaler_spatial_denoise_padding;
  uint32_t temporal_denoise_padding;
  uint32_t sharpen_padding;
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
  cpp_plane_fmt output_plane_fmt;
  /*  the following two are needed to support color conversion
   *  from CbCr to Cb, Cr or vise versa.
   *  they have the same value as in msm_cpp_frame_strip_info
   */
  uint32_t input_bytes_per_pixel;
  uint32_t output_bytes_per_pixel;
  uint32_t temporal_bytes_per_pixel;
  /*  address[2] only needed for color conversion
   * from CbCr to Cb, Cr or vise versa
   */
  uint32_t source_address[2];
  uint32_t destination_address[2];
  uint32_t temporal_source_address[2];
  uint32_t temporal_destination_address[2];

  /*derived*/
  uint32_t is_not_y_plane;
  uint32_t denoise_after_scale_en; /* 1 -> TNR is on, 0 -> TNR is off */
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

  uint32_t prescaler_crop_enable;
  uint32_t tnr_crop_enable;
  uint32_t bf_crop_enable;
  uint32_t asf_crop_enable;
  uint32_t next_state_crop_enable;
  uint32_t bf_enable;
  uint32_t tnr_enable;
  struct cpp_plane_scale_info scale_info;
  struct cpp_stripe_info *stripe_info1;
  struct msm_cpp_frame_strip_info *stripe_info;
};

struct cpp_frame_info_t {
  int32_t frame_id;
  struct timeval timestamp;
  int32_t processed_divert;
  uint32_t identity;
  boolean dup_output;
  uint32_t dup_identity;
  uint32_t buff_index;
  uint32_t native_buff;
  uint32_t in_buff_identity;
  void    *cookie;
  cpp_hardware_buffer_info_t out_buff_info;
  enum msm_cpp_frame_type frame_type;
  cpp_asf_mode asf_mode;
  struct cpp_asf_info asf_info;
  struct cpp_bf_info bf_info[3];
  cpp_tnr_info_t     tnr_info[3];
  cpp_prescaler_bf_info_t prescaler_info[3];
  struct cpp_plane_info_t plane_info[MAX_PLANES];
  int num_planes;
  cpp_plane_fmt out_plane_fmt;
  cpp_plane_fmt in_plane_fmt;
  double noise_profile[3][5];
  double weight[3][5];
  double denoise_ratio[3][5];
  double edge_softness[3][4];
  double sharpness_ratio;
};

/* ---------------------------- new params --------------------------------- */

#define CPP_DENOISE_NUM_PLANES    3
#define CPP_DENOISE_NUM_PROFILES  4

typedef enum {
  CPP_PARAM_ASF_OFF,
  CPP_PARAM_ASF_DUAL_FILTER,
  CPP_PARAM_ASF_EMBOSS,
  CPP_PARAM_ASF_SKETCH,
  CPP_PARAM_ASF_NEON,
} cpp_params_asf_mode_t;

typedef struct _cpp_params_scale_info_t {
  double h_scale_ratio;
  double v_scale_ratio;
} cpp_params_scale_info_t;

typedef struct _cpp_params_denoise_info_t {
  double noise_profile;
  double weight;
  double denoise_ratio;
  double edge_softness;
} cpp_params_denoise_info_t;

typedef struct _cpp_params_crop_window_t {
  uint32_t x, y, dx, dy;
} cpp_params_crop_window_t;

typedef struct _cpp_params_crop_info_t {
  cpp_params_crop_window_t stream_crop;
  cpp_params_crop_window_t is_crop;
  uint32_t process_window_first_pixel;
  uint32_t process_window_first_line;
  uint32_t process_window_width;
  uint32_t process_window_height;
} cpp_params_crop_info_t;

typedef enum {
  CPP_PARAM_PLANE_CBCR,
  CPP_PARAM_PLANE_CRCB,
  CPP_PARAM_PLANE_Y,
  CPP_PARAM_PLANE_CB,
  CPP_PARAM_PLANE_CR,
  CPP_PARAM_PLANE_CBCR422,
  CPP_PARAM_PLANE_CRCB422,
  CPP_PARAM_PLANE_CRCB420
} cpp_params_plane_fmt_t;

typedef struct {
  uint32_t identity;
  uint32_t num_buffs;
  cpp_hardware_buffer_info_t *buffer_info;
} cpp_hardware_stream_buff_info_t;

typedef struct _cpp_params_dim_info_t {
  int32_t                width;
  int32_t                height;
  int32_t                stride;
  int32_t                scanline;
  cpp_params_plane_fmt_t  plane_fmt;
  int32_t                 plane_offsets[3];
} cpp_params_dim_info_t;

typedef struct _cpp_params_asf_info_t {
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
  uint32_t nz_flag_f2;
  uint32_t nz_flag_f3_f5;
  float sobel_h_coeff[CPP_PARAM_ASF_F_KERNEL_MAX];
  float sobel_v_coeff[CPP_PARAM_ASF_F_KERNEL_MAX];
  float hpf_h_coeff[CPP_PARAM_ASF_F_KERNEL_MAX];
  float hpf_v_coeff[CPP_PARAM_ASF_F_KERNEL_MAX];
  float lpf_coeff[CPP_PARAM_ASF_F_KERNEL_MAX];
  float lut1[CPP_PARAM_ASF_LUT_MAX];
  float lut2[CPP_PARAM_ASF_LUT_MAX];
  float lut3[CPP_PARAM_ASF_LUT3_MAX];
  float sharp_min_ds_factor;
  float sharp_max_ds_factor;
  float sharp_max_factor;
  uint8_t checksum_enable;
} cpp_params_asf_info_t;

typedef struct _cpp_params_aec_trigger_params {
  float lowlight_trigger_start;
  float lowlight_trigger_end;
  float brightlight_trigger_start;
  float brightlight_trigger_end;
  float aec_trigger_input;
} cpp_params_aec_trigger_t;

typedef enum _cpp_params_asf_region {
  CPP_PARAM_ASF_LOW_LIGHT,
  CPP_PARAM_ASF_LOW_LIGHT_INTERPOLATE,
  CPP_PARAM_ASF_NORMAL_LIGHT,
  CPP_PARAM_ASF_BRIGHT_LIGHT_INTERPOLATE,
  CPP_PARAM_ASF_BRIGHT_LIGHT,
  CPP_PARAM_ASF_MAX_LIGHT
} cpp_params_asf_region_t;

typedef struct _cpp_params_aec_trigger_info_t {
  float lux_idx;
  float gain;
} cpp_params_aec_trigger_info_t;

typedef struct _cpp_hardware_params_t {
  /* TODO: Remove frameid, identity and timestamp */
  uint32_t                  frame_id;
  uint32_t                  identity;
  struct timeval            timestamp;
  boolean                   duplicate_output;
  uint32_t                  duplicate_identity;
  boolean                   processed_divert;
  double                    sharpness_level;
  cpp_params_asf_mode_t     asf_mode;
  cpp_params_asf_info_t     asf_info;
  cpp_params_scale_info_t   scale_info;
  uint32_t                  rotation;
  uint32_t                  mirror;
  cpp_params_crop_info_t    crop_info;
  cpp_params_dim_info_t     input_info;
  cpp_params_dim_info_t     output_info;
  boolean                   denoise_enable;
  boolean                   tnr_enable;
  boolean                   expect_divert;
  boolean                   uv_upsample_enable;
  boolean                   diagnostic_enable;
  boolean                   denoise_lock;
  boolean                   tnr_denoise_lock;
  boolean                   asf_lock;
  boolean                   ez_tune_wnr_enable;
  boolean                   ez_tune_asf_enable;
  boolean                   scene_mode_on;
  cpp_tnr_info_t            tnr_info_Y;
  cpp_tnr_info_t            tnr_info_Cb;
  cpp_tnr_info_t            tnr_info_Cr;
  cpp_params_denoise_info_t
    denoise_info[CPP_DENOISE_NUM_PLANES][CPP_DENOISE_NUM_PROFILES+1];
  void                     *cookie;
  cpp_hardware_buffer_info_t buffer_info;
  cpp_params_aec_trigger_info_t aec_trigger;
  sensor_dim_output_t       sensor_dim_info;
  /* Current diagnostics */
  asfsharpness7x7_t          asf_diag;
  wavelet_t                  wnr_diag;
  cam_stream_type_t          stream_type;
  uint32_t                  isp_width_map;
  uint32_t                  isp_height_map;
  cpp_hardware_buffer_info_t output_buffer_info;
} cpp_hardware_params_t;

void cpp_params_prepare_frame_info(cpp_hardware_t *cpphw,
  struct cpp_frame_info_t *frame_info, struct msm_cpp_frame_info_t *out_info);
void cpp_params_create_frame_info(cpp_hardware_params_t* hw_params,
  struct cpp_frame_info_t *frame_info);
boolean cpp_hardware_validate_params(cpp_hardware_params_t *hw_params);
boolean cpp_hardware_rotation_swap(cpp_hardware_params_t *hw_params);
int32_t cpp_hw_params_update_wnr_params(chromatix_parms_type *chromatix_ptr,
  cpp_hardware_params_t *hw_params,
  cpp_params_aec_trigger_info_t *aec_trigger);
int32_t cpp_hw_params_interpolate_wnr_params(float interpolation_factor,
  cpp_hardware_params_t *cpp_hw_params,
  ReferenceNoiseProfile_type *ref_noise_profile_i,
  ReferenceNoiseProfile_type *ref_noise_profile_iplus1);
int32_t cpp_hw_params_asf_interpolate(cpp_hardware_t *cpphw,
  cpp_hardware_params_t *hw_params,
  chromatix_parms_type  *chromatix_ptr,
  cpp_params_aec_trigger_info_t *trigger_input);
uint8_t cpp_hardware_fw_version_1_2_x(cpp_hardware_t *cpphw);
void cpp_hw_params_copy_asf_diag_params(struct cpp_frame_info_t *frame_info,
  asfsharpness7x7_t *diag_cpyto_params);
void cpp_hw_params_copy_wnr_diag_params(struct cpp_frame_info_t *frame_info,
  wavelet_t *diag_cpyto_params);

#endif
