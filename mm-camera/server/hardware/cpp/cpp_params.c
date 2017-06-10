/* Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential. */

#include "camera.h"
#include "cam_list.h"
#include "camera_dbg.h"
#include "cpp.h"

/*q factor used in CPP*/
const int q_factor = 21;

/*mask to get the fractional bits*/
const int q_mask = (1 << 21) - 1;

/*Number of extra pixels needed on the left or top for upscaler*/
const int upscale_left_top_padding = 1;

/*Number of extra pixels needed on the right or bottom for upscaler*/
const int upscale_right_bottom_padding = 2;

/*The number of fractional bits used by downscaler*/
const int downscale_interpolation_resolution = 3;

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
run_TW_logic (struct cpp_frame_info_t *in_ary, int src_tile_index_x_counter,
              int src_tile_index_y_counter, struct msm_cpp_frame_strip_info *out_ary)
{
  int i;
  out_ary->dst_start_x = 0;
  out_ary->dst_start_y = 0;
  switch (in_ary->rotate) {
  case 1: /*90 degree*/
    if (in_ary->mirror) {
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
    if (in_ary->mirror) {
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
    if (in_ary->mirror) {
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
    if (in_ary->mirror) {
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
run_TF_logic (struct cpp_frame_info_t *in_ary, int src_tile_index_x_counter,
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
      out_ary->src_start_x =
      (int) ((phase_fg.phase_x_cur -
              in_ary->postscale_padding *
              in_ary->horizontal_scale_ratio) >> q_factor) -
      upscale_left_top_padding - in_ary->prescale_padding;
      out_ary->prescale_crop_width_first_pixel = in_ary->prescale_padding;
      out_ary->postscale_crop_width_first_pixel =
      in_ary->postscale_padding;
    } else {
      /*the leftmost stripe.
        left padding is provided by denoise and sharpening but not upscaling*/
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
      out_ary->src_start_x =
      (int) ((phase_fg.phase_x_cur -
              in_ary->postscale_padding *
              in_ary->horizontal_scale_ratio) >> q_factor) -
      in_ary->prescale_padding;
      out_ary->prescale_crop_width_first_pixel = in_ary->prescale_padding;
      out_ary->postscale_crop_width_first_pixel =
      in_ary->postscale_padding;
    } else {
      /*the leftmost stripe, left padding is done by denoise and sharpening*/
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
    //src_start_x = floor( phase_x_cur )
    if (src_tile_index_x_counter) {
      /*not the leftmost stripe, need to fetch more on the left*/
      out_ary->src_start_x =
      (int) (phase_fg.phase_x_cur >> q_factor) -
      in_ary->postscale_padding - in_ary->prescale_padding;
      out_ary->prescale_crop_width_first_pixel = in_ary->prescale_padding;
      out_ary->postscale_crop_width_first_pixel =
      in_ary->postscale_padding;
    } else {
      /*the leftmost stripe,
        padding is done internally in denoise and sharpening block*/
      out_ary->src_start_x = (int) (phase_fg.phase_x_cur >> q_factor);
      out_ary->prescale_crop_width_first_pixel = 0;
      out_ary->postscale_crop_width_first_pixel = 0;
    }
    // src_end_x = floor( phase_x_next-1 )
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
      out_ary->src_start_y =
      (int) ((phase_fg.phase_y_cur -
              in_ary->postscale_padding *
              in_ary->vertical_scale_ratio) >> q_factor) -
      upscale_left_top_padding - in_ary->prescale_padding;
      out_ary->prescale_crop_height_first_line = in_ary->prescale_padding;
      out_ary->postscale_crop_height_first_line =
      in_ary->postscale_padding;
    } else {
      /*the topmost block, top padding is done
        internally by denoise and sharpening but not upscaling*/
      //src_start_x = floor( phase_x_cur ) - 1
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
      out_ary->src_start_y =
      (int) ((phase_fg.phase_y_cur -
              in_ary->postscale_padding *
              in_ary->vertical_scale_ratio) >> q_factor) -
      in_ary->prescale_padding;
      out_ary->prescale_crop_height_first_line = in_ary->prescale_padding;
      out_ary->postscale_crop_height_first_line =
      in_ary->postscale_padding;
    } else {
      /*the topmost block, top padding internally
        done by denoise and sharpening*/
      // src_start_x = floor( phase_x_cur ) - 1
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
      // src_start_x_luma = floor( phase_x_cur )
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
set_start_of_frame_parameters (struct cpp_frame_info_t *in_ary)
{
  int i;
  /*determine destination stripe width*/
  int block_width = in_ary->line_buffer_size - 2 * in_ary->postscale_padding;
  int rotation_block_height;
  if (in_ary->horizontal_scale_ratio < (1 << q_factor)
      || (in_ary->horizontal_scale_ratio == (1 << q_factor)
          && (in_ary->horizontal_scale_initial_phase & q_mask))) {
    /* upscaler is used
       crop out asf pixels, upscale pixels, and denoise pixels */
    long long temp =
    in_ary->line_buffer_size - 2 * in_ary->prescale_padding -
    upscale_left_top_padding - upscale_right_bottom_padding;
    temp <<= q_factor;
    temp -= q_mask + 1; /* safety margin */
    /*number of pixels that can be produced by upscaler*/
    temp /= in_ary->horizontal_scale_ratio;
    temp -= 2 * in_ary->postscale_padding;
    if (temp < block_width) {
      block_width = (int) temp;
    }
  } else { /*downscaler or no scaler*/
    /*crop out asf pixels, and denoise pixels*/
    long long temp = in_ary->line_buffer_size - 2 * in_ary->prescale_padding;
    temp <<= q_factor;
    temp -= q_mask; /* safety margin */
    /*number of pixels that can be produced by downscaler*/
    temp /= in_ary->horizontal_scale_ratio;
    temp -= 2 * in_ary->postscale_padding;
    if (temp < block_width) {
      block_width = (int) temp;
    }
  }
  in_ary->dst_block_width =
  (block_width / in_ary->mal_byte_size) * in_ary->mal_byte_size;
  if (in_ary->dst_block_width == 0) {
    /*maximum possible stripe width is smaller than the MAL length*/
    in_ary->dst_block_width = block_width;
  }
  if (in_ary->dst_block_width > in_ary->dst_width) {
    /*destination image is smaller than line buffer size*/
    in_ary->dst_block_width = in_ary->dst_width;
  }

  /* number of stripes */
  in_ary->frame_width_mcus =
    (in_ary->dst_width + in_ary->dst_block_width - 1)
    / in_ary->dst_block_width;

  /*destination stripe width*/
  in_ary->stripe_block_width = malloc(sizeof(int) *
                               in_ary->frame_width_mcus);

  for (i = 0; i < in_ary->frame_width_mcus; i++) {
    /*First assume all the stripe widths are the same,
      one entry will be changed later*/
    in_ary->stripe_block_width[i] = in_ary->dst_block_width;
  }
  if (in_ary->rotate == 0 || in_ary->rotate == 2) { /*0 or 180 degree rotation*/
    /*rotation block height is 2,
      the destination image height will be a multiple of 2.*/
    rotation_block_height = 2;
  } else { /*90 or 270 degree rotation*/
    /*rotation block height is MAL length (32?).
      The destination image height will be a multiple of MAL*/
    rotation_block_height = in_ary->mal_byte_size;
  }
  /*The actual destination image height, multiple of rotation block height*/
  in_ary->dst_height_block_aligned =
    ((in_ary->dst_height + rotation_block_height - 1) /
     rotation_block_height) * rotation_block_height;

  if (in_ary->dst_height_block_aligned > in_ary->maximum_dst_stripe_height) {
    /*Maximum allowed block height is smaller than destination image height*/
    /*do block processing*/
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

  /*destination block height*/
  in_ary->stripe_block_height = malloc(sizeof(int) *
                                in_ary->frame_height_mcus);

  for (i = 0; i < in_ary->frame_height_mcus; i++) {
    /*First assume all the block heights are the same,
      one entry will be changed later*/
    in_ary->stripe_block_height[i] = in_ary->dst_block_height;
  }

  switch (in_ary->rotate) {
  case 1: /* 90 degree */
    if (in_ary->mirror) {
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
    in_ary->vertical_scale_block_initial_phase =
    in_ary->vertical_scale_initial_phase;
    break;
  case 2:
    if (in_ary->mirror) {
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
    if (in_ary->mirror) {
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
    if (in_ary->mirror) {
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
}

void cpp_init_frame_params (struct cpp_frame_info_t *frame)
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

void cpp_debug_input_info(struct cpp_frame_info_t *frame)
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
  CDBG("CPP: line_buffer_size %d\n", frame->line_buffer_size);
  CDBG("CPP: maximum_dst_stripe_height %d\n", frame->maximum_dst_stripe_height);
  CDBG("CPP: mal_byte_size %d\n", frame->mal_byte_size);
  CDBG("CPP: bytes_per_pixel %d\n", frame->bytes_per_pixel);
  CDBG("CPP: source_address 0x%x\n", frame->source_address);
  CDBG("CPP: destination_address 0x%x\n", frame->destination_address);
  CDBG("CPP: frame_width_mcus %d\n", frame->frame_width_mcus);
  CDBG("CPP: frame_height_mcus %d\n", frame->frame_height_mcus);
  CDBG("CPP: dst_height_block_aligned %d\n", frame->dst_height_block_aligned);
  CDBG("CPP: dst_block_width %d\n", frame->dst_block_width);
  CDBG("CPP: dst_block_height %d\n", frame->dst_block_height);
  CDBG("CPP: horizontal_scale_block_initial_phase %lld\n",
       frame->horizontal_scale_block_initial_phase);
  CDBG("CPP: vertical_scale_block_initial_phase %lld\n",
       frame->vertical_scale_block_initial_phase);
}

void cpp_init_strip_info(struct cpp_frame_info_t *in_info,
                         struct msm_cpp_frame_info_t *out_info)
{
  struct msm_cpp_frame_strip_info *strip_info;
  uint32_t i;
  for (i = 0; i < out_info->num_strips; i++) {
    strip_info = &out_info->strip_info[i];
    strip_info->bytes_per_pixel = in_info->bytes_per_pixel;
    strip_info->src_stride = in_info->src_stride;
    strip_info->dst_stride = in_info->dst_stride;

    switch (in_info->rotate) {
    case 1:
      if (in_info->mirror) {
        strip_info->horizontal_flip = 0;
      } else {
        strip_info->horizontal_flip = 1;
      }
      strip_info->rotate_270 = 1;
      strip_info->vertical_flip = 1;
      break;
    case 2:
      if (in_info->mirror) {
        strip_info->horizontal_flip = 0;
      } else {
        strip_info->horizontal_flip = 1;
      }
      strip_info->rotate_270 = 0;
      strip_info->vertical_flip = 1;
      break;
    case 3:
      if (in_info->mirror) {
        strip_info->horizontal_flip = 1;
      } else {
        strip_info->horizontal_flip = 0;
      }
      strip_info->rotate_270 = 1;
      strip_info->vertical_flip = 0;
      break;
    case 0:
    default:
      if (in_info->mirror) {
        strip_info->horizontal_flip = 1;
      } else {
        strip_info->horizontal_flip = 0;
      }
      strip_info->rotate_270 = 0;
      strip_info->vertical_flip = 0;
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

void cpp_prepare_frame_info(struct cpp_frame_info_t *in_info,
                            struct msm_cpp_frame_info_t *out_info)
{
  struct msm_cpp_frame_strip_info *strip_info;
  int i, j, strip_num;
  cpp_init_frame_params(in_info);
  set_start_of_frame_parameters(in_info);
  cpp_debug_input_info(in_info);

  out_info->num_strips = (uint32_t) in_info->frame_width_mcus * in_info->frame_height_mcus;
  out_info->strip_info = malloc(sizeof(struct msm_cpp_frame_strip_info) * out_info->num_strips);
  memset(out_info->strip_info, 0, sizeof(struct msm_cpp_frame_strip_info) * out_info->num_strips);


  cpp_init_strip_info(in_info, out_info);

  for (i = 0; i < in_info->frame_height_mcus; i++) {
    for (j = 0; j < in_info->frame_width_mcus; j++) {
      strip_num = i * in_info->frame_width_mcus + j;
      strip_info = &out_info->strip_info[strip_num];
      run_TF_logic(in_info, j, i, strip_info);
      run_TW_logic(in_info, j, i, strip_info);
      cpp_debug_strip_info(strip_info, strip_num);
    }
  }
  free(in_info->stripe_block_width);
  free(in_info->stripe_block_height);
}
