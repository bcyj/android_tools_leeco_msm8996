/*========================================================================


*//** @file mpoe.h

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
01/10/11   staceyw     Created file.

========================================================================== */

#ifndef _MPOE_H
#define _MPOE_H

#include "jpeg_common.h"
#include "jpeg_buffer.h"
//#include "exif_private.h"
#include "mpo.h"

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define MAX_IMAGES  2

#define MAX_FRAGMENTS  8

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* Mpo Header Output Type */
typedef enum
{
    OUTPUT_MPO_EXIF = 0,
    OUTPUT_MPO_JFIF

} mpoe_hdr_output_t;

/* Mpo Encoder Preference */
typedef enum
{
    MPO_ENCODER_PREF_HW_ACCELERATED_PREFERRED = 0,
    MPO_ENCODER_PREF_HW_ACCELERATED_ONLY,
    MPO_ENCODER_PREF_SOFTWARE_PREFERRED,
    MPO_ENCODER_PREF_SOFTWARE_ONLY,
    MPO_ENCODER_PREF_DONT_CARE,

    MPO_ENCODER_PREF_MAX,

} mpoe_preference_t;

/* Opaque definition of the Mpo Encoder */
struct mpo_encoder_t;
typedef struct mpo_encoder_t *mpoe_obj_t;

/* The Mpo Encoder event handler definition */
typedef void(*mpoe_event_handler_t)(void          *p_user_data, // The user data initially stored at _init()
                                    jpeg_event_t  event,        // The event code
                                    void          *p_arg);      // An optional argument for that event:
                                                                 // JPEG_EVENT_DONE:    unused
                                                                 // JPEG_EVENT_WARNING: const char * (warning message)
                                                                 // JPEG_EVENT_ERROR:   const char * (error message)
                                                                 // JPEG_EVENT_THUMBNAIL_DROPPED: unused

/* The handler for dealing with output produced by the Mpo Encoder
 * This is used in mpoe_dst_t to specify the handler function when
 * output is ready to be written. The handler needs to process the
 * output synchronously. In other words, when the handler returns
 * the encoder will assume the output buffer is ready to be reused.
 * In the extreme case where the system is so heavily loaded that
 * the time to consume the output is slower than it is produced by
 * the encoder, this output handler will be invoked before the
 * previous one finishes. The caller should make sure therefore
 * that the implementation is reentrant.
 */
typedef  int(*mpoe_output_handler_t)(void             *p_user_data,  // The user data initially stored at _init()
                                     void             *p_arg,        // The argument which the encoder would pass back
                                                                      // It is useful for example if the handler needs to write
                                                                      // the output directly to a FILE stream object in which
                                                                      // case p_arg is the FILE pointer itself.
                                     jpeg_buffer_t     buf,          // The buffer containing the output
                                     uint8_t           last_buf_flag); // The flag to indicate the last output buffer

/* The structure defining an App2 header data. It contains
 * both the start offset and data buffer containing
 * the actual header data. */
typedef struct
{
    // App2 data start offset
    uint32_t       start_offset;

    // App2 data buffer
    jpeg_buffer_t  data_buf;

} mpoe_app2_data_t;

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

} mpoe_img_frag_t;

/* The structure defining the information about the possible image input
 * to the Mpo Encoder. Note that the image width and height in this structure
 * tells the encoder the real dimension of the image excluding the padded data
 * in case padding was pre-applied. The padded width and height would be captured
 * in the fragment width and height rather. */
typedef struct
{
    jpeg_color_format_t color_format;               // Color format of the image data (e.g. YCrCbLpH2V2)
    uint32_t            width;                      // Width of the image to be encoded
    uint32_t            height;                     // Height of the image to be encoded
    uint32_t            fragment_cnt;               // The number of fragments (max is 8)
    mpoe_img_frag_t     p_fragments[MAX_FRAGMENTS]; // Array of the image fragments composing the image

} mpoe_img_data_t;

/* The structure defining the information about the source of the images to the
 * Mpo Encoder. */
typedef struct
{
    mpoe_img_data_t  *p_main;            // The main image to be encoded (Mandatory field)
    mpoe_img_data_t  *p_thumbnail;       // The thumbnail image to be encoded (Can be null if absent)
    individual_image_attribute_t  image_attribute; // Attribute of an Individual Image

} mpoe_src_t;

/* The structure defining the information about the destination of the encoded
 * bitstream from the Mpo Encoder. */
typedef struct
{
    // The handler function when output data is ready to be written
    mpoe_output_handler_t  p_output_handler;

    // The argument which the encoder would pass back.
    // See definition of the mpoe_output_handler_t.
    void                  *p_arg;

    // The number of supplied destination buffer(s). It can be in the range of 0 - N.
    // The encoder will try to utilize the buffer(s) as much as it can.
    // The encoder makes no guarantee to always utilize the supplied buffers for
    // performance reason. Caller shall expect output to be delivered in internal
    // working buffers of the encoder if it chooses to.
    uint32_t               buffer_cnt;

    // The pointer to supplied destination buffer(s) array.
    jpeg_buffer_t         *p_buffer;

} mpoe_dst_t;

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

} mpoe_scale_cfg_t;

/* The structure defining the configuration to the Mpo Encoder specific
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
    mpoe_scale_cfg_t      scale_cfg;

} mpoe_img_cfg_t;

/* The structure defining the configuration to the Mpo Encoder. */
typedef struct
{
    uint32_t            target_filesize;

    // Header output type: EXIF (default) or JFIF
    mpoe_hdr_output_t   header_type;

    // Configuration specific to main image
    mpoe_img_cfg_t      main_cfg;

    // Configuration specific to thumbnail image
    mpoe_img_cfg_t      thumbnail_cfg;

    // Configure whether thumbnail is present to be encoded
    uint8_t             thumbnail_present;

    // Encoder preference
    mpoe_preference_t   preference;

} mpoe_cfg_t;

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
**                          Function Declarations
** ======================================================================= */

/******************************************************************************
 * Function: mpoe_init
 * Description: Initializes the Mpo Encoder object. Dynamic allocations take
 *              place during the call. One should always call mpoe_destroy to
 *              clean up the Mpo Encoder object after use.
 * Input parameters:
 *   p_obj     - The pointer to the Mpo Encoder object to be initialized.
 *   p_handler - The function pointer to the handler function handling Mpo
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
int mpoe_init(
    mpoe_obj_t            *p_obj,
    mpoe_event_handler_t   p_handler,
    void                  *p_user_data);

/******************************************************************************
 * Function: mpoe_get_default_config
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
int mpoe_get_default_config(
    mpoe_cfg_t    *p_config);

/******************************************************************************
 * Function: mpoe_get_actual_config
 * Description: Get the actual conig that was used int the encoding session.
 *              Typically this is used after the encoding is done and before
 *              encoder is destroyed.
 * Input parameters:
 *   obj       - The Mpo Encoder object.
 *   p_config  - The pointer to the actual configuration structure.
 *   src_id    - The source id.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpoe_get_actual_config(
    mpoe_obj_t     obj,
    mpoe_cfg_t    *p_config,
    uint32_t       src_id);

/******************************************************************************
 * Function: mpoe_set_source
 * Description: Assigns the image source to the Mpo Encoder object.
 * Input parameters:
 *   obj       - The Mpo Encoder object.
 *   p_source  - The pointer to source arrary object to be assigned to the Mpo
 *               Encoder.
 *   source_cnt - The source count
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpoe_set_source(
    mpoe_obj_t    obj,
    mpoe_src_t   *p_source,
    uint32_t      source_cnt);

/******************************************************************************
 * Function: mpoe_set_destination
 * Description: Assigns the destination to the Mpo Encoder object.
 * Input parameters:
 *   obj       - The Mpo Encoder object.
 *   p_dest    - The pointer to the destination object to be assigned to the Mpo
 *               Encoder.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpoe_set_destination(
    mpoe_obj_t    obj,
    mpoe_dst_t   *p_dest);

/******************************************************************************
 * Function: mpoe_start
 * Description: Starts the Mpo Encoder. Encoding will start asynchronously
 *              in a new thread if the call returns JPEGERR_SUCCESS. The caller
 *              thread should interact asynchronously with the encoder through
 *              event and output handlers.
 * Input parameters:
 *   obj             - The Mpo Encoder object.
 *   p_cfg           - The pointer to the configuration array.
 *   p_exif_info     - The pointer to the Exif Info array in which the encoder
 *                     should obtain and exif tag information from. It can be
 *                     set to NULL if no exif tag information is intended to be
 *                     written.
 *   p_mpo_info      - The pointer to the mpo info.
 *   src_cnt         - The count of source
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpoe_start(
    mpoe_obj_t        obj,
    mpoe_cfg_t       *p_cfg,
    exif_info_obj_t  *p_exif_info,
    mpo_info_obj_t   *p_mpo_info,
    uint32_t          src_cnt);

/******************************************************************************
 * Function: mpoe_abort
 * Description: Aborts the encoding session in progress. If there is no encoding
 *              session in progress, a JPEGERR_SUCCESS will be returned
 *              immediately. Otherwise, the call will block until the encoding
 *              is completely aborted, then return a JPEGERR_SUCCESS.
 * Input parameters:
 *   obj       - The Mpo Encoder object
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpoe_abort(
    mpoe_obj_t obj);

/******************************************************************************
 * Function: mpoe_destroy
 * Description: Destroys the Mpo Encoder object, releasing all memory
 *              previously allocated.
 * Input parameters:
 *   p_obj       - The pointer to the Mpo Encoder object
 * Return values: None
 * Notes: none
 *****************************************************************************/
int mpoe_destroy(mpoe_obj_t *p_obj);

/******************************************************************************
* Function: mpoe_enqueue_output_buffer
* Description: Enqueue output buffer(s) to queue. It accepts the pointer to
*              output buffer(s) to be enqueued and the number of buffer(s),
*              appends output buffer(s) sequentially to the queue.
*              Enqueued output buffer(s) is checked, and
*              the number of buffer(s) to be enqueued is checked
*              against the free slots queue, and return fail if
*              it is larger than free slots left inside queue.
* Input parameters:
*   obj                - The Mpo Encoder object.
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
int mpoe_enqueue_output_buffer(mpoe_obj_t       obj,
                               jpeg_buffer_t   *p_enqueue_buf,
                               uint32_t         enqueue_buf_cnt);

/******************************************************************************
* Function: mpoe_get_app2_header
* Description: Get App2 header.
* Input parameters:
*   obj                - The Mpo Encoder object.
*   p_app2_data        - The pointer to App2 header array.
*   count              - The App2 header array count.
* Return values:
*     JPEGERR_SUCCESS
*     JPEGERR_EFAILED
*     JPEGERR_EUNINITIALIZED
*     JPEGERR_EBADPARM
* (See jpegerr.h for description of error values.)
* Notes: none
*****************************************************************************/
int mpoe_get_app2_header(
    mpoe_obj_t         obj,
    mpoe_app2_data_t  *p_app2_data,
    uint32_t           count);

#endif // #ifndef _MPOE_H
