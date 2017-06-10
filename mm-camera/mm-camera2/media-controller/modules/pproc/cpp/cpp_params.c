/*============================================================================

  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

//#include "camera.h"
#include "cam_list.h"
#include <media/msmb_camera.h>
#include <media/msmb_pproc.h>
#include "camera_dbg.h"
#include "mtype.h"
#include "pproc_interface.h"
#include "cpp.h"
#include <math.h>

//#define ENABLE_CPP_DRIVER_DEBUG
#ifdef ENABLE_CPP_DRIVER_DEBUG
#undef CDBG
#define CDBG ALOGE
#endif

/*q factor used in CPP*/
static const int q_factor = 21;

/*mask to get the fractional bits*/
static const int q_mask = (1 << 21) - 1;

/*Number of extra pixels needed on the left or top for upscaler*/
static const int upscale_left_top_padding = 1;

/*Number of extra pixels needed on the right or bottom for upscaler*/
static const int upscale_right_bottom_padding = 2;

/*The number of fractional bits used by downscaler*/
static const int downscale_interpolation_resolution = 3;

/*
  Assumption: Assume that the left top point of source image matches
              the left top point of destination image before rotation

  dst_with: destination image width before rotation
  dst_height: destination image height before rotation
  fetch_address: address of top left point of source image
  destination_address: address of top left point of destination "after" rotation
*/

/*
  phase_x_cur:  phase of the topleft corner of current fetch block
  phase_x_next: phase of the topleft corner of fetch block right
                next to the current fetch block
  phase_y_cur:  phase of the topleft corner of current fetch block
  phase_y_next: phase of the topleft corner of fetch block right
                below the current fetch block
  INIT_PHASE_X: in_ary->horizontal_scale_initial_phase for leftmost stripe,
                in_ary->horizontal_scale_block_initial_phase for others
  INIT_PHASE_Y: in_ary->vertical_scale_initial_phase for topmost stripe,
                in_ary->vertical_scale_block_initial_phase for others
  PHASE_X_STEP: in_ary->horizontal_scale_ratio
  PHASE_Y_STEP: in_ary->vertical_scale_ratio
  DST_TILE_SIZE_X: destination stripe width
  DST_TILE_SIZE_Y: destination stripe height
  X_tile_index: horizontal index starting from 0
  Y_tile_index: vertical index starting from 0
*/

struct cpp_accumulated_phase_t {
  long long phase_x_cur;
  long long phase_x_next;
  long long phase_y_cur;
  long long phase_y_next;
  long long INIT_PHASE_X;
  long long INIT_PHASE_Y;
  long long PHASE_X_STEP;
  long long PHASE_Y_STEP;
  int DST_TILE_SIZE_X;
  int DST_TILE_SIZE_Y;
  int X_tile_index;
  int Y_tile_index;
};

static void
increment_phase (struct cpp_accumulated_phase_t *in_phase,
                 int current_block_width, int current_block_height)
{
  in_phase->phase_x_cur =
  in_phase->PHASE_X_STEP * in_phase->DST_TILE_SIZE_X *
  in_phase->X_tile_index + in_phase->INIT_PHASE_X;
  in_phase->phase_x_next =
  in_phase->phase_x_cur + in_phase->PHASE_X_STEP * current_block_width;
  in_phase->phase_y_cur =
  in_phase->PHASE_Y_STEP * in_phase->DST_TILE_SIZE_Y *
  in_phase->Y_tile_index + in_phase->INIT_PHASE_Y;
  in_phase->phase_y_next =
  in_phase->phase_y_cur + in_phase->PHASE_Y_STEP * current_block_height;
}

void
run_TW_logic (struct cpp_plane_info_t *in_ary, int src_tile_index_x_counter,
              int src_tile_index_y_counter, struct msm_cpp_frame_strip_info *out_ary)
{
  int i;
  out_ary->dst_start_x = 0;
  out_ary->dst_start_y = 0;
  switch (in_ary->rotate) {
  case 1: /*90 degree*/
    if (1) {
      for (i = 0; i < src_tile_index_x_counter; i++) {
        out_ary->dst_start_y += in_ary->stripe_block_width[i];
      }
      out_ary->dst_end_y =
      out_ary->dst_start_y +
      in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
    } else {
      for (i = in_ary->frame_width_mcus - 1;
          i > src_tile_index_x_counter; i--) {
        out_ary->dst_start_y += in_ary->stripe_block_width[i];
      }
      out_ary->dst_end_y =
      out_ary->dst_start_y +
      in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
    }
    for (i = 0; i < src_tile_index_y_counter; i++) {
      out_ary->dst_start_x += in_ary->stripe_block_height[i];
    }
    out_ary->dst_end_x =
    out_ary->dst_start_x +
    in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
    break;
  case 2:
    if (in_ary->mirror & 0x1) {
      for (i = 0; i < src_tile_index_x_counter; i++) {
        out_ary->dst_start_x += in_ary->stripe_block_width[i];
      }
      out_ary->dst_end_x =
      out_ary->dst_start_x +
      in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
    } else {
      for (i = in_ary->frame_width_mcus - 1;
          i > src_tile_index_x_counter; i--) {
        out_ary->dst_start_x += in_ary->stripe_block_width[i];
      }
      out_ary->dst_end_x =
      out_ary->dst_start_x +
      in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
    }
    for (i = in_ary->frame_height_mcus - 1;
        i > src_tile_index_y_counter; i--) {
      out_ary->dst_start_y += in_ary->stripe_block_height[i];
    }
    out_ary->dst_end_y =
    out_ary->dst_start_y +
    in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
    break;
  case 3:
    if (1) {
      for (i = in_ary->frame_width_mcus - 1;
          i > src_tile_index_x_counter; i--) {
        out_ary->dst_start_y += in_ary->stripe_block_width[i];
      }
      out_ary->dst_end_y =
      out_ary->dst_start_y +
      in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
    } else {
      for (i = 0; i < src_tile_index_x_counter; i++) {
        out_ary->dst_start_y += in_ary->stripe_block_width[i];
      }
      out_ary->dst_end_y =
      out_ary->dst_start_y +
      in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
    }
    for (i = in_ary->frame_height_mcus - 1;
        i > src_tile_index_y_counter; i--) {
      out_ary->dst_start_x += in_ary->stripe_block_height[i];
    }
    out_ary->dst_end_x =
    out_ary->dst_start_x +
    in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
    break;
  case 0:
  default:
    if (in_ary->mirror & 0x1) {
      for (i = in_ary->frame_width_mcus - 1;
          i > src_tile_index_x_counter; i--) {
        out_ary->dst_start_x += in_ary->stripe_block_width[i];
      }
      out_ary->dst_end_x =
      out_ary->dst_start_x +
      in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
    } else {
      for (i = 0; i < src_tile_index_x_counter; i++) {
        out_ary->dst_start_x += in_ary->stripe_block_width[i];
      }
      out_ary->dst_end_x =
      out_ary->dst_start_x +
      in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
    }
    for (i = 0; i < src_tile_index_y_counter; i++) {
      out_ary->dst_start_y += in_ary->stripe_block_height[i];
    }
    out_ary->dst_end_y =
    out_ary->dst_start_y +
    in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
    break;
  }
  out_ary->destination_address =
  in_ary->destination_address +
  out_ary->dst_start_x * out_ary->bytes_per_pixel +
  out_ary->dst_start_y * in_ary->dst_stride;
}

/*run tile fetch logic*/
void
run_TF_logic (struct cpp_plane_info_t *in_ary, int src_tile_index_x_counter,
              int src_tile_index_y_counter, struct msm_cpp_frame_strip_info *out_ary)
{
  /*et up information needed by phase accumulator*/

  struct cpp_accumulated_phase_t phase_fg;
  if (src_tile_index_x_counter) { /*not the leftmost stripe*/
    phase_fg.INIT_PHASE_X = in_ary->horizontal_scale_block_initial_phase;
  } else {              /*the leftmost stripe*/
    phase_fg.INIT_PHASE_X = in_ary->horizontal_scale_initial_phase;
  }
  if (src_tile_index_y_counter) { /*not the topmost block*/
    phase_fg.INIT_PHASE_Y = in_ary->vertical_scale_block_initial_phase;
  } else {              /*the topmost block*/
    phase_fg.INIT_PHASE_Y = in_ary->vertical_scale_initial_phase;
  }

  phase_fg.PHASE_X_STEP = in_ary->horizontal_scale_ratio;
  phase_fg.PHASE_Y_STEP = in_ary->vertical_scale_ratio;
  phase_fg.X_tile_index = src_tile_index_x_counter;
  phase_fg.Y_tile_index = src_tile_index_y_counter;
  phase_fg.DST_TILE_SIZE_X = in_ary->dst_block_width;
  phase_fg.DST_TILE_SIZE_Y = in_ary->dst_block_height;

  /*invoke phase accumlator*/
  increment_phase (&phase_fg,
                   in_ary->stripe_block_width[src_tile_index_x_counter],
                   in_ary->stripe_block_height[src_tile_index_y_counter]);

  /*only the fractional bits are needed*/
  out_ary->h_init_phase = (int) (phase_fg.phase_x_cur & q_mask);
  out_ary->v_init_phase = (int) (phase_fg.phase_y_cur & q_mask);

  out_ary->h_phase_step = in_ary->horizontal_scale_ratio;
  out_ary->v_phase_step = in_ary->vertical_scale_ratio;

  /*that is, we are using upscaler for scaling in the x direction*/
  if (in_ary->horizontal_scale_ratio < (1 << q_factor) ||
      (in_ary->horizontal_scale_ratio == (1 << q_factor) &&
       out_ary->h_init_phase != 0)) {
    out_ary->scale_h_en = 1;
    out_ary->upscale_h_en = 1;
    if (src_tile_index_x_counter) {
      /*not the leftmost stripe, need to fetch more on the left*/
        unsigned long long temp = // position of the first pixel of the upscaler output
        phase_fg.phase_x_cur -
        in_ary->postscale_padding * in_ary->horizontal_scale_ratio;

      /*only the fractional bits are needed*/
      out_ary->h_init_phase = (int) (temp & q_mask);
      out_ary->src_start_x =
      (int) (temp >> q_factor) -
        upscale_left_top_padding - in_ary->prescale_padding;

      out_ary->prescale_crop_width_first_pixel = in_ary->prescale_padding;
      out_ary->postscale_crop_width_first_pixel =
      in_ary->postscale_padding;
    } else {
      /*the leftmost stripe.
        left padding is provided by denoise and sharpening but not upscaling*/

      /*only the fractional bits are needed*/
      out_ary->h_init_phase = (int) (phase_fg.phase_x_cur & q_mask);
      out_ary->src_start_x =
      (int) ((phase_fg.phase_x_cur) >> q_factor) -
      upscale_left_top_padding;
      out_ary->prescale_crop_width_first_pixel = 0;
      out_ary->postscale_crop_width_first_pixel = 0;
    }
    if (src_tile_index_x_counter == in_ary->frame_width_mcus - 1) {
      /*the rightmost stripe,
        right padding is provided by denoise and sharpening but not upscaling*/
      out_ary->src_end_x =
      (int) ((phase_fg.phase_x_next -
              in_ary->horizontal_scale_ratio) >> q_factor) +
      upscale_right_bottom_padding;
      out_ary->prescale_crop_width_last_pixel =
      out_ary->src_end_x - out_ary->src_start_x;
    } else { /*not the rightmost stripe. need to fetch more on the right*/
      out_ary->src_end_x =
      (int) ((phase_fg.phase_x_next +
              (in_ary->postscale_padding -
               1) * in_ary->horizontal_scale_ratio) >> q_factor) +
      upscale_right_bottom_padding + in_ary->prescale_padding;
      out_ary->prescale_crop_width_last_pixel =
      out_ary->src_end_x - out_ary->src_start_x -
      in_ary->prescale_padding;
    }
  } else if (in_ary->horizontal_scale_ratio > (1 << q_factor)) {
    /*horizontal downscaler*/
    out_ary->scale_h_en = 1;
    out_ary->upscale_h_en = 0;
    if (src_tile_index_x_counter) {
      /*not the leftmost stripe, need to fetch more on the left*/

      unsigned long long temp = // position of the first pixel of the upscaler output
        phase_fg.phase_x_cur -
        in_ary->postscale_padding * in_ary->horizontal_scale_ratio;
      /*only the fractional bits are needed*/
      out_ary->h_init_phase = (int) (temp & q_mask);
      out_ary->src_start_x =
        (int) (temp >> q_factor) - in_ary->prescale_padding;

      out_ary->prescale_crop_width_first_pixel = in_ary->prescale_padding;
      out_ary->postscale_crop_width_first_pixel =
      in_ary->postscale_padding;
    } else {
      /*the leftmost stripe, left padding is done by denoise and sharpening*/

      /*only the fractional bits are needed*/
      out_ary->h_init_phase = (int) (phase_fg.phase_x_cur & q_mask);
      out_ary->src_start_x = (int) (phase_fg.phase_x_cur >> q_factor);
      out_ary->prescale_crop_width_first_pixel = 0;
      out_ary->postscale_crop_width_first_pixel = 0;
    }
    if (src_tile_index_x_counter == in_ary->frame_width_mcus - 1) {
      /*the rightmost stripe,
        right padding is provided by denoise and sharpening*/
      if ((phase_fg.phase_x_next & q_mask) >> (q_factor -
                                               downscale_interpolation_resolution)) {
        out_ary->src_end_x = (int) (phase_fg.phase_x_next >> q_factor);
      } else {
        /*need to fetch one pixel less*/
        out_ary->src_end_x = (int) (phase_fg.phase_x_next >> q_factor) - 1;
      }
      out_ary->prescale_crop_width_last_pixel =
      out_ary->src_end_x - out_ary->src_start_x;
    } else {
      /*not the rightmost stripe, need to fetch more on the right*/
      long long temp =
      phase_fg.phase_x_next +
      in_ary->postscale_padding * in_ary->horizontal_scale_ratio;
      if ((temp & q_mask) >>
          (q_factor - downscale_interpolation_resolution)) {
        out_ary->src_end_x =
        (int) (temp >> q_factor) + in_ary->prescale_padding;
      } else {
        /*need to fetch one pixel less*/
        out_ary->src_end_x = (int) (temp >> q_factor) - 1 + in_ary->prescale_padding;
      }
      out_ary->prescale_crop_width_last_pixel =
      out_ary->src_end_x - out_ary->src_start_x -
      in_ary->prescale_padding;
    }
  } else {
    /*that is, scaling is disabled in the x direction*/
    out_ary->scale_h_en = 0;
    out_ary->upscale_h_en = 0;
    out_ary->h_init_phase = 0;
    if (src_tile_index_x_counter) {
      /*not the leftmost stripe, need to fetch more on the left*/
      out_ary->src_start_x =
      (int) (phase_fg.phase_x_cur >> q_factor) -
      in_ary->postscale_padding - in_ary->prescale_padding;
      out_ary->prescale_crop_width_first_pixel = in_ary->prescale_padding;
      out_ary->postscale_crop_width_first_pixel =
      in_ary->postscale_padding;
    } else {
      CDBG("Left most stripe\n");
      /*the leftmost stripe,
        padding is done internally in denoise and sharpening block*/
      out_ary->src_start_x = (int) (phase_fg.phase_x_cur >> q_factor);
      out_ary->prescale_crop_width_first_pixel = 0;
      out_ary->postscale_crop_width_first_pixel = 0;
    }
    if (src_tile_index_x_counter == in_ary->frame_width_mcus - 1) {
      /*the rightmost stripe, right padding is done internally in the HW*/
      out_ary->src_end_x = (int) (phase_fg.phase_x_next >> q_factor) - 1;
      out_ary->prescale_crop_width_last_pixel =
      out_ary->src_end_x - out_ary->src_start_x;
    } else {
      /*not the rightmost stripe, need to fetch more on the right*/
      out_ary->src_end_x =
      (int) (phase_fg.phase_x_next >> q_factor) - 1 +
      in_ary->postscale_padding + in_ary->prescale_padding;
      out_ary->prescale_crop_width_last_pixel =
      out_ary->src_end_x - out_ary->src_start_x -
      in_ary->prescale_padding;
    }
  }
  out_ary->postscale_crop_width_last_pixel =
  out_ary->postscale_crop_width_first_pixel +
  in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
  /*output width of the scaler should be
    the write stripe width plus some padding required by sharpening*/
  out_ary->scale_output_width =
  in_ary->stripe_block_width[src_tile_index_x_counter];
  if (src_tile_index_x_counter != 0) {
    out_ary->scale_output_width += in_ary->postscale_padding;
  }
  if (src_tile_index_x_counter != in_ary->frame_width_mcus - 1) {
    out_ary->scale_output_width += in_ary->postscale_padding;
  }

  if (in_ary->vertical_scale_ratio < (1 << q_factor) ||
      (in_ary->vertical_scale_ratio == (1 << q_factor) &&
       out_ary->v_init_phase != 0)) {
    /*that is, we are using FIR for scaling in the y direction*/
    out_ary->scale_v_en = 1;
    out_ary->upscale_v_en = 1;
    if (src_tile_index_y_counter) {
      /*not the topmost block, need to fetch more on top*/
      unsigned long long temp = phase_fg.phase_y_cur -
        in_ary->postscale_padding * in_ary->vertical_scale_ratio;
      out_ary->v_init_phase = (int) (temp & q_mask);
      out_ary->src_start_y = (int) (temp >> q_factor) -
        upscale_left_top_padding - in_ary->prescale_padding;

      out_ary->prescale_crop_height_first_line = in_ary->prescale_padding;
      out_ary->postscale_crop_height_first_line =
      in_ary->postscale_padding;
    } else {
      /*the topmost block, top padding is done
        internally by denoise and sharpening but not upscaling*/
      out_ary->v_init_phase = (int) (phase_fg.phase_y_cur & q_mask);
      out_ary->src_start_y =
      (int) ((phase_fg.phase_y_cur) >> q_factor) -
      upscale_left_top_padding;
      out_ary->prescale_crop_height_first_line = 0;
      out_ary->postscale_crop_height_first_line = 0;
    }
    if (src_tile_index_y_counter == in_ary->frame_height_mcus - 1) {
      /*the bottommost stripe, bottom padding is done
        internally by denoise and sharpening but not upscaler*/
      // src_end_x = floor( phase_x_next - PHASE_X_STEP ) + 2
      out_ary->src_end_y =
      (int) ((phase_fg.phase_y_next -
              in_ary->vertical_scale_ratio) >> q_factor) +
      upscale_right_bottom_padding;
      out_ary->prescale_crop_height_last_line =
      out_ary->src_end_y - out_ary->src_start_y;
    } else {
      /*not the bottommost stripe, need to fetch more on the bottom*/
      out_ary->src_end_y =
      (int) ((phase_fg.phase_y_next +
              (in_ary->postscale_padding -
               1) * in_ary->vertical_scale_ratio) >> q_factor) +
      upscale_right_bottom_padding + in_ary->prescale_padding;
      out_ary->prescale_crop_height_last_line =
      out_ary->src_end_y - out_ary->src_start_y -
      in_ary->prescale_padding;
    }
  } else if (in_ary->vertical_scale_ratio > (1 << q_factor)) {
    /*vertical downscaler*/
    out_ary->scale_v_en = 1;
    out_ary->upscale_v_en = 0;
    if (src_tile_index_y_counter) {
      /*not the topmost block, need to fetch more on the top*/
      unsigned long long temp = phase_fg.phase_y_cur -
        in_ary->postscale_padding * in_ary->vertical_scale_ratio;
        out_ary->v_init_phase = (int) (temp & q_mask);
        out_ary->src_start_y = (int) (temp >> q_factor) - in_ary->prescale_padding;

      out_ary->prescale_crop_height_first_line = in_ary->prescale_padding;
      out_ary->postscale_crop_height_first_line =
      in_ary->postscale_padding;
    } else {
      /*the topmost block, top padding internally
        done by denoise and sharpening*/
      out_ary->v_init_phase = (int) (phase_fg.phase_y_cur & q_mask);
      out_ary->src_start_y = (int) ((phase_fg.phase_y_cur) >> q_factor);
      out_ary->prescale_crop_height_first_line = 0;
      out_ary->postscale_crop_height_first_line = 0;
    }

    if (src_tile_index_y_counter == in_ary->frame_height_mcus - 1) {
      if ((phase_fg.phase_y_next & q_mask) >> (q_factor -
                                               downscale_interpolation_resolution)) {
        out_ary->src_end_y = (int) (phase_fg.phase_y_next >> q_factor);
      } else {
        /*need to fetch one pixel less*/
        out_ary->src_end_y = (int) (phase_fg.phase_y_next >> q_factor) - 1;
      }
      out_ary->prescale_crop_height_last_line =
      out_ary->src_end_y - out_ary->src_start_y;
    } else {
      /*not the bottommost block, need to fetch more on the bottom*/
      long long temp =
      phase_fg.phase_y_next +
      in_ary->postscale_padding * in_ary->vertical_scale_ratio;
      if ((temp & q_mask) >>
          (q_factor - downscale_interpolation_resolution)) {
        out_ary->src_end_y =
        (int) (temp >> q_factor) + in_ary->prescale_padding;
      } else {
        /*need to fetch one less*/
        out_ary->src_end_y = (int) (temp >> q_factor) - 1 + in_ary->prescale_padding;
      }
      out_ary->prescale_crop_height_last_line =
      out_ary->src_end_y - out_ary->src_start_y -
      in_ary->prescale_padding;
    }
  } else {
    /*that is, scaling is disabled in the y direction*/
    out_ary->scale_v_en = 0;
    out_ary->upscale_v_en = 0;
    out_ary->v_init_phase = 0;
    if (src_tile_index_y_counter) {
      /*not the topmost block, need to fetch more on top*/
      out_ary->src_start_y =
      (int) (phase_fg.phase_y_cur >> q_factor) -
      in_ary->postscale_padding - in_ary->prescale_padding;
      out_ary->prescale_crop_height_first_line = in_ary->prescale_padding;
      out_ary->postscale_crop_height_first_line =
      in_ary->postscale_padding;
    } else {
      /*the topmost block, top padding is done
        in the denoise and sharpening block*/
      out_ary->src_start_y = (int) (phase_fg.phase_y_cur >> q_factor);
      out_ary->prescale_crop_height_first_line = 0;
      out_ary->postscale_crop_height_first_line = 0;
    }
    if (src_tile_index_y_counter == in_ary->frame_height_mcus - 1) {
      /*the bottommost block, bottom padding
        by the denoise and sharpening block*/
      out_ary->src_end_y = (int) (phase_fg.phase_y_next >> q_factor) - 1;
      out_ary->prescale_crop_height_last_line =
      out_ary->src_end_y - out_ary->src_start_y;
    } else {
      /*not the bottommost block, need to fetch more on the bottom*/
      out_ary->src_end_y =
      (int) ((phase_fg.phase_y_next) >> q_factor) - 1 +
      in_ary->postscale_padding + in_ary->prescale_padding;
      out_ary->prescale_crop_height_last_line =
      out_ary->src_end_y - out_ary->src_start_y -
      in_ary->prescale_padding;
    }
  }
  out_ary->postscale_crop_height_last_line =
  out_ary->postscale_crop_height_first_line +
  in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
  /*output height of the scaler should be the write block
    height plus some padding required by sharpening*/
  out_ary->scale_output_height =
  in_ary->stripe_block_height[src_tile_index_y_counter];
  if (src_tile_index_y_counter != 0) {
    out_ary->scale_output_height += in_ary->postscale_padding;
  }
  if (src_tile_index_y_counter != in_ary->frame_height_mcus - 1) {
    out_ary->scale_output_height += in_ary->postscale_padding;
  }

  /*left boundary*/
  if (out_ary->src_start_x < 0) {
    out_ary->pad_left = 0 - out_ary->src_start_x;
    out_ary->src_start_x = 0;
  } else {
    out_ary->pad_left = 0;
  }

  /*top boundary*/
  if (out_ary->src_start_y < 0) {
    out_ary->pad_top = 0 - out_ary->src_start_y;
    out_ary->src_start_y = 0;
  } else {
    out_ary->pad_top = 0;
  }

  /*right boundary*/
  if (out_ary->src_end_x >= in_ary->src_width) {
    out_ary->pad_right = out_ary->src_end_x - (in_ary->src_width - 1);
    out_ary->src_end_x = (in_ary->src_width - 1);
  } else {
    out_ary->pad_right = 0;
  }

  /*bottom boundary*/
  if (out_ary->src_end_y >= in_ary->src_height) {
    out_ary->pad_bottom = out_ary->src_end_y - (in_ary->src_height - 1);
    out_ary->src_end_y = (in_ary->src_height - 1);
  } else {
    out_ary->pad_bottom = 0;
  }
  /*The leftmost point of the fetch block in byte addresses*/
  out_ary->source_address =
  in_ary->source_address + out_ary->src_start_x * out_ary->bytes_per_pixel +
  out_ary->src_start_y * in_ary->src_stride;
}

void
set_start_of_frame_parameters (struct cpp_plane_info_t *in_ary)
{
  int i;
  /*determine destination stripe width*/
  int block_width = LINE_BUFFER_SIZE - 2 * in_ary->postscale_padding;
  int rotation_block_height;
  int rotation_block_width;
  if (in_ary->horizontal_scale_ratio < (1 << q_factor)
      || (in_ary->horizontal_scale_ratio == (1 << q_factor)
          && (in_ary->horizontal_scale_initial_phase & q_mask))) {
    /* upscaler is used
       crop out asf pixels, upscale pixels, and denoise pixels */
    long long temp =
    LINE_BUFFER_SIZE - 2 * in_ary->prescale_padding -
    upscale_left_top_padding - upscale_right_bottom_padding;
    temp <<= q_factor;
    temp -= q_mask + 1; /* safety margin */
    /*number of pixels that can be produced by upscaler*/
    temp /= in_ary->horizontal_scale_ratio;
    temp += 1; //safety margin
    temp -= 2 * in_ary->postscale_padding;
    if (temp < block_width) {
      block_width = (int) temp;
    }
  } else { /*downscaler or no scaler*/
    /*crop out asf pixels, and denoise pixels*/
    long long temp = LINE_BUFFER_SIZE - 2 * in_ary->prescale_padding;
    temp <<= q_factor;
    temp -= q_mask; /* safety margin */
    /*number of pixels that can be produced by downscaler*/
    temp /= in_ary->horizontal_scale_ratio;
    temp -= 2 * in_ary->postscale_padding;
    if (temp < block_width) {
      block_width = (int) temp;
    }
  }

  if (in_ary->rotate == 0 || in_ary->rotate == 2)// 0 or 180 degree rotation
  {
    /*rotation block height is 2,
      the destination image height will be a multiple of 2.*/
    rotation_block_height = 2;
  } else { /*90 or 270 degree rotation*/
    /*rotation block height is MAL length (32?).
      The destination image height will be a multiple of MAL.*/
    rotation_block_height = MAL_SIZE;
  }
  rotation_block_width = ROTATION_BUFFER_SIZE / rotation_block_height;

  if (block_width > rotation_block_width) {
    block_width = rotation_block_width;
  }

  in_ary->frame_width_mcus =
    (in_ary->dst_width + block_width - 1) / block_width;
  CDBG("MCUS: %d\n", in_ary->frame_width_mcus);

  //Turn off for easier plane debugging
#if 0
  if ((in_ary->input_plane_fmt == PLANE_Y) && (in_ary->frame_width_mcus % 2)) {
    // try to make number of stripes even for Y plane to run faster
    in_ary->frame_width_mcus++;
  }
#endif
  /*evenly distribute the stripe width*/
  in_ary->dst_block_width =
    (in_ary->dst_width + in_ary->frame_width_mcus - 1) /
    in_ary->frame_width_mcus;

  /* number of stripes */
  in_ary->frame_width_mcus =
    (in_ary->dst_width + in_ary->dst_block_width - 1)
    / in_ary->dst_block_width;

  /*destination stripe width*/
  in_ary->stripe_block_width = malloc(sizeof(int) *
                               in_ary->frame_width_mcus);
  if (!in_ary->stripe_block_width){
      CDBG_ERROR("Cannot assign memory to in_ary->stripe_block_width");
      return;
  }

  for (i = 0; i < in_ary->frame_width_mcus; i++) {
    /*First assume all the stripe widths are the same,
    one entry will be changed later*/
    in_ary->stripe_block_width[i] = in_ary->dst_block_width;
  }
  /*The actual destination image height, multiple of rotation block height*/
  in_ary->dst_height_block_aligned =
    ((in_ary->dst_height + rotation_block_height - 1) /
     rotation_block_height) * rotation_block_height;

  CDBG("Aligned block height: %d\n", in_ary->dst_height_block_aligned);

  if (in_ary->dst_height_block_aligned > in_ary->maximum_dst_stripe_height) {
    /*Maximum allowed block height is smaller than destination image height*/
    /*do block processing*/
    CDBG("Block processing?\n");
    in_ary->dst_block_height =
    (in_ary->maximum_dst_stripe_height / rotation_block_height) *
    rotation_block_height;
  } else {
    /*do stripe processing*/
    in_ary->dst_block_height = in_ary->dst_height_block_aligned;
  }

  /*Number of blocks in the vertical direction*/
  in_ary->frame_height_mcus =
    (in_ary->dst_height_block_aligned + in_ary->dst_block_height - 1) /
    in_ary->dst_block_height;
  CDBG("HMCUS: %d\n", in_ary->frame_height_mcus);

  /*destination block height*/
  in_ary->stripe_block_height = malloc(sizeof(int) *
                                in_ary->frame_height_mcus);
  if (!in_ary->stripe_block_height){
      CDBG_ERROR("Cannot assign memory to in_ary->stripe_block_height");
      free(in_ary->stripe_block_width);
      return;
  }

  for (i = 0; i < in_ary->frame_height_mcus; i++) {
    /*First assume all the block heights are the same,
    one entry will be changed later*/
    in_ary->stripe_block_height[i] = in_ary->dst_block_height;
  }

  switch (in_ary->rotate) {
  case 1: /* 90 degree */
    if (in_ary->mirror & 0x1) {
      /*the rightmost destination stripe width is smaller than others*/
      in_ary->stripe_block_width[in_ary->frame_width_mcus - 1] =
      in_ary->dst_width -
      in_ary->dst_block_width * (in_ary->frame_width_mcus - 1);
    } else {
      /*the leftmost destination stripe width is smaller than others*/
      in_ary->horizontal_scale_block_initial_phase -=
      (in_ary->frame_width_mcus * in_ary->dst_block_width -
       in_ary->dst_width) * in_ary->horizontal_scale_ratio;
      in_ary->stripe_block_width[0] =
      in_ary->dst_width -
      in_ary->dst_block_width * (in_ary->frame_width_mcus - 1);
    }
    /*the bottommost block heights are smaller than others.
     fetch a little more on the bottom so padding will be on the right side*/
    in_ary->stripe_block_height[in_ary->frame_height_mcus - 1] =
    in_ary->dst_height_block_aligned -
    in_ary->dst_block_height * (in_ary->frame_height_mcus - 1);

    CDBG("stripe block w: %d h: %d\n",
         in_ary->stripe_block_width[0], in_ary->stripe_block_height[0]);

    in_ary->vertical_scale_block_initial_phase =
    in_ary->vertical_scale_initial_phase;
    break;
  case 2:
    if (in_ary->mirror & 0x1) {
      /*the rightmost destination stripe width is smaller than others*/
      in_ary->stripe_block_width[in_ary->frame_width_mcus - 1] =
      in_ary->dst_width -
      in_ary->dst_block_width * (in_ary->frame_width_mcus - 1);
    } else {
      /*the leftmost destination stripe width is smaller than others*/
      in_ary->horizontal_scale_block_initial_phase -=
      (in_ary->frame_width_mcus * in_ary->dst_block_width -
       in_ary->dst_width) * in_ary->horizontal_scale_ratio;
      in_ary->stripe_block_width[0] =
      in_ary->dst_width -
      in_ary->dst_block_width * (in_ary->frame_width_mcus - 1);
    }
    /*the topmost block heights are smaller than others
      fetch a little more on the top so padding will be on the bottom*/
    in_ary->vertical_scale_initial_phase -=
    (in_ary->dst_height_block_aligned -
     in_ary->dst_height) * in_ary->vertical_scale_ratio;
    in_ary->vertical_scale_block_initial_phase =
    in_ary->vertical_scale_initial_phase -
    (in_ary->frame_height_mcus * in_ary->dst_block_height -
     in_ary->dst_height_block_aligned) * in_ary->vertical_scale_ratio;
    in_ary->stripe_block_height[0] =
    in_ary->dst_height_block_aligned -
    in_ary->dst_block_height * (in_ary->frame_height_mcus - 1);
    break;
  case 3:
    if (in_ary->mirror & 0x1) {
      /*the leftmost destination stripe width is smaller than others*/
      in_ary->horizontal_scale_block_initial_phase -=
      (in_ary->frame_width_mcus * in_ary->dst_block_width -
       in_ary->dst_width) * in_ary->horizontal_scale_ratio;
      in_ary->stripe_block_width[0] =
      in_ary->dst_width -
      in_ary->dst_block_width * (in_ary->frame_width_mcus - 1);
    } else {
      /*the rightmost destination stripe width is smaller than others*/
      in_ary->stripe_block_width[in_ary->frame_width_mcus - 1] =
      in_ary->dst_width -
      in_ary->dst_block_width * (in_ary->frame_width_mcus - 1);
    }
    /*the topmost block heights are smaller than others
      fetch a little more on the top so padding will be on the right*/
    in_ary->vertical_scale_initial_phase -=
    (in_ary->dst_height_block_aligned -
     in_ary->dst_height) * in_ary->vertical_scale_ratio;
    in_ary->vertical_scale_block_initial_phase =
    in_ary->vertical_scale_initial_phase -
    (in_ary->frame_height_mcus * in_ary->dst_block_height -
     in_ary->dst_height_block_aligned) * in_ary->vertical_scale_ratio;
    in_ary->stripe_block_height[0] =
    in_ary->dst_height_block_aligned -
    in_ary->dst_block_height * (in_ary->frame_height_mcus - 1);
    break;
  case 0:
  default:
    if(in_ary->mirror & 0x1) {
      /*the leftmost destination stripe width is smaller than others*/
      in_ary->horizontal_scale_block_initial_phase -=
      (in_ary->frame_width_mcus * in_ary->dst_block_width -
       in_ary->dst_width) * in_ary->horizontal_scale_ratio;
      in_ary->stripe_block_width[0] =
      in_ary->dst_width -
      in_ary->dst_block_width * (in_ary->frame_width_mcus - 1);
    } else {
      /*the rightmost destination stripe width is smaller than others*/
      in_ary->stripe_block_width[in_ary->frame_width_mcus - 1] =
      in_ary->dst_width -
      in_ary->dst_block_width * (in_ary->frame_width_mcus - 1);
    }
    /*the bottommost block heights are smaller than others.
      fetch a little more on the bottom so padding will be on the bottom*/
    in_ary->stripe_block_height[in_ary->frame_height_mcus - 1] =
    in_ary->dst_height_block_aligned -
    in_ary->dst_block_height * (in_ary->frame_height_mcus - 1);
    in_ary->vertical_scale_block_initial_phase =
    in_ary->vertical_scale_initial_phase;
    break;
  }
  in_ary->num_stripes = (uint32_t)
    in_ary->frame_width_mcus * in_ary->frame_height_mcus;
}

void cpp_init_frame_params (struct cpp_plane_info_t *frame)
{
  frame->horizontal_scale_ratio =
  (int)(frame->h_scale_ratio * (1 << q_factor));
  if (frame->horizontal_scale_ratio <= 0) /*Assume scaler is off*/
    frame->horizontal_scale_ratio = 1 << q_factor;

  frame->vertical_scale_ratio =
  (int)(frame->v_scale_ratio * (1 << q_factor));
  if (frame->vertical_scale_ratio <= 0) /*Assume scaler is off*/
    frame->vertical_scale_ratio = 1 << q_factor;

  frame->horizontal_scale_initial_phase =
  (long long) (frame->h_scale_initial_phase * (1 << q_factor));
  frame->vertical_scale_initial_phase =
  (long long)  (frame->v_scale_initial_phase * (1 << q_factor));

  frame->horizontal_scale_block_initial_phase =
  frame->horizontal_scale_initial_phase;
}

void cpp_debug_input_info(struct cpp_plane_info_t *frame)
{
  CDBG("CPP: src_width %d\n", frame->src_width);
  CDBG("CPP: src_height %d\n", frame->src_height);
  CDBG("CPP: src_stride %d\n", frame->src_stride);
  CDBG("CPP: dst_width %d\n", frame->dst_width);
  CDBG("CPP: dst_height %d\n", frame->dst_height);
  CDBG("CPP: dst_stride %d\n", frame->dst_stride);
  CDBG("CPP: rotate %d\n", frame->rotate);
  CDBG("CPP: mirror %d\n", frame->mirror);
  CDBG("CPP: prescale_padding %d\n", frame->prescale_padding);
  CDBG("CPP: postscale_padding %d\n", frame->postscale_padding);
  CDBG("CPP: horizontal_scale_ratio %d\n", frame->horizontal_scale_ratio);
  CDBG("CPP: vertical_scale_ratio %d\n", frame->vertical_scale_ratio);
  CDBG("CPP: horizontal_scale_initial_phase %lld\n",
       frame->horizontal_scale_initial_phase);
  CDBG("CPP: vertical_scale_initial_phase %lld\n",
       frame->vertical_scale_initial_phase);
  CDBG("CPP: maximum_dst_stripe_height %d\n", frame->maximum_dst_stripe_height);
  CDBG("CPP: input_plane_fmt %d\n", frame->input_plane_fmt);
  CDBG("CPP: source_address 0x%x\n", frame->source_address);
  CDBG("CPP: destination_address 0x%x\n", frame->destination_address);
  CDBG("CPP: frame_width_mcus %d\n", frame->frame_width_mcus);
  CDBG("CPP: frame_height_mcus %d\n", frame->frame_height_mcus);
  CDBG("CPP: dst_height_block_aligned %d\n", frame->dst_height_block_aligned);
  CDBG("CPP: dst_block_width %d\n", frame->dst_block_width);
  CDBG("CPP: dst_block_height %d\n", frame->dst_block_height);
  CDBG("CPP: stripe_block_width %d\n", frame->stripe_block_width[0]);
  CDBG("CPP: stripe_block_height %d\n", frame->stripe_block_height[0]);

  CDBG("CPP: horizontal_scale_block_initial_phase %lld\n",
       frame->horizontal_scale_block_initial_phase);
  CDBG("CPP: vertical_scale_block_initial_phase %lld\n",
       frame->vertical_scale_block_initial_phase);
}

void cpp_init_strip_info(struct cpp_plane_info_t *in_info,
                         struct msm_cpp_frame_strip_info *stripe_info,
                         int num_stripes)
{
  struct msm_cpp_frame_strip_info *cur_strip_info;
  int i;
  for (i = 0; i < num_stripes; i++) {
    cur_strip_info = &stripe_info[i];

    if(in_info->prescale_padding == 0 ||
       (in_info->frame_width_mcus == 1 && in_info->frame_height_mcus == 1)) {
      cur_strip_info->prescale_crop_en = 0;
      in_info->bf_crop_enable = 0;
    } else {
      cur_strip_info->prescale_crop_en = 1;
      in_info->bf_crop_enable = 1;
    }

    if(in_info->postscale_padding == 0 ||
       (in_info->frame_width_mcus == 1 && in_info->frame_height_mcus == 1)) {
      cur_strip_info->postscale_crop_en = 0;
      in_info->asf_crop_enable = 0;
    } else {
      cur_strip_info->postscale_crop_en = 1;
      in_info->asf_crop_enable = 1;
    }

    if (in_info->input_plane_fmt == PLANE_Y ||
        in_info->input_plane_fmt == PLANE_CB ||
        in_info->input_plane_fmt == PLANE_CR) {
      cur_strip_info->bytes_per_pixel = 1;
    } else {
      cur_strip_info->bytes_per_pixel = 2;
    }
    cur_strip_info->src_stride = in_info->src_stride;
    cur_strip_info->dst_stride = in_info->dst_stride;

    switch (in_info->rotate) {
    case 1:
      if (in_info->mirror & 0x1) {
        cur_strip_info->horizontal_flip = 0;
      } else {
        cur_strip_info->horizontal_flip = 1;
      }
      cur_strip_info->rotate_270 = 1;
      cur_strip_info->vertical_flip = 1;
      break;
    case 2:
      if (in_info->mirror & 0x1) {
        cur_strip_info->horizontal_flip = 0;
      } else {
        cur_strip_info->horizontal_flip = 1;
      }
      cur_strip_info->rotate_270 = 0;
      cur_strip_info->vertical_flip = 1;
      break;
    case 3:
      if (in_info->mirror & 0x1) {
        cur_strip_info->horizontal_flip = 1;
      } else {
        cur_strip_info->horizontal_flip = 0;
      }
      cur_strip_info->rotate_270 = 1;
      cur_strip_info->vertical_flip = 0;
      break;
    case 0:
    default:
      cur_strip_info->horizontal_flip = in_info->mirror & 0x1;
      cur_strip_info->vertical_flip = in_info->mirror >> 1;
      cur_strip_info->rotate_270 = 0;
      cur_strip_info->vertical_flip = 0;
      break;
    }
  }
}

void cpp_debug_strip_info(struct msm_cpp_frame_strip_info *strip_info, int n)
{
  CDBG("CPP FRAME STRIP %d", n);

  /*Is vertical scale enabled?*/
  CDBG("CPP scale_v_en 0x%x\n", strip_info->scale_v_en);
  /*Is horizontal scale enabled?*/
  CDBG("CPP scale_h_en 0x%x\n", strip_info->scale_h_en);
  /*Leftmost position of the fetch block in terms of pixels*/
  CDBG("CPP src_start_x 0x%x\n", strip_info->src_start_x);
  /*Rightmost position of the fetch block in terms of pixels*/
  CDBG("CPP src_end_x 0x%x\n", strip_info->src_end_x);
  /*Topmost position of the fetch block in terms of pixels*/
  CDBG("CPP src_start_y 0x%x\n", strip_info->src_start_y);
  /*Bottommost positin of the fetch block in terms of pixels*/
  CDBG("CPP src_end_y 0x%x\n", strip_info->src_end_y);
  /*The amount padded at the fetch block left boundary in terms of pixels*/
  CDBG("CPP pad_left 0x%x\n", strip_info->pad_left);
  /*The amount padded at the fetch block right boundary in terms of pixels*/
  CDBG("CPP pad_right 0x%x\n", strip_info->pad_right);
  /*The amount padded at the fetch block top boundary in terms of pixels*/
  CDBG("CPP pad_top 0x%x\n", strip_info->pad_top);
  /*The amount padded at the fetch block bottom boundary in terms of pixels*/
  CDBG("CPP pad_bottom 0x%x\n", strip_info->pad_bottom);
  /*Leftmost sampling position, range (0, 1)*/
  CDBG("CPP h_init_phase 0x%x\n", strip_info->h_init_phase);
  /*Topmost sampling position, range (0, 1)*/
  CDBG("CPP v_init_phase 0x%x\n", strip_info->v_init_phase);
  /*Horizontal sampling distance, Q21 number*/
  CDBG("CPP h_phase_step 0x%x\n", strip_info->h_phase_step);
  /*Vertical sampling distance, Q21 number*/
  CDBG("CPP v_phase_step 0x%x\n", strip_info->v_phase_step);
  /*Leftmost position of the cropping window right after denoise block*/
  CDBG("CPP prescale_crop_width_first_pixel 0x%x\n", strip_info->prescale_crop_width_first_pixel);
  /*Rightmost position of the cropping window right after denoise block*/
  CDBG("CPP prescale_crop_width_last_pixel 0x%x\n", strip_info->prescale_crop_width_last_pixel);
  /*Topmost position of the cropping window right after denoise block*/
  CDBG("CPP prescale_crop_height_first_line 0x%x\n", strip_info->prescale_crop_height_first_line);
  /*Bottommost position of the cropping window right after denoise block*/
  CDBG("CPP prescale_crop_height_last_line 0x%x\n", strip_info->prescale_crop_height_last_line);
  /*Leftmost position of the cropping window right after sharpening block*/
  CDBG("CPP postscale_crop_width_first_pixel 0x%x\n", strip_info->postscale_crop_width_first_pixel);
  /*Rightmost position of the cropping window right after sharpening block*/
  CDBG("CPP postscale_crop_width_last_pixel 0x%x\n", strip_info->postscale_crop_width_last_pixel);
  /*Topmost position of the cropping window right after denoise block*/
  CDBG("CPP postscale_crop_height_first_line 0x%x\n", strip_info->postscale_crop_height_first_line);
  /*Bottommost position of the cropping window right after denoise block*/
  CDBG("CPP postscale_crop_height_last_line 0x%x\n", strip_info->postscale_crop_height_last_line);
  /*Leftmost position of the write block in terms of pixels*/
  CDBG("CPP dst_start_x 0x%x\n", strip_info->dst_start_x);
  /*Rightmost position of the write block in terms of pixels*/
  CDBG("CPP dst_end_x 0x%x\n", strip_info->dst_end_x);
  /*Topmost position of the write block in terms of pixels*/
  CDBG("CPP dst_start_y 0x%x\n", strip_info->dst_start_y);
  /*Bottommost position of the write block in terms of pixels*/
  CDBG("CPP dst_end_y 0x%x\n", strip_info->dst_end_y);
  /*1-> Planar, 2->CbCr*/
  CDBG("CPP bytes_per_pixel 0x%x\n", strip_info->bytes_per_pixel);
  /*Topleft corner of the fetch block in terms of memory address*/
  CDBG("CPP source_address 0x%x\n", strip_info->source_address);
  /*Topleft corner of the write block in terms of memory address*/
  CDBG("CPP destination_address 0x%x\n", strip_info->destination_address);
  /*Source image stride in terms of bytes*/
  CDBG("CPP src_stride 0x%x\n", strip_info->src_stride);
  /*Destination image stride in terms of bytes*/
  CDBG("CPP dst_stride 0x%x\n", strip_info->dst_stride);
  /*Do we rotate 270 degree or not?*/
  CDBG("CPP rotate_270 0x%x\n", strip_info->rotate_270);
  /*Do we follow rotation with horizontal flip?*/
  CDBG("CPP horizontal_flip 0x%x\n", strip_info->horizontal_flip);
  /*Do we follow rotation with vertical flip?*/
  CDBG("CPP vertical_flip 0x%x\n", strip_info->vertical_flip);
  /*Scaler output width*/
  CDBG("CPP scale_output_width 0x%x\n", strip_info->scale_output_width);
  /*Scaler output height*/
  CDBG("CPP scale_output_height 0x%x\n", strip_info->scale_output_height);
  /*Is vertical upscale enabled?*/
  CDBG("CPP upscale_v_en 0x%x\n", strip_info->upscale_v_en);
  /*Is horizontal upscale enabled?*/
  CDBG("CPP upscale_h_en 0x%x\n", strip_info->upscale_h_en);
}

void cpp_debug_fetch_engine_info(struct cpp_fe_info *fe_info)
{
  CDBG("buffer_ptr   : %d\n", fe_info->buffer_ptr);
  CDBG("buffer_width : %d\n", fe_info->buffer_width);
  CDBG("buffer_height: %d\n", fe_info->buffer_height);
  CDBG("buffer_stride: %d\n", fe_info->buffer_stride);
  CDBG("block_width  : %d\n", fe_info->block_width);
  CDBG("block_height : %d\n", fe_info->block_height);
  CDBG("left_pad     : %d\n", fe_info->left_pad);
  CDBG("right_pad    : %d\n", fe_info->right_pad);
  CDBG("top_pad      : %d\n", fe_info->top_pad);
  CDBG("bottom_pad   : %d\n", fe_info->bottom_pad);
}

void cpp_prepare_fetch_engine_info(struct cpp_plane_info_t *plane_info,
  int stripe_num)
{
  struct cpp_stripe_info *stripe_info1 =
    &plane_info->stripe_info1[stripe_num];
  struct msm_cpp_frame_strip_info *stripe_info =
    &plane_info->stripe_info[stripe_num];
  struct cpp_fe_info *fe_info = &stripe_info1->fe_info;

  fe_info->buffer_ptr = stripe_info->source_address;
  fe_info->buffer_width =
    stripe_info->src_end_x - stripe_info->src_start_x + 1;
  fe_info->buffer_height =
    stripe_info->src_end_y - stripe_info->src_start_y + 1;
  fe_info->buffer_stride = stripe_info->src_stride;
  fe_info->left_pad = stripe_info->pad_left;
  fe_info->right_pad = stripe_info->pad_right;
  fe_info->top_pad = stripe_info->pad_top;
  fe_info->bottom_pad = stripe_info->pad_bottom;
  if (plane_info->rotate == 1 || plane_info->rotate == 3) {
    if (plane_info->mirror & 0x1) {
      fe_info->top_pad = stripe_info->pad_top;
      fe_info->bottom_pad = stripe_info->pad_bottom;
    } else {
      fe_info->top_pad = stripe_info->pad_bottom;
      fe_info->bottom_pad = stripe_info->pad_top;
    }
  }
/*
  if (plane_info->input_plane_fmt == PLANE_Y) {
    fe_info->top_pad = 16;
  } else {
    fe_info->top_pad = 8;
  }
  fe_info->bottom_pad = 0;
*/
  fe_info->block_width =
    fe_info->buffer_width + fe_info->left_pad + fe_info->right_pad;
  fe_info->block_height =
    fe_info->buffer_height + fe_info->top_pad + fe_info->bottom_pad;
  cpp_debug_fetch_engine_info(fe_info);
}

void cpp_debug_bf_info(struct cpp_bf_info *bf_info)
{
  int i;
  for (i = 0; i < 4; i++) {
    CDBG("bilateral_scale[%d]: %f\n", i, bf_info->bilateral_scale[i]);
    CDBG("noise_threshold[%d]: %f\n", i, bf_info->noise_threshold[i]);
    CDBG("weight[%d]: %f\n", i, bf_info->weight[i]);
    if (i != 3) {
      CDBG("bf cfg0: 0x%x\n", (uint32_t) (bf_info->bilateral_scale[i] * (1 << 10)));
    } else {
      CDBG("bf cfg0: 0x%x\n", (uint32_t) (bf_info->bilateral_scale[i] * (1 << 8)));
    }
    CDBG("bf cfg1: 0x%x\n", (uint32_t)
         ((uint32_t)(bf_info->noise_threshold[i] * (1 << 4))) << 8 |
         ((uint32_t)(bf_info->weight[i] * (1 << 4))));
  }
}

void cpp_prepare_bf_info(struct cpp_frame_info_t *frame_info)
{
  int i, j;
  struct cpp_bf_info *bf_info;
  for (j = 0; j < 3; j++) {
    bf_info = &frame_info->bf_info[j];
    for (i = 0; i < 4; i++) {
      if (i != 3) {
        bf_info->bilateral_scale[i] = ((double) 64.0/3.0) /
          (sqrt((double) frame_info->edge_softness[j][i] /1.31) *
          frame_info->noise_profile[j][i]) / 9;
      } else {
        bf_info->bilateral_scale[i] = ((double) 64.0/3.0) /
          (sqrt((double) frame_info->edge_softness[j][i] /1.31) *
          frame_info->noise_profile[j][i]);
      }

      bf_info->weight[i] = 1.0 - frame_info->weight[j][i];
      bf_info->noise_threshold[i] = frame_info->denoise_ratio[j][i] *
        frame_info->noise_profile[j][i];
    }
    cpp_debug_bf_info(bf_info);
  }
}

void cpp_debug_asf_info(struct cpp_asf_info *asf_info)
{
  int i;
  CDBG("%s sp: %d\n", __func__, asf_info->sp);
  CDBG("neg_abs_y1: %d\n", asf_info->neg_abs_y1);
  CDBG("dyna_clamp_en: %d\n", asf_info->dyna_clamp_en);
  CDBG("sp_eff_en: %d\n", asf_info->sp_eff_en);
  CDBG("clamp_h_ul: %d\n", asf_info->clamp_h_ul);
  CDBG("clamp_h_ll: %d\n", asf_info->clamp_h_ll);
  CDBG("clamp_v_ul: %d\n", asf_info->clamp_v_ul);
  CDBG("clamp_v_ll: %d\n", asf_info->clamp_v_ll);
  CDBG("clamp_offset_max: %d\n", asf_info->clamp_offset_max);
  CDBG("clamp_offset_min: %d\n", asf_info->clamp_offset_min);
  CDBG("clamp_scale_max: %f\n", asf_info->clamp_scale_max);
  CDBG("clamp_scale_min: %f\n", asf_info->clamp_scale_min);
  CDBG("nz_flag: %d\n", asf_info->nz_flag);
  CDBG("sobel_h_coeff\n");
  for (i = 0; i < 4; i++) {
    CDBG("%f %f %f %f\n",
         asf_info->sobel_h_coeff[i*4], asf_info->sobel_h_coeff[i*4+1],
         asf_info->sobel_h_coeff[i*4+2], asf_info->sobel_h_coeff[i*4+3]);
  }
  CDBG("sobel_v_coeff\n");
  for (i = 0; i < 4; i++) {
    CDBG("%f %f %f %f\n",
         asf_info->sobel_v_coeff[i*4], asf_info->sobel_v_coeff[i*4+1],
         asf_info->sobel_v_coeff[i*4+2], asf_info->sobel_v_coeff[i*4+3]);
  }
  CDBG("hpf_h_coeff\n");
  for (i = 0; i < 4; i++) {
    CDBG("%f %f %f %f\n",
         asf_info->hpf_h_coeff[i*4], asf_info->hpf_h_coeff[i*4+1],
         asf_info->hpf_h_coeff[i*4+2], asf_info->hpf_h_coeff[i*4+3]);
  }
  CDBG("hpf_v_coeff\n");
  for (i = 0; i < 4; i++) {
    CDBG("%f %f %f %f\n",
         asf_info->hpf_v_coeff[i*4], asf_info->hpf_v_coeff[i*4+1],
         asf_info->hpf_v_coeff[i*4+2], asf_info->hpf_v_coeff[i*4+3]);
  }
  CDBG("lpf_coeff\n");
  for (i = 0; i < 4; i++) {
    CDBG("%f %f %f %f\n",
         asf_info->lpf_coeff[i*4], asf_info->lpf_coeff[i*4+1],
         asf_info->lpf_coeff[i*4+2], asf_info->lpf_coeff[i*4+3]);
  }
  CDBG("lut1\n");
  for (i = 0; i < 6; i++) {
    CDBG("%f %f %f %f\n",
         asf_info->lut1[i*4], asf_info->lut1[i*4+1],
         asf_info->lut1[i*4+2], asf_info->lut1[i*4+3]);
  }
  CDBG("lut2\n");
    for (i = 0; i < 6; i++) {
      CDBG("%f %f %f %f\n",
         asf_info->lut2[i*4], asf_info->lut2[i*4+1],
         asf_info->lut2[i*4+2], asf_info->lut2[i*4+3]);
  }
  CDBG("lut3\n");
  for (i = 0; i < 3; i++) {
    CDBG("%f %f %f %f\n",
         asf_info->lut3[i*4], asf_info->lut3[i*4+1],
         asf_info->lut3[i*4+2], asf_info->lut3[i*4+3]);
  }
}

void cpp_prepare_asf_info(struct cpp_frame_info_t *frame_info)
{
  int i;
  struct cpp_asf_info *asf_info = &frame_info->asf_info;

  cpp_debug_asf_info(asf_info);
}

void cpp_debug_stripe_scale_info(struct cpp_stripe_scale_info *scale_info)
{
  CDBG("block_width: %d\n", scale_info->block_width);
  CDBG("block_height: %d\n", scale_info->block_height);
  CDBG("phase_h_init: %d\n", scale_info->phase_h_init);
}

void cpp_prepare_stripe_scale_info(struct cpp_plane_info_t *plane_info,
  int stripe_num)
{
  struct cpp_stripe_info *stripe_info1 =
    &plane_info->stripe_info1[stripe_num];
  struct msm_cpp_frame_strip_info *stripe_info =
    &plane_info->stripe_info[stripe_num];
  struct cpp_stripe_scale_info *stripe_scale_info = &stripe_info1->scale_info;

/*
  if (stripe_num == 0) {
    stripe_scale_info->block_width =
      stripe_info->postscale_crop_width_last_pixel + 1;
    stripe_scale_info->block_width = 480;
  } else if (stripe_num == (plane_info->num_stripes - 1)) {
    stripe_scale_info->block_width = stripe_info->scale_output_width;
  } else {
    stripe_scale_info->block_width =
    ((stripe_info->postscale_crop_width_last_pixel -
     stripe_info->postscale_crop_width_first_pixel + 1) *
    ((double) (1 << q_factor) / stripe_info->h_phase_step));
    if (stripe_scale_info->block_width > 512) {
      stripe_scale_info->block_width = 512;
    }
    stripe_scale_info->block_width = 489;
  }

  stripe_scale_info->block_width = stripe_info->scale_output_width;

  ((double)(((stripe_info->prescale_crop_width_last_pixel -
   stripe_info->prescale_crop_width_first_pixel + 1 - 4) << 21)
   - stripe_info->h_init_phase) / stripe_info->h_phase_step) + 1);

  if (stripe_num != 0 && stripe_num != (plane_info->num_stripes - 1)) {
    stripe_scale_info->block_width = (int)
      ((double)(((stripe_info->prescale_crop_width_last_pixel -
     stripe_info->prescale_crop_width_first_pixel + 1 - 4) << 21)
     - stripe_info->h_init_phase) / stripe_info->h_phase_step) + 1;
    stripe_scale_info->block_width = 208;
  }
*/

/*
  stripe_scale_info->block_width = (int)
    ((double)(((stripe_info->prescale_crop_width_last_pixel -
   stripe_info->prescale_crop_width_first_pixel + 1 - 4) << 21)
   - stripe_info->h_init_phase) / stripe_info->h_phase_step) + 1;
*/

/*
  stripe_scale_info->block_width = 480;
  if (stripe_num > 0) {
    stripe_scale_info->block_width = 512;
  }
  if (stripe_num == 3) {
    stripe_scale_info->block_width = 484;
  }
*/
//    stripe_info->postscale_crop_width_last_pixel + 1;
//    (stripe_info->postscale_crop_width_last_pixel -
//     stripe_info->postscale_crop_width_first_pixel + 1);// *
//    ((double) (1 << q_factor) / stripe_info->h_phase_step)) - 1;
/*
  stripe_scale_info->block_height =
    ((double)(stripe_info->prescale_crop_height_last_line -
     stripe_info->prescale_crop_height_first_line + 1) *
    ((double) (1 << q_factor) / stripe_info->v_phase_step));
*/
  stripe_scale_info->block_width = stripe_info->scale_output_width;
  stripe_scale_info->block_height = stripe_info->scale_output_height;
  stripe_scale_info->phase_h_init = stripe_info->h_init_phase;
  cpp_debug_stripe_scale_info(stripe_scale_info);
}

void cpp_debug_plane_scale_info(struct cpp_plane_scale_info *scale_info)
{
  CDBG("v_scale_fir_algo: %d\n", scale_info->v_scale_fir_algo);
  CDBG("h_scale_fir_algo: %d\n", scale_info->h_scale_fir_algo);
  CDBG("v_scale_algo: %d\n", scale_info->v_scale_algo);
  CDBG("h_scale_algo: %d\n", scale_info->h_scale_algo);
  CDBG("subsample_en: %d\n", scale_info->subsample_en);
  CDBG("upsample_en: %d\n", scale_info->upsample_en);
  CDBG("vscale_en: %d\n", scale_info->vscale_en);
  CDBG("hscale_en: %d\n", scale_info->hscale_en);
  CDBG("phase_h_step: %d\n", scale_info->phase_h_step);
  CDBG("phase_v_init: %d\n", scale_info->phase_v_init);
  CDBG("phase_v_step: %d\n", scale_info->phase_v_step);
}

void cpp_prepare_plane_scale_info(struct cpp_plane_info_t *plane_info)
{
  struct cpp_stripe_info *stripe_info1 =
    &plane_info->stripe_info1[0];
  struct msm_cpp_frame_strip_info *stripe_info =
    &plane_info->stripe_info[0];
  struct cpp_plane_scale_info *scale_info = &plane_info->scale_info;

  scale_info->vscale_en = stripe_info->scale_v_en;
  scale_info->hscale_en = stripe_info->scale_h_en;

  scale_info->v_scale_algo = stripe_info->upscale_v_en;
  scale_info->h_scale_algo = stripe_info->upscale_h_en;

  scale_info->v_scale_fir_algo = 0;
  scale_info->h_scale_fir_algo = 0;

  scale_info->subsample_en = 1;
  scale_info->upsample_en = 1;

  scale_info->phase_h_step = stripe_info->h_phase_step;
  scale_info->phase_v_init = stripe_info->v_init_phase;
  scale_info->phase_v_step = stripe_info->v_phase_step;
  cpp_debug_plane_scale_info(scale_info);
}

void cpp_debug_crop_info(struct cpp_crop_info *crop_info)
{
  CDBG("enable: %d\n", crop_info->enable);
  CDBG("first_pixel: %d\n", crop_info->first_pixel);
  CDBG("last_pixel: %d\n", crop_info->last_pixel);
  CDBG("first_line: %d\n", crop_info->first_line);
  CDBG("last_line: %d\n", crop_info->last_line);
}

void cpp_prepare_crop_info(struct cpp_plane_info_t *plane_info,
  int stripe_num)
{
  struct cpp_stripe_info *stripe_info1 =
    &plane_info->stripe_info1[stripe_num];
  struct msm_cpp_frame_strip_info *stripe_info =
    &plane_info->stripe_info[stripe_num];
  struct cpp_crop_info *bf_crop_info = &stripe_info1->bf_crop_info;
  struct cpp_crop_info *asf_crop_info = &stripe_info1->asf_crop_info;

  bf_crop_info->enable = stripe_info->prescale_crop_en;
  bf_crop_info->first_pixel = stripe_info->prescale_crop_width_first_pixel;
  bf_crop_info->last_pixel = stripe_info->prescale_crop_width_last_pixel;
  bf_crop_info->first_line = stripe_info->prescale_crop_height_first_line;
  bf_crop_info->last_line = stripe_info->prescale_crop_height_last_line;

  asf_crop_info->enable = stripe_info->postscale_crop_en;
  asf_crop_info->first_pixel = stripe_info->postscale_crop_width_first_pixel;
  asf_crop_info->last_pixel = stripe_info->postscale_crop_width_last_pixel;
  asf_crop_info->first_line = stripe_info->postscale_crop_height_first_line;
  asf_crop_info->last_line = stripe_info->postscale_crop_height_last_line;

  CDBG("BF Crop info\n");
  cpp_debug_crop_info(bf_crop_info);
  CDBG("ASF Crop info\n");
  cpp_debug_crop_info(asf_crop_info);
}

void cpp_debug_rotation_info(struct cpp_rotator_info *rot_info)
{
  CDBG("rot_cfg: %d\n", rot_info->rot_cfg);
  CDBG("block_width: %d\n", rot_info->block_width);
  CDBG("block_height: %d\n", rot_info->block_height);
  CDBG("block_size: %d\n", rot_info->block_size);
  CDBG("rowIndex0: %d\n", rot_info->rowIndex0);
  CDBG("rowIndex1: %d\n", rot_info->rowIndex1);
  CDBG("colIndex0: %d\n", rot_info->colIndex0);
  CDBG("colIndex1: %d\n", rot_info->colIndex1);
  CDBG("initIndex: %d\n", rot_info->initIndex);
  CDBG("modValue: %d\n", rot_info->modValue);
}

void cpp_prepare_rotation_info(struct cpp_plane_info_t *plane_info,
  int stripe_num)
{
  struct cpp_stripe_info *stripe_info =
    &plane_info->stripe_info1[stripe_num];
  uint16_t block_width, block_height;
  uint32_t block_size;
  struct cpp_rotator_info *rot_info = &stripe_info->rot_info;
  rot_info->block_width = stripe_info->asf_crop_info.last_pixel -
    stripe_info->asf_crop_info.first_pixel + 1;
  rot_info->block_height = 2;

  if(stripe_info->rotation == 0) {
    rot_info->rot_cfg = ROT_0 + stripe_info->mirror;
  } else if (stripe_info->rotation == 1) {
    rot_info->rot_cfg = ROT_90 + stripe_info->mirror;
    //rot_info->rot_cfg = ROT_90_HV_FLIP - stripe_info->mirror;
    rot_info->block_height = 32;
  } else if (stripe_info->rotation == 2) {
    rot_info->rot_cfg = ROT_0_HV_FLIP - stripe_info->mirror;
  } else if (stripe_info->rotation == 3) {
   rot_info->rot_cfg = ROT_90_HV_FLIP - stripe_info->mirror;
   //rot_info->rot_cfg = ROT_90 + stripe_info->mirror;
    rot_info->block_height = 32;
  }

/*
  rot_info->rot_cfg =
    plane_info->stripe_info[stripe_num].rotate_270 << 2 |
    plane_info->stripe_info[stripe_num].vertical_flip << 1|
    plane_info->stripe_info[stripe_num].horizontal_flip;

  if (plane_info->stripe_info[stripe_num].rotate_270) {
    rot_info->block_height = 32;
  }
*/
  rot_info->block_size = rot_info->block_width * rot_info->block_height;
  block_width = rot_info->block_width;
  block_height = rot_info->block_height;
  block_size = rot_info->block_size;

  switch (rot_info->rot_cfg) {
  case ROT_0:
    rot_info->rowIndex0 = 1;
    rot_info->rowIndex1 = 0;
    rot_info->colIndex0 = 1;
    rot_info->colIndex1 = 0;
    rot_info->modValue = block_size + 1;
    rot_info->initIndex = 0;
    break;
  case ROT_0_H_FLIP:
    rot_info->rowIndex0 = 0;
    rot_info->rowIndex1 = 1;
    rot_info->colIndex0 = (block_width * 2)-1;
    rot_info->colIndex1 = 0;
    rot_info->modValue = block_size + 1;
    rot_info->initIndex = block_width - 1;
    break;
  case ROT_0_V_FLIP:
    rot_info->rowIndex0 = 1;
    rot_info->rowIndex1 = 0;
    rot_info->colIndex0 = (block_height - 2) * block_width;
    rot_info->colIndex1 = block_size - 1;
    rot_info->modValue = block_size + 1;
    rot_info->initIndex = (block_height - 1) * block_width;
    break;
  case ROT_0_HV_FLIP:
    rot_info->rowIndex0 = 0;
    rot_info->rowIndex1 = 1;
    rot_info->colIndex0 = 0;
    rot_info->colIndex1 = 1;
    rot_info->modValue = block_size + 1;
    rot_info->initIndex = block_size - 1;
    break;
  case ROT_90:
    rot_info->rowIndex0 = 0;
    rot_info->rowIndex1 = block_width;
    rot_info->colIndex0 = ((block_height - 1) * block_width) + 1;
    rot_info->colIndex1 = 0;
    rot_info->modValue = block_size + 1;
    rot_info->initIndex = (block_height - 1) * block_width;
    break;
  case ROT_90_H_FLIP:
    rot_info->rowIndex0 = block_width;
    rot_info->rowIndex1 = 0;
    rot_info->colIndex0 = 1;
    rot_info->colIndex1 = (block_height - 1) * block_width;
    rot_info->modValue = block_size - 1;
    rot_info->initIndex = 0;
    break;
  case ROT_90_V_FLIP:
    rot_info->rowIndex0 = ((block_height - 1) * block_width) - 1;
    rot_info->rowIndex1 = block_size - 1;
    rot_info->colIndex0 = block_size - 2;
    rot_info->colIndex1 = block_width - 1;
    rot_info->modValue = block_size - 1;
    rot_info->initIndex = block_size - 1;
    break;
  case ROT_90_HV_FLIP:
    rot_info->rowIndex0 = (block_width * 2) - 1;
    rot_info->rowIndex1 = block_width - 1;
    rot_info->colIndex0 = block_width - 2;
    rot_info->colIndex1 = block_size - 1;
    rot_info->modValue = block_size + 1;
    rot_info->initIndex = block_width - 1;
    break;
  }
  cpp_debug_rotation_info(rot_info);
}

void cpp_debug_write_engine_info(struct cpp_we_info *we_info)
{
  CDBG("buffer_ptr[0]: %d\n", we_info->buffer_ptr[0]);
  CDBG("buffer_ptr[1]: %d\n", we_info->buffer_ptr[1]);
  CDBG("buffer_ptr[2]: %d\n", we_info->buffer_ptr[2]);
  CDBG("buffer_ptr[3]: %d\n", we_info->buffer_ptr[3]);
  CDBG("buffer_width: %d\n", we_info->buffer_width);
  CDBG("buffer_height: %d\n", we_info->buffer_height);
  CDBG("buffer_stride: %d\n", we_info->buffer_stride);
  CDBG("blocks_per_col: %d\n", we_info->blocks_per_col);
  CDBG("blocks_per_row: %d\n", we_info->blocks_per_row);
  CDBG("h_step: %d\n", we_info->h_step);
  CDBG("v_step: %d\n", we_info->v_step);
  CDBG("h_init: %d\n", we_info->h_init);
  CDBG("v_init: %d\n", we_info->v_init);
}

void cpp_prepare_write_engine_info(struct cpp_plane_info_t *plane_info,
  int stripe_num)
{
  struct cpp_stripe_info *stripe_info1 =
    &plane_info->stripe_info1[stripe_num];
  struct msm_cpp_frame_strip_info *stripe_info =
    &plane_info->stripe_info[stripe_num];
  struct cpp_rotator_info *rot_info = &stripe_info1->rot_info;
  struct cpp_we_info *we_info = &stripe_info1->we_info;
  int blocks_per_stripe = ceil((float)
    (stripe_info1->asf_crop_info.last_line -
     stripe_info1->asf_crop_info.first_line + 1)/ rot_info->block_height);
  int output_block_height, output_block_width;
  int h_pix_offset, v_pix_offset;

  if (rot_info->rot_cfg < ROT_90) {
    output_block_height = rot_info->block_height;
    output_block_width = rot_info->block_width * stripe_info->bytes_per_pixel;
    we_info->buffer_width = output_block_width;
    //we_info->buffer_height = stripe_info->dst_end_y - stripe_info->dst_start_y + 1;
    we_info->buffer_height = output_block_height * blocks_per_stripe;
    we_info->buffer_stride = stripe_info->dst_stride;
    we_info->buffer_ptr[0] = stripe_info->destination_address;
    we_info->buffer_ptr[1] = we_info->buffer_ptr[0];
    we_info->buffer_ptr[2] = we_info->buffer_ptr[0];
    we_info->buffer_ptr[3] = we_info->buffer_ptr[0];
    we_info->blocks_per_col = blocks_per_stripe;
    we_info->blocks_per_row = 1;
  } else {
    output_block_height = rot_info->block_width;
    output_block_width = rot_info->block_height * stripe_info->bytes_per_pixel;
    we_info->buffer_width = (output_block_width * blocks_per_stripe);
    we_info->buffer_height = output_block_height;
    we_info->buffer_stride = stripe_info->dst_stride;
    we_info->buffer_ptr[0] = stripe_info->destination_address;
/*
    if (rot_info->rot_cfg == ROT_90_H_FLIP ||
         rot_info->rot_cfg == ROT_90_HV_FLIP) {
      we_info->buffer_ptr[0] -= stripe_info1->fe_info.top_pad *
        stripe_info->bytes_per_pixel;
    }
*/
    we_info->buffer_ptr[1] = we_info->buffer_ptr[0];
    we_info->buffer_ptr[2] = we_info->buffer_ptr[0];
    we_info->buffer_ptr[3] = we_info->buffer_ptr[0];
    we_info->blocks_per_col = 1;
    we_info->blocks_per_row = blocks_per_stripe;
  }

  h_pix_offset = (blocks_per_stripe - 1) * output_block_width;
  v_pix_offset = (blocks_per_stripe - 1) * output_block_height;

  switch (rot_info->rot_cfg) {
  case ROT_0:
  case ROT_0_H_FLIP:
  case ROT_90_H_FLIP:
  case ROT_90_HV_FLIP:
    we_info->h_init = 0;
    we_info->v_init = 0;
    we_info->h_step = output_block_width;
    we_info->v_step = output_block_height;
    break;
  case ROT_0_V_FLIP:
  case ROT_0_HV_FLIP:
    we_info->h_init = 0;
    we_info->v_init = v_pix_offset;
    we_info->h_step = output_block_width;
    we_info->v_step = output_block_height * -1;
    break;
  case ROT_90:
  case ROT_90_V_FLIP:
    we_info->h_init = h_pix_offset;
    we_info->v_init = 0;
    we_info->h_step = output_block_width * -1;
    we_info->v_step = output_block_height;
    break;
  }
  cpp_debug_write_engine_info(we_info);
}

void cpp_create_frame_message(struct msm_cpp_frame_info_t *frame_info,
                              struct cpp_frame_info_t *in_frame_info)
{
  struct cpp_stripe_info *stripe_info;
  int i, j, k, num_stripes = 0, msg_len;
  uint32_t *frame_msg;
  //in_frame_info->plane_info[0].num_stripes = 4;
  //in_frame_info->plane_info[0].num_stripes = 4;
  //in_frame_info->plane_info[1].num_stripes = 0;
  for (i =0; i < in_frame_info->num_planes; i++) {
    num_stripes += in_frame_info->plane_info[i].num_stripes;
  }
  /*frame info size*/
  msg_len = 126 + 27 * num_stripes + 3;
  /*Total message size = frame info + header + length + trailer*/
  frame_msg = malloc(sizeof(uint32_t) * msg_len);
  if (!frame_msg){
      CDBG_ERROR("Cannot assign memory to frame_msg");
      return;
  }

  memset(frame_msg, 0, sizeof(uint32_t) * msg_len);

  frame_info[0].cpp_cmd_msg = frame_msg;
  frame_info[0].msg_len = msg_len;

  /*Top Level*/
  frame_msg[0] = 0x3E646D63;
  frame_msg[1] = msg_len - 3;
  frame_msg[2] = 0x6;
  frame_msg[3] = 0;
  frame_msg[4] = 0;
  /*Plane info*/
  frame_msg[5] = (in_frame_info->in_plane_fmt << 24) | (in_frame_info->out_plane_fmt << 16) |
    in_frame_info->plane_info[0].stripe_info1[0].rot_info.rot_cfg << 13 |
    1 << 1 | 0;
  /*Plane address HFR*/
  frame_msg[6] = 0x0;
  frame_msg[7] = 0x0;
  frame_msg[8] = 0x0;
  /*Output Plane HFR*/
  frame_msg[9] = 0x0;
  frame_msg[10] = 0x0;
  frame_msg[11] = 0x0;

  for (i =0; i < in_frame_info->num_planes; i++) {
    frame_msg[12] |= in_frame_info->plane_info[i].num_stripes << (i * 10);
  }

  struct cpp_asf_info *asf_info = &in_frame_info->asf_info;
  for (i = 0; i < 12; i++) {
    frame_msg[13+i] =
      (((int) asf_info->lut1[i*2+1] * (1 << 6)) << 16) |
      ((int) (asf_info->lut1[i*2] * (1 << 5)) & 0xFFFF);
    frame_msg[25+i] =
      (((int) asf_info->lut2[i*2+1] * (1 << 6)) << 16) |
      ((int) (asf_info->lut2[i*2] * (1 << 5)) & 0xFFFF);
  }
  for (i = 0; i < 6; i++) {
    frame_msg[37+i] =
      (((int) asf_info->lut3[i*2+1] * (1 << 8)) << 16) |
      ((int) (asf_info->lut3[i*2] * (1 << 6)) & 0x7FFF);
  }

  for (i=0; i < 3; i++) {
    struct cpp_bf_info *bf_info = &in_frame_info->bf_info[i];
    j= 43 + i * 8;
    for (j = 0; j < 4; j++) {
      k = 43 + i * 8 + j;
      if (j != 3) {
        frame_msg[k] = (uint32_t) (bf_info->bilateral_scale[j] * (1 << 10));
      } else {
        frame_msg[k] = (uint32_t) (bf_info->bilateral_scale[j] * (1 << 8));
      }
      frame_msg[k+4] =
        ((uint32_t)(bf_info->noise_threshold[j] * (1 << 4))) << 8 |
         ((uint32_t)(bf_info->weight[j] * (1 << 4)));
    }
  }

  frame_msg[67] = (((uint32_t)asf_info->sp * (1 << 4)) & 0x1F) << 4 |
                   (asf_info->neg_abs_y1 & 0x1) << 2 |
                   (asf_info->dyna_clamp_en & 0x1) << 1 |
                   (asf_info->sp_eff_en & 0x1);

  frame_msg[68] = (asf_info->clamp_h_ul & 0x1FF) << 9 |
                  (asf_info->clamp_h_ll & 0x1FF);

  frame_msg[69] = (asf_info->clamp_v_ul & 0x1FF) << 9 |
                  (asf_info->clamp_v_ll & 0x1FF);

  frame_msg[70] = ((uint32_t)(asf_info->clamp_scale_max * (1 << 4)) & 0x7F) << 16 |
                  ((uint32_t)(asf_info->clamp_scale_min * (1 << 4)) & 0x7F);

  frame_msg[71] = (asf_info->clamp_offset_max & 0x7F) << 16 |
                  (asf_info->clamp_offset_min & 0x7F);

  frame_msg[72] = asf_info->nz_flag;

  for (i = 0; i < 8; i++) {
    frame_msg[73+i] =
      ((int)(asf_info->sobel_h_coeff[i*2+1] * (1 << 10))) << 16 |
      ((int)(asf_info->sobel_h_coeff[i*2] * (1 << 10)) & 0xFFF);
    frame_msg[81+i] =
      ((int)(asf_info->sobel_v_coeff[i*2+1] * (1 << 10))) << 16 |
      ((int)(asf_info->sobel_v_coeff[i*2] * (1 << 10)) & 0xFFF);
    frame_msg[89+i] =
      ((int)(asf_info->hpf_h_coeff[i*2+1] * (1 << 10))) << 16 |
      ((int)(asf_info->hpf_h_coeff[i*2] * (1 << 10)) & 0xFFF);
    frame_msg[97+i] =
      ((int)(asf_info->hpf_v_coeff[i*2+1] * (1 << 10))) << 16 |
      ((int)(asf_info->hpf_v_coeff[i*2] * (1 << 10)) & 0xFFF);
  }

  for (i = 0; i < 8; i++) {
    frame_msg[105+i] =
      ((int)(asf_info->lpf_coeff[i*2+1] * (1 << 10))) << 16 |
      ((int)(asf_info->lpf_coeff[i*2] * (1 << 10)) & 0xFFF);
  }

  for (i = 0; i < in_frame_info->num_planes; i++) {
    j = 113 + i * 5;
    frame_msg[j] = 0x20 |
      in_frame_info->plane_info[i].asf_crop_enable << 4 |
      in_frame_info->plane_info[i].bf_crop_enable << 1 |
      in_frame_info->plane_info[i].bf_enable;
    if (in_frame_info->plane_info[i].scale_info.hscale_en ||
        in_frame_info->plane_info[i].scale_info.vscale_en) {
      frame_msg[j] |= 0x4;
    }
    if (in_frame_info->plane_info[i].input_plane_fmt == PLANE_Y &&
        in_frame_info->asf_mode != 0) {
      frame_msg[j] |= 0x8;
    }

    frame_msg[j+1] =
      in_frame_info->plane_info[i].scale_info.v_scale_fir_algo << 12 |
      in_frame_info->plane_info[i].scale_info.h_scale_fir_algo << 8 |
      in_frame_info->plane_info[i].scale_info.v_scale_algo << 5 |
      in_frame_info->plane_info[i].scale_info.h_scale_algo << 4 |
      in_frame_info->plane_info[i].scale_info.subsample_en << 3 |
      in_frame_info->plane_info[i].scale_info.upsample_en << 2 |
      in_frame_info->plane_info[i].scale_info.vscale_en << 1 |
      in_frame_info->plane_info[i].scale_info.hscale_en;
    frame_msg[j+2] = in_frame_info->plane_info[i].scale_info.phase_h_step;
    frame_msg[j+3] = in_frame_info->plane_info[i].scale_info.phase_v_init;
    frame_msg[j+4] = in_frame_info->plane_info[i].scale_info.phase_v_step;
  }

  i = 128;
  for (j = 0; j < in_frame_info->num_planes; j++) {
    for (k = 0; k < in_frame_info->plane_info[j].num_stripes; k++) {
      stripe_info = &in_frame_info->plane_info[j].stripe_info1[k];
      frame_msg[i + 0] = //STRIPE[0]_PP_m_ROT_CFG_0
        stripe_info->rot_info.rot_cfg << 24 | (stripe_info->rot_info.block_size - 1);
      frame_msg[i + 1] = //STRIPE[0]_PP_m_ROT_CFG_1
        (stripe_info->rot_info.block_height - 1) << 16 |
        (stripe_info->rot_info.block_width - 1);
      frame_msg[i + 2] = //STRIPE[0]_PP_m_ROT_CFG_2
        stripe_info->rot_info.rowIndex1 << 16 |
        stripe_info->rot_info.rowIndex0;
      frame_msg[i + 3] = //STRIPE[0]_PP_m_ROT_CFG_3
        stripe_info->rot_info.colIndex1 << 16 |
        stripe_info->rot_info.colIndex0;
      frame_msg[i + 4] = //STRIPE[0]_PP_m_ROT_CFG_4
        stripe_info->rot_info.modValue << 16 |
        stripe_info->rot_info.initIndex;
      frame_msg[i + 5] = stripe_info->fe_info.buffer_ptr; //STRIPE[0]_FE_n_RD_PNTR
      frame_msg[i + 6] = //STRIPE[0]_FE_n_RD_BUFFER_SIZE
        (stripe_info->fe_info.buffer_height - 1) << 16 |
        (stripe_info->fe_info.buffer_width - 1);
      frame_msg[i + 7] = //STRIPE[0]_FE_n_RD_STRIDE
        stripe_info->fe_info.buffer_stride;
      frame_msg[i + 8] = //STRIPE[0]_FE_n_SWC_RD_BLOCK_DIMENSION
        (stripe_info->fe_info.block_height - 1) << 16 |
        (stripe_info->fe_info.block_width- 1);
      frame_msg[i + 9] = //STRIPE[0]_FE_n_SWC_RD_BLOCK_HPAD
      stripe_info->fe_info.right_pad << 16 |
      stripe_info->fe_info.left_pad;
      frame_msg[i + 10] = //STRIPE[0]_FE_n_SWC_RD_BLOCK_VPAD
      stripe_info->fe_info.bottom_pad << 16 |
      stripe_info->fe_info.top_pad;
      frame_msg[i + 11] = stripe_info->we_info.buffer_ptr[0]; //STRIPE[0]_WE_PLN_0_WR_PNTR
      frame_msg[i + 12] = stripe_info->we_info.buffer_ptr[1]; //STRIPE[0]_WE_PLN_1_WR_PNTR
      frame_msg[i + 13] = stripe_info->we_info.buffer_ptr[2]; //STRIPE[0]_WE_PLN_2_WR_PNTR
      frame_msg[i + 14] = stripe_info->we_info.buffer_ptr[3]; //STRIPE[0]_WE_PLN_3_WR_PNTR
      frame_msg[i + 15] = //STRIPE[0]_WE_PLN_n_WR_BUFFER_SIZE
        stripe_info->we_info.buffer_height << 16 |
        stripe_info->we_info.buffer_width;
      frame_msg[i + 16] = stripe_info->we_info.buffer_stride; //STRIPE[0]_WE_PLN_n_WR_STRIDE
      frame_msg[i + 17] = //STRIPE[0]_WE_PLN_n_WR_CFG_0
        (stripe_info->we_info.blocks_per_row - 1) << 16 |
        (stripe_info->we_info.blocks_per_col - 1);
      frame_msg[i + 18] = stripe_info->we_info.h_step; //STRIPE[0]_WE_PLN_n_WR_CFG_1
      frame_msg[i + 19] = stripe_info->we_info.v_step; //STRIPE[0]_WE_PLN_n_WR_CFG_2
      frame_msg[i + 20] = //STRIPE[0]_WE_PLN_n_WR_CFG_3
      (stripe_info->we_info.h_init) << 16 |
      (stripe_info->we_info.v_init);
      frame_msg[i + 21] = //STRIPE[0]_PP_m_BF_CROP_CFG_0
        stripe_info->bf_crop_info.last_pixel << 16 |
        stripe_info->bf_crop_info.first_pixel;
      frame_msg[i + 22] = //STRIPE[0]_PP_m_BF_CROP_CFG_1
        stripe_info->bf_crop_info.last_line << 16 |
        stripe_info->bf_crop_info.first_line;
      frame_msg[i + 23] = //STRIPE[0]_PP_m_SCALE_OUTPUT_CFG
        (stripe_info->scale_info.block_height-1) << 16 |
        (stripe_info->scale_info.block_width-1);
      frame_msg[i + 24] = //STRIPE[0]_PP_m_SCALE_PHASEH_INIT
        stripe_info->scale_info.phase_h_init;
      frame_msg[i + 25] = //STRIPE[0]_PP_m_ASF_CROP_CFG_0
        stripe_info->asf_crop_info.last_pixel << 16 |
        stripe_info->asf_crop_info.first_pixel;
      frame_msg[i + 26] = //STRIPE[0]_PP_m_ASF_CROP_CFG_1
        stripe_info->asf_crop_info.last_line << 16 |
        stripe_info->asf_crop_info.first_line;
      i+=27;
    }
  }
  frame_msg[msg_len - 1] = 0xABCDEFAA;

  for (i = 0; i < msg_len; i++) {
    CDBG("CPP msg[%d] = 0x%x\n", i, frame_msg[i]);
  }
}

void cpp_prepare_frame_info(struct cpp_frame_info_t *frame_info,
                            struct msm_cpp_frame_info_t *out_info)
{
  struct msm_cpp_frame_strip_info *strip_info;
  struct cpp_stripe_info *stripe_info1;
  struct cpp_plane_info_t *cur_plane_info;
  int i, j, k, strip_num, num_strips;

  out_info->frame_id = frame_info->frame_id;
  out_info->src_fd = frame_info->plane_info[0].src_fd;
  out_info->dst_fd = frame_info->plane_info[0].dst_fd;

  for (i = 0; i < frame_info->num_planes; i++) {
    cur_plane_info = &frame_info->plane_info[i];
    cpp_init_frame_params(cur_plane_info);
    set_start_of_frame_parameters(cur_plane_info);
    cpp_debug_input_info(cur_plane_info);

    CDBG("wmcus: %d hmcus: %d num: %d\n",
         cur_plane_info->frame_width_mcus, cur_plane_info->frame_height_mcus,
         cur_plane_info->num_stripes);
    strip_info = malloc(sizeof(struct msm_cpp_frame_strip_info) *
                        cur_plane_info->num_stripes);
     if (!strip_info){
        CDBG_ERROR("Cannot assign memory to stripe_info");
        return;
    }
    memset(strip_info, 0, sizeof(struct msm_cpp_frame_strip_info) *
           cur_plane_info->num_stripes);
    stripe_info1 = malloc(sizeof(struct cpp_stripe_info) *
                        cur_plane_info->num_stripes);
    if (!stripe_info1){
        CDBG_ERROR("Cannot assign memory to stripe_info1");
        free(strip_info);
        return;
    }
    memset(stripe_info1, 0, sizeof(struct cpp_stripe_info) *
           cur_plane_info->num_stripes);

    cur_plane_info->stripe_info = strip_info;
    cur_plane_info->stripe_info1 = stripe_info1;

    cpp_init_strip_info(cur_plane_info, strip_info, cur_plane_info->num_stripes);

    for (j = 0; j < cur_plane_info->frame_height_mcus; j++) {
      for (k = 0; k < cur_plane_info->frame_width_mcus; k++) {
        strip_num = j * cur_plane_info->frame_width_mcus + k;
        CDBG("Plane: %d, x stripe: %d, y stripe: %d\n", i, k, j);
        run_TF_logic(cur_plane_info, k, j, &strip_info[strip_num]);
        run_TW_logic(cur_plane_info, k, j, &strip_info[strip_num]);
#if 0
        if (i == 1 && k == 0) {
          strip_info[strip_num].src_end_x = 0x1d9;
          strip_info[strip_num].src_end_y = 0x21b;
          strip_info[strip_num].prescale_crop_width_first_pixel = 0;
          strip_info[strip_num].prescale_crop_width_last_pixel = 0x1c3;
          strip_info[strip_num].prescale_crop_height_first_line = 0;
          strip_info[strip_num].prescale_crop_height_last_line = 0x21b;
          strip_info[strip_num].postscale_crop_width_first_pixel = 2;
          strip_info[strip_num].postscale_crop_width_last_pixel = 401;
          strip_info[strip_num].postscale_crop_height_first_line = 0;
          strip_info[strip_num].postscale_crop_height_last_line = 599;
          strip_info[strip_num].dst_start_x = 0;
          strip_info[strip_num].dst_end_x = 0x1bf;
          strip_info[strip_num].dst_start_y = 0;
          strip_info[strip_num].dst_end_y = 0x21b;
        }
#endif
        cpp_debug_strip_info(&strip_info[strip_num], strip_num);
        stripe_info1[strip_num].rotation = cur_plane_info->rotate;
        stripe_info1[strip_num].mirror = cur_plane_info->mirror;
        CDBG("debug: mirror %d\n", stripe_info1[strip_num].mirror);

        cpp_prepare_fetch_engine_info(cur_plane_info, strip_num);
        cpp_prepare_stripe_scale_info(cur_plane_info, strip_num);
        cpp_prepare_crop_info(cur_plane_info, strip_num);
        cpp_prepare_rotation_info(cur_plane_info, strip_num);
        cpp_prepare_write_engine_info(cur_plane_info, strip_num);
      }
    }
    cpp_prepare_plane_scale_info(cur_plane_info);
  }
  cpp_prepare_bf_info(frame_info);
  cpp_prepare_asf_info(frame_info);
  cpp_create_frame_message(out_info, frame_info);

  for (i = 0; i < frame_info->num_planes; i++) {
    cur_plane_info = &frame_info->plane_info[i];
    free(cur_plane_info->stripe_block_width);
    free(cur_plane_info->stripe_block_height);
    free(cur_plane_info->stripe_info);
    free(cur_plane_info->stripe_info1);
  }
}
