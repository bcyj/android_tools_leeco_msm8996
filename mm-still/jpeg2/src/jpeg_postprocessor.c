/*========================================================================


*//** @file jpeg_postprocessor.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-09 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
11/08/09   mingy   Added tiling output support.
09/21/09   mingy   Added region based decoding support.
08/03/09   vma     Switched to use the os abstraction layer (os_*)
05/12/09   mingy   Seperated Data Move, Color Convert and Down Scalar from
                   the post process to different individual files.
                   Move Color Convert line functions out.
                   Move the Configuration function out.
                   Move the CLAMP Macro out.
03/05/09   leim    a) Added H1V1, H1V2 sampling format support;
                   b) Added aRGB output format
07/07/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpeg_buffer_private.h"
#include "jpeg_postprocessor.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include <stdlib.h>

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

/* =======================================================================
**                          Function Definitions
** ======================================================================= */
int jpeg_postprocessor_init(jpeg_postprocessor_t   *p_processor,
                            jpegd_output_handler_t  p_output_handler,
                            void                   *p_user_data)
{
    // Validate input arguments
    if (!p_processor)
        return JPEGERR_ENULLPTR;

    // Initialize the fields inside the engine structure below
    STD_MEMSET(p_processor, 0, sizeof(jpeg_postprocessor_t));  // Zero out the entire structure
    os_mutex_init(&(p_processor->mutex));
    os_cond_init(&(p_processor->cond));

    // Assign output handler
    p_processor->p_output_handler = p_output_handler;

    // Store user data
    p_processor->p_user_data = p_user_data;

    return JPEGERR_SUCCESS;
}

int jpeg_postprocessor_dequeue_output_buf(
    jpeg_postprocessor_t *p_processor,
    jpegd_output_buf_t  **p_output_buf)
{
    os_mutex_lock(&(p_processor->mutex));

    /*************************************************************************
    * Block if output buffer Q is empty
    * enqueue function will send a signal to wake it up,
    * someone else like abort may also send a signal to wake it up.
    *************************************************************************/
    if (!p_processor->output_buffers[p_processor->output_buf_q_head].is_in_q)
    {
        os_cond_wait(&(p_processor->cond), &(p_processor->mutex));
    }

    /**************************************************************************
    * Check again if the Q is empty
    * if the above blocking was waked up by the enqueue function,
    * the is_in_q flag is set, go ahead and dequeue the buffer;
    * if it was waked up by someone other than the enqueue function,
    * the is_in_q flag would not be set, then it should return failed to
    * dequeue.
    **************************************************************************/
    if (!p_processor->output_buffers[p_processor->output_buf_q_head].is_in_q)
    {
        os_mutex_unlock(&(p_processor->mutex));

        return JPEGERR_EFAILED;
    }
    else
    {
        // Dequeue one output buffer from the queue
        *p_output_buf = p_processor->output_buffers + p_processor->output_buf_q_head;
        (*p_output_buf)->is_in_q = false;
        p_processor->output_buf_q_head = (p_processor->output_buf_q_head + 1) % OUTPUT_BUF_Q_SIZE;

        os_mutex_unlock(&(p_processor->mutex));

        return JPEGERR_SUCCESS;
    }
}

int jpeg_postprocessor_process(jpeg_postprocessor_t *p_processor,
                                jpeg_buf_t *p_luma_buf,
                                jpeg_buf_t *p_chroma_buf)
{
    uint32_t i;
    int rc;

    // The output buffer struct to be dequeued from the Q
    jpegd_output_buf_t * p_output_buf;

    // The address of the buffer in the output buffer struct
    uint8_t *tiling_buf_ptr;

    jpegd_postprocess_func_param_t postprocess;

    // Zero out all fields
    STD_MEMSET(&postprocess, 0, sizeof(jpegd_postprocess_func_param_t));

    postprocess.p_processor = p_processor;

    // get the number of rows/cols to process
    postprocess.nRowsToProcess = STD_MIN(p_processor->chunk_height,
                                         p_processor->nInputHeight - p_processor->nRowsProcessed);

    postprocess.nColsToProcess = STD_MIN(p_processor->chunk_width,
                                         p_processor->nInputWidth - p_processor->nColsProcessed);

    // if no rows to process, it is done.
    if (!postprocess.nRowsToProcess)
    {
        return JPEGERR_SUCCESS;
    }

    // get the source and destination address pointer
    postprocess.pSrcLuma   = p_luma_buf->ptr;
    postprocess.pSrcChroma = p_chroma_buf->ptr;

    //Skipping the rows in the first MCU row but above the region
    if ((p_processor->p_dest->region.top != 0) && p_processor->is_first_row)
    {
        postprocess.nRowsToProcess = p_processor->chunk_height - p_processor->lines_to_skip;

        postprocess.pSrcLuma   += p_processor->region_top_offset_in_mcu_row_y;
        postprocess.pSrcChroma += p_processor->region_top_offset_in_mcu_row_crcb;

        p_processor->is_first_row = false;
    }

    // Skipping the pixels in the MCU Row but to the left of the region starting point
    postprocess.pSrcLuma   += p_processor->region_left_offset_in_mcu_row_y;
    postprocess.pSrcChroma += p_processor->region_left_offset_in_mcu_row_crcb;

    if (p_processor->p_dest->output_format == RGB565 ||
        p_processor->p_dest->output_format == RGB888 ||
        p_processor->p_dest->output_format == RGBa)
    {
        if (p_processor->tiling_enabled)
        {
            postprocess.pDstRGB = p_processor->p_internal_buf;
        }
        else
        {
            postprocess.pDstRGB = p_processor->pDestinationRGB;
        }
    }
    else
    {
        postprocess.pDstLuma   = p_processor->pDestinationLuma;
        postprocess.pDstChroma = p_processor->pDestinationChroma;
    }

    // Execute the post process function
    p_processor->jpegd_postprocess_func(&postprocess);

    /*************************************************************************
    *                 Sending the output
    *  1. dequeue the tiling buffer (one line)
    *  2. copy the color converted data from the internal rgb buffer
    *     to the tiling buffer
    *  3. sending the filled tiling buffer to the upper layer
    *************************************************************************/
    if (p_processor->tiling_enabled)
    {
        postprocess.pDstRGB = p_processor->p_internal_buf;

        for (i = 0; i < postprocess.nRowsToProcess; i++)
        {
            // Check abort flag before dequeuing.
            // dequeue is a blocking call. If user called abort, we should not
            // dequeue here. Otherwise deadlock may happen.
            if (p_processor->abort_flag)
            {
                return JPEGERR_EFAILED;
            }

            // dequeue one line of tiling buffer
            // return if failed to dequeue (may be triggered by abort)
            if (jpeg_postprocessor_dequeue_output_buf(p_processor, &p_output_buf))
            {
                return JPEGERR_EFAILED;
            }

            // copy the color converted data from the internal rgb buffer
            // to the tiling buffer
            jpeg_buffer_get_addr(p_output_buf->data.rgb.rgb_buf, &tiling_buf_ptr);

            STD_MEMMOVE(tiling_buf_ptr,
                        postprocess.pDstRGB,
                        p_processor->nStride);

            // decide whether this is the last tiling buffer to send
            if (p_processor->first_row_id + p_output_buf->tile_height >= p_processor->nInputHeight)
            {
                p_processor->is_last_buffer = true;
            }

            // sending the filled tiling buffer to the upper layer
            rc = p_processor->p_output_handler(p_processor->p_user_data,
                                               p_output_buf,
                                               p_processor->first_row_id,
                                               p_processor->is_last_buffer);
            if (JPEG_FAILED(rc))
            {
                JPEG_DBG_MED("jpeg_postprocessor_process: output handling failed\n");
                return rc;
            }

            postprocess.pDstRGB += p_processor->nStride;

            // update the first row id
            p_processor->first_row_id += p_output_buf->tile_height;
        }
    }

    // update rows/cols count
    p_processor->nRowsProcessed += postprocess.nRowsToProcess;
    p_processor->nColsProcessed += postprocess.nColsToProcess;

    // update destination address pointers;
    if (p_processor->nColsProcessed == p_processor->nInputWidth)
    {
        // If one MCU line has been processed, move destination pointer
        // to the next line; and reset cols processed.
        p_processor->nColsProcessed = 0;

        if (p_processor->p_dest->output_format == RGB565 ||
            p_processor->p_dest->output_format == RGB888 ||
            p_processor->p_dest->output_format == RGBa)
        {
            p_processor->pDestinationRGB = postprocess.pDstRGB;
        }
        else
        {
            p_processor->pDestinationLuma = postprocess.pDstLuma;
            p_processor->pDestinationChroma = postprocess.pDstChroma;
        }
    }
    else
    {
        // otherwise, move destination pointer for nColsToProcess
        if (p_processor->p_dest->output_format == RGB565 ||
            p_processor->p_dest->output_format == RGB888 ||
            p_processor->p_dest->output_format == RGBa)
        {
            p_processor->pDestinationRGB += postprocess.nColsToProcess;
        }
        else
        {
            p_processor->pDestinationLuma += postprocess.nColsToProcess;
            p_processor->pDestinationChroma += postprocess.nColsToProcess;
        }
    }

    return JPEGERR_SUCCESS;
}

void jpeg_postprocessor_destroy(jpeg_postprocessor_t *p_processor)
{
    if (p_processor)
    {
        JPEG_DBG_LOW("jpeg_postprocessor_destroy\n");
        os_mutex_destroy(&(p_processor->mutex));
        os_cond_destroy(&(p_processor->cond));
        // Free any allocated memory
        JPEG_FREE(p_processor->pChromaSmpOutput);
        JPEG_FREE(p_processor->pLumaAccum);
        JPEG_FREE(p_processor->pChromaAccum);
        JPEG_FREE(p_processor->pTempLumaLine);
        JPEG_FREE(p_processor->pTempChromaLine);
        JPEG_FREE(p_processor->p_internal_buf);
    }
}

