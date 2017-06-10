/*========================================================================


*//** @file jpse.h

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2010-11 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
11/15/10   staceyw Created file.

========================================================================== */

#ifndef _JPSE_H
#define _JPSE_H

#include "jpeg_common.h"
#include "jpeg_buffer.h"
#include "jps.h"

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define MAX_FRAGMENTS  8

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* Jps Header Output Type */
typedef enum
{
    JPS_OUTPUT_EXIF = 0,
    JPS_OUTPUT_JFIF

} jpse_hdr_output_t;

/* Jps Encoder Preference */
typedef enum
{
    JPS_ENCODER_PREF_HW_ACCELERATED_PREFERRED = 0,
    JPS_ENCODER_PREF_HW_ACCELERATED_ONLY,
    JPS_ENCODER_PREF_SOFTWARE_PREFERRED,
    JPS_ENCODER_PREF_SOFTWARE_ONLY,
    JPS_ENCODER_PREF_DONT_CARE,

    JPS_ENCODER_PREF_MAX,

} jpse_preference_t;

/* Opaque definition of the Jps Encoder */
struct jps_encoder_t;
typedef struct jps_encoder_t *jpse_obj_t;

/* The Jps Encoder event handler definition */
typedef void(*jpse_event_handler_t)(void          *p_user_data, // The user data initially stored at _init()
                                    jpeg_event_t  event,        // The event code
                                    void          *p_arg);      // An optional argument for that event:
                                                                // JPEG_EVENT_DONE:    unused
                                                                // JPEG_EVENT_WARNING: const char * (warning message)
                                                                // JPEG_EVENT_ERROR:   const char * (error message)
                                                                // JPEG_EVENT_THUMBNAIL_DROPPED: unused

/* The handler for dealing with output produced by the Jps Encoder
 * This is used in jpse_dst_t to specify the handler function when
 * output is ready to be written. The handler needs to process the
 * output synchronously. In other words, when the handler returns
 * the encoder will assume the output buffer is ready to be reused.
 * In the extreme case where the system is so heavily loaded that
 * the time to consume the output is slower than it is produced by
 * the encoder, this output handler will be invoked before the
 * previous one finishes. The caller should make sure therefore
 * that the implementation is reentrant.
 */
typedef  int(*jpse_output_handler_t)(void             *p_user_data,  // The user data initially stored at _init()
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

} jpse_img_frag_t;

/* The structure defining the information about the possible image input
 * to the Jps Encoder. Note that the image width and height in this structure
 * tells the encoder the real dimension of the image excluding the padded data
 * in case padding was pre-applied. The padded width and height would be captured
 * in the fragment width and height rather. */
typedef struct
{
    jpeg_color_format_t color_format;               // Color format of the image data (e.g. YCrCbLpH2V2)
    uint32_t            width;                      // Width of the image to be encoded
    uint32_t            height;                     // Height of the image to be encoded
    uint32_t            fragment_cnt;               // The number of fragments (max is 8)
    jpse_img_frag_t     p_fragments[MAX_FRAGMENTS]; // Array of the image fragments composing the image

} jpse_img_data_t;

/* The structure defining the information about the source of the images to the
 * Jps Encoder. */
typedef struct
{
    jpse_img_data_t  *p_main;            // The main image to be encoded (Mandatory field)
    jpse_img_data_t  *p_thumbnail;       // The thumbnail image to be encoded (Can be null if absent)

} jpse_src_t;

/* The structure defining the information about the destination of the encoded
 * bitstream from the Jps Encoder. */
typedef struct
{
    // The handler function when output data is ready to be written
    jpse_output_handler_t  p_output_handler;

    // The argument which the encoder would pass back.
    // See definition of the jpse_output_handler_t.
    void                   *p_arg;

    // The number of supplied destination buffer(s). It can be in the range of 0 - N.
    // The encoder will try to utilize the buffer(s) as much as it can.
    // The encoder makes no guarantee to always utilize the supplied buffers for
    // performance reason. Caller shall expect output to be delivered in internal
    // working buffers of the encoder if it chooses to.
    uint32_t                buffer_cnt;

    // The pointer to supplied destination buffer(s) array.
    jpeg_buffer_t           *p_buffer;

} jpse_dst_t;

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

} jpse_scale_cfg_t;

/* The structure defining the configuration to the Jps Encoder specific
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
    jpse_scale_cfg_t     scale_cfg;

} jpse_img_cfg_t;

/* The structure defining the configuration to the Jps Encoder. */
typedef struct
{
    // Header output type: EXIF (default) or JFIF
    jpse_hdr_output_t   header_type;

    // Configuration specific to main image
    jpse_img_cfg_t      main_cfg;

    // Configuration specific to thumbnail image
    jpse_img_cfg_t      thumbnail_cfg;

    // Configure whether thumbnail is present to be encoded
    uint8_t              thumbnail_present;

    // Encoder preference
    jpse_preference_t    preference;

} jpse_cfg_t;

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
**                          Function Declarations
** ======================================================================= */

/******************************************************************************
 * Function: jpse_init
 * Description: Initializes the Jps Encoder object. Dynamic allocations take
 *              place during the call. One should always call jpse_destroy to
 *              clean up the Jps Encoder object after use.
 * Input parameters:
 *   p_obj     - The pointer to the Jps Encoder object to be initialized.
 *   p_handler - The function pointer to the handler function handling Jps
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
int jpse_init(
    jpse_obj_t            *p_obj,
    jpse_event_handler_t   p_handler,
    void                  *p_user_data);

/******************************************************************************
 * Function: jpse_get_default_config
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
int jpse_get_default_config(
    jpse_cfg_t    *p_config);

/******************************************************************************
 * Function: jpse_get_actual_config
 * Description: Get the actual conig that was used int the encoding session.
 *              Typically this is used after the encoding is done and before
 *              encoder is destroyed.
 * Input parameters:
 *   obj       - The Jps Encoder object.
 *   p_config  - The pointer to the configuration structure to be initialized
 *               with default settings.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpse_get_actual_config(
    jpse_obj_t     obj,
    jpse_cfg_t    *p_config);

/******************************************************************************
 * Function: jpse_set_source
 * Description: Assigns the image source to the Jps Encoder object.
 * Input parameters:
 *   obj       - The Jps Encoder object.
 *   p_source  - The pointer to the source object to be assigned to the Jps
 *               Encoder.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpse_set_source(
    jpse_obj_t   obj,
    jpse_src_t  *p_source);

/******************************************************************************
 * Function: jpse_set_mobicat_data
 * Description: Assigns mobicat data to Jps Encoder object.
 * Input parameters:
 *   obj                      - The Jps Encoder object.
 *  *p_mobicat_data           - Mobicat data
 *   mobicat_data_length      - length of mobicat data
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpse_set_mobicat_data(
    jpse_obj_t    obj,
    uint8_t      *p_mobicat_data,
    int32_t       mobicat_data_length);


/******************************************************************************
 * Function: jpse_set_destination
 * Description: Assigns the destination to the Jps Encoder object.
 * Input parameters:
 *   obj       - The Jps Encoder object.
 *   p_dest    - The pointer to the destination object to be assigned to the Jps
 *               Encoder.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpse_set_destination(
    jpse_obj_t   obj,
    jpse_dst_t  *p_dest);

/******************************************************************************
 * Function: jpse_set_config_3d
 * Description: Set the actual conig 3D that was used int the encoding session.
 *              Typically this is used before starting the encoder.
 * Input parameters:
 *   obj          - The Jps Encoder object.
 *   p_config_3d  - The pointer to the configuration structure of 3D to be initialized.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpse_config_3d(
    jpse_obj_t        obj,
    jps_cfg_3d_t      cfg_3d);

/******************************************************************************
 * Function: jpse_start
 * Description: Starts the Jps Encoder. Encoding will start asynchronously
 *              in a new thread if the call returns JPEGERR_SUCCESS. The caller
 *              thread should interact asynchronously with the encoder through
 *              event and output handlers.
 * Input parameters:
 *   obj             - The Jps Encoder object.
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
int jpse_start(
    jpse_obj_t        obj,
    jpse_cfg_t       *p_cfg,
    exif_info_obj_t  *p_exif_info);

/******************************************************************************
 * Function: jpse_abort
 * Description: Aborts the encoding session in progress. If there is no encoding
 *              session in progress, a JPEGERR_SUCCESS will be returned
 *              immediately. Otherwise, the call will block until the encoding
 *              is completely aborted, then return a JPEGERR_SUCCESS.
 * Input parameters:
 *   obj       - The Jps Encoder object
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpse_abort(
    jpse_obj_t obj);

/******************************************************************************
 * Function: jpse_destroy
 * Description: Destroys the Jps Encoder object, releasing all memory
 *              previously allocated.
 * Input parameters:
 *   p_obj       - The pointer to the Jps Encoder object
 * Return values: None
 * Notes: none
 *****************************************************************************/
void jpse_destroy(jpse_obj_t *p_obj);

/******************************************************************************
* Function: jpse_enqueue_output_buffer
*
Description: Enqueue output buffer(s) to queue. It accepts the pointer to
*              output buffer(s) to be enqueued and the
number of buffer(s),
*              appends output buffer(s) sequentially to the queue
*              Enqueued output buffer(s) is
checked, and
*              the number of buffer(s) to be enqueued is checked
*              against the free slots queue, and
return fail if
*              it is larger than free slots left inside queue.
* Input parameters:
*   obj                - The Jps
Encoder object.
*   p_enqueue_buf      - The pointer to enqueued buffer(s) array
*   enqueue_buf_cnt    - The number of enqueued
buffer(s)
* Return values:
*     JPEGERR_SUCCESS
*     JPEGERR_EFAILED
*     JPEGERR_ENULLPTR
*     JPEGERR_EBADPARM
* (See
jpegerr.h for description of error values.)
* Notes:
none
*****************************************************************************/
int jpse_enqueue_output_buffer(
    jpse_obj_t       obj,
    jpeg_buffer_t   *p_enqueue_buf,
    uint32_t         enqueue_buf_cnt);

#endif // #ifndef _JPSE_H
