/*******************************************************************************
* Copyright (c) 2008-2012, 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

#ifndef _JPEGE_ENGINE_SW_SCALE_H
#define _JPEGE_ENGINE_SW_SCALE_H
/*========================================================================


*//** @file jpege_engine_sw_scale.h

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2010-2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/still/jpeg2/v2/latest/src/jpege_engine_sw_scale.h#2 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
10/16/14   vgaur   Added support for multi threaded fastCV
03/01/12  csriniva Added support for YUY2 and UYVY inputs
03/08/11   mingy   Updated included header file list.
04/06/10   staceyw Added structure jpege_scale_t, and jpege_scale_up_t for
                   upscale and jpege_scale_down_t for downscale.
                   Move internal buffer from jpege_engine_sw_t.
                   Added clean up function for internal buffers.
02/22/10   staceyw Created file

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */

#include "os_int.h"
#include "jpeg_buffer_private.h"
#include "jpeg_common_private.h"

typedef struct jpege_scale_t jpege_scale_t;
typedef void (* jpege_engine_sw_scale_mcu_lines_t)(
                 struct jpege_scale_t  *p_scale_info,
                 uint32_t               rotated_mcu_height);
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define LUMA_BLOCK_SIZE 64
#define LUMA_BLOCK_SIZE_DIGIT 6
#define LUMA_BLOCK_HEIGHT 8
#define BLOCK_HEIGHT_DIGIT 3

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/**
 * This is the implementation of downscale
*/
typedef struct
{
    // Veritcal M/N accumulator, count, step and index
    uint16_t  *p_vert_luma_accum;  // luma accumulator
    uint16_t  *p_vert_chroma_accum;// chroma accumulator
    uint32_t   vert_luma_count;    // luma count
    uint32_t   vert_chroma_count;  // chroma count
    uint32_t   vert_luma_step;     // luma step
    uint32_t   vert_chroma_step;   // chroma step
    uint32_t   vert_luma_row_index;// vertical luma row index
    uint32_t   vert_chroma_row_index;
                                 // vertical chroma row index
    uint32_t   luma_input_seg_height;   // Luma Input segment heght for fastCV
    uint32_t   chroma_input_seg_height;   //Chroma Input segment heght for fastCV
} jpege_scale_down_t;

/**
 * This is the implementation of upscale
*/
typedef struct
{
    //32-polyphase 4-tap filter specs
    uint32_t hori_init;            // Init for horizontal scale
    uint32_t hori_step;            // Step for horizontal scale
    uint32_t vert_init;            // Init for vertical scale
    uint32_t vert_step;            // Step for vertical scale
    uint32_t vert_luma_accum;
                   // Accumulator for vertical scale
    uint32_t vert_chroma_accum;
                   // Accumulator for vertical chroma scale

    uint32_t vert_luma_last_index; // Index for m_pInputLumaLineBuffer
    uint32_t vert_chroma_last_index;
                   // Index for m_pInputChromaLineBuffer
    uint32_t vert_luma_row_index;  // Row Index for next upscale
    uint32_t vert_chroma_row_index;
                   // Row Index for next chroma upscale
    uint32_t crop_extended_width;  // Cropped extended width
    uint32_t crop_chroma_extended_width;
                   // Cropped extended chroma width
    // pixel extension
    uint32_t pixel_extend_left;    // Number of pixel extended left
    uint32_t pixel_extend_right;   // Number of pixel extended right

} jpege_scale_up_t;

/**
 * This is the implementation of scale
*/
struct jpege_scale_t
{
    struct
    {
        jpege_scale_up_t    up;    // upscale

        jpege_scale_down_t  down;  // downscale

    } scale;

    // FastCV enable flag
    uint8_t  fastCV_enable;

    uint32_t piece_height;
    uint8_t  *fastCV_buffer;

    //crop and output Image sizes
    uint32_t crop_width;           // Cropped width
    uint32_t crop_height;          // Cropped height
    uint32_t output_width;         // Output width
    uint32_t output_height;        // Output height

    //crop and output chroma sizes
    uint32_t crop_chroma_width;    // Cropped Chroma width
    uint32_t crop_chroma_height;   // Cropped Chroma height
    uint32_t output_chroma_width;  // Output chroma width
    uint32_t output_chroma_height; // Output chroma height

    uint32_t vert_luma_row_index;  // vertical luma row index
    uint32_t vert_chroma_row_index;// vertical chroma row index
    uint32_t vert_luma_output_height;
    uint32_t vert_chroma_output_height;

    //rotated information
    int32_t  rotated_luma_src_increment;
                                   // Rotated Luma Source Increment
    int32_t  rotated_chroma_src_increment;
                                   // Rotated Chroma Source Increment
    int32_t  rotated_luma_fetch_increment;
                                   // Rotated Luma Fetch Increment
    int32_t  rotated_chroma_fetch_increment;
                                   // Rotated chroma Fetch Increment
    uint32_t vert_luma_fetch_width;
                                   // Luma Fetch Width without rotation
    uint32_t vert_chroma_fetch_width;
                                   // Chroma Fetch Width without rotation

    uint8_t  *p_luma_input;        // Pointer to luma Input
    uint8_t  *p_chroma_input;      // Pointer to chroma Input
    uint8_t  *p_luma_input_start;  // Pointer to luma Input (backup)
    uint8_t  *p_chroma_input_start;// Pointer to chroma Input (backup)

    int32_t  fetched_luma_blocks;  // Fetched blocks in line buffer
    uint8_t  buffer_ratio;         // For different subsampling
    uint8_t  crop_flag;            // Indicate if crop
    uint8_t  upscale_flag;         // Indicate if upscale
    uint8_t  downscale_flag;       // Indicate if downscale

    // Internal buffer containing cropped and rotated luma lines
    jpeg_buf_t  *p_input_luma_line_buffer;
    // Internal buffer containing cropped and rotated chroma lines
    jpeg_buf_t  *p_input_chroma_line_buffer;
    // Vertical scale internal buffer for one luma or chroma line
    jpeg_buf_t  *p_vertical_scaled_buffer;
    // Output fully scale buffer of luma and chroma in MCU height
    jpeg_buf_t  *p_inline_scaled_buffer;

    // scale mcu lines function pointer
    jpege_engine_sw_scale_mcu_lines_t jpege_engine_sw_scale_mcu_lines;

    uint8_t      isPacked;
};

/* =======================================================================
**                          Function Declarations
** ======================================================================= */
/**
 * clean up internal scale buffers
 */
void jpege_engine_sw_scale_destroy(
    jpege_scale_t      *p_scale);

#endif /*  _JPEGE_ENGINE_SW_SCALE_H */
