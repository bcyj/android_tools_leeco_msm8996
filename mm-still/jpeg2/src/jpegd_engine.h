/*========================================================================


*//** @file jpegd_engine.h

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
12/02/09   mingy   Changed engine output handler return type to int.
10/22/09   vma     Changed the way engines are picked (using lists)
09/21/09   mingy   Added retion defition in jpegd_engine_dst_t structure.
07/07/08   vma     Created file.

========================================================================== */

#ifndef _JPEGD_ENGINE_H
#define _JPEGD_ENGINE_H

#include "jpeg_common.h"
#include "jpeg_header.h"
#include "jpegd.h"
#include "jpeg_buffer_private.h"

typedef struct jpegd_engine_obj_struct jpegd_engine_obj_t;

/* The Jpeg Decode Engine event handler definition */
typedef void(*jpegd_engine_event_handler_t)(
    jpegd_engine_obj_t *p_obj,  // The decode engine object calling it
    jpeg_event_t        event,  // The event code
    void               *p_arg); // An optional argument for that event.
                                // JPEG_EVENT_DONE:    unused
                                // JPEG_EVENT_WARNING: const char * (warning message)
                                // JPEG_EVENT_ERROR:   const char * (error message)

/**
 *  The handler for serving input requests from the Jpeg Decode
 *  Engine. The engine expects the buffer would be filled when
 *  the call returns. JPEGERR_SUCCESS is returned if operation
 *  is successful. */
typedef int(*jpegd_engine_input_req_handler_t)(jpegd_obj_t     obj,
                                               jpeg_buf_t     *p_buffer,
                                               uint32_t        start_offset,
                                               uint32_t        length);

/* The handler for dealing with output produced by the Jpeg Decode Engine */
typedef int(*jpegd_engine_output_handler_t)(
    jpegd_engine_obj_t  *p_obj,         // The decode engine object calling it
    jpeg_buf_t          *p_luma_buf,    // The buffer containing the luma output
    jpeg_buf_t          *p_chroma_buf); // The buffer containing the chroma output

/* The Jpeg Decode Engine source type */
typedef struct
{
    jpegd_engine_input_req_handler_t p_input_req_handler;
    jpeg_buf_t *p_buffers[2];
    jpeg_frame_info_t *p_frame_info;
    uint32_t total_length;
    uint32_t hw_rotation;
    uint32_t hw_scale_factor;
    uint32_t num_planes;
} jpegd_engine_src_t;

/* The Jpeg Decode Engine destination type */
typedef struct
{
    // The handler function when output data is ready to be written
    jpegd_engine_output_handler_t  p_output_handler;

    // The supplied destination buffers.
    jpeg_buf_t                    *p_luma_buffers[2];
    jpeg_buf_t                    *p_chroma_buffers[2];

    // The output dimension
    uint32_t                       width;
    uint32_t                       height;

    // The output subsampling
    jpeg_subsampling_t             subsampling;

    // The output region
    jpeg_rectangle_t               region;

    /* output buf array*/
    jpegd_output_buf_t *p_output_buf_array;
} jpegd_engine_dst_t;

/* Function to create a Jpeg Decode Engine */
typedef void(*jpegd_engine_create_t)(
    jpegd_engine_obj_t *p_obj,
    jpegd_obj_t decoder);

/* Profile of a Jpeg Decode Engine */
typedef struct
{
    jpegd_engine_create_t create_func;
    const char*           engine_name;
    uint32_t              need_pmem;

} jpegd_engine_profile_t;

/* List of Jpeg Decode Engines available and the order they should be picked */
/* It contains pointers to different engine profile(s) and is null-terminated */
typedef jpegd_engine_profile_t** jpegd_engine_list_t;

/* Collection of engine lists; one for each possible engine pick preference */
typedef jpegd_engine_list_t jpegd_engine_lists_t[JPEG_DECODER_PREF_MAX];

/******************************************************************************
 * Function: jpegd_engine_init_t
 * Description: Initializes the Jpeg Decode Engine object. Dynamic allocations
 *              take place during the call. One should always call destroy to
 *              clean up the object after use.
 * Input parameters:
 *   p_obj     - The pointer to the Jpeg Decode Engine object to be initialized.
 *   p_handler - The function pointer to the handler function handling Jpeg
 *               Decoder events.
 * Return values:
 *     JPEGDRR_SUCCESS
 *     JPEGDRR_ENULLPTR
 *     JPEGDRR_EFAILED
 * (See jpegdrr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
typedef int(*jpegd_engine_init_t)(
    jpegd_engine_obj_t               *p_obj,
    jpegd_engine_event_handler_t      p_event_handler,
    jpegd_engine_input_req_handler_t  p_input_req_handler);

/******************************************************************************
 * Function: jpegd_engine_configure_t
 * Description: todo
 * Input parameters: todo
 * Return values: todo
 * (See jpegdrr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
typedef int(*jpegd_engine_configure_t)(
    jpegd_engine_obj_t   *p_obj,
    jpegd_engine_src_t   *p_source,
    jpegd_engine_dst_t   *p_dest,
    jpegd_dst_t          *p_jpegd_dst,
    uint32_t             *p_chunk_width,
    uint32_t             *p_chunk_height);

/******************************************************************************
 * Function: jpegd_engine_start_t
 * Description: Starts the Jpeg Decode Engine. Decoding will start asynchronously
 *              in a new thread if the call returns JPEGDRR_SUCCESS. The caller
 *              thread should interact asynchronously with the engine through
 *              event and output handlers.
 * Input parameters:
 *   p_obj           - The pointer to the Jpeg Decode Engine object.
 * Return values:
 *     JPEGDRR_SUCCESS
 *     JPEGDRR_EFAILED
 * (See jpegdrr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
typedef int(*jpegd_engine_start_t)(jpegd_engine_obj_t *p_obj);

/******************************************************************************
 * Function: jpegd_engine_abort_t
 * Description: Aborts the decoding session in progress. If there is no decoding
 *              session in progress, a JPEGDRR_SUCCESS will be returned
 *              immediately. Otherwise, the call will block until the decoding
 *              is completely aborted, then return a JPEGDRR_SUCCESS.
 * Input parameters:
 *   p_obj  - The pointer to the Jpeg Decode Engine object.
 * Return values:
 *     JPEGDRR_SUCCESS
 *     JPEGDRR_ENULLPTR
 *     JPEGDRR_EFAILED
 * (See jpegdrr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
typedef int(*jpegd_engine_abort_t)(
    jpegd_engine_obj_t *p_obj);

/******************************************************************************
 * Function: jpegd_engine_destroy_t
 * Description: Destroys the Jpeg Decode Engine object, releasing all memory
 *              previously allocated.
 * Input parameters:
 *   p_obj  - The pointer to the Jpeg Decode Engine object.
 * Return values:
 *     JPEGDRR_SUCCESS
 *     JPEGDRR_ENULLPTR
 * (See jpegdrr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
typedef void(*jpegd_engine_destroy_t)(
    jpegd_engine_obj_t  *p_obj);

/* Definition of Jpeg Decode Engine object */
struct jpegd_engine_obj_struct
{
    jpegd_engine_create_t        create;
    jpegd_engine_init_t          init;
    jpegd_engine_configure_t     configure;
    jpegd_engine_start_t         start;
    jpegd_engine_abort_t         abort;
    jpegd_engine_destroy_t       destroy;
    void                        *p_engine;
    jpegd_obj_t                  decoder;
    uint8_t                      is_intialized;
    uint16_t                     skip_pp;
};

#endif /* _JPEGD_ENGINE_H */
