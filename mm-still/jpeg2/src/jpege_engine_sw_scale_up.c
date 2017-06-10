/*========================================================================


*//** @file jpege_engine_sw_scale_up.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2010-2011, 2014 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/still/jpeg2/v2/latest/src/jpege_engine_sw_scale_up.c#2 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
03/01/12  csriniva Added support for YUY2 and UYVY inputs
04/06/10   staceyw Added crop and upscale configure and functions.
                   Both horizontal and vertical upscale uses a 4-tap
                   32 polyphase FIR Filter.
02/22/10   staceyw Created file

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include <stdlib.h>
#include "jpege_engine_sw_scale.h"
#include "jpeglog.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Quantity bits for phase of 4-tap 32 polyphase filter
** and the image output width/height can be support
** up to 2^(32-UPSCALE_POLYPHASE_ACCUM_Q_BITS)
** ----------------------------------------------------------------------- */
#define UPSCALE_POLYPHASE_ACCUM_Q_BITS 17     // for quant of accumulator

#define UPSCALE_POLYPHASE_PIXEL_Q_BITS 12     // for quant of pixel index

#define UPSCALE_POLYPHASE_FILTER_COEFFS 32*4  // 4-tap 32 polyphase filter

#define UPSCALE_POLYPHASE_FILTER_TAP 4        // tap is 4

// 4-tap 32 polyphase coefficents
// number of total coefficents = 32*4
// UPSCALE_POLYPHASE_FILTER_COEFFS = 32*4
static const int16_t polyphase_coeff_4taps[UPSCALE_POLYPHASE_FILTER_COEFFS]={
0, 512, 0, 0,
-7, 511, 8, 0,
-14, 507, 19, 0,
-19, 501, 32, -2,
-24, 493, 46, -3,
-28, 483, 62, -5,
-31, 472, 78, -7,
-34, 459, 96, -9,
-36, 444, 116, -12,
-37, 428, 135, -14,
-37, 410, 156, -17,
-37, 391, 177, -19,
-37, 372, 199, -22,
-36, 352, 221, -25,
-35, 331, 243, -27,
-33, 309, 265, -29,
-32, 288, 288, -32,
-29, 265, 309, -33,
-27, 243, 331, -35,
-25, 221, 352, -36,
-22, 199, 372, -37,
-19, 177, 391, -37,
-17, 156, 410, -37,
-14, 135, 428, -37,
-12, 116, 444, -36,
-9, 96, 459, -34,
-7, 78, 472, -31,
-5, 62, 483, -28,
-3, 46, 493, -24,
-2, 32, 501, -19,
0, 19, 507, -14,
0, 8, 511, -7};

/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* functions related with in-line up-scale */
int jpege_engine_sw_upscale_configure_filter(
    jpege_scale_t *p_scale_info,
    uint32_t       rotation);
void jpege_engine_sw_upscale_mcu_lines(
    jpege_scale_t  *p_scale_info,
    uint32_t        rotated_mcu_height);
// upscale vertical/horizontal luma/chroma line
static void jpege_engine_sw_upscale_vertical_luma_line(
    const int16_t  *p_filter,
    uint8_t  *p_input_line,
    uint8_t  *p_output_line,
    uint32_t  crop_extended_width,
    uint32_t  vert_pixel_index);
static void jpege_engine_sw_upscale_horizontal_luma_line(
    uint8_t  *p_input_line,
    uint8_t  *p_output_line,
    uint32_t  hori_accumulator,
    uint32_t  output_width,
    uint32_t  hori_step);
static void jpege_engine_sw_upscale_vertical_chroma_line(
    const int16_t  *p_filter,
    uint8_t  *p_input_line,
    uint8_t  *p_output_line,
    uint32_t  crop_extended_width,
    uint32_t  vert_pixel_index);
static void jpege_engine_sw_upscale_horizontal_chroma_line(
    uint8_t  *p_input_line,
    uint8_t  *p_output_line,
    uint32_t  hori_accumulator,
    uint32_t  output_chroma_width,
    uint32_t  hori_step);
// fetch non-rotated or rotated luma/chroma to lines buffer
static void jpege_engine_sw_upscale_fetch_luma_to_lines_buffer(
    jpege_scale_t *p_scale_info,
    uint32_t       line_index,
    uint32_t       vert_pixel_index);
static void jpege_engine_sw_upscale_fetch_chroma_to_lines_buffer(
    jpege_scale_t *p_scale_info,
    uint32_t       line_index,
    uint32_t       vert_pixel_index);
static void jpege_engine_sw_upscale_fetch_luma_to_lines_buffer_packed(
    jpege_scale_t *p_scale_info,
    uint32_t       line_index,
    uint32_t       vert_pixel_index);
static void jpege_engine_sw_upscale_fetch_chroma_to_lines_buffer_packed(
    jpege_scale_t *p_scale_info,
    uint32_t       line_index,
    uint32_t       vert_pixel_index);
static void jpege_engine_sw_upscale_fetch_rotated_luma_to_lines_buffer(
    jpege_scale_t *p_scale_info,
    uint32_t       line_index,
    uint32_t       vert_pixel_index);
static void jpege_engine_sw_upscale_fetch_rotated_chroma_to_lines_buffer(
    jpege_scale_t *p_scale_info,
    uint32_t       line_index,
    uint32_t       vert_pixel_index);
static void jpege_engine_sw_upscale_fetch_rotated_chroma_to_lines_buffer_packed(
    jpege_scale_t *p_scale_info,
    uint32_t       line_index,
    uint32_t       vert_pixel_index);
// jpeg fetch mcu funtion ptr
static void (* jpege_engine_sw_upscale_fetch_luma_to_lines_buffer_func)(
    jpege_scale_t *p_scale_info,
    uint32_t       line_index,
    uint32_t       vert_pixel_index);
static void (* jpege_engine_sw_upscale_fetch_chroma_to_lines_buffer_func)(
    jpege_scale_t *p_scale_info,
    uint32_t       line_index,
    uint32_t       vert_pixel_index);
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
#define CLAMP255(x) ( ((x) & 0xFFFFFF00) ? ((~(x) >> 31) & 0xFF) : (x))

#define MOD4(a) ((a) & 0x3)
/* =======================================================================
**                          Function Definitions
** ======================================================================= */

/*===========================================================================
FUNCTION        jpege_engine_sw_upscale_configure_filter

DESCRIPTION     upscalar configure 4-tap 32 polyphase filter

DEPENDENCIES    None

RETURN VALUE    INT

SIDE EFFECTS    None
===========================================================================*/
int jpege_engine_sw_upscale_configure_filter(
    jpege_scale_t *p_scale_info,
    uint32_t       rotation)
{
    int      rc;
    uint32_t buffer_size;
    jpege_scale_up_t  *p_scale_up = &(p_scale_info->scale.up);

    /*------------------------------------------------------------
     32 polyphase 4-tap Filter:
     1. set default UPSCALE_POLYPHASE_ACCUM_Q_BITS = 17 bits
        for quantization
     2. polyphase is represented at the highest 5 bits
     3. initial phase = 0
     4. calculate phase step
     5. calculate number of pixels to be extended
     6. calculate luma and chroma extended width
    ------------------------------------------------------------*/
    p_scale_up->vert_luma_last_index= 0;
    p_scale_up->vert_chroma_last_index = 0;
    // vertical row index is initilized, not always from start
    /*------------------------------------------------------------
      Vertical Polyphase init
    ------------------------------------------------------------*/
    // vertical init
    p_scale_up->vert_init = 0;

    // vertical phase step
    p_scale_up->vert_step
         = (((uint32_t)p_scale_info->crop_height << UPSCALE_POLYPHASE_ACCUM_Q_BITS)
                     / p_scale_info->output_height);

    // Init vertical accumulator
    p_scale_up->vert_luma_accum = 0;
    p_scale_up->vert_chroma_accum = 0;

    /*------------------------------------------------------------
      Horizontal Polyphase init
    ------------------------------------------------------------*/
   // vertical init
    p_scale_up->hori_init = 0;

   // vertical phase step
    p_scale_up->hori_step
        = (((uint32_t)p_scale_info->vert_luma_fetch_width << UPSCALE_POLYPHASE_ACCUM_Q_BITS)
                     / p_scale_info->output_width);

    // Compute number of pixels to be extended
    p_scale_up->pixel_extend_left = 1;
    p_scale_up->pixel_extend_right
         = (((p_scale_up->hori_step * (p_scale_info->output_width - 1))
             >> UPSCALE_POLYPHASE_ACCUM_Q_BITS ) + 3 - p_scale_info->vert_luma_fetch_width);

     // Compute extended width
    p_scale_up->crop_extended_width
         = (p_scale_info->vert_luma_fetch_width + p_scale_up->pixel_extend_left
                              + p_scale_up->pixel_extend_right);
    // p_scale_info->vert_chroma_fetch_width = nRotatedCropChromaWidth
    p_scale_up->crop_chroma_extended_width
         = (p_scale_info->vert_chroma_fetch_width + p_scale_up->pixel_extend_left
                                    + p_scale_up->pixel_extend_right);

    // Allocate memory for the luma Lines
    // UPSCALE_POLYPHASE_FILTER_TAP = 4 is the number of luma lines buffer
    if (!p_scale_info->p_input_luma_line_buffer)
    {
        // Init luma line buffer
        rc = jpeg_buffer_init((jpeg_buffer_t *)&(p_scale_info->p_input_luma_line_buffer));
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jpege_engine_sw_upscale_configure: jpege_scale.p_input_luma_line_buffer init failed\n");
            return rc;
        }
    }
    // Reset luma line buffer
    rc = jpeg_buffer_reset((jpeg_buffer_t)p_scale_info->p_input_luma_line_buffer);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_MED("jpege_engine_sw_upscale_configure: jpege_scale.p_input_luma_line_buffer init failed\n");
        return rc;
    }
    buffer_size = UPSCALE_POLYPHASE_FILTER_TAP
                  * p_scale_up->crop_extended_width * sizeof(uint8_t);
    rc = jpeg_buffer_allocate((jpeg_buffer_t)p_scale_info->p_input_luma_line_buffer, buffer_size, false);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_engine_sw_upscale_configure: jpege_scale.p_input_luma_line_buffer %u bytes allocation failed\n",
                     buffer_size);
        jpeg_buffer_destroy((jpeg_buffer_t *)&(p_scale_info->p_input_luma_line_buffer));
        return rc;
    }

    // Allocate memory for the chroma Lines
    // UPSCALE_POLYPHASE_FILTER_TAP = 4 is the number of chroma lines buffer
    if (!p_scale_info->p_input_chroma_line_buffer)
    {
        // Init chroma line buffer
       rc = jpeg_buffer_init((jpeg_buffer_t *)&(p_scale_info->p_input_chroma_line_buffer));
       if (JPEG_FAILED(rc))
       {
           JPEG_DBG_ERROR("jpege_engine_sw_upscale_configure: jpege_scale.p_input_chroma_line_buffer init failed\n");
           return rc;
       }
    }
    // Reset chroma line buffer
    rc = jpeg_buffer_reset((jpeg_buffer_t)p_scale_info->p_input_chroma_line_buffer);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_engine_sw_upscale_configure: jpege_scale.p_input_chroma_line_buffer init failed\n");
        return rc;
    }
    buffer_size = UPSCALE_POLYPHASE_FILTER_TAP
                 * p_scale_up->crop_chroma_extended_width * 2 * sizeof(uint8_t);
    rc = jpeg_buffer_allocate((jpeg_buffer_t)p_scale_info->p_input_chroma_line_buffer, buffer_size, false);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_engine_sw_upscale_configure: jpege_scale.p_input_chroma_line_buffer %u bytes allocation failed\n",
                     buffer_size);
        jpeg_buffer_destroy((jpeg_buffer_t *)&(p_scale_info->p_input_chroma_line_buffer));
        return rc;
    }

    // Allocate memory for internal Vertical Scale Buffer
    if (!p_scale_info->p_vertical_scaled_buffer)
    {
        // Init vertical scaled buffer
        rc = jpeg_buffer_init((jpeg_buffer_t *)&(p_scale_info->p_vertical_scaled_buffer));
        if (JPEG_FAILED(rc))
        {
           JPEG_DBG_ERROR("jpege_engine_sw_upscale_configure: jpege_scale.p_vertical_scaled_buffer init failed\n");
           return rc;
        }
    }
    // Reset vertical scaled buffer
    rc = jpeg_buffer_reset((jpeg_buffer_t)p_scale_info->p_vertical_scaled_buffer);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_engine_sw_upscale_configure: jpege_scale.p_vertical_scaled_buffer init failed\n");
        return rc;
    }
    buffer_size = p_scale_up->crop_chroma_extended_width * 2 * sizeof(uint8_t);
    rc = jpeg_buffer_allocate((jpeg_buffer_t)p_scale_info->p_vertical_scaled_buffer, buffer_size, false);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_engine_sw_upscale_configure: jpege_scale.p_vertical_scaled_buffer %u bytes allocation failed\n",
                     buffer_size);
        jpeg_buffer_destroy((jpeg_buffer_t *)&(p_scale_info->p_vertical_scaled_buffer));
        return rc;
    }

    /*------------------------------------------------------------
      For non-rotation and rotation case,
      set up fetch luma to line buffer function pointer
    ------------------------------------------------------------*/
    if(p_scale_info->isPacked == 0)
    {
        if (rotation == 0)
        {
            jpege_engine_sw_upscale_fetch_luma_to_lines_buffer_func = &jpege_engine_sw_upscale_fetch_luma_to_lines_buffer;
            jpege_engine_sw_upscale_fetch_chroma_to_lines_buffer_func = &jpege_engine_sw_upscale_fetch_chroma_to_lines_buffer;
        }
        else
        {
            jpege_engine_sw_upscale_fetch_luma_to_lines_buffer_func = &jpege_engine_sw_upscale_fetch_rotated_luma_to_lines_buffer;
            jpege_engine_sw_upscale_fetch_chroma_to_lines_buffer_func = &jpege_engine_sw_upscale_fetch_rotated_chroma_to_lines_buffer;
        }
    }
    else
    {
        if (rotation == 0)
        {
            jpege_engine_sw_upscale_fetch_luma_to_lines_buffer_func = &jpege_engine_sw_upscale_fetch_luma_to_lines_buffer_packed;
            jpege_engine_sw_upscale_fetch_chroma_to_lines_buffer_func = &jpege_engine_sw_upscale_fetch_chroma_to_lines_buffer_packed;
        }
        else
        {
            jpege_engine_sw_upscale_fetch_luma_to_lines_buffer_func = &jpege_engine_sw_upscale_fetch_rotated_luma_to_lines_buffer;
            jpege_engine_sw_upscale_fetch_chroma_to_lines_buffer_func = &jpege_engine_sw_upscale_fetch_rotated_chroma_to_lines_buffer_packed;
        }
    }
    return JPEGERR_SUCCESS;
}

/*===========================================================================
FUNCTION        jpege_engine_sw_upscale_mcu_lines

DESCRIPTION     Scale MCU Lines from the input buffers
                1) fetch the luma/chroma lines to line buffer
                2) vertical scale luma/chroma lines
                3) horizontal scale luma/chroma lines
                4) output to m_pScaleBuffer

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
void jpege_engine_sw_upscale_mcu_lines(
    jpege_scale_t  *p_scale_info,
    uint32_t        rotated_mcu_height)
{
    jpege_scale_up_t *p_scale_up = &(p_scale_info->scale.up);
    uint32_t  vert_luma_accum =p_scale_up->vert_luma_accum;
    uint32_t  vert_chroma_accum = p_scale_up->vert_chroma_accum;
    uint32_t  vert_luma_row_index = p_scale_info->vert_luma_row_index;
    uint32_t  vert_luma_row_init = p_scale_info->vert_luma_row_index;
    uint32_t  vert_chroma_row_index = p_scale_info->vert_chroma_row_index;
    uint32_t  vert_luma_row_bound = vert_luma_row_index + rotated_mcu_height;
    uint32_t  vert_chroma_row_bound = vert_chroma_row_index + LUMA_BLOCK_HEIGHT;

    uint32_t  vert_luma_last_index = p_scale_up->vert_luma_last_index;
    uint32_t  vert_chroma_last_index = p_scale_up->vert_chroma_last_index;
    uint32_t  output_height = p_scale_info->vert_luma_output_height;
    uint32_t  output_width = p_scale_info->output_width;
    uint32_t  output_chroma_height = p_scale_info->vert_chroma_output_height;
    uint32_t  output_chroma_width = p_scale_info->output_chroma_width;
    uint32_t  crop_extended_width = p_scale_up->crop_extended_width;
    uint32_t  crop_chroma_extended_width = p_scale_up->crop_chroma_extended_width;
    uint32_t  hori_accumulator = p_scale_up->hori_init; // horizontal accumulator
    uint32_t  hori_step = p_scale_up->hori_step; // horizontal step

    uint8_t  *p_input_line= p_scale_info->p_input_luma_line_buffer->ptr;
    uint8_t  *p_scale_line = p_scale_info->p_inline_scaled_buffer->ptr;
    uint8_t  *p_vert_scale_line = p_scale_info->p_vertical_scaled_buffer->ptr;

    uint32_t  vert_phase_index;
    uint32_t  coeff_index;
    const int16_t  *p_filter;
    uint32_t  vert_pixel_index; // current pixel index
    uint8_t   i;

    // Initialization and fetch to buffer
    if (vert_luma_row_index == 0)
    {
        vert_luma_accum = p_scale_up->vert_init;
    }
    else
    {
        vert_luma_accum = (vert_luma_row_index - 1) * p_scale_up->vert_step;
        vert_pixel_index = (vert_luma_accum >> UPSCALE_POLYPHASE_ACCUM_Q_BITS);
        for (i = 0; i < UPSCALE_POLYPHASE_FILTER_TAP; i++)
        {
            jpege_engine_sw_upscale_fetch_luma_to_lines_buffer_func(p_scale_info,
                                                               vert_luma_row_index,
                                                                vert_pixel_index-i);
        }
    }

    /*------------------------------------------------------------
      Luma upscaling
    ------------------------------------------------------------*/
    while((vert_luma_row_index < vert_luma_row_bound) &&
          (vert_luma_row_index < output_height))
    {
        if (vert_luma_row_index > 0)
        {
            vert_luma_accum += p_scale_up->vert_step;
        }
       /*------------------------------------------------------------
          UPSCALE_POLYPHASE_ACCUM_Q_BITS used for quantity
        ------------------------------------------------------------*/
        vert_pixel_index = (vert_luma_accum >> UPSCALE_POLYPHASE_ACCUM_Q_BITS);

        /*------------------------------------------------------------
          phase Index gets the most significant 5 fractional bits
          of Accumulator
        ------------------------------------------------------------*/
        vert_phase_index = (uint32_t)((vert_luma_accum >> UPSCALE_POLYPHASE_PIXEL_Q_BITS)
                                       - (vert_pixel_index << 5));

        // fetch lines if not in the luma lines buffer
        if ((vert_luma_row_index == 0) || (vert_pixel_index > vert_luma_last_index))
        {
           jpege_engine_sw_upscale_fetch_luma_to_lines_buffer_func(p_scale_info,
                                                                   vert_luma_row_index,
                                                                   vert_pixel_index);
        }

        /*------------------------------------------------------------
          Vertical scale a luma line
          There are one line extended in line buffer at top
        ------------------------------------------------------------*/
        // phase Index is between 0 and 31
        if (vert_phase_index > 31)
        {
            vert_phase_index = 0;
        }
        // 32 polyphase with 4 tap each
        // calculate polyphase coefficient index
        coeff_index = (vert_phase_index << 2);

        p_filter = &(polyphase_coeff_4taps[coeff_index]);

        jpege_engine_sw_upscale_vertical_luma_line(p_filter, p_input_line, p_vert_scale_line,
                                                   crop_extended_width, vert_pixel_index);

        /*------------------------------------------------------------
          Horizontal scale a luma line
        ------------------------------------------------------------*/
        jpege_engine_sw_upscale_horizontal_luma_line(p_vert_scale_line, p_scale_line,
                                                 hori_accumulator, output_width, hori_step);

        vert_luma_last_index = vert_pixel_index;

        p_scale_line += output_width;

        vert_luma_row_index++;
    }

    /*------------------------------------------------------------
      Chroma vertical upscaling
    ------------------------------------------------------------*/
    p_input_line= p_scale_info->p_input_chroma_line_buffer->ptr;
    p_scale_line = p_scale_info->p_inline_scaled_buffer->ptr + output_width * rotated_mcu_height;
    p_vert_scale_line = p_scale_info->p_vertical_scaled_buffer->ptr;

    // Initialization and fetch to buffer
    if (vert_chroma_row_index == 0)
    {
        vert_chroma_row_index = p_scale_up->vert_init;
    }
    else
    {
        vert_chroma_accum = (vert_chroma_row_index - 1) * p_scale_up->vert_step;
        vert_pixel_index = (vert_chroma_accum  >> UPSCALE_POLYPHASE_ACCUM_Q_BITS);
        for (i = 0; i < UPSCALE_POLYPHASE_FILTER_TAP; i++)
        {
           jpege_engine_sw_upscale_fetch_chroma_to_lines_buffer_func(p_scale_info,
                                                                     vert_chroma_row_index,
                                                                     vert_pixel_index-i);
        }
    }

    while((vert_chroma_row_index < vert_chroma_row_bound) &&
          (vert_chroma_row_index < output_chroma_height))
    {
        if (vert_chroma_row_index == 0)
        {
            vert_chroma_accum = p_scale_up->vert_init;
        }
        else
        {
            vert_chroma_accum += p_scale_up->vert_step;
        }

        /*------------------------------------------------------------
          UPSCALE_POLYPHASE_ACCUM_Q_BITS used for quantity
        ------------------------------------------------------------*/
        vert_pixel_index = (vert_chroma_accum >> UPSCALE_POLYPHASE_ACCUM_Q_BITS);

        /*------------------------------------------------------------
          phase Index gets the most significant 5 fractional bits
          of Accumulator
        ------------------------------------------------------------*/
        vert_phase_index = (uint32_t)((vert_chroma_accum >> (UPSCALE_POLYPHASE_PIXEL_Q_BITS))
                                       - (vert_pixel_index << 5));

        // fetch lines if not in the chroma lines buffer
        if ((vert_chroma_row_index == 0) || (vert_pixel_index > vert_chroma_last_index))
        {
           jpege_engine_sw_upscale_fetch_chroma_to_lines_buffer_func(p_scale_info,
                                                                     vert_chroma_row_index,
                                                                     vert_pixel_index);
        }
        /*------------------------------------------------------------
          Vertical scale
        ------------------------------------------------------------*/
        // phase Index is between 0 and 31
        if (vert_phase_index > 31)
        {
            vert_phase_index = 0;
        }
        // 32 polyphase with 4 tap each
        // calculate polyphase coefficient index
        coeff_index = (vert_phase_index << 2);

        p_filter = &(polyphase_coeff_4taps[coeff_index]);
        jpege_engine_sw_upscale_vertical_chroma_line(p_filter, p_input_line, p_vert_scale_line,
                                                     crop_chroma_extended_width, vert_pixel_index);

        /*------------------------------------------------------------
          Horizontal scale
        ------------------------------------------------------------*/
        jpege_engine_sw_upscale_horizontal_chroma_line(p_vert_scale_line, p_scale_line,
                                                       hori_accumulator, output_chroma_width, hori_step);

        vert_chroma_last_index = vert_pixel_index;

        p_scale_line += (output_chroma_width << 1);

        vert_chroma_row_index ++;
    }

    /*------------------------------------------------------------
      Compute feteched luma blocks
    ------------------------------------------------------------*/
    p_scale_info->fetched_luma_blocks = (((vert_luma_row_index - vert_luma_row_init) * output_width
                                          + LUMA_BLOCK_SIZE - 1) >> LUMA_BLOCK_SIZE_DIGIT);

    // in-line upscale accumulator/index update
    p_scale_up->vert_luma_accum = vert_luma_accum;
    p_scale_up->vert_chroma_accum = vert_chroma_accum;
    // update row index
    p_scale_info->vert_luma_row_index = vert_luma_row_index;
    p_scale_info->vert_chroma_row_index = vert_chroma_row_index;
    p_scale_up->vert_luma_last_index = vert_luma_last_index;
    p_scale_up->vert_chroma_last_index = vert_chroma_last_index;
}


/*===========================================================================
FUNCTION        jpege_engine_sw_upscale_vertical_luma_line

DESCRIPTION     Verticallly Scale Luma Line

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_upscale_vertical_luma_line(
    const int16_t  *p_filter,
    uint8_t  *p_input_line,
    uint8_t  *p_output_line,
    uint32_t  crop_extended_width,
    uint32_t  vert_pixel_index)
{
    int32_t   weight_sum;
    uint8_t  *p_last_line, *p_curr_line, *p_next_line, *p_advc_line;
    uint32_t  step_width = crop_extended_width;

    /*------------------------------------------------------------
      Scale Luma Line
    ------------------------------------------------------------*/
    // vertical scale for one line
    // it need pixel_index-1, pixel_index, pixel_index+1, pixel_index+2 lines
    p_last_line = p_input_line + MOD4((vert_pixel_index + 0)) * step_width;
    p_curr_line = p_input_line + MOD4((vert_pixel_index + 1)) * step_width;
    p_next_line = p_input_line + MOD4((vert_pixel_index + 2)) * step_width;
    p_advc_line = p_input_line + MOD4((vert_pixel_index + 3)) * step_width;

    while (crop_extended_width --)
    {
       // 4-tap filter
        weight_sum =   p_filter[0] * (*(p_last_line ++))
                     + p_filter[1] * (*(p_curr_line ++))
                     + p_filter[2] * (*(p_next_line ++))
                     + p_filter[3] * (*(p_advc_line ++)) ;

        // coefficents sum equal to 512
        // make rounding
        weight_sum = ((1 + (weight_sum >> 8)) >> 1);

        // CLAMP255255 to between 0 and 255
        // write output pixel
        *(p_output_line ++) = (uint8_t) (CLAMP255(weight_sum));
    } //end for loop

} // end of jpege_engine_sw_vertical_scale_luma_line

/*===========================================================================
FUNCTION        jpege_engine_sw_upscale_vertical_chroma_line

DESCRIPTION     Verticallly Scale Chroma Lines

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_upscale_vertical_chroma_line(
    const int16_t  *p_filter,
    uint8_t  *p_input_line,
    uint8_t  *p_output_line,
    uint32_t  crop_chroma_extended_width,
    uint32_t  vert_pixel_index)
{
    int32_t   weight_sum;
    uint8_t  *p_last_line, *p_curr_line, *p_next_line, *p_advc_line;
    uint32_t  step_width = (crop_chroma_extended_width << 1);

    /*------------------------------------------------------------
      Scale for both Cb and Cr
    ------------------------------------------------------------*/
    // vertical scale for one line
    // it need pixel_index-1, pixel_index, pixel_index+1, pixel_index+2 lines
    p_last_line = p_input_line + MOD4((vert_pixel_index + 0)) * step_width ;
    p_curr_line = p_input_line + MOD4((vert_pixel_index + 1)) * step_width ;
    p_next_line = p_input_line + MOD4((vert_pixel_index + 2)) * step_width ;
    p_advc_line = p_input_line + MOD4((vert_pixel_index + 3)) * step_width ;

    while (crop_chroma_extended_width --)
    {
        // 4-tap filter
        weight_sum =   p_filter[0] * (*(p_last_line ++))
                     + p_filter[1] * (*(p_curr_line ++))
                     + p_filter[2] * (*(p_next_line ++))
                     + p_filter[3] * (*(p_advc_line ++)) ;

        // coefficents sum equal to 512
        // make rounding
        weight_sum = ((1 + (weight_sum >> 8)) >> 1);

        // CLAMP255 to between 0 and 255
        // write output pixel
        *(p_output_line ++) = (uint8_t)(CLAMP255(weight_sum));

        // 4-tap filter
        weight_sum =   p_filter[0] * (*(p_last_line ++))
                     + p_filter[1] * (*(p_curr_line ++))
                     + p_filter[2] * (*(p_next_line ++))
                     + p_filter[3] * (*(p_advc_line ++)) ;

        // coefficents sum equal to 512
        // make rounding
        weight_sum = ((1 + (weight_sum >> 8)) >> 1);

        // CLAMP255 to between 0 and 255
        // write output pixel
        *(p_output_line ++) = (uint8_t)(CLAMP255(weight_sum));
    } // end for loop

} // end of  jpege_engine_sw_upscale_vertical_chroma_line

/*===========================================================================
FUNCTION        jpege_engine_sw_upscale_horizontal_luma_line

DESCRIPTION     Horizontally Scale Luma Lines
                1) caculate accumulator
                2) caculate pixel index
                3) caculate phase index
                3) output luma line afer scale

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_upscale_horizontal_luma_line(
    uint8_t  *p_input_line,
    uint8_t  *p_output_line,
    uint32_t  hori_accumulator,
    uint32_t  output_width,
    uint32_t  hori_step)
{
    const int16_t  *p_filter;
    int32_t  weight_sum;
    // Compute pixel index
    uint32_t pixel_index = (hori_accumulator >> UPSCALE_POLYPHASE_ACCUM_Q_BITS);

    // Compute phase
    uint32_t phase_index = (uint32_t)((hori_accumulator >> UPSCALE_POLYPHASE_PIXEL_Q_BITS)
                                       - (pixel_index << 5));

    // 32 polyphase with 4 tap each
    // calculate polyphase coefficient index
    uint32_t coeff_index = (phase_index << 2);

    p_filter = &(polyphase_coeff_4taps[coeff_index]);

    // 4-tap filter
    weight_sum =   p_filter[0] * (*(p_input_line + pixel_index))
                 + p_filter[1] * (*(p_input_line + pixel_index + 1))
                 + p_filter[2] * (*(p_input_line + pixel_index + 2))
                 + p_filter[3] * (*(p_input_line + pixel_index + 3));

    // sum of coefficients equals to 516=2^9
    weight_sum = ((1 + (weight_sum >> 8)) >> 1);

    // CLAMP255 to between 0 and 255
    // write output pixel
    *(p_output_line ++) = (uint8_t)(CLAMP255(weight_sum));
    output_width --;
    while (output_width --)
    {
        hori_accumulator += hori_step;

        // Compute pixel index
        pixel_index = (hori_accumulator >> UPSCALE_POLYPHASE_ACCUM_Q_BITS);

        // Compute phase
        phase_index = (uint32_t)((hori_accumulator >> UPSCALE_POLYPHASE_PIXEL_Q_BITS)
                                  - (pixel_index << 5));

        // phase Index is between 0 and 31
        if (phase_index > 31)
        {
            phase_index = 0;
        }
        // 32 polyphase with 4 tap each
        // calculate polyphase coefficient index
        coeff_index = (phase_index << 2);

        p_filter = &(polyphase_coeff_4taps[coeff_index]);

       // 4-tap filter
        weight_sum =   p_filter[0] * (*(p_input_line + pixel_index))
                     + p_filter[1] * (*(p_input_line + pixel_index + 1))
                     + p_filter[2] * (*(p_input_line + pixel_index + 2))
                     + p_filter[3] * (*(p_input_line + pixel_index + 3)) ;

        // sum of coefficients equals to 516=2^9
        weight_sum = ((1 + (weight_sum >> 8)) >> 1);

        // CLAMP255 to between 0 and 255
        // write output pixel
        *(p_output_line ++) = (uint8_t)(CLAMP255(weight_sum));
    }
}

/*===========================================================================
FUNCTION        jpege_engine_sw_upscale_horizontal_chroma_line

DESCRIPTION     Horizontally Scale Chroma Lines
                1) caculate accumulator
                2) caculate pixel index
                3) caculate phase index
                3) output chroma line afer scale

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None

===========================================================================*/
static void jpege_engine_sw_upscale_horizontal_chroma_line(
    uint8_t  *p_input_line,
    uint8_t  *p_output_line,
    uint32_t  hori_accumulator,
    uint32_t  output_chroma_width,
    uint32_t  hori_step)
{
    const int16_t  *p_filter;
    int32_t  weight_sum;
    // Compute pixel index
    uint32_t  pixel_index = (hori_accumulator >> UPSCALE_POLYPHASE_ACCUM_Q_BITS);
    uint32_t  chroma_pixel_index = (pixel_index << 1);

    // Compute phase
    uint32_t  phase_index = (uint32_t)((hori_accumulator >> UPSCALE_POLYPHASE_PIXEL_Q_BITS)
                                        - (pixel_index << 5));

    // 32 polyphase with 4 tap each
    // calculate polyphase coefficient index
    uint32_t  coeff_index = (phase_index << 2);

    /*------------------------------------------------------------
      Scale Lines for both Cb Cr
    ------------------------------------------------------------*/
    p_filter = &(polyphase_coeff_4taps[coeff_index]);

    // 4-tap filter
    weight_sum =   p_filter[0] * (*(p_input_line + chroma_pixel_index))
                 + p_filter[1] * (*(p_input_line + chroma_pixel_index + 2))
                 + p_filter[2] * (*(p_input_line + chroma_pixel_index + 4))
                 + p_filter[3] * (*(p_input_line + chroma_pixel_index + 6)) ;

    // sum of coefficients equals to 516=2^9
    weight_sum = ((1 + (weight_sum >> 8)) >> 1);

    // CLAMP255 to between 0 and 255
    // write output pixel
    *(p_output_line ++) = (uint8_t)(CLAMP255(weight_sum));

    weight_sum =   p_filter[0] * (*(p_input_line + chroma_pixel_index + 1))
                 + p_filter[1] * (*(p_input_line + chroma_pixel_index + 3))
                 + p_filter[2] * (*(p_input_line + chroma_pixel_index + 5))
                 + p_filter[3] * (*(p_input_line + chroma_pixel_index + 7)) ;

    // sum of coefficients equals to 516=2^9
    weight_sum = (1 + (weight_sum >> 8)) >> 1;

    // CLAMP255 to between 0 and 255
    // write output pixel
    *(p_output_line ++) = (uint8_t)(CLAMP255(weight_sum));
    output_chroma_width --;

    while (output_chroma_width --)
    {
        hori_accumulator += hori_step;

        // Compute pixel index
        pixel_index = (hori_accumulator >> UPSCALE_POLYPHASE_ACCUM_Q_BITS);

        chroma_pixel_index = (pixel_index << 1);

       // Compute phase
        phase_index = (uint32_t)((hori_accumulator >> UPSCALE_POLYPHASE_PIXEL_Q_BITS)
                                  - (pixel_index << 5));

        // phase Index is between 0 and 31
        if (phase_index > 31)
        {
            phase_index = 0;
        }
        // 32 polyphase with 4 tap each
        // calculate polyphase coefficient index
        coeff_index = (phase_index << 2);

        p_filter = &(polyphase_coeff_4taps[coeff_index]);
        // 4-tap filter
        weight_sum =   p_filter[0] * (*(p_input_line + chroma_pixel_index))
                     + p_filter[1] * (*(p_input_line + chroma_pixel_index + 2))
                     + p_filter[2] * (*(p_input_line + chroma_pixel_index + 4))
                     + p_filter[3] * (*(p_input_line + chroma_pixel_index + 6)) ;

        // sum of coefficients equals to 516=2^9
        weight_sum = ((1 + (weight_sum >> 8)) >> 1);

        // CLAMP255 to between 0 and 255
        // write output pixel
        *(p_output_line ++) = (uint8_t)(CLAMP255(weight_sum));
        weight_sum =   p_filter[0] * (*(p_input_line + chroma_pixel_index + 1))
                     + p_filter[1] * (*(p_input_line + chroma_pixel_index + 3))
                     + p_filter[2] * (*(p_input_line + chroma_pixel_index + 5))
                     + p_filter[3] * (*(p_input_line + chroma_pixel_index + 7)) ;

        // sum of coefficients equals to 516=2^9
        weight_sum = ((1 + (weight_sum >> 8)) >> 1);

        // CLAMP255 to between 0 and 255
        // write output pixel
        *(p_output_line ++) = (uint8_t)(CLAMP255(weight_sum));
    }
}

/*===========================================================================
FUNCTION        jpege_engine_sw_upscale_fetch_luma_to_lines_buffer

DESCRIPTION     Fetch Luma from the input buffer to lines buffer

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_upscale_fetch_luma_to_lines_buffer(
    jpege_scale_t *p_scale_info,
    uint32_t       line_index,
    uint32_t       vert_pixel_index)
{
    jpege_scale_up_t  *p_scale_up = &(p_scale_info->scale.up);
    /*------------------------------------------------------------
     For rotation =0, it can fetch one line luma to buffer
    ------------------------------------------------------------*/
    uint32_t input_width = p_scale_info->vert_luma_fetch_width;
    uint32_t extended_width = p_scale_up->crop_extended_width;
    int32_t fetch_increment = p_scale_info->rotated_luma_fetch_increment;
    /*---------------------------------------------------------------
     For one output line, it needs line A-1, A, A+1, up to line A+2,
     And this is a 4 line buffer,
     plus there is 1 extended line at the top of the beginning
    ----------------------------------------------------------------*/
    uint32_t pixel_index = MOD4(vert_pixel_index + 3);
    uint32_t pixel_extend_right = p_scale_up->pixel_extend_right;
    uint32_t fetch_pixel_index;
    uint8_t  *p_src = p_scale_info->p_luma_input;
    uint8_t  *p_output = p_scale_info->p_input_luma_line_buffer->ptr;
    uint32_t  i, k = pixel_extend_right;

    /*------------------------------------------------------------
         Fetch from the beginning
    ------------------------------------------------------------*/
    if (line_index && ((vert_pixel_index + 2) >= p_scale_info->crop_height))
    {
       /*------------------------------------------------------------
         Fetch from the last row
        ------------------------------------------------------------*/
        fetch_pixel_index = p_scale_info->crop_height - 1;
        p_src += (int32_t)(fetch_pixel_index * p_scale_info->rotated_luma_src_increment);
        p_output += (pixel_index * extended_width);
    }
    else if (line_index)
    {
        fetch_pixel_index = vert_pixel_index + 2 ;
        p_src += (int32_t)(fetch_pixel_index * p_scale_info->rotated_luma_src_increment);
        p_output += (pixel_index * extended_width);
    }

    /*------------------------------------------------------------
      For non rotated cases, it fetch line by line
    ------------------------------------------------------------*/
    // Extend the first line
    // Extend the left
    *(p_output ++)= *p_src;
    // Fetch a line
    (void)STD_MEMMOVE(p_output, p_src, (input_width * sizeof(uint8_t)));
    p_output += input_width;
    // Extend the right
    while (k --)
    {
        *(p_output ++) = *(p_src + input_width - 1);
    }
    if (line_index == 0)
    {
        // Fill in (UPSCALE_POLYPHASE_FILTER_TAP = 4) lines buffer
        i = UPSCALE_POLYPHASE_FILTER_TAP - 1;
        while (i --)
        {
            // Extend the first line
            // Extend the left
            *(p_output ++)= *p_src;
            // Fetch a line
            (void)STD_MEMMOVE(p_output, p_src, (input_width * sizeof(uint8_t)));
            p_output += input_width;
            // Extend the right
            k = pixel_extend_right;
            while (k --)
            {
                *(p_output ++) = *(p_src + input_width - 1);
            }
            p_src += fetch_increment;
        }
    }
}

/*===========================================================================
FUNCTION        jpege_engine_sw_upscale_fetch_luma_to_lines_buffer_packed

DESCRIPTION     Fetch Luma from the input buffer to lines buffer

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/

static void jpege_engine_sw_upscale_fetch_luma_to_lines_buffer_packed(
    jpege_scale_t *p_scale_info,
    uint32_t       line_index,
    uint32_t       vert_pixel_index)
{
    jpege_scale_up_t  *p_scale_up = &(p_scale_info->scale.up);
    /*------------------------------------------------------------
     For rotation =0, it can fetch one line luma to buffer
    ------------------------------------------------------------*/
    uint32_t input_width = p_scale_info->vert_luma_fetch_width;
    uint32_t extended_width = p_scale_up->crop_extended_width;
    int32_t fetch_increment = p_scale_info->rotated_luma_fetch_increment;
    /*---------------------------------------------------------------
     For one output line, it needs line A-1, A, A+1, up to line A+2,
     And this is a 4 line buffer,
     plus there is 1 extended line at the top of the beginning
    ----------------------------------------------------------------*/
    uint32_t pixel_index = MOD4(vert_pixel_index + 3);
    uint32_t pixel_extend_right = p_scale_up->pixel_extend_right;
    uint32_t fetch_pixel_index;
    uint8_t  *p_src = p_scale_info->p_luma_input;
    uint8_t  *p_output = p_scale_info->p_input_luma_line_buffer->ptr;
    uint32_t  m, i, k = pixel_extend_right;

    /*------------------------------------------------------------
         Fetch from the beginning
    ------------------------------------------------------------*/
    if (line_index && ((vert_pixel_index + 2) >= p_scale_info->crop_height))
    {
        /*------------------------------------------------------------
         Fetch from the last row
        ------------------------------------------------------------*/
        fetch_pixel_index = p_scale_info->crop_height - 1;
        p_src += (int32_t)(fetch_pixel_index * p_scale_info->rotated_luma_src_increment);
        p_output += (pixel_index * extended_width);
    }
    else if (line_index)
    {
        fetch_pixel_index = vert_pixel_index + 2 ;
        p_src += (int32_t)(fetch_pixel_index * p_scale_info->rotated_luma_src_increment);
        p_output += (pixel_index * extended_width);
    }

    /*------------------------------------------------------------
      For non rotated cases, it fetch line by line
    ------------------------------------------------------------*/
    // Extend the first line
    // Extend the left
    *(p_output ++)= *p_src;
    // Fetch a line
    for( m = 0; m < input_width * 2; m+= 2)
    {
        *(p_output + m/2) = *(p_src + m);
    }
    p_output += input_width;
    // Extend the right
    while (k --)
    {
        *(p_output ++) = *(p_src + 2 * input_width - 2);
    }
    if (line_index == 0)
    {
        // Fill in (UPSCALE_POLYPHASE_FILTER_TAP = 4) lines buffer
        i = UPSCALE_POLYPHASE_FILTER_TAP - 1;
        while (i --)
        {
            // Extend the first line
            // Extend the left
            *(p_output ++)= *p_src;
            // Fetch a line
           for( m = 0; m < input_width * 2; m+= 2)
            {
                *(p_output + m/2) = *(p_src + m);
            }
            p_output += input_width;
            // Extend the right
            k = pixel_extend_right;
            while (k --)
            {
                *(p_output ++) = *(p_src + 2 * input_width - 2);
            }
            p_src += fetch_increment;
        }
    }
}

/*===========================================================================
FUNCTION        jpege_engine_sw_upscale_fetch_chroma_to_lines_buffer

DESCRIPTION     Fetch Chroma from the input buffer to lines buffer

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_upscale_fetch_chroma_to_lines_buffer(
    jpege_scale_t *p_scale_info,
    uint32_t       line_index,
    uint32_t       vert_pixel_index)
{
    jpege_scale_up_t  *p_scale_up = &(p_scale_info->scale.up);
    /*------------------------------------------------------------
     For rotation =0, it can fetch one line
    ------------------------------------------------------------*/
    uint32_t input_width = p_scale_info->vert_chroma_fetch_width * 2;
    uint32_t extended_width = p_scale_up->crop_chroma_extended_width * 2;
    int32_t fetch_increment = p_scale_info->rotated_chroma_fetch_increment;
    /*---------------------------------------------------------------
     For one output line, it needs line A-1, A, A+1, up to line A+2,
     And this is a 4 line buffer,
     plus there is 1 extended line at the top of the beginning
    ----------------------------------------------------------------*/
    uint32_t pixel_index = MOD4(vert_pixel_index + 3);
    uint32_t pixel_extend_right = p_scale_up->pixel_extend_right;
    uint32_t fetch_pixel_index;
    uint8_t  *p_src = p_scale_info->p_chroma_input;
    uint8_t  *p_output = p_scale_info->p_input_chroma_line_buffer->ptr;
    uint32_t i, k = pixel_extend_right;

    /*------------------------------------------------------------
         Fetch from the beginning
    ------------------------------------------------------------*/
    if (line_index && ((vert_pixel_index + 2) >= p_scale_info->crop_chroma_height))
    {
        /*------------------------------------------------------------
         Fetch from the last row
        ------------------------------------------------------------*/
        fetch_pixel_index = p_scale_info->crop_chroma_height - 1;
        p_src += (int32_t)(fetch_pixel_index * 2 * p_scale_info->rotated_chroma_src_increment);
        p_output += (pixel_index * extended_width);
    }
    else if (line_index)
    {
        fetch_pixel_index = vert_pixel_index + 2;
        p_src += (int32_t)(fetch_pixel_index * 2 * p_scale_info->rotated_chroma_src_increment);
        p_output += (pixel_index * extended_width);
    }

    /*------------------------------------------------------------
      For non rotated cases, it fetch line by line
    ------------------------------------------------------------*/
    // Extend the left
    *(p_output ++) = *p_src;
    *(p_output ++) = *(p_src + 1);
    (void)STD_MEMMOVE(p_output, p_src, (input_width * sizeof(uint8_t)));
    p_output += input_width;
    // Extend the right
    while (k --)
    {
        *(p_output ++) = *(p_src + input_width - 2);
        *(p_output ++) = *(p_src + input_width - 1);
    }
    if (line_index == 0)
    {
        // Fill in (UPSCALE_POLYPHASE_FILTER_TAP = 4) lines buffer
        i = UPSCALE_POLYPHASE_FILTER_TAP - 1;
        while (i --)
        {
            // Extend the left
            *(p_output ++) = *p_src;
            *(p_output ++) = *(p_src + 1);
            (void)STD_MEMMOVE(p_output, p_src, (input_width * sizeof(uint8_t)));
            p_output += input_width;
            // Extend the right
            k = pixel_extend_right;
            while (k --)
            {
                *(p_output ++) = *(p_src + input_width - 2);
                *(p_output ++) = *(p_src + input_width - 1);
            }
            p_src += fetch_increment * 2;
        }
    }
}

/*===========================================================================
FUNCTION        jpege_engine_sw_upscale_fetch_chroma_to_lines_buffer_packed

DESCRIPTION     Fetch Chroma from the input buffer to lines buffer

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_upscale_fetch_chroma_to_lines_buffer_packed(
    jpege_scale_t *p_scale_info,
    uint32_t       line_index,
    uint32_t       vert_pixel_index)
{
    jpege_scale_up_t  *p_scale_up = &(p_scale_info->scale.up);
    /*------------------------------------------------------------
     For rotation =0, it can fetch one line
    ------------------------------------------------------------*/
    uint32_t input_width = p_scale_info->vert_chroma_fetch_width * 2;
    uint32_t extended_width = p_scale_up->crop_chroma_extended_width * 2;
    int32_t fetch_increment = p_scale_info->rotated_chroma_fetch_increment;
    /*---------------------------------------------------------------
     For one output line, it needs line A-1, A, A+1, up to line A+2,
     And this is a 4 line buffer,
     plus there is 1 extended line at the top of the beginning
    ----------------------------------------------------------------*/
    uint32_t pixel_index = MOD4(vert_pixel_index + 3);
    uint32_t pixel_extend_right = p_scale_up->pixel_extend_right;
    uint32_t fetch_pixel_index;
    uint8_t  *p_src = p_scale_info->p_chroma_input;
    uint8_t  *p_output = p_scale_info->p_input_chroma_line_buffer->ptr;
    uint32_t  m, i, k = pixel_extend_right;


    /*------------------------------------------------------------
         Fetch from the beginning
    ------------------------------------------------------------*/
    if (line_index && ((vert_pixel_index + 2) >= p_scale_info->crop_chroma_height))
    {
        /*------------------------------------------------------------
         Fetch from the last row
        ------------------------------------------------------------*/
        fetch_pixel_index = p_scale_info->crop_chroma_height - 1;
        p_src += (int32_t)(fetch_pixel_index * 2 * p_scale_info->rotated_chroma_src_increment);
        p_output += (pixel_index * extended_width);
    }
    else if (line_index)
    {
        fetch_pixel_index = vert_pixel_index + 2;
        p_src += (int32_t)(fetch_pixel_index *  2 *p_scale_info->rotated_chroma_src_increment);
        p_output += (pixel_index * extended_width);
    }

    /*------------------------------------------------------------
      For non rotated cases, it fetch line by line
    ------------------------------------------------------------*/
    // Extend the left
    *(p_output ++) = *p_src;
    *(p_output ++) = *(p_src + 2);
    for( m = 0; m < input_width * 2; m+= 2)
    {
        *(p_output + m/2) = *(p_src + m);
    }

    p_output += input_width;
    // Extend the right
    while (k --)
    {
        *(p_output ++) = *(p_src + input_width * 2 - 4);
        *(p_output ++) = *(p_src + input_width * 2 - 2);
    }
    if (line_index == 0)
    {
        // Fill in (UPSCALE_POLYPHASE_FILTER_TAP = 4) lines buffer
        i = UPSCALE_POLYPHASE_FILTER_TAP - 1;
        while (i --)
        {
            // Extend the left
            *(p_output ++) = *p_src;
            *(p_output ++) = *(p_src + 2);
            for( m=0; m < input_width * 2; m+= 2)
            {
                *(p_output + m/2) = *(p_src + m);
            }
            p_output += input_width;
            // Extend the right
            k = pixel_extend_right;
            while (k --)
            {
                *(p_output ++) = *(p_src + input_width * 2 - 4);
                *(p_output ++) = *(p_src + input_width * 2 - 2);
            }
            p_src += fetch_increment * 2;
        }
    }
}

/*===========================================================================
FUNCTION        jpege_engine_sw_upscale_fetch_rotated_luma_to_lines_buffer

DESCRIPTION     Fetch rotated Luma from the input buffer to lines buffer

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_upscale_fetch_rotated_luma_to_lines_buffer(
    jpege_scale_t *p_scale_info,
    uint32_t       line_index,
    uint32_t       vert_pixel_index)
{
    jpege_scale_up_t  *p_scale_up = &(p_scale_info->scale.up);
    /*------------------------------------------------------------
     For rotation =0, it can fetch one line luma to buffer
    ------------------------------------------------------------*/
    uint32_t input_width = p_scale_info->vert_luma_fetch_width;
    uint32_t extended_width = p_scale_up->crop_extended_width;
    int32_t fetch_increment = p_scale_info->rotated_luma_fetch_increment;
    int32_t rotated_luma_src_increment = p_scale_info->rotated_luma_src_increment;
    /*---------------------------------------------------------------
     For one output line, it needs line A-1, A, A+1, up to line A+2,
     And this is a 4 line buffer,
     plus there is 1 extended line at the top of the beginning
    ----------------------------------------------------------------*/
    uint32_t pixel_index = MOD4(vert_pixel_index + 3);
    uint32_t pixel_extend_left = 1;
    uint32_t pixel_extend_right = p_scale_up->pixel_extend_right;
    uint32_t fetch_pixel_index;
    uint8_t  *p_src = p_scale_info->p_luma_input;
    uint8_t  *p_output = p_scale_info->p_input_luma_line_buffer->ptr;
    uint32_t i;

    if (line_index && ((vert_pixel_index + 2) >= p_scale_info->crop_height))
    {
        /*------------------------------------------------------------
         Fetch from the last row
        ------------------------------------------------------------*/
        fetch_pixel_index = p_scale_info->crop_height - 1;
        p_src += (int32_t)(fetch_pixel_index * rotated_luma_src_increment);
        p_output += (pixel_index * extended_width);
    }
    else if (line_index)
    {
        fetch_pixel_index = vert_pixel_index + 2 ;
        p_src += (int32_t)(fetch_pixel_index * rotated_luma_src_increment);
        p_output += (pixel_index * extended_width);
    }

    /*------------------------------------------------------------
      For rotated cases, it fetch pixel by pixel
    ------------------------------------------------------------*/
    jpege_engine_sw_copy_rotated_luma_lines(p_src,
                                            p_output,
                                            input_width,
                                            fetch_increment,
                                            pixel_extend_left,
                                            pixel_extend_right);
    if (line_index == 0)
    {
        p_output += extended_width;
        // Fill in (UPSCALE_POLYPHASE_FILTER_TAP = 4) lines buffer
        i = UPSCALE_POLYPHASE_FILTER_TAP - 1;
        while (i --)
        {
            jpege_engine_sw_copy_rotated_luma_lines(p_src,
                                                    p_output,
                                                    input_width,
                                                    fetch_increment,
                                                    pixel_extend_left,
                                                    pixel_extend_right);
            p_src += rotated_luma_src_increment;
            p_output += extended_width;
        }
    }
}

/*===========================================================================
FUNCTION        jpege_engine_sw_upscale_fetch_rotated_chroma_to_lines_buffer

DESCRIPTION     Fetch rotated Chroma from the input buffer to lines buffer

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_upscale_fetch_rotated_chroma_to_lines_buffer(
    jpege_scale_t *p_scale_info,
    uint32_t       line_index,
    uint32_t       vert_pixel_index)
{
    jpege_scale_up_t  *p_scale_up = &(p_scale_info->scale.up);
    /*------------------------------------------------------------
     For rotation =0, it can fetch one line
    ------------------------------------------------------------*/
    uint32_t input_width = p_scale_info->vert_chroma_fetch_width * 2;
    uint32_t extended_width = p_scale_up->crop_chroma_extended_width * 2;
    int32_t  fetch_increment = p_scale_info->rotated_chroma_fetch_increment;
    int32_t  rotated_chroma_src_increment = p_scale_info->rotated_chroma_src_increment;
    /*---------------------------------------------------------------
     For one output line, it needs line A-1, A, A+1, up to line A+2,
     And this is a 4 line buffer,
     plus there is 1 extended line at the top of the beginning
    ----------------------------------------------------------------*/
    uint32_t pixel_index = MOD4(vert_pixel_index + 3);
    uint32_t pixel_extend_left = 1;
    uint32_t pixel_extend_right = p_scale_up->pixel_extend_right;
    uint32_t fetch_pixel_index;
    uint8_t *p_src = p_scale_info->p_chroma_input;
    uint8_t *p_output = p_scale_info->p_input_chroma_line_buffer->ptr;
    uint32_t i;

    if (line_index && ((vert_pixel_index + 2) >= p_scale_info->crop_chroma_height))
    {
        /*------------------------------------------------------------
         Fetch from the last row
        ------------------------------------------------------------*/
        fetch_pixel_index = p_scale_info->crop_chroma_height - 1;
        p_src += (int32_t)(fetch_pixel_index * 2 * rotated_chroma_src_increment);
        p_output += (pixel_index * extended_width);
    }
    else if (line_index)
    {
        fetch_pixel_index = vert_pixel_index + 2;
        p_src += (int32_t)(fetch_pixel_index * 2 * rotated_chroma_src_increment);
        p_output += (pixel_index * extended_width);
    }

    /*------------------------------------------------------------
      For rotated cases, it fetch pixel by pixel
    ------------------------------------------------------------*/
    jpege_engine_sw_copy_rotated_chroma_lines(p_src,
                                              p_output,
                                              input_width,
                                              fetch_increment,
                                              pixel_extend_left,
                                              pixel_extend_right);
    if (line_index == 0)
    {
        p_output += extended_width;
        // Fill in (UPSCALE_POLYPHASE_FILTER_TAP = 4) lines buffer
        i = UPSCALE_POLYPHASE_FILTER_TAP - 1;
        while (i --)
        {
            jpege_engine_sw_copy_rotated_chroma_lines(p_src,
                                                      p_output,
                                                      input_width,
                                                      fetch_increment,
                                                      pixel_extend_left,
                                                      pixel_extend_right);
            p_src += 2 * rotated_chroma_src_increment;
            p_output += extended_width;
        }
    }
}
/*===========================================================================
FUNCTION        jpege_engine_sw_upscale_fetch_rotated_chroma_to_lines_buffer

DESCRIPTION     Fetch rotated Chroma from the input buffer to lines buffer

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_upscale_fetch_rotated_chroma_to_lines_buffer_packed(
    jpege_scale_t *p_scale_info,
    uint32_t       line_index,
    uint32_t       vert_pixel_index)
{
    jpege_scale_up_t  *p_scale_up = &(p_scale_info->scale.up);
    //------------------------------------------------------------
    // For rotation =0, it can fetch one line
    //------------------------------------------------------------
    uint32_t input_width = p_scale_info->vert_chroma_fetch_width * 2;
    uint32_t extended_width = p_scale_up->crop_chroma_extended_width * 2;
    int32_t  fetch_increment = p_scale_info->rotated_chroma_fetch_increment;
    int32_t  rotated_chroma_src_increment = p_scale_info->rotated_chroma_src_increment;
    //---------------------------------------------------------------
    // For one output line, it needs line A-1, A, A+1, up to line A+2,
    // And this is a 4 line buffer,
    // plus there is 1 extended line at the top of the beginning
    //----------------------------------------------------------------
    uint32_t pixel_index = MOD4(vert_pixel_index + 3);
    uint32_t pixel_extend_left = 1;
    uint32_t pixel_extend_right = p_scale_up->pixel_extend_right;
    uint32_t fetch_pixel_index;
    uint8_t *p_src = p_scale_info->p_chroma_input;
    uint8_t *p_output = p_scale_info->p_input_chroma_line_buffer->ptr;
    uint32_t i;

    if (line_index && ((vert_pixel_index + 2) >= p_scale_info->crop_chroma_height))
    {
        //------------------------------------------------------------
        //Fetch from the last row
        //------------------------------------------------------------
        fetch_pixel_index = p_scale_info->crop_chroma_height - 1;
        p_src += (int32_t)(fetch_pixel_index * 2 * rotated_chroma_src_increment);
        p_output += (pixel_index * extended_width);
    }
    else if (line_index)
    {
        fetch_pixel_index = vert_pixel_index + 2;
        p_src += (int32_t)(fetch_pixel_index * 2 * rotated_chroma_src_increment);
        p_output += (pixel_index * extended_width);
    }

    //------------------------------------------------------------
    //  For rotated cases, it fetch pixel by pixel
    //------------------------------------------------------------
    jpege_engine_sw_copy_rotated_chroma_lines_packed(p_src,
                                                     p_output,
                                                     input_width,
                                                     fetch_increment,
                                                     pixel_extend_left,
                                                     pixel_extend_right);
    if (line_index == 0)
    {
        p_output += extended_width;
        // Fill in (UPSCALE_POLYPHASE_FILTER_TAP = 4) lines buffer
        i = UPSCALE_POLYPHASE_FILTER_TAP - 1;
        while (i --)
        {
            jpege_engine_sw_copy_rotated_chroma_lines_packed(p_src,
                                                             p_output,
                                                             input_width,
                                                             fetch_increment,
                                                             pixel_extend_left,
                                                             pixel_extend_right);
            p_src += 2 * rotated_chroma_src_increment;
            p_output += extended_width;
        }
    }
}
