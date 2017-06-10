/*========================================================================
Copyright (C) 2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
03/10/11   mingy   Created file.

========================================================================== */

#ifndef _MPOD_H
#define _MPOD_H

#include "mpo.h"

/* Opaque definition of the MPO Decoder */
struct mpo_decoder_t;
typedef struct mpo_decoder_t *mpod_obj_t;

typedef struct
{
    union
    {
        struct
        {
            jpeg_buffer_t luma_buf;    // Buffer to hold the luma data (Y)
            jpeg_buffer_t chroma_buf;  // Buffer to hold the chroma data (CrCb or CbCr interlaced)
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

} mpod_output_buf_t;

/* The Jpeg Decoder event handler definition */
typedef void(*mpod_event_handler_t)(void         *p_user_data, // The user data passed back to client untouched
                                                               // (originally passed jpegd_init)
                                    jpeg_event_t  event,       // The event code
                                    void         *p_arg);      // An optional argument for that event:
                                                               // JPEG_EVENT_DONE:    unused
                                                               // JPEG_EVENT_WARNING: const char * (warning message)
                                                               // JPEG_EVENT_ERROR:   const char * (error message)

/* The Jpeg Decoder output handler definition */
typedef int(*mpod_output_handler_t)(void               *p_user_data,     // The user data passed back to client untouched
                                                                         // (originally passed jpegd_init)
                                    mpod_output_buf_t  *p_output_buffer, // The output buffer filled with decoded data
                                    uint32_t            first_row_id,    // The sequence id of the first output buffer row
                                    uint8_t             is_last_buffer); // The flag indicating the last buffer

/**
 *  The handler for serving input requests from the Jpeg
 *  Decoder. The decoder expects the buffer would be filled when
 *  the call returns. The handler should return the number of
 *  bytes written to the buffer.  */
typedef uint32_t(*mpod_input_req_handler_t)(void          *p_user_data, // The users data passed back to client untouched
                                                                         // (originally passed to jpegd_init)
                                            jpeg_buffer_t  buffer,
                                            uint32_t       start_offset,
                                            uint32_t       length);

// MPO decoder source structure
typedef struct
{
    mpod_input_req_handler_t   p_input_req_handler;  // Pointer to callback
                                                     // function called by
                                                     // decoder when input data
                                                     // need to be requested

    void                      *p_arg;                // Deprecated, currently
                                                     // not in use

    jpeg_buffer_t              buffers[2];           // Two initial empty
                                                     // buffers provided to
                                                     // the decoder for
                                                     // filling requested input
                                                     // data. Decoder will pass
                                                     // them back in a
                                                     // ping-pong fashion in
                                                     // input request callbacks.

    uint32_t                   total_length;         // Optional hint to the
                                                     // decoder about the total
                                                     // length of the input
                                                     // stream. if it is unknown
                                                     // set it to 0.
} mpod_src_t;

// MPO decoder configuration structure
typedef struct
{
    jpegd_preference_t   preference;
    jpegd_decode_from_t  decode_from;
    int32_t              rotation;

} mpod_cfg_t;

// MOP decoder destination structure
typedef struct
{
    jpeg_color_format_t  output_format;
    uint32_t             width;
    uint32_t             height;
    uint32_t             stride;
    jpeg_rectangle_t     region;

} mpod_dst_t;

/******************************************************************************
 * Function: mpod_init
 * Description: Initializes the MPO Decoder object. Dynamic allocations take
 *              place during the call. One should always call mpod_destroy to
 *              clean up the MPO Decoder object after use.
 * Input parameters:
 *   p_mpod_obj  - The pointer to the MPO Decoder object to be initialized.
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
 *     JPEGERR_EMALLOC
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_init(mpod_obj_t             *p_mpod_obj,
              jpegd_event_handler_t   p_event_handler,
              mpod_output_handler_t   p_output_handler,
              void                   *p_user_data);

/******************************************************************************
 * Function: mpod_set_source
 * Description: Assigns the image source to the MPO Decoder object.
 * Input parameters:
 *   obj       - The MPO Decoder object
 *   p_source  - The pointer to the source object to be assigned to the MPO
 *               Decoder.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_set_source(mpod_obj_t  mpod_obj, mpod_src_t *p_source);

/******************************************************************************
 * Function: mpod_read_first_header
 * Description: Obtains the header information from the first individual image.
 *              This is a synchronous call. When it returns, the header
 *              structure is filled.
 *              The MPO header structure (mpo_hdr_t) contains the mpo_index_obj_t
 *              field and mpo_attribute_obj_t in which their data will be
 *              destroyed when the MPO decoder gets destroyed. The caller
 *              therefore should ensure that the MPO decoder is kept alive
 *              as long as the MPO index data and MPO attribute data need to be
 *              retrieved.
 * Input parameters:
 *   mpod_obj_t    - The MPO Decoder object.
 *   p_mpo_header  - The pointer to the MPO header structure to be filled.
 * Return value:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EUNINITIALIZED
 *     JPEGERR_EBADSTATE
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_read_first_header(mpod_obj_t mpo_obj, mpo_hdr_t *p_mpo_header);

/******************************************************************************
 * Function: mpod_read_first_header
 * Description: Obtains the header information from the individual images other
 *              than the first individual image. This is a synchronous call.
 *              When it returns, the header structure is filled.
 *              The MPO header structure (mpo_hdr_t) contains the mpo_index_obj_t
 *              field and mpo_attribute_obj_t in which their data will be
 *              destroyed when the MPO decoder gets destroyed. The caller
 *              therefore should ensure that the MPO decoder is kept alive
 *              as long as the MPO index data and MPO attribute data need to be
 *              retrieved.
 * Input parameters:
 *   mpod_obj_t    - The MPO Decoder object.
 *   p_mpo_header  - The pointer to the MPO header structure to be filled.
 * Return value:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EUNINITIALIZED
 *     JPEGERR_EBADSTATE
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_read_header(mpod_obj_t mpo_obj, mpo_hdr_t *p_mpo_header);

/******************************************************************************
 * Function: mpod_enqueue_output_buf
 * Description: Enqueue a sequence of buffers. It accepts the pointer to the
 *              buffer array to be enqueued and the number of buffers in the
 *              array, appends the buffers sequentially to the queue, and
 *              updates the is_in_q flag accordingly. The number of buffers
 *              to be enqueued is checked against the size of the queue, and
 *              return fail if it is larger than the queue size.
 * Input parameters:
 *   mpo_obj            - The MPO Decoder object.
 *   p_output_buf_array - The pointer to the buffer array.
 *   list_count         - The number of buffers being passed.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_enqueue_output_buf(
    mpod_obj_t          mpo_obj,
    mpod_output_buf_t  *p_output_buf_array,
    uint32_t            list_count);

/******************************************************************************
 * Function: mpod_start
 * Description:
 * Input parameters:
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_start(
    mpod_obj_t          mpod_obj,
    mpod_cfg_t         *p_cfg,
    mpod_dst_t         *p_dest,
    mpod_output_buf_t  *p_output_buf_array,
    uint32_t            list_count);

/******************************************************************************
 * Function: mpod_abort
 * Description: Aborts the decoding session in progress. If there is no decoding
 *              session in progress, a JPEGERR_SUCCESS will be returned
 *              immediately. Otherwise, the call will block until the decoding
 *              is completely aborted, then return a JPEGERR_SUCCESS.
 * Input parameters:
 *   mpo_obj    - The MPO Decoder object
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_abort(mpod_obj_t mpod_obj);

/******************************************************************************
 * Function: mpod_destroy
 * Description: Destroys the MPO Decoder object, releasing memory previously
 *              allocated by MPO Decoder.
 * Input parameters:
 *   p_mpod_obj  - The pointer to the MPO Decoder object
 * Return value: None
 * Notes: none
 *****************************************************************************/
void mpod_destroy(mpod_obj_t *p_mpod_obj);

/******************************************************************************
 * Function: mpod_get_mp_entry_value
 * Description: Obtains the MP entry values.
 * Input parameters:
 *   mpod_obj     - The MPO decoder object.
 *   p_mp_entry   - The pointer to the MP entry structure.
 *   num_image    - The number of images in the MPO file.
 * Return value:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EUNINITIALIZED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_get_mp_entry_value(mpod_obj_t mpod_obj, mp_entry_t *p_mp_entry, uint32_t num_image);

/******************************************************************************
 * Function: mpod_get_num_image
 * Description: Retrives the number of images in the MPO file.
 * Input parameters:
 *   mpod_obj     - The MPO decoder object.
 *   p_num_image  - The pointer to the number of images in the MPO file.
 * Return value:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_get_num_image(mpod_obj_t mpod_obj, uint32_t *p_num_image);

/******************************************************************************
 * Function: mpod_get_mp_entry_value
 * Description: Reset the starting address in the input buffer so that it
 *              points to the beginning of the next individual image.
 * Input parameters:
 *   mpo_obj     - The MPO decoder object.
 *   data_offset - The offset to the beginning of the next individual image.
 * Return value:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EUNINITIALIZED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpod_set_input_offset(mpod_obj_t mpod_obj, uint32_t data_offset);

#endif // #ifndef _MPOD_H

