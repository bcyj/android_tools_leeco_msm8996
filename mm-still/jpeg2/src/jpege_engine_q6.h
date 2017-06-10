/*****************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential. *
*****************************************************************************/

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
07/09/13   staceyw  Created file.

========================================================================== */

#ifndef _JPEGE_ENGINE_Q6_H
#define _JPEGE_ENGINE_Q6_H

#include "os_thread.h"
#include "jpeg_common.h"
#include "jpege_engine_hybrid.h"
#include "jpeg_buffer_private.h"
#include "adsp_jpege.h"

typedef struct
{
    jpege_engine_hybrid_obj_t    *p_wrapper;      // The wrapper engine object
    os_thread_t                   thread;         // The encode thread
    os_mutex_t                    mutex;          // os mutex object
    os_cond_t                     cond;           // os condition variable
    uint8_t                       abort_flag;     // abort flag
    uint8_t                       error_flag;     // error flag
    jpege_q6_enc_cfg_target_t     q6_enc_target_cfg;
    uint8_t                      *output_buffer_ptr;
    uint8_t                      *plane_ptr[4];
    uint32_t                      plane_ptr_size[4];
    uint8_t                       is_active;      // Flag indicating whether the engine is
                                                  // actively encoding or not
    jpege_engine_hybrid_event_handler_t   p_event_handler;   // the event handler
    jpege_engine_hybrid_output_handler_t  p_output_handler;  // the output handler

} jpege_engine_q6_t;

#endif /* _JPEGE_ENGINE_DSP_H */
