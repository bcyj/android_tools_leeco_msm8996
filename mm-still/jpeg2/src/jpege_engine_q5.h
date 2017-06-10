/*========================================================================


*//** @file jpege_engine_q5.h

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (c) 2008 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary.  Export of this technology or software is regulated
by the U.S. Government, Diversion contrary to U.S. law prohibited.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
10/22/09   vma     Export the engine 'profile' for easier engine picking.
08/03/09   vma     Switched to use the os abstraction layer (os_*)
10/08/08   vma     Took out turning on of VDC clock (taken care of by kernel
                   when JPEGTASK is opened)
07/29/08   vma     Created file.

========================================================================== */

#ifndef _JPEGE_ENGINE_Q5_H
#define _JPEGE_ENGINE_Q5_H

#include "os_thread.h"
#include "jpeg_common.h"
#include "jpege_engine.h"
#include "jpege.h"
#include "jpeg_buffer_private.h"
#include "jpeg_q5_helper.h"

extern jpege_engine_profile_t jpege_engine_q5_profile;

typedef enum
{
    JPEGE_Q5_UNINITIALIZED, // Uninitialized state
    JPEGE_Q5_IDLE,          // Idle state
    JPEGE_Q5_ENCODING,      // Encoding
    JPEGE_Q5_ENCODE_WAIT,   // Waiting for ENCODE_ACK from dsp
    JPEGE_Q5_DONE_WAIT,     // Waiting for IDLE_ACK from dsp after encoding is done
    JPEGE_Q5_ABORT_WAIT,    // Waiting for IDLE_ACK from dsp after encoding is aborted

} jpege_engine_q5_state_t;

typedef struct
{
    jpege_engine_obj_t           *p_wrapper;      // The wrapper engine object
    os_thread_t                   thread;         // The encode thread
    os_mutex_t                    mutex;          // os mutex object
    os_cond_t                     cond;           // os condition variable
    jpege_engine_dst_t            dest;           // Destination object
    jpege_engine_event_handler_t  p_handler;      // Event handler
    jpege_engine_q5_state_t       state;          // Engine state
    jpeg_q5_helper_t              q5_helper;      // Helper object to communicate with q5
    jpeg_buf_t                   *p_out_bufs[2];  // Internal output buffers in case none is supplied


} jpege_engine_q5_t;

#endif /* _JPEGE_ENGINE_DSP_H */
