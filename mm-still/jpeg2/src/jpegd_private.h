/*============================================================================

   Copyright  2011 Qualcomm Technologies, Inc. All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/


#ifndef __JPEGD_PRIV_H__
#define __JPEGD_PRIV_H__

#include "jpeg_reader.h"
#include "jpeg_buffer_private.h"
#include "jpeg_postprocessor.h"

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define INPUT_REQ_Q_SIZE       2

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef enum
{
    JPEGD_IDLE = 0,
    JPEGD_DECODING = 1,
    JPEGD_ABORTING = 2,

} jpegd_state_t;

typedef struct
{
    jpeg_buf_t  *p_buffer;
    uint32_t     start_offset;
    uint32_t     length;
    uint8_t      is_valid;

} jpegd_input_request_t;


typedef struct jpeg_decoder_t
{
    // The Jpeg Reader
    jpeg_reader_t          reader;
    // The Jpeg Postprocessor
    jpeg_postprocessor_t   processor;
    // The Jpeg Decode Engine
    jpegd_engine_obj_t     engine;
    // The Jpeg event handler
    jpegd_event_handler_t  p_event_handler;
    // The input request handler
    jpegd_input_req_handler_t p_input_req_handler;
    // The pointer to user supplied data (store untouched)
    void                  *p_user_data;
    // The Jpeg decoder source
    jpegd_src_t            source;
    // The Jpeg decoder destination
    jpegd_dst_t            dest;
    // The Jpeg decode engine source
    jpegd_engine_src_t     engine_src;
    // The Jpeg decode engine destination
    jpegd_engine_dst_t     engine_dst;
    // The state of the decoder
    jpegd_state_t          state;
    // os thread to handle input requests
    os_thread_t            thread;
    // os mutex
    os_mutex_t             mutex;
    // os condition variable
    os_cond_t              cond;
    // Flag to signal the input request thread to exit
    uint8_t                thread_exit_flag;
    // Full Jpeg header
    jpeg_header_t         *p_full_hdr;
    // Index of next ping-pong input buffer to be used to receive input
    uint8_t                next_input_idx;
    // Input request queue
    jpegd_input_request_t  input_requests[INPUT_REQ_Q_SIZE];
    // Input request queue head
    uint16_t               input_req_q_head;
    // Input request queue tail
    uint16_t               input_req_q_tail;

} jpeg_decoder_t;

#endif // #ifndef __JPEGD_PRIV_H__
