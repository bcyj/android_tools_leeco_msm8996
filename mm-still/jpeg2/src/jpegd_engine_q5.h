/*========================================================================


*//** @file jpegd_engine_q5.h 

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
10/22/09   vma     Export the engine 'profile' for easier engine picking.
08/03/09   vma     Switched to use the os abstraction layer (os_*)
02/26/09   vma     Took away destination buffer lookup
10/08/08   vma     Took out turning on of VDC clock (taken care of by kernel
                   when JPEGTASK is opened)
07/29/08   vma     Created file.

========================================================================== */

#ifndef _JPEGD_ENGINE_Q5_H
#define _JPEGD_ENGINE_Q5_H

#include "os_thread.h"
#include "jpeg_common.h"
#include "jpegd_engine.h"
#include "jpegd.h"
#include "jpeg_buffer_private.h"
#include "jpeg_q5_helper.h"
#include "jpegd_engine_utils.h"

#define  EVENT_Q_SIZE 5

extern jpegd_engine_profile_t jpegd_engine_q5_profile;

typedef enum
{
    JPEGD_Q5_UNINITIALIZED, // Uninitialized state
    JPEGD_Q5_IDLE,          // Idle state
    JPEGD_Q5_DECODING,      // Decoding
    JPEGD_Q5_DECODE_WAIT,   // Waiting for DECODE_ACK from dsp
    JPEGD_Q5_DONE_WAIT,     // Waiting for IDLE_ACK from dsp after decoding is done
    JPEGD_Q5_ABORT_WAIT,    // Waiting for IDLE_ACK from dsp after decoding is aborted
    JPEGD_Q5_ERROR_WAIT,    // Waiting for IDLE_ACK from dsp after error was encountered
                          
} jpegd_engine_q5_state_t;

typedef struct
{
    jpeg_event_t  event;
    void         *payload;
    uint8_t       is_valid;

} jpegd_engine_event_q_entry; // entries in the event queue

typedef struct
{ 
    jpegd_engine_obj_t              *p_wrapper;             // The wrapper engine object
    uint8_t                          thread_running;        // Flag indicating whether thread is running
    uint8_t                          thread_exit_flag;      // Flag indicating the thread should exit
    os_thread_t                      thread;                // The decode thread
    os_mutex_t                       mutex;                 // os mutex object
    os_cond_t                        cond;                  // os condition variable
    jpegd_engine_src_t               source;                // Source object                                                          
    jpegd_engine_dst_t               dest;                  // Destination object    
    jpegd_engine_event_handler_t     p_event_handler;       // The event handler        
    jpegd_engine_q5_state_t          state;                 // Engine state
    jpeg_q5_helper_t                 q5_helper;             // Helper object to communicate with q5    
    uint32_t                         scaling_factor;        // The downscale factor
    jpegd_engine_input_fetcher_t     input_fetcher;         // The input fetch helper
    jpegd_engine_event_q_entry       event_q[EVENT_Q_SIZE]; // Event queue
    uint32_t                         event_q_head;          // Head of event queue
    uint32_t                         event_q_tail;          // Tail of event queue
    uint32_t                         dst_buf_index;         // Index of the next destination buffer to use

} jpegd_engine_q5_t;

#endif /* _JPEGD_ENGINE_DSP_H */
