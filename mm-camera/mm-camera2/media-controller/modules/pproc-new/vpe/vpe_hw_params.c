/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "mtype.h"
#include "vpe_log.h"
#include "vpe_hw_params.h"
#include "vpe_util.h"
#include "vpe_reg_pack.h"
#include "vpe_proc.h"

static const mm_vpe_out_format_packed vpe_default_out_format_packed = {
  0x3,                       //dstc0_bits          :   2;
  0x3,                       //dstc1_bits          :   2;
  0x3,                       //dstc2_bits          :   2;
  0,                         //dstc3_bits          :   2;
  0,                         //dstc3_en            :   1;
  1,                         //pack_count          :   4;
  1,                         //pack_tight          :   1;
  0,                         //pack_align          :   1;
  0,                         //reserved_bit15      :   1;
  1,                         //dst_bpp             :   2;
  0x2,                       //write_planes        :   2;
  0,                         //comp_en             :   1;
  0,                         //comp_mode           :   2;
  // 0,                      //reserved            :   9;
};

static const mm_vpe_src_format_packed vpe_default_src_format_packed = {
  0x3,                       //srcc0_bits                  :   2;
  0x3,                       //srcc1_bits                  :   2;
  0x3,                       //srcc2_bits                  :   2;
  0,                         //srcc3_bits                  :   2;
  0,                         //srcc3_en                    :   1;
  1,                         //src_bpp                     :   2;
  //0,                       //reserved                    :   2;
  1,                         //unpack_count                :   4;
  1,                         //unpack_tight                :   1;
  0,                         //unpack_align                :   1;
  0x2,                       //fetch_planes                :   2;
  0,                         //wmv9_mode                   :   1;
  //0,                       //reserved                    :  10;
};

static void mm_vpe_set_input_plane(mm_vpe_surface_attr_type *pinput_plane,
  mm_vpe_input_plane_config_type *phw_cfg)
{
  /* for NV12 & NV21, stride_0 = src image width.  one byte per pixel.*/
  switch (pinput_plane->color_format) {
    case MM_COLOR_FORMAT_Y_CBCR_NV21_COSITE:
    case MM_COLOR_FORMAT_Y_CBCR_NV21_OFFSITE:
      phw_cfg->src_format = vpe_default_src_format_packed;//0x12223f;
      phw_cfg->src_unpack_pattern1 = 0x102;
      phw_cfg->src_buffer_size.src_img_w =
        pinput_plane->buffer_dimension.stride_0;
      break;

    case MM_COLOR_FORMAT_Y_CRCB_NV12_COSITE:
    case MM_COLOR_FORMAT_Y_CRCB_NV12_OFFSITE:
      phw_cfg->src_format = vpe_default_src_format_packed;//0x12223f;
      phw_cfg->src_unpack_pattern1 = 0x201;
      phw_cfg->src_buffer_size.src_img_w =
        pinput_plane->buffer_dimension.stride_0;
      break;
  }

  phw_cfg->src_buffer_size.src_img_h =
    pinput_plane->frame_dimension.frame_height; // buffer height;

  phw_cfg->src_ystride1.srcp0_ystride =
    pinput_plane->buffer_dimension.stride_0; // y stride

  phw_cfg->src_ystride1.srcp1_ystride =
    pinput_plane->buffer_dimension.stride_1; // cbcr stride

  phw_cfg->src_roi_size.src_h =
    pinput_plane->frame_dimension.frame_height;  // ROI height

  phw_cfg->src_roi_size.src_w =
    pinput_plane->frame_dimension.frame_width;  // ROI width

  phw_cfg->src_roi_offset.src_x =
    pinput_plane->frame_dimension.frame_offset_x;  //ROI x

  phw_cfg->src_roi_offset.src_y =
    pinput_plane->frame_dimension.frame_offset_y;  //ROI y

  CDBG_LOW("%s: src_unpack_pattern1=0x%x,src_img_w=0x%x,src_img_h=0x%x"
       "srcp0_ystride=0x%x,srcp1_ystride=0x%x,roi.src_x=0x%x,roi.src_y=0x%x,"
       "roi.src_w=0x%x,roi.src_h=0x%x", __func__,
       phw_cfg->src_unpack_pattern1,
       phw_cfg->src_buffer_size.src_img_w, phw_cfg->src_buffer_size.src_img_h,
       phw_cfg->src_ystride1.srcp0_ystride, phw_cfg->src_ystride1.srcp1_ystride,
       phw_cfg->src_roi_offset.src_x, phw_cfg->src_roi_offset.src_y,
       phw_cfg->src_roi_size.src_w, phw_cfg->src_roi_size.src_h);
} /* vpe_set_input_plane() */

static void mm_vpe_set_output_plane(mm_vpe_surface_attr_type *poutput_plane,
  mm_vpe_output_plane_config_type *phw_cfg) {

  switch (poutput_plane->color_format) {
    case MM_COLOR_FORMAT_Y_CBCR_NV21_COSITE:
    case MM_COLOR_FORMAT_Y_CBCR_NV21_OFFSITE:
      phw_cfg->out_format = vpe_default_out_format_packed; //0x9223f;
      phw_cfg->out_pack_pattern1 = 0x102;
      break;

    case MM_COLOR_FORMAT_Y_CRCB_NV12_COSITE:
    case MM_COLOR_FORMAT_Y_CRCB_NV12_OFFSITE:
      phw_cfg->out_format = vpe_default_out_format_packed;//0x9223f;
      phw_cfg->out_pack_pattern1 = 0x201;
      break;
  }

  phw_cfg->out_ystride1.dstp0_ystride =
    poutput_plane->buffer_dimension.stride_0;

  phw_cfg->out_ystride1.dstp1_ystride =
    poutput_plane->buffer_dimension.stride_1;

  /* typically,  ROI offset at output side = 0 */
  phw_cfg->out_roi_offset.dst_x =
    poutput_plane->frame_dimension.frame_offset_x;

  phw_cfg->out_roi_offset.dst_y =
    poutput_plane->frame_dimension.frame_offset_y;

  phw_cfg->out_roi_size.dst_h =
    poutput_plane->frame_dimension.frame_height;

  phw_cfg->out_roi_size.dst_w =
    poutput_plane->frame_dimension.frame_width;

  CDBG_LOW("%s: out_pack_pattern1=0x%x"
       "dstp0_ystride=0x%x,dstp1_ystride=0x%x,roi.dst_x=0x%x,roi.dst_y=0x%x,"
       "roi.dst_w=0x%x,roi.dst_h=0x%x", __func__,
       phw_cfg->out_pack_pattern1,
       phw_cfg->out_ystride1.dstp0_ystride, phw_cfg->out_ystride1.dstp1_ystride,
       phw_cfg->out_roi_offset.dst_x, phw_cfg->out_roi_offset.dst_y,
       phw_cfg->out_roi_size.dst_w, phw_cfg->out_roi_size.dst_h);
} /* VPE_SetupOutputPlane() */

static const mm_vpe_op_mode_packed vpe_default_op_mode_packed = {
  0,           //scalex_en               :   1;
  0,           //scaley_en               :   1;
  1,           //src_data_format         :   1;
  0,           //convert_matrix_en       :   1;
  0,           //convert_matrix_sel      :   1;
  0,           //lut_c0_en               :   1;
  0,           //lut_c1_en               :   1;
  0,           //lut_c2_en               :   1;
  0,           //rot_en                  :   1;
  0,           //rot_mode                :   3;
  0,           //blend_en                :   1;
  0,           //blend_alpha_sel         :   2;
  0,           //blend_eq_sel            :   1;
  0,           //dither_en               :   1;
  //0,         //reserved                :   1;
  0x3,         //src_chroma_samp         :   2;
  1,           //src_chroma_site         :   1;
  0x3,         //dst_chroma_samp         :   2;
  1,           //dst_chroma_site         :   1;
  0,           //blend_transp_en         :   1;
  0,           //bg_chroma_samp          :   2;
  0,           //bg_chroma_site          :   1;
  0,           //deint_en                :   1;
  0,           //deint_odd_ref           :   1;
  1,           //dst_data_format         :   1;
  //0,         //reserved                :   1;
};


static MM_COLOR_FORMAT_ENUM vpe_get_fmt(vpe_plane_fmt in_fmt)
{
  switch(in_fmt) {
    case VPE_PLANE_CRCB:
      return MM_COLOR_FORMAT_Y_CRCB_NV12_OFFSITE;
    case VPE_PLANE_CBCR:
      return MM_COLOR_FORMAT_Y_CBCR_NV21_OFFSITE;
    default:
      CDBG_HIGH("%s: Error Other fmts are not supported yet.", __func__);
      return MM_COLOR_FORMAT_Y_CRCB_NV12_OFFSITE;
  }
}

void vpe_config_pipeline(struct vpe_transaction_setup_t *setup,
                         struct vpe_frame_info_t *frame_info,
                         struct msm_vpe_frame_info_t *msm_frame_info)
{
  uint32_t stats_in_w, stats_in_h, margin_x, margin_y;
  uint32_t vpe_output_w = 0, vpe_output_h = 0;
  uint32_t vpe_output_stride0 = 0, vpe_output_stride1 = 0;
  mm_vpe_surface_attr_type input_plane, output_plane;
  struct vpe_plane_info_t *yplane_info = &frame_info->plane_info[0];
  struct vpe_plane_info_t *cbcrplane_info = &frame_info->plane_info[1];
  setup->hw_op_mode = vpe_default_op_mode_packed; //0x40FC0004

  memset(&setup->input_hw_cfg, 0, sizeof(mm_vpe_input_plane_config_type));
  memset(&setup->output_hw_cfg, 0, sizeof(mm_vpe_output_plane_config_type));

  memset(&input_plane, 0, sizeof(mm_vpe_surface_attr_type));
  memset(&output_plane, 0, sizeof(mm_vpe_surface_attr_type));

  memset(&setup->scaler_cfg_0, 0, sizeof(mm_vpe_scale_coef_cfg_type));
  memset(&setup->scaler_cfg_1, 0, sizeof(mm_vpe_scale_coef_cfg_type));
  memset(&setup->scaler_cfg_2, 0, sizeof(mm_vpe_scale_coef_cfg_type));
  memset(&setup->scaler_cfg_3, 0, sizeof(mm_vpe_scale_coef_cfg_type));

  /* fill the information for input */
  input_plane.color_format = vpe_get_fmt(frame_info->in_plane_fmt);
  input_plane.frame_dimension.frame_offset_x
    = msm_frame_info->strip_info.src_x;  // default 0
  input_plane.frame_dimension.frame_offset_y
    = msm_frame_info->strip_info.src_y;  // default 0

  CDBG_LOW("%s: yplane: src_width=%d,src_height=%d,src_stride=%d; "
	   "cbcrplane: src_stride=%d\n",
	   __func__, yplane_info->src_width, yplane_info->src_height,
	   yplane_info->src_stride, cbcrplane_info->src_stride);
  /* double check on the unit. Is it number of pixel ? */
  input_plane.frame_dimension.frame_width = yplane_info->src_width;
  input_plane.frame_dimension.frame_height = yplane_info->src_height;

  /* buffer width is the same as frame width */
  input_plane.buffer_dimension.stride_0 = yplane_info->src_stride;
  input_plane.buffer_dimension.stride_1 = cbcrplane_info->src_stride;

  /* set up the input plane */
  mm_vpe_set_input_plane(&input_plane, &setup->input_hw_cfg);

  /* note: mm_vpe_set_input_plane set the default width and height already. */
  /* TODO: FIGURE OUT THIS ROI STUFF */
  /* if(cfg->input_roi.width) */
  /*   setup->input_hw_cfg.src_roi_size.src_w = cfg->input_roi.width; */

  /* if(cfg->input_roi.height) */
  /*   setup->input_hw_cfg.src_roi_size.src_h = cfg->input_roi.height; */

  CDBG_LOW("%s: vpe input pipeline cfg: width=%d,height=%d,stride0=%d,"
       "stride1=%d,off_x=%d,off_y=%d,roiw=%d,roih=%d",
       __func__, input_plane.frame_dimension.frame_width,
       input_plane.frame_dimension.frame_height,
       input_plane.buffer_dimension.stride_0,
       input_plane.buffer_dimension.stride_1,
       input_plane.frame_dimension.frame_offset_x,
       input_plane.frame_dimension.frame_offset_y,
       setup->input_hw_cfg.src_roi_size.src_w,
       setup->input_hw_cfg.src_roi_size.src_h);

  /* src * dest  chroma offsite, 4:2:0, */
  /* Modify vpe output based on rotation and dis */
  switch (yplane_info->rotate) {
    case ROT_NONE:
      vpe_output_w = yplane_info->dst_width;
      vpe_output_h = yplane_info->dst_height;
      vpe_output_stride0 = yplane_info->dst_stride;
      vpe_output_stride1 = cbcrplane_info->dst_stride;
      break;
    case ROT_CLOCKWISE_90:
    case ROT_CLOCKWISE_270:
      setup->hw_op_mode.rot_en = 1;
      vpe_output_w = yplane_info->dst_height; /* swap dimension */
      vpe_output_h = yplane_info->dst_width;
      vpe_output_stride0 = yplane_info->dst_height;
      vpe_output_stride1 = yplane_info->dst_height;
      break;
    case ROT_CLOCKWISE_180:
      setup->hw_op_mode.rot_en = 1;
      vpe_output_w = yplane_info->dst_width;
      vpe_output_h = yplane_info->dst_height;
      vpe_output_stride0 = yplane_info->dst_stride;
      vpe_output_stride1 = cbcrplane_info->dst_stride;
      break;
    default:
      CDBG_HIGH("Invalid Rotation value.\n");
      break;
  }
  setup->hw_op_mode.rot_mode = yplane_info->rotate;

  /* set up the output plane. */
  /* fill the information for input */
  output_plane.color_format = vpe_get_fmt(frame_info->out_plane_fmt);
  output_plane.frame_dimension.frame_offset_x = 0;
  output_plane.frame_dimension.frame_offset_y = 0;
  /* add the src_chroma_site info */
  setup->hw_op_mode.src_chroma_site = vpe_get_fmt(frame_info->in_plane_fmt);

  /* a note on dimension swap:  better to have HAL layer to do
     the swap. */
  /* double check on the unit. Is it number of pixel ? */
  output_plane.frame_dimension.frame_width = vpe_output_w;
  output_plane.frame_dimension.frame_height = vpe_output_h;

  /* buffer width is the same as frame width */
  output_plane.buffer_dimension.stride_0 = vpe_output_stride0;
  output_plane.buffer_dimension.stride_1 = vpe_output_stride1;

  CDBG_LOW("%s: vpe output pipeline cfg: width=%d,height=%d,stride0=%d,"
       "stride1=%d,off_x=%d,off_y=%d, rotation_enum=%d",
       __func__, output_plane.frame_dimension.frame_width,
       output_plane.frame_dimension.frame_height,
       output_plane.buffer_dimension.stride_0,
       output_plane.buffer_dimension.stride_1,
       output_plane.frame_dimension.frame_offset_x,
       output_plane.frame_dimension.frame_offset_y, setup->hw_op_mode.rot_mode);
  mm_vpe_set_output_plane(&output_plane, &setup->output_hw_cfg);

  mm_vpe_init_scale_table(&setup->scaler_cfg_0, 0);
  mm_vpe_init_scale_table(&setup->scaler_cfg_1, 1);
  mm_vpe_init_scale_table(&setup->scaler_cfg_2, 2);
  mm_vpe_init_scale_table(&setup->scaler_cfg_3, 3);
}

void vpe_params_prepare_frame_info(struct vpe_frame_info_t *frame_info,
  struct msm_vpe_frame_info_t *out_info)
{
  out_info->frame_id = frame_info->frame_id;
  out_info->timestamp = frame_info->timestamp;
  out_info->src_fd = frame_info->plane_info[0].src_fd;
  out_info->dst_fd = frame_info->plane_info[0].dst_fd;
  out_info->input_buffer_info.fd = frame_info->plane_info[0].src_fd;
  out_info->input_buffer_info.index = frame_info->buff_index;
  out_info->input_buffer_info.native_buff = frame_info->native_buff;
  out_info->identity = frame_info->identity;
  out_info->cookie = frame_info->cookie;

  out_info->strip_info.src_w = frame_info->plane_info[0].src_width;
  out_info->strip_info.src_h = frame_info->plane_info[0].src_height;
  out_info->strip_info.dst_w = frame_info->plane_info[0].dst_width;
  out_info->strip_info.dst_h = frame_info->plane_info[0].dst_height;
  out_info->strip_info.src_x = 0;
  out_info->strip_info.src_y = 0;
  out_info->strip_info.phase_step_x = 0;
  out_info->strip_info.phase_step_y = 0;
  out_info->strip_info.phase_init_x = 0;
  out_info->strip_info.phase_init_y = 0;

  out_info->src_chroma_plane_offset
      = frame_info->plane_info[1].source_address;
  out_info->dest_chroma_plane_offset
      = frame_info->plane_info[1].destination_address;
}

/* currently hardcodes some values */
void vpe_params_create_frame_info(vpe_hardware_params_t *hw_params,
  struct vpe_frame_info_t *frame_info)
{
  int i;
  struct vpe_plane_info_t *plane_info = frame_info->plane_info;

  for(i = 0; i < 2; i++) {
    memset(&plane_info[i], 0, sizeof(struct vpe_plane_info_t));
      plane_info[i].rotate = 0; //hard
      plane_info[i].mirror = hw_params->mirror;
      plane_info[i].h_scale_ratio =
        (double)hw_params->crop_info.process_window_width /
          hw_params->output_info.width;
      plane_info[i].v_scale_ratio =
        (double)hw_params->crop_info.process_window_height /
          hw_params->output_info.height;
      plane_info[i].h_scale_initial_phase =
        hw_params->crop_info.process_window_first_pixel;
      plane_info[i].v_scale_initial_phase =
        hw_params->crop_info.process_window_first_line;
      plane_info[i].src_width = hw_params->input_info.width;
      plane_info[i].src_height = hw_params->input_info.height;
      plane_info[i].src_stride = hw_params->input_info.stride;
      plane_info[i].dst_width = hw_params->output_info.width;
      plane_info[i].dst_height = hw_params->output_info.height;
      plane_info[i].prescale_padding = 22;//org
      plane_info[i].postscale_padding = 4;//org
      /* plane_info[i].bf_enable = hw_params->denoise_enable; */

      if (plane_info[i].rotate == 0 || plane_info[i].rotate == 2) {
        plane_info[i].dst_stride = hw_params->output_info.stride;
        plane_info[i].maximum_dst_stripe_height =
          PAD_TO_2(hw_params->output_info.scanline);
      } else {
        plane_info[i].dst_stride =
          PAD_TO_32(hw_params->output_info.scanline/2) * 2;
        plane_info[i].maximum_dst_stripe_height =
          hw_params->output_info.stride;
      }
  }

  plane_info[0].input_plane_fmt = VPE_PLANE_Y;
  plane_info[1].src_width /= 2;
  plane_info[1].src_height /= 2;
  plane_info[1].dst_width /= 2;
  plane_info[1].dst_height /= 2;
  plane_info[1].h_scale_initial_phase /= 2;
  plane_info[1].v_scale_initial_phase /= 2;
  plane_info[1].maximum_dst_stripe_height =
    PAD_TO_2(plane_info[1].maximum_dst_stripe_height / 2);
  plane_info[1].postscale_padding = 0;
  plane_info[1].input_plane_fmt = VPE_PLANE_CBCR;
  plane_info[1].source_address =
    plane_info[0].src_stride * hw_params->input_info.scanline;
  plane_info[1].destination_address =
  plane_info[0].dst_stride * plane_info[0].maximum_dst_stripe_height;

  frame_info->num_planes = 2;
  frame_info->in_plane_fmt = VPE_PLANE_CBCR;
  frame_info->out_plane_fmt = VPE_PLANE_CBCR;
  if (VPE_PARAM_PLANE_CRCB == hw_params->input_info.plane_fmt) {
    frame_info->in_plane_fmt = VPE_PLANE_CRCB;
  }
  if (hw_params->input_info.plane_fmt != hw_params->output_info.plane_fmt) {
    frame_info->out_plane_fmt = VPE_PLANE_CRCB;
  }

  return;
}

boolean vpe_hardware_validate_params(vpe_hardware_params_t *hw_params)
{
  CDBG_LOW("%s:%d, inw=%d, inh=%d, outw=%d, outh=%d", __func__, __LINE__,
    hw_params->input_info.width, hw_params->input_info.height,
    hw_params->output_info.width, hw_params->output_info.height);
  CDBG_LOW("%s:%d, inst=%d, insc=%d, outst=%d, outsc=%d", __func__, __LINE__,
    hw_params->input_info.stride, hw_params->input_info.scanline,
    hw_params->output_info.stride, hw_params->output_info.scanline);

  if (hw_params->input_info.width <= 0 || hw_params->input_info.height <= 0) {
    CDBG_ERROR("%s:%d, invalid input dim: %d x %d", __func__, __LINE__,
               hw_params->input_info.width, hw_params->input_info.height);
    return FALSE;
  }
  /* TODO: add mode sanity checks */
  return TRUE;
}
