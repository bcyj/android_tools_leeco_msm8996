/*========================================================================


*//** @file jpege_engine_bs.h

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2009-2011, 2014 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
07/13/10   zhiminl Integrated with PPF to support rotation and scaling.
02/16/10   staceyw Add two flag and cond for last buffer output to fix
                   syncronization with encode and output two different threads.
02/08/10   staceyw Added bitstream engine to support bitstream input.
11/11/09   staceyw Created file.

========================================================================== */

#ifndef _JPEGE_ENGINE_BS_H
#define _JPEGE_ENGINE_BS_H

#include "os_thread.h"
#include "jpeg_common.h"
#include "jpege_engine.h"
#include "jpege.h"
#if 0
#include "ppf.h"
#endif
/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define PENDING_OUTPUT_Q_SIZE 2

typedef struct
{
    jpeg_buf_t  *p_buffer;
    uint8_t      is_valid;
    // flag for last buffer
    uint8_t      is_last_buffer;

} jpege_engine_bs_output_t;

typedef struct
{
    jpege_engine_obj_t  *p_wrapper;             // The wrapper engine object
    os_mutex_t           mutex;                 // os mutex object
    os_cond_t            cond;                  // os condition variable
    os_cond_t            output_cond;           // os condition variable (for output signaling)
    os_cond_t            consume_cond;          // os condition variable (for output thread produce signaling)
    os_cond_t            final_output_cond;     // os condition variable (for output last buffer flush signaling)
    os_thread_t          thread;                // The encode thread
    os_thread_t          output_thread;         // The output thread
    jpege_engine_dst_t   dest;                  // the destination object

    jpege_engine_event_handler_t p_handler;     // the event handler

    uint8_t              is_active;             // Flag indicating whether the engine is
                                                // actively encoding or not
    uint8_t              thread_exit_flag;      // Flag indicating whether the pending
                                                // output thread should exit

    jpege_engine_bs_output_t pending_outputs[PENDING_OUTPUT_Q_SIZE]; // Pending output queue
    uint16_t             pending_output_q_head; // Pending output queue head
    uint16_t             pending_output_q_tail; // Pending output queue tail

    uint8_t             *p_input_bitstream;     // Input bitstream buffer ptr
    uint32_t             bitstream_size;        // Bitstream size

    jpeg_buf_t          *p_dest_buffer;         // Destination buffer
#if 0
    uint8_t              use_ppf;               // Flag indicating whether PPF is needed
                                                // for post-processing input bitstream
    ppf_obj_t            ppf;                   // The PPF object for post-processing
                                                // input bitstream
    ppf_image_t          ppf_dest;              // The PPF output image
#endif
    uint8_t              abort_flag;            // abort flag
    uint8_t              error_flag;            // error flag
    uint8_t              final_output_flag;     // final output flag

} jpege_engine_bs_t;

#endif /* _JPEGE_ENGINE_BS_H */
