/*========================================================================


*//** @file jpse.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (c) 1991-1998, Thomas G. Lane
See the IJG_README.txt file for more details.
Copyright (C) 2010-11 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when         who     what, where, why
--------     ---     -------------------------------------------------------
11/15/2010   staceyw Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpeg_common_private.h"
#include "jpse.h"
#include "jps_writer.h"

#include "jpegerr.h"
#include "jpeglog.h"
#include "jpeg_queue.h"
#include "jpeg_buffer_private.h"
#include "jpege_engine.h"

#include "os_thread.h"
#include "os_timer.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
// time out in milli-second to wait if queue is empty,
// this number needs to be chosen long enough that
// upper-layer can enqueue back output buffer to queue.
#define TIME_OUT_MS 9000

typedef enum
{
    JPSE_IDLE = 0,
    JPSE_ENCODING_THUMBNAIL,
    JPSE_ENCODING_MAIN,
    JPSE_ABORTING,

} jpse_state_t;

typedef struct jps_encoder_t
{
    // The Jpeg Writer
    jps_writer_t           writer;
    // The Jpeg output queue
    jpeg_queue_t           output_queue;
    // The output buffer pointer for the purpose of reuse
    // during encoding header and file size control process
    jpeg_buf_t             *p_output_buffer_ref;
    // Flag indicate thumbnail need to be dropped
    uint8_t                thumbnail_drop_flag;
    // Thumbnail buffer
    jpeg_buf_t             *p_thumbnail_buf;
    // The Jpeg Encode Engine
    jpege_engine_obj_t     engine;
    // The Jpeg event handler
    jpse_event_handler_t   p_event_handler;
    // The Jpeg encode source
    jpse_src_t            source;
    // The Jpeg encode destination
    jpse_dst_t            dest;
    // The Jpeg encode configuration
    jpse_cfg_t            config;
    // The Jps encode 3d configuration
    jps_cfg_3d_t          config_3d;
    // The jpag mobicat data config
    jpse_mbc_t            mobicat_data;
    // Pointer to user data
    void                  *p_user_data;
    // Scaled quantization tables
    uint16_t              *p_main_luma_qtable;
    uint16_t              *p_main_chroma_qtable;
    uint16_t              *p_tn_luma_qtable;
    uint16_t              *p_tn_chroma_qtable;
    // The Jpeg encode engine destination
    jpege_engine_dst_t     engine_dst;
    // The state of the encoder
    jpse_state_t           state;
    // os mutex
    os_mutex_t             mutex;
    // os condition variable
    os_cond_t              cond;
#ifdef _DEBUG
    os_timer_t             timer;
#endif

} jps_encoder_t;


/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
/* Standard Quantization Tables */
static const uint16_t standard_luma_q_tbl[] =
{
    16,  11,  10,  16,  24,  40,  51,  61,
    12,  12,  14,  19,  26,  58,  60,  55,
    14,  13,  16,  24,  40,  57,  69,  56,
    14,  17,  22,  29,  51,  87,  80,  62,
    18,  22,  37,  56,  68, 109, 103,  77,
    24,  35,  55,  64,  81, 104, 113,  92,
    49,  64,  78,  87, 103, 121, 120, 101,
    72,  92,  95,  98, 112, 100, 103,  99
};

static const uint16_t standard_chroma_q_tbl[] =
{
    17,  18,  24,  47,  99,  99,  99,  99,
    18,  21,  26,  66,  99,  99,  99,  99,
    24,  26,  56,  99,  99,  99,  99,  99,
    47,  66,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99,
    99,  99,  99,  99,  99,  99,  99,  99
};

/*lint -save -e785 Intentional insufficient initializers */
/* Standard Huffman Tables */
static const jpeg_huff_table_t standard_luma_dc_huff_tbl =
{
    /* Bits (0-based) */
    {0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
    /* huffValues */
    { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
};

static const jpeg_huff_table_t standard_chroma_dc_huff_tbl =
{
    /* Bits (0-based) */
    {0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0},
    /* huffValues */
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
};

static const jpeg_huff_table_t standard_luma_ac_huff_tbl =
{
    /* bits (0-based) */
    {0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d},
    /* huffValues */
    {0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
     0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
     0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
     0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
     0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
     0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
     0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
     0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
     0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
     0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
     0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
     0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
     0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
     0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
     0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
     0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
     0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
     0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
     0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
     0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
     0xf9, 0xfa},
};

static const jpeg_huff_table_t standard_chroma_ac_huff_tbl =
{
    /* Bits (0-based) */
    {0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77},
    /* huffValues */
    {0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
     0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
     0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
     0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
     0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
     0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
     0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
     0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
     0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
     0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
     0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
     0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
     0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
     0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
     0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
     0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
     0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
     0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
     0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
     0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
     0xf9, 0xfa},
};
/*lint -restore */

/* -----------------------------------------------------------------------
** Forward Declarations of helper functions
** ----------------------------------------------------------------------- */
static void jpse_engine_event_handler(
    jpege_engine_obj_t *p_obj,  // The encode engine object calling it
    jpeg_event_t        event,  // The event code
    void               *p_arg); // An optional argument for that event.

static int jpse_engine_output_handler(
    jpege_engine_obj_t  *p_obj,    // The encode engine object calling it
    jpeg_buf_t          *p_buf,    // The buffer containing the output bitstream_t
    uint8_t             last_buf_flag); // The flag to indicate last output buffer

static int jps_writer_output_handler(
    jpse_obj_t          encoder,
    jpeg_buf_t          *p_buf);

static int jpse_configure(jps_encoder_t *p_encoder, jpse_cfg_t *p_cfg);
static int jpse_validate_image_data(jpse_img_data_t *p_img);
static void jpse_get_default_img_cfg(jpse_img_cfg_t *p_img_cfg);
static jpeg_quant_table_t jpse_scale_quant_table(uint16_t **dst_quant_tbl,
                                           uint16_t *src_quant_tbl,
                                           uint32_t quality);
static int jpse_try_engines(jpse_obj_t obj, jpse_preference_t pref);

// Get next output buffer from queue
static int  jpse_get_output_buffer(jpege_engine_obj_t  *p_obj,
                                    jpeg_buf_t   **pp_dequeue_buffer);
static int  jpsw_get_output_buffer(jps_encoder_t  *p_encoder,
                                    jpeg_buf_t      **pp_dequeue_buffer);
/* -----------------------------------------------------------------------
** Extern variables
** ----------------------------------------------------------------------- */
extern jpege_engine_lists_t jpse_engine_try_lists;

/* =======================================================================
**                          Function Definitions
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
    void                  *p_user_data)
{
    int rc;
    jps_encoder_t *p_encoder;

    // Input validation
    if (!p_handler)
        return JPEGERR_ENULLPTR;

    // Allocate for the Jps Encoder structure
    p_encoder = (jps_encoder_t *)JPEG_MALLOC(sizeof(jps_encoder_t));
    if (!p_encoder)
        return JPEGERR_EMALLOC;

    // Initialize all fields in the encoder structure
    STD_MEMSET(p_encoder, 0, sizeof(jps_encoder_t));

    // Store the event handler and user data
    p_encoder->p_event_handler = p_handler;
    p_encoder->p_user_data = p_user_data;

    // Initialize jpeg writer
    rc = jpsw_init(&(p_encoder->writer), (jpse_obj_t)p_encoder, jps_writer_output_handler);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpse_init: jpegw_init failed\n");
        JPEG_FREE(p_encoder);
        return rc;
    }
    // Initialize jpeg output queue
    rc = jpeg_queue_init(&(p_encoder->output_queue));
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpse_init: jpeg_queue_init failed\n");
        // Destroy the writer
        jpsw_destroy(&(p_encoder->writer));
        JPEG_FREE(p_encoder);
        return rc;
    }
    // Init buffer to hold the thumbnail data
    jpeg_buffer_init((jpeg_buffer_t *)&(p_encoder->p_thumbnail_buf));

    // Initialize thread condition variable and mutex
    (void)os_mutex_init(&(p_encoder->mutex));
    (void)os_cond_init(&(p_encoder->cond));

    // Cast the created encoder into the opaque object pointer
    *p_obj = (jpse_obj_t)p_encoder;
    return JPEGERR_SUCCESS;
}

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
    jpse_cfg_t  *p_config)
{
    if (!p_config)
        return JPEGERR_ENULLPTR;

    // Zero out all fields in the structure
    // Automatically take care of defaulting:
    // header_type: EXIF
    // thumbnail_present: 0 (absent)
    // rotation in each image config: 0
    // restart interval in each image config: 0
    STD_MEMSET(p_config, 0, sizeof(jpse_cfg_t));

    // Set default for main and thumbnail
    jpse_get_default_img_cfg(&(p_config->main_cfg));
    jpse_get_default_img_cfg(&(p_config->thumbnail_cfg));

    return JPEGERR_SUCCESS;
}

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
    jpse_cfg_t    *p_config)
{
    jps_encoder_t *p_encoder = (jps_encoder_t *)obj;
    if (!p_encoder)
        return JPEGERR_EUNINITIALIZED;

    if (!p_config)
        return JPEGERR_ENULLPTR;

    STD_MEMSET(p_config, 0, sizeof(jpse_cfg_t));

    // copy actual config used by encoder
    *p_config = (*p_encoder).config;

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: jpse_set_source
 * Description: Assigns the image source to the Jps Encoder object.
 * Input parameters:
 *   obj       - The Jps Encoder object
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
    jpse_obj_t    obj,
    jpse_src_t   *p_source)
{
    int rc;
    jps_encoder_t *p_encoder = (jps_encoder_t *)obj;
    if (!p_encoder)
        return JPEGERR_EUNINITIALIZED;

    // Input validation
    if (!p_source || !p_source->p_main)
       return JPEGERR_ENULLPTR;

    // Validate main image data
    rc = jpse_validate_image_data(p_source->p_main);
    if (JPEG_FAILED(rc)) return rc;

    // Validate thumbnail image if present
    if (p_source->p_thumbnail)
    {
        rc = jpse_validate_image_data(p_source->p_thumbnail);
        if (JPEG_FAILED(rc)) return rc;
    }

    // If all validation passed, store the source pointer
    p_encoder->source = *p_source;
    return JPEGERR_SUCCESS;
}

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
    int32_t       mobicat_data_length)
{
    uint8_t *p_buffer;

    jps_encoder_t *p_encoder = (jps_encoder_t *)obj;
    if (!p_encoder)
        return JPEGERR_EUNINITIALIZED;

    // validate input
    if (0==mobicat_data_length)
    {
        // if mobicat data length is 0, no data to set.
        // consider it done.
        p_encoder->mobicat_data.length = 0;
        p_encoder->mobicat_data.data = 0;

        return JPEGERR_SUCCESS;
    }
    if (!p_mobicat_data)
    {
        JPEG_DBG_ERROR("jpeg_set_mobicat_data: invalid mobicat data buffer\n");
        return JPEGERR_EBADPARM;
    }

    // check if mobicat data is set before or not
    // and delete it if it is previously set
    if (p_encoder->mobicat_data.data)
    {
        p_encoder->mobicat_data.length = 0;
        JPEG_FREE(p_encoder->mobicat_data.data);
    }

    // allocate internal buffer for mobicat data
    p_buffer = (uint8_t *)JPEG_MALLOC(sizeof(uint8_t)*mobicat_data_length);
    if (!p_buffer)
        return JPEGERR_EMALLOC;

    // copy data over to internal buffer
    STD_MEMMOVE(p_buffer, p_mobicat_data, mobicat_data_length);

    p_encoder->mobicat_data.length = mobicat_data_length;
    p_encoder->mobicat_data.data = p_buffer;

    return JPEGERR_SUCCESS;
}


/******************************************************************************
 * Function: jpse_set_destination
 * Description: Assigns the destination to the Jps Encoder object.
 * Input parameters:
 *   obj       - The Jps Encoder object
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
    jpse_dst_t  *p_dest)
{
    uint8_t  i;
    jps_encoder_t *p_encoder = (jps_encoder_t *)obj;
    if (!p_encoder)
        return JPEGERR_EUNINITIALIZED;

    // Input validations
    if (!p_dest || !p_dest->p_output_handler)
        return JPEGERR_ENULLPTR;

    if (p_dest->buffer_cnt < 1)
    {
        JPEG_DBG_ERROR("jpse_set_destination: invalid buffer cnt\n");
        return JPEGERR_EBADPARM;
    }

    // Check enqueue output buffer pointer, size and offset
    for (i = 0; i < p_dest->buffer_cnt; i++)
    {
        jpeg_buf_t *p_buf = (jpeg_buf_t *)*(p_dest->p_buffer + i);
        if (!p_buf || !p_buf->ptr || !p_buf->size || p_buf->offset >= p_buf->size)
        {
            JPEG_DBG_ERROR("jpse_set_destination: invalid outpt buffer\n");
            return JPEGERR_EBADPARM;
        }
    }

    // Store the dest pointer
    p_encoder->dest = *p_dest;

    // Derive engine destination
    p_encoder->engine_dst.p_output_handler = &jpse_engine_output_handler;
    p_encoder->engine_dst.p_get_output_buffer_handler = &jpse_get_output_buffer;

    return JPEGERR_SUCCESS;
}


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
    jpse_obj_t       obj,
    jps_cfg_3d_t     cfg_3d)
{
    jps_encoder_t *p_encoder = (jps_encoder_t *)obj;

    if (!p_encoder)
    {
        JPEG_DBG_ERROR("jpse_config_3d: invalid Jps Encoder object\n");
        return JPEGERR_EUNINITIALIZED;
    }

    p_encoder->config_3d = cfg_3d;

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: jpse_start
 * Description: Starts the Jps Encoder. Encoding will start asynchronously
 *              in a new thread if the call returns JPEGERR_SUCCESS. The caller
 *              thread should interact asynchronously with the encoder through
 *              event and output handlers.
 * Input parameters:
 *   obj             - The Jps Encoder object.
 *   p_cfg           - The pointer to the configuration structure.
 *   p_exif_info     - The pointer to the Exif Info object in which the encoder
 *                     should obtain and exif tag information from. It can be
 *                     set to NULL if no exif tag information is intended to be
 *                     written.
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
    exif_info_obj_t  *p_exif_info)
{
    int rc;
    jps_encoder_t *p_encoder = (jps_encoder_t *)obj;

    if (!p_encoder)
        return JPEGERR_EUNINITIALIZED;

    // Check whether the Jps Encoder is IDLE before proceeding
    if (p_encoder->state != JPSE_IDLE)
        return JPEGERR_EBADSTATE;

    // Reset output queue
    rc = jpeg_queue_reset(p_encoder->output_queue);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpse_start: failed to reset output queue\n");
        return rc;
    }

    // Configure the Jps Writer
    jpsw_configure (&(p_encoder->writer),
                    &(p_encoder->source),
                    &(p_encoder->dest),
                    &(p_encoder->config),
                    &(p_encoder->config_3d),
                    &(p_encoder->mobicat_data),
                    p_exif_info);

    // Configure the encoder
    rc = jpse_configure(p_encoder, p_cfg);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpse_start: failed to configure encoder\n");
        return rc;
    }

    // Try to create and initialize available underlying engines
    os_mutex_lock(&(p_encoder->mutex));

    if (p_encoder->state == JPSE_ABORTING)
    {
        p_encoder->state = JPSE_IDLE;
        os_mutex_unlock(&(p_encoder->mutex));
        return JPEGERR_EFAILED;
    }
    if (p_encoder->p_thumbnail_buf)
    {
        rc = jpeg_buffer_reset((jpeg_buffer_t)p_encoder->p_thumbnail_buf);
    }
    if (JPEG_SUCCEEDED(rc))
    {
        // Start Encode Engine on thumbnail if it is present
        // otherwise start on main
        if (p_cfg->thumbnail_present)
        {
            // Allocate thumbnail buffer if necessary,
            // then it uses this buffer for output during encoding thumbnail,
            // and writes this thumbnail buffer to header.
            if (p_encoder->p_thumbnail_buf)
            {
                uint32_t thumbnail_size;
                thumbnail_size = STD_MIN(p_encoder->source.p_thumbnail->width *
                                         p_encoder->source.p_thumbnail->height * 3,
                                         EXIF_MAX_APP1_LENGTH);
                if (JPEG_SUCCEEDED(rc))
                {
                    if (p_cfg->preference == JPS_ENCODER_PREF_SOFTWARE_ONLY)
                    {
                        rc = jpeg_buffer_allocate((jpeg_buffer_t)p_encoder->p_thumbnail_buf, thumbnail_size, false);
                    }
                    else
                    {
                        /*else, allocate it from pmem*/
                        rc = jpeg_buffer_allocate((jpeg_buffer_t)p_encoder->p_thumbnail_buf, thumbnail_size, true);
                    }
                    if (JPEG_SUCCEEDED(rc))
                    {
                        p_encoder->state = JPSE_ENCODING_THUMBNAIL;

                        rc = jpse_try_engines(obj, p_cfg->preference);
                        if (JPEG_SUCCEEDED(rc))
                        {
                            rc = p_encoder->engine.start(&(p_encoder->engine), (jpege_img_cfg_t *)&(p_encoder->config.thumbnail_cfg),
                                                         (jpege_img_data_t *)p_encoder->source.p_thumbnail,
                                                         &(p_encoder->engine_dst));
                        }
                    }
                }
            }
            os_mutex_unlock(&(p_encoder->mutex));
        }
        else
        {
            p_encoder->state = JPSE_ENCODING_MAIN;

            rc = jpse_try_engines(obj, p_cfg->preference);
            if (JPEG_SUCCEEDED(rc))
            {
                // Temporary workaround - transpose quant tables as indicated by engine
                if (p_encoder->engine_dst.transposed)
                {
                    uint16_t i, j, tmp;

                    for (i = 0; i < 8; i++)
                        for (j = 0; j < i; j++)
                        {
                            /* Swap luma qtable[i][j] and qtable[j][i] */
                            tmp = p_encoder->config.main_cfg.luma_quant_tbl[i*8+j];
                            p_encoder->config.main_cfg.luma_quant_tbl[i*8+j] =
                                p_encoder->config.main_cfg.luma_quant_tbl[j*8+i];
                            p_encoder->config.main_cfg.luma_quant_tbl[j*8+i] = tmp;

                            /* Swap chroma qtable[i][j] and qtable[j][i] */
                            tmp = p_encoder->config.main_cfg.chroma_quant_tbl[i*8+j];
                            p_encoder->config.main_cfg.chroma_quant_tbl[i*8+j] =
                                p_encoder->config.main_cfg.chroma_quant_tbl[j*8+i];
                            p_encoder->config.main_cfg.chroma_quant_tbl[j*8+i] = tmp;
                        }
                }

                // Write file header
                rc = jpsw_emit_header(&(p_encoder->writer), p_encoder->p_thumbnail_buf);

                // Start Encode Engine on main image
                if (JPEG_SUCCEEDED(rc))
                {
                    rc = p_encoder->engine.start(&(p_encoder->engine), (jpege_img_cfg_t *)&(p_encoder->config.main_cfg),
                                                 (jpege_img_data_t *)p_encoder->source.p_main, &(p_encoder->engine_dst));
                }
            }
            os_mutex_unlock(&(p_encoder->mutex));
        }
    }
    if (JPEG_FAILED(rc))
    {
        os_mutex_lock(&(p_encoder->mutex));
        p_encoder->state = JPSE_IDLE;
        os_mutex_unlock(&(p_encoder->mutex));
    }

    return rc;
}

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
int jpse_abort(jpse_obj_t obj)
{
    int rc = JPEGERR_SUCCESS;
    jps_encoder_t *p_encoder = (jps_encoder_t *)obj;
    if (!p_encoder)
        return JPEGERR_EUNINITIALIZED;

    os_mutex_lock(&(p_encoder->mutex));
    if (p_encoder->state != JPSE_IDLE)
    {
        JPEG_DBG_LOW("jpse_abort: abort.\n");
        p_encoder->state = JPSE_ABORTING;
        os_mutex_unlock(&(p_encoder->mutex));
        JPEG_DBG_LOW("jpse_abort: aborting output req queue.\n");
        rc = jpeg_queue_abort(&(p_encoder->output_queue));
        JPEG_DBG_LOW("jpse_abort: output req queue aborted.\n");
        if (JPEG_SUCCEEDED(rc))
        {
            JPEG_DBG_LOW("jpse_abort: aborting engine.\n");
            rc = p_encoder->engine.abort(&(p_encoder->engine));
            JPEG_DBG_LOW("jpse_abort: engine aborted.\n");
        }
    }
    else
        os_mutex_unlock(&(p_encoder->mutex));

    return rc;
}

/******************************************************************************
 * Function: jpse_destroy
 * Description: Destroys the Jps Encoder object, releasing all memory
 *              previously allocated.
 * Input parameters:
 *   p_obj       - The pointer to the Jps Encoder object
 * Return values: None
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
void jpse_destroy(jpse_obj_t *p_obj)
{
    if (p_obj)
    {
        jps_encoder_t *p_encoder = (jps_encoder_t *)(*p_obj);
        if (p_encoder)
        {
            JPEG_DBG_LOW("jpse_destroy\n");

            // Free thumbnail buffer if allocated
            if (p_encoder->p_thumbnail_buf)
            {
                jpeg_buffer_destroy((jpeg_buffer_t *)&(p_encoder->p_thumbnail_buf));
            }
            // Destroy the engine
            if (p_encoder->engine.destroy)
            {
                p_encoder->engine.destroy(&(p_encoder->engine));
            }
            // Destroy the writer
            jpsw_destroy(&(p_encoder->writer));
            // Destroy the output queue
            jpeg_queue_destroy(&(p_encoder->output_queue));

            // Destroy any scaled quant tables allocated
            JPEG_FREE(p_encoder->p_main_luma_qtable);
            JPEG_FREE(p_encoder->p_main_chroma_qtable);
            JPEG_FREE(p_encoder->p_tn_luma_qtable);
            JPEG_FREE(p_encoder->p_tn_chroma_qtable);
            JPEG_FREE(p_encoder->mobicat_data.data);

            // Destroy thread condition variable and mutex
            (void)os_mutex_destroy(&(p_encoder->mutex));
            (void)os_cond_destroy(&(p_encoder->cond));

            // Release memory of encoder
            JPEG_FREE(p_encoder);
            *p_obj = NULL;
        }
        jpeg_show_leak();
    }
}

static void jpse_engine_event_handler(
    jpege_engine_obj_t *p_obj,  // The encode engine object calling it
    jpeg_event_t        event,  // The event code
    void               *p_arg)  // An optional argument for that event.
{
    int rc = JPEGERR_SUCCESS;
    jps_encoder_t *p_encoder;
    if (!p_obj)
    {
        JPEG_DBG_ERROR("jpse_engine_event_handler: p_obj NULL!\n");
        return;
    }
    p_encoder = (jps_encoder_t *)p_obj->encoder;

    // if Encoding Thumbnail done and thumbnail buffer is Full
    if (p_encoder->state == JPSE_ENCODING_THUMBNAIL &&
        p_encoder->thumbnail_drop_flag)
    {
        // will drop the thumbnail when encode header,
        // then contiue to encode main image
        event = JPEG_EVENT_DONE;
    }

    // event handling will not be interruptable
    os_mutex_lock(&(p_encoder->mutex));
    if (p_encoder->state == JPSE_ABORTING)
    {
        //p_encoder->state = JPSE_IDLE;
        os_mutex_unlock(&(p_encoder->mutex));
        return;
    }

    switch (event)
    {
    case JPEG_EVENT_DONE:

        // Take action based on the state
        // Done with thumbnail
        if (p_encoder->state == JPSE_ENCODING_THUMBNAIL)
        {
            // Enter critical section
            p_encoder->state = JPSE_ENCODING_MAIN;
            // Leave critical section

            // Write file header
            rc = jpsw_emit_header(&(p_encoder->writer),p_encoder->p_thumbnail_buf);

            // Thumbnail does not fit into App1 marker and is dropped
            // Needs to notify the upper layer
            if (rc == JPEGERR_THUMBNAIL_DROPPED)
            {
                p_encoder->p_event_handler(p_encoder->p_user_data, JPEG_EVENT_THUMBNAIL_DROPPED, NULL);
                rc = JPEGERR_SUCCESS;
            }

            if (JPEG_SUCCEEDED(rc))
            {
                // Start encode main image
                if(p_encoder->config.preference >=0 && p_encoder->config.preference < JPS_ENCODER_PREF_MAX)
                     rc = jpse_try_engines((jpse_obj_t)p_obj->encoder, p_encoder->config.preference);
                else
                    rc = JPEGERR_EBADPARM;
                if (JPEG_SUCCEEDED(rc)) {
                    rc = p_encoder->engine.start(&(p_encoder->engine), (jpege_img_cfg_t *)&(p_encoder->config.main_cfg),
                                                 (jpege_img_data_t *)p_encoder->source.p_main, &(p_encoder->engine_dst));
                }
            }
            if (JPEG_FAILED(rc))
            {
                // Change state to IDLE
                p_encoder->state = JPSE_IDLE;
                p_encoder->p_event_handler(p_encoder->p_user_data, JPEG_EVENT_ERROR, NULL);
            }
        }
        // Done with main image
        else if (p_encoder->state == JPSE_ENCODING_MAIN)
        {
            // Change state to IDLE
            p_encoder->state = JPSE_IDLE;
            p_encoder->p_event_handler(p_encoder->p_user_data, JPEG_EVENT_DONE, NULL);
        }
        break;
    case JPEG_EVENT_WARNING:
    case JPEG_EVENT_ERROR:
        // Forward to the upper layer
        p_encoder->p_event_handler(p_encoder->p_user_data, event, p_arg);
        break;
    }
    os_mutex_unlock(&(p_encoder->mutex));
}

static int jpse_engine_output_handler(
    jpege_engine_obj_t  *p_obj,    // The encode engine object calling it
    jpeg_buf_t          *p_buf,    // The buffer containing the output bitstream_t
    uint8_t              last_buf_flag) // The flag to indicate last output buffer
{
    int rc = JPEGERR_SUCCESS;
    jps_encoder_t *p_encoder;
    if (!p_obj)
    {
        JPEG_DBG_ERROR("jpse_engine_output_handler: p_obj NULL!\n");
        return JPEGERR_EFAILED;
    }
    p_encoder = (jps_encoder_t *)p_obj->encoder;

    // Forward the output to file writer if the output is for thumbnail
    if (p_encoder->state != JPSE_ENCODING_THUMBNAIL)
    {
        // Forward to upper layer
        rc = p_encoder->dest.p_output_handler(p_encoder->p_user_data,
                                              p_encoder->dest.p_arg,
                                              (jpeg_buffer_t)p_buf,
                                              last_buf_flag);
        return rc;
    }
    return rc;
}

static int jps_writer_output_handler(
    jpse_obj_t          encoder,
    jpeg_buf_t         *p_buf)
{
    jps_encoder_t *p_encoder = (jps_encoder_t *)encoder;
    jpeg_buf_t *p_dest;
    uint32_t bytes_left = p_buf->offset;
    uint32_t bytes_written = 0;
    uint32_t bytes_to_copy;
    int      rc = JPEGERR_SUCCESS;

    // Check encoder pointer
    if (!p_encoder)
    {
        JPEG_DBG_ERROR("jps_writer_output_handler: failed with null encoder\n");
        return JPEGERR_ENULLPTR;
    }

    // Copy it to the destination buffer
    while (bytes_left)
    {
        // Get the next output buffer from buffer(s) queue
        rc = jpsw_get_output_buffer(p_encoder, &p_dest);
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jps_writer_output_handler: failed to get next output buffer\n");
            return rc;
        }
        if (!p_dest)
        {
            JPEG_DBG_ERROR("jps_writer_output_handler: failed to get next output buffer\n");
            return JPEGERR_EFAILED;
        }
        bytes_to_copy = STD_MIN(bytes_left, p_dest->size - p_dest->offset);

        STD_MEMMOVE(p_dest->ptr + p_dest->offset, p_buf->ptr + bytes_written, bytes_to_copy);
        bytes_left     -= bytes_to_copy;
        bytes_written  += bytes_to_copy;
        p_dest->offset += bytes_to_copy;

        // Deliver it if it's full
        if (p_dest->offset == p_dest->size)
        {
            rc = p_encoder->dest.p_output_handler(p_encoder->p_user_data,
                                                  p_encoder->dest.p_arg,
                                                  (jpeg_buffer_t)p_dest,
                                                  false);
            if (p_encoder->p_output_buffer_ref)
            {
                p_encoder->p_output_buffer_ref = NULL;
            }
        }
    }
    return rc;
}

static int jpse_validate_image_data(jpse_img_data_t *p_img)
{
    uint32_t i;

    // Validate input pointer
    if (!p_img)
        return JPEGERR_ENULLPTR;

    // Validate color format
    if ((p_img->color_format > YCBCRLP_H1V1) && (p_img->color_format < JPEG_BITSTREAM_H2V2))
    {
        JPEG_DBG_ERROR("jpege_validate_image_data: RGB input not supported: %d\n",
                     p_img->color_format);
        return JPEGERR_EUNSUPPORTED;
    }

    // Validate fragments
    if (p_img->fragment_cnt < 1 || p_img->fragment_cnt > MAX_FRAGMENTS)
        return JPEGERR_EBADPARM;
    if (p_img->color_format <= YCBCRLP_H1V1)
    {
        for (i = 0; i < p_img->fragment_cnt; i++)
        {
            if (!((jpeg_buf_t *)(p_img->p_fragments[i].color.yuv.luma_buf))->ptr ||
                !((jpeg_buf_t *)(p_img->p_fragments[i].color.yuv.chroma_buf))->ptr)
            {
                JPEG_DBG_ERROR("jpege_validate_image_data: broken fragment (%d)\n", i);
                return JPEGERR_EBADPARM;
            }
        }
    }
    else if ((p_img->color_format >= JPEG_BITSTREAM_H2V2) &&
             (p_img->color_format < JPEG_COLOR_FORMAT_MAX))
    {
        for (i = 0; i < p_img->fragment_cnt; i++)
        {
            if (!((jpeg_buf_t *)(p_img->p_fragments[i].color.bitstream.bitstream_buf))->ptr )
            {
                JPEG_DBG_ERROR("jpege_validate_image_data: broken fragment (%d)\n", i);
                return JPEGERR_EBADPARM;
            }
        }
    }

    // Validate dimension
    if (!p_img->height || !p_img->width)
    {
        JPEG_DBG_ERROR("jpege_validate_image_data: bad image dimension: (%dx%d)\n",
                     p_img->width, p_img->height);
        return JPEGERR_EBADPARM;
    }

    return JPEGERR_SUCCESS;
}

int jpse_validate_image_config(jpse_img_cfg_t *p_cfg)
{
    if (p_cfg->quality < 1 || p_cfg->quality > 100 ||         // Validate quality factor
        !p_cfg->luma_quant_tbl || !p_cfg->chroma_quant_tbl || // Validate quant tables
        p_cfg->rotation_degree_clk > 0)                       // Validate rotation (not support rotation)
    {
        return JPEGERR_EBADPARM;
    }
    return JPEGERR_SUCCESS;
}

static int jpse_configure(jps_encoder_t *p_encoder, jpse_cfg_t *p_cfg)
{
    int rc;

    if (!p_cfg)
        return JPEGERR_ENULLPTR;

    // Check header type
    if (p_cfg->header_type != JPS_OUTPUT_JFIF && p_cfg->header_type != JPS_OUTPUT_EXIF)
        return JPEGERR_EBADPARM;

    // Check preference
    if (p_cfg->preference >= JPS_ENCODER_PREF_MAX)
        return JPEGERR_EBADPARM;

    // Enqueue the output buffer(s) to queue
    rc = jpeg_queue_enqueue(p_encoder->output_queue,
                            (void **)(p_encoder->dest.p_buffer),
                            p_encoder->dest.buffer_cnt);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_configure: enqueue output buffer failed\n");
        return rc;
    }

    // Make a copy of the config
    p_encoder->config = *p_cfg;

    // Check image specific configuration
    rc = jpse_validate_image_config(&(p_cfg->main_cfg)); // main
    if (JPEG_FAILED(rc)) return rc;

    if (p_cfg->main_cfg.scale_cfg.enable)  // Validate zoom (not support zoom)
    {
        JPEG_DBG_ERROR("jpege_configure: no zoom supported for jps encoder\n");
        return JPEGERR_EBADPARM;
    }

    // Scale quant tables
    p_encoder->config.main_cfg.luma_quant_tbl = jpse_scale_quant_table(&(p_encoder->p_main_luma_qtable),
                                                                        p_cfg->main_cfg.luma_quant_tbl,
                                                                        p_cfg->main_cfg.quality);
    p_encoder->config.main_cfg.chroma_quant_tbl = jpse_scale_quant_table(&(p_encoder->p_main_chroma_qtable),
                                                                          p_cfg->main_cfg.chroma_quant_tbl,
                                                                          p_cfg->main_cfg.quality);

    if (!p_encoder->config.main_cfg.luma_quant_tbl || !p_encoder->config.main_cfg.chroma_quant_tbl)
    {
        return JPEGERR_EMALLOC;
    }

    if (p_cfg->thumbnail_present)
    {
        // Apply same check and configuration to thumbnail
        rc = jpse_validate_image_config(&(p_cfg->thumbnail_cfg));
        if (JPEG_FAILED(rc)) return rc;

        // Scale quant tables
        p_encoder->config.thumbnail_cfg.luma_quant_tbl = jpse_scale_quant_table(&(p_encoder->p_tn_luma_qtable),
                                                                                 p_cfg->thumbnail_cfg.luma_quant_tbl,
                                                                                 p_cfg->thumbnail_cfg.quality);
        p_encoder->config.thumbnail_cfg.chroma_quant_tbl = jpse_scale_quant_table(&(p_encoder->p_tn_chroma_qtable),
                                                                                   p_cfg->thumbnail_cfg.chroma_quant_tbl,
                                                                                   p_cfg->thumbnail_cfg.quality);
        if (!p_encoder->config.thumbnail_cfg.luma_quant_tbl || !p_encoder->config.thumbnail_cfg.chroma_quant_tbl)
        {
            return JPEGERR_EMALLOC;
        }
    }
    return JPEGERR_SUCCESS;
}

static void jpse_get_default_img_cfg(jpse_img_cfg_t *p_img_cfg)
{
    // Default quality is 75
    p_img_cfg->quality = 75;

    // Set standard quantization and Huffman tables
    p_img_cfg->luma_quant_tbl     = (jpeg_quant_table_t)standard_luma_q_tbl;
    p_img_cfg->chroma_quant_tbl   = (jpeg_quant_table_t)standard_chroma_q_tbl;
    p_img_cfg->luma_dc_huff_tbl   = standard_luma_dc_huff_tbl;
    p_img_cfg->chroma_dc_huff_tbl = standard_chroma_dc_huff_tbl;
    p_img_cfg->luma_ac_huff_tbl   = standard_luma_ac_huff_tbl;
    p_img_cfg->chroma_ac_huff_tbl = standard_chroma_ac_huff_tbl;
}

/**
 * This function takes care of allocating a new table to hold the
 * the scaled quantization table. The pointer to the newly
 * allocated table will be assigned back to the dst_quant_tbl so
 * it can be released later on. The returned table pointer may
 * be pointing to the newly allocated table or the same source
 * table in case no scaling is needed.
 */
static jpeg_quant_table_t jpse_scale_quant_table(uint16_t **dst_quant_tbl,
                                                 uint16_t *src_quant_tbl,
                                                 uint32_t quality_factor)
{
    int i;
    double scale_factor;
    uint16_t *dst_qtable = *dst_quant_tbl;

    // 50% is equalivalent to no scaling
    if (quality_factor == 50)
    {
        return (jpeg_quant_table_t)src_quant_tbl;
    }

    // Allocate destination table if not yet allocated
    if (!dst_qtable)
    {
        dst_qtable = (uint16_t *)JPEG_MALLOC(64 * sizeof(uint16_t));
        if (!dst_qtable) return NULL;
    }

    CLAMP(quality_factor, 1, 98);

    // Turn quality percent into scale factor
    if (quality_factor > 50)
    {
        scale_factor = 50.0 / (float) (100 - quality_factor);
    }
    else
    {
        scale_factor = (float) quality_factor / 50.0;
    }

    // Scale quant entries
    for (i = 0; i < 64; i++)
    {
        // Compute new value based on input percent
        // and on the 50% table (low)
        // Add 0.5 after the divide to round up fractional divides to be
        // more conservative.
        dst_qtable[i] = (uint16_t) (((double)src_quant_tbl[i] / scale_factor) + 0.5);

        // Clamp
        CLAMP (dst_qtable[i], 1, 255);
    }
    *dst_quant_tbl = dst_qtable;
    return dst_qtable;
}

static int jpege_check_start_param(jps_encoder_t *p_encoder)
{
    int rc = JPEGERR_EFAILED;

    // Check to see if the engine supports the requested encoding
    // Temporary workaround - p_encoder->engine_dst.transposed is implicitly
    // cleared by BS engine which is the first one in engine list
    if (p_encoder->state == JPSE_ENCODING_MAIN)
    {
        rc = p_encoder->engine.check_start_param((jpege_img_cfg_t *)&(p_encoder->config.main_cfg),
                                                 (jpege_img_data_t *)p_encoder->source.p_main,
                                                 &(p_encoder->engine_dst));
    }
    else if (p_encoder->state == JPSE_ENCODING_THUMBNAIL)
    {
        // Check thumbnail configuration also if necessary
        if (p_encoder->config.thumbnail_present)
        {
            rc = p_encoder->engine.check_start_param((jpege_img_cfg_t *)&(p_encoder->config.thumbnail_cfg),
                                                     (jpege_img_data_t *)p_encoder->source.p_thumbnail,
                                                     &(p_encoder->engine_dst));
        }
    }

    return rc;
}

static int jpse_try_engines(jpse_obj_t obj, jpse_preference_t pref)
{
    jps_encoder_t *p_encoder = (jps_encoder_t *)obj;
    jpege_engine_profile_t** profile_list = jpse_engine_try_lists[pref];

    // Go through the engine try list according to the pref
    while (*profile_list)
    {
        int rc = JPEGERR_SUCCESS;
        jpege_engine_profile_t *engine_profile = *profile_list;

        // Check if the same engine is previously created and init-ed
        if (p_encoder->engine.create == engine_profile->create_func &&
            p_encoder->engine.is_initialized)
        {
            rc = jpege_check_start_param(p_encoder);
            if (JPEG_SUCCEEDED(rc))
            {
                JPEG_DBG_MED("jpege_try_engines: engine %s already in use\n", engine_profile->engine_name);
                return rc;
            }
        }

        // Create the engine
        engine_profile->create_func(&(p_encoder->engine), (jpege_obj_t)obj);

        rc = jpege_check_start_param(p_encoder);
        if (JPEG_SUCCEEDED(rc))
        {
            // Initialize engine
            rc = p_encoder->engine.init(&(p_encoder->engine), &jpse_engine_event_handler);
        }
        if (JPEG_SUCCEEDED(rc))
        {
            JPEG_DBG_MED("jpege_try_engines: use %s\n", engine_profile->engine_name);
            return rc;
        }
        // Failed to use the current engine, destroy it first
        if (p_encoder->engine.destroy)
        {
            p_encoder->engine.destroy(&(p_encoder->engine));
        }
        // Move on to the next one
        profile_list++;
    }
    // Exhausted all engines without success
    return JPEGERR_EFAILED;
}

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
*              it is larger than free slots left inside queue
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
    jpse_obj_t
     obj,
    jpeg_buffer_t   *p_enqueue_buf,
    uint32_t        enqueue_buf_cnt)
{
    int  rc;
    uint8_t i;
    jps_encoder_t
*p_encoder = (jps_encoder_t *)obj;

    // Check input parameter
    if ((!p_encoder) ||
        (!p_enqueue_buf))
    {

JPEG_DBG_ERROR("jpege_enqueue_output_buffer: failed with null input parameter\n");
        return JPEGERR_ENULLPTR;
    }


    //Check enqueue output buffer pointer, size and offset
    for (i = 0; i < enqueue_buf_cnt; i++)
    {
        jpeg_buf_t *p_buf =
(jpeg_buf_t *)*(p_enqueue_buf + i);
        if (!p_buf || !p_buf->ptr || !p_buf->size || p_buf->offset >= p_buf->size)
        {

          JPEG_DBG_ERROR("jpege_enqueue_output_buffer: invalid outpt buffer\n");
            return JPEGERR_EBADPARM;
        }

 }
    // Enqueue the output buffer(s) to queue
    rc = jpeg_queue_enqueue(p_encoder->output_queue, (void **)(p_enqueue_buf),
enqueue_buf_cnt);
    return rc;
}
/******************************************************************************
* Function: jpse_get_output_buffer
* Description: Get a output buffer from the buffer(s) queue.
*              It is called by jpeg encoder engine.
*              It returns the pointer to the thumbnail buffer of jpeg writer if
*              encoding thumbnail.
*              It reuse one output buffer if encoding
*              during file size control procedure.
*              It returns the pointer to a new output buffer if
*              encoding main image.
* Input parameters:
*   p_obj              - The pointer to Jps Encoder object.
*   pp_dequeue_buffer  - The pointer to dequeued buffer.
* Return values:
*   - JPEGERR_SUCCESS
*     JPEGERR_EFAILED
*     JPEGERR_ENULLPTR
*     JPEGERR_ETIMEDOUT
*     JPEGERR_THUMBNAIL_DROPPED
* Notes: none
*****************************************************************************/
static int  jpse_get_output_buffer(jpege_engine_obj_t  *p_obj,
                                    jpeg_buf_t         **pp_dequeue_buffer )
{
    jps_encoder_t  *p_encoder = (jps_encoder_t *)p_obj->encoder;
    int  rc;

    // Check encoder pointer
    if (!p_encoder)
    {
        JPEG_DBG_ERROR("jpse_get_output_buffer: failed with null encoder\n");
        return JPEGERR_ENULLPTR;
    }

    // Returns the pointer to the thumbnail buffer of jpeg writer
    // if encoding thumbnail.
    if (p_encoder->state == JPSE_ENCODING_THUMBNAIL)
    {
        if (!p_encoder->p_thumbnail_buf)
        {
            JPEG_DBG_ERROR("jpse_get_output_buffer: failed with null thumb buffer\n");
            return JPEGERR_ENULLPTR;
        }
        // Set thumbnail drop flag if the thumbnail buffer is full
        // When writing header, the thumbnail will be dropped
        if (p_encoder->p_thumbnail_buf->offset == p_encoder->p_thumbnail_buf->size)
        {
            p_encoder->thumbnail_drop_flag = 1;
            return JPEGERR_THUMBNAIL_DROPPED;
        }
        *pp_dequeue_buffer = p_encoder->p_thumbnail_buf;
        return JPEGERR_SUCCESS;
    }

    // Returns the output buffer pointer to the output buffer reference
    // and keep reuse it
    if (p_encoder->p_output_buffer_ref)
    {
        *pp_dequeue_buffer = p_encoder->p_output_buffer_ref;
        // Pass the output pointer to engine,
        // it continues the encoding of main image
        p_encoder->p_output_buffer_ref = NULL;
        return  JPEGERR_SUCCESS;
    }

    // Dequeue a buffer from the output buffer queue
    os_mutex_lock(&(p_encoder->mutex));
    if (p_encoder->state == JPSE_ABORTING)
    {
        JPEG_DBG_ERROR("jpse_get_output_buffer: encoder aborting.\n");
        os_mutex_unlock(&(p_encoder->mutex));
        return JPEGERR_EFAILED;
    }
    os_mutex_unlock(&(p_encoder->mutex));

    rc = jpeg_queue_dequeue(p_encoder->output_queue, (void **)pp_dequeue_buffer, TIME_OUT_MS);
    if JPEG_FAILED(rc)
    {
        JPEG_DBG_ERROR("jpse_get_output_buffer: failed with dequeue buffer\n");
        return rc;
    }
    return  JPEGERR_SUCCESS;
}

/******************************************************************************
* Function: jpsw_get_output_buffer
* Description: Get a output buffer from the buffer(s) queue
*              when encoding header of main image.
*              It is called by jpeg writer,
*              and it returns the pointer to a new output buffer.
*              And reuse if that buffer is NOT full.
* Input parameters:
*   p_obj               - The pointer to Jps Encoder object.
*   pp_dequeue_buffer   - The pointer to dequeued buffer.
* Return values:
*   - JPEGERR_SUCCESS
*     JPEGERR_EFAILED
*     JPEGERR_ENULLPTR
*     JPEGERR_ETIMEDOUT
* Notes: none
*****************************************************************************/
static int  jpsw_get_output_buffer(jps_encoder_t  *p_encoder,
                                   jpeg_buf_t    **pp_dequeue_buffer)
{
    int  rc;

    // Returns the pointer to a new output buffer of jpeg writer
    // if encoding header of main image.
    // And reuse if that buffer is NOT full.
    if (p_encoder->p_output_buffer_ref)
    {
        // If current buffer is not full
        // Keep reuse it
        *pp_dequeue_buffer = p_encoder->p_output_buffer_ref;
        return  JPEGERR_SUCCESS;
    }

    // Dequeue a buffer from the output buffer queue
    os_mutex_lock(&(p_encoder->mutex));
    if (p_encoder->state == JPSE_ABORTING)
    {
        JPEG_DBG_ERROR("jpsw_get_output_buffer: encoder aborted.\n");
        os_mutex_unlock(&(p_encoder->mutex));
        return JPEGERR_EFAILED;
    }
    os_mutex_unlock(&(p_encoder->mutex));

    rc = jpeg_queue_dequeue(p_encoder->output_queue, (void **)pp_dequeue_buffer, TIME_OUT_MS);
    if JPEG_FAILED(rc)
    {
        JPEG_DBG_ERROR("jpsw_get_output_buffer: failed with dequeue buffer\n");
        return rc;
    }
    p_encoder->p_output_buffer_ref = *pp_dequeue_buffer;
    return  JPEGERR_SUCCESS;
}
