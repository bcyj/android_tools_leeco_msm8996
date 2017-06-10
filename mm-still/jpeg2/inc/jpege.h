/*========================================================================


*//** @file jpege.h

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-2011, 2014 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
10/16/14   vgaur   Added multi-threaded fastCV flag and fastCV buffer pointer.
03/25/14  revathys Added stride support.
06/18/10   staceyw Changed jpege_upsample_cfg_t to jpege_scale_cfg_t, which
                   can be used both by upscale and downscale config.
06/14/10   leim    Added return type to output handler callback.
04/16/10   staceyw Added last output buffer flag in jpege_output_handler_t.
02/26/10   staceyw Changed to multiple piece-wise output buffers,
                   and added support for enqueue output buffer(s) back to
                   buffer(s) pool.
02/19/10   leim    Remove jpege_obj_t parameter in function call
                   jpege_get_default_config(), so this function is static,
                   and can be called any time.
02/08/10   staceyw Added input bitstream buffer in image fragment definition
                   for in-line jpeg mode.
10/13/09   vma     Changed to pass back user-prepared data in event
                   handlers and output produced handlers instead of the
                   encoder object itself.
06/02/09   vma     Added don't care preference.
04/23/09   vma     Added reporting of the event where thumbnail is
                   too big and is dropped.
09/12/08   vma     Added upsampler support.
07/07/08   vma     Created file.

========================================================================== */

#ifndef _JPEGE_H
#define _JPEGE_H

#include "jpeg_common.h"
#include "jpeg_buffer.h"

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define MAX_FRAGMENTS  8

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* Jpeg Header Output Type */
typedef enum
{
    OUTPUT_EXIF = 0,
    OUTPUT_JFIF

} jpege_hdr_output_t;

/* Jpeg Encoder Preference */
typedef enum
{
    JPEG_ENCODER_PREF_HW_ACCELERATED_PREFERRED = 0,
    JPEG_ENCODER_PREF_HW_ACCELERATED_ONLY,
    JPEG_ENCODER_PREF_SOFTWARE_PREFERRED,
    JPEG_ENCODER_PREF_SOFTWARE_ONLY,
    JPEG_ENCODER_PREF_DONT_CARE,

    JPEG_ENCODER_PREF_MAX,

} jpege_preference_t;

/* Opaque definition of the Jpeg Encoder */
struct jpeg_encoder_t;
typedef struct jpeg_encoder_t *jpege_obj_t;

/* The Jpeg Encoder event handler definition */
typedef void(*jpege_event_handler_t)(void          *p_user_data, // The user data initially stored at _init()
                                     jpeg_event_t  event,        // The event code
                                     void          *p_arg);      // An optional argument for that event:
                                                                 // JPEG_EVENT_DONE:    unused
                                                                 // JPEG_EVENT_WARNING: const char * (warning message)
                                                                 // JPEG_EVENT_ERROR:   const char * (error message)
                                                                 // JPEG_EVENT_THUMBNAIL_DROPPED: unused

/* The handler for dealing with output produced by the Jpeg Encoder
 * This is used in jpege_dst_t to specify the handler function when
 * output is ready to be written. The handler needs to process the
 * output synchronously. In other words, when the handler returns
 * the encoder will assume the output buffer is ready to be reused.
 * In the extreme case where the system is so heavily loaded that
 * the time to consume the output is slower than it is produced by
 * the encoder, this output handler will be invoked before the
 * previous one finishes. The caller should make sure therefore
 * that the implementation is reentrant.
 */
typedef  int(*jpege_output_handler_t)(void             *p_user_data,  // The user data initially stored at _init()
                                      void             *p_arg,        // The argument which the encoder would pass back
                                                                      // It is useful for example if the handler needs to write
                                                                      // the output directly to a FILE stream object in which
                                                                      // case p_arg is the FILE pointer itself.
                                      jpeg_buffer_t     buf,          // The buffer containing the output
                                      uint8_t           last_buf_flag); // The flag to indicate the last output buffer

/* The structure defining an image fragment. It contains both
 * the width and height of the fragment as well as the pointer
 * to the actual pixel data. The union inside the structure
 * contains YUV, RGB and bitstream fields. They should be used
 * accordingly depending on the color format of the pixel data. */
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

        struct
        {
            jpeg_buffer_t bitstream_buf;
                                       // Buffer to hold the bitstream
        } bitstream;

    } color;

    uint32_t   width;           // Width of the fragment
    uint32_t   height;          // Height of the fragment
    uint16_t   num_of_planes;

} jpege_img_frag_t;

/* The structure defining the information about the possible image input
 * to the Jpeg Encoder. Note that the image width and height in this structure
 * tells the encoder the real dimension of the image excluding the padded data
 * in case padding was pre-applied. The padded width and height would be captured
 * in the fragment width and height rather. */
typedef struct
{
    jpeg_color_format_t color_format;               // Color format of the image data (e.g. YCrCbLpH2V2)
    uint32_t            width;                      // Width of the image to be encoded
    uint32_t            height;                     // Height of the image to be encoded
    uint32_t            fragment_cnt;               // The number of fragments (max is 8)
    jpege_img_frag_t    p_fragments[MAX_FRAGMENTS]; // Array of the image fragments composing the image
                                                    // When set source, first fragment must be available.
                                                    // One fragment at each time to be delivered.
                                                    // It targets with hardware encoding only.

} jpege_img_data_t;

/* The structure defining the information about the source of the images to the
 * Jpeg Encoder. */
typedef struct
{
    jpege_img_data_t  *p_main;            // The main image to be encoded (Mandatory field)
    jpege_img_data_t  *p_thumbnail;       // The thumbnail image to be encoded (Can be null if absent)

} jpege_src_t;

/* The structure defining the information about the destination of the encoded
 * bitstream from the Jpeg Encoder. */
typedef struct
{
    // The handler function when output data is ready to be written
    jpege_output_handler_t  p_output_handler;

    // The argument which the encoder would pass back.
    // See definition of the jpege_output_handler_t.
    void                   *p_arg;

    // The number of supplied destination buffer(s). It can be in the range of 0 - N.
    // The encoder will try to utilize the buffer(s) as much as it can.
    // The encoder makes no guarantee to always utilize the supplied buffers for
    // performance reason. Caller shall expect output to be delivered in internal
    // working buffers of the encoder if it chooses to.
    uint32_t                buffer_cnt;

    // The pointer to supplied destination buffer(s) array.
    jpeg_buffer_t           *p_buffer;

} jpege_dst_t;

/* The structure defining the scale configuration */
typedef struct
{
    /*
     Scale is supported by certain hardware chipsets.
     Scale can used to crop the input image and encode it a different resolution.
     The cropped image can be cropped and upscaled by a factor of 1x - 8x
     or downscaled by a factor of 1x - 1/8x.
     An upscale is enabled automaticcally when at least one of the output image
     dimensions is greater than the corresponding input image dimension.
     An downScale is enabled automaticcally when at least one of the output image
     dimensions is less than the corresponding input image dimension.

            |-----------------------------------------------------|  /\
            |                /\                                   |   |
            |                 |                                   |   |
            |             v_offset                                |   |
            |                 |                                   |   |
            |                \/                                   |   |
            |              |-------------------|      /\          |   |
            |              |    scale          |       |          |   |
            |              |<-  input_width  ->|    scale         |   | input
            |              |                   |   input_height   |   | height
            |<- h_offset ->|                   |       |          |   |
            |              |                   |       |          |   |
            |              |-------------------|      \/          |   |
            |                                                     |   |
            |                                                     |   |
            |                                                     |   |
            |<--------------- input_width ----------------------->|   |
            |                                                     |   |
            |                                                     |   |
            |-----------------------------------------------------|  \/
    */

    uint32_t    input_width;   // input width
    uint32_t    input_height;  // input height
    uint32_t    h_offset;      // h_offset
    uint32_t    v_offset;      // v_offset
    uint32_t    output_width;  // output width
    uint32_t    output_height; // output height
    uint8_t     enable;        // enable flag

} jpege_scale_cfg_t;

///
///
///     <---input_luma_stride--->
///     <---width----->
///      _ _ _ _ _ _ _ _ _ _ _ _
///     |             |         |        W - Stride
///     |             |         |        w - Actual luma or chroma data width
///     |             |         |        Stride can be different for input and output buffers
///     |_ _ _ _ _ _ _|         |        as well as luma and chroma buffers.
///     |                       |
///     |_ _ _ _ _ _ _ _ _ _ _ _|
///
///
typedef struct
{
    uint8_t  enable;              //stride enable flag
    uint32_t input_luma_stride;   //Input luma stride
    uint32_t input_chroma_stride; //Input chroma stride
    uint32_t output_luma_stride;  //Output luma stride
    uint32_t output_chroma_stride;//Output chroma stride

} jpege_stride_cfg_t;

/* The structure defining the configuration to the Jpeg Encoder specific
 * specific to the image. */
typedef struct
{
    // Encode quality - From 1-100: A linear factor used in scaling the
    // quantization table used in encoding and ultimately controlling
    // the encoded image quality (filesize as well).
    uint32_t              quality;
    // Rotation in degree (clockwise). Valid values are: 0, 90, 180, 270.
    uint32_t              rotation_degree_clk;
    // The luma quantization table.
    jpeg_quant_table_t    luma_quant_tbl;
    // The chroma quantization table.
    jpeg_quant_table_t    chroma_quant_tbl;
    // The luma Huffman table for DC components.
    jpeg_huff_table_t     luma_dc_huff_tbl;
    // The chroma Huffman table for DC components.
    jpeg_huff_table_t     chroma_dc_huff_tbl;
    // The luma Huffman table for AC components.
    jpeg_huff_table_t     luma_ac_huff_tbl;
    // The chroma Huffman table for AC components.
    jpeg_huff_table_t     chroma_ac_huff_tbl;
    // Encode restart marker interval - how often restart markers are
    // inserted into the bitstream_t for error recovery.
    uint32_t              restart_interval;
    // Scale configuration
    jpege_scale_cfg_t     scale_cfg;
    // Stride configuration
    jpege_stride_cfg_t    stride_cfg;

    uint8_t                *fastCV_buffer;
    uint8_t                fastCV_flag;

} jpege_img_cfg_t;

/* The structure defining the configuration to the Jpeg Encoder. */
typedef struct
{
    // The target file size in bytes. If non-zero, it enables file size
    // control. The Jpeg Encoder will override the quality and quantization
    // table settings and automatically adjust them to limit the encoded
    // file size. It impacts performance as it might require the encoder
    // to perform multiple pass of encoding. Use it with care.
    uint32_t             target_filesize;

    // Header output type: EXIF (default) or JFIF
    jpege_hdr_output_t   header_type;

    // Configuration specific to main image
    jpege_img_cfg_t      main_cfg;

    // Configuration specific to thumbnail image
    jpege_img_cfg_t      thumbnail_cfg;

    // Configure whether thumbnail is present to be encoded
    uint8_t              thumbnail_present;

    // Encoder preference
    jpege_preference_t   preference;

    // The pre-calculated App2 header length
    uint32_t            app2_header_length;
    uint8_t            *fastCV_buffer;
    uint8_t             fastCV_flag;

} jpege_cfg_t;

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
**                          Function Declarations
** ======================================================================= */

/******************************************************************************
 * Function: jpege_init
 * Description: Initializes the Jpeg Encoder object. Dynamic allocations take
 *              place during the call. One should always call jpege_destroy to
 *              clean up the Jpeg Encoder object after use.
 * Input parameters:
 *   p_obj     - The pointer to the Jpeg Encoder object to be initialized.
 *   p_handler - The function pointer to the handler function handling Jpeg
 *               Encoder events.
 *   p_user_data - User data that will be stored by the encoder and passed
 *                 back untouched in output produced and event handlers.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpege_init(
    jpege_obj_t           *p_obj,
    jpege_event_handler_t  p_handler,
    void                  *p_user_data);

/******************************************************************************
 * Function: jpege_get_default_config
 * Description: Initializes the input configuration structure with default
 *              encode settings. Typical usage would be to utilize this function
 *              to assign default settings and then apply custom settings
 *              to the configuration to make sure all settings are configured
 *              properly.
 * Input parameters:
 *   p_config  - The pointer to the configuration structure to be initialized
 *               with default settings.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpege_get_default_config(
    jpege_cfg_t    *p_config);

/******************************************************************************
 * Function: jpege_get_actual_config
 * Description: Get the actual conig that was used int the encoding session.
 *              Typically this is used after the encoding is done and before
 *              encoder is destroyed.
 * Input parameters:
 *   obj       - The Jpeg Encoder object.
 *   p_config  - The pointer to the configuration structure to be initialized
 *               with default settings.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpege_get_actual_config(
    jpege_obj_t     obj,
    jpege_cfg_t    *p_config);

/******************************************************************************
 * Function: jpege_set_source
 * Description: Assigns the image source to the Jpeg Encoder object.
 * Input parameters:
 *   obj       - The Jpeg Encoder object.
 *   p_source  - The pointer to the source object to be assigned to the Jpeg
 *               Encoder.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpege_set_source(
    jpege_obj_t   obj,
    jpege_src_t  *p_source);

/******************************************************************************
 * Function: jpege_set_mobicat_data
 * Description: Assigns mobicat data to Jpeg Encoder object.
 * Input parameters:
 *   obj                      - The Jpeg Encoder object.
 *  *p_mobicat_data           - Mobicat data
 *   mobicat_data_length      - length of mobicat data
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpege_set_mobicat_data(
    jpege_obj_t   obj,
    uint8_t      *p_mobicat_data,
    int32_t       mobicat_data_length);

/******************************************************************************
 * Function: jpege_set_debug_data
 * Description: Assigns debug data to Jpeg Encoder object.
 * Input parameters:
 *   obj                      - The Jpeg Encoder object.
 *  *p_debug_data           - debug data
 *   debug_data_length      - length of debug data
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpege_set_debug_data(
    jpege_obj_t   obj,
    uint8_t      *p_debug_data,
    int32_t       debug_data_length,
    int32_t       debug_data_version);


/******************************************************************************
 * Function: jpege_set_destination
 * Description: Assigns the destination to the Jpeg Encoder object.
 * Input parameters:
 *   obj       - The Jpeg Encoder object.
 *   p_dest    - The pointer to the destination object to be assigned to the Jpeg
 *               Encoder.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpege_set_destination(
    jpege_obj_t   obj,
    jpege_dst_t  *p_dest);

/******************************************************************************
 * Function: jpege_start
 * Description: Starts the Jpeg Encoder. Encoding will start asynchronously
 *              in a new thread if the call returns JPEGERR_SUCCESS. The caller
 *              thread should interact asynchronously with the encoder through
 *              event and output handlers.
 * Input parameters:
 *   obj             - The Jpeg Encoder object.
 *   p_cfg           - The pointer to the configuration structure.
 *   p_exif_info_obj - The pointer to the Exif Info object in which the encoder
 *                     should obtain and exif tag information from. It can be
 *                     set to NULL if no exif tag information is supplied. It is
 *                     valid to supply a NULL pointer even if the header type
 *                     is set to EXIF. Defaults would be used for mandatory
 *                     EXIF tags.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpege_start(
    jpege_obj_t       obj,
    jpege_cfg_t      *p_cfg,
    exif_info_obj_t  *p_exif_info);

/******************************************************************************
 * Function: jpege_abort
 * Description: Aborts the encoding session in progress. If there is no encoding
 *              session in progress, a JPEGERR_SUCCESS will be returned
 *              immediately. Otherwise, the call will block until the encoding
 *              is completely aborted, then return a JPEGERR_SUCCESS.
 * Input parameters:
 *   obj       - The Jpeg Encoder object
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpege_abort(
    jpege_obj_t obj);

/******************************************************************************
 * Function: jpege_destroy
 * Description: Destroys the Jpeg Encoder object, releasing all memory
 *              previously allocated.
 * Input parameters:
 *   p_obj       - The pointer to the Jpeg Encoder object
 * Return values: None
 * Notes: none
 *****************************************************************************/
void jpege_destroy(jpege_obj_t *p_obj);

/******************************************************************************
* Function: jpege_enqueue_output_buffer
* Description: Enqueue output buffer(s) to queue. It accepts the pointer to
*              output buffer(s) to be enqueued and the number of buffer(s),
*              appends output buffer(s) sequentially to the queue.
*              Enqueued output buffer(s) is checked, and
*              the number of buffer(s) to be enqueued is checked
*              against the free slots queue, and return fail if
*              it is larger than free slots left inside queue.
* Input parameters:
*   obj                - The Jpeg Encoder object.
*   p_enqueue_buf      - The pointer to enqueued buffer(s) array.
*   enqueue_buf_cnt    - The number of enqueued buffer(s).
* Return values:
*     JPEGERR_SUCCESS
*     JPEGERR_EFAILED
*     JPEGERR_ENULLPTR
*     JPEGERR_EBADPARM
* (See jpegerr.h for description of error values.)
* Notes: none
*****************************************************************************/
int jpege_enqueue_output_buffer(jpege_obj_t      obj,
                                jpeg_buffer_t    *p_enqueue_buf,
                                uint32_t         enqueue_buf_cnt);

/******************************************************************************
 * Function: jpege_deliver_image_fragment
 * Description: Deliver fragment to the Jpeg Encoder object. Fragment has to be
                Vertical in the frame: fragment width has to equal to frame width,
                sum of all fragment height is equal to frame height. Fragment must
                be continuously delivered, and combine all fragments to one during
                delivery.
* Input parameters:
*   obj                - The Jpeg Encoder object.
*   fragment           - The fragment of a frame to be sent to Encoder.
*   fragment_id        - The fragment id number.
* Return values:
*     JPEGERR_SUCCESS
*     JPEGERR_EFAILED
*     JPEGERR_ENULLPTR
*     JPEGERR_EBADPARM
* (See jpegerr.h for description of error values.)
* Notes: none
 *****************************************************************************/
int jpege_deliver_image_fragment(
    jpege_obj_t         obj,
    jpege_img_frag_t    fragment,
    uint8_t             fragment_id
);

#endif // #ifndef _JPEGE_H

