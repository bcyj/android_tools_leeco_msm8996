/*========================================================================


*//** @file jpeg_postprocess_config.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2009 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
11/18/09   mingy   Added tiling output support.
11/03/09   mingy   Added CbCr output support.
09/21/09   mingy   Added region based decoding support and odd sized image
                   decoding support.
04/22/09   mingy   Created file.
                   Branch out post process operation configuration from
                   jpeg_postprocessor.c.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpeg_postprocessor.h"
#include "jpeglog.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */


/* =======================================================================
**                       Macro/Constant Definitions
** ======================================================================= */

/******************************************************************************
*  Here we define a Macro to find the number of lines to skip for
*  region decoding.
*
*  If region decoding is requested, we need to skip the lines in the first
*  decoded MCU row that are not in the specified region.
*
*  For the Chroma subsampled cases:
*
*  a) If there is no scalable decoding, the MCU size is 16x16, i.e.  16 Rows
*     and 16 Cols. The number of lines to skip is computed as:
*     clipping.top % pBufferFormat->nHeight.
*
*  b) If there is 1/2 scalable decoding, the MCU size is 8x8, the number of
*     lines to skip is computed as:
*     (clipping.top / 2 ) % pBufferFormat->nHeight.
*
*  c) If there is 1/4 scalable decoding, the MCU size is 4x4, the number of
*     lines to skip is computed as:
*     (clipping.top / 4) % pBufferFormat->nHeight.
*
*  d) If there is 1/8 scalable decoding, the MUC size is 2x2, the number of
*     lines to skip is computed as:
*     (clipping.top / 8 ) % pBufferFormat->nHeight.
*
*  Notice that the buffer height (MCU Rows) is different in each of the
*  above cases, we can actually use it to derive a equation to cover all the
*  4 cases:
*
*  (clipping.top / (16 / pBufferFormat->nHeight)) % pBufferFormat->nHeight
*
*  Another thing to keep in mind is that for the Chroma subsampled cases,
*  the number to skip can not be odd. A right shift by 1 followed by
*  a left shift by 1 will round down an odd number to an even number.
*
*  The final equation to compute the lines to skipp is:
*
*  ((clipping.top * pBufferFormat->nHeight) / 16) % pBufferFormat->nHeight
*  plus round down to even.
*
*********************************************************************/
#define GETLINESTOSKIP16(x, y) ((( ( ( (x) * (y) ) >> 4 ) % (y) )>> 1) << 1)

/******************************************************************************
*  For the Chroma non-subsampled cases, the MCU size is 8x8, and it is not
*  necessary to round down an odd number to even.
*******************************************************************************/
#define GETLINESTOSKIP8(x, y) ( (( (x) * (y) ) >> 3 ) % (y) )


/* =======================================================================
**                          Function Definitions
** ======================================================================= */

/* Helper functions */

/**************************************************************************
 * Jpeg decoder post process operation configuration
 *************************************************************************/
static int jpeg_operation_config (jpeg_postprocessor_t *p_processor,
                                  jpegd_dst_t          *p_dest,
                                  jpegd_engine_dst_t   *p_engine_dst,
                                  int32_t               nRotationClockwiseInDegree);

/**************************************************************************
* swap two values
*************************************************************************/
static void swap (uint32_t *a, uint32_t *b)
{
    uint32_t  t;
    t = *a;
    *a = *b;
    *b = t;
}

/******************************************************************************
** Jpeg decoder post processor configuration function
******************************************************************************/
int jpeg_postprocessor_configure(jpeg_postprocessor_t *p_processor,
                                 jpegd_engine_dst_t   *p_engine_dst,
                                 jpegd_dst_t          *p_dest,
                                 uint32_t              chunk_width,
                                 uint32_t              chunk_height,
                                 uint8_t               use_pmem,
                                 int32_t               rotation)
{
    int rc;
    int32_t weight = 1;
    uint32_t num_pixels = p_dest->width * p_dest->height;
    jpeg_rectangle_t region;

    // The output buffer struct to be dequeued from the Q
    jpegd_output_buf_t *p_output_buf;

    if (!num_pixels)
    {
        return JPEGERR_EBADPARM;
    }

    // Reset some processor values in case decoder is called more than once.
    p_processor->first_row_id = 0;
    p_processor->is_last_buffer = false;

    // Reject odd output YUV dimensions in case of Chroma subsampling
    if (p_dest->output_format == YCRCBLP_H2V2 ||
        p_dest->output_format == YCBCRLP_H2V2)
    {
        if (p_dest->width & 0x01 || p_dest->height & 0x01)
        {
            return JPEGERR_EUNSUPPORTED;
        }
    }
    else if (p_dest->output_format == YCRCBLP_H2V1 ||
             p_dest->output_format == YCBCRLP_H2V1)
    {
        if (p_dest->width & 0x01)
        {
            return JPEGERR_EUNSUPPORTED;
        }
    }

    p_processor->data_move = DATA_MOVE_MAX;
    p_processor->color_convert = COLOR_CONVERT_MAX;
    p_processor->down_scale = DOWN_SCALE_MAX;
    p_processor->p_dest = p_dest;

    /**********************************************************************
    * make local copy of the output dimension.
    *
    * For 90/270 rotation cases, in order for the down scalar to work,
    * output width/height are swapped. We don't want this swapping
    * be passed back to p_dest so a seperate copy of the output
    * dimension is necessary.
    *********************************************************************/
    p_processor->nInputWidth   = p_engine_dst->width;
    p_processor->nInputHeight  = p_engine_dst->height;
    p_processor->nOutputWidth = p_dest->width;
    p_processor->nOutputHeight = p_dest->height;

    // Initialize this variable with the specified region information
    region = p_dest->region;

    /**********************************************************************
    *  If no region information is specified, then set the region to     *
    *  the whole image.                                                  *
    *********************************************************************/
    if (region.right == 0 && region.bottom == 0)
    {
        region.right  = p_dest->width  - 1;
        region.bottom = p_dest->height - 1;
    }

    /**********************************************************************
     * Validate the region:
     * 0 <= left < right
     * 0 <= top  < bottom
     *********************************************************************/
    if (0 <= region.left && region.left < region.right &&
        0 <= region.top  && region.top  < region.bottom)
    {
        /**********************************************************************
        *  If no stride info was specified, use image width as the stride.
        *
        *  For RGB cases, set stride = width * depth/8,
        *  For YUV cases, set stride = width.
        *
        *  Reject when the stride is less than the image width (*depth/8)
        *********************************************************************/
        switch (p_dest->output_format)
        {
        case RGB565:
            weight = 2;
            break;

        case RGB888:
            weight = 3;
            break;

        case RGBa:
            weight = 4;
            break;

        default:
            weight = 1;
            break;
        }

        if (p_dest->stride == 0)
        {
            p_processor->nStride = p_dest->width * weight;
        }
        else if (p_dest->stride >= p_dest->width * weight)
        {
            p_processor->nStride = p_dest->stride;
        }
        else  // Reject stride < output width (except for stride = 0)
        {
            return JPEGERR_EUNSUPPORTED;
        }

        switch (p_engine_dst->subsampling)
        {
        case JPEG_H2V2:
            // Obtain the # of lines to skip in the first MCU Row
            p_processor->lines_to_skip = GETLINESTOSKIP16((uint32_t)(p_dest->region.top), chunk_height);
            // Compute the top offset in the region.
            // For H2V2 case here, the top CbCr offset is half to the Y offset
            p_processor->region_top_offset_in_mcu_row_y     = chunk_width * p_processor->lines_to_skip;
            p_processor->region_top_offset_in_mcu_row_crcb  = p_processor->region_top_offset_in_mcu_row_y >> 1;
            // Compute the left offset in the MCU Row.
            // For this H2V2 case, CbCr left offset is the same to the Y left offset.
            p_processor->region_left_offset_in_mcu_row_y    = GETLINESTOSKIP16((uint32_t)(p_dest->region.left), chunk_height);
            p_processor->region_left_offset_in_mcu_row_crcb = p_processor->region_left_offset_in_mcu_row_y;

            break;

        case JPEG_H2V1:
            p_processor->lines_to_skip = GETLINESTOSKIP8((uint32_t)(p_dest->region.top), chunk_height);
            p_processor->region_top_offset_in_mcu_row_y     = chunk_width * p_processor->lines_to_skip;
            p_processor->region_top_offset_in_mcu_row_crcb  = p_processor->region_top_offset_in_mcu_row_y;
            p_processor->region_left_offset_in_mcu_row_y    = GETLINESTOSKIP16((uint32_t)(p_dest->region.left), chunk_height*2);
            p_processor->region_left_offset_in_mcu_row_crcb = p_processor->region_left_offset_in_mcu_row_y;

            break;

        case JPEG_H1V2:
            p_processor->lines_to_skip = GETLINESTOSKIP16((uint32_t)(p_dest->region.top), chunk_height);
            p_processor->region_top_offset_in_mcu_row_y     = chunk_width * p_processor->lines_to_skip;
            p_processor->region_top_offset_in_mcu_row_crcb  = p_processor->region_top_offset_in_mcu_row_y;
            p_processor->region_left_offset_in_mcu_row_y    = GETLINESTOSKIP8((uint32_t)(p_dest->region.left), chunk_height/2);
            p_processor->region_left_offset_in_mcu_row_crcb = p_processor->region_left_offset_in_mcu_row_y << 1;

            break;

        case JPEG_H1V1:
            p_processor->lines_to_skip = GETLINESTOSKIP8((uint32_t)(p_dest->region.top), chunk_height);
            p_processor->region_top_offset_in_mcu_row_y     = chunk_width * p_processor->lines_to_skip;
            p_processor->region_top_offset_in_mcu_row_crcb  = p_processor->region_top_offset_in_mcu_row_y << 1;
            p_processor->region_left_offset_in_mcu_row_y    = GETLINESTOSKIP8((uint32_t)(p_dest->region.left), chunk_height);
            p_processor->region_left_offset_in_mcu_row_crcb = p_processor->region_left_offset_in_mcu_row_y << 1;

            break;

        default:
            break;
        }
    }

    /**********************************************************************
     * Region decoding is reqired but parameters given are wrong
     *********************************************************************/
    else
    {
        return JPEGERR_EBADPARM;
    }

    /**********************************************************************
    *  Config the post process operation
    *
    *  Notice that this config function will use the member variabls
    *  m_outputFormat and m_nStride so it can not come before where these
    *  2 member variables are assigned the value.
    *
    *  Also keep in mind that this config function will swap
    *  p_processor->nOutputWidth and p_processor->nOutputHeight
    *  in 90/270 rotation cases.
    *********************************************************************/

    rc = jpeg_operation_config(p_processor, p_dest, p_engine_dst, rotation);

    if (p_processor->data_move     == DATA_MOVE_MAX     &&
        p_processor->color_convert == COLOR_CONVERT_MAX &&
        p_processor->down_scale    == DOWN_SCALE_MAX)
    {
        JPEG_DBG_LOW("jpeg_postprocessor_configure: operation not supported\n");
        return JPEGERR_EUNSUPPORTED;
    }

    /************************************************************************
    * if no tiling requested, a buffer large enough to hold the whole
    * final output image was allocated and enqueued.
    * in this case we only dequeue once here and obtain the large buffer
    *************************************************************************/
    if (!p_processor->tiling_enabled)
    {
        /********************************************************************
        * dequeue one line of tiling buffer
        * return if failed to dequeue (may be triggered by abort)
        *********************************************************************/
        if(jpeg_postprocessor_dequeue_output_buf(p_processor, &p_output_buf))
            return JPEGERR_EFAILED;

        // Validate destination buffer is big enough
        switch (p_dest->output_format)
        {
        case YCRCBLP_H2V2:
        case YCBCRLP_H2V2:
            if (((jpeg_buf_t *)p_output_buf->data.yuv.luma_buf)->size   < p_processor->nStride * p_processor->nHeight ||
                ((jpeg_buf_t *)p_output_buf->data.yuv.chroma_buf)->size < (p_processor->nStride * p_processor->nHeight >> 1))
            {
                JPEG_DBG_LOW("jpeg_postprocessor_configure: destination buffer too small\n");
                return JPEGERR_EFAILED;
            }
            break;

        case YCRCBLP_H2V1:
        case YCBCRLP_H2V1:
            if (((jpeg_buf_t *)p_output_buf->data.yuv.luma_buf)->size   < p_processor->nStride * p_processor->nHeight ||
                ((jpeg_buf_t *)p_output_buf->data.yuv.chroma_buf)->size < p_processor->nStride * p_processor->nHeight)
            {
                JPEG_DBG_LOW("jpeg_postprocessor_configure: destination buffer too small\n");
                return JPEGERR_EFAILED;
            }
            break;

        case RGB565:
        case RGB888:
        case RGBa:
            if (((jpeg_buf_t *)p_output_buf->data.rgb.rgb_buf)->size < p_processor->nStride * p_processor->nHeight)
            {
                JPEG_DBG_LOW("jpeg_postprocessor_configure: destination buffer too small\n");
                return JPEGERR_EFAILED;
            }
            break;

        default:
            JPEG_DBG_LOW("jpeg_postprocessor_configure: output format not supported\n");
            return JPEGERR_EUNSUPPORTED;
        }

        // Set destination pointers
        if (p_dest->output_format == RGB565 ||
            p_dest->output_format == RGB888 ||
            p_dest->output_format == RGBa)
        {
            p_processor->pDestinationRGB = ((jpeg_buf_t *)p_output_buf->data.rgb.rgb_buf)->ptr;
            STD_MEMSET(p_processor->pDestinationRGB, 0, ((jpeg_buf_t *)p_output_buf->data.rgb.rgb_buf)->size * sizeof(uint8_t));
        }
        else
        {
            p_processor->pDestinationLuma   = ((jpeg_buf_t *)p_output_buf->data.yuv.luma_buf)->ptr;
            p_processor->pDestinationChroma = ((jpeg_buf_t *)p_output_buf->data.yuv.chroma_buf)->ptr;
            STD_MEMSET(p_processor->pDestinationLuma,   0, ((jpeg_buf_t *)p_output_buf->data.yuv.luma_buf)->size * sizeof(uint8_t));
            STD_MEMSET(p_processor->pDestinationChroma, 0, ((jpeg_buf_t *)p_output_buf->data.yuv.chroma_buf)->size * sizeof(uint8_t));
        }

        /**********************************************************************
        * The following statements are used for non-zero
        * degree rotation cases. Adjust destination starting address.
        *********************************************************************/
        p_processor->pDestinationLuma   += p_processor->nLumaAddressOffset;
        p_processor->pDestinationChroma += p_processor->nChromaAddressOffset;
        p_processor->pDestinationRGB    += p_processor->nRGBAddressOffset;
    }

    /************************************************************************
    * Allocate memory for the Chroma upsampled output
    * Chroma upsampling is based on the post process input, so use
    * the input width to define the size of the buffer.
    *************************************************************************/
    JPEG_FREE(p_processor->pChromaSmpOutput);
    p_processor->pChromaSmpOutput = (uint8_t *)JPEG_MALLOC(2 * p_processor->nInputWidth * sizeof(uint8_t));
    if (p_processor->pChromaSmpOutput)
    {
        STD_MEMSET(p_processor->pChromaSmpOutput, 0, 2 * p_processor->nInputWidth * sizeof(uint8_t));
    }
    else
    {
        return JPEGERR_EMALLOC;
    }

    // Allocate memory for the Accumulate line buffers.
    JPEG_FREE(p_processor->pLumaAccum);
    p_processor->pLumaAccum = (uint32_t *)JPEG_MALLOC(p_processor->nOutputWidth * sizeof(uint32_t));
    if (p_processor->pLumaAccum)
    {
        STD_MEMSET(p_processor->pLumaAccum, 0, p_processor->nOutputWidth * sizeof(uint32_t));
    }
    else
    {
        return JPEGERR_EMALLOC;
    }

    JPEG_FREE(p_processor->pChromaAccum);
    p_processor->pChromaAccum = (uint32_t *)JPEG_MALLOC(2 * p_processor->nOutputWidth * sizeof(uint32_t));
    if (p_processor->pChromaAccum)
    {
        STD_MEMSET(p_processor->pChromaAccum, 0, 2 * p_processor->nOutputWidth * sizeof(uint32_t));
    }
    else
    {
        return JPEGERR_EMALLOC;
    }

    JPEG_FREE(p_processor->pTempLumaLine);
    p_processor->pTempLumaLine = (uint8_t *)JPEG_MALLOC(p_processor->nOutputWidth * sizeof(uint8_t));
    if (p_processor->pTempLumaLine)
    {
        STD_MEMSET(p_processor->pTempLumaLine, 0, p_processor->nOutputWidth * sizeof(uint8_t));
    }
    else
    {
        return JPEGERR_EMALLOC;
    }

    JPEG_FREE(p_processor->pTempChromaLine);
    p_processor->pTempChromaLine = (uint8_t *)JPEG_MALLOC(2 * p_processor->nOutputWidth * sizeof(uint8_t));
    if (p_processor->pTempChromaLine)
    {
        STD_MEMSET(p_processor->pTempChromaLine, 0, 2 * p_processor->nOutputWidth * sizeof(uint8_t));
    }
    else
    {
        return JPEGERR_EMALLOC;
    }

    // allocate internal MCU Row buffers for tiling cases
    JPEG_FREE(p_processor->p_internal_buf);
    p_processor->p_internal_buf = (uint8_t *)JPEG_MALLOC(p_processor->nStride * chunk_height * sizeof(uint8_t));
    if (p_processor->p_internal_buf)
    {
        STD_MEMSET(p_processor->p_internal_buf, 0, p_processor->nStride * chunk_height * sizeof(uint8_t));
    }
    else
    {
        return JPEGERR_EMALLOC;
    }

    // initialize intermediate variables.
    p_processor->nColsProcessed = 0;
    p_processor->nRowsProcessed = 0;

    // initialize intermediate variables.
    p_processor->nVStep            = 1;
    p_processor->nVCount           = 0;
    p_processor->fVerticalChroma   = true;
    p_processor->nInputWidth       = p_engine_dst->width;
    p_processor->nInputHeight      = p_engine_dst->height;
    p_processor->inputSubsampling  = p_engine_dst->subsampling;
    p_processor->chunk_width       = chunk_width;
    p_processor->chunk_height      = chunk_height;
    p_processor->is_first_row      = true;

    // Reset the engine destination buffers
    rc = jpeg_buffer_reset(p_engine_dst->p_luma_buffers[0]) |
         jpeg_buffer_reset(p_engine_dst->p_luma_buffers[1]) |
         jpeg_buffer_reset(p_engine_dst->p_chroma_buffers[0]) |
         jpeg_buffer_reset(p_engine_dst->p_chroma_buffers[1]);

    if (JPEG_SUCCEEDED(rc))
    {
        uint32_t luma_size = chunk_width * chunk_height;
        uint32_t chroma_size = luma_size;

        if (p_engine_dst->subsampling == JPEG_H2V2)
        {
            chroma_size = luma_size >> 1;
        }
        else if (p_engine_dst->subsampling == JPEG_H1V1)
        {
            chroma_size = luma_size << 1;
        }
        // Allocate engine destination buffers
        rc = jpeg_buffer_allocate(p_engine_dst->p_luma_buffers[0],   luma_size,   use_pmem) |
             jpeg_buffer_allocate(p_engine_dst->p_luma_buffers[1],   luma_size,   use_pmem) |
             jpeg_buffer_allocate(p_engine_dst->p_chroma_buffers[0], chroma_size, use_pmem) |
             jpeg_buffer_allocate(p_engine_dst->p_chroma_buffers[1], chroma_size, use_pmem);
    }

    return rc;
}

/**************************************************************************
 * Jpeg decoder post process operation configuration
 *************************************************************************/
static int jpeg_operation_config(jpeg_postprocessor_t *p_processor,
                                 jpegd_dst_t          *p_dest,
                                 jpegd_engine_dst_t   *p_engine_dst,
                                 int32_t               nRotationClockwiseInDegree)
{
    // Save the final output height.
    // In 90/270 rotation cases they swap the output width/height
    // p_processor->nHeight will save the original output height.
    p_processor->nHeight = p_processor->nOutputHeight;

    // Initialization
    p_processor->jpegd_postprocess_func = NULL;
    p_processor->data_move              = DATA_MOVE_MAX;
    p_processor->color_convert          = COLOR_CONVERT_MAX;
    p_processor->down_scale             = DOWN_SCALE_MAX;
    p_processor->nLumaAddressOffset     = 0;
    p_processor->nChromaAddressOffset   = 0;
    p_processor->nRGBAddressOffset      = 0;

    switch (nRotationClockwiseInDegree)
    {
    /**********************************************************************
    *   0 degree rotation case.
    *********************************************************************/
    case 0:
        // Reject upscaling
        if (p_engine_dst->width  < p_processor->nOutputWidth ||
            p_engine_dst->height < p_processor->nOutputHeight)
        {
            return JPEGERR_EUNSUPPORTED;
        }

        // Input dimensions == output dimensions ==> Data Move
        else if (p_engine_dst->width  == p_processor->nOutputWidth &&
                 p_engine_dst->height == p_processor->nOutputHeight)
        {
            switch (p_engine_dst->subsampling)
            {
            // Input format H2V2
            case JPEG_H2V2:

                switch (p_dest->output_format)
                {
                case YCRCBLP_H2V2:            //H2V2-->H2V2
                    p_processor->jpegd_postprocess_func = &data_move;
                    p_processor->data_move = H2V2_TO_H2V2;
                    break;

                case YCBCRLP_H2V2:            //H2V2-->H2V2_CbCr
                    p_processor->jpegd_postprocess_func = &data_move;
                    p_processor->data_move = H2V2_TO_H2V2_CbCr;
                    break;

                case YCRCBLP_H2V1:            //H2V2-->H2V1
                    p_processor->jpegd_postprocess_func = &data_move;
                    p_processor->data_move = H2V2_TO_H2V1;
                    break;

                case YCBCRLP_H2V1:            //H2V2-->H2V1_CbCr
                    p_processor->jpegd_postprocess_func = &data_move;
                    p_processor->data_move = H2V2_TO_H2V1_CbCr;
                    break;

                case RGB565:                  //H2V2-->RGB565
                    p_processor->jpegd_postprocess_func = &color_convert;
                    p_processor->color_convert = H2V2_TO_RGB565;
                    break;

                case RGB888:                  //H2V2-->RGB888
                    p_processor->jpegd_postprocess_func = &color_convert;
                    p_processor->color_convert = H2V2_TO_RGB888;
                    break;

                case RGBa:                    //H2V2-->RGBa
                    p_processor->jpegd_postprocess_func = &color_convert;
                    p_processor->color_convert = H2V2_TO_RGBa;
                    break;

                default:
                    break;
                }

                break;

            // Input format H2V1
            case JPEG_H2V1:

                switch (p_dest->output_format)
                {
                case YCRCBLP_H2V2:            //H2V1-->H2V2
                    p_processor->jpegd_postprocess_func = &data_move;
                    p_processor->data_move = H2V1_TO_H2V2;
                    break;

                case YCBCRLP_H2V2:            //H2V1-->H2V2_CbCr
                    p_processor->jpegd_postprocess_func = &data_move;
                    p_processor->data_move = H2V1_TO_H2V2_CbCr;
                    break;

                case YCRCBLP_H2V1:            //H2V1-->H2V1
                    p_processor->jpegd_postprocess_func = &data_move;
                    p_processor->data_move = H2V1_TO_H2V1;
                    break;

                case YCBCRLP_H2V1:            //H2V1-->H2V1_CbCr
                    p_processor->jpegd_postprocess_func = &data_move;
                    p_processor->data_move = H2V1_TO_H2V1_CbCr;
                    break;
                case RGB565:                  //H2V1-->RGB565
                    p_processor->jpegd_postprocess_func = &color_convert;
                    p_processor->color_convert = H2V1_TO_RGB565;
                    break;

                case RGB888:                  //H2V1-->RGB888
                    p_processor->jpegd_postprocess_func = &color_convert;
                    p_processor->color_convert = H2V1_TO_RGB888;
                    break;

                case RGBa:                    //H2V1-->RGBa
                    p_processor->jpegd_postprocess_func = &color_convert;
                    p_processor->color_convert = H2V1_TO_RGBa;
                    break;

                default:
                    break;
                }

                break;


            // Input format H1V2
            case JPEG_H1V2:

                switch (p_dest->output_format)
                {
                case YCRCBLP_H2V2:            //H1V2-->H2V2
                    p_processor->jpegd_postprocess_func = &data_move;
                    p_processor->data_move = H1V2_TO_H2V2;
                    break;

                case YCBCRLP_H2V2:            //H1V2-->H2V2_CbCr
                    p_processor->jpegd_postprocess_func = &data_move;
                    p_processor->data_move = H1V2_TO_H2V2_CbCr;
                    break;

                case YCRCBLP_H2V1:            //H1V2-->H2V1
                    p_processor->jpegd_postprocess_func = &data_move;
                    p_processor->data_move = H1V2_TO_H2V1;
                    break;

                case YCBCRLP_H2V1:            //H1V2-->H2V1_CbCr
                    p_processor->jpegd_postprocess_func = &data_move;
                    p_processor->data_move = H1V2_TO_H2V1_CbCr;
                    break;

                case RGB565:                  //H1V2-->RGB565
                    p_processor->jpegd_postprocess_func = &color_convert;
                    p_processor->color_convert = H1V2_TO_RGB565;
                    break;

                case RGB888:                  //H1V2-->RGB888
                    p_processor->jpegd_postprocess_func = &color_convert;
                    p_processor->color_convert = H1V2_TO_RGB888;
                    break;

                case RGBa:                    //H1V2-->RGBa
                    p_processor->jpegd_postprocess_func = &color_convert;
                    p_processor->color_convert = H1V2_TO_RGBa;
                    break;

                default:
                    break;
                }

                break;


            // Input format H1V1
            case JPEG_H1V1:

                switch (p_dest->output_format)
                {
                case YCRCBLP_H2V2:            //H1V1-->H2V2
                    p_processor->jpegd_postprocess_func = &data_move;
                    p_processor->data_move = H1V1_TO_H2V2;
                    break;

                case YCBCRLP_H2V2:            //H1V1-->H2V2_CbCr
                    p_processor->jpegd_postprocess_func = &data_move;
                    p_processor->data_move = H1V1_TO_H2V2_CbCr;
                    break;

                case YCRCBLP_H2V1:            //H1V1-->H2V1
                    p_processor->jpegd_postprocess_func = &data_move;
                    p_processor->data_move = H1V1_TO_H2V1;
                    break;

                case YCBCRLP_H2V1:            //H1V1-->H2V1_CbCr
                    p_processor->jpegd_postprocess_func = &data_move;
                    p_processor->data_move = H1V1_TO_H2V1_CbCr;
                    break;

                case RGB565:                  //H1V1-->RGB565
                    p_processor->jpegd_postprocess_func = &color_convert;
                    p_processor->color_convert = H1V1_TO_RGB565;
                    break;

                case RGB888:                  //H1V1-->RGB888
                    p_processor->jpegd_postprocess_func = &color_convert;
                    p_processor->color_convert = H1V1_TO_RGB888;
                    break;

                case RGBa:                    //H1V1-->RGBa
                    p_processor->jpegd_postprocess_func = &color_convert;
                    p_processor->color_convert = H1V1_TO_RGBa;
                    break;

                default:
                    break;
                }

                break;

            default:
                break;

            } //end switch (p_engine_dst->subsampling)

        }
        else // Output dimension < input dimension ==> Down Scaling
        {
            p_processor->jpegd_postprocess_func = &down_scale;

            switch (p_dest->output_format)
            {
            case YCRCBLP_H2V2:
                p_processor->down_scale = DS_H2V2;
                break;

            case YCRCBLP_H2V1:
                p_processor->down_scale = DS_H2V1;
                break;

            case YCBCRLP_H2V2:
                p_processor->down_scale = DS_H2V2_CbCr;
                break;

            case YCBCRLP_H2V1:
                p_processor->down_scale = DS_H2V1_CbCr;
                break;

            case RGB565:
                p_processor->down_scale = DS_RGB565;
                break;

            case RGB888:
                p_processor->down_scale = DS_RGB888;
                break;

            case RGBa:
                p_processor->down_scale = DS_RGBa;
                break;

            default:
                break;
            }
        }

        break; // case 0

    /**********************************************************************
    *   90 degree rotation case.
    *   The Luma/Chroma/RGB starting address offset will be calculated
    *   based on operation.
    *********************************************************************/
    case 90:

        // Reject upscaling
        if (p_engine_dst->width  < p_processor->nOutputHeight ||
            p_engine_dst->height < p_processor->nOutputWidth)
        {
            return JPEGERR_EUNSUPPORTED;
        }

        // Input dimensions == output dimensions ==> Data Move
        else if (p_engine_dst->width  == p_processor->nOutputHeight &&
                 p_engine_dst->height == p_processor->nOutputWidth)
        {
            switch (p_engine_dst->subsampling)
            {
            // Input format H2V2
            case JPEG_H2V2:

                switch (p_dest->output_format)
                {
                case YCRCBLP_H2V2:            //H2V2-->H2V2
                    p_processor->jpegd_postprocess_func = &data_move_rot90;
                    p_processor->data_move = H2V2_TO_H2V2_R90;
                    p_processor->nLumaAddressOffset  = p_processor->nOutputWidth - 1;
                    p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                    break;

                case YCBCRLP_H2V2:            //H2V2-->H2V2_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot90;
                    p_processor->data_move = H2V2_TO_H2V2_R90_CbCr;
                    p_processor->nLumaAddressOffset   = p_processor->nOutputWidth - 1;
                    p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                    break;

                case YCRCBLP_H2V1:            //H2V2-->H2V1
                    p_processor->jpegd_postprocess_func = &data_move_rot90;
                    p_processor->data_move = H2V2_TO_H2V1_R90;
                    p_processor->nLumaAddressOffset  = p_processor->nOutputWidth - 1;
                    p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                    break;

                case YCBCRLP_H2V1:            //H2V2-->H2V1_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot90;
                    p_processor->data_move = H2V2_TO_H2V1_R90_CbCr;
                    p_processor->nLumaAddressOffset   = p_processor->nOutputWidth - 1;
                    p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                    break;

                case RGB565:                  //H2V2-->RGB565
                    p_processor->jpegd_postprocess_func = &color_convert_rot90;
                    p_processor->color_convert = H2V2_TO_RGB565_R90;
                    p_processor->nRGBAddressOffset = p_processor->nOutputWidth * 2 - 2;
                    break;

                case RGB888:                  //H2V2-->RGB888
                    p_processor->jpegd_postprocess_func = &color_convert_rot90;
                    p_processor->color_convert = H2V2_TO_RGB888_R90;
                    p_processor->nRGBAddressOffset = p_processor->nOutputWidth * 3 - 3;
                    break;

                case RGBa:                    //H2V2-->RGBa
                    p_processor->jpegd_postprocess_func = &color_convert_rot90;
                    p_processor->color_convert = H2V2_TO_RGBa_R90;
                    p_processor->nRGBAddressOffset = p_processor->nOutputWidth * 4 - 4;
                    break;

                default:
                    break;
                }

                break;

            // Input format H2V1
            case JPEG_H2V1:

                switch (p_dest->output_format)
                {
                case YCRCBLP_H2V2:            //H2V1-->H2V2
                    p_processor->jpegd_postprocess_func = &data_move_rot90;
                    p_processor->data_move = H2V1_TO_H2V2_R90;
                    p_processor->nLumaAddressOffset  = p_processor->nOutputWidth - 1;
                    p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                    break;

                case YCBCRLP_H2V2:            //H2V1-->H2V2_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot90;
                    p_processor->data_move = H2V1_TO_H2V2_R90_CbCr;
                    p_processor->nLumaAddressOffset  = p_processor->nOutputWidth - 1;
                    p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                    break;

                case YCRCBLP_H2V1:            //H2V1-->H2V1
                    p_processor->jpegd_postprocess_func = &data_move_rot90;
                    p_processor->data_move = H2V1_TO_H2V1_R90;
                    p_processor->nLumaAddressOffset  = p_processor->nOutputWidth - 1;
                    p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                    break;

                case YCBCRLP_H2V1:            //H2V1-->H2V1_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot90;
                    p_processor->data_move = H2V1_TO_H2V1_R90_CbCr;
                    p_processor->nLumaAddressOffset  = p_processor->nOutputWidth - 1;
                    p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                    break;

                case RGB565:                  //H2V1-->RGB565
                    p_processor->jpegd_postprocess_func = &color_convert_rot90;
                    p_processor->color_convert = H2V1_TO_RGB565_R90;
                    p_processor->nRGBAddressOffset = p_processor->nOutputWidth * 2 - 2;
                    break;

                case RGB888:                  //H2V1-->RGB888
                    p_processor->jpegd_postprocess_func = &color_convert_rot90;
                    p_processor->color_convert = H2V1_TO_RGB888_R90;
                    p_processor->nRGBAddressOffset = p_processor->nOutputWidth * 3 - 3;
                    break;

                case RGBa:                    //H2V1-->RGBa
                    p_processor->jpegd_postprocess_func = &color_convert_rot90;
                    p_processor->color_convert = H2V1_TO_RGBa_R90;
                    p_processor->nRGBAddressOffset = p_processor->nOutputWidth * 4 - 4;
                    break;

                default:
                    break;
                }

                break;


            // Input format H1V2
            case JPEG_H1V2:

                switch (p_dest->output_format)
                {
                case YCRCBLP_H2V2:            //H1V2-->H2V2
                    p_processor->jpegd_postprocess_func = &data_move_rot90;
                    p_processor->data_move = H1V2_TO_H2V2_R90;
                    p_processor->nLumaAddressOffset  = p_processor->nOutputWidth - 1;
                    p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                    break;

                case YCBCRLP_H2V2:            //H1V2-->H2V2_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot90;
                    p_processor->data_move = H1V2_TO_H2V2_R90_CbCr;
                    p_processor->nLumaAddressOffset  = p_processor->nOutputWidth - 1;
                    p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                    break;

                case YCRCBLP_H2V1:            //H1V2-->H2V1
                    p_processor->jpegd_postprocess_func = &data_move_rot90;
                    p_processor->data_move = H1V2_TO_H2V1_R90;
                    p_processor->nLumaAddressOffset  = p_processor->nOutputWidth - 1;
                    p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                    break;

                case YCBCRLP_H2V1:            //H1V2-->H2V1_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot90;
                    p_processor->data_move = H1V2_TO_H2V1_R90_CbCr;
                    p_processor->nLumaAddressOffset  = p_processor->nOutputWidth - 1;
                    p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                    break;

                case RGB565:                  //H1V2-->RGB565
                    p_processor->jpegd_postprocess_func = &color_convert_rot90;
                    p_processor->color_convert = H1V2_TO_RGB565_R90;
                    p_processor->nRGBAddressOffset = p_processor->nOutputWidth * 2 - 2;
                    break;

                case RGB888:                  //H1V2-->RGB888
                    p_processor->jpegd_postprocess_func = &color_convert_rot90;
                    p_processor->color_convert = H1V2_TO_RGB888_R90;
                    p_processor->nRGBAddressOffset = p_processor->nOutputWidth * 3 - 3;
                    break;

                case RGBa:                    //H1V2-->RGBa
                    p_processor->jpegd_postprocess_func = &color_convert_rot90;
                    p_processor->color_convert = H1V2_TO_RGBa_R90;
                    p_processor->nRGBAddressOffset = p_processor->nOutputWidth * 4 - 4;
                    break;
                default:
                    break;
                }

                break;


            // Input format H1V1
            case JPEG_H1V1:

                switch (p_dest->output_format)
                {
                case YCRCBLP_H2V2:            //H1V1-->H2V2
                    p_processor->jpegd_postprocess_func = &data_move_rot90;
                    p_processor->data_move = H1V1_TO_H2V2_R90;
                    p_processor->nLumaAddressOffset  = p_processor->nOutputWidth - 1;
                    p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                    break;

                case YCBCRLP_H2V2:            //H1V1-->H2V2_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot90;
                    p_processor->data_move = H1V1_TO_H2V2_R90_CbCr;
                    p_processor->nLumaAddressOffset  = p_processor->nOutputWidth - 1;
                    p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                    break;

                case YCRCBLP_H2V1:            //H1V1-->H2V1
                    p_processor->jpegd_postprocess_func = &data_move_rot90;
                    p_processor->data_move = H1V1_TO_H2V1_R90;
                    p_processor->nLumaAddressOffset  = p_processor->nOutputWidth - 1;
                    p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                    break;

                case YCBCRLP_H2V1:            //H1V1-->H2V1_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot90;
                    p_processor->data_move = H1V1_TO_H2V1_R90_CbCr;
                    p_processor->nLumaAddressOffset  = p_processor->nOutputWidth - 1;
                    p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                    break;

                case RGB565:                  //H1V1-->RGB565
                    p_processor->jpegd_postprocess_func = &color_convert_rot90;
                    p_processor->color_convert = H1V1_TO_RGB565_R90;
                    p_processor->nRGBAddressOffset = p_processor->nOutputWidth * 2 - 2;
                    break;

                case RGB888:                  //H1V1-->RGB888
                    p_processor->jpegd_postprocess_func = &color_convert_rot90;
                    p_processor->color_convert = H1V1_TO_RGB888_R90;
                    p_processor->nRGBAddressOffset = p_processor->nOutputWidth * 3 - 3;
                    break;

                case RGBa:                    //H1V1-->RGBa
                    p_processor->jpegd_postprocess_func = &color_convert_rot90;
                    p_processor->color_convert = H1V1_TO_RGBa_R90;
                    p_processor->nRGBAddressOffset = p_processor->nOutputWidth * 4 - 4;
                    break;

                default:
                    break;
                }

                break;

            default:
                break;

            } //end switch (p_engine_dst->subsampling)

        }
        else // Output dimension < input dimension ==> Down Scaling
        {
            /**********************************************************************
            * The down scaling algorithm is based on the un-rotated output
            * image dimensions. For 90/270 rotation cases we need to switch
            * the output image width/height for the down scalar to be working.
            * The member variable m_nHeight will save the original output height.
            *********************************************************************/

            p_processor->jpegd_postprocess_func = &down_scale;

            switch (p_dest->output_format)
            {
            case YCRCBLP_H2V2:
                p_processor->down_scale = DS_H2V2_R90;
                p_processor->nLumaAddressOffset   = p_processor->nOutputWidth - 1;
                p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                break;

            case YCRCBLP_H2V1:
                p_processor->down_scale = DS_H2V1_R90;
                p_processor->nLumaAddressOffset   = p_processor->nOutputWidth - 1;
                p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                break;

            case YCBCRLP_H2V2:
                p_processor->down_scale = DS_H2V2_R90_CbCr;
                p_processor->nLumaAddressOffset   = p_processor->nOutputWidth - 1;
                p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                break;

            case YCBCRLP_H2V1:
                p_processor->down_scale = DS_H2V1_R90_CbCr;
                p_processor->nLumaAddressOffset   = p_processor->nOutputWidth - 1;
                p_processor->nChromaAddressOffset = p_processor->nOutputWidth - 2;
                break;

            case RGB565:
                p_processor->down_scale = DS_RGB565_R90;
                p_processor->nRGBAddressOffset = p_processor->nOutputWidth * 2 - 2;
                break;

            case RGB888:
                p_processor->down_scale = DS_RGB888_R90;
                p_processor->nRGBAddressOffset = p_processor->nOutputWidth * 3 - 3;
                break;

            case RGBa:
                p_processor->down_scale = DS_RGBa_R90;
                p_processor->nRGBAddressOffset = p_processor->nOutputWidth * 4 - 4;
                break;

            default:
                break;
            }

            swap(&(p_processor->nOutputWidth), &(p_processor->nOutputHeight));
        }

        break; // case 90


    /**********************************************************************
    *   180 degree rotation case.
    *   The Luma/Chroma/RGB starting address offset will be calculated
    *   based on operation.
    *********************************************************************/
    case 180:

         // Reject upscaling
        if (p_engine_dst->width < p_processor->nOutputWidth ||
            p_engine_dst->height < p_processor->nOutputHeight)
        {
            return JPEGERR_EUNSUPPORTED;
        }

        // Input dimensions == output dimensions ==> Data Move
        else if (p_engine_dst->width  == p_processor->nOutputWidth &&
                 p_engine_dst->height == p_processor->nOutputHeight)
        {
            switch (p_engine_dst->subsampling)
            {
            // Input format H2V2
            case JPEG_H2V2:

                switch (p_dest->output_format)
                {
                case YCRCBLP_H2V2:            //H2V2-->H2V2
                    p_processor->jpegd_postprocess_func = &data_move_rot180;
                    p_processor->data_move              = H2V2_TO_H2V2_R180;
                    p_processor->nLumaAddressOffset     = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 1;
                    p_processor->nChromaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight/2 -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 2;
                    break;

                case YCBCRLP_H2V2:            //H2V2-->H2V2_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot180;
                    p_processor->data_move              = H2V2_TO_H2V2_R180_CbCr;
                    p_processor->nLumaAddressOffset     = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 1;
                    p_processor->nChromaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight/2 -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 2;
                    break;

                case YCRCBLP_H2V1:            //H2V2-->H2V1
                    p_processor->jpegd_postprocess_func = &data_move_rot180;
                    p_processor->data_move              = H2V2_TO_H2V1_R180;
                    p_processor->nLumaAddressOffset     = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 1;
                    p_processor->nChromaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 2;
                    break;

                case YCBCRLP_H2V1:            //H2V2-->H2V1_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot180;
                    p_processor->data_move              = H2V2_TO_H2V1_R180_CbCr;
                    p_processor->nLumaAddressOffset     = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 1;
                    p_processor->nChromaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 2;
                    break;

                case RGB565:                  //H2V2-->RGB565
                    p_processor->jpegd_postprocess_func = &color_convert_rot180;
                    p_processor->color_convert          = H2V2_TO_RGB565_R180;
                    p_processor->nRGBAddressOffset      = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth*2) - 2;
                    break;

                case RGB888:                  //H2V2-->RGB888
                    p_processor->jpegd_postprocess_func = &color_convert_rot180;
                    p_processor->color_convert          = H2V2_TO_RGB888_R180;
                    p_processor->nRGBAddressOffset      = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth*3) - 3;
                    break;

                case RGBa:                    //H2V2-->RGBa
                    p_processor->jpegd_postprocess_func = &color_convert_rot180;
                    p_processor->color_convert          = H2V2_TO_RGBa_R180;
                    p_processor->nRGBAddressOffset      = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth*4) - 4;
                    break;

                default:
                    break;
                }

                break;

            // Input format H2V1
            case JPEG_H2V1:

                switch (p_dest->output_format)
                {
                case YCRCBLP_H2V2:            //H2V1-->H2V2
                    p_processor->jpegd_postprocess_func = &data_move_rot180;
                    p_processor->data_move              = H2V1_TO_H2V2_R180;
                    p_processor->nLumaAddressOffset     = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 1;
                    p_processor->nChromaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight/2 -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 2;
                    break;

                case YCBCRLP_H2V2:            //H2V1-->H2V2_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot180;
                    p_processor->data_move              = H2V1_TO_H2V2_R180_CbCr;
                    p_processor->nLumaAddressOffset     = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 1;
                    p_processor->nChromaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight/2 -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 2;
                    break;

                case YCRCBLP_H2V1:            //H2V1-->H2V1
                    p_processor->jpegd_postprocess_func = &data_move_rot180;
                    p_processor->data_move              = H2V1_TO_H2V1_R180;
                    p_processor->nLumaAddressOffset     = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 1;
                    p_processor->nChromaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 2;
                    break;

                case YCBCRLP_H2V1:            //H2V1-->H2V1_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot180;
                    p_processor->data_move              = H2V1_TO_H2V1_R180_CbCr;
                    p_processor->nLumaAddressOffset     = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 1;
                    p_processor->nChromaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 2;
                    break;

                case RGB565:                  //H2V1-->RGB565
                    p_processor->jpegd_postprocess_func = &color_convert_rot180;
                    p_processor->color_convert          = H2V1_TO_RGB565_R180;
                    p_processor->nRGBAddressOffset      = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth*2) - 2;
                    break;

                case RGB888:                  //H2V1-->RGB888
                    p_processor->jpegd_postprocess_func = &color_convert_rot180;
                    p_processor->color_convert          = H2V1_TO_RGB888_R180;
                    p_processor->nRGBAddressOffset      = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth*3) - 3;
                    break;

                case RGBa:                    //H2V1-->RGBa
                    p_processor->jpegd_postprocess_func = &color_convert_rot180;
                    p_processor->color_convert          = H2V1_TO_RGBa_R180;
                    p_processor->nRGBAddressOffset      = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth*4) - 4;
                    break;

                default:
                    break;
                }

                break;


            // Input format H1V2
            case JPEG_H1V2:

                switch (p_dest->output_format)
                {
                case YCRCBLP_H2V2:            //H1V2-->H2V2
                    p_processor->jpegd_postprocess_func = &data_move_rot180;
                    p_processor->data_move              = H1V2_TO_H2V2_R180;
                    p_processor->nLumaAddressOffset     = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 1;
                    p_processor->nChromaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight/2 -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 2;
                    break;

                case YCBCRLP_H2V2:            //H1V2-->H2V2_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot180;
                    p_processor->data_move              = H1V2_TO_H2V2_R180_CbCr;
                    p_processor->nLumaAddressOffset     = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 1;
                    p_processor->nChromaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight/2 -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 2;
                    break;

                case YCRCBLP_H2V1:            //H1V2-->H2V1
                    p_processor->jpegd_postprocess_func = &data_move_rot180;
                    p_processor->data_move              = H1V2_TO_H2V1_R180;
                    p_processor->nLumaAddressOffset     = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 1;
                    p_processor->nChromaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 2;
                    break;

                case YCBCRLP_H2V1:            //H1V2-->H2V1_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot180;
                    p_processor->data_move              = H1V2_TO_H2V1_R180_CbCr;
                    p_processor->nLumaAddressOffset     = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 1;
                    p_processor->nChromaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 2;
                    break;

                case RGB565:                  //H1V2-->RGB565
                    p_processor->jpegd_postprocess_func = &color_convert_rot180;
                    p_processor->color_convert          = H1V2_TO_RGB565_R180;
                    p_processor->nRGBAddressOffset      = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth*2) - 2;
                    break;

                case RGB888:                  //H1V2-->RGB888
                    p_processor->jpegd_postprocess_func = &color_convert_rot180;
                    p_processor->color_convert          = H1V2_TO_RGB888_R180;
                    p_processor->nRGBAddressOffset      = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth*3) - 3;
                    break;

                case RGBa:                    //H1V2-->RGBa
                    p_processor->jpegd_postprocess_func = &color_convert_rot180;
                    p_processor->color_convert          = H1V2_TO_RGBa_R180;
                    p_processor->nRGBAddressOffset      = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth*4) - 4;
                    break;

                default:
                    break;
                }

                break;


            // Input format H1V1
            case JPEG_H1V1:

                switch (p_dest->output_format)
                {
                case YCRCBLP_H2V2:            //H1V1-->H2V2
                    p_processor->jpegd_postprocess_func = &data_move_rot180;
                    p_processor->data_move              = H1V1_TO_H2V2_R180;
                    p_processor->nLumaAddressOffset     = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 1;
                    p_processor->nChromaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight/2 -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 2;
                    break;

                case YCBCRLP_H2V2:            //H1V1-->H2V2_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot180;
                    p_processor->data_move              = H1V1_TO_H2V2_R180_CbCr;
                    p_processor->nLumaAddressOffset     = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 1;
                    p_processor->nChromaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight/2 -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 2;
                    break;

                case YCRCBLP_H2V1:            //H1V1-->H2V1
                    p_processor->jpegd_postprocess_func = &data_move_rot180;
                    p_processor->data_move              = H1V1_TO_H2V1_R180;
                    p_processor->nLumaAddressOffset     = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 1;
                    p_processor->nChromaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 2;
                    break;

                case YCBCRLP_H2V1:            //H1V1-->H2V1_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot180;
                    p_processor->data_move              = H1V1_TO_H2V1_R180_CbCr;
                    p_processor->nLumaAddressOffset     = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 1;
                    p_processor->nChromaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth) - 2;
                    break;

                case RGB565:                  //H1V1-->RGB565
                    p_processor->jpegd_postprocess_func = &color_convert_rot180;
                    p_processor->color_convert          = H1V1_TO_RGB565_R180;
                    p_processor->nRGBAddressOffset      = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth*2) - 2;
                    break;

                case RGB888:                  //H1V1-->RGB888
                    p_processor->jpegd_postprocess_func = &color_convert_rot180;
                    p_processor->color_convert          = H1V1_TO_RGB888_R180;
                    p_processor->nRGBAddressOffset      = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth*3) - 3;
                    break;

                case RGBa:                    //H1V1-->RGBa
                    p_processor->jpegd_postprocess_func = &color_convert_rot180;
                    p_processor->color_convert          = H1V1_TO_RGBa_R180;
                    p_processor->nRGBAddressOffset      = p_processor->nStride * p_processor->nOutputHeight -
                                                         (p_processor->nStride - p_processor->nOutputWidth*4) - 4;
                    break;

                default:
                    break;
                }

                break;

            default:
                break;

            } //end switch (p_engine_dst->subsampling)

        }
        else // Output dimension < input dimension ==> Down Scaling
        {
            /**********************************************************************
            * The down scaling algorithm is based on the un-rotated output
            * image dimensions. For 90/270 rotation cases we need to switch
            * the output image width/height for the down scalar to be working.
            * The member variable m_nHeight will save the original output height.
            *********************************************************************/

            p_processor->jpegd_postprocess_func = &down_scale;

            switch (p_dest->output_format)
            {
            case YCRCBLP_H2V2:
                p_processor->down_scale           = DS_H2V2_R180;
                p_processor->nLumaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight -
                                                   (p_processor->nStride - p_processor->nOutputWidth) - 1;
                p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight/2 -
                                                   (p_processor->nStride - p_processor->nOutputWidth) - 2;
                break;

            case YCRCBLP_H2V1:
                p_processor->down_scale           = DS_H2V1_R180;
                p_processor->nLumaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight -
                                                   (p_processor->nStride - p_processor->nOutputWidth) - 1;
                p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight -
                                                   (p_processor->nStride - p_processor->nOutputWidth) - 2;
                break;

            case YCBCRLP_H2V2:
                p_processor->down_scale           = DS_H2V2_R180_CbCr;
                p_processor->nLumaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight -
                                                   (p_processor->nStride - p_processor->nOutputWidth) - 1;
                p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight/2 -
                                                   (p_processor->nStride - p_processor->nOutputWidth) - 2;
                break;

            case YCBCRLP_H2V1:
                p_processor->down_scale           = DS_H2V1_R180_CbCr;
                p_processor->nLumaAddressOffset   = p_processor->nStride * p_processor->nOutputHeight -
                                                   (p_processor->nStride - p_processor->nOutputWidth) - 1;
                p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight -
                                                   (p_processor->nStride - p_processor->nOutputWidth) - 2;
                break;

            case RGB565:
                p_processor->down_scale        = DS_RGB565_R180;
                p_processor->nRGBAddressOffset = p_processor->nStride * p_processor->nOutputHeight -
                                                (p_processor->nStride - p_processor->nOutputWidth*2) - 2;
                break;

            case RGB888:
                p_processor->down_scale        = DS_RGB888_R180;
                p_processor->nRGBAddressOffset = p_processor->nStride * p_processor->nOutputHeight -
                                                (p_processor->nStride - p_processor->nOutputWidth*3) - 3;
                break;

            case RGBa:
                p_processor->down_scale        = DS_RGBa_R180;
                p_processor->nRGBAddressOffset = p_processor->nStride * p_processor->nOutputHeight -
                                                (p_processor->nStride - p_processor->nOutputWidth*4) - 4;
                break;

            default:
                break;
            }
        }

        break; // case 180


    /**********************************************************************
    *   270 degree rotation case.
    *   The Luma/Chroma/RGB starting address offset will be calculated
    *   based on operation.
    *********************************************************************/
    case 270:

        // Reject upscaling
        if (p_engine_dst->width  < p_processor->nOutputHeight ||
            p_engine_dst->height < p_processor->nOutputWidth)
        {
            return JPEGERR_EUNSUPPORTED;
        }

        // Input dimensions == output dimensions ==> Data Move
        else if (p_engine_dst->width  == p_processor->nOutputHeight &&
                 p_engine_dst->height == p_processor->nOutputWidth)
        {
            switch (p_engine_dst->subsampling)
            {
            // Input format H2V2
            case JPEG_H2V2:

                switch (p_dest->output_format)
                {
                case YCRCBLP_H2V2:            //H2V2-->H2V2
                    p_processor->jpegd_postprocess_func = &data_move_rot270;
                    p_processor->data_move = H2V2_TO_H2V2_R270;
                    p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight/2 - p_processor->nStride;
                    break;

                case YCBCRLP_H2V2:            //H2V2-->H2V2_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot270;
                    p_processor->data_move = H2V2_TO_H2V2_R270_CbCr;
                    p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight/2 - p_processor->nStride;
                    break;

                case YCRCBLP_H2V1:            //H2V2-->H2V1
                    p_processor->jpegd_postprocess_func = &data_move_rot270;
                    p_processor->data_move = H2V2_TO_H2V1_R270;
                    p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                case YCBCRLP_H2V1:            //H2V2-->H2V1_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot270;
                    p_processor->data_move = H2V2_TO_H2V1_R270_CbCr;
                    p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                case RGB565:                  //H2V2-->RGB565
                    p_processor->jpegd_postprocess_func = &color_convert_rot270;
                    p_processor->color_convert = H2V2_TO_RGB565_R270;
                    p_processor->nRGBAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                case RGB888:                  //H2V2-->RGB888
                    p_processor->jpegd_postprocess_func = &color_convert_rot270;
                    p_processor->color_convert = H2V2_TO_RGB888_R270;
                    p_processor->nRGBAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                case RGBa:                    //H2V2-->RGBa
                    p_processor->jpegd_postprocess_func = &color_convert_rot270;
                    p_processor->color_convert = H2V2_TO_RGBa_R270;
                    p_processor->nRGBAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                default:
                    break;
                }

                break;

            // Input format H2V1
            case JPEG_H2V1:

                switch (p_dest->output_format)
                {
                case YCRCBLP_H2V2:            //H2V1-->H2V2
                    p_processor->jpegd_postprocess_func = &data_move_rot270;
                    p_processor->data_move = H2V1_TO_H2V2_R270;
                    p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight/2 - p_processor->nStride;
                    break;

                case YCBCRLP_H2V2:            //H2V1-->H2V2_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot270;
                    p_processor->data_move = H2V1_TO_H2V2_R270_CbCr;
                    p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight/2 - p_processor->nStride;
                    break;

                case YCRCBLP_H2V1:            //H2V1-->H2V1
                    p_processor->jpegd_postprocess_func = &data_move_rot270;
                    p_processor->data_move = H2V1_TO_H2V1_R270;
                    p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                case YCBCRLP_H2V1:            //H2V1-->H2V1_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot270;
                    p_processor->data_move = H2V1_TO_H2V1_R270_CbCr;
                    p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                case RGB565:                  //H2V1-->RGB565
                    p_processor->jpegd_postprocess_func = &color_convert_rot270;
                    p_processor->color_convert = H2V1_TO_RGB565_R270;
                    p_processor->nRGBAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                case RGB888:                  //H2V1-->RGB888
                    p_processor->jpegd_postprocess_func = &color_convert_rot270;
                    p_processor->color_convert = H2V1_TO_RGB888_R270;
                    p_processor->nRGBAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                case RGBa:                    //H2V1-->RGBa
                    p_processor->jpegd_postprocess_func = &color_convert_rot270;
                    p_processor->color_convert = H2V1_TO_RGBa_R270;
                    p_processor->nRGBAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                default:
                    break;
                }

                break;


            // Input format H1V2
            case JPEG_H1V2:

                switch (p_dest->output_format)
                {
                case YCRCBLP_H2V2:            //H1V2-->H2V2
                    p_processor->jpegd_postprocess_func = &data_move_rot270;
                    p_processor->data_move = H1V2_TO_H2V2_R270;
                    p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight/2 - p_processor->nStride;
                    break;

                case YCBCRLP_H2V2:            //H1V2-->H2V2_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot270;
                    p_processor->data_move = H1V2_TO_H2V2_R270_CbCr;
                    p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight/2 - p_processor->nStride;
                    break;

                case YCRCBLP_H2V1:            //H1V2-->H2V1
                    p_processor->jpegd_postprocess_func = &data_move_rot270;
                    p_processor->data_move = H1V2_TO_H2V1_R270;
                    p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                case YCBCRLP_H2V1:            //H1V2-->H2V1_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot270;
                    p_processor->data_move = H1V2_TO_H2V1_R270_CbCr;
                    p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                case RGB565:                  //H1V2-->RGB565
                    p_processor->jpegd_postprocess_func = &color_convert_rot270;
                    p_processor->color_convert = H1V2_TO_RGB565_R270;
                    p_processor->nRGBAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                case RGB888:                  //H1V2-->RGB888
                    p_processor->jpegd_postprocess_func = &color_convert_rot270;
                    p_processor->color_convert = H1V2_TO_RGB888_R270;
                    p_processor->nRGBAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                case RGBa:                    //H1V2-->RGBa
                    p_processor->jpegd_postprocess_func = &color_convert_rot270;
                    p_processor->color_convert = H1V2_TO_RGBa_R270;
                    p_processor->nRGBAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                default:
                    break;
                }

                break;


            // Input format H1V1
            case JPEG_H1V1:

                switch (p_dest->output_format)
                {
                case YCRCBLP_H2V2:            //H1V1-->H2V2
                    p_processor->jpegd_postprocess_func = &data_move_rot270;
                    p_processor->data_move = H1V1_TO_H2V2_R270;
                    p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight/2 - p_processor->nStride;
                    break;

                case YCBCRLP_H2V2:            //H1V1-->H2V2_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot270;
                    p_processor->data_move = H1V1_TO_H2V2_R270_CbCr;
                    p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight/2 - p_processor->nStride;
                    break;

                case YCRCBLP_H2V1:            //H1V1-->H2V1
                    p_processor->jpegd_postprocess_func = &data_move_rot270;
                    p_processor->data_move = H1V1_TO_H2V1_R270;
                    p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                case YCBCRLP_H2V1:            //H1V1-->H2V1_CbCr
                    p_processor->jpegd_postprocess_func = &data_move_rot270;
                    p_processor->data_move = H1V1_TO_H2V1_R270_CbCr;
                    p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                case RGB565:                  //H1V1-->RGB565
                    p_processor->jpegd_postprocess_func = &color_convert_rot270;
                    p_processor->color_convert = H1V1_TO_RGB565_R270;
                    p_processor->nRGBAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                case RGB888:                  //H1V1-->RGB888
                    p_processor->jpegd_postprocess_func = &color_convert_rot270;
                    p_processor->color_convert = H1V1_TO_RGB888_R270;
                    p_processor->nRGBAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                case RGBa:                    //H1V1-->RGBa
                    p_processor->jpegd_postprocess_func = &color_convert_rot270;
                    p_processor->color_convert = H1V1_TO_RGBa_R270;
                    p_processor->nRGBAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                    break;

                default:
                    break;
                }

                break;

            default:
                break;

            } //end switch (p_engine_dst->subsampling)

        }
        else // Output dimension < input dimension ==> Down Scaling
        {
            /**********************************************************************
            * The down scaling algorithm is based on the un-rotated output
            * image dimensions. For 90/270 rotation cases we need to switch
            * the output image width/height for the down scalar to be working.
            * The member variable m_nHeight will save the original output height.
            *********************************************************************/

            p_processor->jpegd_postprocess_func = &down_scale;

            switch (p_dest->output_format)
            {
            case YCRCBLP_H2V2:
                p_processor->down_scale = DS_H2V2_R270;
                p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight/2 - p_processor->nStride;
                break;

            case YCRCBLP_H2V1:
                p_processor->down_scale = DS_H2V1_R270;
                p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                break;

            case YCBCRLP_H2V2:
                p_processor->down_scale = DS_H2V2_R270_CbCr;
                p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight/2 - p_processor->nStride;
                break;

            case YCBCRLP_H2V1:
                p_processor->down_scale = DS_H2V1_R270_CbCr;
                p_processor->nLumaAddressOffset  = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                p_processor->nChromaAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                break;

            case RGB565:
                p_processor->down_scale = DS_RGB565_R270;
                p_processor->nRGBAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                break;

            case RGB888:
                p_processor->down_scale = DS_RGB888_R270;
                p_processor->nRGBAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                break;

            case RGBa:
                p_processor->down_scale = DS_RGBa_R270;
                p_processor->nRGBAddressOffset = p_processor->nStride * p_processor->nOutputHeight - p_processor->nStride;
                break;

            default:
                break;
            }

            swap(&(p_processor->nOutputWidth), &(p_processor->nOutputHeight));
        }

        break; // case 270

        default:
            break;

    } // End switch (nRotationClockwiseInDegree)


    if ( (p_processor->jpegd_postprocess_func == NULL) ||
         (p_processor->data_move == DATA_MOVE_MAX &&
          p_processor->color_convert == COLOR_CONVERT_MAX &&
          p_processor->down_scale == DOWN_SCALE_MAX) )
    {
        return JPEGERR_EUNSUPPORTED;
    }

    return JPEGERR_SUCCESS;

}



