/*========================================================================


*//** @file jpege_engine.h

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-2011,2014 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
10/16/14   vgaur   Added multi-threaded fastCV flag and fastCV buffer pointer.
07/13/10   zhiminl Integrated with PPF to support rotation and scaling
                   for bitstream input.
06/14/10   leim    Added return type to engine output handler callback.
04/16/10   staceyw Added last output buffer flag in jpege_engine_output_handler_t.
02/16/10   staceyw Added handler for receiving output buffer from queue.
10/22/09   vma     Changed the way engines are picked (using lists)
07/07/08   vma     Created file.

========================================================================== */

#ifndef _JPEGE_ENGINE_H
#define _JPEGE_ENGINE_H

#include "jpeg_common.h"
#include "jpege.h"
#include "jpeg_buffer_private.h"

typedef struct jpege_engine_obj_struct jpege_engine_obj_t;

/* The Jpeg Encode Engine event handler definition */
typedef void(*jpege_engine_event_handler_t)(
    jpege_engine_obj_t *p_obj,  // The encode engine object calling it
    jpeg_event_t        event,  // The event code
    void               *p_arg); // An optional argument for that event.
                                // JPEG_EVENT_DONE:    unused
                                // JPEG_EVENT_WARNING: const char * (warning message)
                                // JPEG_EVENT_ERROR:   const char * (error message)

/* The handler for dealing with output produced by the Jpeg Encode Engine */
typedef int (*jpege_engine_output_handler_t)(
    jpege_engine_obj_t  *p_obj,    // The encode engine object calling it
    jpeg_buf_t          *p_buf,    // The buffer containing the output
    uint8_t              last_buf_flag); // The flag to indicate output last buffer

/* The handler for dealing with receiving output buffer from buffer queue */
typedef int (*jpege_engine_get_output_buffer_handler_t)(
     jpege_engine_obj_t  *p_obj,          // The encode object calling it
     jpeg_buf_t          **pp_buf);       // The vaid output buffer from queue

/* The Jpeg Encode Engine destination type */
typedef struct
{
    // The handler function when output data is ready to be written
    jpege_engine_output_handler_t  p_output_handler;

    // The handler function to get next output buffer from queue
    jpege_engine_get_output_buffer_handler_t  p_get_output_buffer_handler;

    // The temporary workaround for BS engine to notify service
    // whether quant tables need to be transposed or not, other
    // engines simply ignore it
    uint8_t                        transposed;

} jpege_engine_dst_t;

/* Function to create a Jpeg Encode Engine */
typedef void(*jpege_engine_create_t)(
    jpege_engine_obj_t *p_obj,
    jpege_obj_t encoder);

/* Profile of a Jpeg Encode Engine */
typedef struct
{
    jpege_engine_create_t create_func;
    const char*           engine_name;

} jpege_engine_profile_t;

/* List of Jpeg Encode Engines available and the order they should be picked */
/* It contains pointers to different engine profile(s) and is null-terminated */
typedef jpege_engine_profile_t** jpege_engine_list_t;

/* Collection of engine lists; one for each possible engine pick preference */
typedef jpege_engine_list_t jpege_engine_lists_t[JPEG_ENCODER_PREF_MAX];

/******************************************************************************
 * Function: jpege_engine_init_t
 * Description: Initializes the Jpeg Encode Engine object. Dynamic allocations
 *              take place during the call. One should always call destroy to
 *              clean up the object after use.
 * Input parameters:
 *   p_obj     - The pointer to the Jpeg Encode Engine object to be initialized.
 *   p_handler - The function pointer to the handler function handling Jpeg
 *               Encoder events.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
typedef int(*jpege_engine_init_t)(
    jpege_engine_obj_t           *p_obj,
    jpege_engine_event_handler_t  p_handler);

/******************************************************************************
 * Function: jpege_engine_check_start_param_t
 * Description: Checks the parameters which is going to be passed to
 *              jpege_engine_start_t function to see whether the engine can
 *              support it. It can be called to an engine which is not even
 *              'initialized'. Caller can therefore make this call to determine
 *              whether a particular engine is okay to be used at the earliest
 *              possible time to avoid wasteful resource acquisition (e.g. DSP)
 * Input parameters:
 *   p_cfg           - The pointer to the configuration structure.
 *   p_source        - The pointer to the source of the image.
 *   p_dest          - The pointer to the destination object.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
typedef int(*jpege_engine_check_start_param_t)(
    jpege_img_cfg_t      *p_cfg,
    jpege_img_data_t     *p_source,
    jpege_engine_dst_t   *p_dest);

/******************************************************************************
 * Function: jpege_engine_start_t
 * Description: Starts the Jpeg Encode Engine. Encoding will start asynchronously
 *              in a new thread if the call returns JPEGERR_SUCCESS. The caller
 *              thread should interact asynchronously with the engine through
 *              event and output handlers.
 * Input parameters:
 *   p_obj           - The pointer to the Jpeg Encode Engine object.
 *   p_cfg           - The pointer to the configuration structure.
 *   p_source        - The pointer to the source of the image.
 *   p_dest          - The pointer to the destination object.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
typedef int(*jpege_engine_start_t)(
    jpege_engine_obj_t   *p_obj,
    jpege_img_cfg_t      *p_cfg,
    jpege_img_data_t     *p_source,
    jpege_engine_dst_t   *p_dest);

/******************************************************************************
 * Function: jpege_engine_abort_t
 * Description: Aborts the encoding session in progress. If there is no encoding
 *              session in progress, a JPEGERR_SUCCESS will be returned
 *              immediately. Otherwise, the call will block until the encoding
 *              is completely aborted, then return a JPEGERR_SUCCESS.
 * Input parameters:
 *   p_obj  - The pointer to the Jpeg Encode Engine object.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
typedef int(*jpege_engine_abort_t)(
    jpege_engine_obj_t *p_obj);

/******************************************************************************
 * Function: jpege_engine_destroy_t
 * Description: Destroys the Jpeg Encode Engine object, releasing all memory
 *              previously allocated.
 * Input parameters:
 *   p_obj  - The pointer to the Jpeg Encode Engine object.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
typedef void(*jpege_engine_destroy_t)(
    jpege_engine_obj_t  *p_obj);

/* Definition of Jpeg Encode Engine object */
struct jpege_engine_obj_struct
{
    jpege_engine_create_t              create;
    jpege_engine_init_t                init;
    jpege_engine_check_start_param_t   check_start_param;
    jpege_engine_start_t               start;
    jpege_engine_abort_t               abort;
    jpege_engine_destroy_t             destroy;
    void                              *p_engine;
    void                              *p_hybrid;
    jpege_obj_t                        encoder;
    uint8_t                            is_initialized;
    uint8_t                           *fastCV_buffer;
    uint8_t                            fastCV_flag;
};

#endif /* _JPEGE_ENGINE_H */
