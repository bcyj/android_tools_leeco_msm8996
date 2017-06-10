/*========================================================================


*//** @file jpegd_engine_utils.c  

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
08/03/09   vma     Switched to use the os abstraction layer (os_*)
07/07/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpeg_buffer_private.h"
#include "jpegd_engine.h"
#include "jpeg_writer.h"
#include "jpegd.h"
#include "jpegerr.h"
#include "jpegd_engine_utils.h"
#include <stdlib.h>

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
const int32_t zag[64] = { 
    0,  1,  8,  16, 9,  2,  3,  10,
    17, 24, 32, 25, 18, 11, 4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63};
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
void jpegd_engine_fetcher_init(jpegd_engine_input_fetcher_t     *p_fetcher,
                               jpegd_engine_obj_t               *p_wrapper, 
                               jpegd_engine_input_req_handler_t  p_input_req_handler)
{
    STD_MEMSET(p_fetcher, 0, sizeof(jpegd_engine_input_fetcher_t));
    p_fetcher->p_wrapper           = p_wrapper;
    p_fetcher->p_input_req_handler = p_input_req_handler;
}


int jpegd_engine_fetcher_get_buf_in_use(jpegd_engine_input_fetcher_t *p_fetcher,
                                        jpeg_buf_t *p_buf)
{
    p_buf = p_fetcher->p_source->p_buffers[p_fetcher->buffer_in_use];
    return JPEGERR_SUCCESS;
}


int jpegd_engine_input_fetcher_fetch(jpegd_engine_input_fetcher_t *p_fetcher)
{
    int rc;
    jpeg_buf_t *p_buf_in_use;

    // Mark the current input buffer consumed
    jpeg_buffer_mark_empty(p_fetcher->p_source->p_buffers[p_fetcher->buffer_in_use]);

    // Switch to use the other input buffer as the current buffer
    p_fetcher->buffer_in_use = 1 - p_fetcher->buffer_in_use;
    p_buf_in_use = p_fetcher->p_source->p_buffers[p_fetcher->buffer_in_use];

    // Make input request if the current buffer is empty and is not being filled
    os_mutex_lock(&(p_buf_in_use->mutex));
    if (!p_buf_in_use->is_busy && p_buf_in_use->is_empty)
    {
        p_buf_in_use->is_busy = true;
        os_mutex_unlock(&(p_buf_in_use->mutex));
        rc = p_fetcher->p_input_req_handler(p_fetcher->p_wrapper->decoder,
                                            p_buf_in_use, p_fetcher->next_byte_to_fetch_offset,
                                            STD_MIN(p_buf_in_use->size, 
                                                    p_fetcher->p_source->total_length - 
                                                    p_fetcher->next_byte_to_fetch_offset));
        if (JPEG_FAILED(rc))
        {
            return rc;
        }
    }
    else
    {
        os_mutex_unlock(&(p_buf_in_use->mutex));
    }

    // Wait for the current input buffer to be marked as filled
    jpeg_buffer_wait_until_filled(p_buf_in_use);

    p_fetcher->p_fetched_bytes = p_buf_in_use->ptr;
    p_fetcher->num_bytes_left = p_buf_in_use->offset;

    // update fetch off set for next fetch
    p_fetcher->next_byte_to_fetch_offset += p_buf_in_use->offset;

    p_fetcher->last_byte_fetched = (p_fetcher->next_byte_to_fetch_offset >= p_fetcher->p_source->total_length);

    // See if we should pre-fetch for the next buffer
    if (!p_fetcher->last_byte_fetched)
    {
        jpeg_buf_t *p_next_buf = p_fetcher->p_source->p_buffers[1 - p_fetcher->buffer_in_use];
        // Mark the buffer as empty and busy
        jpeg_buffer_mark_empty(p_next_buf);
        jpeg_buffer_mark_busy(p_next_buf);

        rc = p_fetcher->p_input_req_handler(p_fetcher->p_wrapper->decoder,
                                            p_next_buf, p_fetcher->next_byte_to_fetch_offset,
                                            STD_MIN(p_next_buf->size, 
                                                    p_fetcher->p_source->total_length - 
                                                    p_fetcher->next_byte_to_fetch_offset));
        if (JPEG_FAILED(rc))
        {
            return rc;
        }
    }

    return JPEGERR_SUCCESS;
}

