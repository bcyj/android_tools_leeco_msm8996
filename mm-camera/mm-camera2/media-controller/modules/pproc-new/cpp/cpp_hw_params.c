
/*============================================================================

  Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "eztune_diagnostics.h"
#include "cpp_hw_params.h"
#include "cpp_log.h"
#include <math.h>

#ifndef Round
#define Round(x) (int)(x + sign(x)*0.5)
#endif

#ifndef MIN
#define MIN(x,y) (((x)<(y)) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y) (((x)>(y)) ? (x) : (y))
#endif

#ifndef CLAMP
#define CLAMP(x, min, max) MAX (MIN (x, max), min)
#endif

#ifndef sign
#define sign(x) ((x < 0) ?(-1) : (1))
#endif

#ifndef Round64
#define Round64(x) (unsigned long)((x) + ((x) > 0 ? 1 : -1) * 0.5)
#endif

#ifndef PAD_TO_SIZE
#define PAD_TO_SIZE(size, padding)  ((size + padding - 1) & ~(padding - 1))
#endif

/* TODO: really need code clean-up in this file */

/*q factor used in CPP*/
static const int q_factor = 21;

/*mask to get the fractional bits*/
static const int q_mask = (1 << 21) - 1;

/*Number of extra pixels needed on the left or stop for upscaler*/
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

static void increment_phase (struct cpp_accumulated_phase_t *in_phase,
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

void run_TW_logic (struct cpp_plane_info_t *in_ary,
  int src_tile_index_x_counter, int src_tile_index_y_counter,
  struct msm_cpp_frame_strip_info *out_ary)
{
  int i;
  out_ary->dst_start_x = 0;
  out_ary->dst_start_y = 0;
  switch (in_ary->rotate) {
  case 1: /*90 degree*/
    if (in_ary->mirror & 0x1) {
      for (i = 0; i < src_tile_index_y_counter; i++) {
        out_ary->dst_start_x += in_ary->stripe_block_height[i];
      }
      out_ary->dst_end_x = out_ary->dst_start_x +
        in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
    } else {
      for (i = in_ary->frame_height_mcus - 1;
        i > src_tile_index_y_counter; i--) {
        out_ary->dst_start_x += in_ary->stripe_block_height[i];
      }
      out_ary->dst_end_x = out_ary->dst_start_x +
      in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
    }
    if (in_ary->mirror & 0x2) {
      for (i = in_ary->frame_width_mcus - 1;
           i > src_tile_index_x_counter; i--) {
        out_ary->dst_start_y += in_ary->stripe_block_width[i];
      }
    } else {
      for (i = 0; i < src_tile_index_x_counter; i++) {
        out_ary->dst_start_y += in_ary->stripe_block_width[i];
      }
    }
    out_ary->dst_end_y = out_ary->dst_start_y +
      in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
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
    if (in_ary->mirror & 0x1) {
      for (i = in_ary->frame_height_mcus - 1;
        i > src_tile_index_y_counter; i--) {
        out_ary->dst_start_x += in_ary->stripe_block_height[i];
      }
      out_ary->dst_end_x = out_ary->dst_start_x +
        in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
    } else {
      for (i = 0; i < src_tile_index_y_counter; i++) {
        out_ary->dst_start_x += in_ary->stripe_block_height[i];
      }
      out_ary->dst_end_x = out_ary->dst_start_x +
        in_ary->stripe_block_height[src_tile_index_y_counter] - 1;
    }
    if (in_ary->mirror & 0x2) {
      for (i = 0; i < src_tile_index_x_counter; i++) {
        out_ary->dst_start_y += in_ary->stripe_block_width[i];
      }
    } else {
      for (i = in_ary->frame_width_mcus - 1;
        i > src_tile_index_x_counter; i--) {
        out_ary->dst_start_y += in_ary->stripe_block_width[i];
      }
    }
    out_ary->dst_end_y = out_ary->dst_start_y +
      in_ary->stripe_block_width[src_tile_index_x_counter] - 1;
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
  out_ary->destination_address[0] =
    in_ary->destination_address[0] +
    out_ary->dst_start_x * out_ary->output_bytes_per_pixel +
    out_ary->dst_start_y * in_ary->dst_stride;
  out_ary->destination_address[1] =
    in_ary->destination_address[1] +
    out_ary->dst_start_x * out_ary->output_bytes_per_pixel +
    out_ary->dst_start_y * in_ary->dst_stride;
}

/*run tile fetch logic*/
void run_TF_logic (struct cpp_plane_info_t *in_ary,
  int src_tile_index_x_counter, int src_tile_index_y_counter,
  struct msm_cpp_frame_strip_info *out_ary)
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

      out_ary->spatial_denoise_crop_width_first_pixel = in_ary->prescale_padding;
      out_ary->sharpen_crop_width_first_pixel =
      in_ary->postscale_padding;
    } else {
      /*the leftmost stripe.
        left padding is provided by denoise and sharpening but not upscaling*/

      /*only the fractional bits are needed*/
      out_ary->h_init_phase = (int) (phase_fg.phase_x_cur & q_mask);
      out_ary->src_start_x =
      (int) ((phase_fg.phase_x_cur) >> q_factor) -
      upscale_left_top_padding;
      out_ary->spatial_denoise_crop_width_first_pixel = 0;
      out_ary->sharpen_crop_width_first_pixel = 0;
    }
    if (src_tile_index_x_counter == in_ary->frame_width_mcus - 1) {
      /*the rightmost stripe,
        right padding is provided by denoise and sharpening but not upscaling*/
      out_ary->src_end_x =
      (int) ((phase_fg.phase_x_next -
              in_ary->horizontal_scale_ratio) >> q_factor) +
      upscale_right_bottom_padding;
      out_ary->spatial_denoise_crop_width_last_pixel =
      out_ary->src_end_x - out_ary->src_start_x;
    } else { /*not the rightmost stripe. need to fetch more on the right*/
      out_ary->src_end_x =
      (int) ((phase_fg.phase_x_next +
              (in_ary->postscale_padding -
               1) * in_ary->horizontal_scale_ratio) >> q_factor) +
      upscale_right_bottom_padding + in_ary->prescale_padding;
      out_ary->spatial_denoise_crop_width_last_pixel =
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

      out_ary->spatial_denoise_crop_width_first_pixel = in_ary->prescale_padding;
      out_ary->sharpen_crop_width_first_pixel =
      in_ary->postscale_padding;
    } else {
      /*the leftmost stripe, left padding is done by denoise and sharpening*/

      /*only the fractional bits are needed*/
      out_ary->h_init_phase = (int) (phase_fg.phase_x_cur & q_mask);
      out_ary->src_start_x = (int) (phase_fg.phase_x_cur >> q_factor);
      out_ary->spatial_denoise_crop_width_first_pixel = 0;
      out_ary->sharpen_crop_width_first_pixel = 0;
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
      out_ary->spatial_denoise_crop_width_last_pixel =
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
      out_ary->spatial_denoise_crop_width_last_pixel =
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
      out_ary->spatial_denoise_crop_width_first_pixel = in_ary->prescale_padding;
      out_ary->sharpen_crop_width_first_pixel =
      in_ary->postscale_padding;
    } else {
      CDBG_LOW("Left most stripe\n");
      /*the leftmost stripe,
        padding is done internally in denoise and sharpening block*/
      out_ary->src_start_x = (int) (phase_fg.phase_x_cur >> q_factor);
      out_ary->spatial_denoise_crop_width_first_pixel = 0;
      out_ary->sharpen_crop_width_first_pixel = 0;
    }
    if (src_tile_index_x_counter == in_ary->frame_width_mcus - 1) {
      /*the rightmost stripe, right padding is done internally in the HW*/
      out_ary->src_end_x = (int) (phase_fg.phase_x_next >> q_factor) - 1;
      out_ary->spatial_denoise_crop_width_last_pixel =
      out_ary->src_end_x - out_ary->src_start_x;
    } else {
      /*not the rightmost stripe, need to fetch more on the right*/
      out_ary->src_end_x =
      (int) (phase_fg.phase_x_next >> q_factor) - 1 +
      in_ary->postscale_padding + in_ary->prescale_padding;
      out_ary->spatial_denoise_crop_width_last_pixel =
      out_ary->src_end_x - out_ary->src_start_x -
      in_ary->prescale_padding;
    }
  }
  out_ary->sharpen_crop_width_last_pixel =
  out_ary->sharpen_crop_width_first_pixel +
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

      out_ary->spatial_denoise_crop_height_first_line = in_ary->prescale_padding;
      out_ary->sharpen_crop_height_first_line =
      in_ary->postscale_padding;
    } else {
      /*the topmost block, top padding is done
        internally by denoise and sharpening but not upscaling*/
      out_ary->v_init_phase = (int) (phase_fg.phase_y_cur & q_mask);
      out_ary->src_start_y =
      (int) ((phase_fg.phase_y_cur) >> q_factor) -
      upscale_left_top_padding;
      out_ary->spatial_denoise_crop_height_first_line = 0;
      out_ary->sharpen_crop_height_first_line = 0;
    }
    if (src_tile_index_y_counter == in_ary->frame_height_mcus - 1) {
      /*the bottommost stripe, bottom padding is done
        internally by denoise and sharpening but not upscaler*/
      // src_end_x = floor( phase_x_next - PHASE_X_STEP ) + 2
      out_ary->src_end_y =
      (int) ((phase_fg.phase_y_next -
              in_ary->vertical_scale_ratio) >> q_factor) +
      upscale_right_bottom_padding;
      out_ary->spatial_denoise_crop_height_last_line =
      out_ary->src_end_y - out_ary->src_start_y;
    } else {
      /*not the bottommost stripe, need to fetch more on the bottom*/
      out_ary->src_end_y =
      (int) ((phase_fg.phase_y_next +
              (in_ary->postscale_padding -
               1) * in_ary->vertical_scale_ratio) >> q_factor) +
      upscale_right_bottom_padding + in_ary->prescale_padding;
      out_ary->spatial_denoise_crop_height_last_line =
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

      out_ary->spatial_denoise_crop_height_first_line = in_ary->prescale_padding;
      out_ary->sharpen_crop_height_first_line =
      in_ary->postscale_padding;
    } else {
      /*the topmost block, top padding internally
        done by denoise and sharpening*/
      out_ary->v_init_phase = (int) (phase_fg.phase_y_cur & q_mask);
      out_ary->src_start_y = (int) ((phase_fg.phase_y_cur) >> q_factor);
      out_ary->spatial_denoise_crop_height_first_line = 0;
      out_ary->sharpen_crop_height_first_line = 0;
    }

    if (src_tile_index_y_counter == in_ary->frame_height_mcus - 1) {
      if ((phase_fg.phase_y_next & q_mask) >> (q_factor -
                                               downscale_interpolation_resolution)) {
        out_ary->src_end_y = (int) (phase_fg.phase_y_next >> q_factor);
      } else {
        /*need to fetch one pixel less*/
        out_ary->src_end_y = (int) (phase_fg.phase_y_next >> q_factor) - 1;
      }
      out_ary->spatial_denoise_crop_height_last_line =
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
      out_ary->spatial_denoise_crop_height_last_line =
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
      out_ary->spatial_denoise_crop_height_first_line = in_ary->prescale_padding;
      out_ary->sharpen_crop_height_first_line =
      in_ary->postscale_padding;
    } else {
      /*the topmost block, top padding is done
        in the denoise and sharpening block*/
      out_ary->src_start_y = (int) (phase_fg.phase_y_cur >> q_factor);
      out_ary->spatial_denoise_crop_height_first_line = 0;
      out_ary->sharpen_crop_height_first_line = 0;
    }
    if (src_tile_index_y_counter == in_ary->frame_height_mcus - 1) {
      /*the bottommost block, bottom padding
        by the denoise and sharpening block*/
      out_ary->src_end_y = (int) (phase_fg.phase_y_next >> q_factor) - 1;
      out_ary->spatial_denoise_crop_height_last_line =
      out_ary->src_end_y - out_ary->src_start_y;
    } else {
      /*not the bottommost block, need to fetch more on the bottom*/
      out_ary->src_end_y =
      (int) ((phase_fg.phase_y_next) >> q_factor) - 1 +
      in_ary->postscale_padding + in_ary->prescale_padding;
      out_ary->spatial_denoise_crop_height_last_line =
      out_ary->src_end_y - out_ary->src_start_y -
      in_ary->prescale_padding;
    }
  }
  out_ary->sharpen_crop_height_last_line =
  out_ary->sharpen_crop_height_first_line +
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
  out_ary->source_address[0] =
  in_ary->source_address[0] + out_ary->src_start_x *
  in_ary->input_bytes_per_pixel + out_ary->src_start_y * in_ary->src_stride;
}

void set_start_of_frame_parameters (struct cpp_plane_info_t *in_ary)
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
  CDBG_LOW("MCUS: %d\n", in_ary->frame_width_mcus);

  if ((in_ary->input_plane_fmt == PLANE_Y) && (in_ary->frame_width_mcus % 2)) {
    // try to make number of stripes even for Y plane to run faster
    in_ary->frame_width_mcus++;
  }

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

  CDBG_LOW(" >>>> Aligned block height: %d\n", in_ary->dst_height_block_aligned);

  if (in_ary->dst_height_block_aligned > in_ary->maximum_dst_stripe_height) {
    /*Maximum allowed block height is smaller than destination image height*/
    /*do block processing*/
    CDBG_LOW("Block processing?\n");
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
  CDBG_LOW("HMCUS: %d\n", in_ary->frame_height_mcus);

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
      // the bottommost block heights are smaller than others.
      // fetch a little more on the bottom so padding will be on the bottom
      in_ary->stripe_block_height[in_ary->frame_height_mcus - 1] =
        in_ary->dst_height_block_aligned - in_ary->dst_block_height *
        (in_ary->frame_height_mcus - 1);
    } else {
      // the topmost block heights are smaller than others
      // fetch a little more on the top so padding will be on the right
      in_ary->vertical_scale_initial_phase -=
        (in_ary->dst_height_block_aligned - in_ary->dst_height) *
        in_ary->vertical_scale_ratio;
      in_ary->vertical_scale_block_initial_phase =
        in_ary->vertical_scale_initial_phase - (in_ary->frame_height_mcus *
        in_ary->dst_block_height - in_ary->dst_height_block_aligned) *
        in_ary->vertical_scale_ratio;
      in_ary->stripe_block_height[0] =
        in_ary->dst_height_block_aligned - in_ary->dst_block_height *
        (in_ary->frame_height_mcus - 1);
    }
    // the rightmost destination stripe width is smaller than others
    in_ary->stripe_block_width[in_ary->frame_width_mcus - 1] =
      in_ary->dst_width - in_ary->dst_block_width *
      (in_ary->frame_width_mcus - 1);
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
      // the rightmost destination stripe width is smaller than others
      in_ary->vertical_scale_initial_phase -=
        (in_ary->dst_height_block_aligned - in_ary->dst_height) *
        in_ary->vertical_scale_ratio;
      in_ary->vertical_scale_block_initial_phase =
        in_ary->vertical_scale_initial_phase -
        (in_ary->frame_height_mcus * in_ary->dst_block_height -
        in_ary->dst_height_block_aligned) * in_ary->vertical_scale_ratio;
      in_ary->stripe_block_height[0] =
        in_ary->dst_height_block_aligned - in_ary->dst_block_height *
        (in_ary->frame_height_mcus - 1);
    } else {
      //the bottommost block heights are smaller than others.
      //fetch a little more on the bottom so padding will be on the right side
      in_ary->stripe_block_height[in_ary->frame_height_mcus - 1] =
        in_ary->dst_height_block_aligned - in_ary->dst_block_height *
        (in_ary->frame_height_mcus - 1);
    }

    // the leftmost destination stripe width is smaller than others
    in_ary->horizontal_scale_block_initial_phase -=
      (in_ary->frame_width_mcus * in_ary->dst_block_width -
      in_ary->dst_width) * in_ary->horizontal_scale_ratio;
    in_ary->stripe_block_width[0] = in_ary->dst_width -
        in_ary->dst_block_width * (in_ary->frame_width_mcus - 1);
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
  CDBG_LOW("CPP: src_width %d\n", frame->src_width);
  CDBG_LOW("CPP: src_height %d\n", frame->src_height);
  CDBG_LOW("CPP: src_stride %d\n", frame->src_stride);
  CDBG_LOW("CPP: dst_width %d\n", frame->dst_width);
  CDBG_LOW("CPP: dst_height %d\n", frame->dst_height);
  CDBG_LOW("CPP: dst_stride %d\n", frame->dst_stride);
  CDBG_LOW("CPP: rotate %d\n", frame->rotate);
  CDBG_LOW("CPP: mirror %d\n", frame->mirror);
  CDBG_LOW("CPP: prescale_padding %d\n", frame->prescale_padding);
  CDBG_LOW("CPP: postscale_padding %d\n", frame->postscale_padding);
  CDBG_LOW("CPP: horizontal_scale_ratio %d\n", frame->horizontal_scale_ratio);
  CDBG_LOW("CPP: vertical_scale_ratio %d\n", frame->vertical_scale_ratio);
  CDBG_LOW("CPP: horizontal_scale_initial_phase %lld\n",
       frame->horizontal_scale_initial_phase);
  CDBG_LOW("CPP: vertical_scale_initial_phase %lld\n",
       frame->vertical_scale_initial_phase);
  CDBG_LOW("CPP: maximum_dst_stripe_height %d\n", frame->maximum_dst_stripe_height);
  CDBG_LOW("CPP: input_plane_fmt %d\n", frame->input_plane_fmt);
  CDBG_LOW("CPP: source_address 0x%x\n", frame->source_address);
  CDBG_LOW("CPP: destination_address 0x%x\n", frame->destination_address);
  CDBG_LOW("CPP: frame_width_mcus %d\n", frame->frame_width_mcus);
  CDBG_LOW("CPP: frame_height_mcus %d\n", frame->frame_height_mcus);
  CDBG_LOW("CPP: dst_height_block_aligned %d\n", frame->dst_height_block_aligned);
  CDBG_LOW("CPP: dst_block_width %d\n", frame->dst_block_width);
  CDBG_LOW("CPP: dst_block_height %d\n", frame->dst_block_height);
  CDBG_LOW("CPP: stripe_block_width %d\n", frame->stripe_block_width[0]);
  CDBG_LOW("CPP: stripe_block_height %d\n", frame->stripe_block_height[0]);

  CDBG_LOW("CPP: horizontal_scale_block_initial_phase %lld\n",
       frame->horizontal_scale_block_initial_phase);
  CDBG_LOW("CPP: vertical_scale_block_initial_phase %lld\n",
       frame->vertical_scale_block_initial_phase);
}

void enable_crop_engines(struct cpp_plane_info_t *in_info,
  struct msm_cpp_frame_strip_info *cur_strip_info)
{
  if(in_info->prescale_padding == 0 ||
      (in_info->frame_width_mcus == 1 && in_info->frame_height_mcus == 1)) {
     cur_strip_info->spatial_denoise_crop_en = 0;
     in_info->bf_crop_enable = 0;
   } else {
     cur_strip_info->spatial_denoise_crop_en = 1;
     in_info->bf_crop_enable = 1;
   }

   if(in_info->postscale_padding == 0 ||
      (in_info->frame_width_mcus == 1 && in_info->frame_height_mcus == 1)) {
     cur_strip_info->sharpen_crop_en = 0;
     in_info->asf_crop_enable = 0;
   } else {
     cur_strip_info->sharpen_crop_en = 1;
     in_info->asf_crop_enable = 1;
   }
}

void cpp_init_strip_info(struct cpp_plane_info_t *in_info,
                         struct msm_cpp_frame_strip_info *stripe_info,
                         int num_stripes)
{
  struct msm_cpp_frame_strip_info *cur_strip_info;
  int i;
  for (i = 0; i < num_stripes; i++) {
    cur_strip_info = &stripe_info[i];

    enable_crop_engines(in_info,cur_strip_info);
    cur_strip_info->output_bytes_per_pixel =
      in_info->output_bytes_per_pixel;
    cur_strip_info->input_bytes_per_pixel =
      in_info->input_bytes_per_pixel;
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
  CDBG_LOW("CPP FRAME STRIP %d", n);

  /*Is vertical scale enabled?*/
  CDBG_LOW("CPP scale_v_en 0x%x\n", strip_info->scale_v_en);
  /*Is horizontal scale enabled?*/
  CDBG_LOW("CPP scale_h_en 0x%x\n", strip_info->scale_h_en);
  /*Leftmost position of the fetch block in terms of pixels*/
  CDBG_LOW("CPP src_start_x 0x%x\n", strip_info->src_start_x);
  /*Rightmost position of the fetch block in terms of pixels*/
  CDBG_LOW("CPP src_end_x 0x%x\n", strip_info->src_end_x);
  /*Topmost position of the fetch block in terms of pixels*/
  CDBG_LOW("CPP src_start_y 0x%x\n", strip_info->src_start_y);
  /*Bottommost positin of the fetch block in terms of pixels*/
  CDBG_LOW("CPP src_end_y 0x%x\n", strip_info->src_end_y);
  /*The amount padded at the fetch block left boundary in terms of pixels*/
  CDBG_LOW("CPP pad_left 0x%x\n", strip_info->pad_left);
  /*The amount padded at the fetch block right boundary in terms of pixels*/
  CDBG_LOW("CPP pad_right 0x%x\n", strip_info->pad_right);
  /*The amount padded at the fetch block top boundary in terms of pixels*/
  CDBG_LOW("CPP pad_top 0x%x\n", strip_info->pad_top);
  /*The amount padded at the fetch block bottom boundary in terms of pixels*/
  CDBG_LOW("CPP pad_bottom 0x%x\n", strip_info->pad_bottom);
  /*Leftmost sampling position, range (0, 1)*/
  CDBG_LOW("CPP h_init_phase 0x%x\n", strip_info->h_init_phase);
  /*Topmost sampling position, range (0, 1)*/
  CDBG_LOW("CPP v_init_phase 0x%x\n", strip_info->v_init_phase);
  /*Horizontal sampling distance, Q21 number*/
  CDBG_LOW("CPP h_phase_step 0x%x\n", strip_info->h_phase_step);
  /*Vertical sampling distance, Q21 number*/
  CDBG_LOW("CPP v_phase_step 0x%x\n", strip_info->v_phase_step);
  /*Leftmost position of the cropping window right after denoise block*/
  CDBG_LOW("CPP spatial_denoise_crop_width_first_pixel 0x%x\n", strip_info->spatial_denoise_crop_width_first_pixel);
  /*Rightmost position of the cropping window right after denoise block*/
  CDBG_LOW("CPP spatial_denoise_crop_width_last_pixel 0x%x\n", strip_info->spatial_denoise_crop_width_last_pixel);
  /*Topmost position of the cropping window right after denoise block*/
  CDBG_LOW("CPP spatial_denoise_crop_height_first_line 0x%x\n", strip_info->spatial_denoise_crop_height_first_line);
  /*Bottommost position of the cropping window right after denoise block*/
  CDBG_LOW("CPP spatial_denoise_crop_height_last_line 0x%x\n", strip_info->spatial_denoise_crop_height_last_line);
  /*Leftmost position of the cropping window right after sharpening block*/
  CDBG_LOW("CPP sharpen_crop_width_first_pixel 0x%x\n", strip_info->sharpen_crop_width_first_pixel);
  /*Rightmost position of the cropping window right after sharpening block*/
  CDBG_LOW("CPP sharpen_crop_width_last_pixel 0x%x\n", strip_info->sharpen_crop_width_last_pixel);
  /*Topmost position of the cropping window right after denoise block*/
  CDBG_LOW("CPP sharpen_crop_height_first_line 0x%x\n", strip_info->sharpen_crop_height_first_line);
  /*Bottommost position of the cropping window right after denoise block*/
  CDBG_LOW("CPP sharpen_crop_height_last_line 0x%x\n", strip_info->sharpen_crop_height_last_line);
  /*Leftmost position of the write block in terms of pixels*/
  CDBG_LOW("CPP dst_start_x 0x%x\n", strip_info->dst_start_x);
  /*Rightmost position of the write block in terms of pixels*/
  CDBG_LOW("CPP dst_end_x 0x%x\n", strip_info->dst_end_x);
  /*Topmost position of the write block in terms of pixels*/
  CDBG_LOW("CPP dst_start_y 0x%x\n", strip_info->dst_start_y);
  /*Bottommost position of the write block in terms of pixels*/
  CDBG_LOW("CPP dst_end_y 0x%x\n", strip_info->dst_end_y);
  /*1-> Planar, 2->CbCr*/
  CDBG_LOW("CPP bytes_per_pixel 0x%x\n", strip_info->input_bytes_per_pixel);
  /*Topleft corner of the fetch block in terms of memory address*/
  CDBG_LOW("CPP source_address 0x%x\n", strip_info->source_address);
  /*Topleft corner of the write block in terms of memory address*/
  CDBG_LOW("CPP destination_address 0x%x\n", strip_info->destination_address);
  /*Source image stride in terms of bytes*/
  CDBG_LOW("CPP src_stride 0x%x\n", strip_info->src_stride);
  /*Destination image stride in terms of bytes*/
  CDBG_LOW("CPP dst_stride 0x%x\n", strip_info->dst_stride);
  /*Do we rotate 270 degree or not?*/
  CDBG_LOW("CPP rotate_270 0x%x\n", strip_info->rotate_270);
  /*Do we follow rotation with horizontal flip?*/
  CDBG_LOW("CPP horizontal_flip 0x%x\n", strip_info->horizontal_flip);
  /*Do we follow rotation with vertical flip?*/
  CDBG_LOW("CPP vertical_flip 0x%x\n", strip_info->vertical_flip);
  /*Scaler output width*/
  CDBG_LOW("CPP scale_output_width 0x%x\n", strip_info->scale_output_width);
  /*Scaler output height*/
  CDBG_LOW("CPP scale_output_height 0x%x\n", strip_info->scale_output_height);
  /*Is vertical upscale enabled?*/
  CDBG_LOW("CPP upscale_v_en 0x%x\n", strip_info->upscale_v_en);
  /*Is horizontal upscale enabled?*/
  CDBG_LOW("CPP upscale_h_en 0x%x\n", strip_info->upscale_h_en);
}

void cpp_debug_fetch_engine_info(struct cpp_fe_info *fe_info)
{
  CDBG_LOW("buffer_ptr   : %d\n", fe_info->buffer_ptr);
  CDBG_LOW("buffer_width : %d\n", fe_info->buffer_width);
  CDBG_LOW("buffer_height: %d\n", fe_info->buffer_height);
  CDBG_LOW("buffer_stride: %d\n", fe_info->buffer_stride);
  CDBG_LOW("block_width  : %d\n", fe_info->block_width);
  CDBG_LOW("block_height : %d\n", fe_info->block_height);
  CDBG_LOW("left_pad     : %d\n", fe_info->left_pad);
  CDBG_LOW("right_pad    : %d\n", fe_info->right_pad);
  CDBG_LOW("top_pad      : %d\n", fe_info->top_pad);
  CDBG_LOW("bottom_pad   : %d\n", fe_info->bottom_pad);
}

void cpp_prepare_fetch_engine_info(struct cpp_plane_info_t *plane_info,
  int stripe_num)
{
  struct cpp_stripe_info *stripe_info1 =
    &plane_info->stripe_info1[stripe_num];
  struct msm_cpp_frame_strip_info *stripe_info =
    &plane_info->stripe_info[stripe_num];
  struct cpp_fe_info *fe_info = &stripe_info1->fe_info;

  fe_info->buffer_ptr = stripe_info->source_address[0];
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
      fe_info->top_pad = stripe_info->pad_bottom;
      fe_info->bottom_pad = stripe_info->pad_top;
    } else {
      fe_info->top_pad = stripe_info->pad_top;
      fe_info->bottom_pad = stripe_info->pad_bottom;
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
    //Q10 and Q8 format for bilateral_scale is calculated in cpp_prepare_bf_info()
    CDBG_LOW("bilateral_scale(bf cfg0)[%d]: %f\n",
      i, bf_info->bilateral_scale[i]);
    CDBG_LOW("noise_threshold[%d]: %f\n", i, bf_info->noise_threshold[i]);
    CDBG_LOW("weight[%d]: %f\n", i, bf_info->weight[i]);
    CDBG_LOW("bf cfg1: 0x%x\n", (uint32_t)
         ((uint32_t)(bf_info->noise_threshold[i] * (1 << 4))) << 8 |
         ((uint32_t)(bf_info->weight[i] * (1 << 4))));
  }
}

void cpp_prepare_bf_info(struct cpp_frame_info_t *frame_info)
{
  int i, j;
  struct cpp_bf_info *bf_info;
  double bf_scale = 0.0f;
  for (j = 0; j < 3; j++) {
    bf_info = &frame_info->bf_info[j];
    for (i = 0; i < 4; i++) {
      if (i != 3) {
        bf_scale = (((double) 64.0/3.0) /
          (sqrt((double) frame_info->edge_softness[j][i] / 1.31) *
          frame_info->noise_profile[j][i]) / 9) * (1 << 10);
        bf_info->bilateral_scale[i] = Round64(bf_scale);
      } else {
        bf_scale = (((double) 64.0/3.0) /
          (sqrt((double) frame_info->edge_softness[j][i] / 1.31) *
          frame_info->noise_profile[j][i])) * (1 << 8);
        bf_info->bilateral_scale[i] = Round64(bf_scale);
      }
      frame_info->weight[j][i] = CLAMP(frame_info->weight[j][i], 0.0f, 1.0f);
      bf_info->weight[i] = 1.0 - frame_info->weight[j][i];
      bf_info->noise_threshold[i] = frame_info->denoise_ratio[j][i] *
        frame_info->noise_profile[j][i];
    }
    bf_info = &frame_info->bf_info[1];
    bf_info->weight[4] = 1.0 - frame_info->weight[1][4];
    bf_info->noise_threshold[4] = frame_info->denoise_ratio[1][4] *
      frame_info->noise_profile[1][4];
    bf_info = &frame_info->bf_info[2];
    bf_info->weight[4] = 1.0 - frame_info->weight[2][4];
    bf_info->noise_threshold[4] = frame_info->denoise_ratio[2][4] *
      frame_info->noise_profile[2][4];
    cpp_debug_bf_info(bf_info);
  }
}

void cpp_debug_asf_info(struct cpp_asf_info *asf_info)
{
  int i;
  CDBG_LOW("sp: %f\n", asf_info->sp);
  CDBG_LOW("neg_abs_y1: %d\n", asf_info->neg_abs_y1);
  CDBG_LOW("dyna_clamp_en: %d\n", asf_info->dyna_clamp_en);
  CDBG_LOW("sp_eff_en: %d\n", asf_info->sp_eff_en);
  CDBG_LOW("clamp_h_ul: %d\n", asf_info->clamp_h_ul);
  CDBG_LOW("clamp_h_ll: %d\n", asf_info->clamp_h_ll);
  CDBG_LOW("clamp_v_ul: %d\n", asf_info->clamp_v_ul);
  CDBG_LOW("clamp_v_ll: %d\n", asf_info->clamp_v_ll);
  CDBG_LOW("clamp_offset_max: %d\n", asf_info->clamp_offset_max);
  CDBG_LOW("clamp_offset_min: %d\n", asf_info->clamp_offset_min);
  CDBG_LOW("clamp_scale_max: %f\n", asf_info->clamp_scale_max);
  CDBG_LOW("clamp_scale_min: %f\n", asf_info->clamp_scale_min);
  CDBG_LOW("nz_flag: %d\n", asf_info->nz_flag);
  CDBG_LOW("nz_flag_f2: %d\n", asf_info->nz_flag_f2);
  CDBG_LOW("nz_flag_f3_f4: %d\n", asf_info->nz_flag_f3_f5);
  CDBG_LOW("sobel_h_coeff\n");
  for (i = 0; i < 4; i++) {
    CDBG_LOW("%f %f %f %f\n",
         asf_info->sobel_h_coeff[i*4], asf_info->sobel_h_coeff[i*4+1],
         asf_info->sobel_h_coeff[i*4+2], asf_info->sobel_h_coeff[i*4+3]);
  }
  CDBG_LOW("sobel_v_coeff\n");
  for (i = 0; i < 4; i++) {
    CDBG_LOW("%f %f %f %f\n",
         asf_info->sobel_v_coeff[i*4], asf_info->sobel_v_coeff[i*4+1],
         asf_info->sobel_v_coeff[i*4+2], asf_info->sobel_v_coeff[i*4+3]);
  }
  CDBG_LOW("hpf_h_coeff\n");
  for (i = 0; i < 4; i++) {
    CDBG_LOW("%f %f %f %f\n",
         asf_info->hpf_h_coeff[i*4], asf_info->hpf_h_coeff[i*4+1],
         asf_info->hpf_h_coeff[i*4+2], asf_info->hpf_h_coeff[i*4+3]);
  }
  CDBG_LOW("hpf_v_coeff\n");
  for (i = 0; i < 4; i++) {
    CDBG_LOW("%f %f %f %f\n",
         asf_info->hpf_v_coeff[i*4], asf_info->hpf_v_coeff[i*4+1],
         asf_info->hpf_v_coeff[i*4+2], asf_info->hpf_v_coeff[i*4+3]);
  }
  CDBG_LOW("lpf_coeff\n");
  for (i = 0; i < 4; i++) {
    CDBG_LOW("%f %f %f %f\n",
         asf_info->lpf_coeff[i*4], asf_info->lpf_coeff[i*4+1],
         asf_info->lpf_coeff[i*4+2], asf_info->lpf_coeff[i*4+3]);
  }
  CDBG_LOW("lut1\n");
  for (i = 0; i < 6; i++) {
    CDBG_LOW("%f %f %f %f\n",
         asf_info->lut1[i*4], asf_info->lut1[i*4+1],
         asf_info->lut1[i*4+2], asf_info->lut1[i*4+3]);
  }
  CDBG_LOW("lut2\n");
    for (i = 0; i < 6; i++) {
      CDBG_LOW("%f %f %f %f\n",
         asf_info->lut2[i*4], asf_info->lut2[i*4+1],
         asf_info->lut2[i*4+2], asf_info->lut2[i*4+3]);
  }
  CDBG_LOW("lut3\n");
  for (i = 0; i < 3; i++) {
    CDBG_LOW("%f %f %f %f\n",
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
  CDBG_LOW("block_width: %d\n", scale_info->block_width);
  CDBG_LOW("block_height: %d\n", scale_info->block_height);
  CDBG_LOW("phase_h_init: %d\n", scale_info->phase_h_init);
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
      stripe_info->sharpen_crop_width_last_pixel + 1;
    stripe_scale_info->block_width = 480;
  } else if (stripe_num == (plane_info->num_stripes - 1)) {
    stripe_scale_info->block_width = stripe_info->scale_output_width;
  } else {
    stripe_scale_info->block_width =
    ((stripe_info->sharpen_crop_width_last_pixel -
     stripe_info->sharpen_crop_width_first_pixel + 1) *
    ((double) (1 << q_factor) / stripe_info->h_phase_step));
    if (stripe_scale_info->block_width > 512) {
      stripe_scale_info->block_width = 512;
    }
    stripe_scale_info->block_width = 489;
  }

  stripe_scale_info->block_width = stripe_info->scale_output_width;

  ((double)(((stripe_info->spatial_denoise_crop_width_last_pixel -
   stripe_info->spatial_denoise_crop_width_first_pixel + 1 - 4) << 21)
   - stripe_info->h_init_phase) / stripe_info->h_phase_step) + 1);

  if (stripe_num != 0 && stripe_num != (plane_info->num_stripes - 1)) {
    stripe_scale_info->block_width = (int)
      ((double)(((stripe_info->spatial_denoise_crop_width_last_pixel -
     stripe_info->spatial_denoise_crop_width_first_pixel + 1 - 4) << 21)
     - stripe_info->h_init_phase) / stripe_info->h_phase_step) + 1;
    stripe_scale_info->block_width = 208;
  }
*/

/*
  stripe_scale_info->block_width = (int)
    ((double)(((stripe_info->spatial_denoise_crop_width_last_pixel -
   stripe_info->spatial_denoise_crop_width_first_pixel + 1 - 4) << 21)
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
//    stripe_info->sharpen_crop_width_last_pixel + 1;
//    (stripe_info->sharpen_crop_width_last_pixel -
//     stripe_info->sharpen_crop_width_first_pixel + 1);// *
//    ((double) (1 << q_factor) / stripe_info->h_phase_step)) - 1;
/*
  stripe_scale_info->block_height =
    ((double)(stripe_info->spatial_denoise_crop_height_last_line -
     stripe_info->spatial_denoise_crop_height_first_line + 1) *
    ((double) (1 << q_factor) / stripe_info->v_phase_step));
*/
  stripe_scale_info->block_width = stripe_info->scale_output_width;
  stripe_scale_info->block_height = stripe_info->scale_output_height;
  stripe_scale_info->phase_h_init = stripe_info->h_init_phase;
  cpp_debug_stripe_scale_info(stripe_scale_info);
}

void cpp_debug_plane_scale_info(struct cpp_plane_scale_info *scale_info)
{
  CDBG_LOW("v_scale_fir_algo: %d\n", scale_info->v_scale_fir_algo);
  CDBG_LOW("h_scale_fir_algo: %d\n", scale_info->h_scale_fir_algo);
  CDBG_LOW("v_scale_algo: %d\n", scale_info->v_scale_algo);
  CDBG_LOW("h_scale_algo: %d\n", scale_info->h_scale_algo);
  CDBG_LOW("subsample_en: %d\n", scale_info->subsample_en);
  CDBG_LOW("upsample_en: %d\n", scale_info->upsample_en);
  CDBG_LOW("vscale_en: %d\n", scale_info->vscale_en);
  CDBG_LOW("hscale_en: %d\n", scale_info->hscale_en);
  CDBG_LOW("phase_h_step: %d\n", scale_info->phase_h_step);
  CDBG_LOW("phase_v_init: %d\n", scale_info->phase_v_init);
  CDBG_LOW("phase_v_step: %d\n", scale_info->phase_v_step);
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

  scale_info->v_scale_fir_algo = 1;
  scale_info->h_scale_fir_algo = 1;

  scale_info->subsample_en = 1;
  scale_info->upsample_en = 1;

  scale_info->phase_h_step = stripe_info->h_phase_step;
  scale_info->phase_v_init = stripe_info->v_init_phase;
  scale_info->phase_v_step = stripe_info->v_phase_step;
  cpp_debug_plane_scale_info(scale_info);
}

void cpp_debug_crop_info(struct cpp_crop_info *crop_info)
{
  CDBG_LOW("enable: %d\n", crop_info->enable);
  CDBG_LOW("first_pixel: %d\n", crop_info->first_pixel);
  CDBG_LOW("last_pixel: %d\n", crop_info->last_pixel);
  CDBG_LOW("first_line: %d\n", crop_info->first_line);
  CDBG_LOW("last_line: %d\n", crop_info->last_line);
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

  bf_crop_info->enable = stripe_info->spatial_denoise_crop_en;
  bf_crop_info->first_pixel = stripe_info->spatial_denoise_crop_width_first_pixel;
  bf_crop_info->last_pixel = stripe_info->spatial_denoise_crop_width_last_pixel;
  bf_crop_info->first_line = stripe_info->spatial_denoise_crop_height_first_line;
  bf_crop_info->last_line = stripe_info->spatial_denoise_crop_height_last_line;

  asf_crop_info->enable = stripe_info->sharpen_crop_en;
  asf_crop_info->first_pixel = stripe_info->sharpen_crop_width_first_pixel;
  asf_crop_info->last_pixel = stripe_info->sharpen_crop_width_last_pixel;
  asf_crop_info->first_line = stripe_info->sharpen_crop_height_first_line;
  asf_crop_info->last_line = stripe_info->sharpen_crop_height_last_line;

  CDBG_LOW("BF Crop info\n");
  cpp_debug_crop_info(bf_crop_info);
  CDBG_LOW("ASF Crop info\n");
  cpp_debug_crop_info(asf_crop_info);
}

void cpp_debug_rotation_info(struct cpp_rotator_info *rot_info)
{
  CDBG_LOW("rot_cfg: %d\n", rot_info->rot_cfg);
  CDBG_LOW("block_width: %d\n", rot_info->block_width);
  CDBG_LOW("block_height: %d\n", rot_info->block_height);
  CDBG_LOW("block_size: %d\n", rot_info->block_size);
  CDBG_LOW("rowIndex0: %d\n", rot_info->rowIndex0);
  CDBG_LOW("rowIndex1: %d\n", rot_info->rowIndex1);
  CDBG_LOW("colIndex0: %d\n", rot_info->colIndex0);
  CDBG_LOW("colIndex1: %d\n", rot_info->colIndex1);
  CDBG_LOW("initIndex: %d\n", rot_info->initIndex);
  CDBG_LOW("modValue: %d\n", rot_info->modValue);
}

void set_default_crop_padding(struct cpp_plane_info_t *plane_info)
{
  plane_info->prescale_padding = 22;//org
  plane_info->postscale_padding = 4;//org
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
  CDBG_LOW("buffer_ptr[0]: %d\n", we_info->buffer_ptr[0]);
  CDBG_LOW("buffer_ptr[1]: %d\n", we_info->buffer_ptr[1]);
  CDBG_LOW("buffer_ptr[2]: %d\n", we_info->buffer_ptr[2]);
  CDBG_LOW("buffer_ptr[3]: %d\n", we_info->buffer_ptr[3]);
  CDBG_LOW("buffer_width: %d\n", we_info->buffer_width);
  CDBG_LOW("buffer_height: %d\n", we_info->buffer_height);
  CDBG_LOW("buffer_stride: %d\n", we_info->buffer_stride);
  CDBG_LOW("blocks_per_col: %d\n", we_info->blocks_per_col);
  CDBG_LOW("blocks_per_row: %d\n", we_info->blocks_per_row);
  CDBG_LOW("h_step: %d\n", we_info->h_step);
  CDBG_LOW("v_step: %d\n", we_info->v_step);
  CDBG_LOW("h_init: %d\n", we_info->h_init);
  CDBG_LOW("v_init: %d\n", we_info->v_init);
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
    output_block_width = rot_info->block_width * stripe_info->output_bytes_per_pixel;
    we_info->buffer_width = output_block_width;
    //we_info->buffer_height = stripe_info->dst_end_y - stripe_info->dst_start_y + 1;
    we_info->buffer_height = output_block_height * blocks_per_stripe;
    we_info->buffer_stride = stripe_info->dst_stride;
    we_info->buffer_ptr[0] = stripe_info->destination_address[0];
    we_info->buffer_ptr[1] = we_info->buffer_ptr[0];
    we_info->buffer_ptr[2] = stripe_info->destination_address[1];
    we_info->buffer_ptr[3] = we_info->buffer_ptr[2];
    we_info->blocks_per_col = blocks_per_stripe;
    we_info->blocks_per_row = 1;
  } else {
    output_block_height = rot_info->block_width;
    output_block_width = rot_info->block_height * stripe_info->output_bytes_per_pixel;
    we_info->buffer_width = (output_block_width * blocks_per_stripe);
    we_info->buffer_height = output_block_height;
    we_info->buffer_stride = stripe_info->dst_stride;
    we_info->buffer_ptr[0] = stripe_info->destination_address[0];
    we_info->buffer_ptr[1] = we_info->buffer_ptr[0];
/*
    if (rot_info->rot_cfg == ROT_90_H_FLIP ||
         rot_info->rot_cfg == ROT_90_HV_FLIP) {
      we_info->buffer_ptr[0] -= stripe_info1->fe_info.top_pad *
        stripe_info->bytes_per_pixel;
    }
*/
    we_info->buffer_ptr[2] = stripe_info->destination_address[1];
    we_info->buffer_ptr[3] = we_info->buffer_ptr[2];
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

/* cpp_pack_asf_kernel_1_4_x:
*
* @frame_msg: pointer to CPP frame payload
* @filter: pointer to ASF H and V filter
* Description:
*     Packs ASF filter values into frame payload as per firmware
*     register.
* Return: void
**/
void cpp_pack_asf_kernel_1_4_x(uint32_t *frame_msg, int16_t *filter)
{
  frame_msg[0] = ((filter[1]) << 16) | ((filter[0]) & 0xFFF);
  frame_msg[1] = ((filter[3]) << 16) | ((filter[2]) & 0xFFF);
  frame_msg[2] = ((filter[4]) << 16);
  frame_msg[3] = ((filter[6]) << 16) | ((filter[5]) & 0xFFF);
  frame_msg[4] = ((filter[7]) & 0xFFF);
  frame_msg[5] = ((filter[9]) << 16) | ((filter[8]) & 0xFFF);
  frame_msg[6] = ((filter[11]) << 16) | ((filter[10]) & 0xFFF);
  frame_msg[7] = ((filter[12]) << 16);
  frame_msg[8] = ((filter[14]) << 16) | ((filter[13]) & 0xFFF);
  frame_msg[9] = ((filter[15]) & 0xFFF);
}

/* cpp_create_frame_message_1_4_x:
*
* @cpp_frame_info: pointer to CPP frame info
* @len: pointer to CPP frame payload length
* Description:
*     Creates/packs CPP frame payload for firmware version
*     1.4 before sending to CPP firmware.
*
* Return: int*
*     Pointer to CPP frame payload
**/
uint32_t* cpp_create_frame_message_1_4_x(
  struct cpp_frame_info_t *cpp_frame_info, uint32_t* len)
{
  struct cpp_stripe_info *stripe_info = NULL;
  int i = 0, j = 0, k = 0, num_stripes = 0, msg_len = 0;
  uint32_t *frame_msg = NULL;
  uint16_t LUT1[24] = {0};
  uint16_t LUT2[24] = {0};
  uint16_t LUT3[12] = {0};
  int16_t  F1[16] = {0};
  int16_t  F2[16] = {0};
  int16_t  F3[16] = {0};
  int16_t  F4[16] = {0};
  int16_t  F5[16] = {0};
  int32_t LUT1_Value[24] = {0};
  int32_t LUT1_Delta[24] = {0};
  int32_t LUT2_Value[24] = {0};
  int32_t LUT2_Delta[24] = {0};
  int32_t LUT3_Value[12] = {0};
  int32_t LUT3_Delta[12] = {0};
  int checksum = 0;
  uint8_t checksum_enable = 0;
  struct cpp_asf_info *asf_info = &(cpp_frame_info->asf_info);
  uint32_t checksum_mask = asf_info->checksum_en;
  uint32_t noise_threshold = 0;

  if (asf_info->sp_eff_en == 1) {
    checksum_mask = 0;
  }

  for (i =0; i < cpp_frame_info->num_planes; i++) {
    num_stripes += cpp_frame_info->plane_info[i].num_stripes;
  }
  /*frame info size*/
  msg_len = 136 + 2 + 27 * num_stripes + 3;
  /*Total message size = frame info + header + length + trailer*/
  frame_msg = malloc(sizeof(uint32_t) * msg_len);
  if (!frame_msg){
      CDBG_ERROR("malloc() failed");
      return NULL;
  }
  memset(frame_msg, 0, sizeof(uint32_t) * msg_len);

  *len = msg_len;

  /*Top Level*/
  frame_msg[0] = 0x3E646D63;
  frame_msg[1] = msg_len - 3;
  frame_msg[2] = 0x6;
  frame_msg[3] = 0;
  frame_msg[4] = 0;
  /*Plane info*/
  frame_msg[5] = (cpp_frame_info->in_plane_fmt << 24) |
    (cpp_frame_info->out_plane_fmt << 16) |
    (cpp_frame_info->plane_info[0].stripe_info1[0].rot_info.rot_cfg << 13) |
    1 << 1 | 0;
  /*Plane address HFR*/
  frame_msg[6] = 0x0;
  frame_msg[7] = 0x0;
  frame_msg[8] = 0x0;
  /*Output Plane HFR*/
  frame_msg[9] = 0x0;
  frame_msg[10] = 0x0;
  frame_msg[11] = 0x0;

  for ( i = 0; i < 16; i++) {
    F1[i] = (int16_t)(Round(asf_info->sobel_h_coeff[i]*(1<<10)));
    F2[i] = (int16_t)(Round(asf_info->sobel_v_coeff[i]*(1<<10)));
    F3[i] = (int16_t)(Round(asf_info->hpf_h_coeff[i]*(1<<10)));
    F4[i] = (int16_t)(Round(asf_info->hpf_v_coeff[i]*(1<<10)));
    F5[i] = (int16_t)(Round(asf_info->lpf_coeff[i]*(1<<10)));
  }
  checksum_enable = checksum_mask & 1;

  if (checksum_enable) {
    checksum = 4*(F1[0] + F1[1] + F1[2]\
      + F1[4] + F1[5] + F1[6]\
      + F1[8] + F1[9] + F1[10])\
      + 2*(F1[3] + F1[7] + F1[11]\
      + F1[12] + F1[13] + F1[14])
      + F1[15];
    if (checksum != 0)
      F1[15] = F1[15]- checksum;
  }

  checksum_enable = (checksum_mask >> 1) & 1;
  if (checksum_enable) {
    checksum = 4*(F2[0] + F2[1] + F2[2]\
      + F2[4] + F2[5] + F2[6]\
      + F2[8] + F2[9] + F2[10])\
      + 2*(F2[3] + F2[7] + F2[11]\
      + F2[12] + F2[13] + F2[14])
      + F2[15];
    if (checksum != 0)
      F2[15] = F2[15]- checksum;
  }
  checksum_enable = (checksum_mask >> 2) & 1;
  if (checksum_enable) {
    checksum = 4*(F3[0] + F3[1] + F3[2]\
      + F3[4] + F3[5] + F3[6]\
      + F3[8] + F3[9] + F3[10])\
      + 2*(F3[3] + F3[7] + F3[11]\
      + F3[12] + F3[13] + F3[14])
      + F3[15];
    if (checksum != 0)
      F3[15] = F3[15]- checksum;
  }

  checksum_enable = (checksum_mask >> 3) & 1;
  if (checksum_enable) {
    checksum = 4*(F4[0] + F4[1] + F4[2]\
      + F4[4] + F4[5] + F4[6]\
      + F4[8] + F4[9] + F4[10])\
      + 2*(F4[3] + F4[7] + F4[11]\
      + F4[12] + F4[13] + F4[14])
      + F4[15];
    if (checksum != 0)
      F4[15] = F4[15]- checksum;
  }

  checksum_enable = (checksum_mask >> 4) & 1 ;
  if (checksum_enable) {
    checksum = 4*(F5[0] + F5[1] + F5[2]\
      + F5[4] + F5[5] + F5[6]\
      + F5[8] + F5[9] + F5[10])\
      + 2*(F5[3] + F5[7] + F5[11]\
      + F5[12] + F5[13] + F5[14])
      + F5[15];
    if (checksum != (1<<10))
      F5[15] = F5[15] + ((1<<10)-checksum);
  }

  for (i = 0; i < 23; i++) {
    LUT1_Delta[i] =
      (int32_t)(Round((asf_info->lut1[i+1]-asf_info->lut1[i])*(1<<4)));
    LUT1_Value[i] =
      (int32_t)(Round((asf_info->lut1[i])*(1<<5)));
    LUT2_Delta[i] =
      (int32_t)(Round((asf_info->lut2[i+1]-asf_info->lut2[i])*(1<<4)));
    LUT2_Value[i] = (int32_t)(Round((asf_info->lut2[i])*(1<<5)));
  }
  LUT1_Delta[23] = 0;
  LUT1_Value[23] = (int32_t)(Round(asf_info->lut1[23]*(1<<5)));
  LUT2_Delta[23] = 0;
  LUT2_Value[23] = (int32_t)(Round(asf_info->lut2[23]*(1<<5)));

  for (i = 0; i < 11; i++) {
    LUT3_Delta[i] =
      (int32_t)(Round((asf_info->lut3[i+1]-asf_info->lut3[i])*(1<<6)));
    LUT3_Value[i] = (int32_t)(Round(asf_info->lut3[i]*(1<<6)));
  }
  LUT3_Delta[11] = 0;
  LUT3_Value[11] = (int32_t)(Round(asf_info->lut3[11]*(1<<6)));

  for (i =0; i < cpp_frame_info->num_planes; i++) {
    frame_msg[12] |= cpp_frame_info->plane_info[i].num_stripes << (i * 10);
  }

  for (i = 0; i < 23; i++) {
    LUT1[i] = ((LUT1_Delta[i]) << 8) | (LUT1_Value[i]);
    LUT2[i] = (( LUT2_Delta[i]) << 8) | (LUT2_Value[i]);
  }
  LUT1[23] = LUT1_Value[23] ;
  LUT2[23] = LUT2_Value[23] ;

  for (i = 0; i < 11; i++) {
    LUT3[i] = ((LUT3_Delta[i]) << 7) | (LUT3_Value[i]);
  }
  LUT3[11] = LUT3_Value[11] ;

  for (i = 0; i < 12; i++) {
    frame_msg[13 + i] = (((LUT1[(i * 2) + 1]) << 16) | (LUT1[i * 2]));
    frame_msg[25 + i] = (((LUT2[(i * 2) + 1]) << 16) | (LUT2[i * 2]));
  }
  for (i = 0; i < 6; i++) {
    frame_msg[37 + i] = (((LUT3[(i * 2) + 1]) << 16) | (LUT3[i * 2]));
  }

  for (i=0; i < 3; i++) {
    struct cpp_bf_info *bf_info = &cpp_frame_info->bf_info[i];
    j= 43 + i * 8;
    for (j = 0; j < 4; j++) {
      k = 43 + i * 8 + j;
      if (j != 3) {
        frame_msg[k] =
          (uint32_t)(bf_info->bilateral_scale[j]);
      } else {
        frame_msg[k] =
          (uint32_t)(bf_info->bilateral_scale[j]);
      }
      noise_threshold = Round(bf_info->noise_threshold[j] * (1 << 4));
      noise_threshold = CLAMP(noise_threshold, 0, 4095);
      frame_msg[k+4] = noise_threshold << 8 |
         ((uint32_t)(Round(bf_info->weight[j] * (1 << 4))));
    }
  }

  frame_msg[67] = (((uint32_t)Round(asf_info->sp * (1 << 4))) & 0x1F) << 4 |
                   (asf_info->neg_abs_y1 & 0x1) << 2 |
                   (asf_info->dyna_clamp_en & 0x1) << 1 |
                   (asf_info->sp_eff_en & 0x1);

  frame_msg[68] = (asf_info->clamp_h_ul & 0x1FF) << 9 |
                  (asf_info->clamp_h_ll & 0x1FF);

  frame_msg[69] = (asf_info->clamp_v_ul & 0x1FF) << 9 |
                  (asf_info->clamp_v_ll & 0x1FF);

  frame_msg[70] = ((uint32_t)(Round(asf_info->clamp_scale_max * (1 << 4)))
                    & 0x1FF) << 16 |
                  ((uint32_t)(Round(asf_info->clamp_scale_min * (1 << 4)))
                    & 0x1FF);

  frame_msg[71] = (asf_info->clamp_offset_max & 0x7F) << 16 |
                  (asf_info->clamp_offset_min & 0x7F);

  frame_msg[72] = asf_info->nz_flag;

  cpp_pack_asf_kernel_1_4_x(&frame_msg[73], &F1[0]);
  cpp_pack_asf_kernel_1_4_x(&frame_msg[83], &F2[0]);
  cpp_pack_asf_kernel_1_4_x(&frame_msg[93], &F3[0]);
  cpp_pack_asf_kernel_1_4_x(&frame_msg[103], &F4[0]);
  cpp_pack_asf_kernel_1_4_x(&frame_msg[113], &F5[0]);

  frame_msg[123] = asf_info->nz_flag_f2;
  frame_msg[124] = asf_info->nz_flag_f3_f5;

  for (i = 0; i < cpp_frame_info->num_planes; i++) {
    j = (123 + 2) + i * 5;
    frame_msg[j] = 0x20 |
      cpp_frame_info->plane_info[i].asf_crop_enable << 4 |
      cpp_frame_info->plane_info[i].bf_crop_enable << 1 |
      cpp_frame_info->plane_info[i].bf_enable;
    if (cpp_frame_info->plane_info[i].scale_info.hscale_en ||
        cpp_frame_info->plane_info[i].scale_info.vscale_en) {
      frame_msg[j] |= 0x4;
    }
    if (cpp_frame_info->plane_info[i].input_plane_fmt == PLANE_Y &&
        cpp_frame_info->asf_mode != 0) {
      frame_msg[j] |= 0x8;
    }

    frame_msg[j+1] =
      cpp_frame_info->plane_info[i].scale_info.v_scale_fir_algo << 12 |
      cpp_frame_info->plane_info[i].scale_info.h_scale_fir_algo << 8 |
      cpp_frame_info->plane_info[i].scale_info.v_scale_algo << 5 |
      cpp_frame_info->plane_info[i].scale_info.h_scale_algo << 4 |
      cpp_frame_info->plane_info[i].scale_info.subsample_en << 3 |
      cpp_frame_info->plane_info[i].scale_info.upsample_en << 2 |
      cpp_frame_info->plane_info[i].scale_info.vscale_en << 1 |
      cpp_frame_info->plane_info[i].scale_info.hscale_en;
    frame_msg[j+2] = cpp_frame_info->plane_info[i].scale_info.phase_h_step;
    frame_msg[j+3] = cpp_frame_info->plane_info[i].scale_info.phase_v_init;
    frame_msg[j+4] = cpp_frame_info->plane_info[i].scale_info.phase_v_step;
  }

  i = 138 + 2;
  for (j = 0; j < cpp_frame_info->num_planes; j++) {
    for (k = 0; k < cpp_frame_info->plane_info[j].num_stripes; k++) {
      stripe_info = &cpp_frame_info->plane_info[j].stripe_info1[k];
      frame_msg[i + 0] = //STRIPE[0]_PP_m_ROT_CFG_0
        stripe_info->rot_info.rot_cfg << 24 |
        (stripe_info->rot_info.block_size - 1);
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
      //STRIPE[0]_FE_n_RD_PNTR
      frame_msg[i + 5] = stripe_info->fe_info.buffer_ptr;
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
      //STRIPE[0]_WE_PLN_0_WR_PNTR
      frame_msg[i + 11] = stripe_info->we_info.buffer_ptr[0];
      //STRIPE[0]_WE_PLN_1_WR_PNTR
      frame_msg[i + 12] = stripe_info->we_info.buffer_ptr[1];
      //STRIPE[0]_WE_PLN_2_WR_PNTR
      frame_msg[i + 13] = stripe_info->we_info.buffer_ptr[2];
      //STRIPE[0]_WE_PLN_3_WR_PNTR
      frame_msg[i + 14] = stripe_info->we_info.buffer_ptr[3];
      frame_msg[i + 15] = //STRIPE[0]_WE_PLN_n_WR_BUFFER_SIZE
        stripe_info->we_info.buffer_height << 16 |
        stripe_info->we_info.buffer_width;
      //STRIPE[0]_WE_PLN_n_WR_STRIDE
      frame_msg[i + 16] = stripe_info->we_info.buffer_stride;
      frame_msg[i + 17] = //STRIPE[0]_WE_PLN_n_WR_CFG_0
        (stripe_info->we_info.blocks_per_row - 1) << 16 |
        (stripe_info->we_info.blocks_per_col - 1);
      //STRIPE[0]_WE_PLN_n_WR_CFG_1
      frame_msg[i + 18] = stripe_info->we_info.h_step;
      //STRIPE[0]_WE_PLN_n_WR_CFG_2
      frame_msg[i + 19] = stripe_info->we_info.v_step;
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
  frame_msg[msg_len-1] = 0xABCDEFAA;
  return frame_msg;
}

/* cpp_create_frame_message_1_2_x:
*
* @cpp_frame_info: pointer to CPP frame info
* @len: pointer to CPP frame payload length
* Description:
*     Creates/packs CPP frame payload for firmware version
*     1.2 before sending to CPP firmware.
*
* Return: int*
*     Pointer to CPP frame payload
**/
uint32_t* cpp_create_frame_message_1_2_x(
  struct cpp_frame_info_t *cpp_frame_info, uint32_t* len)
{
  struct cpp_stripe_info *stripe_info = NULL;
  int i = 0, j = 0, k = 0, num_stripes = 0, msg_len = 0;
  uint32_t *frame_msg = NULL;
  uint16_t LUT1[24] = {0};
  uint16_t LUT2[24] = {0};
  uint16_t LUT3[12] = {0};
  int16_t  F1[16] = {0};
  int16_t  F2[16] = {0};
  int16_t  F3[16] = {0};
  int16_t  F4[16] = {0};
  int16_t  F5[16] = {0};
  int32_t LUT1_Value[24] = {0};
  int32_t LUT1_Delta[24] = {0};
  int32_t LUT2_Value[24] = {0};
  int32_t LUT2_Delta[24] = {0};
  int32_t LUT3_Value[12] = {0};
  int32_t LUT3_Delta[12] = {0};
  int checksum = 0;
  uint8_t checksum_enable = 0;
  struct cpp_asf_info *asf_info = &(cpp_frame_info->asf_info);
  uint32_t checksum_mask = asf_info->checksum_en;
  uint32_t noise_threshold = 0;

  if (asf_info->sp_eff_en == 1) {
    checksum_mask = 0;
  }

  for (i =0; i < cpp_frame_info->num_planes; i++) {
    num_stripes += cpp_frame_info->plane_info[i].num_stripes;
  }
  /*frame info size*/
  msg_len = 126 + 2 + 27 * num_stripes + 3;
  /*Total message size = frame info + header + length + trailer*/
  frame_msg = malloc(sizeof(uint32_t) * msg_len);
  if (!frame_msg){
      CDBG_ERROR("malloc() failed");
      return NULL;
  }
  memset(frame_msg, 0, sizeof(uint32_t) * msg_len);

  *len = msg_len;

  /*Top Level*/
  frame_msg[0] = 0x3E646D63;
  frame_msg[1] = msg_len - 3;
  frame_msg[2] = 0x6;
  frame_msg[3] = 0;
  frame_msg[4] = 0;
  /*Plane info*/
  frame_msg[5] = (cpp_frame_info->in_plane_fmt << 24) |
    (cpp_frame_info->out_plane_fmt << 16) |
    (cpp_frame_info->plane_info[0].stripe_info1[0].rot_info.rot_cfg << 13) |
    1 << 1 | 0;
  /*Plane address HFR*/
  frame_msg[6] = 0x0;
  frame_msg[7] = 0x0;
  frame_msg[8] = 0x0;
  /*Output Plane HFR*/
  frame_msg[9] = 0x0;
  frame_msg[10] = 0x0;
  frame_msg[11] = 0x0;

  for ( i = 0; i < 16; i++) {
    F1[i] = (int16_t)(Round(asf_info->sobel_h_coeff[i]*(1<<10)));
    F2[i] = (int16_t)(Round(asf_info->sobel_v_coeff[i]*(1<<10)));
    F3[i] = (int16_t)(Round(asf_info->hpf_h_coeff[i]*(1<<10)));
    F4[i] = (int16_t)(Round(asf_info->hpf_v_coeff[i]*(1<<10)));
    F5[i] = (int16_t)(Round(asf_info->lpf_coeff[i]*(1<<10)));
  }
  checksum_enable = checksum_mask & 1;

  if (checksum_enable) {
    checksum = 4*(F1[0] + F1[1] + F1[2]\
      + F1[4] + F1[5] + F1[6]\
      + F1[8] + F1[9] + F1[10])\
      + 2*(F1[3] + F1[7] + F1[11]\
      + F1[12] + F1[13] + F1[14])
      + F1[15];
    if (checksum != 0)
      F1[15] = F1[15]- checksum;
  }

  checksum_enable = (checksum_mask >> 1) & 1;
  if (checksum_enable) {
    checksum = 4*(F2[0] + F2[1] + F2[2]\
      + F2[4] + F2[5] + F2[6]\
      + F2[8] + F2[9] + F2[10])\
      + 2*(F2[3] + F2[7] + F2[11]\
      + F2[12] + F2[13] + F2[14])
      + F2[15];
    if (checksum != 0)
      F2[15] = F2[15]- checksum;
  }
  checksum_enable = (checksum_mask >> 2) & 1;
  if (checksum_enable) {
    checksum = 4*(F3[0] + F3[1] + F3[2]\
      + F3[4] + F3[5] + F3[6]\
      + F3[8] + F3[9] + F3[10])\
      + 2*(F3[3] + F3[7] + F3[11]\
      + F3[12] + F3[13] + F3[14])
      + F3[15];
    if (checksum != 0)
      F3[15] = F3[15]- checksum;
  }

  checksum_enable = (checksum_mask >> 3) & 1;
  if (checksum_enable) {
    checksum = 4*(F4[0] + F4[1] + F4[2]\
      + F4[4] + F4[5] + F4[6]\
      + F4[8] + F4[9] + F4[10])\
      + 2*(F4[3] + F4[7] + F4[11]\
      + F4[12] + F4[13] + F4[14])
      + F4[15];
    if (checksum != 0)
      F4[15] = F4[15]- checksum;
  }

  checksum_enable = (checksum_mask >> 4) & 1 ;
  if (checksum_enable) {
    checksum = 4*(F5[0] + F5[1] + F5[2]\
      + F5[4] + F5[5] + F5[6]\
      + F5[8] + F5[9] + F5[10])\
      + 2*(F5[3] + F5[7] + F5[11]\
      + F5[12] + F5[13] + F5[14])
      + F5[15];
    if (checksum != (1<<10))
      F5[15] = F5[15] + ((1<<10)-checksum);
  }

  for (i = 0; i < 23; i++) {
    LUT1_Delta[i] =
      (int32_t)(Round((asf_info->lut1[i+1]-asf_info->lut1[i])*(1<<4)));
    LUT1_Value[i] =
      (int32_t)(Round((asf_info->lut1[i])*(1<<5)));
    LUT2_Delta[i] =
      (int32_t)(Round((asf_info->lut2[i+1]-asf_info->lut2[i])*(1<<4)));
    LUT2_Value[i] = (int32_t)(Round((asf_info->lut2[i])*(1<<5)));
  }
  LUT1_Delta[23] = 0;
  LUT1_Value[23] = (int32_t)(Round(asf_info->lut1[23]*(1<<5)));
  LUT2_Delta[23] = 0;
  LUT2_Value[23] = (int32_t)(Round(asf_info->lut2[23]*(1<<5)));

  for (i = 0; i < 11; i++) {
    LUT3_Delta[i] =
      (int32_t)(Round((asf_info->lut3[i+1]-asf_info->lut3[i])*(1<<6)));
    LUT3_Value[i] = (int32_t)(Round(asf_info->lut3[i]*(1<<6)));
  }
  LUT3_Delta[11] = 0;
  LUT3_Value[11] = (int32_t)(Round(asf_info->lut3[11]*(1<<6)));

  for (i =0; i < cpp_frame_info->num_planes; i++) {
    frame_msg[12] |= cpp_frame_info->plane_info[i].num_stripes << (i * 10);
  }

  for (i = 0; i < 23; i++) {
    LUT1[i] = ((LUT1_Delta[i]) << 8) | (LUT1_Value[i]);
    LUT2[i] = (( LUT2_Delta[i]) << 8) | (LUT2_Value[i]);
  }
  LUT1[23] = LUT1_Value[23] ;
  LUT2[23] = LUT2_Value[23] ;

  for (i = 0; i < 11; i++) {
    LUT3[i] = ((LUT3_Delta[i]) << 7) | (LUT3_Value[i]);
  }
  LUT3[11] = LUT3_Value[11] ;

  for (i = 0; i < 12; i++) {
    frame_msg[13 + i] = (((LUT1[(i * 2) + 1]) << 16) | (LUT1[i * 2]));
    frame_msg[25 + i] = (((LUT2[(i * 2) + 1]) << 16) | (LUT2[i * 2]));
  }
  for (i = 0; i < 6; i++) {
    frame_msg[37 + i] = (((LUT3[(i * 2) + 1]) << 16) | (LUT3[i * 2]));
  }

  for (i=0; i < 3; i++) {
    struct cpp_bf_info *bf_info = &cpp_frame_info->bf_info[i];
    j= 43 + i * 8;
    for (j = 0; j < 4; j++) {
      k = 43 + i * 8 + j;
      //Q10 and Q8 format for bilateral_scale is calculated in cpp_prepare_bf_info()
      frame_msg[k] = (uint32_t)(bf_info->bilateral_scale[j]);
      noise_threshold = Round(bf_info->noise_threshold[j] * (1 << 4));
      noise_threshold = CLAMP(noise_threshold, 0, 4095);
      frame_msg[k+4] = noise_threshold << 8 |
         ((uint32_t)(Round(bf_info->weight[j] * (1 << 4))));
    }
  }

  frame_msg[67] = (((uint32_t)Round(asf_info->sp * (1 << 4))) & 0x1F) << 4 |
                   (asf_info->neg_abs_y1 & 0x1) << 2 |
                   (asf_info->dyna_clamp_en & 0x1) << 1 |
                   (asf_info->sp_eff_en & 0x1);

  frame_msg[68] = (asf_info->clamp_h_ul & 0x1FF) << 9 |
                  (asf_info->clamp_h_ll & 0x1FF);

  frame_msg[69] = (asf_info->clamp_v_ul & 0x1FF) << 9 |
                  (asf_info->clamp_v_ll & 0x1FF);

  frame_msg[70] =
    ((uint32_t)(Round(asf_info->clamp_scale_max * (1 << 4))) & 0x7F) << 16 |
    ((uint32_t)(Round(asf_info->clamp_scale_min * (1 << 4))) & 0x7F);

  frame_msg[71] = (asf_info->clamp_offset_max & 0x7F) << 16 |
                  (asf_info->clamp_offset_min & 0x7F);

  frame_msg[72] = asf_info->nz_flag;

  for (i = 0; i < 8; i++) {
    frame_msg[73+i] =
      ((F1[i*2+1]) << 16 )|  ((F1[i*2]) & 0xFFF);
    frame_msg[81+i] =
      ((F2[i*2+1] ) << 16) | ((F2[i*2] ) & 0xFFF);
    frame_msg[89+i] =
      ((F3[i*2+1] ) << 16) | ((F3[i*2] ) & 0xFFF);
    frame_msg[97+i] =
      ((F4[i*2+1]) << 16) |  ((F4[i*2] ) & 0xFFF);
  }
  for (i = 0; i < 8; i++) {
    frame_msg[105+i] =
      ((F5[i*2+1] ) << 16 )| ((F5[i*2] ) & 0xFFF);
  }


  for (i = 0; i < cpp_frame_info->num_planes; i++) {
    j = (113 + 2) + i * 5;
    frame_msg[j] = 0x20 |
      cpp_frame_info->plane_info[i].asf_crop_enable << 4 |
      cpp_frame_info->plane_info[i].bf_crop_enable << 1 |
      cpp_frame_info->plane_info[i].bf_enable;
    if (cpp_frame_info->plane_info[i].scale_info.hscale_en ||
        cpp_frame_info->plane_info[i].scale_info.vscale_en) {
      frame_msg[j] |= 0x4;
    }
    if (cpp_frame_info->plane_info[i].input_plane_fmt == PLANE_Y &&
        cpp_frame_info->asf_mode != 0) {
      frame_msg[j] |= 0x8;
    }

    frame_msg[j+1] =
      cpp_frame_info->plane_info[i].scale_info.v_scale_fir_algo << 12 |
      cpp_frame_info->plane_info[i].scale_info.h_scale_fir_algo << 8 |
      cpp_frame_info->plane_info[i].scale_info.v_scale_algo << 5 |
      cpp_frame_info->plane_info[i].scale_info.h_scale_algo << 4 |
      cpp_frame_info->plane_info[i].scale_info.subsample_en << 3 |
      cpp_frame_info->plane_info[i].scale_info.upsample_en << 2 |
      cpp_frame_info->plane_info[i].scale_info.vscale_en << 1 |
      cpp_frame_info->plane_info[i].scale_info.hscale_en;
    frame_msg[j+2] = cpp_frame_info->plane_info[i].scale_info.phase_h_step;
    frame_msg[j+3] = cpp_frame_info->plane_info[i].scale_info.phase_v_init;
    frame_msg[j+4] = cpp_frame_info->plane_info[i].scale_info.phase_v_step;
  }

  i = 128 + 2;
  for (j = 0; j < cpp_frame_info->num_planes; j++) {
    for (k = 0; k < cpp_frame_info->plane_info[j].num_stripes; k++) {
      stripe_info = &cpp_frame_info->plane_info[j].stripe_info1[k];
      frame_msg[i + 0] = //STRIPE[0]_PP_m_ROT_CFG_0
        stripe_info->rot_info.rot_cfg << 24 |
        (stripe_info->rot_info.block_size - 1);
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
      //STRIPE[0]_FE_n_RD_PNTR
      frame_msg[i + 5] = stripe_info->fe_info.buffer_ptr;
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
      //STRIPE[0]_WE_PLN_0_WR_PNTR
      frame_msg[i + 11] = stripe_info->we_info.buffer_ptr[0];
      //STRIPE[0]_WE_PLN_1_WR_PNTR
      frame_msg[i + 12] = stripe_info->we_info.buffer_ptr[1];
      //STRIPE[0]_WE_PLN_2_WR_PNTR
      frame_msg[i + 13] = stripe_info->we_info.buffer_ptr[2];
      //STRIPE[0]_WE_PLN_3_WR_PNTR
      frame_msg[i + 14] = stripe_info->we_info.buffer_ptr[3];
      frame_msg[i + 15] = //STRIPE[0]_WE_PLN_n_WR_BUFFER_SIZE
        stripe_info->we_info.buffer_height << 16 |
        stripe_info->we_info.buffer_width;
      //STRIPE[0]_WE_PLN_n_WR_STRIDE
      frame_msg[i + 16] = stripe_info->we_info.buffer_stride;
      frame_msg[i + 17] = //STRIPE[0]_WE_PLN_n_WR_CFG_0
        (stripe_info->we_info.blocks_per_row - 1) << 16 |
        (stripe_info->we_info.blocks_per_col - 1);
      //STRIPE[0]_WE_PLN_n_WR_CFG_1
      frame_msg[i + 18] = stripe_info->we_info.h_step;
      //STRIPE[0]_WE_PLN_n_WR_CFG_2
      frame_msg[i + 19] = stripe_info->we_info.v_step;
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
  frame_msg[msg_len-1] = 0xABCDEFAA;
  return frame_msg;
}

/* cpp_create_frame_message:
*
* @cpphw: pointer to CPP hw struct
* @msm_cpp_frame_info: pointer to kernel CPP driver frame struct
* @cpp_frame_info: pointer to CPP frame info

* Description:
*     Calls CPP create frame based on CPP firmware version
*
* Return: void
*
**/
void cpp_create_frame_message(cpp_hardware_t *cpphw,
  struct msm_cpp_frame_info_t *msm_cpp_frame_info,
  struct cpp_frame_info_t *cpp_frame_info)
{
  if (!msm_cpp_frame_info || !cpp_frame_info || !cpphw) {
    CDBG_ERROR("failed");
    return;
  }
  uint32_t fw_version =
    cpp_hardware_get_fw_version(cpphw);
  switch (fw_version) {
  case CPP_FW_VERSION_1_2_0:
    msm_cpp_frame_info->cpp_cmd_msg =
      cpp_create_frame_message_1_2_x(cpp_frame_info,
        &(msm_cpp_frame_info->msg_len));
    break;
  case CPP_FW_VERSION_1_4_0:
    msm_cpp_frame_info->cpp_cmd_msg =
      cpp_create_frame_message_1_4_x(cpp_frame_info,
        &(msm_cpp_frame_info->msg_len));
    break;
  case CPP_FW_VERSION_1_6_0:
  default:
    CDBG_ERROR("failed, unsupported fw version: %d", fw_version);
    return;
  }
}

void cpp_params_prepare_frame_info(cpp_hardware_t *cpphw,
  struct cpp_frame_info_t *frame_info, struct msm_cpp_frame_info_t *out_info)
{
  struct msm_cpp_frame_strip_info *strip_info;
  struct cpp_stripe_info *stripe_info1;
  struct cpp_plane_info_t *cur_plane_info;
  int i, j, k, strip_num, num_strips;

  out_info->frame_id = frame_info->frame_id;
  out_info->timestamp = frame_info->timestamp;
  out_info->output_buffer_info[0].processed_divert = frame_info->processed_divert;
  out_info->src_fd = frame_info->plane_info[0].src_fd;
  out_info->dst_fd = frame_info->plane_info[0].dst_fd;
  out_info->input_buffer_info.fd = frame_info->plane_info[0].src_fd;
  out_info->input_buffer_info.index = frame_info->buff_index;
  out_info->input_buffer_info.native_buff = frame_info->native_buff;
  out_info->input_buffer_info.identity = frame_info->in_buff_identity;
  out_info->identity = frame_info->identity;
  out_info->cookie = frame_info->cookie;
  out_info->duplicate_output = (int32_t) frame_info->dup_output;
  out_info->duplicate_identity = frame_info->dup_identity;
  out_info->output_buffer_info[0].native_buff =
    frame_info->out_buff_info.native_buff;
  out_info->output_buffer_info[0].fd = frame_info->out_buff_info.fd;
  out_info->output_buffer_info[0].index = frame_info->out_buff_info.index;
  out_info->output_buffer_info[0].offset = frame_info->out_buff_info.offset;

  for (i = 0; i < frame_info->num_planes; i++) {
    cur_plane_info = &frame_info->plane_info[i];
    cpp_init_frame_params(cur_plane_info);
    set_start_of_frame_parameters(cur_plane_info);
    cpp_debug_input_info(cur_plane_info);

    CDBG_LOW("wmcus: %d hmcus: %d num: %d\n",
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
        CDBG_LOW("Plane: %d, x stripe: %d, y stripe: %d\n", i, k, j);
        run_TF_logic(cur_plane_info, k, j, &strip_info[strip_num]);
        run_TW_logic(cur_plane_info, k, j, &strip_info[strip_num]);
#if 0
        if (i == 1 && k == 0) {
          strip_info[strip_num].src_end_x = 0x1d9;
          strip_info[strip_num].src_end_y = 0x21b;
          strip_info[strip_num].spatial_denoise_crop_width_first_pixel = 0;
          strip_info[strip_num].spatial_denoise_crop_width_last_pixel = 0x1c3;
          strip_info[strip_num].spatial_denoise_crop_height_first_line = 0;
          strip_info[strip_num].spatial_denoise_crop_height_last_line = 0x21b;
          strip_info[strip_num].sharpen_crop_width_first_pixel = 2;
          strip_info[strip_num].sharpen_crop_width_last_pixel = 401;
          strip_info[strip_num].sharpen_crop_height_first_line = 0;
          strip_info[strip_num].sharpen_crop_height_last_line = 599;
          strip_info[strip_num].dst_start_x = 0;
          strip_info[strip_num].dst_end_x = 0x1bf;
          strip_info[strip_num].dst_start_y = 0;
          strip_info[strip_num].dst_end_y = 0x21b;
        }
#endif
        cpp_debug_strip_info(&strip_info[strip_num], strip_num);
        stripe_info1[strip_num].rotation = cur_plane_info->rotate;
        stripe_info1[strip_num].mirror = cur_plane_info->mirror;
        CDBG_LOW("debug: mirror %d\n", stripe_info1[strip_num].mirror);

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
  cpp_create_frame_message(cpphw, out_info, frame_info);

  for (i = 0; i < frame_info->num_planes; i++) {
    cur_plane_info = &frame_info->plane_info[i];
    free(cur_plane_info->stripe_block_width);
    free(cur_plane_info->stripe_block_height);
    free(cur_plane_info->stripe_info);
    free(cur_plane_info->stripe_info1);
  }
}

static void cpp_hw_params_set_emboss_effect(cpp_hardware_params_t *hw_params)
{
  uint32_t i = 0;
  cpp_params_asf_info_t *asf_info = &hw_params->asf_info;

  asf_info->sp_eff_en = 1;
  asf_info->sp = 0;
  asf_info->nz_flag = asf_info->nz_flag_f2 = asf_info->nz_flag_f3_f5 = 0x1A90;
  asf_info->dyna_clamp_en = 0;
  for (i = 0; i < CPP_PARAM_ASF_F_KERNEL_MAX; i++) {
    asf_info->sobel_v_coeff[i] = 0.0;
  }
  for (i = 0; i < CPP_PARAM_ASF_F_KERNEL_MAX; i++) {
    asf_info->hpf_h_coeff[i] = 0.0;
  }
  for (i = 0; i < CPP_PARAM_ASF_LUT_MAX; i++) {
    asf_info->lut1[i] = 0.0;
  }
  for (i = 0; i < CPP_PARAM_ASF_LUT3_MAX; i++) {
    asf_info->lut3[i] = 0.0;
  }

  for (i = 0; i < CPP_PARAM_ASF_F_KERNEL_MAX; i++) {
    asf_info->sobel_h_coeff[i] = 0.0;
  }
  asf_info->neg_abs_y1 = 0;
  for (i = 0; i < CPP_PARAM_ASF_F_KERNEL_MAX; i++) {
    asf_info->hpf_v_coeff[i] = 0.0;
  }
  asf_info->hpf_v_coeff[0] = -0.25;
  asf_info->hpf_v_coeff[10] = 0.25;
  for (i = 0; i < CPP_PARAM_ASF_F_KERNEL_MAX; i++) {
    asf_info->lpf_coeff[i] = 0.0;
  }
  asf_info->lpf_coeff[5] = 0.0625;
  for (i = 0; i < CPP_PARAM_ASF_LUT_MAX; i++) {
    asf_info->lut2[i] = 1.0;
  }
  asf_info->clamp_h_ul = 128;
  asf_info->clamp_h_ll = 128;
  asf_info->clamp_v_ul = 255;
  asf_info->clamp_v_ll = -255;
}

static void cpp_hw_params_set_sketch_effect(cpp_hardware_params_t *hw_params)
{
  uint32_t i = 0;
  cpp_params_asf_info_t *asf_info = &hw_params->asf_info;

  asf_info->sp_eff_en = 1;
  asf_info->sp = 0;
  asf_info->nz_flag = asf_info->nz_flag_f2 = asf_info->nz_flag_f3_f5 = 0x1A90;
  asf_info->dyna_clamp_en = 0;
  for (i = 0; i < CPP_PARAM_ASF_F_KERNEL_MAX; i++) {
    asf_info->sobel_v_coeff[i] = 0.0;
  }
  for (i = 0; i < CPP_PARAM_ASF_F_KERNEL_MAX; i++) {
    asf_info->hpf_h_coeff[i] = 0.0;
  }
  for (i = 0; i < CPP_PARAM_ASF_LUT_MAX; i++) {
    asf_info->lut1[i] = 0.0;
  }
  for (i = 0; i < CPP_PARAM_ASF_LUT3_MAX; i++) {
    asf_info->lut3[i] = 0.0;
  }

  for (i = 0; i < CPP_PARAM_ASF_F_KERNEL_MAX; i++) {
    asf_info->sobel_h_coeff[i] = 0.0;
  }

  asf_info->sobel_h_coeff[5] = -0.0625;
  asf_info->sobel_h_coeff[6] = -0.125;
  asf_info->sobel_h_coeff[7] = -0.0625;
  asf_info->sobel_h_coeff[9] = -0.125;
  asf_info->sobel_h_coeff[10] = -0.25;
  asf_info->sobel_h_coeff[11] = -0.25;
  asf_info->sobel_h_coeff[13] = -0.0625;
  asf_info->sobel_h_coeff[14] = -0.25;

  asf_info->neg_abs_y1 = 1;
  for (i = 0; i < CPP_PARAM_ASF_F_KERNEL_MAX; i++) {
    asf_info->hpf_v_coeff[i] = 0.0;
  }
  for (i = 0; i < CPP_PARAM_ASF_F_KERNEL_MAX; i++) {
    asf_info->lpf_coeff[i] = 0.0;
  }
  asf_info->lpf_coeff[15] = 0.25;
  for (i = 0; i < CPP_PARAM_ASF_LUT_MAX; i++) {
    asf_info->lut2[i] = 0.0;
  }
  asf_info->clamp_h_ul = 255;
  asf_info->clamp_h_ll = -255;
  asf_info->clamp_v_ul = 192;
  asf_info->clamp_v_ll = 192;
}

static void cpp_hw_params_set_neon_effect(cpp_hardware_params_t *hw_params)
{
  uint32_t i = 0;
  cpp_params_asf_info_t *asf_info = &hw_params->asf_info;

  asf_info->sp_eff_en = 1;
  asf_info->sp = 0;
  asf_info->nz_flag = asf_info->nz_flag_f2 = asf_info->nz_flag_f3_f5 = 0x1A90;
  asf_info->dyna_clamp_en = 0;
  for (i = 0; i < CPP_PARAM_ASF_F_KERNEL_MAX; i++) {
    asf_info->sobel_v_coeff[i] = 0.0;
  }
  for (i = 0; i < CPP_PARAM_ASF_F_KERNEL_MAX; i++) {
    asf_info->hpf_h_coeff[i] = 0.0;
  }
  for (i = 0; i < CPP_PARAM_ASF_LUT_MAX; i++) {
    asf_info->lut1[i] = 0.0;
  }
  for (i = 0; i < CPP_PARAM_ASF_LUT3_MAX; i++) {
    asf_info->lut3[i] = 0.0;
  }

  for (i = 0; i < CPP_PARAM_ASF_F_KERNEL_MAX; i++) {
    asf_info->sobel_h_coeff[i] = 0.0;
  }
  asf_info->sobel_h_coeff[10] = 0.25;
  asf_info->sobel_h_coeff[11] = 0.25;
  asf_info->sobel_h_coeff[14] = 0.25;
  asf_info->neg_abs_y1 = 0;
  for (i = 0; i < CPP_PARAM_ASF_F_KERNEL_MAX; i++) {
    asf_info->hpf_v_coeff[i] = 0.0;
  }
  for (i = 0; i < CPP_PARAM_ASF_F_KERNEL_MAX; i++) {
    asf_info->lpf_coeff[i] = 0.0;
  }
  for (i = 0; i < CPP_PARAM_ASF_LUT_MAX; i++) {
    asf_info->lut2[i] = 0.0;
  }
  asf_info->clamp_h_ul = 255;
  asf_info->clamp_h_ll = -255;
  asf_info->clamp_v_ul = 0;
  asf_info->clamp_v_ll = 0;
}

static void cpp_hw_params_update_default_asf_params(
  cpp_hardware_params_t *hw_params)
{
  uint32_t i = 0;
  cpp_params_asf_info_t *asf_info = &hw_params->asf_info;

  CDBG_LOW("%s:%d fill default params\n", __func__, __LINE__);
  if (hw_params->asf_mode == CPP_PARAM_ASF_DUAL_FILTER || hw_params->scene_mode_on) {
    asf_info->sobel_h_coeff[0] = 0;
    asf_info->sobel_h_coeff[1] = 0;
    asf_info->sobel_h_coeff[2] = -0.010;
    asf_info->sobel_h_coeff[3] = -0.010;
    asf_info->sobel_h_coeff[4] = 0;
    asf_info->sobel_h_coeff[5] = -0.010;
    asf_info->sobel_h_coeff[6] = -0.030;
    asf_info->sobel_h_coeff[7] = -0.030;
    asf_info->sobel_h_coeff[8] = -0.010;
    asf_info->sobel_h_coeff[9] = -0.030;
    asf_info->sobel_h_coeff[10] = 0;
    asf_info->sobel_h_coeff[11] = 0.070;
    asf_info->sobel_h_coeff[12] = -0.010;
    asf_info->sobel_h_coeff[13] = -0.030;
    asf_info->sobel_h_coeff[14] = 0.070;
    asf_info->sobel_h_coeff[15] = 0.240;

    asf_info->sobel_v_coeff[0] = 0;
    asf_info->sobel_v_coeff[1] = 0;
    asf_info->sobel_v_coeff[2] = -0.010;
    asf_info->sobel_v_coeff[3] = -0.010;
    asf_info->sobel_v_coeff[4] = 0;
    asf_info->sobel_v_coeff[5] = -0.010;
    asf_info->sobel_v_coeff[6] = -0.030;
    asf_info->sobel_v_coeff[7] = -0.030;
    asf_info->sobel_v_coeff[8] = -0.010;
    asf_info->sobel_v_coeff[9] = -0.030;
    asf_info->sobel_v_coeff[10] = 0;
    asf_info->sobel_v_coeff[11] = 0.070;
    asf_info->sobel_v_coeff[12] = -0.010;
    asf_info->sobel_v_coeff[13] = -0.030;
    asf_info->sobel_v_coeff[14] = 0.07;
    asf_info->sobel_v_coeff[15] = 0.24;

    asf_info->hpf_h_coeff[0] = 0;
    asf_info->hpf_h_coeff[1] = 0;
    asf_info->hpf_h_coeff[2] = -0.010;
    asf_info->hpf_h_coeff[3] = -0.010;
    asf_info->hpf_h_coeff[4] = 0;
    asf_info->hpf_h_coeff[5] = -0.010;
    asf_info->hpf_h_coeff[6] = -0.030;
    asf_info->hpf_h_coeff[7] = -0.030;
    asf_info->hpf_h_coeff[8] = -0.010;
    asf_info->hpf_h_coeff[9] = -0.030;
    asf_info->hpf_h_coeff[10] = 0;
    asf_info->hpf_h_coeff[11] = 0.070;
    asf_info->hpf_h_coeff[12] = -0.010;
    asf_info->hpf_h_coeff[13] = -0.030;
    asf_info->hpf_h_coeff[14] = 0.070;
    asf_info->hpf_h_coeff[15] = 0.240;

    asf_info->hpf_v_coeff[0] = 0;
    asf_info->hpf_v_coeff[1] = 0;
    asf_info->hpf_v_coeff[2] = -0.010;
    asf_info->hpf_v_coeff[3] = -0.010;
    asf_info->hpf_v_coeff[4] = 0;
    asf_info->hpf_v_coeff[5] = -0.010;
    asf_info->hpf_v_coeff[6] = -0.030;
    asf_info->hpf_v_coeff[7] = -0.030;
    asf_info->hpf_v_coeff[8] = -0.010;
    asf_info->hpf_v_coeff[9] = -0.030;
    asf_info->hpf_v_coeff[10] = 0;
    asf_info->hpf_v_coeff[11] = 0.070;
    asf_info->hpf_v_coeff[12] = -0.010;
    asf_info->hpf_v_coeff[13] = -0.030;
    asf_info->hpf_v_coeff[14] = 0.07;
    asf_info->hpf_v_coeff[15] = 0.24;

    asf_info->lpf_coeff[0] = 0;
    asf_info->lpf_coeff[1] = 0;
    asf_info->lpf_coeff[2] = 0;
    asf_info->lpf_coeff[3] = 0;
    asf_info->lpf_coeff[4] = 0;
    asf_info->lpf_coeff[6] = 0;
    asf_info->lpf_coeff[7] = 0;
    asf_info->lpf_coeff[8] = 0;
    asf_info->lpf_coeff[9] = 0;
    asf_info->lpf_coeff[10] = 0;
    asf_info->lpf_coeff[11] = 0;
    asf_info->lpf_coeff[12] = 0;
    asf_info->lpf_coeff[13] = 0;
    asf_info->lpf_coeff[14] = 0;
    asf_info->lpf_coeff[15] = 1.0;

    asf_info->lut1[0] =  0.3502;
    asf_info->lut1[1] =  0.6286;
    asf_info->lut1[2] =  1.0791;
    asf_info->lut1[3] =  1.2971;
    asf_info->lut1[4] =  1.3446;
    asf_info->lut1[5] =  1.3497;
    asf_info->lut1[6] =  1.3500;
    asf_info->lut1[7] =  1.3500;
    asf_info->lut1[8] =  1.3500;
    asf_info->lut1[9] =  1.3500;
    asf_info->lut1[10] = 1.3500;
    asf_info->lut1[11] = 1.3500;
    asf_info->lut1[12] = 1.3500;
    asf_info->lut1[13] = 1.3500;
    asf_info->lut1[14] = 1.3500;
    asf_info->lut1[15] = 1.3500;
    asf_info->lut1[16] = 1.3500;
    asf_info->lut1[17] = 1.3500;
    asf_info->lut1[18] = 1.3500;
    asf_info->lut1[19] = 1.3500;
    asf_info->lut1[20] = 1.3500;
    asf_info->lut1[21] = 1.3500;
    asf_info->lut1[22] = 1.3500;
    asf_info->lut1[23] = 1.3500;

    asf_info->lut2[0] =  0.3502;
    asf_info->lut2[1] =  0.6286;
    asf_info->lut2[2] =  1.0791;
    asf_info->lut2[3] =  1.2971;
    asf_info->lut2[4] =  1.3446;
    asf_info->lut2[5] =  1.3497;
    asf_info->lut2[6] =  1.3500;
    asf_info->lut2[7] =  1.3500;
    asf_info->lut2[8] =  1.3500;
    asf_info->lut2[9] =  1.3500;
    asf_info->lut2[10] = 1.3500;
    asf_info->lut2[11] = 1.3500;
    asf_info->lut2[12] = 1.3500;
    asf_info->lut2[13] = 1.3500;
    asf_info->lut2[14] = 1.3500;
    asf_info->lut2[15] = 1.3500;
    asf_info->lut2[16] = 1.3500;
    asf_info->lut2[17] = 1.3500;
    asf_info->lut2[18] = 1.3500;
    asf_info->lut2[19] = 1.3500;
    asf_info->lut2[20] = 1.3500;
    asf_info->lut2[21] = 1.3500;
    asf_info->lut2[22] = 1.3500;
    asf_info->lut2[23] = 1.3500;

    for (i = 0; i < 24; i++) {
      asf_info->lut1[i] *= hw_params->sharpness_level;
      asf_info->lut2[i] *= hw_params->sharpness_level;
    }

    asf_info->lut3[0] = 1;
    asf_info->lut3[1] = 1;
    asf_info->lut3[2] = 1;
    asf_info->lut3[3] = 1;
    asf_info->lut3[4] = 1;
    asf_info->lut3[5] = 1;
    asf_info->lut3[6] = 1;
    asf_info->lut3[7] = 1;
    asf_info->lut3[8] = 1;
    asf_info->lut3[9] = 1;
    asf_info->lut3[10] = 1;
    asf_info->lut3[11] = 1;

    asf_info->sp = 0;
    asf_info->clamp_h_ul = 14;
    asf_info->clamp_h_ll = -14;
    asf_info->clamp_v_ul = 14;
    asf_info->clamp_v_ll = -14;
    asf_info->clamp_scale_max = 1;
    asf_info->clamp_scale_min = 1;
    asf_info->clamp_offset_max = 6;
    asf_info->clamp_offset_min = 6;
  } else if (hw_params->asf_mode == CPP_PARAM_ASF_EMBOSS) {
    cpp_hw_params_set_emboss_effect(hw_params);
  } else if (hw_params->asf_mode == CPP_PARAM_ASF_SKETCH) {
    cpp_hw_params_set_sketch_effect(hw_params);
  } else if (hw_params->asf_mode == CPP_PARAM_ASF_NEON) {
    cpp_hw_params_set_neon_effect(hw_params);
  }
}

static cpp_params_asf_region_t cpp_hw_params_asf_find_region(
  cpp_params_aec_trigger_t *aec_trigger_params)
{
  if (aec_trigger_params->aec_trigger_input <=
      aec_trigger_params->brightlight_trigger_end) {
    return CPP_PARAM_ASF_BRIGHT_LIGHT;
  } else if ((aec_trigger_params->aec_trigger_input >
              aec_trigger_params->brightlight_trigger_end) &&
             (aec_trigger_params->aec_trigger_input <
              aec_trigger_params->brightlight_trigger_start)) {
    return CPP_PARAM_ASF_BRIGHT_LIGHT_INTERPOLATE;
  } else if ((aec_trigger_params->aec_trigger_input >=
              aec_trigger_params->brightlight_trigger_start) &&
             (aec_trigger_params->aec_trigger_input <=
              aec_trigger_params->lowlight_trigger_start)) {
    return CPP_PARAM_ASF_NORMAL_LIGHT;
  } else if ((aec_trigger_params->aec_trigger_input >
              aec_trigger_params->lowlight_trigger_start) &&
             (aec_trigger_params->aec_trigger_input <
              aec_trigger_params->lowlight_trigger_end)) {
    return CPP_PARAM_ASF_LOW_LIGHT_INTERPOLATE;
  } else if (aec_trigger_params->aec_trigger_input >=
             aec_trigger_params->lowlight_trigger_end) {
    return CPP_PARAM_ASF_LOW_LIGHT;
  } else {
    CDBG_ERROR("%s:%d code flow won't reach here\n", __func__, __LINE__);
    return CPP_PARAM_ASF_MAX_LIGHT;
  }
  return CPP_PARAM_ASF_MAX_LIGHT;
}

static void cpp_hw_params_update_asf_kernel_coeff(
  chromatix_asf_7_7_type *asf_7_7, cpp_params_asf_info_t *asf_info,
  ASF_7x7_light_type type)
{
  uint32_t i = 0;

  /* Update en_dyna_clamp */
  asf_info->dyna_clamp_en = asf_7_7->en_dyna_clamp[type];

  for (i = 0; i < CPP_PARAM_ASF_F_KERNEL_MAX; i++) {
    /* Update F1 kernel */
    asf_info->sobel_h_coeff[i] = asf_7_7->f1[type][i];
    /* Update F2 kernel */
    asf_info->sobel_v_coeff[i] = asf_7_7->f2[type][i];
    /* Update F3 kernel */
    asf_info->hpf_h_coeff[i] = asf_7_7->f3[type][i];
    /* Update F4 kernel */
    asf_info->hpf_v_coeff[i] = asf_7_7->f4[type][i];
    /* Update F5 kernel */
    asf_info->lpf_coeff[i] = asf_7_7->f5[i];
  }
  //asf_info->checksum_enable = asf_7_7->checksum_flag;
  asf_info->checksum_enable = 255;
  return;
}

static int32_t cpp_hw_params_fill_asf_kernel(
  chromatix_asf_7_7_type *asf_7_7, cpp_params_asf_info_t *asf_info,
  cpp_params_asf_region_t asf_region)
{
  int32_t ret = 0;
  uint32_t i = 0;
  switch (asf_region) {
  case CPP_PARAM_ASF_LOW_LIGHT:
    cpp_hw_params_update_asf_kernel_coeff(asf_7_7, asf_info,
      ASF_7x7_LOW_LIGHT);
    break;
  case CPP_PARAM_ASF_LOW_LIGHT_INTERPOLATE:
  case CPP_PARAM_ASF_NORMAL_LIGHT:
  case CPP_PARAM_ASF_BRIGHT_LIGHT_INTERPOLATE:
    cpp_hw_params_update_asf_kernel_coeff(asf_7_7, asf_info,
      ASF_7x7_NORMAL_LIGHT);
    break;
  case CPP_PARAM_ASF_BRIGHT_LIGHT:
    cpp_hw_params_update_asf_kernel_coeff(asf_7_7, asf_info,
      ASF_7x7_BRIGHT_LIGHT);
    break;
  default:
    CDBG_ERROR("%s:%d invalid asf_region %d\n", __func__, __LINE__, asf_region);
    ret = -EINVAL;
    break;
  }
  return ret;
}

static float cpp_hw_params_calculate_interpolate_factor(
  float trigger_start, float trigger_end, float trigger_input)
{
  float factor = 0.0f;
  if (trigger_end - trigger_start) {
    factor = (trigger_input - trigger_start) / (trigger_end - trigger_start);
    CDBG_LOW("%s:%d factor %f = (trigger_input %f - trigger_start %f) /"
      "(trigger_end %f - trigger_start %f)\n",
      __func__, __LINE__, factor, trigger_input, trigger_start, trigger_end,
      trigger_start);
  } else {
    CDBG_ERROR("%s:%d trigger start and end has same values %f %f\n", __func__,
      __LINE__, trigger_start, trigger_end);
    factor = (trigger_input - trigger_start);
    CDBG_LOW("%s:%d factor %f = (trigger_input %f - trigger_start %f)\n",
      __func__, __LINE__, factor, trigger_input, trigger_start);
  }
  return factor;
}

static void cpp_hw_params_interpolate_lut_params(
  cpp_hardware_params_t *hw_params,
  chromatix_asf_7_7_type *asf_7_7, cpp_params_asf_info_t *asf_info,
  ASF_7x7_light_type type1, ASF_7x7_light_type type2, float interpolate_factor)
{
  uint32_t i = 0;
  for (i = 0; i < CPP_PARAM_ASF_LUT_MAX; i++) {
    /* Interpolate LUT1 */
    asf_info->lut1[i] = LINEAR_INTERPOLATE(interpolate_factor,
      asf_7_7->lut1[type1][i], asf_7_7->lut1[type2][i]);
    CDBG_LOW("%s:%d lut1[%d] %f = ((factor %f * reference_iplus1 %f) +"
      "((1 - factor) %f * reference_i %f))\n",
      __func__, __LINE__, i, asf_info->lut1[i], interpolate_factor,
      asf_7_7->lut1[type2][i], 1 - interpolate_factor, asf_7_7->lut1[type1][i]);
    /* Update sharpness ratio */
    asf_info->lut1[i] *= hw_params->sharpness_level;
    CDBG_LOW("%s:%d asf_info->lut1[%d] %f *= session_params->sharpness %f\n",
      __func__, __LINE__, i, asf_info->lut1[i], hw_params->sharpness_level);
    /* Interpolate LUT2 */
    asf_info->lut2[i] = LINEAR_INTERPOLATE(interpolate_factor,
      asf_7_7->lut2[type1][i], asf_7_7->lut2[type2][i]);
    CDBG_LOW("%s:%d lut2[%d] %f = ((factor %f * reference_iplus1 %f) +"
      "((1 - factor) %f * reference_i %f))\n",
      __func__, __LINE__, i, asf_info->lut2[i], interpolate_factor,
      asf_7_7->lut2[type2][i], 1 - interpolate_factor, asf_7_7->lut2[type1][i]);
    /* Update sharpness ratio */
    asf_info->lut2[i] *= hw_params->sharpness_level;
    CDBG_LOW("%s:%d asf_info->lut2[%d] %f *= session_params->sharpness %f\n",
      __func__, __LINE__, i, asf_info->lut2[i], hw_params->sharpness_level);
  }
  for (i = 0; i < CPP_PARAM_ASF_LUT3_MAX; i++) {
    /* Interpolate LUT3 */
    asf_info->lut3[i] = LINEAR_INTERPOLATE(interpolate_factor,
      asf_7_7->lut3[type1][i], asf_7_7->lut3[type2][i]);
    CDBG_LOW("%s:%d lut3[%d] %f = ((factor %f * reference_iplus1 %f) +"
      "((1 - factor) %f * reference_i %f))\n",
      __func__, __LINE__, i, asf_info->lut3[i], interpolate_factor,
      asf_7_7->lut3[type2][i], 1 - interpolate_factor, asf_7_7->lut3[type1][i]);
  }
  /* Interpolate sp */
  asf_info->sp = LINEAR_INTERPOLATE(interpolate_factor, asf_7_7->sp[type1],
    asf_7_7->sp[type2]);
  /* convert the scale of [0-100] to [0.0-1.0]*/
  asf_info->sp /= 100.0f;
  /* Interpolate clamp */
  asf_info->clamp_h_ul = Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->reg_hh[type1], asf_7_7->reg_hh[type2]));
  asf_info->clamp_h_ll = Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->reg_hl[type1], asf_7_7->reg_hl[type2]));
  asf_info->clamp_v_ul = Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->reg_vh[type1], asf_7_7->reg_vh[type2]));
  asf_info->clamp_v_ll = Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->reg_vl[type1], asf_7_7->reg_vl[type2]));
  asf_info->clamp_scale_max = LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->smax[type1], asf_7_7->smax[type2]);
  asf_info->clamp_scale_min = LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->smin[type1], asf_7_7->smin[type2]);
  asf_info->clamp_offset_max = Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->omax[type1], asf_7_7->omax[type2]));
  asf_info->clamp_offset_min = Round(LINEAR_INTERPOLATE(interpolate_factor,
    asf_7_7->omin[type1], asf_7_7->omin[type2]));

  return;
}

static void cpp_hw_params_fill_noninterpolate_params(
  cpp_hardware_params_t *hw_params, chromatix_asf_7_7_type *asf_7_7,
  cpp_params_asf_info_t *asf_info, ASF_7x7_light_type asf_region)
{
  uint32_t i = 0;
  for (i = 0; i < CPP_PARAM_ASF_LUT_MAX; i++) {
    asf_info->lut1[i] = asf_7_7->lut1[asf_region][i] *
      hw_params->sharpness_level;
    asf_info->lut2[i] = asf_7_7->lut2[asf_region][i] *
      hw_params->sharpness_level;
  }
  for (i = 0; i < CPP_PARAM_ASF_LUT3_MAX; i++) {
    asf_info->lut3[i] = asf_7_7->lut3[asf_region][i];
  }
  asf_info->sp = asf_7_7->sp[asf_region] / 100.0f;
  asf_info->clamp_h_ul = asf_7_7->reg_hh[asf_region];
  asf_info->clamp_h_ll = asf_7_7->reg_hl[asf_region];
  asf_info->clamp_v_ul = asf_7_7->reg_vh[asf_region];
  asf_info->clamp_v_ll = asf_7_7->reg_vl[asf_region];
  asf_info->clamp_scale_max = asf_7_7->smax[asf_region];
  asf_info->clamp_scale_min = asf_7_7->smin[asf_region];
  asf_info->clamp_offset_max = asf_7_7->omax[asf_region];
  asf_info->clamp_offset_min = asf_7_7->omin[asf_region];
}

static int32_t cpp_hw_params_fill_lut_params(
  cpp_hardware_params_t *hw_params,
  chromatix_asf_7_7_type *asf_7_7, cpp_params_asf_info_t *asf_info,
  cpp_params_aec_trigger_t *aec_trigger_params,
  cpp_params_asf_region_t asf_region)
{
  int32_t ret = 0;
  uint32_t i = 0;
  float interpolate_factor = 0.0f;
  switch (asf_region) {
  case CPP_PARAM_ASF_LOW_LIGHT:
    cpp_hw_params_fill_noninterpolate_params(hw_params, asf_7_7, asf_info,
      ASF_7x7_LOW_LIGHT);
    break;
  case CPP_PARAM_ASF_LOW_LIGHT_INTERPOLATE:
    interpolate_factor = cpp_hw_params_calculate_interpolate_factor(
      aec_trigger_params->lowlight_trigger_start,
      aec_trigger_params->lowlight_trigger_end,
      aec_trigger_params->aec_trigger_input);
    if (interpolate_factor == 0.0f) {
      CDBG_ERROR("%s:%d interpolate_factor zero\n", __func__, __LINE__);
      ret = -EINVAL;
      break;
    }
    cpp_hw_params_interpolate_lut_params(hw_params,
      asf_7_7, asf_info, ASF_7x7_NORMAL_LIGHT, ASF_7x7_LOW_LIGHT,
      interpolate_factor);
    break;
  case CPP_PARAM_ASF_NORMAL_LIGHT:
    cpp_hw_params_fill_noninterpolate_params(hw_params, asf_7_7, asf_info,
      ASF_7x7_NORMAL_LIGHT);
    break;
  case CPP_PARAM_ASF_BRIGHT_LIGHT:
    cpp_hw_params_fill_noninterpolate_params(hw_params, asf_7_7, asf_info,
      ASF_7x7_BRIGHT_LIGHT);
    break;
  case CPP_PARAM_ASF_BRIGHT_LIGHT_INTERPOLATE:
    interpolate_factor = cpp_hw_params_calculate_interpolate_factor(
      aec_trigger_params->brightlight_trigger_start,
      aec_trigger_params->brightlight_trigger_end,
      aec_trigger_params->aec_trigger_input);
    if (interpolate_factor == 0.0f) {
      CDBG_ERROR("%s:%d interpolate_factor zero\n", __func__, __LINE__);
      ret = -EINVAL;
      break;
    }
    cpp_hw_params_interpolate_lut_params(hw_params,
      asf_7_7, asf_info, ASF_7x7_NORMAL_LIGHT, ASF_7x7_BRIGHT_LIGHT,
      interpolate_factor);
    break;
  default:
    CDBG_ERROR("%s:%d invalid asf_region %d\n", __func__, __LINE__,
      asf_region);
    ret = -EINVAL;
    break;
  }
  return ret;
}

static int32_t cpp_hw_params_update_asf_params(cpp_hardware_params_t *hw_params,
  chromatix_parms_type  *chromatix_ptr,
  cpp_params_aec_trigger_t *aec_trigger_params)
{
  int32_t                 ret = 0;
  uint32_t                i = 0;
  chromatix_ASF_7x7_type *chromatix_ASF_7x7 = NULL;
  chromatix_asf_7_7_type *asf_7_7 = NULL;
  cpp_params_asf_info_t  *asf_info = NULL;
  cpp_params_asf_region_t asf_region = CPP_PARAM_ASF_MAX_LIGHT;

  chromatix_ASF_7x7 = &chromatix_ptr->chromatix_VFE.chromatix_ASF_7x7;
  asf_7_7 = &chromatix_ASF_7x7->asf_7_7;
  asf_info = &hw_params->asf_info;

  /* Fill downscale factor in asf_info which will be used later to adjust
     LUT params */
  asf_info->sharp_min_ds_factor =
    chromatix_ASF_7x7->asf_7_7_sharp_min_ds_factor;
  asf_info->sharp_max_ds_factor =
    chromatix_ASF_7x7->asf_7_7_sharp_max_ds_factor;
  asf_info->sharp_max_factor =
    chromatix_ASF_7x7->asf_7_7_sharp_max_factor;

  /* Fill ASF parameters in frame_params */
  if (!chromatix_ASF_7x7->asf_7_7.asf_en ||
    aec_trigger_params->aec_trigger_input <= 0.0f) {
    /* Use default */
    cpp_hw_params_update_default_asf_params(hw_params);
    return ret;
  }
  if (hw_params->asf_mode == CPP_PARAM_ASF_DUAL_FILTER || hw_params->scene_mode_on) {
    asf_region = cpp_hw_params_asf_find_region(aec_trigger_params);
    if (asf_region >= CPP_PARAM_ASF_MAX_LIGHT) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      return -EINVAL;
    }
    CDBG_LOW("%s:%d asf region %d\n", __func__, __LINE__, asf_region);
    /* Update F kernel and sp */
    ret = cpp_hw_params_fill_asf_kernel(asf_7_7, asf_info, asf_region);
    if (ret < 0) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      return ret;
    }
    /* Interpolate LUT params */
    ret = cpp_hw_params_fill_lut_params(hw_params, asf_7_7,
      asf_info, aec_trigger_params, asf_region);
    if (ret < 0) {
      CDBG_ERROR("%s:%d failed\n", __func__, __LINE__);
      return ret;
    }
    asf_info->sp_eff_en = 0;
    asf_info->neg_abs_y1 = asf_7_7->neg_abs_y1;
    asf_info->nz_flag = asf_info->nz_flag_f2 = asf_info->nz_flag_f3_f5 = asf_7_7->nz[0];

    CDBG_LOW("%s:%d min_ds %f max_ds %f max_factor %f\n", __func__, __LINE__,
      asf_info->sharp_min_ds_factor, asf_info->sharp_max_ds_factor,
      asf_info->sharp_max_factor);

  } else if (hw_params->asf_mode == CPP_PARAM_ASF_EMBOSS) {
    cpp_hw_params_set_emboss_effect(hw_params);
  } else if (hw_params->asf_mode == CPP_PARAM_ASF_SKETCH) {
    cpp_hw_params_set_sketch_effect(hw_params);
  } else if (hw_params->asf_mode == CPP_PARAM_ASF_NEON) {
    cpp_hw_params_set_neon_effect(hw_params);
  }
  return 0;
}

int32_t cpp_hw_params_asf_interpolate_1_2_x(cpp_hardware_params_t *hw_params,
  chromatix_parms_type  *chromatix_ptr,
  cpp_params_aec_trigger_info_t *trigger_input)
{
  chromatix_ASF_7x7_type *chromatix_ASF_7x7 = NULL;
  cpp_params_aec_trigger_t aec_trigger_params;

  if (hw_params->asf_lock) {
    CDBG_ERROR("%s:%d ASFe is locked by Chromatix",__func__, __LINE__);
    return 0;
  }

  if (!chromatix_ptr) {
    CDBG_LOW("%s:%d chromatix NULL\n", __func__, __LINE__);
    /* Use default */
    cpp_hw_params_update_default_asf_params(hw_params);
    return 0;
  }

  chromatix_ASF_7x7 = &chromatix_ptr->chromatix_VFE.chromatix_ASF_7x7;

  hw_params->aec_trigger.lux_idx = trigger_input->lux_idx;
  hw_params->aec_trigger.gain = trigger_input->gain;

  /* determine the control method */
  if (chromatix_ASF_7x7->control_asf_7x7 == 0) {
    /* Lux index based */
    aec_trigger_params.lowlight_trigger_start =
      chromatix_ASF_7x7->asf_7x7_lowlight_trigger.lux_index_start;
    aec_trigger_params.lowlight_trigger_end =
      chromatix_ASF_7x7->asf_7x7_lowlight_trigger.lux_index_end;
    aec_trigger_params.brightlight_trigger_start =
      chromatix_ASF_7x7->asf_7x7_brightlight_trigger.lux_index_start;
    aec_trigger_params.brightlight_trigger_end =
      chromatix_ASF_7x7->asf_7x7_brightlight_trigger.lux_index_end;
    aec_trigger_params.aec_trigger_input = trigger_input->lux_idx;
  } else if (chromatix_ASF_7x7->control_asf_7x7 == 1) {
    /* Gain based */
    aec_trigger_params.lowlight_trigger_start =
      chromatix_ASF_7x7->asf_7x7_lowlight_trigger.gain_start;
    aec_trigger_params.lowlight_trigger_end =
      chromatix_ASF_7x7->asf_7x7_lowlight_trigger.gain_end;
    aec_trigger_params.brightlight_trigger_start =
      chromatix_ASF_7x7->asf_7x7_brightlight_trigger.gain_start;
    aec_trigger_params.brightlight_trigger_end =
      chromatix_ASF_7x7->asf_7x7_brightlight_trigger.gain_end;
    aec_trigger_params.aec_trigger_input = trigger_input->gain;
  } else {
    /* Error in chromatix */
    CDBG_ERROR("%s:%d error in chromatix control type\n", __func__, __LINE__);
    return -EINVAL;
  }
  CDBG_LOW("%s:%d low start, end %f %f, bright start, end %f %f, trigger %f\n",
    __func__, __LINE__, aec_trigger_params.lowlight_trigger_start,
    aec_trigger_params.lowlight_trigger_end,
    aec_trigger_params.brightlight_trigger_start,
    aec_trigger_params.brightlight_trigger_end,
    aec_trigger_params.aec_trigger_input);

  return cpp_hw_params_update_asf_params(hw_params, chromatix_ptr,
    &aec_trigger_params);
}

/* cpp_hw_params_asf_interpolate:
*
* @cpphw: pointer to CPP hw struct
* @hw_params: pointer to CPP hw parm struct
* @chromatix_ptr: pointer to chromatix parameter struct
* @trigger_input: pointer to AEC trigger input
* Description:
*     Interpolates ASF values based on AEC trigger input
*
* Return: int
*     0: if success, negative value otherwise.
**/
int32_t cpp_hw_params_asf_interpolate(cpp_hardware_t *cpphw,
  cpp_hardware_params_t *hw_params,
  chromatix_parms_type  *chromatix_ptr,
  cpp_params_aec_trigger_info_t *trigger_input)
{
  if (!cpphw || !hw_params || !chromatix_ptr || !trigger_input) {
    CDBG_ERROR("Fail,cpphw=%p, hw_params=%p, chromatix_ptr=%p, trigger=%p",
      cpphw, hw_params, chromatix_ptr, trigger_input);
    return -EINVAL;
  }
  uint32_t fw_version =
    cpp_hardware_get_fw_version(cpphw);
  CDBG_LOW("fw_version = %x", fw_version);
  switch (fw_version) {
  case CPP_FW_VERSION_1_2_0:
  case CPP_FW_VERSION_1_4_0:
    return cpp_hw_params_asf_interpolate_1_2_x(hw_params,
             chromatix_ptr, trigger_input);
  case CPP_FW_VERSION_1_6_0:
  default:
    CDBG_ERROR("failed, unsupported fw version: 0x%x", fw_version);
    return -EINVAL;
  }
  return 0;
}
int32_t cpp_hw_params_init_wnr_params(cpp_hardware_params_t *cpp_hw_params)
{
  uint32_t j, k;
  uint32_t profile = 8;

  if (!cpp_hw_params) {
    CDBG_ERROR("%s:%d frame params error\n", __func__, __LINE__);
    return -EINVAL;
  }

  for (k = 0; k < CPP_DENOISE_NUM_PROFILES; k++) {
    for (j = 0; j < CPP_DENOISE_NUM_PLANES; j++) {
      cpp_hw_params->denoise_info[j][k].noise_profile = profile;
      cpp_hw_params->denoise_info[j][k].weight = 0;
      cpp_hw_params->denoise_info[j][k].edge_softness = 12.75;
      cpp_hw_params->denoise_info[j][k].denoise_ratio = 12.75;
    }
    /* Update profile value for next level */
    profile /= 2;
  }

  return 0;
}

int32_t cpp_hw_params_noninterpolate_wnr_params(
  cpp_hardware_params_t *cpp_hw_params,
  ReferenceNoiseProfile_type *ref_noise_profile)
{
  uint32_t k, j, offset;

  if (!cpp_hw_params || !ref_noise_profile) {
    CDBG_ERROR("%s:%d frame params error\n", __func__, __LINE__);
    return -EINVAL;
  }

  for (k = 0; k < CPP_DENOISE_NUM_PROFILES; k++) {
    for (j = 0; j < CPP_DENOISE_NUM_PLANES; j++) {
      offset = k + 4 + j * 8;
      cpp_hw_params->denoise_info[j][k].noise_profile =
        ref_noise_profile->referenceNoiseProfileData[offset];
    }

    /* Weight factor */
    cpp_hw_params->denoise_info[0][k].weight =
      ref_noise_profile->denoise_weight_y[k];
    cpp_hw_params->denoise_info[1][k].weight =
      ref_noise_profile->denoise_weight_chroma[k];
    cpp_hw_params->denoise_info[2][k].weight =
      ref_noise_profile->denoise_weight_chroma[k];

    /* edge softness factor */
    cpp_hw_params->denoise_info[0][k].edge_softness =
      ref_noise_profile->denoise_edge_softness_y[k];
    cpp_hw_params->denoise_info[1][k].edge_softness =
      ref_noise_profile->denoise_edge_softness_chroma[k];
    cpp_hw_params->denoise_info[2][k].edge_softness =
      ref_noise_profile->denoise_edge_softness_chroma[k];

    /* denoise ratio */
    cpp_hw_params->denoise_info[0][k].denoise_ratio =
      ref_noise_profile->denoise_scale_y[k];
    cpp_hw_params->denoise_info[1][k].denoise_ratio =
      ref_noise_profile->denoise_scale_chroma[k];
    cpp_hw_params->denoise_info[2][k].denoise_ratio =
      ref_noise_profile->denoise_scale_chroma[k];
  }
  return 0;
}

int32_t cpp_hw_params_interpolate_wnr_params(float interpolation_factor,
  cpp_hardware_params_t *cpp_hw_params,
  ReferenceNoiseProfile_type *ref_noise_profile_i,
  ReferenceNoiseProfile_type *ref_noise_profile_iplus1)
{
  uint32_t k, j, offset;

  if (!cpp_hw_params || !ref_noise_profile_i || !ref_noise_profile_iplus1) {
    CDBG_ERROR("%s:%d frame params error\n", __func__, __LINE__);
    return -EINVAL;
  }

  for (k = 0; k < CPP_DENOISE_NUM_PROFILES; k++) {
    for (j = 0; j < CPP_DENOISE_NUM_PLANES; j++) {
      offset = k + 4 + j * 8;
      cpp_hw_params->denoise_info[j][k].noise_profile =
        LINEAR_INTERPOLATE(interpolation_factor,
        ref_noise_profile_i->referenceNoiseProfileData[offset],
        ref_noise_profile_iplus1->referenceNoiseProfileData[offset]);
    }

    /* Weight factor */
    cpp_hw_params->denoise_info[0][k].weight =
      LINEAR_INTERPOLATE(interpolation_factor,
      ref_noise_profile_i->denoise_weight_y[k],
      ref_noise_profile_iplus1->denoise_weight_y[k]);
    cpp_hw_params->denoise_info[1][k].weight =
      LINEAR_INTERPOLATE(interpolation_factor,
      ref_noise_profile_i->denoise_weight_chroma[k],
      ref_noise_profile_iplus1->denoise_weight_chroma[k]);
    cpp_hw_params->denoise_info[2][k].weight =
      LINEAR_INTERPOLATE(interpolation_factor,
      ref_noise_profile_i->denoise_weight_chroma[k],
      ref_noise_profile_iplus1->denoise_weight_chroma[k]);

    /* edge softness factor */
    cpp_hw_params->denoise_info[0][k].edge_softness =
      LINEAR_INTERPOLATE(interpolation_factor,
      ref_noise_profile_i->denoise_edge_softness_y[k],
      ref_noise_profile_iplus1->denoise_edge_softness_y[k]);
    cpp_hw_params->denoise_info[1][k].edge_softness =
      LINEAR_INTERPOLATE(interpolation_factor,
      ref_noise_profile_i->denoise_edge_softness_chroma[k],
      ref_noise_profile_iplus1->denoise_edge_softness_chroma[k]);
    cpp_hw_params->denoise_info[2][k].edge_softness =
      LINEAR_INTERPOLATE(interpolation_factor,
      ref_noise_profile_i->denoise_edge_softness_chroma[k],
      ref_noise_profile_iplus1->denoise_edge_softness_chroma[k]);

    /* denoise ratio */
    cpp_hw_params->denoise_info[0][k].denoise_ratio =
      LINEAR_INTERPOLATE(interpolation_factor,
      ref_noise_profile_i->denoise_scale_y[k],
      ref_noise_profile_iplus1->denoise_scale_y[k]);
    cpp_hw_params->denoise_info[1][k].denoise_ratio =
      LINEAR_INTERPOLATE(interpolation_factor,
      ref_noise_profile_i->denoise_scale_chroma[k],
      ref_noise_profile_iplus1->denoise_scale_chroma[k]);
    cpp_hw_params->denoise_info[2][k].denoise_ratio =
      LINEAR_INTERPOLATE(interpolation_factor,
      ref_noise_profile_i->denoise_scale_chroma[k],
      ref_noise_profile_iplus1->denoise_scale_chroma[k]);
  }
  return 0;
}

int32_t cpp_hw_params_update_wnr_params(chromatix_parms_type *chromatix_ptr,
  cpp_hardware_params_t *hw_params, cpp_params_aec_trigger_info_t *aec_trigger)
{
  wavelet_denoise_type       *wavelet_denoise;
  uint32_t                    i;
  float                       trigger_i, trigger_iplus1;
  float                       interpolation_factor;
  ReferenceNoiseProfile_type *ref_noise_profile_i;
  ReferenceNoiseProfile_type *ref_noise_profile_iplus1;
  float                       numerator, denominator;
  float                       trigger_input;

  if (!hw_params) {
    CDBG_ERROR("%s:%d] failed hw_params:%p, chromatix_ptr:%p\n", __func__,
      __LINE__, chromatix_ptr, hw_params);
    return -EINVAL;
  }

  if (hw_params->denoise_lock) {
    CDBG_ERROR("%s:%d Wavelet denoise is locked by Chromatix",
      __func__, __LINE__);
    return 0;
  }

  if (!chromatix_ptr) {
    cpp_hw_params_init_wnr_params(hw_params);
    return 0;
  } else {
    wavelet_denoise =
      &chromatix_ptr->chromatix_VFE.chromatix_wavelet.wavelet_denoise_HW_420;

    hw_params->aec_trigger.lux_idx = aec_trigger->lux_idx;
    hw_params->aec_trigger.gain = aec_trigger->gain;

    if (wavelet_denoise->control_denoise == 0) {
      /* Lux index based */
      trigger_input = aec_trigger->lux_idx;
    } else {
      /* Gain based */
      trigger_input = aec_trigger->gain;
    }

    if (trigger_input <= 0.0f) {
      cpp_hw_params_init_wnr_params(hw_params);
      CDBG_ERROR("%s:%d invalid trigger input %f\n", __func__, __LINE__,
        trigger_input);
      return 0;
    }
    if (wavelet_denoise->noise_profile[0].trigger_value >= trigger_input) {
      cpp_hw_params_interpolate_wnr_params(0, hw_params,
        &wavelet_denoise->noise_profile[0],
        &wavelet_denoise->noise_profile[0]);
      return 0;
    }

    /* Find the range in the availble grey patches */
    for (i = 0; i < NUM_GRAY_PATCHES - 1; i++) {
        trigger_i = wavelet_denoise->noise_profile[i].trigger_value;
        trigger_iplus1 = wavelet_denoise->noise_profile[i+1].trigger_value;
        if ((trigger_input > trigger_i) && (trigger_input <= trigger_iplus1)) {
          /* Interpolate all the values using i & iplus1 */
          numerator = (trigger_input - trigger_i);
          denominator = (trigger_iplus1 - trigger_i);
          if (denominator == 0.0f) {
            return 0;
          }
          interpolation_factor = numerator / denominator;
          ref_noise_profile_i = &wavelet_denoise->noise_profile[i];
          ref_noise_profile_iplus1 = &wavelet_denoise->noise_profile[i+1];

          cpp_hw_params_interpolate_wnr_params(interpolation_factor,
            hw_params, ref_noise_profile_i,
            ref_noise_profile_iplus1);
          return 0;
        } /* else iterate */
    }

    if (i == (NUM_GRAY_PATCHES - 1)) {
      cpp_hw_params_interpolate_wnr_params(0, hw_params,
        &wavelet_denoise->noise_profile[NUM_GRAY_PATCHES - 1],
        &wavelet_denoise->noise_profile[NUM_GRAY_PATCHES - 1]);
      return 0;
    }
  }

  return -EINVAL;
}

int32_t cpp_params_calculate_crop(cpp_hardware_params_t *hw_params)
{
  uint32_t x, y;
  assert(hw_params->input_info.width != 0);
  assert(hw_params->input_info.height != 0);

  /* Due to limitation from ISP, need to assume
     (0, 0, 0, 0) as indication of no-crop setting */
  if(hw_params->crop_info.stream_crop.x == 0 &&
      hw_params->crop_info.stream_crop.y == 0 &&
      hw_params->crop_info.stream_crop.dx == 0 &&
      hw_params->crop_info.stream_crop.dy == 0) {
    hw_params->crop_info.stream_crop.dx = hw_params->input_info.width;
    hw_params->crop_info.stream_crop.dy = hw_params->input_info.height;
  }

  x = (hw_params->crop_info.stream_crop.x * hw_params->crop_info.is_crop.dx) /
    hw_params->input_info.width;
  y = (hw_params->crop_info.stream_crop.y * hw_params->crop_info.is_crop.dy) /
    hw_params->input_info.height;

  /* calculate the first pixel in window */
  hw_params->crop_info.process_window_first_pixel =
    x + hw_params->crop_info.is_crop.x;
  /* calculate the first line in window */
  hw_params->crop_info.process_window_first_line =
    y + hw_params->crop_info.is_crop.y;
  /* calculate the window width */
  hw_params->crop_info.process_window_width =
    (hw_params->crop_info.stream_crop.dx * hw_params->crop_info.is_crop.dx)/
     hw_params->input_info.width;
  /* calculate the window height */
  hw_params->crop_info.process_window_height =
    (hw_params->crop_info.stream_crop.dy * hw_params->crop_info.is_crop.dy)/
     hw_params->input_info.height;
  if (hw_params->crop_info.process_window_height &&
    hw_params->output_info.height) {
    float dst_aspect_ratio = (float)hw_params->output_info.width /
      (float)hw_params->output_info.height;
    float src_aspect_ratio = (float)hw_params->crop_info.process_window_width /
      (float)hw_params->crop_info.process_window_height;
    uint32_t process_window_first_pixel, process_window_first_line;
    uint32_t process_window_width, process_window_height;

    if (dst_aspect_ratio > src_aspect_ratio) {
      process_window_height =
        (float)hw_params->crop_info.process_window_width / dst_aspect_ratio;
      process_window_width = hw_params->crop_info.process_window_width;
      process_window_first_pixel =
        hw_params->crop_info.process_window_first_pixel;
      process_window_first_line =
        hw_params->crop_info.process_window_first_line +
        ((float)(hw_params->crop_info.process_window_height -
        process_window_height)) / 2.0;
    } else {
      process_window_width =
        (float)hw_params->crop_info.process_window_height * dst_aspect_ratio;
      process_window_height = hw_params->crop_info.process_window_height;
      process_window_first_line =
        hw_params->crop_info.process_window_first_line;
      process_window_first_pixel =
        hw_params->crop_info.process_window_first_pixel +
        ((float)(hw_params->crop_info.process_window_width -
        process_window_width)) / 2.0;
    }

    hw_params->crop_info.process_window_width = process_window_width;
    hw_params->crop_info.process_window_height = process_window_height;
    hw_params->crop_info.process_window_first_pixel =
      process_window_first_pixel;
    hw_params->crop_info.process_window_first_line =
      process_window_first_line;
  }
  CDBG("%s:%d, pwfp=%d, pwfl=%d, pww=%d, pwh=%d", __func__, __LINE__,
    hw_params->crop_info.process_window_first_pixel,
    hw_params->crop_info.process_window_first_line,
    hw_params->crop_info.process_window_width,
    hw_params->crop_info.process_window_height);
  return 0;
}

float cpp_params_calculate_scalor_adj_ratio(
  cpp_hardware_params_t *hw_params)
{
  float scalor_ratio = 0.0f;
  float scalor_adj_factor = 1.0f;
  float isp_scale_ratio, cpp_scale_ratio;

  if (!hw_params->isp_width_map || !hw_params->isp_height_map ||
    !hw_params->input_info.width || !hw_params->input_info.height) {
    isp_scale_ratio = 1.0f;
  } else {
    float width_ratio, height_ratio;
    width_ratio = (float)hw_params->isp_width_map /
      hw_params->input_info.width;
    height_ratio = (float)hw_params->isp_height_map /
      hw_params->input_info.height;
    if (width_ratio <= height_ratio) {
      isp_scale_ratio = width_ratio;
    } else {
      isp_scale_ratio = height_ratio;
    }
  }
  CDBG_LOW("%s isp_width_map = %d,isp_height_map = %d,input_info.width = %d, \
    input_info.height = %d,isp_scale_ratio = %f", __func__, \
    hw_params->isp_width_map,hw_params->isp_height_map, \
    hw_params->input_info.width,hw_params->input_info.height,isp_scale_ratio);

   if (!hw_params->output_info.width || !hw_params->output_info.height ||
     !hw_params->crop_info.process_window_width ||
     !hw_params->crop_info.process_window_height) {
     cpp_scale_ratio = 1.0f;
   } else {
     float width_ratio, height_ratio;
     width_ratio = (float)hw_params->crop_info.process_window_width /
       hw_params->output_info.width;
     height_ratio = (float)hw_params->crop_info.process_window_height /
       hw_params->output_info.height;
     if (width_ratio <= height_ratio) {
       cpp_scale_ratio = width_ratio;
     } else {
       cpp_scale_ratio = height_ratio;
     }
    }

    CDBG_LOW("%s - process_window_width = %d,process_window_height = %d, \
      output_info.width = %d,output_info.height = %d,cpp_scale_ratio = %f",\
      __func__,hw_params->crop_info.process_window_width , \
      hw_params->crop_info.process_window_height,hw_params->output_info.width, \
      hw_params->output_info.height,cpp_scale_ratio);

  if (!hw_params->asf_info.sharp_min_ds_factor ||
      !hw_params->asf_info.sharp_max_ds_factor ||
      !hw_params->asf_info.sharp_max_factor) {
    CDBG_LOW("%s:%d min_ds %f max_ds %f max_factor %f\n",
      __func__, __LINE__, hw_params->asf_info.sharp_min_ds_factor,
      hw_params->asf_info.sharp_max_ds_factor,
      hw_params->asf_info.sharp_max_factor);
    return 1;
  }
  CDBG_LOW("%s:%d min_ds %f max_ds %f max_factor %f\n",
    __func__, __LINE__, hw_params->asf_info.sharp_min_ds_factor,
    hw_params->asf_info.sharp_max_ds_factor,
    hw_params->asf_info.sharp_max_factor);

  scalor_ratio = isp_scale_ratio*cpp_scale_ratio;

  CDBG_LOW("%s:%d scale_ratio %f\n", __func__, __LINE__, scalor_ratio);

  if ((scalor_ratio != 0) && (scalor_ratio != 1)) {
    /* downscalor */
    if (scalor_ratio >= hw_params->asf_info.sharp_max_ds_factor) {
      scalor_adj_factor = 0;
    } else if (scalor_ratio >= 1) {
      if (hw_params->asf_info.sharp_max_ds_factor - 1) {
        scalor_adj_factor =
          (hw_params->asf_info.sharp_max_ds_factor - scalor_ratio) /
         (hw_params->asf_info.sharp_max_ds_factor - 1);
      }
    /* upscalor */
    } else if (scalor_ratio >= hw_params->asf_info.sharp_min_ds_factor) {
      if (1 - hw_params->asf_info.sharp_min_ds_factor) {
        scalor_adj_factor =
          ((hw_params->asf_info.sharp_max_factor - 1) /
          (1 - hw_params->asf_info.sharp_min_ds_factor)) *
          (1 - scalor_ratio) + 1;
      }
    } else {
      scalor_adj_factor = hw_params->asf_info.sharp_max_factor;
    }
  }
  CDBG_LOW("%s:%d scalor_adj_factor %f\n", __func__, __LINE__,
    scalor_adj_factor);

  return scalor_adj_factor;
}

/* currently hardcodes some values */
void cpp_params_create_frame_info(cpp_hardware_params_t *hw_params,
  struct cpp_frame_info_t *frame_info)
{
  int i = 0, j, k, chroma_1 = 1, chroma_2 = 2;
  int plane_count;
  struct cpp_plane_info_t *plane_info = frame_info->plane_info;
  struct cpp_asf_info *asf_info = &frame_info->asf_info;
  float scalor_adj_factor = 0.0f;

  /* calculate the crop */
  cpp_params_calculate_crop(hw_params);
  assert(hw_params->output_info.width != 0);
  assert(hw_params->output_info.height != 0);

 /* Hack to equalize the snpashots in ZSL and NON-ZSl mode when flip is not 0 */
  if((hw_params->mirror == 2) && (hw_params->rotation == 1)) {
     hw_params->rotation = 3;
  }
  else if((hw_params->mirror == 2) && (hw_params->rotation == 3)) {
     hw_params->rotation = 1;
  }
  else if((hw_params->mirror == 1) && (hw_params->rotation == 1)) {
     hw_params->rotation = 3;
  }
  else if((hw_params->mirror == 1) && (hw_params->rotation == 3)) {
     hw_params->rotation = 1;
  }

  if(hw_params->input_info.plane_fmt == CPP_PARAM_PLANE_CRCB420)
    plane_count = 3;
  else
    plane_count = 2;

  for(i = 0; i < plane_count; i++) {
    memset(&plane_info[i], 0, sizeof(struct cpp_plane_info_t));
      plane_info[i].rotate = hw_params->rotation;
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
      plane_info[i].scanline = hw_params->input_info.scanline;
      set_default_crop_padding(&plane_info[i]);
      plane_info[i].bf_enable = hw_params->denoise_enable;
      if (hw_params->diagnostic_enable) {
        if (hw_params->ez_tune_wnr_enable)
          plane_info[i].bf_enable = TRUE;
        else
          plane_info[i].bf_enable = FALSE;
      }

      plane_info[i].tnr_enable = hw_params->tnr_enable;
      plane_info[i].dst_stride = hw_params->output_info.stride;
      plane_info[i].maximum_dst_stripe_height =
        PAD_TO_2(hw_params->output_info.scanline);

      if(i > 0) {
          plane_info[i].src_width /= 2;
          if((hw_params->rotation == 0) || (hw_params->rotation == 2)) {
            plane_info[i].dst_width /= 2;
          } else if ((hw_params->rotation == 1) || (hw_params->rotation == 3)) {
            plane_info[i].dst_height /= 2;
          }
          plane_info[i].scanline /= 2;
          plane_info[i].h_scale_initial_phase /= 2;
          if((hw_params->input_info.plane_fmt != CPP_PARAM_PLANE_CBCR422) &&
             (hw_params->input_info.plane_fmt != CPP_PARAM_PLANE_CRCB422)){
            plane_info[i].src_height /= 2;
            if((hw_params->rotation == 0) || (hw_params->rotation == 2)) {
              plane_info[i].dst_height /= 2;
            } else if ((hw_params->rotation == 1) || (hw_params->rotation == 3)) {
              plane_info[i].dst_width /= 2;
            }
            plane_info[i].v_scale_initial_phase /= 2;
            plane_info[i].maximum_dst_stripe_height =
              PAD_TO_2(plane_info[i].maximum_dst_stripe_height / 2);
           } else if ((hw_params->rotation == 1) || (hw_params->rotation == 3)) {
               plane_info[i].h_scale_ratio *=
                   ((double)plane_info[i].src_width / plane_info[i].dst_width);
               plane_info[i].v_scale_ratio *=
                   ((double)plane_info[i].src_height / plane_info[i].dst_height);
           }

           if(hw_params->input_info.plane_fmt == CPP_PARAM_PLANE_CRCB420) {
                plane_info[i].src_stride = PAD_TO_SIZE(
                    hw_params->input_info.stride /2,CAM_PAD_TO_16);
                if(hw_params->output_info.plane_fmt ==
                    CPP_PARAM_PLANE_CRCB420) {
                    plane_info[i].dst_stride = PAD_TO_SIZE(
                        hw_params->output_info.stride /2,CAM_PAD_TO_16);
               }
           }

            if (hw_params->uv_upsample_enable) {
              plane_info[i].src_width /= 2;
              plane_info[i].src_height /= 2;
              plane_info[i].src_stride /= 2;
              plane_info[i].h_scale_initial_phase /= 2;
              plane_info[i].v_scale_initial_phase /= 2;
              plane_info[i].h_scale_ratio /= 2;
              plane_info[i].v_scale_ratio /= 2;
            }

             plane_info[i].postscale_padding = 0;
             plane_info[i].source_address[0] = plane_info[i-1].source_address[0] +
               plane_info[i-1].src_stride * plane_info[i-1].scanline +
               hw_params->input_info.plane_offsets[i-1];

             plane_info[i].destination_address[0] = plane_info[i-1].destination_address[0] +
               plane_info[i-1].dst_stride *
               plane_info[i-1].maximum_dst_stripe_height +
               hw_params->output_info.plane_offsets[i-1];

             if((hw_params->output_info.plane_fmt == CPP_PARAM_PLANE_CRCB420) &&
               (hw_params->input_info.plane_fmt != CPP_PARAM_PLANE_CRCB420)){
                 plane_info[i].dst_stride = (plane_info[i].dst_stride + 31)&~31;
                 plane_info[i].dst_stride /= 2;
                 plane_info[i].destination_address[1] =
                   plane_info[0].maximum_dst_stripe_height*
                   plane_info[i-1].dst_stride * i +
                   hw_params->output_info.plane_offsets[i-1];
                  plane_info[i].destination_address[0] =
                    (plane_info[i].dst_stride) *
                    plane_info[i].maximum_dst_stripe_height;
                  if(hw_params->output_info.plane_offsets[i])
                    plane_info[i].destination_address[0] +=
                        hw_params->output_info.plane_offsets[i];
                  else
                    plane_info[i].destination_address[0] +=
                        plane_info[i].destination_address[1];
             }
             else
               plane_info[i].destination_address[1] =
                 plane_info[i].destination_address[0];
      }

      if (plane_info[i].h_scale_ratio < 1)
        plane_info[i].h_scale_initial_phase +=
          (plane_info[i].h_scale_ratio - 1) / 2;
      if (plane_info[i].v_scale_ratio < 1)
        plane_info[i].v_scale_initial_phase +=
          (plane_info[i].v_scale_ratio - 1) / 2;
  }

  plane_info[0].input_plane_fmt = PLANE_Y;
  plane_info[0].input_bytes_per_pixel = 1;
  plane_info[0].output_plane_fmt = PLANE_Y;
  plane_info[0].output_bytes_per_pixel = 1;
  plane_info[0].temporal_bytes_per_pixel = 1;

  if(hw_params->input_info.plane_fmt == CPP_PARAM_PLANE_CRCB420) {
     plane_info[1].input_plane_fmt = PLANE_CB;
     plane_info[2].input_plane_fmt = PLANE_CR;
     plane_info[1].input_bytes_per_pixel = 1;
     plane_info[2].input_bytes_per_pixel = 1;
     plane_info[1].temporal_bytes_per_pixel = 1;
     plane_info[2].temporal_bytes_per_pixel = 1;
  }
  else {
     plane_info[1].input_plane_fmt = PLANE_CBCR;
     plane_info[1].input_bytes_per_pixel = 2;
     plane_info[1].temporal_bytes_per_pixel = 2;
  }
  if(hw_params->output_info.plane_fmt == CPP_PARAM_PLANE_CRCB420){
     plane_info[1].output_plane_fmt = PLANE_CB;
     plane_info[2].output_plane_fmt = PLANE_CR;
     plane_info[1].output_bytes_per_pixel = 1;
     plane_info[2].output_bytes_per_pixel = 1;
  }
  else {
     plane_info[1].output_plane_fmt = PLANE_CBCR;
     plane_info[1].output_bytes_per_pixel = 2;
  }

  if ((hw_params->input_info.plane_fmt == CPP_PARAM_PLANE_CRCB) ||
    (hw_params->input_info.plane_fmt == CPP_PARAM_PLANE_CRCB422) ||
    (hw_params->input_info.plane_fmt == CPP_PARAM_PLANE_CRCB420)) {
    /* Swap the chroma planes */
    chroma_1 = 2;
    chroma_2 = 1;
  }

  for (k = 0; k < CPP_DENOISE_NUM_PROFILES; k++) {
    frame_info->noise_profile[0][k] =
      hw_params->denoise_info[0][k].noise_profile;
    frame_info->noise_profile[chroma_1][k] =
      hw_params->denoise_info[1][k].noise_profile;
    frame_info->noise_profile[chroma_2][k] =
      hw_params->denoise_info[2][k].noise_profile;
    for (j = 0; j < CPP_DENOISE_NUM_PLANES; j++) {
      frame_info->weight[j][k] = hw_params->denoise_info[j][k].weight;
      frame_info->denoise_ratio[j][k] =
        hw_params->denoise_info[j][k].denoise_ratio;
      frame_info->edge_softness[j][k] =
        hw_params->denoise_info[j][k].edge_softness;
    }
  }
  frame_info->noise_profile[chroma_1][4] =
    hw_params->denoise_info[1][4].noise_profile;
  frame_info->noise_profile[chroma_2][4] =
    hw_params->denoise_info[2][4].noise_profile;
  frame_info->weight[1][4] = hw_params->denoise_info[1][4].weight;
  frame_info->weight[2][4] = hw_params->denoise_info[2][4].weight;
  frame_info->denoise_ratio[1][4] =
    hw_params->denoise_info[1][4].denoise_ratio;
  frame_info->denoise_ratio[2][4] =
    hw_params->denoise_info[2][4].denoise_ratio;

  frame_info->asf_mode = hw_params->asf_mode;

  if (hw_params->diagnostic_enable) {
    if (hw_params->ez_tune_asf_enable) {
        if (hw_params->sharpness_level > 0.0f)
          frame_info->asf_mode = ASF_DUAL_FILTER;
    }
    else
      frame_info->asf_mode = ASF_OFF;
  }

  frame_info->sharpness_ratio = hw_params->sharpness_level;
  asf_info->sp = hw_params->asf_info.sp;
  asf_info->neg_abs_y1 = hw_params->asf_info.neg_abs_y1;
  asf_info->dyna_clamp_en = hw_params->asf_info.dyna_clamp_en;
  asf_info->sp_eff_en = hw_params->asf_info.sp_eff_en;
  asf_info->clamp_h_ul = hw_params->asf_info.clamp_h_ul;
  asf_info->clamp_h_ll = hw_params->asf_info.clamp_h_ll;
  asf_info->clamp_v_ul = hw_params->asf_info.clamp_v_ul;
  asf_info->clamp_v_ll = hw_params->asf_info.clamp_v_ll;
  asf_info->clamp_offset_max = hw_params->asf_info.clamp_offset_max;
  asf_info->clamp_offset_min = hw_params->asf_info.clamp_offset_min;
  asf_info->clamp_scale_max = hw_params->asf_info.clamp_scale_max;
  asf_info->clamp_scale_min = hw_params->asf_info.clamp_scale_min;
  asf_info->nz_flag = hw_params->asf_info.nz_flag;
  asf_info->nz_flag_f2 = hw_params->asf_info.nz_flag_f2;
  asf_info->nz_flag_f3_f5 = hw_params->asf_info.nz_flag_f3_f5;
  asf_info->checksum_en = hw_params->asf_info.checksum_enable;
  for (i = 0; i < CPP_PARAM_ASF_F_KERNEL_MAX; i++) {
    /* Update F1 kernel */
    asf_info->sobel_h_coeff[i] = hw_params->asf_info.sobel_h_coeff[i];
    /* Update F2 kernel */
    asf_info->sobel_v_coeff[i] = hw_params->asf_info.sobel_v_coeff[i];
    /* Update F3 kernel */
    asf_info->hpf_h_coeff[i] = hw_params->asf_info.hpf_h_coeff[i];
    /* Update F4 kernel */
    asf_info->hpf_v_coeff[i] = hw_params->asf_info.hpf_v_coeff[i];
    /* Update F5 kernel */
    asf_info->lpf_coeff[i] = hw_params->asf_info.lpf_coeff[i];
  }

  scalor_adj_factor = cpp_params_calculate_scalor_adj_ratio(hw_params);

  for (i = 0; i < CPP_PARAM_ASF_LUT_MAX; i++) {
    asf_info->lut1[i] = hw_params->asf_info.lut1[i] * scalor_adj_factor;
    asf_info->lut2[i] = hw_params->asf_info.lut2[i] * scalor_adj_factor;
  }
  for (i = 0; i < CPP_PARAM_ASF_LUT3_MAX; i++) {
    asf_info->lut3[i] = hw_params->asf_info.lut3[i];
  }

  frame_info->num_planes = plane_count;

  switch(hw_params->input_info.plane_fmt) {
  case CPP_PARAM_PLANE_CRCB:
  case CPP_PARAM_PLANE_CRCB422:
    frame_info->in_plane_fmt = PLANE_CRCB;
    break;
  case CPP_PARAM_PLANE_CBCR:
  case CPP_PARAM_PLANE_CBCR422:
    frame_info->in_plane_fmt = PLANE_CBCR;
    break;
  default:
    CDBG_ERROR("%s:%d, bad input format", __func__, __LINE__);
    frame_info->in_plane_fmt = PLANE_CBCR;
    break;
  }

  switch(hw_params->output_info.plane_fmt) {
  case CPP_PARAM_PLANE_CRCB:
  case CPP_PARAM_PLANE_CRCB422:
    frame_info->out_plane_fmt = PLANE_CRCB;
    break;
  case CPP_PARAM_PLANE_CBCR:
  case CPP_PARAM_PLANE_CBCR422:
    frame_info->out_plane_fmt = PLANE_CBCR;
    break;
  default:
    CDBG_ERROR("%s:%d, bad output format", __func__, __LINE__);
    frame_info->out_plane_fmt = PLANE_CBCR;
    break;
  }
  if (CPP_PARAM_PLANE_CRCB420 == hw_params->input_info.plane_fmt) {
    frame_info->in_plane_fmt = PLANE_Y;
  }
  if (CPP_PARAM_PLANE_CRCB420 == hw_params->output_info.plane_fmt) {
    frame_info->out_plane_fmt = PLANE_Y;
  }

  CDBG_LOW("%s:%d, punit: input_fmt=%d output_fmt=%d ", __func__, __LINE__,
         frame_info->in_plane_fmt,frame_info->out_plane_fmt);

  CDBG("%s:%d] bf_enable:%d,asf_mode=%d for frame_id=%d,identity=%d\n",
    __func__, __LINE__,
    hw_params->denoise_enable,frame_info->asf_mode,
    hw_params->frame_id,hw_params->identity);
  return;
}

boolean cpp_hardware_validate_params(cpp_hardware_params_t *hw_params)
{

  CDBG_LOW("%s:%d, inw=%d, inh=%d, outw=%d, outh=%d", __func__, __LINE__,
    hw_params->input_info.width, hw_params->input_info.height,
    hw_params->output_info.width, hw_params->output_info.height);
  CDBG_LOW("%s:%d, inst=%d, insc=%d, outst=%d, outsc=%d", __func__, __LINE__,
    hw_params->input_info.stride, hw_params->input_info.scanline,
    hw_params->output_info.stride, hw_params->output_info.scanline);

  if (hw_params->input_info.width <= 0 || hw_params->input_info.height <= 0) {
    CDBG_ERROR("%s:%d, invalid input dim", __func__, __LINE__);
    return FALSE;
  }
  /* TODO: add mode sanity checks */
  return TRUE;
}


boolean cpp_hardware_rotation_swap(cpp_hardware_params_t *hw_params)
{
  int32_t   swap_dim;

  if((hw_params->rotation == 1) || (hw_params->rotation == 3)) {
    swap_dim = hw_params->output_info.width;
    hw_params->output_info.width = hw_params->output_info.height;
    hw_params->output_info.height = swap_dim;
  }

  return TRUE;
}

void cpp_hw_params_copy_asf_diag_params(struct cpp_frame_info_t *frame_info,
  asfsharpness7x7_t *diag_cpyto_params)
{
  int i;
  struct cpp_asf_info *diag_cpyfrm_params = &frame_info->asf_info;

  /* Copy the members from hw_params to diagnostics structure */
  diag_cpyto_params->smoothpercent =
    (int32_t)Round(diag_cpyfrm_params->sp * (1 << 4));
  diag_cpyto_params->neg_abs_y1 = diag_cpyfrm_params->neg_abs_y1;
  diag_cpyto_params->dyna_clamp_en = diag_cpyfrm_params->dyna_clamp_en;
  diag_cpyto_params->sp_eff_en = diag_cpyfrm_params->sp_eff_en;
  diag_cpyto_params->clamp_hh = diag_cpyfrm_params->clamp_h_ul;
  diag_cpyto_params->clamp_hl = diag_cpyfrm_params->clamp_h_ll;
  diag_cpyto_params->clamp_vh = diag_cpyfrm_params->clamp_v_ul;
  diag_cpyto_params->clamp_vl = diag_cpyfrm_params->clamp_v_ll;
  diag_cpyto_params->clamp_scale_max =
    (int32_t)Round(diag_cpyfrm_params->clamp_scale_max * (1 << 4));
  diag_cpyto_params->clamp_scale_min =
    (int32_t)Round(diag_cpyfrm_params->clamp_scale_min * (1 << 4));
  diag_cpyto_params->clamp_offset_max = diag_cpyfrm_params->clamp_offset_max;
  diag_cpyto_params->clamp_offset_min = diag_cpyfrm_params->clamp_offset_min;
  diag_cpyto_params->nz_flag = diag_cpyfrm_params->nz_flag;

  for ( i = 0; i < CPP_PARAM_ASF_F_KERNEL_MAX; i++) {
    diag_cpyto_params->sobel_h_coeff[i] =
      (int32_t)(Round(diag_cpyfrm_params->sobel_h_coeff[i]*(1<<10)));
    diag_cpyto_params->sobel_v_coeff[i] =
      (int32_t)(Round(diag_cpyfrm_params->sobel_v_coeff[i]*(1<<10)));
    diag_cpyto_params->hpf_h_coeff[i] =
      (int32_t)(Round(diag_cpyfrm_params->hpf_h_coeff[i]*(1<<10)));
    diag_cpyto_params->hpf_v_coeff[i] =
      (int32_t)(Round(diag_cpyfrm_params->hpf_v_coeff[i]*(1<<10)));
    diag_cpyto_params->lpf_coeff[i] =
      (int32_t)(Round(diag_cpyfrm_params->lpf_coeff[i]*(1<<10)));
  }

  for (i = 0; i < CPP_PARAM_ASF_LUT_MAX; i++) {
    diag_cpyto_params->lut1[i] =
      (int32_t)(Round((diag_cpyfrm_params->lut1[i])*(1<<5)));
    diag_cpyto_params->lut2[i] =
      (int32_t)(Round((diag_cpyfrm_params->lut2[i])*(1<<5)));
  }

  for (i = 0; i < CPP_PARAM_ASF_LUT3_MAX; i++) {
    diag_cpyto_params->lut3[i] =
      (int32_t)(Round(diag_cpyfrm_params->lut3[i]*(1<<6)));
  }

  return;
}

void cpp_hw_params_copy_wnr_diag_params(struct cpp_frame_info_t *frame_info,
  wavelet_t *diag_cpyto_params)
{
  int i;
  for (i = 0; i < CPP_DENOISE_NUM_PROFILES; i++) {
    /* Plane 1 */
    //Q10 and Q8 format for bilateral_scale is calculated in cpp_prepare_bf_info()
    diag_cpyto_params->bilateral_scalecore0[i] =
      (int32_t)frame_info->bf_info[0].bilateral_scale[i];
    diag_cpyto_params->weightcore0[i] =
      (int32_t)(frame_info->bf_info[0].weight[i] * (1 << 4));
    diag_cpyto_params->noise_thresholdcore0[i] =
      (int32_t)(frame_info->bf_info[0].noise_threshold[i] * (1 << 4));

    /* Plane 2 */
    diag_cpyto_params->bilateral_scalecore1[i] =
      (int32_t)frame_info->bf_info[1].bilateral_scale[i];
    diag_cpyto_params->weightcore1[i] =
      (int32_t)(frame_info->bf_info[1].weight[i] * (1 << 4));
    diag_cpyto_params->noise_thresholdcore1[i] =
      (int32_t)(frame_info->bf_info[1].noise_threshold[i] * (1 << 4));

    /* Plane 3 */
    diag_cpyto_params->bilateral_scalecore2[i] =
      (int32_t)frame_info->bf_info[2].bilateral_scale[i];
    diag_cpyto_params->weightcore2[i] =
      (int32_t)(frame_info->bf_info[2].weight[i] * (1 << 4));
    diag_cpyto_params->noise_thresholdcore2[i] =
      (int32_t)(frame_info->bf_info[2].noise_threshold[i] * (1 << 4));
  }
  return;
}
