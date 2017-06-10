/*******************************************************************************
* Copyright (c) 2008-2012, 2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

/*========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/still/jpeg2/v2/latest/src/jpege_engine_sw_scale.c#3 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
10/16/14   vgaur   Added support for multi threaded fastCV
03/25/14  revathys Added stride support.
03/21/12  csriniva Pointer Out of buffer Issue in Horizontal Direction
03/01/12  csriniva Added support for YUY2 and UYVY inputs
04/06/10   staceyw Added crop and scale set-up and configure functions.
                   Added fetch scaled mcu function, the jpeg encoder software
                   engine calls it to get mcu in the scenario of scale cases.
02/22/10   staceyw Created file

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include <stdlib.h>
#include "jpege_engine_sw.h"
#include "jpeglog.h"

/* -----------------------------------------------------------------------
** Constant / Define Declarations / Macro Definitions
** ----------------------------------------------------------------------- */
#define UPSCALE_MAX_FACTOR_DIGIT  3   // support up to 8 upscale

#define DOWNSCALE_MAX_FACTOR_DIGIT  4 // support up to 1/16 downscale

/* Static constants */
static const int16_t zigzag_table[JBLOCK_SIZE] =
{
    0,   1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* functions related with in-line scale */
/**
 * scale congifuration
 */
int jpege_engine_sw_scale_configure(
    jpege_engine_sw_t  *p_engine,
    jpege_img_cfg_t    *p_config,
    jpege_img_data_t   *p_source);
static int jpege_engine_sw_scale_crop_setup_fastCV(
    jpege_engine_sw_t  *p_engine,
    jpege_img_cfg_t    *p_config,
    jpege_img_data_t   *p_source);
static int jpege_engine_sw_scale_crop_setup(
    jpege_engine_sw_t  *p_engine,
    jpege_img_cfg_t    *p_config,
    jpege_img_data_t   *p_source);
static int jpege_engine_sw_scale_crop_setup_packed(
    jpege_engine_sw_t  *p_engine,
    jpege_img_cfg_t    *p_config,
    jpege_img_data_t   *p_source);
static void jpege_engine_sw_scale_configure_tables(
    jpege_engine_sw_t  *p_engine,
    jpege_img_cfg_t    *p_config);
// fetch scaled mcu and do dct
static void jpege_engine_sw_fetch_dct_scaled_mcu (
    jpege_engine_sw_t *p_engine,
    int16_t *curr_mcu_luma_dct_output,
    int16_t *curr_mcu_chroma_dct_output);
static void jpege_engine_sw_fetch_dct_scaled_block_partial_luma(
    jpege_scale_t *p_upscale,
    const uint8_t *p_luma_block_origin,
    uint32_t       hori_bound,
    uint32_t       vert_bound,
    int16_t       *p_dct_output);
static void jpege_engine_sw_fetch_dct_scaled_block_partial_chroma(
    jpege_engine_sw_t *p_engine,
    const uint8_t     *p_chroma_block_origin,
    uint32_t           hori_bound,
    uint32_t           vert_bound,
    int16_t           *p_dct_output);
// copy rotated luma/chroma
void jpege_engine_sw_copy_rotated_luma_lines(
    uint8_t *p_src,
    uint8_t *p_output,
    uint32_t input_width,
    int32_t  fetch_increment,
    uint32_t pixel_extend_left,
    uint32_t pixel_extend_right);
void jpege_engine_sw_copy_rotated_chroma_lines(
    uint8_t *p_src,
    uint8_t *p_output,
    uint32_t input_width,
    int32_t  fetch_increment,
    uint32_t pixel_extend_left,
    uint32_t pixel_extend_right);
void jpege_engine_sw_copy_rotated_chroma_lines_packed(
    uint8_t *p_src,
    uint8_t *p_output,
    uint32_t input_width,
    int32_t  fetch_increment,
    uint32_t pixel_extend_left,
    uint32_t pixel_extend_right);
/* Extern functions from other files within the same engine */
extern void jpege_engine_sw_fetch_dct_block_luma(
    const uint8_t *p_luma_block_origin,
    int16_t       *p_dct_output,
    uint32_t       luma_width);
extern void jpege_engine_sw_fetch_dct_block_chroma(
    const uint8_t  *p_chroma_block_origin,
    int16_t        *p_dct_output,
    uint32_t        chroma_width,
    input_format_t  input_format);
extern void jpege_engine_sw_fetch_dct_block_luma_packed(
    const uint8_t *p_luma_block_origin,
    int16_t       *p_dct_output,
    uint32_t       luma_width);
extern void jpege_engine_sw_fetch_dct_block_chroma_packed(
    const uint8_t  *p_chroma_block_origin,
    int16_t        *p_dct_output,
    uint32_t        chroma_width,
    input_format_t  input_format);

extern void jpege_engine_sw_fdct_block (const uint8_t *pixel_domain_block,
                                        DCTELEM       *frequency_domain_block);
// upscale/downscale mcu lines functions
extern void jpege_engine_sw_upscale_mcu_lines(
    jpege_scale_t  *p_scale_info,
    uint32_t  rotated_mcu_height);
extern void jpege_engine_sw_downscale_mcu_lines(
    jpege_scale_t  *p_scale_info,
    uint32_t  rotated_mcu_height);
extern void jpege_engine_sw_downscale_mcu_lines_packed(
    jpege_scale_t  *p_scale_info,
    uint32_t  rotated_mcu_height);
extern void jpege_engine_sw_downscale_rotated_mcu_lines(
    jpege_scale_t  *p_scale_info,
    uint32_t  rotated_mcu_height);
extern void jpege_engine_sw_downscale_rotated_mcu_lines(
    jpege_scale_t  *p_scale_info,
    uint32_t  rotated_mcu_height);
extern int jpege_engine_sw_upscale_configure_filter(
    jpege_scale_t *p_scale_info,
    uint32_t       rotation);
extern int jpege_engine_sw_downscale_configure_filter(
    jpege_scale_t *p_scale_info);
extern int jpege_engine_sw_downscale_configure_filter_packed(
    jpege_scale_t *p_scale_info);
extern int jpege_engine_sw_downscale_configure_filter_fastCV(
    jpege_scale_t *p_scale_info);
/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
**                          Function Definitions
** ======================================================================= */

/*===========================================================================
FUNCTION        jpege_engine_sw_scale_configure

DESCRIPTION     JpegEncode Configure Function for crop and scale

DEPENDENCIES    Called for crop and scale cases

RETURN VALUE    INT

SIDE EFFECTS    Set fetch mcu function for crop and scale cases
===========================================================================*/
int jpege_engine_sw_scale_configure(jpege_engine_sw_t  *p_engine,
                                    jpege_img_cfg_t    *p_config,
                                    jpege_img_data_t   *p_source)
{
    int       rc = JPEGERR_SUCCESS;
    jpege_scale_t *p_scale_info = &p_engine->jpege_scale;

    /*-----------------------------------------------------------------------
      1. Initialize scale information
    -----------------------------------------------------------------------*/
    //init scale information
    p_scale_info->fetched_luma_blocks = 0;
    p_scale_info->crop_flag = false;
    p_scale_info->upscale_flag = false;
    p_scale_info->downscale_flag = false;

    /* in-line crop and scale set up */
    if(p_engine->jpege_scale.isPacked==0)
    {
        if(p_scale_info->fastCV_enable)
            rc = jpege_engine_sw_scale_crop_setup_fastCV(p_engine,
                                              p_config,
                                              p_source);
        else
            rc = jpege_engine_sw_scale_crop_setup(p_engine,
                                              p_config,
                                              p_source);
    }
    else
    {
        rc = jpege_engine_sw_scale_crop_setup_packed(p_engine,
                                                     p_config,
                                                     p_source);
    }

    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_engine_sw_scale_configure failed\n");
        return rc;
    }

    // if none of upscale, downscale or crop,
    // return
    if (!(p_scale_info->upscale_flag ||
          p_scale_info->downscale_flag ||
          p_scale_info->crop_flag))
    {
         JPEG_DBG_LOW("jpege_engine_sw_scale_configure: not crop or scale\n");
         return JPEGERR_SUCCESS;
    }

    /* in-line crop and scale set-up huffman and quant tables*/
    jpege_engine_sw_scale_configure_tables(p_engine,
                                           p_config);

    /* fetchdct mcu function for crop and scale cases*/
    p_engine->jpege_engine_sw_fetch_dct_mcu_func = &jpege_engine_sw_fetch_dct_scaled_mcu;

    if (p_scale_info->upscale_flag )
    {
        p_scale_info->jpege_engine_sw_scale_mcu_lines = &jpege_engine_sw_upscale_mcu_lines;

        /* Configue 4-tap 32 polyphase Filter for in-line crop and upscale */
        rc = jpege_engine_sw_upscale_configure_filter(&(p_engine->jpege_scale),
                                                      p_engine->rotation);
    }
    else if (p_scale_info->downscale_flag ||
             p_scale_info->crop_flag)
    {
        /*------------------------------------------------------------
          For non-rotation and rotation case,
          set up scale mcu function pointer
        ------------------------------------------------------------*/
        if(p_engine->jpege_scale.isPacked==0)
        {
            if (p_engine->rotation == 0)
            {
                p_scale_info->jpege_engine_sw_scale_mcu_lines = &jpege_engine_sw_downscale_mcu_lines;
            }
            else
            {
                p_scale_info->jpege_engine_sw_scale_mcu_lines = &jpege_engine_sw_downscale_rotated_mcu_lines;
            }
        }
        else
        {
            if (p_engine->rotation == 0)
            {
                p_scale_info->jpege_engine_sw_scale_mcu_lines = &jpege_engine_sw_downscale_mcu_lines_packed;
            }
            else
            {
                p_scale_info->jpege_engine_sw_scale_mcu_lines = &jpege_engine_sw_downscale_rotated_mcu_lines;
            }
        }

        /* Configue M/N Filter for crop and downscale */
        if(p_scale_info->fastCV_enable) {
            rc = jpege_engine_sw_downscale_configure_filter_fastCV(&(p_engine->jpege_scale));
        }
        else {
            rc = jpege_engine_sw_downscale_configure_filter(&(p_engine->jpege_scale));
        }
    }

    return rc;
}

static int jpege_engine_sw_scale_crop_setup_fastCV(jpege_engine_sw_t  *p_engine,
                                            jpege_img_cfg_t    *p_config,
                                            jpege_img_data_t   *p_source)
{
    int       rc;
    uint32_t  width, height, chroma_width, chroma_height;
    uint32_t  crop_width, crop_height, chroma_crop_width, chroma_crop_height;
    uint32_t  rotated_crop_width, rotated_crop_chroma_width;
    uint32_t  output_chroma_width, output_chroma_height;
    uint32_t  crop_top, crop_bottom, crop_left, crop_right;
    uint32_t  chroma_crop_top, chroma_crop_bottom, chroma_crop_left, chroma_crop_right;
    int32_t   rotated_ratio, buffer_size, buffer_ratio;
    jpege_scale_t *p_scale_info = &p_engine->jpege_scale;
    jpege_scale_cfg_t scale_cfg = p_config->scale_cfg;
    jpege_stride_cfg_t stride_cfg = p_engine->stride_cfg;

    //set image sizes
    width = p_source->width;
    height = p_source->height;

    //set corp sizes
    crop_width = scale_cfg.input_width;
    crop_height = scale_cfg.input_height;
    crop_left = scale_cfg.h_offset;
    crop_right = scale_cfg.h_offset
                 + scale_cfg.input_width - 1;
    crop_top = scale_cfg.v_offset;
    crop_bottom = scale_cfg.v_offset
                  + scale_cfg.input_height - 1;

    /*-----------------------------------------------------------------------
      Initialize crop and scale flag
    -----------------------------------------------------------------------*/
    if ((scale_cfg.h_offset != 0 ||
         scale_cfg.v_offset != 0 ||
        ((scale_cfg.h_offset + scale_cfg.input_width) < p_source->width) ||
        ((scale_cfg.v_offset + scale_cfg.input_height) < p_source->height)) &&
       (crop_width != p_source->width ||
        crop_height != p_source->height))
    {
        p_scale_info->crop_flag = true;
    }

    if  ((scale_cfg.input_width < scale_cfg.output_width) ||
         (scale_cfg.input_height < scale_cfg.output_height))
    {
        // validate upscale ratio
        // support up to 8x upscale
        if ((scale_cfg.output_width > (scale_cfg.input_width << UPSCALE_MAX_FACTOR_DIGIT)) ||
            (scale_cfg.output_height > (scale_cfg.input_height << UPSCALE_MAX_FACTOR_DIGIT)))
        {
            JPEG_DBG_ERROR("jpege_engine_sw_scale_crop_setup: support up to %d upscale: \
                          (input width, input height)(%d, %d)  (output width,output height)(%d, %d)\n",
                          (1 << UPSCALE_MAX_FACTOR_DIGIT), scale_cfg.input_width, scale_cfg.input_height,
                          scale_cfg.output_width, scale_cfg.output_height);
            return JPEGERR_EBADPARM;
        }

        p_scale_info->upscale_flag = true;
    }

    if  ((scale_cfg.input_width > scale_cfg.output_width) ||
         (scale_cfg.input_height > scale_cfg.output_height))
    {
        // validate downscale ratio
        // support up to 1/8 downscale
        if ((scale_cfg.output_width < (scale_cfg.input_width >> DOWNSCALE_MAX_FACTOR_DIGIT)) ||
            (scale_cfg.output_height < (scale_cfg.input_height >> DOWNSCALE_MAX_FACTOR_DIGIT)))
        {
            JPEG_DBG_ERROR("jpege_engine_sw_scale_crop_setup: support up to 1/%d downscale: \
                         (input width, input height)(%d, %d)  (output width,output height)(%d, %d)\n",
                         (1 << UPSCALE_MAX_FACTOR_DIGIT), scale_cfg.input_width, scale_cfg.input_height,
                         scale_cfg.output_width, scale_cfg.output_height);
            return JPEGERR_EBADPARM;
        }

        p_scale_info->downscale_flag = true;
    }

    // if none of upscale, downscale or crop,
    // return
    if (!(p_scale_info->upscale_flag ||
          p_scale_info->downscale_flag ||
          p_scale_info->crop_flag))
    {
        return JPEGERR_SUCCESS;
    }

    /*-----------------------------------------------------------------------
      Calculate Chroma Width and Height
      and crop infomation for chroma
      and prepare to calculate the scale buffer size
    -----------------------------------------------------------------------*/
    switch (p_engine->subsampling)
    {
        case JPEG_H2V2:
        {
            chroma_width = (width + 1) >> 1;
            chroma_height = (height + 1) >> 1;
            chroma_crop_width = (crop_width + 1) >> 1;
            chroma_crop_height = (crop_height + 1) >> 1;
            output_chroma_width = (scale_cfg.output_width + 1) >> 1;
            output_chroma_height = (scale_cfg.output_height + 1) >> 1;

            chroma_crop_left = crop_left >> 1;  // one half
            chroma_crop_right = crop_right >> 1; // one half
            chroma_crop_top = crop_top >> 1;
            chroma_crop_bottom = crop_bottom >> 1;
            buffer_ratio = 2;
            rotated_ratio = 2;
            break;
        }
    default:
        JPEG_DBG_ERROR("jpege_engine_sw_up_scale_configure: invalid jpeg subsampling: %d\n", p_engine->subsampling);
        return JPEGERR_EBADPARM;
    }

    // setup relative step
    p_engine->horiIncrement_luma = 1;
    p_engine->vertIncrement_luma = p_engine->lumaWidth;
    p_engine->horiIncrement_chroma = 2;
    p_engine->vertIncrement_chroma = 2 * p_engine->chromaWidth;

    // crop width, output width and input width
    rotated_crop_width = crop_width;
    p_scale_info->crop_width = crop_width;
    p_scale_info->crop_height = crop_height;
    p_scale_info->output_width = scale_cfg.output_width;
    p_scale_info->output_height = scale_cfg.output_height;
    // chroma crop width, output width
    rotated_crop_chroma_width = chroma_crop_width;
    p_scale_info->crop_chroma_width = chroma_crop_width;
    p_scale_info->crop_chroma_height = chroma_crop_height;
    p_scale_info->output_chroma_width =  output_chroma_width;
    p_scale_info->output_chroma_height = output_chroma_height;

    // rotated fetch increment from input to line buffer
    //In case of stride enabled, fetch increment is configured to stride set by user. In case of stride disabled, it is set to the width.
    p_scale_info->rotated_luma_fetch_increment = stride_cfg.input_luma_stride;
    p_scale_info->rotated_luma_src_increment = stride_cfg.input_luma_stride;
    p_scale_info->rotated_chroma_fetch_increment
        = stride_cfg.input_chroma_stride / rotated_ratio;
    p_scale_info->rotated_chroma_src_increment
        = stride_cfg.input_chroma_stride / rotated_ratio;

    p_scale_info->vert_luma_fetch_width = rotated_crop_width;
    p_scale_info->vert_chroma_fetch_width = rotated_crop_chroma_width;

    // Initialize input Y and CbCr/CrCb pointers
    p_scale_info->p_luma_input = (unsigned char *)((p_source->p_fragments[0].color.yuv.luma_buf)->ptr) +
                             ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->start_offset;
    p_scale_info->p_chroma_input = (unsigned char *)((p_source->p_fragments[0].color.yuv.chroma_buf)->ptr) +
                             ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->start_offset;

    // Set up input pointer to luma and chroma
    //If stride is enabled, input pointer is calculated using input stride.If stride is disabled, it is calculated using input width.
    if (p_scale_info->crop_flag == true)
    {
        // crop cases
        p_scale_info->p_luma_input += stride_cfg.input_luma_stride * crop_top + crop_left;
        p_scale_info->p_chroma_input += stride_cfg.input_chroma_stride * chroma_crop_top
                                                           + 2 * chroma_crop_left;
    }
    p_scale_info->p_luma_input_start = p_scale_info->p_luma_input;
    p_scale_info->p_chroma_input_start = p_scale_info->p_chroma_input;
    return JPEGERR_SUCCESS;
}

/*===========================================================================
FUNCTION        jpege_engine_sw_scale_crop_setup

DESCRIPTION     in-line scalar set up for crop and scale

DEPENDENCIES    None

RETURN VALUE    INT

SIDE EFFECTS    None
===========================================================================*/
static int jpege_engine_sw_scale_crop_setup(jpege_engine_sw_t  *p_engine,
                                            jpege_img_cfg_t    *p_config,
                                            jpege_img_data_t   *p_source)
{
    int       rc;
    uint32_t  width, height, chroma_width, chroma_height;
    uint32_t  crop_width, crop_height, chroma_crop_width, chroma_crop_height;
    uint32_t  rotated_crop_width, rotated_crop_chroma_width;
    uint32_t  output_chroma_width, output_chroma_height;
    uint32_t  crop_top, crop_bottom, crop_left, crop_right;
    uint32_t  chroma_crop_top, chroma_crop_bottom, chroma_crop_left, chroma_crop_right;
    int32_t   rotated_ratio, buffer_size, buffer_ratio;
    jpege_scale_t *p_scale_info = &p_engine->jpege_scale;
    jpege_scale_cfg_t scale_cfg = p_config->scale_cfg;
    jpege_stride_cfg_t stride_cfg = p_engine->stride_cfg;

    //set image sizes
    width = p_source->width;
    height = p_source->height;

    //set corp sizes
    crop_width = scale_cfg.input_width;
    crop_height = scale_cfg.input_height;
    crop_left = scale_cfg.h_offset;
    crop_right = scale_cfg.h_offset
                 + scale_cfg.input_width - 1;
    crop_top = scale_cfg.v_offset;
    crop_bottom = scale_cfg.v_offset
                  + scale_cfg.input_height - 1;

    /*-----------------------------------------------------------------------
      Initialize crop and scale flag
    -----------------------------------------------------------------------*/
    if ((scale_cfg.h_offset != 0 ||
         scale_cfg.v_offset != 0 ||
        ((scale_cfg.h_offset + scale_cfg.input_width) < p_source->width) ||
        ((scale_cfg.v_offset + scale_cfg.input_height) < p_source->height)) &&
       (crop_width != p_source->width ||
        crop_height != p_source->height))
    {
        p_scale_info->crop_flag = true;
    }

    if  ((scale_cfg.input_width < scale_cfg.output_width) ||
         (scale_cfg.input_height < scale_cfg.output_height))
    {
        // validate upscale ratio
        // support up to 8x upscale
        if ((scale_cfg.output_width > (scale_cfg.input_width << UPSCALE_MAX_FACTOR_DIGIT)) ||
            (scale_cfg.output_height > (scale_cfg.input_height << UPSCALE_MAX_FACTOR_DIGIT)))
        {
            JPEG_DBG_ERROR("jpege_engine_sw_scale_crop_setup: support up to %d upscale: \
                          (input width, input height)(%d, %d)  (output width,output height)(%d, %d)\n",
                          (1 << UPSCALE_MAX_FACTOR_DIGIT), scale_cfg.input_width, scale_cfg.input_height,
                          scale_cfg.output_width, scale_cfg.output_height);
            return JPEGERR_EBADPARM;
        }

        p_scale_info->upscale_flag = true;
    }

    if  ((scale_cfg.input_width > scale_cfg.output_width) ||
         (scale_cfg.input_height > scale_cfg.output_height))
    {
        // validate downscale ratio
        // support up to 1/8 downscale
        if ((scale_cfg.output_width < (scale_cfg.input_width >> DOWNSCALE_MAX_FACTOR_DIGIT)) ||
            (scale_cfg.output_height < (scale_cfg.input_height >> DOWNSCALE_MAX_FACTOR_DIGIT)))
        {
            JPEG_DBG_ERROR("jpege_engine_sw_scale_crop_setup: support up to 1/%d downscale: \
                         (input width, input height)(%d, %d)  (output width,output height)(%d, %d)\n",
                         (1 << UPSCALE_MAX_FACTOR_DIGIT), scale_cfg.input_width, scale_cfg.input_height,
                         scale_cfg.output_width, scale_cfg.output_height);
            return JPEGERR_EBADPARM;
        }

        p_scale_info->downscale_flag = true;
    }

    // if none of upscale, downscale or crop,
    // return
    if (!(p_scale_info->upscale_flag ||
          p_scale_info->downscale_flag ||
          p_scale_info->crop_flag))
    {
        return JPEGERR_SUCCESS;
    }

    /*-----------------------------------------------------------------------
      Calculate Chroma Width and Height
      and crop infomation for chroma
      and prepare to calculate the scale buffer size
    -----------------------------------------------------------------------*/
    switch (p_engine->subsampling)
    {
        case JPEG_H1V1:
        {
            chroma_width = width;
            chroma_height = height;
            chroma_crop_width = crop_width;
            chroma_crop_height = crop_height;
            output_chroma_width = scale_cfg.output_width;
            output_chroma_height = scale_cfg.output_height;
            chroma_crop_left = crop_left;
            chroma_crop_right = crop_right;
            chroma_crop_top = crop_top;
            chroma_crop_bottom = crop_bottom;
            buffer_ratio = 3;
            rotated_ratio = 1;
            break;
        }
        case JPEG_H1V2:
        {
            chroma_width = width;
            chroma_height = (height + 1) >> 1;
            chroma_crop_width = crop_width;
            chroma_crop_height = (crop_height + 1) >> 1;
            output_chroma_width = scale_cfg.output_width;
            output_chroma_height = (scale_cfg.output_height + 1) >> 1;
            chroma_crop_left = crop_left;
            chroma_crop_right = crop_right;
            chroma_crop_top = (crop_top + 1) >> 1; // one half
            chroma_crop_bottom = (crop_bottom + 1) >> 1; // one half
            buffer_ratio = 2;
            rotated_ratio = 1;
            break;
        }
        case JPEG_H2V1:
        {
            chroma_width = (width + 1) >> 1;
            chroma_height = height;
            chroma_crop_width = (crop_width + 1) >> 1;
            chroma_crop_height = crop_height;

            output_chroma_width = (scale_cfg.output_width + 1) >> 1;
            output_chroma_height = scale_cfg.output_height;
            chroma_crop_left = (crop_left + 1) >> 1;  // one half
            chroma_crop_right = (crop_right + 1) >> 1; // one half
            chroma_crop_top = crop_top;
            chroma_crop_bottom = crop_bottom;
            buffer_ratio = 2;
            rotated_ratio = 2;
            break;
        }
        case JPEG_H2V2:
        {
            chroma_width = (width + 1) >> 1;
            chroma_height = (height + 1) >> 1;
            chroma_crop_width = (crop_width + 1) >> 1;
            chroma_crop_height = (crop_height + 1) >> 1;
            output_chroma_width = (scale_cfg.output_width + 1) >> 1;
            output_chroma_height = (scale_cfg.output_height + 1) >> 1;

            chroma_crop_left = crop_left >> 1;  // one half
            chroma_crop_right = crop_right >> 1; // one half
            chroma_crop_top = crop_top >> 1;
            chroma_crop_bottom = crop_bottom >> 1;
            buffer_ratio = 2;
            rotated_ratio = 2;
            break;
        }
    default:
        JPEG_DBG_ERROR("jpege_engine_sw_up_scale_configure: invalid jpeg subsampling: %d\n", p_engine->subsampling);
        return JPEGERR_EBADPARM;
    }

    switch (p_engine->rotation)
    {
        case 0:
        {
            // setup relative step
            p_engine->horiIncrement_luma = 1;
            p_engine->vertIncrement_luma = p_engine->lumaWidth;
            p_engine->horiIncrement_chroma = 2;
            p_engine->vertIncrement_chroma = 2 * p_engine->chromaWidth;

            // crop width, output width and input width
            rotated_crop_width = crop_width;
            p_scale_info->crop_width = crop_width;
            p_scale_info->crop_height = crop_height;
            p_scale_info->output_width = scale_cfg.output_width;
            p_scale_info->output_height = scale_cfg.output_height;
            // chroma crop width, output width
            rotated_crop_chroma_width = chroma_crop_width;
            p_scale_info->crop_chroma_width = chroma_crop_width;
            p_scale_info->crop_chroma_height = chroma_crop_height;
            p_scale_info->output_chroma_width =  output_chroma_width;
            p_scale_info->output_chroma_height = output_chroma_height;

            // rotated fetch increment from input to line buffer
            //In case of stride enabled, fetch increment is configured to stride set by user. In case of stride disabled, it is set to the width.
            p_scale_info->rotated_luma_fetch_increment = stride_cfg.input_luma_stride;
            p_scale_info->rotated_luma_src_increment = stride_cfg.input_luma_stride;
            p_scale_info->rotated_chroma_fetch_increment
                = stride_cfg.input_chroma_stride / rotated_ratio;
            p_scale_info->rotated_chroma_src_increment
                = stride_cfg.input_chroma_stride / rotated_ratio;
            break;
        }
        case 90:
        {
            // setup relative step
            p_engine->horiIncrement_luma = 1;
            p_engine->vertIncrement_luma = p_engine->lumaHeight;
            p_engine->horiIncrement_chroma = 2;
            p_engine->vertIncrement_chroma = 2 * p_engine->chromaHeight;

            // crop width, output width and input width
            rotated_crop_width = crop_height;
            p_scale_info->crop_width = crop_height;
            p_scale_info->crop_height = crop_width;
            p_scale_info->output_width = scale_cfg.output_height;
            p_scale_info->output_height = scale_cfg.output_width;
            // chroma crop width, output width
            rotated_crop_chroma_width = chroma_crop_height;
            p_scale_info->crop_chroma_width = chroma_crop_height;
            p_scale_info->crop_chroma_height = chroma_crop_width;
            p_scale_info->output_chroma_width = output_chroma_height;
            p_scale_info->output_chroma_height = output_chroma_width;

            // rotated fetch increment from input to line buffer
            //In case of stride enabled, fetch increment is configured to stride set by user. In case of stride disabled, it is set to the width.
            p_scale_info->rotated_luma_fetch_increment
                = (int32_t)(0 - stride_cfg.input_luma_stride);
            p_scale_info->rotated_luma_src_increment = 1;
            p_scale_info->rotated_chroma_fetch_increment
                = (int32_t)(0 - stride_cfg.input_chroma_stride) / rotated_ratio;
            p_scale_info->rotated_chroma_src_increment = 1;
            break;
        }
        case 180:
        {
            // setup relative step
            p_engine->horiIncrement_luma = 1;
            p_engine->vertIncrement_luma = p_engine->lumaWidth;
            p_engine->horiIncrement_chroma = 2;
            p_engine->vertIncrement_chroma = 2 * p_engine->chromaWidth;

            // crop width, output width and input width
            rotated_crop_width = crop_width;
            p_scale_info->crop_width = crop_width;
            p_scale_info->crop_height = crop_height;
            p_scale_info->output_width = scale_cfg.output_width;
            p_scale_info->output_height = scale_cfg.output_height;
            // chroma crop width, output width
            rotated_crop_chroma_width = chroma_crop_width;
            p_scale_info->crop_chroma_width = chroma_crop_width;
            p_scale_info->crop_chroma_height = chroma_crop_height;
            p_scale_info->output_chroma_width = output_chroma_width ;
            p_scale_info->output_chroma_height = output_chroma_height ;

            // rotated fetch increment from input to line buffer
            //In case of stride enabled, fetch increment is configured to stride set by user. In case of stride disabled, it is set to the width.
            p_scale_info->rotated_luma_fetch_increment = -1;
            p_scale_info->rotated_luma_src_increment
                = (int32_t)(0 - stride_cfg.input_luma_stride);
            p_scale_info->rotated_chroma_fetch_increment = -1;
            p_scale_info->rotated_chroma_src_increment
                = (int32_t)(0 - stride_cfg.input_chroma_stride) / rotated_ratio;
            break;
        }
        case 270:
        {
            // setup relative step
            p_engine->horiIncrement_luma = 1;
            p_engine->vertIncrement_luma = p_engine->lumaHeight;
            p_engine->horiIncrement_chroma = 2;
            p_engine->vertIncrement_chroma = 2 * p_engine->chromaHeight;

            // crop width, output width and input width
            rotated_crop_width = crop_height;
            p_scale_info->crop_width = crop_height;
            p_scale_info->crop_height = crop_width;
            p_scale_info->output_width = scale_cfg.output_height;
            p_scale_info->output_height = scale_cfg.output_width;
            // chroma crop width, output width
            rotated_crop_chroma_width = chroma_crop_height;
            p_scale_info->crop_chroma_width = chroma_crop_height;
            p_scale_info->crop_chroma_height = chroma_crop_width;
            p_scale_info->output_chroma_width = output_chroma_height;
            p_scale_info->output_chroma_height = output_chroma_width;

            // rotated fetch increment from input to line buffer
            //In case of stride enabled, fetch increment is configured to stride set by user. In case of stride disabled, it is set to the width.
            p_scale_info->rotated_luma_fetch_increment = stride_cfg.input_luma_stride;
            p_scale_info->rotated_luma_src_increment = -1;
            p_scale_info->rotated_chroma_fetch_increment
                = stride_cfg.input_chroma_stride / rotated_ratio;
            p_scale_info->rotated_chroma_src_increment = -1;
            break;
        }
        default:
            JPEG_DBG_ERROR("jpege_engine_sw_up_scale_configure: unrecognized rotation: %d\n", p_engine->rotation);
            return JPEGERR_EBADPARM;
   }

    /*-----------------------------------------------------------------------
      Allocate buffer for scaled mcu lines
    -----------------------------------------------------------------------*/
    buffer_size = p_scale_info->output_width * p_engine->rotatedMCUHeight;
    buffer_size *= buffer_ratio;
    if (!p_scale_info->p_inline_scaled_buffer)
    {
        // Allocate inline scale output buffer
        rc = jpeg_buffer_init((jpeg_buffer_t *)&(p_scale_info->p_inline_scaled_buffer));
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jpege_engine_sw_scale_crop_setup: p_inline_scaled_buffer init failed\n");
            return rc;
        }
    }
    rc = jpeg_buffer_reset((jpeg_buffer_t)p_scale_info->p_inline_scaled_buffer);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_engine_sw_scale_crop_setup: p_inline_scaled_buffer reset failed\n");
        return rc;
    }
    rc = jpeg_buffer_allocate((jpeg_buffer_t)p_scale_info->p_inline_scaled_buffer, buffer_size * sizeof(uint8_t), false);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_engine_sw_scale_crop_setup: p_inline_scaled_buffer %u bytes allocation failed\n",
                     buffer_size);
        jpeg_buffer_destroy((jpeg_buffer_t *)&(p_scale_info->p_inline_scaled_buffer));
        return rc;
    }
    p_scale_info->vert_luma_fetch_width = rotated_crop_width;
    p_scale_info->vert_chroma_fetch_width = rotated_crop_chroma_width;

    // Initialize input Y and CbCr/CrCb pointers
    p_scale_info->p_luma_input = (unsigned char *)((p_source->p_fragments[0].color.yuv.luma_buf)->ptr) +
                             ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->start_offset;
    p_scale_info->p_chroma_input = (unsigned char *)((p_source->p_fragments[0].color.yuv.chroma_buf)->ptr) +
                             ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->start_offset;

    // Set up input pointer to luma and chroma
    //If stride is enabled, input pointer is calculated using input stride.If stride is disabled, it is calculated using input width.
    if (p_scale_info->crop_flag == true)
    {
        // crop cases
        switch (p_engine->rotation)
        {
            case 0:
            {
                p_scale_info->p_luma_input += stride_cfg.input_luma_stride * crop_top + crop_left;
                p_scale_info->p_chroma_input += stride_cfg.input_chroma_stride * chroma_crop_top
                                                           + 2 * chroma_crop_left;
                break;
            }
            case 90:
            {
                p_scale_info->p_luma_input += stride_cfg.input_luma_stride * crop_bottom + crop_left;
                p_scale_info->p_chroma_input += stride_cfg.input_chroma_stride * chroma_crop_bottom
                                                           + 2 * chroma_crop_left;
                break;
            }
            case 180:
            {
                p_scale_info->p_luma_input += stride_cfg.input_luma_stride * crop_bottom + crop_right;
                p_scale_info->p_chroma_input += stride_cfg.input_chroma_stride * chroma_crop_bottom
                                                           + 2 * chroma_crop_right;
                break;
            }
            case 270:
            {
                p_scale_info->p_luma_input += stride_cfg.input_luma_stride * crop_top + crop_right;
                p_scale_info->p_chroma_input += stride_cfg.input_chroma_stride * chroma_crop_top
                                                           + 2 * chroma_crop_right;
                break;
            }
            default:
                JPEG_DBG_ERROR("jpege_engine_sw_up_scale_configure: unrecognized rotation: %d\n", p_engine->rotation);
                return JPEGERR_EBADPARM;
        }
    }
    else
    {
        // no crop cases
        switch ( p_engine->rotation)
        {
            case 0:
                break;
            case 90:
            {
                p_scale_info->p_luma_input += stride_cfg.input_luma_stride * (p_source->height - 1);
                p_scale_info->p_chroma_input += stride_cfg.input_chroma_stride * (chroma_height - 1);
                break;
            }
            case 180:
            {
                p_scale_info->p_luma_input += (stride_cfg.input_luma_stride * (p_source->height - 1)) + (p_source->width - 1);
                p_scale_info->p_chroma_input += (stride_cfg.input_chroma_stride * (chroma_height - 1)) + ((chroma_width * 2) - 2);
                break;
            }
            case 270:
            {
                p_scale_info->p_luma_input += p_source->width - 1;
                p_scale_info->p_chroma_input += (chroma_width - 1) * 2;
                break;
            }
            default:
                JPEG_DBG_ERROR("jpege_engine_sw_up_scale_configure: unrecognized rotation: %d\n", p_engine->rotation);
                return JPEGERR_EBADPARM;
        }
    }
    return JPEGERR_SUCCESS;
}

/*===========================================================================
FUNCTION        jpege_engine_sw_scale_crop_setup_packed

DESCRIPTION     in-line scalar set up for crop and scale

DEPENDENCIES    None

RETURN VALUE    INT

SIDE EFFECTS    None
===========================================================================*/

static int jpege_engine_sw_scale_crop_setup_packed(jpege_engine_sw_t  *p_engine,
                                            jpege_img_cfg_t    *p_config,
                                            jpege_img_data_t   *p_source)
{
    int       rc;
    uint32_t  width, height, chroma_width, chroma_height;
    uint32_t  crop_width, crop_height, chroma_crop_width, chroma_crop_height;
    uint32_t  rotated_crop_width, rotated_crop_chroma_width;
    uint32_t  output_chroma_width, output_chroma_height;
    uint32_t  crop_top, crop_bottom, crop_left, crop_right;
    uint32_t  chroma_crop_top, chroma_crop_bottom, chroma_crop_left, chroma_crop_right;
    int32_t   rotated_ratio, buffer_size, buffer_ratio;
    jpege_scale_t *p_scale_info = &p_engine->jpege_scale;
    jpege_scale_cfg_t scale_cfg = p_config->scale_cfg;
    jpege_stride_cfg_t stride_cfg = p_engine->stride_cfg;
    //set image sizes
    width = p_source->width;
    height = p_source->height;

    //set corp sizes
    crop_width = scale_cfg.input_width;
    crop_height = scale_cfg.input_height;
    crop_left = scale_cfg.h_offset;
    crop_right = (scale_cfg.h_offset
                 + scale_cfg.input_width - 1);
    crop_top = scale_cfg.v_offset;
    crop_bottom = scale_cfg.v_offset
                  + scale_cfg.input_height - 1;

    /*-----------------------------------------------------------------------
      Initialize crop and scale flag
    -----------------------------------------------------------------------*/
    if ((scale_cfg.h_offset != 0 ||
         scale_cfg.v_offset != 0 ||
        ((scale_cfg.h_offset + scale_cfg.input_width) < p_source->width) ||
        ((scale_cfg.v_offset + scale_cfg.input_height) < p_source->height)) &&
       (crop_width != p_source->width ||
        crop_height != p_source->height))
    {
        p_scale_info->crop_flag = true;
    }

    if  ((scale_cfg.input_width < scale_cfg.output_width) ||
         (scale_cfg.input_height < scale_cfg.output_height))
    {
        // validate upscale ratio
        // support up to 8x upscale
        if ((scale_cfg.output_width > (scale_cfg.input_width << UPSCALE_MAX_FACTOR_DIGIT)) ||
            (scale_cfg.output_height > (scale_cfg.input_height << UPSCALE_MAX_FACTOR_DIGIT)))
        {
            JPEG_DBG_ERROR("jpege_engine_sw_scale_crop_setup: support up to %d upscale: \
                          (input width, input height)(%d, %d)  (output width,output height)(%d, %d)\n",
                          (1 << UPSCALE_MAX_FACTOR_DIGIT), scale_cfg.input_width, scale_cfg.input_height,
                          scale_cfg.output_width, scale_cfg.output_height);
            return JPEGERR_EBADPARM;
        }

        p_scale_info->upscale_flag = true;
    }

    if  ((scale_cfg.input_width > scale_cfg.output_width) ||
         (scale_cfg.input_height > scale_cfg.output_height))
    {
        // validate downscale ratio
        // support up to 1/8 downscale
        if ((scale_cfg.output_width < (scale_cfg.input_width >> DOWNSCALE_MAX_FACTOR_DIGIT)) ||
            (scale_cfg.output_height < (scale_cfg.input_height >> DOWNSCALE_MAX_FACTOR_DIGIT)))
        {
            JPEG_DBG_ERROR("jpege_engine_sw_scale_crop_setup: support up to 1/%d downscale: \
                         (input width, input height)(%d, %d)  (output width,output height)(%d, %d)\n",
                         (1 << UPSCALE_MAX_FACTOR_DIGIT), scale_cfg.input_width, scale_cfg.input_height,
                         scale_cfg.output_width, scale_cfg.output_height);
            return JPEGERR_EBADPARM;
        }

        p_scale_info->downscale_flag = true;
    }

    // if none of upscale, downscale or crop,
    // return
   if (!(p_scale_info->upscale_flag ||
          p_scale_info->downscale_flag ||
          p_scale_info->crop_flag))
    {
        return JPEGERR_SUCCESS;
    }

    /*-----------------------------------------------------------------------
      Calculate Chroma Width and Height
      and crop infomation for chroma
      and prepare to calculate the scale buffer size
    -----------------------------------------------------------------------*/
    chroma_width = (width + 1) >> 1;
    chroma_height = height;
    chroma_crop_width = (crop_width + 1) >> 1;
    chroma_crop_height = crop_height;

    output_chroma_width = (scale_cfg.output_width + 1) >> 1;
    output_chroma_height = scale_cfg.output_height;
    chroma_crop_left = (crop_left + 1) >> 1;  // one half
    chroma_crop_right = (crop_right + 1) >> 1; // one half
    chroma_crop_top = crop_top;
    chroma_crop_bottom = crop_bottom;
    buffer_ratio = 2;
    rotated_ratio = 2;

    switch (p_engine->rotation)
    {
        case 0:
        {
            // setup relative step
            p_engine->horiIncrement_luma = 1;
            p_engine->vertIncrement_luma = p_engine->lumaWidth;
            p_engine->horiIncrement_chroma = 2;
            p_engine->vertIncrement_chroma = 2 * p_engine->chromaWidth;

            // crop width, output width and input width
            rotated_crop_width = crop_width;
            p_scale_info->crop_width = crop_width;
            p_scale_info->crop_height = crop_height;
            p_scale_info->output_width = scale_cfg.output_width;
            p_scale_info->output_height = scale_cfg.output_height;
            // chroma crop width, output width
            rotated_crop_chroma_width = chroma_crop_width;
            p_scale_info->crop_chroma_width = chroma_crop_width;
            p_scale_info->crop_chroma_height = chroma_crop_height;
            p_scale_info->output_chroma_width =  output_chroma_width;
            p_scale_info->output_chroma_height = output_chroma_height;

            // rotated fetch increment from input to line buffer
            //In case of stride enabled, fetch increment is configured to stride set by user. In case of stride disabled, it is set to the width.
            p_scale_info->rotated_luma_fetch_increment = 2 * stride_cfg.input_luma_stride;
            p_scale_info->rotated_luma_src_increment = 2 * stride_cfg.input_luma_stride;
            p_scale_info->rotated_chroma_fetch_increment
                = (2 * stride_cfg.input_chroma_stride) / rotated_ratio;
            p_scale_info->rotated_chroma_src_increment
                = (2 * stride_cfg.input_chroma_stride) / rotated_ratio;
            break;
        }
        case 90:
        {
            // setup relative step
            p_engine->horiIncrement_luma = 1;
            p_engine->vertIncrement_luma = p_engine->lumaHeight;
            p_engine->horiIncrement_chroma = 2;
            p_engine->vertIncrement_chroma = 2 * p_engine->chromaHeight;

            // crop width, output width and input width
            rotated_crop_width = crop_height;
            p_scale_info->crop_width = crop_height;
            p_scale_info->crop_height = crop_width;
            p_scale_info->output_width = scale_cfg.output_height;
            p_scale_info->output_height = scale_cfg.output_width;
            // chroma crop width, output width
            rotated_crop_chroma_width = chroma_crop_height;
            p_scale_info->crop_chroma_width = chroma_crop_height;
            p_scale_info->crop_chroma_height = chroma_crop_width;
            p_scale_info->output_chroma_width = output_chroma_height;
            p_scale_info->output_chroma_height = output_chroma_width;

            // rotated fetch increment from input to line buffer
            //In case of stride enabled, fetch increment is configured to stride set by user. In case of stride disabled, it is set to the width.
            p_scale_info->rotated_luma_fetch_increment
                = (int32_t)(0 - stride_cfg.input_luma_stride) * 2;
            p_scale_info->rotated_luma_src_increment = 2;
            p_scale_info->rotated_chroma_fetch_increment
                = (int32_t)((0 - stride_cfg.input_chroma_stride) * 2) / rotated_ratio;
            p_scale_info->rotated_chroma_src_increment = 2;
            break;
        }
        case 180:
        {
            // setup relative step
            p_engine->horiIncrement_luma = 1;
            p_engine->vertIncrement_luma = p_engine->lumaWidth;
            p_engine->horiIncrement_chroma = 2;
            p_engine->vertIncrement_chroma = 2 * p_engine->chromaWidth;

            // crop width, output width and input width
            rotated_crop_width = crop_width;
            p_scale_info->crop_width = crop_width;
            p_scale_info->crop_height = crop_height;
            p_scale_info->output_width = scale_cfg.output_width;
            p_scale_info->output_height = scale_cfg.output_height;
            // chroma crop width, output width
            rotated_crop_chroma_width = chroma_crop_width;
            p_scale_info->crop_chroma_width = chroma_crop_width;
            p_scale_info->crop_chroma_height = chroma_crop_height;
            p_scale_info->output_chroma_width = output_chroma_width ;
            p_scale_info->output_chroma_height = output_chroma_height;

            // rotated fetch increment from input to line buffer
            //In case of stride enabled, fetch increment is configured to stride set by user. In case of stride disabled, it is set to the width.
            p_scale_info->rotated_luma_fetch_increment = -2;
            p_scale_info->rotated_luma_src_increment
                = (int32_t)(0 - stride_cfg.input_luma_stride) * 2;
            p_scale_info->rotated_chroma_fetch_increment = -2;
            p_scale_info->rotated_chroma_src_increment
                = (int32_t)((0 - stride_cfg.input_chroma_stride) * 2) / rotated_ratio;
            break;
        }
        case 270:
        {
            // setup relative step
            p_engine->horiIncrement_luma = 1;
            p_engine->vertIncrement_luma = p_engine->lumaHeight;
            p_engine->horiIncrement_chroma = 2;
            p_engine->vertIncrement_chroma = 2 * p_engine->chromaHeight;

            // crop width, output width and input width
            rotated_crop_width = crop_height;
            p_scale_info->crop_width = crop_height;
            p_scale_info->crop_height = crop_width;
            p_scale_info->output_width = scale_cfg.output_height;
            p_scale_info->output_height = scale_cfg.output_width;
            // chroma crop width, output width
            //In case of stride enabled, fetch increment is configured to stride set by user. In case of stride disabled, it is set to the width.
            rotated_crop_chroma_width = chroma_crop_height;
            p_scale_info->crop_chroma_width = chroma_crop_height;
            p_scale_info->crop_chroma_height = chroma_crop_width;
            p_scale_info->output_chroma_width = output_chroma_height;
            p_scale_info->output_chroma_height = output_chroma_width;

            // rotated fetch increment from input to line buffer
            p_scale_info->rotated_luma_fetch_increment = 2 * stride_cfg.input_luma_stride;
            p_scale_info->rotated_luma_src_increment = -2;
            p_scale_info->rotated_chroma_fetch_increment
                = (stride_cfg.input_chroma_stride * 2) / rotated_ratio;
            p_scale_info->rotated_chroma_src_increment = -2;
            break;
        }
        default:
            JPEG_DBG_ERROR("jpege_engine_sw_up_scale_configure: unrecognized rotation: %d\n", p_engine->rotation);
            return JPEGERR_EBADPARM;
    }

    /*-----------------------------------------------------------------------
      Allocate buffer for scaled mcu lines
    -----------------------------------------------------------------------*/
    buffer_size = p_scale_info->output_width * p_engine->rotatedMCUHeight;
    buffer_size *= buffer_ratio;
    if (!p_scale_info->p_inline_scaled_buffer)
    {
        // Allocate inline scale output buffer
        rc = jpeg_buffer_init((jpeg_buffer_t *)&(p_scale_info->p_inline_scaled_buffer));
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jpege_engine_sw_scale_crop_setup: p_inline_scaled_buffer init failed\n");
            return rc;
        }
    }
    rc = jpeg_buffer_reset((jpeg_buffer_t)p_scale_info->p_inline_scaled_buffer);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_engine_sw_scale_crop_setup: p_inline_scaled_buffer reset failed\n");
        return rc;
    }
    rc = jpeg_buffer_allocate((jpeg_buffer_t)p_scale_info->p_inline_scaled_buffer, buffer_size * sizeof(uint8_t), false);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_engine_sw_scale_crop_setup: p_inline_scaled_buffer %u bytes allocation failed\n",
                     buffer_size);
        jpeg_buffer_destroy((jpeg_buffer_t *)&(p_scale_info->p_inline_scaled_buffer));
        return rc;
    }

    p_scale_info->vert_luma_fetch_width = rotated_crop_width;
    p_scale_info->vert_chroma_fetch_width = rotated_crop_chroma_width;

    // Initialize input Y and CbCr/CrCb pointers
    p_scale_info->p_luma_input = p_engine->inputYPtr;
    p_scale_info->p_chroma_input = p_engine->inputCbCrPtr;

    // Set up input pointer to luma and chroma
    //If stride is enabled, input pointer is calculated using input stride.If stride is disabled, it is calculated using input width.
    if (p_scale_info->crop_flag == true)
    {
        // crop cases
        switch (p_engine->rotation)
        {
            case 0:
            {
                p_scale_info->p_luma_input += stride_cfg.input_luma_stride * crop_top * 2 + crop_left * 2;
                p_scale_info->p_chroma_input += 4 * (stride_cfg.input_chroma_stride / 2) * chroma_crop_top
                                                           + 4 * chroma_crop_left;
                break;
            }
            case 90:
            {
                p_scale_info->p_luma_input += stride_cfg.input_luma_stride * crop_bottom * 2 + crop_left * 2;
                p_scale_info->p_chroma_input += 4 * (stride_cfg.input_chroma_stride / 2) * chroma_crop_bottom
                                                           + 4 * chroma_crop_left;
                break;
            }
            case 180:
            {
                p_scale_info->p_luma_input += stride_cfg.input_luma_stride * crop_bottom * 2 + crop_right * 2;
                p_scale_info->p_chroma_input += 4 * (stride_cfg.input_chroma_stride / 2) * chroma_crop_bottom
                                                           + 4 * chroma_crop_right;
                break;
            }
            case 270:
            {
                p_scale_info->p_luma_input += 2 * stride_cfg.input_luma_stride * crop_top + 2 * crop_right;
                p_scale_info->p_chroma_input += 4 * (stride_cfg.input_chroma_stride / 2) * chroma_crop_top
                                                          + 4 * chroma_crop_right;
                break;
            }
            default:
                JPEG_DBG_ERROR("jpege_engine_sw_up_scale_configure: unrecognized rotation: %d\n", p_engine->rotation);
                return JPEGERR_EBADPARM;
        }
    }
    else
    {
        // no crop cases
        switch ( p_engine->rotation)
        {
            case 0:
                break;
            case 90:
            {
                   p_scale_info->p_luma_input += 2 * stride_cfg.input_luma_stride * (p_source->height - 1);
                   p_scale_info->p_chroma_input += 4 * (stride_cfg.input_chroma_stride / 2) * (chroma_height - 1);
                break;
            }
            case 180:
            {
                    p_scale_info->p_luma_input += 2 * stride_cfg.input_luma_stride * (p_source->height - 1) + (p_source->width - 2);
                    p_scale_info->p_chroma_input += 2 * stride_cfg.input_chroma_stride * (chroma_height - 1) + ((chroma_width - 1) * 4);
                break;
            }
            case 270:
            {
                p_scale_info->p_luma_input += 2 * (p_source->width - 1);
                p_scale_info->p_chroma_input += 4 * (chroma_width - 1);
                break;
            }
            default:
                JPEG_DBG_ERROR("jpege_engine_sw_up_scale_configure: unrecognized rotation: %d\n", p_engine->rotation);
                return JPEGERR_EBADPARM;
       }
    }
    return JPEGERR_SUCCESS;
}

/*===========================================================================
FUNCTION        jpege_engine_sw_scale_configure_tables

DESCRIPTION     scalar copy quant and zigzag tables

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_scale_configure_tables(jpege_engine_sw_t  *p_engine,
                                                   jpege_img_cfg_t    *p_config)
{
    uint16_t  *p_temp_luma_qtable, *p_temp_chroma_qtable;
    uint16_t  qfactor_invq15;
    const int16_t *p_zigzag_table;
    uint16_t  i, j;

    /*-----------------------------------------------------------------------
      Copy Quant Tables
    -----------------------------------------------------------------------*/
    /* Initialize Luma and Chroma Quantization tables */
    p_temp_luma_qtable = (uint16_t *)p_config->luma_quant_tbl;
    p_temp_chroma_qtable = (uint16_t *)p_config->chroma_quant_tbl;
    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
            qfactor_invq15 = (uint16_t)(1.0/p_temp_luma_qtable[i*8+j] * ((1<<(FIX_POINT_FACTOR-3))-1) + 0.5);
            p_engine->lumaQuantTableQ16[i*8+j] = (int16_t)qfactor_invq15;

            qfactor_invq15 = (uint16_t)(1.0/p_temp_chroma_qtable[i*8+j] * ((1<<(FIX_POINT_FACTOR-3))-1) + 0.5);
            p_engine->chromaQuantTableQ16[i*8+j] = (int16_t)qfactor_invq15;
        }
    }
    // zigzag table
    p_zigzag_table = zigzag_table;

    /* Calculate the zigzagOffsetTable, the offset is the relative BYTES !!!
     * between DCTOutput[i] and DCTOutput[i+1] (which is int16_t), this is due to
     * the limitation of ARM LDRSH instruction which does not allow <opsh>, like:
     *
     * LDRSH Rd, [Rn], Rm, LSL #1
     *
     * and this zigzagOffsetTable is scanned decrementally, that is from
     * 63 down to 0.
     */
    for (i = JBLOCK_SIZE_MINUS_1; i != 0; i--)
    {
        p_engine->zigzagOffsetTable[i] =
            (p_zigzag_table[i - 1] - p_zigzag_table[i]) * sizeof(int16_t);
    }
    p_engine->zigzagOffsetTable[0] = 0;
}

/*===========================================================================
FUNCTION        jpege_engine_sw_fetch_scaled_mcu

DESCRIPTION     Fetch Scale MCU from the input buffers
                Do DCT

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_fetch_dct_scaled_mcu (jpege_engine_sw_t *p_engine,
                                                  int16_t *curr_mcu_luma_dct_output,
                                                  int16_t *curr_mcu_chroma_dct_output)
{
    uint8_t *curr_mcu_ystart_addr, *curr_mcu_cbcrstart_addr;
    uint8_t *curr_block_start_addr;
    uint32_t hori_pixel_index, vert_pixel_index;
    uint32_t vert_mcu_offset, hori_mcu_offset, vert_bound, hori_bound;
    int32_t vert_cross , hori_cross ;
    uint32_t i, j;
    int16_t *p_curr_dct_output;
    jpege_scale_t *p_scale_info = &(p_engine->jpege_scale);

    /* fetch luma data */
    p_curr_dct_output = curr_mcu_luma_dct_output;

    /*------------------------------------------------------------
      If the scaled luma blocks in the buffer is empty,
      scale the Luma and Chroma to output mcu Lines into buffer
    ------------------------------------------------------------*/
    if (p_scale_info->fetched_luma_blocks <= 0)
    {
        (* (p_scale_info->jpege_engine_sw_scale_mcu_lines))(p_scale_info,
                                                            p_engine->rotatedMCUHeight);
    }

    // Calculate the modified origin for the luma data
    hori_pixel_index = p_engine->rotatedhoriMCUIndex * p_engine->rotatedMCUWidth;
    vert_pixel_index = p_engine->rotatedvertMCUIndex * p_engine->rotatedMCUHeight;

    /*------------------------------------------------------------
      Use the MCU x and y coordinates from above to calculate the address of
      the first pixel in the current MCU
      in-line fetch from p_engine->rotatedMCUHeight Lines Luma
    ------------------------------------------------------------*/
    curr_mcu_ystart_addr = (unsigned char *)(p_scale_info->p_inline_scaled_buffer->ptr)
        + (int32_t)hori_pixel_index * p_engine->horiIncrement_luma
        + (int32_t)(vert_pixel_index % (p_engine->rotatedMCUHeight))
                                    * p_engine->vertIncrement_luma;
    // Two layers of loops for the luma blocks within a MCU
    vert_mcu_offset = 0;

    for (i = 0; i < (p_engine->rotatedMCUHeight >> BLOCK_HEIGHT_DIGIT); i++)
    {
        // Calculate how many pixels are needed to pad vertically
        vert_cross  = (int32_t)(p_engine->rotatedlumaHeight -
            (vert_pixel_index + vert_mcu_offset + 1));
        if (vert_cross  >= BLOCK_HEIGHT) // No pad
        {
            vert_bound = BLOCK_HEIGHT;
            curr_block_start_addr = curr_mcu_ystart_addr +
                ((int32_t)vert_mcu_offset * p_engine->vertIncrement_luma);
        }
        else if (vert_cross  >= 0) // Partially pad
        {
            vert_bound = vert_cross  + 1;
            curr_block_start_addr = curr_mcu_ystart_addr + (int32_t)vert_mcu_offset *
                                 p_engine->vertIncrement_luma;
        }
        else // Fully pad
        {
            vert_bound = 1;
            curr_block_start_addr = curr_mcu_ystart_addr +
                                 ((int32_t)(vert_mcu_offset - p_engine->rotatedMCUHeight) +
                                 (BLOCK_HEIGHT + vert_cross )) * p_engine->vertIncrement_luma;
            curr_block_start_addr = curr_block_start_addr < (unsigned char *)p_scale_info->p_inline_scaled_buffer->ptr
                                 ?  (unsigned char *)p_scale_info->p_inline_scaled_buffer->ptr : curr_block_start_addr;
        }
        hori_mcu_offset = 0;
        for (j=0; j < p_engine->rotatedMCUWidth/BLOCK_WIDTH; j++)
        {
            // Calculate how many pixels are needed to pad horizontally
            hori_cross  = p_engine->rotatedlumaWidth - (hori_pixel_index + hori_mcu_offset + 1);
            if (hori_cross  >= BLOCK_WIDTH) // No pad
            {
                hori_bound = BLOCK_WIDTH;
                curr_block_start_addr += (int32_t)hori_mcu_offset * p_engine->horiIncrement_luma;
            }
            else if (hori_cross  >= 0) // Partially pad
            {
                hori_bound = hori_cross  + 1;
                curr_block_start_addr += (int32_t)hori_mcu_offset * p_engine->horiIncrement_luma;
            }
            else // Fully pad
            {
                hori_bound = 1;
                curr_block_start_addr += ((int32_t)(hori_mcu_offset - p_engine->rotatedMCUWidth) +
                                        (BLOCK_WIDTH + hori_cross )) * p_engine->horiIncrement_luma;
                curr_block_start_addr = curr_block_start_addr < (unsigned char *)p_scale_info->p_inline_scaled_buffer->ptr
                                    ?  (unsigned char *)p_scale_info->p_inline_scaled_buffer->ptr : curr_block_start_addr;
            }

            // fetch block from scaled luma
            if ((vert_bound == BLOCK_HEIGHT) && (hori_bound == BLOCK_WIDTH))
            {
                jpege_engine_sw_fetch_dct_block_luma (curr_block_start_addr,
                                                      p_curr_dct_output,
                                                      p_scale_info->output_width);
            }
            else
            {
                jpege_engine_sw_fetch_dct_scaled_block_partial_luma (&p_engine->jpege_scale,
                                                                     curr_block_start_addr,
                                                                     hori_bound, vert_bound, p_curr_dct_output);
            }
            // Fetching one Luma block
            p_scale_info->fetched_luma_blocks -= 1;

            p_curr_dct_output += JBLOCK_SIZE;

            hori_mcu_offset += BLOCK_WIDTH;
        }
        vert_mcu_offset += BLOCK_HEIGHT;
    }

    // fetch chroma data
    p_curr_dct_output = curr_mcu_chroma_dct_output;

    // Calculate the modified origin for the chroma data
    hori_pixel_index = p_engine->rotatedhoriMCUIndex * BLOCK_WIDTH;
    vert_pixel_index = p_engine->rotatedvertMCUIndex * BLOCK_HEIGHT;
    /*------------------------------------------------------------
      Use the MCU x and y coordinates from above to calculate the address of
      the first pixel in the current MCU
      in-line fetch from BLOCK_HEIGHT Lines chroma
    ------------------------------------------------------------*/
    curr_mcu_cbcrstart_addr = (unsigned char *)p_scale_info->p_inline_scaled_buffer->ptr
        + p_scale_info->output_width * p_engine->rotatedMCUHeight
        + (int32_t)hori_pixel_index * p_engine->horiIncrement_chroma
        + (int32_t)(vert_pixel_index % (BLOCK_HEIGHT)) * p_engine->vertIncrement_chroma;

    // Calculate how many pixels are needed to pad vertically
    vert_cross  = p_engine->rotatedchromaHeight - (vert_pixel_index + 1);
    if (vert_cross  >= BLOCK_HEIGHT)
    {
        vert_bound = BLOCK_HEIGHT; // No pad
    }
    else
    {
        vert_bound = vert_cross  + 1; // Partially Pad
    }
    // Calculate how many pixels are needed to pad horizontally
    hori_cross  = p_engine->rotatedchromaWidth - (hori_pixel_index + 1);
    if (hori_cross  >= BLOCK_WIDTH)
    {
        hori_bound = BLOCK_WIDTH; // No pad
    }
    else
    {
        hori_bound = hori_cross  + 1; // Partially pad
    }

    // Dual Format Selection
    // fetch block from scaled chroma
    if ((vert_bound == BLOCK_HEIGHT) && (hori_bound == BLOCK_WIDTH))
    {
       jpege_engine_sw_fetch_dct_block_chroma (curr_mcu_cbcrstart_addr,
                                                p_curr_dct_output,
                                                p_scale_info->output_chroma_width,
                                                p_engine->InputFormat);
    }
    else
    {
        jpege_engine_sw_fetch_dct_scaled_block_partial_chroma (p_engine,
                                                               curr_mcu_cbcrstart_addr,
                                                               hori_bound, vert_bound, p_curr_dct_output);
    }
}
/*===========================================================================
FUNCTION        jpege_engine_sw_fetch_dct_scaled_block_partial_luma

DESCRIPTION     Fetch partial Luma block from the scaled input
                buffers and Do Dct

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_fetch_dct_scaled_block_partial_luma(jpege_scale_t *p_upscale,
                                                                const uint8_t *p_luma_block_origin,
                                                                uint32_t       hori_bound,
                                                                uint32_t       vert_bound,
                                                                int16_t       *p_dct_output)
{
    const uint8_t *p_luma_block;
    const uint8_t *p_padding_line;
    uint32_t       luma_width = p_upscale->output_width;
    uint8_t        fetched_block[JBLOCK_SIZE];
    uint8_t       *p_fetched_block = fetched_block;
    uint32_t       i, j;

    p_luma_block = p_luma_block_origin;

    // Vertically unpadded part
    for (i = 0; i < vert_bound; i++)
    {
        // Horizontally unpadded part
        STD_MEMMOVE(p_fetched_block, p_luma_block, (hori_bound * sizeof(uint8_t)));
        p_fetched_block += hori_bound;

        // Horizontally padded part
        for (j = hori_bound; j < BLOCK_WIDTH; j++)
        {
            *p_fetched_block = *(p_fetched_block - 1);
            p_fetched_block++;
        }
        p_luma_block += luma_width;
    }

    // Vertically padded part
    p_padding_line = p_fetched_block - BLOCK_WIDTH;
    for (i = vert_bound; i < BLOCK_HEIGHT; i++)
    {
        STD_MEMMOVE(p_fetched_block, p_padding_line, (BLOCK_WIDTH * sizeof(uint8_t)));
        p_fetched_block += BLOCK_WIDTH;
        p_luma_block    += luma_width;
    }

    // Do DCT
    jpege_engine_sw_fdct_block(fetched_block, p_dct_output);
}

/*===========================================================================
FUNCTION        jpege_engine_sw_fetch_dct_scaled_block_partial_chroma

DESCRIPTION     Fetch partial Chroma block from the scaled input
                buffers and Do Dct

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_fetch_dct_scaled_block_partial_chroma(jpege_engine_sw_t *p_engine,
                                                                  const uint8_t  *p_chroma_block_origin,
                                                                  uint32_t hori_bound,
                                                                  uint32_t vert_bound,
                                                                  int16_t        *p_dct_output)
{
    const uint8_t *p_chroma_block;
    const uint8_t *p_padding_line;
    uint32_t       chroma_width = p_engine->jpege_scale.output_chroma_width;
    uint8_t        fetched_block[JBLOCK_SIZE * NUM_CHROMA_BLOCKS]; // chroma is interleaved
    uint8_t        deinterleaved_block[JBLOCK_SIZE * NUM_CHROMA_BLOCKS];
    uint8_t       *p_fetched_block = fetched_block;
    uint8_t       *p_deinterleaved_block1 = deinterleaved_block;
    uint8_t       *p_deinterleaved_block2 = p_deinterleaved_block1 + JBLOCK_SIZE;
    uint8_t       *p_cb_block, *p_cr_block;
    uint32_t       i, j;

    p_chroma_block = p_chroma_block_origin;

    // Vertically unpadded part
    for (i = 0; i < vert_bound; i++)
    {
        // Horizontally unpadded part
        STD_MEMMOVE(p_fetched_block, p_chroma_block, (hori_bound * sizeof(uint8_t) * 2));
        p_fetched_block += (hori_bound * 2);

        // Horizontally padded part
        for (j = hori_bound; j < BLOCK_WIDTH; j++)
        {
            *(p_fetched_block)     = *(p_fetched_block - 2);
            *(p_fetched_block + 1) = *(p_fetched_block - 1);
            p_fetched_block += 2;
        }
        p_chroma_block += (chroma_width * 2);
    }

    // Vertically padded part
    p_padding_line = p_fetched_block - (BLOCK_WIDTH * 2);
    for (i = vert_bound; i < BLOCK_HEIGHT; i++)
    {
        STD_MEMMOVE(p_fetched_block, p_padding_line, (BLOCK_WIDTH * sizeof(uint8_t) * 2));
        p_fetched_block += (BLOCK_WIDTH * 2);
        p_chroma_block  += (chroma_width * 2);
    }

    // Deinterleave chroma block
    for (i = 0; i < BLOCK_HEIGHT; i++)
    {
        for (j = 0; j < BLOCK_WIDTH; j++)
        {
            *p_deinterleaved_block1++ = fetched_block[i*BLOCK_WIDTH*2+j*2];
            *p_deinterleaved_block2++ = fetched_block[i*BLOCK_WIDTH*2+j*2+1];
        }
    }

    // Dual Format Selection
    if (p_engine->InputFormat == YCrCb)
    {
        p_cb_block = deinterleaved_block + JBLOCK_SIZE;
        p_cr_block = deinterleaved_block;
    }
    else
    {
        p_cb_block = deinterleaved_block;
        p_cr_block = deinterleaved_block + JBLOCK_SIZE;
    }

    // Do DCT - always in the order of CbCr
    jpege_engine_sw_fdct_block(p_cb_block, p_dct_output);
    jpege_engine_sw_fdct_block(p_cr_block, (p_dct_output + JBLOCK_SIZE));
}

/*===========================================================================
FUNCTION        jpege_engine_sw_copy_rotated_luma_lines

DESCRIPTION     copy rotated luma lines

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
void jpege_engine_sw_copy_rotated_luma_lines(uint8_t *p_src,
                                             uint8_t *p_output,
                                             uint32_t input_width,
                                             int32_t  fetch_increment,
                                             uint32_t pixel_extend_left,
                                             uint32_t pixel_extend_right)
{
    uint32_t i;
    // Extend the first line
    // Extend the left
    while (pixel_extend_left --)
    {
        *(p_output ++)= *p_src;
    }
    // Fetch pixel by pixel
    for (i = 0; i < input_width; i++)
    {
        *(p_output ++) = *(p_src + fetch_increment * i);
    }
    // Extend the right
    while (pixel_extend_right --)
    {
        *(p_output ++) = *(p_src + fetch_increment * (input_width - 1));
    }
}

/*===========================================================================
FUNCTION        jpege_engine_sw_copy_rotated_chroma_lines

DESCRIPTION     Copy rotated chroma lines

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
void jpege_engine_sw_copy_rotated_chroma_lines(uint8_t *p_src,
                                               uint8_t *p_output,
                                               uint32_t input_width,
                                               int32_t  fetch_increment,
                                               uint32_t pixel_extend_left,
                                               uint32_t pixel_extend_right)
{
    uint32_t k;
    // Extend the first line
    // Extend the left
    while (pixel_extend_left --)
    {
        *(p_output ++) = *p_src;
        *(p_output ++) = *(p_src + 1);
    }
    for (k = 0; k < input_width; k += 2)
    {
        *(p_output ++)
            = *(p_src + fetch_increment * k);
        *(p_output ++)
            = *(p_src + fetch_increment * k + 1);
    }
    // Extend the right
    while (pixel_extend_right --)
    {
        *(p_output ++)
            = *(p_src + fetch_increment * (input_width - 2));
        *(p_output ++)
            = *(p_src + fetch_increment * (input_width - 2) + 1);
    }
}
/*===========================================================================
FUNCTION        jpege_engine_sw_copy_rotated_chroma_lines_packed

DESCRIPTION     Copy rotated chroma lines

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
void jpege_engine_sw_copy_rotated_chroma_lines_packed(uint8_t *p_src,
                                                      uint8_t *p_output,
                                                      uint32_t input_width,
                                                      int32_t  fetch_increment,
                                                      uint32_t pixel_extend_left,
                                                      uint32_t pixel_extend_right)
{
    uint32_t k;
    // Extend the first line
    // Extend the left
    while (pixel_extend_left --)
    {
        *(p_output ++) = *p_src;
        *(p_output ++) = *(p_src + 2);
    }
    for (k = 0; k < input_width; k +=2)
    {
        *(p_output ++)
            = *(p_src + fetch_increment * k);
        *(p_output ++)
            = *(p_src + fetch_increment * k + 2);
    }
    // Extend the right
    while (pixel_extend_right --)
    {
        *(p_output ++)
            = *(p_src + fetch_increment * (input_width - 2));
        *(p_output ++)
            = *(p_src + fetch_increment * (input_width - 2) + 2);
    }
}

/*===========================================================================
FUNCTION        jpege_sw_engine_scale_destroy_buffer

DESCRIPTION     Destroy and clean up scale internal buffer

DEPENDENCIES    None

RETURN VALUE    void

SIDE EFFECTS    None
===========================================================================*/
void jpege_engine_sw_scale_destroy(
    jpege_scale_t      *p_scale)
{
    if (p_scale)
    {
        // internal scale buffer clean up
        if (p_scale->p_inline_scaled_buffer)
        {
            jpeg_buffer_destroy((jpeg_buffer_t *)&(p_scale->p_inline_scaled_buffer));
        }
        if (p_scale->p_vertical_scaled_buffer)
        {
            jpeg_buffer_destroy((jpeg_buffer_t *)&(p_scale->p_vertical_scaled_buffer));
        }
        if (p_scale->p_input_luma_line_buffer)
        {
            jpeg_buffer_destroy((jpeg_buffer_t *)&(p_scale->p_input_luma_line_buffer));
        }
        if (p_scale->p_input_chroma_line_buffer)
        {
            jpeg_buffer_destroy((jpeg_buffer_t *)&(p_scale->p_input_chroma_line_buffer));
        }
        if (p_scale->downscale_flag || p_scale->crop_flag)
        {
            if (p_scale->scale.down.p_vert_luma_accum)
            {
                JPEG_FREE(p_scale->scale.down.p_vert_luma_accum);
            }
            if (p_scale->scale.down.p_vert_chroma_accum)
            {
                JPEG_FREE(p_scale->scale.down.p_vert_chroma_accum);
            }
        }
    }
}
