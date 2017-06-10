/*========================================================================


*//** @file jped.h

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
11/18/09   mingy   Added output_buffer type, changed destination structure,
                   defined jpeg decoder output handler and added to
                   jpegd_init().
09/21/09   mingy   Added region definition in jpegd_dst_t structure.
06/02/09   vma     Added "Don't care" decoder preference.
06/01/09   vma     Added user data to be passed back on callbacks.
04/28/09   mingy   Added stride and rotation variables to the structures.
10/09/08   vma     Filled up comments.
07/07/08   vma     Created file.

========================================================================== */

#ifndef _JPEGD_H
#define _JPEGD_H

#include "jpeg_common.h"
#include "jpeg_buffer.h"

#define SQUARE(x) ((x)*(x))

/* Opaque definition of the Jpeg Decoder */
struct jpeg_decoder_t;
typedef struct jpeg_decoder_t *jpegd_obj_t;

/* The choice of where the image should be decoded from (main/thumbnail/auto) */
typedef enum
{
    JPEGD_DECODE_FROM_AUTO = 0,  // Automatically chosen based the desired output dimension
    JPEGD_DECODE_FROM_MAIN,      // Force the decoding on the main image
    JPEGD_DECODE_FROM_THUMB,     // Force the decoding on the thumbnail image (if
                                 // thumbnail is absent, the configuration will fail)
    JPEGD_DECODE_FROM_MAX

} jpegd_decode_from_t;

typedef struct
{
    union
    {
        struct
        {
            jpeg_buffer_t luma_buf;    // Buffer to hold the luma data (Y)
            jpeg_buffer_t chroma_buf;  // Buffer to hold the chroma data (CrCb or CbCr interlaced)
            jpeg_buffer_t chroma2_buf;  // Buffer to hold the 2nd chroma plane in 3 planar case
        } yuv;

        struct
        {
            jpeg_buffer_t rgb_buf;     // Buffer to hold the RGB data
        } rgb;

    } data;

    /*************************************************************************
    * meaningful tile_width and tile_height will enable tiling.
    * set tile_width and tile_height to 0 or full image width and height
    * to disable tiling.
    **************************************************************************/
    uint32_t               tile_width;
    uint32_t               tile_height;

    /*************************************************************************
    * Flag to indicate whether the current buf is in q.
    * This is for internal use only --> please do not modify it.
    * Will be removed very soon.
    **************************************************************************/
    uint8_t                is_in_q;

} jpegd_output_buf_t;

/* The Jpeg Decoder event handler definition */
typedef void(*jpegd_event_handler_t)(void         *p_user_data,  // The user data passed back to client untouched
                                                                 // (originally passed jpegd_init)
                                     jpeg_event_t  event,   // The event code
                                     void         *p_arg);  // An optional argument for that event:
                                                            // JPEG_EVENT_DONE:    unused
                                                            // JPEG_EVENT_WARNING: const char * (warning message)
                                                            // JPEG_EVENT_ERROR:   const char * (error message)

/* The Jpeg Decoder output handler definition */
typedef int(*jpegd_output_handler_t)(void               *p_user_data,     // The user data passed back to client untouched
                                                                          // (originally passed jpegd_init)
                                     jpegd_output_buf_t *p_output_buffer, // The output buffer filled with decoded data
                                     uint32_t            first_row_id,    // The sequence id of the first output buffer row
                                     uint8_t             is_last_buffer); // The flag indicating the last buffer

/**
 *  The handler for serving input requests from the Jpeg
 *  Decoder. The decoder expects the buffer would be filled when
 *  the call returns. The handler should return the number of
 *  bytes written to the buffer.  */
typedef uint32_t(*jpegd_input_req_handler_t)(void          *p_user_data, // The users data passed back to client untouched
                                                                          // (originally passed to jpegd_init)
                                             jpeg_buffer_t  buffer,
                                             uint32_t       start_offset,
                                             uint32_t       length);

/* Jpeg Decoder Preference */
typedef enum
{
    JPEG_DECODER_PREF_HW_ACCELERATED_PREFERRED = 0,
    JPEG_DECODER_PREF_HW_ACCELERATED_ONLY,
    JPEG_DECODER_PREF_SOFTWARE_PREFERRED,
    JPEG_DECODER_PREF_SOFTWARE_ONLY,
    JPEG_DECODER_PREF_MAX,

} jpegd_preference_t;

typedef struct
{
    jpegd_input_req_handler_t  p_input_req_handler;
    void                      *p_arg;
    jpeg_buffer_t              buffers[2];
    uint32_t                   total_length;

} jpegd_src_t;

/* dont alter the enumerations*/
typedef enum {
  SCALE_NONE,
  SCALE_1_8,
  SCALE_2_8,
  SCALE_3_8,
  SCALE_4_8,
  SCALE_5_8,
  SCALE_6_8,
  SCALE_7_8,
  SCALE_MAX = SCALE_7_8,
} jpegd_scale_type_t;

typedef struct
{
    jpegd_preference_t   preference;
    jpegd_decode_from_t  decode_from;
    int32_t              rotation;
    jpegd_scale_type_t   scale_factor;
    uint32_t             hw_rotation;
    uint8_t              num_planes;
} jpegd_cfg_t;

typedef struct
{
    jpeg_color_format_t  output_format;
    uint32_t             width;
    uint32_t             height;
    uint32_t             stride;
    jpeg_rectangle_t     region;
    uint32_t back_to_back_count;


} jpegd_dst_t;

/******************************************************************************
 * Function: jpegd_init
 * Description: Initializes the Jpeg Decoder object. Dynamic allocations take
 *              place during the call. One should always call jpegd_destroy to
 *              clean up the Jpeg Decoder object after use.
 * Input parameters:
 *   p_obj             - The pointer to the Jpeg Decoder object to be
 *                       initialized.
 *   p_event_handler   - The function pointer to the handler function handling
 *                       Jpeg Decoder events.
 *   p_output_handler  - The function pointer to the output handler function
 *                       handling the filled output buffer.
 *   p_user_data       - The pointer to user data that the decoder stores
 *                       untouched and will be passed back to client in any
 *                       callbacks (input request handler or event handler).
 *                       It is not mandatory and can be set to NULL.
 * Return value:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpegd_init(
    jpegd_obj_t            *p_obj,
    jpegd_event_handler_t   p_event_handler,
    jpegd_output_handler_t  p_output_handler,
    void                   *p_user_data);

/******************************************************************************
 * Function: jpegd_set_source
 * Description: Specifies the information about the data source to the decoder.
 * Input parameters:
 *   obj       - The Jpeg Decoder object.
 *   p_source  - The pointer to the source object.
 * Return value:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpegd_set_source(
    jpegd_obj_t  obj,
    jpegd_src_t *p_source);

/******************************************************************************
 * Function: jpegd_read_header
 * Description: Obtains the header information from the decoder. Caller must
 *              have called jpegd_set_source successfully prior to this call.
 *              This is a synchronous call. When it returns, the header
 *              structure is filled. Caller should expect the decoder to
 *              invoke the input request handler pointed by the jpegd_src_t
 *              previously provided to the decoder to obtain data from the input
 *              stream.
 *              The header structure (jpeg_hdr_t) contains the exif_info_t
 *              field in which its data will be destroyed when the decoder
 *              gets destroyed. The caller therefore should ensure that the
 *              decoder is kept alive as long as the exif data needs to be
 *              retrieved.
 * Input parameters:
 *   obj       - The Jpeg Decoder object.
 *   p_header  - The pointer to the header structure to be filled.
 * Return value:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpegd_read_header(
    jpegd_obj_t  obj,
    jpeg_hdr_t  *p_header);

/******************************************************************************
 * Function: jpegd_enqueue_output_buf
 * Description: Enqueue a sequence of buffers. It accepts the pointer to the
 *              buffer array to be enqueued and the number of buffers in the
 *              array, appends the buffers sequentially to the queue, and
 *              updates the is_in_q flag accordingly. The number of buffers
 *              to be enqueued is checked against the size of the queue, and
 *              return fail if it is larger than the queue size.
 * Input parameters:
 *   obj                - The Jpeg Decoder object.
 *   p_output_buf_array - The pointer to the buffer array.
 *   list_count         - The number of buffers being passed.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpegd_enqueue_output_buf(
    jpegd_obj_t         obj,
    jpegd_output_buf_t *p_output_buf_array,
    uint32_t            list_count);

/******************************************************************************
 * Function: jpegd_start
 * Description: Starts the decoding. Decoding will start asynchronously in a
 *              new thread if the call returns JPEGERR_SUCCESS. The caller
 *              thread should interact asynchronously with the decoder through
 *              the event and input request handlers.
 * Input parameters:
 *   obj       - The Jpeg Decoder object.
 *   p_cfg     - The pointer to the configuration structure.
 *   p_dest    - The pointer to the destination structure.
 *   p_output_buf_array - The pointer to the buffer array.
 *   list_count         - The number of buffers being passed.
 * Return value:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpegd_start(
    jpegd_obj_t         obj,
    jpegd_cfg_t        *p_cfg,
    jpegd_dst_t        *p_dest,
    jpegd_output_buf_t *p_output_buf_array,
    uint32_t            list_count);

/******************************************************************************
 * Function: jpegd_abort
 * Description: Aborts the decoding session in progress. If there is no decoding
 *              session in progress, a JPEGERR_SUCCESS will be returned
 *              immediately. Otherwise, the call will block until the decoding
 *              is completely aborted, then return a JPEGERR_SUCCESS.
 * Input parameters:
 *   obj       - The Jpeg Decoder object
 * Return value:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EFAILED
 * Notes: none
 *****************************************************************************/
int jpegd_abort(
    jpegd_obj_t  obj);

/******************************************************************************
 * Function: jpegd_destroy
 * Description: Destroys the Jpeg Decoder object, releasing all memory
 *              previously allocated.
 * Input parameters:
 *   p_obj       - The pointer to the Jpeg Decoder object
 * Return value: None
 * Notes: none
 *****************************************************************************/
void jpegd_destroy(
     jpegd_obj_t *p_obj);

#endif // #ifndef _JPEGD_H

