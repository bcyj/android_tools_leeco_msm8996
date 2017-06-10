/*============================================================================
   Copyright (c) 2011-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#include <string.h>
#include <sys/time.h>
#include <linux/media.h>
#include "camera_dbg.h"
#include "vpe_api.h"
#include "vpe_util.h"
#include "vpe_proc.h"
#include "mctl_pp.h"

#if 0
#undef CDBG
#define CDBG LOGE
#endif

static uint32_t vpe_open_count;
static vpe_ctrl_type_t my_vpe_obj;

const mm_vpe_out_format_packed vpe_default_out_format_packed = {
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

const mm_vpe_src_format_packed vpe_default_src_format_packed = {
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

static vpe_pp_node_t *vpe_get_free_node(uint32_t handle)
{
  int i;
  for (i = 0; i < VPE_FREE_NODE_MAX_NUM; i++) {
    if (my_vpe_obj.free_node[i].state == VPE_FRAME_NULL) {
      my_vpe_obj.free_node[i].state = VPE_FRAME_QUEUED;
      return &my_vpe_obj.free_node[i];
    }
  }
  return NULL;
}

static void vpe_put_free_node(uint32_t handle, vpe_pp_node_t *node)
{
  memset(node, 0, sizeof(vpe_pp_node_t));
}

static void vpe_time_profile_begin()
{
  clock_gettime(CLOCK_REALTIME, &my_vpe_obj.in_t);
  CDBG("%s: profiling begin at (sec:msec) = %ld:%ld\n", __func__,
    my_vpe_obj.in_t.tv_sec, (my_vpe_obj.in_t.tv_nsec/1000000));
}

static void vpe_time_profile_end()
{
  struct timespec t_diff;
  clock_gettime(CLOCK_REALTIME, &my_vpe_obj.out_t);
  if (my_vpe_obj.out_t.tv_nsec > my_vpe_obj.in_t.tv_nsec) {
    t_diff.tv_nsec = my_vpe_obj.out_t.tv_nsec - my_vpe_obj.in_t.tv_nsec;
    t_diff.tv_sec = my_vpe_obj.out_t.tv_sec - my_vpe_obj.in_t.tv_sec;
  } else {
    t_diff.tv_nsec = 1000000000 +
      (my_vpe_obj.out_t.tv_nsec - my_vpe_obj.in_t.tv_nsec);
    t_diff.tv_sec = my_vpe_obj.out_t.tv_sec - my_vpe_obj.in_t.tv_sec - 1;
  }
  CDBG("%s: profiling end at (sec:msec) = %ld:%ld, delta = %ld:%ld",
       __func__, my_vpe_obj.in_t.tv_sec, my_vpe_obj.in_t.tv_nsec/1000000,
       t_diff.tv_sec, t_diff.tv_nsec/1000000);
}

/*==========================================================================
     FUNCTION:       vpe_set_input_plane
     DESCRIPTION:
                     This function setup foreground plane

@par REGISTERS IMPACTED:

      [internal] Set up SRC ROI related registers
                     VPE_SRC_SIZE:       Width and height of source ROI
                     VPE_SRC_IMAGE_SIZE: Width and height of
                                             source frame in pixels
                     VPE_SRCP0_ADDR:     Byte address of the source Y plane
                     VPE_SRCP1_ADDR:     Byte address of the source CrCb
                                             plane
                     VPE_SRC_YSTRIDE1:   Line strides in bytes for Y and
                                             CbCr planes

@par SIDE EFFECTS:
                     None.
==========================================================================*/
void mm_vpe_set_input_plane(mm_vpe_surface_attr_type *pinput_plane,
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

  CDBG("%s: src_unpack_pattern1=0x%x,src_img_w=0x%x,src_img_h=0x%x"
       "srcp0_ystride=0x%x,srcp1_ystride=0x%x,roi.src_x=0x%x,roi.src_y=0x%x,"
       "roi.src_w=0x%x,roi.src_h=0x%x", __func__,
       phw_cfg->src_unpack_pattern1,
       phw_cfg->src_buffer_size.src_img_w, phw_cfg->src_buffer_size.src_img_h,
       phw_cfg->src_ystride1.srcp0_ystride, phw_cfg->src_ystride1.srcp1_ystride,
       phw_cfg->src_roi_offset.src_x, phw_cfg->src_roi_offset.src_y,
       phw_cfg->src_roi_size.src_w, phw_cfg->src_roi_size.src_h);
} /* vpe_set_input_plane() */

/*==========================================================================
     FUNCTION:       vpe_set_output_plane


           VPE_OUT_SIZE:       Width and height of dst ROI
           VPE_OUTP0_ADDR:     Byte address of the dst
            Y plane VPE_OUTP1_ADDR:     Byte address of the dst
       CrCb plane VPE_OUT_YSTRIDE1:   Line strides in bytes for
                                             Y and CbCr planes
==========================================================================*/
void mm_vpe_set_output_plane(mm_vpe_surface_attr_type *poutput_plane,
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

  CDBG("%s: out_pack_pattern1=0x%x"
       "dstp0_ystride=0x%x,dstp1_ystride=0x%x,roi.dst_x=0x%x,roi.dst_y=0x%x,"
       "roi.dst_w=0x%x,roi.dst_h=0x%x", __func__,
       phw_cfg->out_pack_pattern1,
       phw_cfg->out_ystride1.dstp0_ystride, phw_cfg->out_ystride1.dstp1_ystride,
       phw_cfg->out_roi_offset.dst_x, phw_cfg->out_roi_offset.dst_y,
       phw_cfg->out_roi_size.dst_w, phw_cfg->out_roi_size.dst_h);
} /* VPE_SetupOutputPlane() */

static mm_vpe_op_mode_packed   hw_op_mode; // = 0x40FC0004;

const mm_vpe_op_mode_packed vpe_default_op_mode_packed = {
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

static MM_COLOR_FORMAT_ENUM vpe_get_fmt(cam_format_t in_fmt)
{
  MM_COLOR_FORMAT_ENUM out_fmt;
  switch(in_fmt) {
    case CAMERA_YUV_420_NV12:
      out_fmt = MM_COLOR_FORMAT_Y_CRCB_NV12_OFFSITE;
      break;
    case CAMERA_YUV_420_NV21:
      out_fmt = MM_COLOR_FORMAT_Y_CBCR_NV21_OFFSITE;
      break;
    default:
      CDBG_HIGH("%s: Error Other fmts are not supported yet.", __func__);
      out_fmt = MM_COLOR_FORMAT_Y_CRCB_NV12_OFFSITE;
      break;
  }

  return out_fmt;
}

int vpe_config_pipeline(uint32_t handle, mm_vpe_pipe_config_parm_type *cfg)
{
  int rc = 0;
  uint32_t stats_in_w, stats_in_h, margin_x, margin_y;
  uint32_t vpe_output_w = 0, vpe_output_h = 0;
  uint32_t vpe_output_stride0 = 0, vpe_output_stride1 = 0;
  mm_vpe_surface_attr_type input_plane, output_plane;
  mm_vpe_input_plane_config_type input_hw_cfg;
  mm_vpe_output_plane_config_type output_hw_cfg;
  mm_vpe_scale_coef_cfg_type scaler_cfg_0;
  mm_vpe_scale_coef_cfg_type scaler_cfg_1;
  mm_vpe_scale_coef_cfg_type scaler_cfg_2;
  mm_vpe_scale_coef_cfg_type scaler_cfg_3;


  hw_op_mode = vpe_default_op_mode_packed; //0x40FC0004

  memset(&input_hw_cfg, 0, sizeof(mm_vpe_input_plane_config_type));
  memset(&output_hw_cfg, 0, sizeof(mm_vpe_output_plane_config_type));

  memset(&input_plane, 0, sizeof(mm_vpe_surface_attr_type));
  memset(&output_plane, 0, sizeof(mm_vpe_surface_attr_type));

  memset(&scaler_cfg_0, 0, sizeof(mm_vpe_scale_coef_cfg_type));
  memset(&scaler_cfg_1, 0, sizeof(mm_vpe_scale_coef_cfg_type));
  memset(&scaler_cfg_2, 0, sizeof(mm_vpe_scale_coef_cfg_type));
  memset(&scaler_cfg_3, 0, sizeof(mm_vpe_scale_coef_cfg_type));

  /* fill the information for input */
  input_plane.color_format = vpe_get_fmt(cfg->input.fmt);
  input_plane.frame_dimension.frame_offset_x = cfg->input_roi.x;  // default 0
  input_plane.frame_dimension.frame_offset_y = cfg->input_roi.y;  // default 0

  /* double check on the unit. Is it number of pixel ? */
  input_plane.frame_dimension.frame_width = cfg->input.width;
  input_plane.frame_dimension.frame_height = cfg->input.height;

  /* buffer width is the same as frame width */
  input_plane.buffer_dimension.stride_0 = cfg->input.stride0;
  input_plane.buffer_dimension.stride_1 = cfg->input.stride1;

  /* set up the input plane */
  mm_vpe_set_input_plane(&input_plane, &input_hw_cfg);

  /* note: mm_vpe_set_input_plane set the default width and height already. */
  if(cfg->input_roi.width)
    input_hw_cfg.src_roi_size.src_w = cfg->input.width;

  if(cfg->input_roi.height)
    input_hw_cfg.src_roi_size.src_h = cfg->input_roi.height;

  CDBG("%s: vpe input pipeline cfg: width=%d,height=%d,stride0=%d,"
       "stride1=%d,off_x=%d,off_y=%d,roiw=%d,roih=%d",
       __func__, input_plane.frame_dimension.frame_width,
       input_plane.frame_dimension.frame_height,
       input_plane.buffer_dimension.stride_0,
       input_plane.buffer_dimension.stride_1,
       input_plane.frame_dimension.frame_offset_x,
       input_plane.frame_dimension.frame_offset_y,
       input_hw_cfg.src_roi_size.src_w,
       input_hw_cfg.src_roi_size.src_h);

  rc = mm_vpe_util_sendcmd(my_vpe_obj.ops.fd, (void *)&input_hw_cfg,
    sizeof(mm_vpe_input_plane_config_type),
    VPE_CMD_INPUT_PLANE_CFG);
  if(rc) {
    CDBG("%s: VPE_CMD_INPUT_PLANE_CFG err, rc=%d", __func__, rc);
    return rc;
  }

  /* src * dest  chroma offsite, 4:2:0, */
  /* Modify vpe output based on rotation and dis */
  switch (cfg->rot) {
    case ROT_NONE:
      vpe_output_w = cfg->output.width;
      vpe_output_h = cfg->output.height;
      vpe_output_stride0 = cfg->output.stride0;
      vpe_output_stride1 = cfg->output.stride1;
      break;
    case ROT_CLOCKWISE_90:
    case ROT_CLOCKWISE_270:
      hw_op_mode.rot_en = 1;
      vpe_output_w = cfg->output.height; /* swap dimension */
      vpe_output_h = cfg->output.width;
      vpe_output_stride0 = cfg->output.height;
      vpe_output_stride1 = cfg->output.height;
      break;
    case ROT_CLOCKWISE_180:
      hw_op_mode.rot_en = 1;
      vpe_output_w = cfg->output.width;
      vpe_output_h = cfg->output.height;
      vpe_output_stride0 = cfg->output.stride0;
      vpe_output_stride1 = cfg->output.stride1;
      break;
    default:
      CDBG_HIGH("Invalid Rotation value.\n");
      break;
  }
  hw_op_mode.rot_mode = cfg->rot;

  /* set up the output plane. */
  /* fill the information for input */
  output_plane.color_format = vpe_get_fmt(cfg->output.fmt);
  output_plane.frame_dimension.frame_offset_x = 0;
  output_plane.frame_dimension.frame_offset_y = 0;
  /* add the src_chroma_site info */
  hw_op_mode.src_chroma_site =  vpe_get_fmt(cfg->input.fmt);

  /* a note on dimension swap:  better to have HAL layer to do
     the swap. */
  /* double check on the unit. Is it number of pixel ? */
  output_plane.frame_dimension.frame_width = vpe_output_w;
  output_plane.frame_dimension.frame_height = vpe_output_h;

  /* buffer width is the same as frame width */
  output_plane.buffer_dimension.stride_0 = vpe_output_stride0;
  output_plane.buffer_dimension.stride_1 = vpe_output_stride1;

  CDBG("%s: vpe output pipeline cfg: width=%d,height=%d,stride0=%d,"
       "stride1=%d,off_x=%d,off_y=%d, rotation_enum=%d",
       __func__, output_plane.frame_dimension.frame_width,
       output_plane.frame_dimension.frame_height,
       output_plane.buffer_dimension.stride_0,
       output_plane.buffer_dimension.stride_1,
       output_plane.frame_dimension.frame_offset_x,
       output_plane.frame_dimension.frame_offset_y, cfg->rot);
  mm_vpe_set_output_plane(&output_plane, &output_hw_cfg);

  rc = mm_vpe_util_sendcmd(my_vpe_obj.ops.fd, (void *) &output_hw_cfg,
    sizeof(mm_vpe_output_plane_config_type),
    VPE_CMD_OUTPUT_PLANE_CFG);
  if(rc) {
    CDBG("%s: VPE_CMD_OUTPUT_PLANE_CFG err, rc=%d", __func__, rc);
    return rc;
  }

  mm_vpe_init_scale_table(&scaler_cfg_0, 0);
  mm_vpe_init_scale_table(&scaler_cfg_1, 1);
  mm_vpe_init_scale_table(&scaler_cfg_2, 2);
  mm_vpe_init_scale_table(&scaler_cfg_3, 3);

  rc = mm_vpe_util_sendcmd(my_vpe_obj.ops.fd, (void *) &scaler_cfg_0,
    sizeof(mm_vpe_scale_coef_cfg_type),
    VPE_CMD_SCALE_CFG_TYPE);
  if(rc) {
    CDBG("%s: VPE_CMD_SCALE_CFG_TYPE err, rc=%d", __func__, rc);
    return rc;
  }

  rc = mm_vpe_util_sendcmd(my_vpe_obj.ops.fd, (void *) &scaler_cfg_1,
    sizeof(mm_vpe_scale_coef_cfg_type),
    VPE_CMD_SCALE_CFG_TYPE);
  if(rc) {
    CDBG("%s: VPE_CMD_SCALE_CFG_TYPE err, rc=%d", __func__, rc);
    return rc;
  }

  rc = mm_vpe_util_sendcmd(my_vpe_obj.ops.fd, (void *) &scaler_cfg_2,
    sizeof(mm_vpe_scale_coef_cfg_type),
    VPE_CMD_SCALE_CFG_TYPE);
  if(rc) {
    CDBG("%s: VPE_CMD_SCALE_CFG_TYPE err, rc=%d", __func__, rc);
    return rc;
  }

  rc = mm_vpe_util_sendcmd(my_vpe_obj.ops.fd, (void *) &scaler_cfg_3,
    sizeof(mm_vpe_scale_coef_cfg_type),
    VPE_CMD_SCALE_CFG_TYPE);
  if(rc) {
    CDBG("%s: VPE_CMD_SCALE_CFG_TYPE err, rc=%d", __func__, rc);
    return rc;
  }

  rc = mm_vpe_util_sendcmd(my_vpe_obj.ops.fd, (void *) &hw_op_mode,
    sizeof(mm_vpe_op_mode_packed),
    VPE_CMD_OPERATION_MODE_CFG);
  if(rc) {
    CDBG("%s: VPE_CMD_OPERATION_MODE_CFG err, rc=%d", __func__, rc);
    return rc;
  }
  CDBG("%s: vpe pipeline config end, rc = %d", __func__, rc);
  return rc;
}

int mm_vpe_util_sendcmd(int fd, void *pCmdData, unsigned int messageSize,
  int cmd_id)
{
  int rc = 0;
  struct msm_vpe_cfg_cmd cmd;

  switch(cmd_id) {
    case VPE_CMD_INIT:
      if (messageSize) {
        CDBG_ERROR("%s: VPE_CMD_INIT length error cmd_id=%d, length = %d,"
          " ptr = 0x%p", __func__, cmd_id, messageSize, pCmdData);
        return -EINVAL;
      }
      break;

    case VPE_CMD_DEINIT:
      if (messageSize) {
        CDBG_ERROR("%s: VPE_CMD_DEINIT length error cmd_id=%d, length = %d,"
          " ptr = 0x%p", __func__, cmd_id, messageSize, pCmdData);
        return -EINVAL;
      }
      break;

    case VPE_CMD_DISABLE:
      if (messageSize) {
        CDBG_ERROR("%s: VPE_CMD_DISABLE length error cmd_id=%d, length = %d,"
          " ptr = 0x%p", __func__, cmd_id, messageSize, pCmdData);
        return -EINVAL;
      }
      break;

    case VPE_CMD_RESET:
      if (messageSize) {
        CDBG_ERROR("%s: VPE_CMD_RESET length error cmd_id=%d, length = %d,"
          " ptr = 0x%p", __func__, cmd_id, messageSize, pCmdData);
        return -EINVAL;
      }
      break;

    case VPE_CMD_ENABLE:
      if (messageSize != sizeof(struct msm_vpe_clock_rate)) {
        CDBG_ERROR("%s: VPE_CMD_ENABLE length error cmd_id=%d, expected len=%d"
             "actual length = %d, ptr = 0x%p", __func__, cmd_id,
             sizeof(struct msm_vpe_clock_rate), messageSize, pCmdData);
        return -EINVAL;
      }
      break;

    case VPE_CMD_OPERATION_MODE_CFG:
      if (messageSize != sizeof(struct msm_vpe_op_mode_cfg)) {
        CDBG_ERROR("%s: VPE_CMD_OPERATION_MODE_CFG length error cmd_id=%d,"
          "expected length = %d, actual length = %d ptr = 0x%p", __func__,
          cmd_id, sizeof(struct msm_vpe_op_mode_cfg), messageSize, pCmdData);
        return -EINVAL;
      }
      break;

    case VPE_CMD_INPUT_PLANE_CFG:
      if (messageSize != sizeof(struct msm_vpe_input_plane_cfg)) {
        CDBG_ERROR("%s: VPE_CMD_INPUT_PLANE_CFG length error cmd_id=%d, "
          "expected length = %d, actual length = %d ptr = 0x%p", __func__,
          cmd_id, sizeof(struct msm_vpe_input_plane_cfg),
          messageSize, pCmdData);
        return -EINVAL;
      }
      break;

    case VPE_CMD_OUTPUT_PLANE_CFG:
      if (messageSize != sizeof(struct msm_vpe_output_plane_cfg)) {
        CDBG_ERROR("%s: VPE_CMD_OUTPUT_PLANE_CFG length error cmd_id=%d, "
          "expected length = %d, actual length = %d ptr = 0x%p", __func__,
          cmd_id, sizeof(struct msm_vpe_output_plane_cfg),
          messageSize, pCmdData);
        return -EINVAL;
      }
      break;

    case VPE_CMD_INPUT_PLANE_UPDATE:
      if (messageSize != sizeof(struct msm_vpe_input_plane_update_cfg)) {
        CDBG_ERROR("%s: VPE_CMD_INPUT_PLANE_UPDATE length error cmd_id=%d, "
          "expected length = %d, actual length = %d ptr = 0x%p", __func__,
          cmd_id, sizeof(struct msm_vpe_input_plane_update_cfg),
          messageSize, pCmdData);
        return -EINVAL;
      }
      break;

    case VPE_CMD_SCALE_CFG_TYPE:
      if (messageSize != sizeof(struct msm_vpe_scaler_cfg)) {
        CDBG_ERROR("%s: VPE_CMD_SCALE_CFG_TYPE length error cmd_id=%d, "
          "expected length = %d, actual length = %d ptr = 0x%p", __func__,
          cmd_id, sizeof(struct msm_vpe_scaler_cfg), messageSize, pCmdData);
        return -EINVAL;
      }
      break;

    case VPE_CMD_ZOOM:
      if(messageSize != sizeof(struct msm_mctl_pp_frame_cmd)) {
        CDBG_ERROR("%s: VPE_CMD_ZOOM length error cmd_id=%d, "
          "expected length = %d, actual length = %d ptr = 0x%p", __func__,
          cmd_id, sizeof(struct msm_mctl_pp_frame_cmd), messageSize, pCmdData);
        return -EINVAL;
      }
      break;
    default:
      return -EINVAL;
  }
  cmd.cmd_type = cmd_id;
  cmd.length = messageSize;
  cmd.value = pCmdData;
  rc = ioctl(my_vpe_obj.dev_fd, MSM_CAM_V4L2_IOCTL_CFG_VPE, &cmd);
  if (rc < 0) {
    CDBG_ERROR("%s: CFG_VPE ioctl failed for cmd %d, rc = %d",
               __func__, cmd_id, rc);
  }
  return rc;
}

int vpe_state_deinit(uint32_t handle)
{
  int rc = 0;
  struct cam_list *head = &my_vpe_obj.cmd_list.list;
  vpe_pp_node_t *node;

  if(my_vpe_obj.vpe_state == VPE_STATE_NULL) {
    CDBG_HIGH("%s VPE already deinited ", __func__);
    return 0;
  }

  vpe_time_profile_begin();
  rc = mm_vpe_util_sendcmd(my_vpe_obj.ops.fd, NULL, 0, VPE_CMD_RESET);
  if(rc != 0) {
    CDBG_ERROR("%s: VPE_CMD_RESET err = %d", __func__, rc);
  }
  rc = mm_vpe_util_sendcmd(my_vpe_obj.ops.fd, NULL, 0, VPE_CMD_DISABLE);
  if(rc != 0) {
    CDBG_ERROR("%s: VPE_CMD_DISABLE err = %d", __func__, rc);
  }
  rc = mm_vpe_util_sendcmd(my_vpe_obj.ops.fd, NULL, 0, VPE_CMD_DEINIT);
  if(rc != 0) {
    CDBG_ERROR("%s: VPE_CMD_DEINIT err = %d", __func__, rc);
  }
  my_vpe_obj.vpe_state = VPE_STATE_NULL;
  while(head->next != head) {
    node = member_of(head->next, vpe_pp_node_t, list);
    cam_list_del_node(&node->list);
    vpe_put_free_node(handle, node);
    head = &my_vpe_obj.cmd_list.list;
  }
  /* zero out all cmd_list */
  cam_list_init(&my_vpe_obj.cmd_list.list);
  vpe_time_profile_end();
  return rc;
}

int vpe_state_init(uint32_t handle)
{
  int rc = 0;
  struct msm_vpe_clock_rate clk_rate;
  mm_vpe_pipe_config_parm_type vpe_pipeline_cfg_parm;

  if(my_vpe_obj.vpe_state != VPE_STATE_NULL) {
    CDBG_HIGH("%s VPE already initialized ", __func__);
    return 0;
  }
  vpe_pipeline_cfg_parm = my_vpe_obj.cfg;
  rc = mm_vpe_util_sendcmd(my_vpe_obj.ops.fd, NULL, 0, VPE_CMD_INIT);
  if(rc != 0) {
    CDBG_ERROR("%s: VPE_CMD_INIT err = %d", __func__, rc);
    goto err;
  }
  clk_rate.rate = my_vpe_obj.clk_rate;
  rc = mm_vpe_util_sendcmd(my_vpe_obj.ops.fd, (void *)&clk_rate,
          sizeof(struct msm_vpe_clock_rate), VPE_CMD_ENABLE);
  if(rc != 0) {
    CDBG_ERROR("%s: VPE_CMD_ENABLE err = %d", __func__, rc);
    goto err;
  }
  rc = mm_vpe_util_sendcmd(my_vpe_obj.ops.fd, NULL, 0, VPE_CMD_RESET);
  if(rc != 0) {
    CDBG_ERROR("%s: VPE_CMD_RESET err = %d", __func__, rc);
    goto err;
  }
  rc = vpe_config_pipeline(handle, &vpe_pipeline_cfg_parm);
  if(rc != 0) {
    CDBG_ERROR("%s: vpe_config_pipeline err = %d", __func__, rc);
    goto err;
  }
  cam_list_init(&my_vpe_obj.cmd_list.list);
  my_vpe_obj.vpe_state = VPE_STATE_INIT;
  CDBG("%s: end, vpe_state = %d, rc=%d", __func__, my_vpe_obj.vpe_state, rc);
  return rc;
err:
  CDBG_ERROR("%s: error, rc = %d", __func__, rc);
  mm_vpe_util_sendcmd(my_vpe_obj.ops.fd, NULL, 0, VPE_CMD_DEINIT);
  return rc;
}

static vpe_pp_node_t *vpe_queue_frame(uint32_t handle, pp_frame_data_t *frame)
{
  vpe_pp_node_t *node = vpe_get_free_node(handle);

  if(!node) {
    CDBG_HIGH("%s No free node available\n", __func__);
    return NULL;
  }

  node->frame = *frame;
  cam_list_add_tail_node(&node->list, &my_vpe_obj.cmd_list.list);
  CDBG("%s: frame queued in DOING state", __func__);
  return node;
}

static int vpe_send_frame(uint32_t handle, vpe_pp_node_t *node)
{
    int rc = 0, i;
    struct msm_mctl_pp_frame_cmd zoom;

    memset(&zoom,  0,  sizeof(zoom));
    zoom.cookie = (uint32_t)node;
    zoom.vpe_output_action = node->frame.action_flag;
    zoom.src_frame = node->frame.src->frame;
    zoom.dest_frame = node->frame.dest->frame;
    zoom.crop = node->frame.crop;
    zoom.path = node->frame.path;
    /* Copy the timestamp generated at source into the output
     * frame.*/
    zoom.dest_frame.timestamp = zoom.src_frame.timestamp;

    for (i = 0; i < zoom.src_frame.num_planes; i++)
      zoom.src_frame.mp[i].fd = node->frame.src_buf_data.fd;

    for (i = 0; i < zoom.dest_frame.num_planes; i++)
      zoom.dest_frame.mp[i].fd = node->frame.dst_buf_data.fd;

    CDBG("%s Sending buffer fd %d idx %d dest fd %d idx %dto VPE ",
      __func__, zoom.src_frame.mp[0].fd, zoom.src_frame.buf_idx,
      zoom.dest_frame.mp[0].fd, zoom.dest_frame.buf_idx);

    rc = mm_vpe_util_sendcmd(my_vpe_obj.ops.fd, (void *)&zoom,
                             sizeof(zoom), VPE_CMD_ZOOM);
    vpe_time_profile_begin();
    if(rc == 0) {
      my_vpe_obj.vpe_state = VPE_STATE_DOING;
      CDBG("%s: VPE cmd sent, cookie = 0x%p, state moves to DOING", __func__,
        node);
      node->state = VPE_FRAME_DOING;
    } else {
      CDBG_ERROR("%s: sending pp_cmd to VPE HW err = %d",  __func__,  rc);
      CDBG("VPE State: %d\n", my_vpe_obj.vpe_state);
      return -EINVAL;
    }
    return rc;
}
static int vpe_proc_frame(uint32_t handle, pp_frame_data_t *frame)
{
    int rc = 0;
    vpe_pp_node_t *node = NULL;

    CDBG("%s: begin", __func__);

    if (my_vpe_obj.low_power_mode &&
       (frame->crop.src_w == frame->crop.dst_w &&
        frame->crop.src_h == frame->crop.dst_h)) {
       frame->not_pp_flag = 1;
       CDBG("%s Low power mode: No zoom. No need to send to VPE", __func__);
       return 0;
    }
    node = vpe_queue_frame(handle, frame);
    if(!node) {
      CDBG_ERROR("%s: no mem for queuing the frame",  __func__);
      return -1;
    }
    rc = vpe_send_frame(handle, node);
    if(rc < 0) {
        cam_list_del_node(&node->list);
        vpe_put_free_node(handle, node);
    }
    return rc;
}

static int vpe_proc_ack(uint32_t handle, struct msm_mctl_pp_event_info *ack)
{
  struct cam_list *head = &my_vpe_obj.cmd_list.list;
  vpe_pp_node_t *node = NULL;
  struct cam_list *pos;

  CDBG("%s: VPE ACK received, cookie = 0x%x", __func__, ack->ack.cookie);
  for (pos = head->next; pos != head; pos = pos->next) {
    node = member_of(pos, vpe_pp_node_t, list);
    if(node == (vpe_pp_node_t *)ack->ack.cookie) {
      cam_list_del_node(&node->list);
      CDBG("%s: buf done, node = 0x%p, frame_id = 0x%x", __func__, node,
        node->frame.frame_id);
      my_vpe_obj.ops.buf_done(my_vpe_obj.ops.parent, &node->frame);
      vpe_put_free_node(handle, node);
      break;
    }
  }
  return 0;
}

static int vpe_next_pp_cmd(uint32_t handle)
{
  int rc = 0;
  struct msm_mctl_pp_frame_cmd zoom;
  vpe_pp_node_t *node = NULL;
  struct cam_list *head = &my_vpe_obj.cmd_list.list;

  while(head->next != head) {
    node = member_of(head->next, vpe_pp_node_t, list);
    rc = vpe_send_frame(handle, node);
    if(!rc) {
      CDBG("%s: pp cmd sent to VPE, cookie = 0x%p", __func__, node);
      return rc;
    } else {
      CDBG_ERROR("%s: cannot send pp cmd to VPE, rc = %d",  __func__,  rc);
      /* TBD: should tell mctl_pp about this? this is dead end */
      cam_list_del_node(&node->list);
      node->frame.status = -1;
      my_vpe_obj.ops.buf_done(my_vpe_obj.ops.parent, &node->frame);
      vpe_put_free_node(handle, node);
      break;
    }
    head = &my_vpe_obj.cmd_list.list;
  }
  CDBG("%s: no queued frame, state moves to INIT", __func__);
  my_vpe_obj.vpe_state = VPE_STATE_INIT;
  return 0;
}

int vpe_run_state_machine(uint32_t handle, vpe_event_type_t event, void *parm)
{
  int rc = 0;

  CDBG("%s:vpe state = %d, event = %d", __func__, my_vpe_obj.vpe_state, event);
  switch(event) {
  case VPE_EVENT_ACK: {
    struct timeval t_diff;
    if(VPE_STATE_DOING != my_vpe_obj.vpe_state) {
      /* old ack. dump it */
      vpe_time_profile_end();
      break;
    }
    vpe_time_profile_end();
    vpe_proc_ack(handle, (struct msm_mctl_pp_event_info *)parm);
    vpe_next_pp_cmd(handle);
    break;
  }
  case VPE_EVENT_DO_PP: {
    pp_frame_data_t *frame = parm;
    switch(my_vpe_obj.vpe_state) {
      case VPE_STATE_NULL:
        frame->not_pp_flag = 1;
        break;
      case VPE_STATE_INIT:
        rc = vpe_proc_frame(handle, frame);
        break;
      case VPE_STATE_DOING:
        if(vpe_queue_frame(handle, frame) == NULL) {
          CDBG_ERROR("%s: queue pp cmd err",  __func__);
          rc = -1;
        }
        break;
      default:
        rc = -1;
        break;
    }
    break;
  }
  default:
      break;
  }
end:
  return 0;
}

/*==============================================================================
 * FUNCTION    - vpe_thread_ready_signal -
 *
 * DESCRIPTION:
 *============================================================================*/
static void vpe_thread_ready_signal(int status)
{
  CDBG("%s: E\n", __func__);

  pthread_mutex_lock(&my_vpe_obj.thread_data.mutex);
  my_vpe_obj.thread_data.thread_status = status;
  pthread_cond_signal(&my_vpe_obj.thread_data.cond_v);
  pthread_mutex_unlock(&my_vpe_obj.thread_data.mutex);

  CDBG("%s: X\n", __func__);
} /* vpe_thread_ready_signal */

/*==============================================================================
 * FUNCTION    - vpe_thread_release -
 *
 * DESCRIPTION:
 *============================================================================*/
static void vpe_thread_release()
{
  int rc = -1, retry = 0;
  vpe_thread_msg_t msg;

  CDBG("%s: E\n", __func__);

  msg.type = VPE_THREAD_CMD_EXIT;
  msg.data = NULL;

  do {
    CDBG("%s: Send VPE_THREAD_CMD_EXIT. Attemp = %d", __func__, retry++);
    rc = write(my_vpe_obj.thread_data.in_pipe_fds[1], &msg, sizeof(msg));
  } while ((rc < 0) && (retry < 10));

  if (pthread_join(my_vpe_obj.thread_data.pid, NULL) != 0)
    CDBG("%s: vpe thread exited \n", __func__);

  pthread_mutex_destroy(&my_vpe_obj.thread_data.mutex);
  pthread_cond_destroy(&my_vpe_obj.thread_data.cond_v);

  CDBG("%s: X\n", __func__);
} /* vpe_thread_release */

static int vpe_open_device() {
  int dev_fd = 0;
  CDBG("%s Opening VPE device %s %d", __func__, my_vpe_obj.dev_name, gettid());
  dev_fd = open(my_vpe_obj.dev_name, O_RDWR | O_NONBLOCK);
  if (dev_fd < 0) {
    CDBG_ERROR("%s VPE Subdev open failed", __func__);
    return -EINVAL;
  }
  return dev_fd;
}

static void vpe_close_device() {
  int rc = 0;

  CDBG("%s Closing VPE device %d", __func__, my_vpe_obj.dev_fd);
  if (my_vpe_obj.dev_fd > 0) {
      rc = close(my_vpe_obj.dev_fd);
      CDBG("%s Close VPE device rc = %d ", __func__, rc);
      my_vpe_obj.dev_fd = 0;
  }
}

static int vpe_interface_init(uint32_t handle, mctl_ops_t *ops, void *init_data)
{
  CDBG("%s E\n", __func__);
  int rc = 0;
  my_vpe_obj.ops = *ops;

  rc = ioctl(my_vpe_obj.dev_fd, VIDIOC_MSM_VPE_INIT);
  if(rc < 0) {
    CDBG_ERROR("%s: subdev init failed; error: %d %s\n",
      __func__, rc, strerror(errno));
    return -EINVAL;
  }
  CDBG("%s out pipe fds = %d %d ", __func__,
    *((int *)init_data), *(((int *)init_data) + 1));
  my_vpe_obj.thread_data.out_pipe_fds[0] = *((int *)init_data);
  my_vpe_obj.thread_data.out_pipe_fds[1] = *(((int *)init_data) + 1);

  CDBG("%s : X", __func__);
  return 0;
}

static int vpe_interface_destroy(uint32_t handle)
{
  int rc = 0;
  CDBG("%s\n", __func__);

  if (handle != my_vpe_obj.handle) {
    CDBG_ERROR("%s: Invalid handle passed", __func__);
    return -EINVAL;
  }
  vpe_thread_release();

  if (my_vpe_obj.thread_data.in_pipe_fds[0])
    close(my_vpe_obj.thread_data.in_pipe_fds[0]);
  if (my_vpe_obj.thread_data.in_pipe_fds[1])
    close(my_vpe_obj.thread_data.in_pipe_fds[1]);

  rc = ioctl(my_vpe_obj.dev_fd, VIDIOC_MSM_VPE_RELEASE);
  if(rc < 0) {
    CDBG_ERROR("%s: subdev release failed; error: %d %s\n",
      __func__, rc, strerror(errno));
    return -EINVAL;
  }
  if (my_vpe_obj.dev_fd <= 0)
    CDBG_HIGH("%s VPE device already closed", __func__);
  else
    vpe_close_device();

  memset(&my_vpe_obj, 0, sizeof(my_vpe_obj));
  return 0;
}

static int vpe_interface_process(uint32_t handle, int event, void *parm)
{
  int rc = 0;
  switch(event) {
    case VPE_EVENT_DO_PP:
      rc = vpe_run_state_machine(handle, event, parm);
      break;
    case VPE_EVENT_ACK:
      rc = vpe_run_state_machine(handle, event, parm);
      break;
    case VPE_EVENT_START:
      rc = vpe_state_init(handle);
      break;
    case VPE_EVENT_STOP:
      CDBG("%s Sending Deinit to VPE ", __func__);
      rc = vpe_state_deinit(handle);
      break;
    default:
      rc = -1;
      break;
  }
  return rc;
}

static int vpe_interface_set_params(uint32_t handle, int parm_type,
  void *parm1, void *parm2)
{
  int rc = 0;
  if(handle != my_vpe_obj.handle) {
    CDBG_ERROR("%s: handle mismatch", __func__);
    return -1;
  }
  switch(parm_type) {
    case VPE_PARM_CLK_RATE:
      my_vpe_obj.clk_rate = *((int *)parm1);
      break;
    case VPE_PARM_PIPELINE_CFG:
      my_vpe_obj.cfg = *((mm_vpe_pipe_config_parm_type *)parm1);
      break;
    case VPE_PARM_LOW_POWER_MODE:
      my_vpe_obj.low_power_mode = *((int *)parm1);
      CDBG_HIGH("%s Low power mode %s ", __func__,
        my_vpe_obj.low_power_mode ? "Enabled" : "Disabled");
      break;
    case VPE_PARM_PP_OPS:
      CDBG_HIGH("%s: parm_type VPE_PARM_PP_OPS not supported", __func__);
      rc = -1;
      break;
    default:
      CDBG_ERROR("%s: Invalid parm_type %d", __func__, parm_type);
      rc = -1;
      break;
  }
  return rc;
}

static int vpe_interface_get_params(uint32_t handle, int parm_type,
                                    void *parm, int parm_len)
{
  return -1;
}

/*==============================================================================
 * FUNCTION    - vpe_processing_thread -
 *
 * DESCRIPTION:
 *============================================================================*/
void *vpe_processing_thread(void *data)
{
  int rc = 0, retry;
  struct pollfd read_fds[2];
  int timeoutms = -1;
  vpe_thread_msg_t msg;
  mctl_pp_cmd_t cmd;
  struct v4l2_event v4l2_evt;
  struct msm_camera_v4l2_ioctl_t v4l2_ioctl;
  struct v4l2_event_subscription sub;
  struct msm_mctl_pp_event_info pp_event;

  CDBG("%s: E", __func__);

  memset(&sub, 0, sizeof(struct v4l2_event_subscription));
  sub.type = V4L2_EVENT_PRIVATE_START + MSM_CAM_RESP_MCTL_PP_EVENT;
  rc = ioctl(my_vpe_obj.dev_fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
  if (rc < 0) {
    CDBG_ERROR("%s Error subscribing events from VPE device rc = %d",
      __func__, rc);
    vpe_thread_ready_signal(VPE_THREAD_CREATE_FAIL);
    return NULL;
  }

  vpe_thread_ready_signal(VPE_THREAD_CREATE_SUCCESS);
  /* VPE thread polls on two events:
   * - ACK Events coming from the vpe subdev node
   * - CTRL Events coming from the pipe between the vpe
   * interface(mctl_pp thread context) and this thread.*/
  read_fds[0].fd = my_vpe_obj.dev_fd;
  read_fds[0].events = POLLPRI;
  read_fds[1].fd = my_vpe_obj.thread_data.in_pipe_fds[0];
  read_fds[1].events = POLLIN|POLLRDNORM;
  do {
    rc = poll(read_fds, 2, timeoutms);
    CDBG("%s: Woke Up rc = %d", __func__, rc);
    if (rc > 0) {

      /* Check for ACK events from hardware*/
      if ((read_fds[0].revents & POLLPRI)) {
        memset(&cmd, 0, sizeof(cmd));
        cmd.cmd_type = QCAM_MCTL_CMD_DATA;
        cmd.evt_type = MSM_CAM_RESP_MCTL_PP_EVENT;
        cmd.pp_event.event = MCTL_PP_EVENT_CMD_ACK;

        rc = ioctl(my_vpe_obj.dev_fd, VIDIOC_DQEVENT, &v4l2_evt);
        if (rc >= 0) {
          CDBG("%s Got event from kernel %d %d", __func__, v4l2_evt.type,
             V4L2_EVENT_PRIVATE_START + MSM_CAM_RESP_MCTL_PP_EVENT);
          memset(&v4l2_ioctl, 0, sizeof(struct msm_camera_v4l2_ioctl_t));
          v4l2_ioctl.ioctl_ptr = &pp_event;
          rc = ioctl(my_vpe_obj.dev_fd, MSM_CAM_V4L2_IOCTL_GET_EVENT_PAYLOAD,
            &v4l2_ioctl);
          if (v4l2_evt.type ==
              (V4L2_EVENT_PRIVATE_START + MSM_CAM_RESP_MCTL_PP_EVENT)
              && rc >= 0) {
            CDBG("%s Got ACK event %d status %d ", __func__, pp_event.ack.cmd,
              pp_event.ack.status);
            cmd.pp_event = pp_event;
            retry = 0;
            do {
              CDBG("%s: Send MCTL_PP_EVENT_CMD_ACK to fd=%d. Attempt = %d rc = %d",
                __func__, my_vpe_obj.thread_data.out_pipe_fds[1], retry++, rc);
              rc = write(my_vpe_obj.thread_data.out_pipe_fds[1],
                &cmd, sizeof(cmd));
            } while ((rc < 0) && (retry < 10)); 
          } else {
            CDBG("%s Got Event payload, but type did not match ", __func__);
          }
        } else {
          CDBG("%s DQEVENT failed on VPE device ", __func__);
          continue;
        }
      }

      /* Check for CTRL events from mctl pp. */
      if ((read_fds[1].revents & POLLIN) &&
          (read_fds[1].revents & POLLRDNORM)) {
            
        memset(&msg, 0, sizeof(msg));
        rc = read(my_vpe_obj.thread_data.in_pipe_fds[0], &msg, sizeof(msg));
        if (rc < 0) {
          CDBG("%s: Error in reading from VPE pipe", __func__);
          continue;
        }

        CDBG("%s: msg.type = %d", __func__, msg.type);
        if (msg.type == VPE_THREAD_CMD_EXIT) {
          CDBG("%s Exiting VPE processing thread ", __func__);
          break;
        } else {
          CDBG("%s: Invalid msg type = %d", __func__, msg.type);
          continue;
        }
      }
    } else {
      usleep(10);
      continue;
    }
  } while (TRUE);

  memset(&sub, 0, sizeof(struct v4l2_event_subscription));
  sub.type = V4L2_EVENT_PRIVATE_START + MSM_CAM_RESP_MCTL_PP_EVENT;
  rc = ioctl(my_vpe_obj.dev_fd, VIDIOC_UNSUBSCRIBE_EVENT, &sub);
  if (rc < 0)
    CDBG_ERROR("%s Error unsubscribing events from VPE device ", __func__);

  CDBG("%s: X", __func__);
  return NULL;
} /* vpe_processing_thread */

/*==============================================================================
 * FUNCTION    - vpe_launch_thread -
 *
 * DESCRIPTION:
 *============================================================================*/
static int vpe_launch_thread(void)
{
  int rc = 0;

  CDBG("%s: E\n", __func__);

  pthread_mutex_init(&my_vpe_obj.thread_data.mutex, NULL);
  pthread_cond_init(&my_vpe_obj.thread_data.cond_v, NULL);
  my_vpe_obj.thread_data.thread_status = VPE_THREAD_CREATE_REQUESTED;

  pthread_mutex_lock(&my_vpe_obj.thread_data.mutex);

  rc = pthread_create(&my_vpe_obj.thread_data.pid, NULL, vpe_processing_thread,
       NULL);
  if (rc < 0) {
    CDBG_ERROR("%s: Cannot launch vpe_processing_thread rc = %d", __func__, rc);
    return rc;
  }

  if (VPE_THREAD_CREATE_REQUESTED == my_vpe_obj.thread_data.thread_status)
    pthread_cond_wait(&my_vpe_obj.thread_data.cond_v,
      &my_vpe_obj.thread_data.mutex);

  if (VPE_THREAD_CREATE_FAIL == my_vpe_obj.thread_data.thread_status) {
    CDBG_ERROR("%s: vpe thread create failed", __func__);
    rc = pthread_join(my_vpe_obj.thread_data.pid, NULL);
    pthread_mutex_unlock(&my_vpe_obj.thread_data.mutex);
    return -rc;
  }

  pthread_mutex_unlock(&my_vpe_obj.thread_data.mutex);

  CDBG("%s: X\n", __func__);
  return rc;
} /* vpe_launch_thread */

uint32_t vpe_interface_create(module_ops_t *ops, int sdev_number)
{
  int rc = 0;
  CDBG("%s\n", __func__);
  if(!ops) {
    CDBG_ERROR("%s: null ops pointer",  __func__);
    return 0;
  }

  if(my_vpe_obj.handle == 0) {
    my_vpe_obj.handle = ++vpe_open_count;
    my_vpe_obj.vpe_state = VPE_STATE_NULL;
  }

  if (my_vpe_obj.dev_fd) {
    CDBG_ERROR("%s VPE Device already open. ", __func__);
    return (uint32_t)NULL;
  }

  snprintf(my_vpe_obj.dev_name, MAX_DEV_NAME_LEN,
    SUBDEV_STR, sdev_number);
  CDBG(" VPE dev_name = %s\n", my_vpe_obj.dev_name);

  if (!my_vpe_obj.dev_name[0]) {
    CDBG_ERROR("%s VPE Node discovery failed. Cannot open VPE device ",
      __func__);
    return (uint32_t)NULL;
  }

  my_vpe_obj.dev_fd = vpe_open_device();
  if (my_vpe_obj.dev_fd < 0) {
    CDBG_ERROR("%s Error opening VPE device ", __func__);
    return (uint32_t)NULL;
  }

  memset(ops,  0,  sizeof(module_ops_t));
  ops->handle = my_vpe_obj.handle;
  ops->init = vpe_interface_init;
  ops->process = vpe_interface_process;
  ops->set_params = vpe_interface_set_params;
  ops->get_params = vpe_interface_get_params;
  ops->destroy = vpe_interface_destroy;

  rc = pipe(my_vpe_obj.thread_data.in_pipe_fds);
  if (rc < 0) {
    CDBG_ERROR("%s: VPE pipe creation failed. rc = %d", __func__, rc);
    goto pipe_creation_error;
  }

  rc = vpe_launch_thread();
  if (rc < 0) {
    CDBG_ERROR("%s: vpe_launch_thread failed", __func__);
    goto vpe_thread_fail;
  }
  
  return my_vpe_obj.handle;

pipe_creation_error:
  memset(ops,  0,  sizeof(module_ops_t));

vpe_thread_fail:
  if (my_vpe_obj.thread_data.in_pipe_fds[0])
    close(my_vpe_obj.thread_data.in_pipe_fds[0]);
  if (my_vpe_obj.thread_data.in_pipe_fds[1])
    close(my_vpe_obj.thread_data.in_pipe_fds[1]);

  vpe_close_device();
  return (uint32_t)NULL;
}
