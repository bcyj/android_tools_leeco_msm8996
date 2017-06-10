/*========================================================================


*//** @file jpegd_engine_utils.h

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
07/07/09   zhiminl Decoupled bit buffer from jpegd_engine_input_fetcher_t.
07/07/08   vma     Created file.

========================================================================== */

#ifndef _JPEGD_ENGINE_UTILS_H
#define _JPEGD_ENGINE_UTILS_H

#include "os_thread.h"
#include "jpeg_common.h"
#include "jpegd_engine.h"
#include "jpegd.h"

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef struct
{
    // The wrapper engine object
    jpegd_engine_obj_t              *p_wrapper;
    // The source object
    jpegd_engine_src_t              *p_source;
    // The input request handler
    jpegd_engine_input_req_handler_t p_input_req_handler;
    // Index of the input buffer in use
    uint32_t                         buffer_in_use;
    // Offset of the next byte to be fetched
    uint32_t                         next_byte_to_fetch_offset;
    // Pointer to the fetched stream
    uint8_t                         *p_fetched_bytes;
    // Number of bytes left in the fetched stream
    int32_t                          num_bytes_left;
    // Number of bytes fetched
    int32_t                          num_bytes_fetched;
    // Flag indicating whether end of file is reached
    uint8_t                          last_byte_fetched;

} jpegd_engine_input_fetcher_t;

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
**                          Function Declarations
** ======================================================================= */
int  jpegd_engine_input_fetcher_fetch(jpegd_engine_input_fetcher_t *p_fetcher);
int jpegd_engine_fetcher_get_buf_in_use(jpegd_engine_input_fetcher_t *p_fetcher,
                                        jpeg_buf_t *p_buf);
#endif /* _JPEGD_ENGINE_UTILS_H */
