/*========================================================================


*//** @file jpege_engine_sw_scale_down.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2010-2011, 2014 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/still/jpeg2/v2/latest/src/jpege_engine_sw_scale_down.c#2 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
10/16/14   vgaur   Added support for multi threaded fastCV
12/05/13   staceyw Fixed initialization during multiple sessions encoding.
04/30/13   staceyw Added clamping for M/N downscale.
04/02/13   staceyw Added rounding for M/N downscale output generation.
08/29/12   staceyw Added support for M/N algorithm up to scale ratio of 1/16,
                   Extended division look-up table to handle the division range from 1 to 1/16.
03/28/12  csriniva Fixed Compiler warnings on QNX
03/01/12  csriniva Added support for YUY2 and UYVY inputs
04/06/10   staceyw Added crop and downscale configure and processing.
                   Both horizontal and vertical downscale uses M/N algorithm
                   which is based on a M/N counter.
02/22/09   staceyw Created file

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include <stdlib.h>
#include "fastcv.h"
#include "jpege_engine_sw_scale.h"
#include "jpeglog.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
#define DIGIT_Q10 10
#define DIGIT_Q10_ROUND (1 << (DIGIT_Q10-1))
#define CLAMP255(x) ( ((x) & (0xFFFFFF00)) ? (((~(x)) >> 31) | (0xFF)) : (x))

/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* functions related with in-line down-scale */
int jpege_engine_sw_downscale_configure_filter_fastCV(
    jpege_scale_t *p_scale_info);
int jpege_engine_sw_downscale_configure_filter(
    jpege_scale_t *p_scale_info);
void jpege_engine_sw_downscale_mcu_lines(
    jpege_scale_t *p_scale_info,
    uint32_t       rotated_mcu_height);
void jpege_engine_sw_downscale_mcu_lines_packed(
    jpege_scale_t *p_scale_info,
    uint32_t       rotated_mcu_height);
void jpege_engine_sw_downscale_rotated_mcu_lines(
    jpege_scale_t *p_scale_info,
    uint32_t       rotated_mcu_height);
void jpege_engine_sw_downscale_mcu_lines_fastCV(
    jpege_scale_t *p_scale_info,
    uint32_t        rotated_mcu_height);
// fetch non-rotated/rotated luma/chroma line to internal buffer
static void jpege_engine_sw_downscale_fetch_rotated_luma_to_line_buffer(
    jpege_scale_t *p_scale_info);
static void jpege_engine_sw_downscale_fetch_rotated_chroma_to_line_buffer(
    jpege_scale_t *p_scale_info);
// vertical/horizontal downscale luma/chroma line
static void jpege_engine_sw_downscale_horizontal_luma_line(
    uint8_t  *p_input_line,
    uint8_t  *p_output_line,
    uint32_t  crop_width,
    uint32_t  output_width);
static void jpege_engine_sw_downscale_vertical_luma_line(
    uint8_t *p_input_line,
    uint8_t **pp_output_line,
    jpege_scale_t *p_scale_info);
static void jpege_engine_sw_downscale_horizontal_chroma_line(
    uint8_t  *p_input_line,
    uint8_t  *p_output_line,
    uint32_t  crop_chroma_width,
    uint32_t  output_chroma_width);
static void jpege_engine_sw_downscale_vertical_chroma_line(
    uint8_t *p_input_line,
    uint8_t **pp_output_line,
    jpege_scale_t *p_scale_info);
// copy rotated luma/chroma lines
extern void jpege_engine_sw_copy_rotated_luma_lines(
    uint8_t  *p_src,
    uint8_t  *p_output,
    uint32_t  input_width,
    int32_t   fetch_increment,
    uint32_t  pixel_extend_left,
    uint32_t  pixel_extend_right);
extern void jpege_engine_sw_copy_rotated_chroma_lines(
    uint8_t  *p_src,
    uint8_t  *p_output,
    uint32_t  input_width,
    int32_t   fetch_increment,
    uint32_t  pixel_extend_left,
    uint32_t  pixel_extend_right);
extern void jpege_engine_sw_copy_rotated_chroma_lines_packed(
    uint8_t  *p_src,
    uint8_t  *p_output,
    uint32_t  input_width,
    int32_t   fetch_increment,
    uint32_t  pixel_extend_left,
    uint32_t  pixel_extend_right);

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
**                          Function Definitions
** ======================================================================= */

/*===========================================================================
FUNCTION        jpege_engine_sw_downscale_mcu_lines

DESCRIPTION     downscale with MCU Lines using M/N algorithm:
                1) fetch luma/chroma lines
                2) horizontal downscale luma/chroma lines
                3) vertical downscale luma/chroma lines
                4) output to scaled mcu lines buffer

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
void jpege_engine_sw_downscale_mcu_lines(
    jpege_scale_t  *p_scale_info,
    uint32_t        rotated_mcu_height)
{
    uint32_t  row_index_init = p_scale_info->vert_luma_row_index;
    uint32_t  row_index_bound = p_scale_info->vert_luma_row_index + rotated_mcu_height;
    uint32_t  chroma_row_index_bound = p_scale_info->vert_chroma_row_index + LUMA_BLOCK_HEIGHT;
    uint32_t  vert_luma_row_index = p_scale_info->vert_luma_row_index;
    uint32_t  vert_chroma_row_index = p_scale_info->vert_chroma_row_index;
    uint32_t  luma_src_increment = p_scale_info->rotated_luma_src_increment;
    uint32_t  chroma_src_increment = (p_scale_info->rotated_chroma_src_increment << 1);

    uint32_t  input_width = p_scale_info->vert_luma_fetch_width;
    uint32_t  input_chroma_width = (p_scale_info->vert_chroma_fetch_width << 1);
    uint32_t  crop_width = p_scale_info->crop_width;
    uint32_t  crop_chroma_width = p_scale_info->crop_chroma_width;
    uint32_t  output_width = p_scale_info->output_width;
    uint32_t  output_chroma_width = p_scale_info->output_chroma_width;
    uint32_t  output_height = p_scale_info->vert_luma_output_height;
    uint32_t  output_chroma_height = p_scale_info->vert_chroma_output_height;

    uint8_t  *p_luma_src = p_scale_info->p_luma_input;
    uint8_t  *p_chroma_src = p_scale_info->p_chroma_input;
    uint8_t  *p_input_line= p_scale_info->p_input_luma_line_buffer->ptr;
    uint8_t  *p_vert_scale_line = p_scale_info->p_vertical_scaled_buffer->ptr;
    uint8_t  *p_scale_line = p_scale_info->p_inline_scaled_buffer->ptr;

    /*------------------------------------------------------------
     M/N Algorithm is based on an M/N counter:
     An M/N counter is a counter that increments by M and
     rolls over at N.
     M : number of horizontal/vertical pixels in the output resolution
     N:  number of horizontal/vertical pixels in the input resolution
    ------------------------------------------------------------*/

    /*------------------------------------------------------------
      Luma downscaling and output one downscaled MCU line
    ------------------------------------------------------------*/
    while ((vert_luma_row_index < row_index_bound) &&
           (vert_luma_row_index < output_height))
    {
        /*------------------------------------------------------------
          For non rotation case,
          fetch one luma line from source to line buffer
        ------------------------------------------------------------*/

       (void)STD_MEMMOVE(p_input_line, p_luma_src, (input_width * sizeof(uint8_t)));

        // update luma input pointer
        p_luma_src += luma_src_increment;

        /*------------------------------------------------------------
          M/N horizontal downscale a luma line
        ------------------------------------------------------------*/
        jpege_engine_sw_downscale_horizontal_luma_line(p_input_line, p_vert_scale_line,
                                                       crop_width, output_width);

        /*------------------------------------------------------------
          M/N vertical dowanscale a luma line
        ------------------------------------------------------------*/
        jpege_engine_sw_downscale_vertical_luma_line(p_vert_scale_line, &p_scale_line, p_scale_info);

        // update luma row index
        vert_luma_row_index = p_scale_info->vert_luma_row_index;
    }

    /*------------------------------------------------------------
      Chroma downscaling and output one downscaled MCU line
    ------------------------------------------------------------*/
    p_input_line= p_scale_info->p_input_chroma_line_buffer->ptr;
    p_scale_line = p_scale_info->p_inline_scaled_buffer->ptr + output_width * rotated_mcu_height;
    p_vert_scale_line = p_scale_info->p_vertical_scaled_buffer->ptr;

    while ((vert_chroma_row_index < chroma_row_index_bound) &&
          (vert_chroma_row_index < output_chroma_height))
    {
        /*------------------------------------------------------------
          For non rotation case,
          it can fetch one chroma line from source to line buffer
        ------------------------------------------------------------*/
        // fetch a line
        (void)STD_MEMMOVE(p_input_line, p_chroma_src, (input_chroma_width * sizeof(uint8_t)));

        // update chroma input pointer
        p_chroma_src += chroma_src_increment;

        /*------------------------------------------------------------
          M/N horizontal downscale one chroma line
        ------------------------------------------------------------*/
        jpege_engine_sw_downscale_horizontal_chroma_line(p_input_line, p_vert_scale_line,
                                                         crop_chroma_width, output_chroma_width);
        /*------------------------------------------------------------
          M/N vertical downscale one chroma line
        ------------------------------------------------------------*/
        jpege_engine_sw_downscale_vertical_chroma_line(p_vert_scale_line, &p_scale_line, p_scale_info);
        //update chroma row index;
        vert_chroma_row_index = p_scale_info->vert_chroma_row_index;
    }

    // update luma input pointer inside each encoding session
    p_scale_info->p_luma_input = p_luma_src;
    // update chroma input pointer inside each encoding session
    p_scale_info->p_chroma_input = p_chroma_src;
    /*------------------------------------------------------------
      Compute feteched luma blocks
    ------------------------------------------------------------*/
    p_scale_info->fetched_luma_blocks = (((vert_luma_row_index - row_index_init) * output_width
                                         + LUMA_BLOCK_SIZE - 1) >> LUMA_BLOCK_SIZE_DIGIT);

}

/*===========================================================================
FUNCTION        jpege_engine_sw_downscale_mcu_lines_packed

DESCRIPTION     downscale with MCU Lines using M/N algorithm for YUY2 and UYVY formats:
                1) fetch luma/chroma lines
                2) horizontal downscale luma/chroma lines
                3) vertical downscale luma/chroma lines
                4) output to scaled mcu lines buffer

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
void jpege_engine_sw_downscale_mcu_lines_packed(
    jpege_scale_t  *p_scale_info,
    uint32_t        rotated_mcu_height)
{
    jpege_scale_down_t  *p_scale_down = &(p_scale_info->scale.down);
    uint32_t  row_index_init = p_scale_down->vert_luma_row_index;
    uint32_t  row_index_bound = p_scale_down->vert_luma_row_index + rotated_mcu_height;
    uint32_t  chroma_row_index_bound = p_scale_down->vert_chroma_row_index + LUMA_BLOCK_HEIGHT;
    uint32_t  vert_luma_row_index = p_scale_down->vert_luma_row_index;
    uint32_t  vert_chroma_row_index = p_scale_down->vert_chroma_row_index;
    uint32_t  luma_src_increment = p_scale_info->rotated_luma_src_increment;
    uint32_t  chroma_src_increment = (p_scale_info->rotated_chroma_src_increment << 1);

    uint32_t  input_width = p_scale_info->vert_luma_fetch_width;
    uint32_t  input_chroma_width = (p_scale_info->vert_chroma_fetch_width << 1);
    uint32_t  crop_width = p_scale_info->crop_width;
    uint32_t  crop_chroma_width = p_scale_info->crop_chroma_width;
    uint32_t  output_width = p_scale_info->output_width;
    uint32_t  output_chroma_width = p_scale_info->output_chroma_width;
    uint32_t  output_height = p_scale_info->output_height;
    uint32_t  output_chroma_height = p_scale_info->output_chroma_height;

    uint8_t  *p_luma_src = p_scale_info->p_luma_input;
    uint8_t  *p_chroma_src = p_scale_info->p_chroma_input;
    uint8_t  *p_input_line= p_scale_info->p_input_luma_line_buffer->ptr;
    uint8_t  *p_vert_scale_line = p_scale_info->p_vertical_scaled_buffer->ptr;
    uint8_t  *p_scale_line = p_scale_info->p_inline_scaled_buffer->ptr;
    uint32_t  m;

    /*------------------------------------------------------------
     M/N Algorithm is based on an M/N counter:
     An M/N counter is a counter that increments by M and
     rolls over at N.
     M : number of horizontal/vertical pixels in the output resolution
     N:  number of horizontal/vertical pixels in the input resolution
    ------------------------------------------------------------*/
    /*------------------------------------------------------------
      Luma downscaling and output one downscaled MCU line
    ------------------------------------------------------------*/
    while ((vert_luma_row_index < row_index_bound) &&
           (vert_luma_row_index < output_height))
    {
        /*------------------------------------------------------------
          For non rotation case,
          fetch one luma line from source to line buffer
        ------------------------------------------------------------*/
        for( m = 0; m < input_width * 2; m+= 2)
        {
            *(p_input_line + m/2) = *(p_luma_src + m);
        }

        // update luma input pointer
        p_luma_src += luma_src_increment;

        /*------------------------------------------------------------
          M/N horizontal downscale a luma line
        ------------------------------------------------------------*/
        jpege_engine_sw_downscale_horizontal_luma_line(p_input_line, p_vert_scale_line,
                                                       crop_width, output_width);

        /*------------------------------------------------------------
          M/N vertical dowanscale a luma line
        ------------------------------------------------------------*/
        jpege_engine_sw_downscale_vertical_luma_line(p_vert_scale_line, &p_scale_line, p_scale_info);

        vert_luma_row_index = p_scale_down->vert_luma_row_index;
    }

    /*------------------------------------------------------------
      Chroma downscaling and output one downscaled MCU line
    ------------------------------------------------------------*/
    p_input_line= p_scale_info->p_input_chroma_line_buffer->ptr;
    p_scale_line = p_scale_info->p_inline_scaled_buffer->ptr + output_width * rotated_mcu_height;
    p_vert_scale_line = p_scale_info->p_vertical_scaled_buffer->ptr;
    while ((vert_chroma_row_index < chroma_row_index_bound) &&
           (vert_chroma_row_index < output_chroma_height))
    {
        /*------------------------------------------------------------
          For non rotation case,
          it can fetch one chroma line from source to line buffer
        ------------------------------------------------------------*/
        // fetch a line
        for( m = 0; m < input_chroma_width * 2; m+= 2)
        {
            *(p_input_line + m/2) = *(p_chroma_src + m);
        }

        // update chroma input pointer
        p_chroma_src += chroma_src_increment;

        /*------------------------------------------------------------
          M/N horizontal downscale one chroma line
        ------------------------------------------------------------*/
        jpege_engine_sw_downscale_horizontal_chroma_line(p_input_line, p_vert_scale_line,
                                                         crop_chroma_width, output_chroma_width);

        /*------------------------------------------------------------
          M/N vertical downscale one chroma line
        ------------------------------------------------------------*/
        jpege_engine_sw_downscale_vertical_chroma_line(p_vert_scale_line, &p_scale_line, p_scale_info);
        vert_chroma_row_index = p_scale_down->vert_chroma_row_index;
    }

    // update luma input pointer inside each encoding session
    p_scale_info->p_luma_input = p_luma_src;
    // update chroma input pointer inside each encoding session
    p_scale_info->p_chroma_input = p_chroma_src;
    /*------------------------------------------------------------
      Compute feteched luma blocks
    ------------------------------------------------------------*/
    p_scale_info->fetched_luma_blocks = (((vert_luma_row_index - row_index_init) * output_width
                                         + LUMA_BLOCK_SIZE - 1) >> LUMA_BLOCK_SIZE_DIGIT);

}

/*===========================================================================
FUNCTION        jpege_engine_sw_downscale_horizontal_luma_line

DESCRIPTION     Downscale horizontally Luma line using M/N algorithm

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_downscale_horizontal_luma_line(
    uint8_t  *p_input_line,
    uint8_t  *p_output_line,
    uint32_t  crop_width,
    uint32_t  output_width)
{
    uint32_t step, count;
    uint16_t hori_luma_accumulator;
    uint32_t curr_index = crop_width;
    static const uint16_t div_q10[17]={1024, 1024, 512, 341, 256, 205, 171, 146, 128, 114, 102, 93, 85, 79, 73, 68, 64};

    // Initialization
    step = 0;
    count = 0;
    hori_luma_accumulator = 0;

    /*------------------------------------------------------------
     Perform M/N downscale on luma horizontally
    ------------------------------------------------------------*/
    while (curr_index --)
    {
        // accumulation
        hori_luma_accumulator += *(p_input_line ++);
        // update step
        step ++;
        // update count
        count += output_width;

        if (count >= crop_width)
        {
            // average of N/M  values of the input
            *(p_output_line ++) = (uint8_t)(CLAMP255((hori_luma_accumulator * div_q10[step] + DIGIT_Q10_ROUND)
                                             >> DIGIT_Q10));
            // roll over and update count
            count -= crop_width;
            // reset accumulator
            hori_luma_accumulator = 0;
            // reset step
            step = 0;
        }
    } //end for loop
} // end of jpege_engine_sw_downscale_horizontal_luma_line

/*===========================================================================
FUNCTION        jpege_engine_sw_downscale_vertical_luma_line

DESCRIPTION     M/N downscale vertically Luma line

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_downscale_vertical_luma_line(
    uint8_t *p_input_line,
    uint8_t **pp_output_line,
    jpege_scale_t *p_scale_info)
{
    uint32_t vert_luma_step;
    uint32_t output_width = p_scale_info->output_width;
    uint32_t curr_index = output_width;
    uint32_t output_height = p_scale_info->output_height;
    uint32_t crop_height = p_scale_info->crop_height;
    uint8_t  *p_output_line = *pp_output_line;
    jpege_scale_down_t  *p_scale_down = &(p_scale_info->scale.down);
    uint16_t *p_vert_luma_accum = p_scale_down->p_vert_luma_accum;
    static const uint16_t div_q10[17]={1024, 1024, 512, 341, 256, 205, 171, 146, 128, 114, 102, 93, 85, 79, 73, 68, 64};

    /*------------------------------------------------------------
     Perform M/N downscale vertically
    ------------------------------------------------------------*/
    // accumulation
    while (curr_index --)
    {
        *(p_vert_luma_accum ++) += *(p_input_line ++);
    }
    // update step
    p_scale_down->vert_luma_step ++;

    // update vertical luma count of M/N algorithm
    p_scale_down->vert_luma_count += output_height;

    // compare count and number of input vertical pixels
    if (p_scale_down->vert_luma_count >= crop_height)
    {
        p_vert_luma_accum = p_scale_down->p_vert_luma_accum;
        vert_luma_step = p_scale_down->vert_luma_step;
        curr_index = output_width;
        while (curr_index --)
        {
            // average of N/M  values of the input
            uint32_t sum = (((*p_vert_luma_accum) * div_q10[vert_luma_step] + DIGIT_Q10_ROUND)
                                >> DIGIT_Q10);
            *(p_output_line ++) = (uint8_t)(CLAMP255(sum));
        p_vert_luma_accum ++;
        }
        // reset accumulator
        (void)STD_MEMSET(p_scale_down->p_vert_luma_accum, 0, (output_width * sizeof(uint16_t)));
        // roll over and update count
        p_scale_down->vert_luma_count -= crop_height;
        // update index
        p_scale_info->vert_luma_row_index ++;
        // reset step
        p_scale_down->vert_luma_step = 0;
        // update output line pointer
        *pp_output_line = p_output_line;
    }
} // end of jpege_engine_sw_downscale_vertical_luma_line

/*===========================================================================
FUNCTION        jpege_engine_sw_downscale_horizontal_chroma_line

DESCRIPTION     M/N downscale horizontally Chroma line

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_downscale_horizontal_chroma_line(
    uint8_t  *p_input_line,
    uint8_t  *p_output_line,
    uint32_t  crop_chroma_width,
    uint32_t  output_chroma_width)
{
    uint32_t step, count;
    uint16_t horiCbAccumulator, horiCrAccumulator;
    uint32_t curr_index = crop_chroma_width;
    static const uint16_t div_q10[17]={1024, 1024, 512, 341, 256, 205, 171, 146, 128, 114, 102, 93, 85, 79, 73, 68, 64};

    // Initialization
    step = 0;
    count = 0;
    horiCbAccumulator = 0;
    horiCrAccumulator = 0;
    /*------------------------------------------------------------
     Perform M/N downscale on chroma horizontally
    ------------------------------------------------------------*/
    while (curr_index --)
    {
        // accumulation
        horiCbAccumulator += *(p_input_line ++);
        horiCrAccumulator += *(p_input_line ++);
        // update step
        step ++;
        // update count
        count += output_chroma_width;

        if (count >= crop_chroma_width)
        {
            // average of N/M  values of the input
            *(p_output_line ++) = (uint8_t)(CLAMP255((horiCbAccumulator * div_q10[step] + DIGIT_Q10_ROUND)
                                             >> DIGIT_Q10));

            *(p_output_line ++) = (uint8_t)(CLAMP255((horiCrAccumulator * div_q10[step] + DIGIT_Q10_ROUND)
                                             >> DIGIT_Q10));

            // roll over and update count
            count -= crop_chroma_width;
            // reset accumulator
            horiCbAccumulator = 0;
            horiCrAccumulator = 0;
            // reset step
            step = 0;
        }
    } //end for loop
}// end of jpege_engine_sw_downscale_horizontal_chroma_line

/*===========================================================================
FUNCTION        jpege_engine_sw_downscale_vertical_chroma_line

DESCRIPTION     M/N downscale vertically Chroma line

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_downscale_vertical_chroma_line(
    uint8_t *p_input_line,
    uint8_t **pp_output_line,
    jpege_scale_t *p_scale_info)
{
    uint32_t curr_index = p_scale_info->output_chroma_width;
    uint32_t output_chroma_width = (p_scale_info->output_chroma_width << 1);
    uint32_t output_chroma_height = p_scale_info->output_chroma_height;
    uint32_t crop_chroma_height = p_scale_info->crop_chroma_height;
    uint32_t vert_chroma_step;
    uint8_t  *p_output_line = *pp_output_line;
    jpege_scale_down_t  *p_scale_down = &(p_scale_info->scale.down);
    uint16_t *p_vert_chroma_accum = p_scale_down->p_vert_chroma_accum;
    static const uint16_t div_q10[17]={1024, 1024, 512, 341, 256, 205, 171, 146, 128, 114, 102, 93, 85, 79, 73, 68, 64};

    // accumulation
    while (curr_index --)
    {
        *(p_vert_chroma_accum ++) += *(p_input_line ++);
        *(p_vert_chroma_accum ++) += *(p_input_line ++);
    }
    //update step
    p_scale_down->vert_chroma_step ++;

    // update vertical chroma count of M/N algorithm
    p_scale_down->vert_chroma_count += output_chroma_height;

    /*------------------------------------------------------------
     Perform M/N downscale on chroma horizontally
    ------------------------------------------------------------*/
    // compare count and number of input vertical pixels
    if (p_scale_down->vert_chroma_count >= crop_chroma_height)
    {
        p_vert_chroma_accum = p_scale_down->p_vert_chroma_accum;
        vert_chroma_step = p_scale_down->vert_chroma_step;
        curr_index = p_scale_info->output_chroma_width;
        while (curr_index --)
        {
             uint32_t sum = (((*(p_vert_chroma_accum)) * div_q10[vert_chroma_step] + DIGIT_Q10_ROUND)
                                 >> DIGIT_Q10);
             *(p_output_line ++) = (uint8_t)(CLAMP255(sum));
             p_vert_chroma_accum ++;

             sum = (((*(p_vert_chroma_accum)) * div_q10[vert_chroma_step] + DIGIT_Q10_ROUND)
                                >> DIGIT_Q10);
             *(p_output_line ++) = (uint8_t)(CLAMP255(sum));
             p_vert_chroma_accum ++;
        }
        // reset accumulator
        (void)STD_MEMSET(p_scale_down->p_vert_chroma_accum, 0, (output_chroma_width * sizeof(uint16_t)));
        // roll over count
        p_scale_down->vert_chroma_count -= crop_chroma_height;
        // update index
        p_scale_info->vert_chroma_row_index ++;
        // reset step
        p_scale_down->vert_chroma_step = 0;
        // update output line pointer
        *pp_output_line = p_output_line;
    }
} // end of jpege_engine_sw_downscale_vertical_chroma_line

/*===========================================================================
FUNCTION        jpege_engine_sw_downscale_configure_filter_fastCV

DESCRIPTION     Configures the Luma and Chroma input pointers for
                downscaling and calculates the heights of input
                segments required for generating scaled output.

DEPENDENCIES    None

RETURN VALUE    INT

SIDE EFFECTS    None
===========================================================================*/
int jpege_engine_sw_downscale_configure_filter_fastCV(
    jpege_scale_t *p_scale_info)
{
    int        rc;
    uint32_t   buffer_size;
    uint32_t   vert_row_index;
    int        lumadiff;

    /*------------------------------------------------------------
     M/N Algorithm:  downscale and output MCU lines
     1. init vertical count/row index/step
     2. init vertical accumulator
     3. allocate Luma input line buffer
     4. allocate Chroma input line buffer
     5. init function pointers for fetch
    ------------------------------------------------------------*/
    /*------------------------------------------------------------
      Vertical M/N init
    ------------------------------------------------------------*/
    p_scale_info->scale.down.vert_luma_count = 0;
    p_scale_info->scale.down.vert_chroma_count = 0;
    // Vertical row index is initialized, not always from start
    p_scale_info->scale.down.vert_luma_step = 0;
    p_scale_info->scale.down.vert_chroma_step = 0;
    p_scale_info->scale.down.vert_luma_row_index = p_scale_info->vert_luma_row_index;
    p_scale_info->scale.down.vert_chroma_row_index = p_scale_info->vert_chroma_row_index;
    /*------------------------------------------------------------
      At each encoding session
      1. Need to update luma and chroma input pointer for fetching
      2. Need to accumulate luma and chroma count
     ------------------------------------------------------------*/
    // Luma
    vert_row_index = 0;
    while (vert_row_index < p_scale_info->scale.down.vert_luma_row_index)
    {
        // update luma input pointer
        p_scale_info->p_luma_input += p_scale_info->rotated_luma_src_increment;

        // update vertical luma count of M/N algorithm
        p_scale_info->scale.down.vert_luma_count += p_scale_info->output_height;

        // compare count and number of input vertical pixels
        if (p_scale_info->scale.down.vert_luma_count >= p_scale_info->crop_height)
        {
            vert_row_index ++;
            p_scale_info->scale.down.vert_luma_count -= p_scale_info->crop_height;
        }
    }

    p_scale_info->scale.down.luma_input_seg_height = 0;
    p_scale_info->scale.down.vert_luma_count = 0;

    while (vert_row_index < p_scale_info->scale.down.vert_luma_row_index + p_scale_info->piece_height)
    {
        p_scale_info->scale.down.vert_luma_count += p_scale_info->output_height;
        p_scale_info->scale.down.luma_input_seg_height ++;
        if (p_scale_info->scale.down.vert_luma_count >= p_scale_info->crop_height)
        {
            vert_row_index ++;
            p_scale_info->scale.down.vert_luma_count -= p_scale_info->crop_height;
        }
    }

    lumadiff = (int)((p_scale_info->p_luma_input + p_scale_info->scale.down.luma_input_seg_height * p_scale_info->crop_width)
                - (p_scale_info->p_luma_input_start + (p_scale_info->crop_height * p_scale_info->crop_width)));
    if(lumadiff > 0)
    {
        p_scale_info->scale.down.luma_input_seg_height-=(lumadiff/p_scale_info->crop_width);
    }
    // Chroma
    vert_row_index = 0;
    while (vert_row_index < p_scale_info->scale.down.vert_chroma_row_index)
    {
        // update chroma input pointer (Assuming H2V2)
        p_scale_info->p_chroma_input += (p_scale_info->rotated_chroma_src_increment) << 1;

        // update vertical chroma count of M/N algorithm
        p_scale_info->scale.down.vert_chroma_count += p_scale_info->output_chroma_height;

       // compare count and number of input vertical pixels
        if (p_scale_info->scale.down.vert_chroma_count >= p_scale_info->crop_chroma_height)
        {
             vert_row_index ++;
             p_scale_info->scale.down.vert_chroma_count -= p_scale_info->crop_chroma_height;
        }
    }
    return JPEGERR_SUCCESS;
}

/*===========================================================================
FUNCTION        jpege_engine_sw_downscale_configure_filter

DESCRIPTION     downscale configure based on an M/N algorithm

DEPENDENCIES    None

RETURN VALUE    INT

SIDE EFFECTS    None
===========================================================================*/
int jpege_engine_sw_downscale_configure_filter(
    jpege_scale_t *p_scale_info)
{
    int        rc;
    uint32_t   buffer_size;
    uint32_t   vert_row_index;
    /*------------------------------------------------------------
     M/N Algorithm:  downscale and output MCU lines
     1. init vertical count/row index/step
     2. init vertical accumulator
     3. allocate Luma input line buffer
     4. allocate Chroma input line buffer
     5. init function pointers for fetch
    ------------------------------------------------------------*/
    /*------------------------------------------------------------
      Vertical M/N init
    ------------------------------------------------------------*/
    p_scale_info->scale.down.vert_luma_count = 0;
    p_scale_info->scale.down.vert_chroma_count = 0;
    // Vertical row index is initialized, not always from start
    p_scale_info->scale.down.vert_luma_step = 0;
    p_scale_info->scale.down.vert_chroma_step = 0;
    p_scale_info->scale.down.vert_luma_row_index = p_scale_info->vert_luma_row_index;
    p_scale_info->scale.down.vert_chroma_row_index = p_scale_info->vert_chroma_row_index;
    /*------------------------------------------------------------
      At each encoding session
      1. Need to update luma and chroma input pointer for fetching
      2. Need to accumulate luma and chroma count
     ------------------------------------------------------------*/
    // Luma
    vert_row_index = 0;
    while (vert_row_index < p_scale_info->scale.down.vert_luma_row_index)
    {
        // update luma input pointer
        p_scale_info->p_luma_input += p_scale_info->rotated_luma_src_increment;

        // update vertical luma count of M/N algorithm
        p_scale_info->scale.down.vert_luma_count += p_scale_info->output_height;

        // compare count and number of input vertical pixels
        if (p_scale_info->scale.down.vert_luma_count >= p_scale_info->crop_height)
        {
            vert_row_index ++;
            p_scale_info->scale.down.vert_luma_count -= p_scale_info->crop_height;
        }
    }
    // Chroma
    vert_row_index = 0;
    while (vert_row_index < p_scale_info->scale.down.vert_chroma_row_index)
    {
        // update chroma input pointer
        p_scale_info->p_chroma_input += (p_scale_info->rotated_chroma_src_increment) << 1;

        // update vertical chroma count of M/N algorithm
        p_scale_info->scale.down.vert_chroma_count += p_scale_info->output_chroma_height;

       // compare count and number of input vertical pixels
        if (p_scale_info->scale.down.vert_chroma_count >= p_scale_info->crop_chroma_height)
        {
             vert_row_index ++;
             p_scale_info->scale.down.vert_chroma_count -= p_scale_info->crop_chroma_height;
        }
    }

    /*------------------------------------------------------------
      Vertical luma/chroma accumulator init
     ------------------------------------------------------------*/
    buffer_size = p_scale_info->output_width;
    if (p_scale_info->scale.down.p_vert_luma_accum)
    {
        JPEG_FREE(p_scale_info->scale.down.p_vert_luma_accum);
    }
    p_scale_info->scale.down.p_vert_luma_accum = (uint16_t *)JPEG_MALLOC(buffer_size * sizeof (uint16_t));
    if (!p_scale_info->scale.down.p_vert_luma_accum)
    {
        JPEG_DBG_ERROR("jpege_engine_sw_downscale_configure_filter: failed to allocate luma accumulator\n");
        return JPEGERR_EMALLOC;
    }
    STD_MEMSET(p_scale_info->scale.down.p_vert_luma_accum, 0, buffer_size * sizeof (uint16_t));

    buffer_size = p_scale_info->output_chroma_width * 2;
    if (p_scale_info->scale.down.p_vert_chroma_accum)
    {
        JPEG_FREE(p_scale_info->scale.down.p_vert_chroma_accum);
    }
    p_scale_info->scale.down.p_vert_chroma_accum = (uint16_t *)JPEG_MALLOC(buffer_size * sizeof (uint16_t));
    if (!p_scale_info->scale.down.p_vert_chroma_accum)
    {
        JPEG_DBG_ERROR("jpege_engine_sw_downscale_configure_filter: failed to allocate chroma accumulator\n");
        return JPEGERR_EMALLOC;
    }
    STD_MEMSET(p_scale_info->scale.down.p_vert_chroma_accum, 0, buffer_size * sizeof (uint16_t));

    /*------------------------------------------------------------
      Allocate internal luma and chroma line buffer
     ------------------------------------------------------------*/
    if (!p_scale_info->p_input_luma_line_buffer)
    {
        // Init luma line buffer
        rc = jpeg_buffer_init((jpeg_buffer_t *)&(p_scale_info->p_input_luma_line_buffer));
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jpege_engine_sw_downscale_configure_filter: p_input_luma_line_buffer init failed\n");
            return rc;
        }
    }
    // Reset luma line buffer
    rc = jpeg_buffer_reset((jpeg_buffer_t)p_scale_info->p_input_luma_line_buffer);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_MED("jpege_engine_sw_downscale_configure_filter: p_input_luma_line_buffer reset failed\n");
        return rc;
    }
    buffer_size = p_scale_info->crop_width * sizeof(uint8_t);
    // Allocate memory for luma Line buffer
    rc = jpeg_buffer_allocate((jpeg_buffer_t)p_scale_info->p_input_luma_line_buffer, buffer_size, false);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_engine_sw_downscale_configure_filter: p_input_luma_line_buffer %u bytes allocation failed\n",
                     buffer_size);
        jpeg_buffer_destroy((jpeg_buffer_t *)&(p_scale_info->p_input_luma_line_buffer));
        return rc;
    }
    STD_MEMSET(p_scale_info->p_input_luma_line_buffer->ptr, 0, buffer_size);

    if (!p_scale_info->p_input_chroma_line_buffer)
    {
        // Init chroma line buffer
        rc = jpeg_buffer_init((jpeg_buffer_t *)&(p_scale_info->p_input_chroma_line_buffer));
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jpege_engine_sw_downscale_configure_filter: p_input_chroma_line_buffer init failed\n");
            return rc;
        }
    }
    // Reset chroma line buffer
    rc = jpeg_buffer_reset((jpeg_buffer_t )p_scale_info->p_input_chroma_line_buffer);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_engine_sw_downscale_configure_filter: p_input_chroma_line_buffer reset failed\n");
        return rc;
    }
    buffer_size = p_scale_info->crop_chroma_width * 2 * sizeof(uint8_t);
    // Allocate chroma line buffer
    rc = jpeg_buffer_allocate((jpeg_buffer_t)p_scale_info->p_input_chroma_line_buffer, buffer_size, false);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_engine_sw_downscale_configure_filter: p_input_chroma_line_buffer %u bytes allocation failed\n",
                     buffer_size);
        jpeg_buffer_destroy((jpeg_buffer_t *)&(p_scale_info->p_input_chroma_line_buffer));
        return rc;
    }
    STD_MEMSET(p_scale_info->p_input_chroma_line_buffer->ptr, 0, buffer_size);

    if (!p_scale_info->p_vertical_scaled_buffer)
    {
       // Init vertical scaled buffer
        rc = jpeg_buffer_init((jpeg_buffer_t *)&(p_scale_info->p_vertical_scaled_buffer));
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jpege_engine_sw_downscale_configure_filter: p_vertical_scaled_buffer init failed\n");
            return rc;
        }
    }
    // Reset vertical scaled buffer
    rc = jpeg_buffer_reset((jpeg_buffer_t)p_scale_info->p_vertical_scaled_buffer);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jjpege_engine_sw_downscale_configure_filter: p_vertical_scaled_buffer reset failed\n");
        return rc;
    }
    // Allocate vertical scaled buffer
    rc = jpeg_buffer_allocate((jpeg_buffer_t)p_scale_info->p_vertical_scaled_buffer, buffer_size, false);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_engine_sw_downscale_configure_filter: p_vertical_scaled_buffer %u bytes allocation failed\n",
                       buffer_size);
        jpeg_buffer_destroy((jpeg_buffer_t *)&(p_scale_info->p_vertical_scaled_buffer));
        return rc;
    }
    STD_MEMSET(p_scale_info->p_vertical_scaled_buffer->ptr, 0, buffer_size);

    return JPEGERR_SUCCESS;
}

/*===========================================================================
FUNCTION       jpege_engine_sw_downscale_fetch_rotated_luma_to_line_buffer

DESCRIPTION     Fetch rotated Luma from the input buffer to one line buffer

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_downscale_fetch_rotated_luma_to_line_buffer(
    jpege_scale_t *p_scale_info)
{
    /*------------------------------------------------------------
     For rotation =0, it can fetch one line luma to buffer
    ------------------------------------------------------------*/
    uint32_t input_width = p_scale_info->vert_luma_fetch_width;
    int32_t  fetch_increment = p_scale_info->rotated_luma_fetch_increment;
    uint32_t pixel_extend_left = 0;
    uint32_t pixel_extend_right = 0;
    uint8_t  *p_src, *p_output;

    /*------------------------------------------------------------
     Fetch from the last row
    ------------------------------------------------------------*/
    p_src = p_scale_info->p_luma_input ;
    p_output = p_scale_info->p_input_luma_line_buffer->ptr;

    /*------------------------------------------------------------
      For rotated cases, it fetch pixel by pixel
      rotation = 90, 180 , 270
    ------------------------------------------------------------*/
    jpege_engine_sw_copy_rotated_luma_lines(p_src,
                                            p_output,
                                            input_width,
                                            fetch_increment,
                                            pixel_extend_left,
                                            pixel_extend_right);

    // update luma input pointer
    p_scale_info->p_luma_input += p_scale_info->rotated_luma_src_increment;
}

/*===========================================================================
FUNCTION        jpege_engine_sw_downscale_fetch_rotated_chroma_to_line_buffer

DESCRIPTION     Fetch rotated Chroma from the input buffer to line buffer

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_downscale_fetch_rotated_chroma_to_line_buffer(
    jpege_scale_t *p_scale_info)
{
    /*------------------------------------------------------------
     For rotation =0, it can fetch one line
    ------------------------------------------------------------*/
    uint32_t input_width = p_scale_info->vert_chroma_fetch_width << 1;
    int32_t  fetch_increment = p_scale_info->rotated_chroma_fetch_increment;
    uint32_t pixel_extend_left = 0;
    uint32_t pixel_extend_right = 0;
    uint8_t  *p_src, *p_output;

   /*------------------------------------------------------------
     Fetch from the last row
    ------------------------------------------------------------*/
    p_src = p_scale_info->p_chroma_input ;
    p_output = p_scale_info->p_input_chroma_line_buffer->ptr;

   /*------------------------------------------------------------
      For rotated cases, it fetch pixel by pixel
      rotation = 90, 180 , 270
    ------------------------------------------------------------*/
    if(p_scale_info->isPacked == 0 )
    {

        jpege_engine_sw_copy_rotated_chroma_lines(p_src,
                                                  p_output,
                                                  input_width,
                                                  fetch_increment,
                                                  pixel_extend_left,
                                                  pixel_extend_right);
    }
    else
    {
        jpege_engine_sw_copy_rotated_chroma_lines_packed(p_src,
                                                         p_output,
                                                         input_width,
                                                         fetch_increment,
                                                         pixel_extend_left,
                                                         pixel_extend_right);
    }
    // Update chroma input pointer
    p_scale_info->p_chroma_input += 2 * p_scale_info->rotated_chroma_src_increment;
}

/*===========================================================================
FUNCTION        jpege_engine_sw_downscale_rotated_mcu_lines

DESCRIPTION     downscale with MCU Lines using M/N algorithm:
                1) fetch rotated luma/chroma lines
                2) horizontal downscale luma/chroma lines
                3) vertical downscale luma/chroma lines
                4) output to scaled mcu lines buffer

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
void jpege_engine_sw_downscale_rotated_mcu_lines(
    jpege_scale_t  *p_scale_info,
    uint32_t        rotated_mcu_height)
{
    uint32_t  row_index_init = p_scale_info->vert_luma_row_index;
    uint32_t  row_index_bound = p_scale_info->vert_luma_row_index + rotated_mcu_height;
    uint32_t  chroma_row_index_bound = p_scale_info->vert_chroma_row_index + LUMA_BLOCK_HEIGHT;
    uint32_t  vert_luma_row_index = p_scale_info->vert_luma_row_index;
    uint32_t  vert_chroma_row_index = p_scale_info->vert_chroma_row_index;
    uint32_t  crop_width = p_scale_info->crop_width;
    uint32_t  crop_chroma_width = p_scale_info->crop_chroma_width;
    uint32_t  output_width = p_scale_info->output_width;
    uint32_t  output_chroma_width = p_scale_info->output_chroma_width;
    uint32_t  output_height = p_scale_info->output_height;
    uint32_t  output_chroma_height = p_scale_info->output_chroma_height;

    uint8_t  *p_input_line= p_scale_info->p_input_luma_line_buffer->ptr;
    uint8_t  *p_vert_scale_line = p_scale_info->p_vertical_scaled_buffer->ptr;
    uint8_t  *p_scale_line = p_scale_info->p_inline_scaled_buffer->ptr;

    /*------------------------------------------------------------
      Luma downscaling and output one downscaled MCU line
    ------------------------------------------------------------*/
    while ((vert_luma_row_index < row_index_bound) &&
           (vert_luma_row_index < output_height))
    {
        /*------------------------------------------------------------
          For rotation cases,
          fetch luma from source to line buffer
        ------------------------------------------------------------*/
        jpege_engine_sw_downscale_fetch_rotated_luma_to_line_buffer(p_scale_info);

        /*------------------------------------------------------------
          M/N horizontal downscale a luma line
        ------------------------------------------------------------*/
        jpege_engine_sw_downscale_horizontal_luma_line(p_input_line, p_vert_scale_line,
                                                       crop_width, output_width);

        /*------------------------------------------------------------
          M/N vertical dowanscale a luma line
        ------------------------------------------------------------*/
        jpege_engine_sw_downscale_vertical_luma_line(p_vert_scale_line, &p_scale_line, p_scale_info);

        vert_luma_row_index = p_scale_info->vert_luma_row_index;
    }

    /*------------------------------------------------------------
      Chroma downscaling and output one downscaled MCU line
    ------------------------------------------------------------*/
    p_input_line= p_scale_info->p_input_chroma_line_buffer->ptr;
    p_scale_line = p_scale_info->p_inline_scaled_buffer->ptr + output_width * rotated_mcu_height;
    p_vert_scale_line = p_scale_info->p_vertical_scaled_buffer->ptr;

    while ((vert_chroma_row_index < chroma_row_index_bound) &&
           (vert_chroma_row_index < output_chroma_height))
    {
        /*------------------------------------------------------------
          For rotation cases,
          fetch chroma from source to line buffer
        ------------------------------------------------------------*/
        jpege_engine_sw_downscale_fetch_rotated_chroma_to_line_buffer(p_scale_info);

        /*------------------------------------------------------------
          M/N horizontal downscale one chroma line
        ------------------------------------------------------------*/
        jpege_engine_sw_downscale_horizontal_chroma_line(p_input_line, p_vert_scale_line,
                                                         crop_chroma_width, output_chroma_width);

        /*------------------------------------------------------------
          M/N vertical downscale one chroma line
        ------------------------------------------------------------*/
        jpege_engine_sw_downscale_vertical_chroma_line(p_vert_scale_line, &p_scale_line, p_scale_info);

        vert_chroma_row_index = p_scale_info->vert_chroma_row_index;
    }

    /*------------------------------------------------------------
      Compute feteched luma blocks
    ------------------------------------------------------------*/
    p_scale_info->fetched_luma_blocks = (((vert_luma_row_index - row_index_init) * output_width
                                         + LUMA_BLOCK_SIZE - 1) >> LUMA_BLOCK_SIZE_DIGIT);
}

void jpege_engine_sw_downscale_mcu_lines_fastCV(
    jpege_scale_t *p_scale_info,
    uint32_t        rotated_mcu_height)
{
    uint32_t inputLumaHeight = p_scale_info->scale.down.luma_input_seg_height;
    uint32_t inputChromaHeight = p_scale_info->scale.down.chroma_input_seg_height;
    uint32_t outputHeight = p_scale_info->piece_height;
    uint32_t inputWidth = p_scale_info->crop_width;
    uint32_t outputWidth = p_scale_info->output_width;
    uint32_t inputStride = p_scale_info->rotated_luma_fetch_increment;
    uint8_t *p_luma_output_buffer = p_scale_info->fastCV_buffer + outputWidth * p_scale_info->scale.down.vert_luma_row_index;    //offset to output_pointer
    uint8_t *p_chroma_output_buffer = p_scale_info->fastCV_buffer + outputWidth * p_scale_info->output_height
                                                                    + outputWidth * p_scale_info->scale.down.vert_chroma_row_index;
    uint8_t *p_luma_input_buffer = p_scale_info->p_luma_input;
    uint8_t *p_chroma_input_buffer = p_scale_info->p_chroma_input;

    JPEG_DBG_LOW("inputLumaHeight = %d \n ", inputLumaHeight);
    JPEG_DBG_LOW("inputWidth = %d \n ", inputWidth);
    JPEG_DBG_LOW("outputHeight = %d \n ", outputHeight);
    JPEG_DBG_LOW("outputWidth = %d \n ", outputWidth);
    JPEG_DBG_LOW("inputChromaHeight = %d \n ", inputChromaHeight);
    JPEG_DBG_LOW("inputStride = %d \n ", inputStride);
    JPEG_DBG_LOW("p_luma_output_buffer = 0x%x \n " (uint32_t)p_luma_output_buffer);
    JPEG_DBG_LOW("p_chroma_output_buffer = 0x%x \n ", (uint32_t)p_chroma_output_buffer);

    fcvSetOperationMode(FASTCV_OP_CPU_PERFORMANCE);
    fcvScaleDownMNu8(p_luma_input_buffer,
        inputWidth,
        inputLumaHeight,
        inputStride,
        p_luma_output_buffer,
        outputWidth,
        outputHeight,
        outputWidth
    );
    fcvScaleDownMNInterleaveu8(p_chroma_input_buffer,
        inputWidth >> 1,
        inputLumaHeight >> 1,
        inputStride,
        p_chroma_output_buffer,
        outputWidth >> 1,
        outputHeight >> 1,
        outputWidth
    );
}
