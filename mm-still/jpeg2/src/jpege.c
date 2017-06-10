/*========================================================================


*//** @file jpege.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (c) 1991-1998, Thomas G. Lane
See the IJG_README.txt file for more details.

Copyright (C) 2008-2011,2014 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/still/v3/jpege/core/main/latest/src/jpege.c#15 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
10/16/14   vgaur   Added support for multi threaded fastCV
03/25/14  revathys Added stride support.
05/06/14  revathys Fixed Klockwork errors.
04/05/13   staceyw Add image fragment deliver function.
07/31/12   apant   Adding debug info to JPEG header
03/01/12  csriniva Added support for YUY2 and UYVY inputs
11/18/11   sigu    Always set the thumbnail preference to be "S/W only"
01/10/11   staceyw Added support for App2 header .
08/19/10    sv     Fix to make sure thumbnail buffer is properly reset
08/09/10   sigu    Appropriate selection of heap (for thumbnail), according
                   to the pref tag
07/22/10   sigu    Fixed compiler warnings
07/13/10   zhiminl Integrated with PPF to support rotation and scaling
                   for bitstream input.
06/15/10   leim    Fixed abort issue.
05/22/10   leim    Fixed mutex/cond leak in jpege.c
04/27/10   staceyw Added output queue reset during jpeg encoder configue,
                   and moved enqueue output buffer from jpege_set_destination()
                   to jpege_configure(), so that to support back-to-back jpeg
                   encoding.
04/20/10   leim    Added mobicat data support implementation
04/16/10   staceyw Added last output buffer flag in jpege_engine_output_handler
                   and jpege_output_handler.
02/26/10   staceyw Added support for multiple output buffers,
                   and support for handling enqueue and dequeue output
                   buffer(s) from queue during output.
02/19/10   leim    Remove jpege_obj_t parameter in function call
                   jpege_get_default_config(), so this function is static,
                   and can be called any time.
02/08/10   staceyw Added support for bitstream input with bitstream engine,
                   and before encoding main or thumbnail, choose right engine
                   for different input(yuv or bitstream).
11/02/09   staceyw Added file size control support, and allowed multiple
                   passes encoding with scaled quant tables.
10/15/09   vma     Rewrite engine picking to be based on an engine list
                   declared in a separate file which is platform-specific.
10/13/09   vma     Invoke output produced only if the destination buffer
                   is full (except the last one).
10/13/09   vma     Allowed passing back of user-defined data in output
                   produced and event handlers instead of the encoder itself.
08/03/09   vma     Switched to use the os abstraction layer (os_*)
06/23/09   zhiminl Added jpege_check_start_param().
04/23/09   vma     Added reporting of the event where thumbnail is
                   too big and is dropped.
02/19/09   vma     Fixed bug where main image output produced by DSP
                   is not sent to upper layer (to thumbnail scratch buffer
                   instead.) This is due to improper setting of state.
11/25/08   vma     Changed default encode quality to 75 instead of 50.
10/08/08   vma     Fixed abort deadlock.
07/07/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpeg_common.h"
#include "jpege.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include "jpeg_writer.h"
#include "jpeg_file_size_control.h"
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
#define TIME_OUT_MS 90000

typedef enum
{
    JPEGE_IDLE = 0,
    JPEGE_ENCODING_THUMBNAIL,
    JPEGE_ENCODING_MAIN,
    JPEGE_ABORTING,

} jpege_state_t;

typedef struct jpeg_encoder_t
{
    // The Jpeg Writer
    jpeg_writer_t          writer;
    // The Jpeg File Size Control
    jpege_file_size_control_t  file_size_control;
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
    jpege_event_handler_t  p_event_handler;
    // The Jpeg encode source
    jpege_src_t            source;
    // The Jpeg encode destination
    jpege_dst_t            dest;
    // The Jpeg encode configuration
    jpege_cfg_t            config;
    // The jpag mobicat data config
    jpege_mbc_t            mobicat_data;
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
    jpege_state_t          state;
    // os mutex
    os_mutex_t             mutex;
    // os condition variable
    os_cond_t              cond;
#ifdef _DEBUG
    os_timer_t             timer;
#endif

} jpeg_encoder_t;

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
static void jpege_engine_event_handler(
    jpege_engine_obj_t *p_obj,  // The encode engine object calling it
    jpeg_event_t        event,  // The event code
    void               *p_arg); // An optional argument for that event.

static int jpege_engine_output_handler(
    jpege_engine_obj_t  *p_obj,    // The encode engine object calling it
    jpeg_buf_t          *p_buf,    // The buffer containing the output bitstream_t
    uint8_t             last_buf_flag); // The flag to indicate last output buffer

static int jpeg_writer_output_handler(
    jpege_obj_t          encoder,
    jpeg_buf_t          *p_buf);

static int jpege_configure(jpeg_encoder_t *p_encoder, jpege_cfg_t *p_cfg);
static int jpege_validate_image_data(jpege_img_data_t *p_img);
static void jpege_get_default_img_cfg(jpege_img_cfg_t *p_img_cfg);
static jpeg_quant_table_t jpege_scale_quant_table(uint16_t **dst_quant_tbl,
                                           uint16_t *src_quant_tbl,
                                           uint32_t quality);
static int jpege_try_engines(jpege_obj_t obj, jpege_preference_t pref);

// Get next output buffer from queue
static int  jpege_get_output_buffer(jpege_engine_obj_t  *p_obj,
                                    jpeg_buf_t   **pp_dequeue_buffer);
static int  jpegw_get_output_buffer(jpeg_encoder_t  *p_encoder,
                                    jpeg_buf_t      **pp_dequeue_buffer);

// Configure restart marker for each MCU row
static int jpege_configure_restart(
    jpeg_encoder_t *p_encoder,
    jpege_cfg_t    *p_cfg);

// Validate image fragment
static int jpege_validate_image_fragment(
    jpeg_encoder_t  *p_encoder,
    jpege_img_frag_t fragment,
    uint32_t         fragment_id);
/* -----------------------------------------------------------------------
** Extern variables
** ----------------------------------------------------------------------- */
extern jpege_engine_lists_t jpege_engine_try_lists;

/* =======================================================================
**                          Function Definitions
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
    void                  *p_user_data)
{
    int rc;
    jpeg_encoder_t *p_encoder;

    // Input validation
    if (!p_handler)
        return JPEGERR_ENULLPTR;

    // Allocate for the Jpeg Encoder structure
    p_encoder = (jpeg_encoder_t *)JPEG_MALLOC(sizeof(jpeg_encoder_t));
    if (!p_encoder)
        return JPEGERR_EMALLOC;

    // Initialize all fields in the encoder structure
    STD_ZEROAT(p_encoder);

    // Store the event handler and user data
    p_encoder->p_event_handler = p_handler;
    p_encoder->p_user_data = p_user_data;

    // Initialize jpeg writer
    rc = jpegw_init(&(p_encoder->writer), (jpege_obj_t)p_encoder, jpeg_writer_output_handler);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_init: jpegw_init failed\n");
        JPEG_FREE(p_encoder);
        return rc;
    }
    // Initialize jpeg output queue
    rc = jpeg_queue_init(&(p_encoder->output_queue));
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_init: jpeg_queue_init failed\n");
        // Destroy the writer
        jpegw_destroy(&(p_encoder->writer));
        JPEG_FREE(p_encoder);
        return rc;
    }
    // Init buffer to hold the thumbnail data
    jpeg_buffer_init((jpeg_buffer_t *)&(p_encoder->p_thumbnail_buf));

    // Initialize thread condition variable and mutex
    (void)os_recursive_mutex_init(&(p_encoder->mutex));
    (void)os_cond_init(&(p_encoder->cond));

    // Cast the created encoder into the opaque object pointer
    *p_obj = (jpege_obj_t)p_encoder;
    return JPEGERR_SUCCESS;
}

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
    jpege_cfg_t  *p_config)
{
    if (!p_config)
        return JPEGERR_ENULLPTR;

    // Zero out all fields in the structure
    // Automatically take care of defaulting:
    // target_filesize: 0 (disabled)
    // header_type: EXIF
    // thumbnail_present: 0 (absent)
    // rotation in each image config: 0
    // restart interval in each image config: 0
    STD_ZEROAT(p_config);

    // Set default for main and thumbnail
    jpege_get_default_img_cfg(&(p_config->main_cfg));
    jpege_get_default_img_cfg(&(p_config->thumbnail_cfg));

    return JPEGERR_SUCCESS;
}

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
    jpege_cfg_t    *p_config)
{
    jpeg_encoder_t *p_encoder = (jpeg_encoder_t *)obj;
    if (!p_encoder)
        return JPEGERR_EUNINITIALIZED;

    if (!p_config)
        return JPEGERR_ENULLPTR;

    STD_ZEROAT(p_config);

    // copy actual config used by encoder
    *p_config = (*p_encoder).config;

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: jpege_set_source
 * Description: Assigns the image source to the Jpeg Encoder object.
 * Input parameters:
 *   obj       - The Jpeg Encoder object
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
    jpege_src_t  *p_source)
{
    int rc;
    jpeg_encoder_t *p_encoder = (jpeg_encoder_t *)obj;
    if (!p_encoder)
        return JPEGERR_EUNINITIALIZED;

    // Input validation
    if (!p_source || !p_source->p_main)
       return JPEGERR_ENULLPTR;

    // Validate main image data
    rc = jpege_validate_image_data(p_source->p_main);
    if (JPEG_FAILED(rc)) return rc;

    // Validate thumbnail image if present
    if (p_source->p_thumbnail)
    {
        rc = jpege_validate_image_data(p_source->p_thumbnail);
        if (JPEG_FAILED(rc)) return rc;
    }

    // If all validation passed, store the source pointer
    p_encoder->source = *p_source;
    return JPEGERR_SUCCESS;
}

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
    int32_t       mobicat_data_length)
{
    uint8_t *p_buffer;

    jpeg_encoder_t *p_encoder = (jpeg_encoder_t *)obj;
    if (!p_encoder)
        return JPEGERR_EUNINITIALIZED;

    JPEG_DBG_LOW("%s,%d\n",__func__,__LINE__);

    if (0==mobicat_data_length)
    {
        // if mobicat data length is 0, no data to set.
        // consider it done.
        p_encoder->mobicat_data.length = 0;
        p_encoder->mobicat_data.data = 0;
        JPEG_DBG_ERROR("%s,%d\n",__func__,__LINE__);
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
        JPEG_DBG_LOW("%s,%d\n",__func__,__LINE__);
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
    JPEG_DBG_LOW("%s,%d,p_encoder->mobicat_data.length: %dAddr is %d\n",__func__,__LINE__,
                   p_encoder->mobicat_data.length,( uint8_t *)p_encoder->mobicat_data.data);
    return JPEGERR_SUCCESS;
}


/******************************************************************************
 * Function: jpege_set_destination
 * Description: Assigns the destination to the Jpeg Encoder object.
 * Input parameters:
 *   obj       - The Jpeg Encoder object
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
    jpege_dst_t  *p_dest)
{
    uint8_t  i;
    jpeg_encoder_t *p_encoder = (jpeg_encoder_t *)obj;
    if (!p_encoder)
        return JPEGERR_EUNINITIALIZED;

    // Input validations
    if (!p_dest || !p_dest->p_output_handler)
        return JPEGERR_ENULLPTR;

    if (p_dest->buffer_cnt < 1)
    {
        JPEG_DBG_ERROR("jpege_set_destination: invalid buffer cnt\n");
        return JPEGERR_EBADPARM;
    }

    // Check enqueue output buffer pointer, size and offset
    for (i = 0; i < p_dest->buffer_cnt; i++)
    {
        jpeg_buf_t *p_buf = (jpeg_buf_t *)*(p_dest->p_buffer + i);
        if (!p_buf || !p_buf->ptr || !p_buf->size || p_buf->offset >= p_buf->size)
        {
            JPEG_DBG_ERROR("jpege_set_destination: invalid outpt buffer\n");
            return JPEGERR_EBADPARM;
        }
    }

    // Store the dest pointer
    p_encoder->dest = *p_dest;

    // Derive engine destination
    p_encoder->engine_dst.p_output_handler = &jpege_engine_output_handler;
    p_encoder->engine_dst.p_get_output_buffer_handler = &jpege_get_output_buffer;

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: jpege_start
 * Description: Starts the Jpeg Encoder. Encoding will start asynchronously
 *              in a new thread if the call returns JPEGERR_SUCCESS. The caller
 *              thread should interact asynchronously with the encoder through
 *              event and output handlers.
 * Input parameters:
 *   obj             - The Jpeg Encoder object.
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
int jpege_start(
    jpege_obj_t       obj,
    jpege_cfg_t      *p_cfg,
    exif_info_obj_t  *p_exif_info)
{
    int rc;
    jpeg_encoder_t *p_encoder = (jpeg_encoder_t *)obj;

    if (!p_encoder) {
        JPEG_DBG_ERROR("jpege_start:failed: JPEGERR_EUNINITIALIZED \n");
        return JPEGERR_EUNINITIALIZED;
    }
    // Check whether the Jpeg Encoder is IDLE before proceeding
    if (p_encoder->state != JPEGE_IDLE) {
        JPEG_DBG_ERROR("jpege_start:failed: JPEGERR_EBADSTATE\n");
        return JPEGERR_EBADSTATE;
    }

    // Reset output queue
    rc = jpeg_queue_reset(p_encoder->output_queue);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_start: failed to reset output queue\n");
        return rc;
    }

    // Save pre-calculated app2 data length
    p_encoder->config.app2_header_length = p_cfg->app2_header_length;

    // If main image has multiple fragments
    if (p_encoder->source.p_main->fragment_cnt > 1)
    {
        JPEG_DBG_ERROR("jpege_start: not supported fragment %d\n", p_encoder->source.p_main->fragment_cnt);
        return JPEGERR_EUNSUPPORTED;
    }

    // Calculate and Configure restart interval
    jpege_configure_restart(p_encoder, p_cfg);

    // Configure the Jpeg Writer
    jpegw_configure(&(p_encoder->writer),
                    &(p_encoder->source),
                    &(p_encoder->dest),
                    &(p_encoder->config),
                    &(p_encoder->mobicat_data),
                    p_exif_info);

    // Configure the encoder
    rc = jpege_configure(p_encoder, p_cfg);

    // Try to create and initialize available underlying engines
    os_mutex_lock(&(p_encoder->mutex));

    if (p_encoder->state == JPEGE_ABORTING)
    {
        p_encoder->state = JPEGE_IDLE;
        os_mutex_unlock(&(p_encoder->mutex));
        return JPEGERR_EFAILED;
    }
    if (p_encoder->p_thumbnail_buf)
    {
        rc = jpeg_buffer_reset((jpeg_buffer_t)p_encoder->p_thumbnail_buf);
    }

    // Pass the buffer address and flag status to the lower layers.
    p_encoder->engine.fastCV_buffer = p_cfg->fastCV_buffer;
    p_encoder->engine.fastCV_flag = p_cfg->fastCV_flag;

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
                                     EXIF_MAX_THUMBNAIL_LENGTH);
            if (JPEG_SUCCEEDED(rc))
            {
                    //Always set the thumbnail preference to be "S/W only" for the following reasons.
                    // 1. Thumbnails are smaller, Incase of H/W prefered encoding, the setup time cost
                    // for this images are way comparitevely higher to real encoding time.
                    // 2. Incase of H/W inline encoding, gemini or "inline h/w" is still owned by VFE during
                    // the full session of the encoding, and it is wise to perform the thumbnail in S/W to
                    // avoid conflict of resources.

                    //Allocate from normal pool (not pmem), by indicating false
                    rc = jpeg_buffer_allocate((jpeg_buffer_t)p_encoder->p_thumbnail_buf, thumbnail_size, false);
                    if (JPEG_SUCCEEDED(rc))
                    {
                        p_encoder->state = JPEGE_ENCODING_THUMBNAIL;

                        rc = jpege_try_engines(obj, JPEG_ENCODER_PREF_SOFTWARE_ONLY);
                        if (JPEG_SUCCEEDED(rc))
                        {
                            rc = p_encoder->engine.start(&(p_encoder->engine), &(p_encoder->config.thumbnail_cfg),
                                                         p_encoder->source.p_thumbnail, &(p_encoder->engine_dst));
                        }
                }
                }
            }
            os_mutex_unlock(&(p_encoder->mutex));
        }
        else
        {
            p_encoder->state = JPEGE_ENCODING_MAIN;

            rc = jpege_try_engines(obj, p_cfg->preference);
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
                // Start Encode Engine on main image
                if (JPEG_SUCCEEDED(rc))
                {
                    rc = p_encoder->engine.start(&(p_encoder->engine), &(p_encoder->config.main_cfg),
                                  p_encoder->source.p_main, &(p_encoder->engine_dst));
                }
            }
            os_mutex_unlock(&(p_encoder->mutex));
        }
    }
    if (JPEG_FAILED(rc))
    {
        os_mutex_lock(&(p_encoder->mutex));
        p_encoder->state = JPEGE_IDLE;
        os_mutex_unlock(&(p_encoder->mutex));
    }

    return rc;
}

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
int jpege_abort(jpege_obj_t obj)
{
    int rc = JPEGERR_SUCCESS;
    jpeg_encoder_t *p_encoder = (jpeg_encoder_t *)obj;
    if (!p_encoder)
        return JPEGERR_EUNINITIALIZED;

    os_mutex_lock(&(p_encoder->mutex));
    if (p_encoder->state != JPEGE_IDLE)
    {
        JPEG_DBG_LOW("jpege_abort: abort.\n");
        p_encoder->state = JPEGE_ABORTING;
        os_mutex_unlock(&(p_encoder->mutex));
        JPEG_DBG_LOW("jpege_abort: aborting output req queue.\n");
        rc = jpeg_queue_abort(&(p_encoder->output_queue));
        JPEG_DBG_LOW("jpege_abort: output req queue aborted.\n");
        if (JPEG_SUCCEEDED(rc))
        {
            JPEG_DBG_LOW("jpege_abort: aborting engine.\n");
            rc = p_encoder->engine.abort(&(p_encoder->engine));
            JPEG_DBG_LOW("jpege_abort: engine aborted.\n");
        }
    }
    else
        os_mutex_unlock(&(p_encoder->mutex));

    return rc;
}

/******************************************************************************
 * Function: jpege_destroy
 * Description: Destroys the Jpeg Encoder object, releasing all memory
 *              previously allocated.
 * Input parameters:
 *   p_obj       - The pointer to the Jpeg Encoder object
 * Return values: None
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
void jpege_destroy(jpege_obj_t *p_obj)
{
    if (p_obj)
    {
        jpeg_encoder_t *p_encoder = (jpeg_encoder_t *)(*p_obj);
        if (p_encoder)
        {
            JPEG_DBG_LOW("jpege_destroy\n");
            os_mutex_lock(&(p_encoder->mutex));

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
            jpegw_destroy(&(p_encoder->writer));
            // Destroy the output queue
            jpeg_queue_destroy(&(p_encoder->output_queue));

            // Destroy any scaled quant tables allocated
            JPEG_FREE(p_encoder->p_main_luma_qtable);
            JPEG_FREE(p_encoder->p_main_chroma_qtable);
            JPEG_FREE(p_encoder->p_tn_luma_qtable);
            JPEG_FREE(p_encoder->p_tn_chroma_qtable);
            JPEG_FREE(p_encoder->mobicat_data.data);

            os_mutex_unlock(&(p_encoder->mutex));
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

static void jpege_engine_event_handler(
    jpege_engine_obj_t *p_obj,  // The encode engine object calling it
    jpeg_event_t        event,  // The event code
    void               *p_arg)  // An optional argument for that event.
{
    int rc = JPEGERR_SUCCESS;
    jpeg_encoder_t *p_encoder;
    if (!p_obj)
    {
        JPEG_DBG_ERROR("jpege_engine_event_handler: p_obj NULL!\n");
        return;
    }
    p_encoder = (jpeg_encoder_t *)p_obj->encoder;

    // if Encoding Thumbnail done and thumbnail buffer is Full
    if (p_encoder->state == JPEGE_ENCODING_THUMBNAIL &&
        p_encoder->thumbnail_drop_flag)
    {
        // will drop the thumbnail when encode header,
        // then contiue to encode main image
        event = JPEG_EVENT_DONE;
    }

    // event handling will not be interruptable
    os_mutex_lock(&(p_encoder->mutex));
    if (p_encoder->state == JPEGE_ABORTING)
    {
        //p_encoder->state = JPEGE_IDLE;
        os_mutex_unlock(&(p_encoder->mutex));
        return;
    }

    switch (event)
    {
    case JPEG_EVENT_DONE:

        // Take action based on the state
        // Done with thumbnail
        if (p_encoder->state == JPEGE_ENCODING_THUMBNAIL)
        {
            // Enter critical section
            p_encoder->state = JPEGE_ENCODING_MAIN;
            // Leave critical section

            // Write file header
            if (!p_encoder->config.target_filesize)
            {
                rc = jpegw_emit_header(&(p_encoder->writer),p_encoder->p_thumbnail_buf);
            }

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
                rc = jpege_try_engines(p_obj->encoder, p_encoder->config.preference);

                if (JPEG_SUCCEEDED(rc))
                {
                    rc = p_encoder->engine.start(&(p_encoder->engine), &(p_encoder->config.main_cfg),
                              p_encoder->source.p_main, &(p_encoder->engine_dst));
                    JPEG_DBG_LOW("%s:Started main image encoding",__func__);
                }
            }
            if (JPEG_FAILED(rc))
            {
                // Change state to IDLE
                p_encoder->state = JPEGE_IDLE;
                p_encoder->p_event_handler(p_encoder->p_user_data, JPEG_EVENT_ERROR, NULL);
            }
        }
        // Done with main image
        else if (p_encoder->state == JPEGE_ENCODING_MAIN)
        {
            if (p_encoder->config.target_filesize)
            {
                rc = jpegfsc_encode_done_handler(&(p_encoder->file_size_control),
                                                 &(p_encoder->config),
                                                 &(p_encoder->p_main_luma_qtable),
                                                 &(p_encoder->p_main_chroma_qtable));
                if (JPEG_FAILED(rc))
                {
                    JPEG_DBG_HIGH("jpege_engine_event_handler: file size control failed\n");
                    p_encoder->p_event_handler(p_encoder->p_user_data, JPEG_EVENT_ERROR, NULL);
                    break;
                }
                // Check whether need to try another pass
                if (jpegfsc_is_succeed(p_encoder->file_size_control))
                {
                    // Restart encode engine
                    if (jpegfsc_is_state_output (p_encoder->file_size_control))
                    {
                        // If this pass succeed
                        // will follow the same routine without file size control
                        p_encoder->config.target_filesize = 0;
                        rc = jpegw_emit_header(&(p_encoder->writer), p_encoder->p_thumbnail_buf);
                    }
                    // Start engine
                    rc = p_encoder->engine.start(&(p_encoder->engine), &(p_encoder->config.main_cfg),
                                                 p_encoder->source.p_main, &(p_encoder->engine_dst));
                    if (JPEG_FAILED(rc))
                    {
                        JPEG_DBG_HIGH("jpege_engine_event_handler: encoder engine failed to start for a new pass of file size control\n");
                        p_encoder->p_event_handler(p_encoder->p_user_data, JPEG_EVENT_ERROR, NULL);
                        break;
                    }
                    break;
                }
                else
                {
                    // If Failed
                    JPEG_DBG_HIGH("jpege_engine_event_handler: file size control failed\n");
                    p_encoder->p_event_handler(p_encoder->p_user_data, JPEG_EVENT_ERROR, NULL);
                    break;
                }
            }
            // Change state to IDLE
            p_encoder->state = JPEGE_IDLE;
            // Leave critical section
            JPEG_DBG_HIGH("%s main image encoding done",__func__);
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

static int jpege_engine_output_handler(
    jpege_engine_obj_t  *p_obj,    // The encode engine object calling it
    jpeg_buf_t          *p_buf,    // The buffer containing the output bitstream_t
    uint8_t             last_buf_flag) // The flag to indicate last output buffer
{
    int rc = JPEGERR_SUCCESS;
    jpeg_encoder_t *p_encoder;
    if (!p_obj)
    {
        JPEG_DBG_ERROR("jpege_engine_output_handler: p_obj NULL!\n");
        return JPEGERR_EFAILED;
    }
    p_encoder = (jpeg_encoder_t *)p_obj->encoder;

    // Forward the output to file writer if the output is for thumbnail
    if (p_encoder->state != JPEGE_ENCODING_THUMBNAIL)
    {
        if (p_encoder->config.target_filesize)
        {
            jpegfsc_update_output_size(&(p_encoder->file_size_control), p_buf->offset);
            p_buf->offset = 0;
            // Encoding thread waits this output buffer to be marked empty,
            // then gets it and reuses it during file size control.
            jpeg_buffer_mark_empty(p_buf);
        }
        else
        {
            if (p_encoder->config.app2_header_length)
            {
                // Forward to upper layer with App2 header start offset
                rc = p_encoder->dest.p_output_handler(p_encoder->p_user_data,
                                                    &(p_encoder->writer.app2_start_offset),
                                                     (jpeg_buffer_t)p_buf,
                                                      last_buf_flag);
            }
            else
            {
            // Forward to upper layer
            rc = p_encoder->dest.p_output_handler(p_encoder->p_user_data,
                                                  p_encoder->dest.p_arg,
                                                  (jpeg_buffer_t)p_buf,
                                                  last_buf_flag);
            }
        }
        return rc;
    }
    return rc;
}

static int jpeg_writer_output_handler(
    jpege_obj_t          encoder,
    jpeg_buf_t          *p_buf)
{
    jpeg_encoder_t *p_encoder = (jpeg_encoder_t *)encoder;
    jpeg_buf_t *p_dest;
    uint32_t bytes_left = p_buf->offset;
    uint32_t bytes_written = 0;
    uint32_t bytes_to_copy;
    int      rc = JPEGERR_SUCCESS;

    // Check encoder pointer
    if (!p_encoder)
    {
        JPEG_DBG_ERROR("jpeg_writer_output_handler: failed with null encoder\n");
        return JPEGERR_ENULLPTR;
    }

    // Copy it to the destination buffer
    while (bytes_left)
    {
        // Get the next output buffer from buffer(s) queue
        rc = jpegw_get_output_buffer(p_encoder, &p_dest);
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jpeg_writer_output_handler: failed to get next output buffer\n");
            return rc;
        }
        if (!p_dest)
        {
            JPEG_DBG_ERROR("jpeg_writer_output_handler: failed to get next output buffer\n");
            return JPEGERR_EFAILED;
        }
        bytes_to_copy = STD_MIN(bytes_left, p_dest->size - p_dest->offset);

        STD_MEMMOVE(p_dest->ptr + p_dest->start_offset + p_dest->offset, p_buf->ptr + bytes_written, bytes_to_copy);
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

static int jpege_validate_image_fragment(
    jpeg_encoder_t  *p_encoder,
    jpege_img_frag_t fragment,
    uint32_t         fragment_id)
{
    // Validate dimension
    if (!fragment.width || !fragment.height)
    {
        JPEG_DBG_ERROR("jpege_validate_image_fragment: bad image dimension: (%dx%d)\n",
                        fragment.width, fragment.height);
        return JPEGERR_EBADPARM;
    }

    // Validate fragment width
    if (fragment.width != p_encoder->source.p_main->width)
    {
        // Support vertical only Fragment
        // Fragment width is the same as frame width
        JPEG_DBG_ERROR("jpege_validate_image_fragment: Fragment width %d NOT equal to Image width %d\n",
                     fragment.width, p_encoder->source.p_main->width);
        return JPEGERR_EUNSUPPORTED;
    }

    // Validate fragment id
    if (fragment_id < 1 || fragment_id >= p_encoder->source.p_main->fragment_cnt || fragment_id >= MAX_FRAGMENTS)
    {
        JPEG_DBG_ERROR("jpege_validate_image_fragment: unsupported fragment id %d\n",
                     fragment_id);
        return JPEGERR_EBADPARM;
    }

    if (p_encoder->source.p_main->color_format <= YCBCRLP_H1V1 )//|| p_encoder->source.p_main->color_format == YUY2 || p_encoder->source.p_main->color_format == UYVY )
    {
        if (!((jpeg_buf_t *)(fragment.color.yuv.luma_buf))->ptr ||
            !((jpeg_buf_t *)(fragment.color.yuv.chroma_buf))->ptr)
        {
            JPEG_DBG_ERROR("jpege_validate_image_fragment: NULL fragment (%d)\n", fragment_id);
        return JPEGERR_EBADPARM;

        }
    }
    else if ((p_encoder->source.p_main->color_format >= JPEG_BITSTREAM_H2V2) &&
             (p_encoder->source.p_main->color_format <= JPEG_BITSTREAM_H1V1))
    {
        if (!((jpeg_buf_t *)(fragment.color.bitstream.bitstream_buf))->ptr )
      {
            JPEG_DBG_ERROR("jpege_validate_image_fragment: NULL fragment (%d)\n", fragment_id);
          return JPEGERR_EBADPARM;
        }
      }

    return JPEGERR_SUCCESS;
}

static int jpege_validate_image_data(jpege_img_data_t *p_img)
{
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

    // Validate fragment cnt
    if (p_img->fragment_cnt < 1 || p_img->fragment_cnt > MAX_FRAGMENTS)
    {
        JPEG_DBG_ERROR("jpege_validate_image_fragment: unsupported fragment cnt %d\n",
                     p_img->fragment_cnt);
        return JPEGERR_EBADPARM;
    }
    // Validate fragment
    if (p_img->color_format <= YCBCRLP_H1V1 || p_img->color_format == YUY2 || p_img->color_format == UYVY )
    {
        if (!((jpeg_buf_t *)(p_img->p_fragments[0].color.yuv.luma_buf))->ptr ||
            !((jpeg_buf_t *)(p_img->p_fragments[0].color.yuv.chroma_buf))->ptr)
        {
            JPEG_DBG_ERROR("jpege_validate_image_data: broken fragment 0\n");
            return JPEGERR_EBADPARM;
        }
    }
    else if ((p_img->color_format >= JPEG_BITSTREAM_H2V2) &&
             (p_img->color_format <= JPEG_BITSTREAM_H1V1))
    {
        if (!((jpeg_buf_t *)(p_img->p_fragments[0].color.bitstream.bitstream_buf))->ptr )
        {
            JPEG_DBG_ERROR("jpege_validate_image_data: broken fragment 0\n");
            return JPEGERR_EBADPARM;
        }
    }

    // Validate dimension
    if (!p_img->height || !p_img->width)
    {
        JPEG_DBG_ERROR("jpege_validate_image_data: bad image dimension: (%dx%d)\n",
                     p_img->width, p_img->height);
        return JPEGERR_EBADPARM;
    }
    // Validate fragment width
    if (p_img->p_fragments[0].width != p_img->width)
    {
        // Support vertical only Fragment
        // Fragment width is the same as frame width
        JPEG_DBG_ERROR("jpege_validate_image_fragment: Fragment width %d NOT equal to Image width %d\n",
                   p_img->p_fragments[0].width, p_img->width);
        return JPEGERR_EUNSUPPORTED;
    }

    return JPEGERR_SUCCESS;
}

int jpege_validate_image_config(jpege_img_cfg_t *p_cfg)
{
    if (p_cfg->quality < 1 || p_cfg->quality > 100 ||         // Validate quality factor
        !p_cfg->luma_quant_tbl || !p_cfg->chroma_quant_tbl)   // Validate quant tables
    {
        return JPEGERR_EBADPARM;
    }
    return JPEGERR_SUCCESS;
}

static int jpege_configure(jpeg_encoder_t *p_encoder, jpege_cfg_t *p_cfg)
{
    int rc;
    uint32_t thumbnail_size = 0;

    if (!p_cfg)
        return JPEGERR_ENULLPTR;

    // Check header type
    if (p_cfg->header_type != OUTPUT_JFIF && p_cfg->header_type != OUTPUT_EXIF)
        return JPEGERR_EBADPARM;

    // Check preference
    if (p_cfg->preference >= JPEG_ENCODER_PREF_MAX)
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

    // Set target file size
    if (p_cfg->target_filesize)
    {
        // Initialize jpeg file size control
        rc = jpegfsc_init(&(p_encoder->file_size_control), *p_cfg);
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jpege_configure: file size control failed to enable\n");
            return rc;
        }
        JPEG_DBG_LOW("jpege_configure: file size control enabled\n");
    }

    // Make a copy of the config
    p_encoder->config = *p_cfg;

    // Check image specific configuration
    rc = jpege_validate_image_config(&(p_cfg->main_cfg)); // main
    if (JPEG_FAILED(rc)) return rc;

    /*------------------------------------------------------------
      In case of file size control
      1. Set the Max Quality
      2. Calculate target quality and scaling factor
         based on the given image
      3. Init the Quant tables
    ------------------------------------------------------------*/
    if (p_cfg->target_filesize)
    {
        // Set the Max Quality
        rc = jpegfsc_set_max_quality (&(p_encoder->file_size_control),
                                      p_cfg->main_cfg.quality);
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jpege_configure: file size control failed to set max quality\n");
            return rc;
        }
        // Estimate quality for a given image resolution and
        // target file size
        if (p_cfg->thumbnail_present)
        {
            thumbnail_size = p_encoder->source.p_thumbnail->width *
                                              p_encoder->source.p_thumbnail->height;
        }
        rc = jpegfsc_estimate_target_quality (&(p_encoder->file_size_control),
                                              p_encoder->source.p_main->width,
                                              p_encoder->source.p_main->height,
                                              thumbnail_size,
                                              (jpeg_subsampling_t)p_encoder->source.p_main->color_format);
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jpege_configure: file size control failed to estimate quality\n");
            return rc;
        }
        // Init main quantization table according to the quality factor
        rc = jpegfsc_init_quant_tables (p_encoder->file_size_control,
                                       &(p_encoder->config.main_cfg),
                                       &(p_encoder->p_main_luma_qtable),
                                       p_cfg->main_cfg.luma_quant_tbl,
                                       &(p_encoder->p_main_chroma_qtable),
                                       p_cfg->main_cfg.chroma_quant_tbl);
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jpege_configure: file size control failed to init main quant tables\n");
            return rc;
        }
    }
    else
    {
        // Scale quant tables
        p_encoder->config.main_cfg.luma_quant_tbl = jpege_scale_quant_table(&(p_encoder->p_main_luma_qtable),
                                                                            p_cfg->main_cfg.luma_quant_tbl,
                                                                            p_cfg->main_cfg.quality);
        p_encoder->config.main_cfg.chroma_quant_tbl = jpege_scale_quant_table(&(p_encoder->p_main_chroma_qtable),
                                                                              p_cfg->main_cfg.chroma_quant_tbl,
                                                                              p_cfg->main_cfg.quality);
    }

    if (!p_encoder->config.main_cfg.luma_quant_tbl || !p_encoder->config.main_cfg.chroma_quant_tbl)
    {
        return JPEGERR_EMALLOC;
    }

    if (p_cfg->thumbnail_present)
    {
        // Apply same check and configuration to thumbnail
        rc = jpege_validate_image_config(&(p_cfg->thumbnail_cfg));
        if (JPEG_FAILED(rc)) return rc;

        if (p_cfg->target_filesize)
        {
            // Init thumbnail quantization table according to the quality factor
            rc = jpegfsc_init_quant_tables (p_encoder->file_size_control,
                                           &(p_encoder->config.thumbnail_cfg),
                                           &(p_encoder->p_tn_luma_qtable),
                                           p_cfg->thumbnail_cfg.luma_quant_tbl,
                                           &(p_encoder->p_tn_chroma_qtable),
                                           p_cfg->thumbnail_cfg.chroma_quant_tbl);
            if (JPEG_FAILED(rc))
            {
                JPEG_DBG_ERROR("jpege_configure: file size control failed to init thumbnail quant tables\n");
                return rc;
            }
        }
        else
        {
            // Scale quant tables
            p_encoder->config.thumbnail_cfg.luma_quant_tbl = jpege_scale_quant_table(&(p_encoder->p_tn_luma_qtable),
                                                                                     p_cfg->thumbnail_cfg.luma_quant_tbl,
                                                                                     p_cfg->thumbnail_cfg.quality);
            p_encoder->config.thumbnail_cfg.chroma_quant_tbl = jpege_scale_quant_table(&(p_encoder->p_tn_chroma_qtable),
                                                                                       p_cfg->thumbnail_cfg.chroma_quant_tbl,
                                                                                       p_cfg->thumbnail_cfg.quality);
        }
        if (!p_encoder->config.thumbnail_cfg.luma_quant_tbl || !p_encoder->config.thumbnail_cfg.chroma_quant_tbl)
        {
            return JPEGERR_EMALLOC;
        }
    }
    return JPEGERR_SUCCESS;
}

static void jpege_get_default_img_cfg(jpege_img_cfg_t *p_img_cfg)
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
static jpeg_quant_table_t jpege_scale_quant_table(uint16_t **dst_quant_tbl,
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

static int jpege_check_start_param(jpeg_encoder_t *p_encoder)
{
    int rc = JPEGERR_EFAILED;

    // Check to see if the engine supports the requested encoding
    // Temporary workaround - p_encoder->engine_dst.transposed is implicitly
    // cleared by BS engine which is the first one in engine list
    if (p_encoder->state == JPEGE_ENCODING_MAIN)
    {
        rc = p_encoder->engine.check_start_param(&(p_encoder->config.main_cfg),
                                                 p_encoder->source.p_main,
                                                 &(p_encoder->engine_dst));
    }
    else if (p_encoder->state == JPEGE_ENCODING_THUMBNAIL)
    {
        // Check thumbnail configuration also if necessary
        if (p_encoder->config.thumbnail_present)
        {
            rc = p_encoder->engine.check_start_param(&(p_encoder->config.thumbnail_cfg),
                                                     p_encoder->source.p_thumbnail,
                                                     &(p_encoder->engine_dst));
        }
    }

    return rc;
}

static int jpege_try_engines(jpege_obj_t obj, jpege_preference_t pref)
{
    jpeg_encoder_t *p_encoder = (jpeg_encoder_t *)obj;
    jpege_engine_profile_t** profile_list = jpege_engine_try_lists[pref];

    // Go through the engine try list according to the pref
    while (*profile_list)
    {
        int rc = JPEGERR_SUCCESS;
        jpege_engine_profile_t *engine_profile = *profile_list;

        // Check if the same engine is previously created and init-ed
/*        if (p_encoder->engine.create == engine_profile->create_func &&
            p_encoder->engine.is_initialized)
        {
            rc = jpege_check_start_param(p_encoder);
            if (JPEG_SUCCEEDED(rc))
            {
                JPEG_DBG_MED("jpege_try_engines: engine %s already in use\n", engine_profile->engine_name);
                return rc;
            }
        }
*/
        // Create the engine
        engine_profile->create_func(&(p_encoder->engine), obj);

        rc = jpege_check_start_param(p_encoder);
        if (JPEG_SUCCEEDED(rc))
        {
            // Initialize engine
            rc = p_encoder->engine.init(&(p_encoder->engine), &jpege_engine_event_handler);
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

// Configure restart marker for each MCU row
static int jpege_configure_restart(jpeg_encoder_t *p_encoder, jpege_cfg_t  *p_cfg)
{
    jpeg_subsampling_t main_subsampling, tn_subsampling;
    uint8_t            mcu_width, mcu_height;
    uint32_t           luma_width, luma_height;

    if (!p_encoder || !p_cfg)
    {
        JPEG_DBG_ERROR("jpege_configure_restart: NULL pointer\n");
        return JPEGERR_ENULLPTR;
    }

    main_subsampling = (jpeg_subsampling_t)((uint8_t)p_encoder->source.p_main->color_format / 2);

    // main image
    switch (main_subsampling)
    {
    case JPEG_H1V1:
    {
        mcu_width = 8;
        mcu_height = 8;
        break;
    }
    case JPEG_H1V2:
    {
        mcu_width = 8;
        mcu_height = 16;
        break;
    }
    case JPEG_H2V1:
    case JPEG_YUY2:
    case JPEG_UYVY:
    {
        mcu_width = 16;
        mcu_height = 8;
        break;
    }
    case JPEG_H2V2:
    {
        mcu_width = 16;
        mcu_height = 16;
        break;
    }
    default:
        JPEG_DBG_ERROR("jpege_configure_restart: invalid jpeg subsampling: %d\n", main_subsampling);
        return JPEGERR_EBADPARM;
    }

    // Check on scaling
    if (p_cfg->main_cfg.scale_cfg.enable)
    {
        luma_width = p_cfg->main_cfg.scale_cfg.output_width;
        luma_height = p_cfg->main_cfg.scale_cfg.output_height;
    }
    else
    {
        luma_width = p_encoder->source.p_main->width;
        luma_height = p_encoder->source.p_main->height;
    }

    // Calculate restart interval for Main Image
    switch (p_cfg->main_cfg.rotation_degree_clk)
    {
    case 0:
    case 180:
        p_cfg->main_cfg.restart_interval = (luma_width + mcu_width - 1) / mcu_width;
        break;

    case 90:
    case 270:
        p_cfg->main_cfg.restart_interval = (luma_height + mcu_height - 1) / mcu_height;
        break;

    default:
        JPEG_DBG_ERROR("jpege_configure_restart: invalid jpeg rotation: %d\n", p_cfg->main_cfg.rotation_degree_clk);
        return JPEGERR_EBADPARM;;
    }

    JPEG_DBG_LOW("jpeg configured restart interval of main %d\n", p_cfg->main_cfg.restart_interval);

    if (p_cfg->thumbnail_present)
    {

    tn_subsampling   = (jpeg_subsampling_t)((uint8_t)p_encoder->source.p_thumbnail->color_format / 2);

    // Thumbnail
    switch (tn_subsampling)
    {
    case JPEG_H1V1:
    {
        mcu_width = 8;
        mcu_height = 8;
        break;
    }
    case JPEG_H1V2:
    {
        mcu_width = 8;
        mcu_height = 16;
        break;
    }
    case JPEG_H2V1:
    case JPEG_YUY2:
    case JPEG_UYVY:
    {
        mcu_width = 16;
        mcu_height = 8;
        break;
    }
    case JPEG_H2V2:
    {
        mcu_width = 16;
        mcu_height = 16;
        break;
    }
    default:
        JPEG_DBG_ERROR("jpege_configure_restart: invalid jpeg subsampling: %d\n", tn_subsampling);
        return JPEGERR_EBADPARM;
    }

    // Check on scaling
    if (p_cfg->thumbnail_cfg.scale_cfg.enable)
    {
        luma_width = p_cfg->thumbnail_cfg.scale_cfg.output_width;
        luma_height = p_cfg->thumbnail_cfg.scale_cfg.output_height;
    }
    else
    {
        luma_width = p_encoder->source.p_thumbnail->width;
        luma_height = p_encoder->source.p_thumbnail->height;
    }

    switch (p_cfg->thumbnail_cfg.rotation_degree_clk)
    {
    case 0:
    case 180:
        p_cfg->thumbnail_cfg.restart_interval = (luma_width + mcu_width - 1) / mcu_width;
        break;

    case 90:
    case 270:
        p_cfg->thumbnail_cfg.restart_interval = (luma_height + mcu_height - 1) / mcu_height;
        break;

    default:
        JPEG_DBG_ERROR("jpege_configure_restart: invalid jpeg rotation: %d\n", p_cfg->thumbnail_cfg.rotation_degree_clk);
        return JPEGERR_EBADPARM;;
    }

    JPEG_DBG_LOW("jpeg configured restart interval of thumbnail %d\n", p_cfg->thumbnail_cfg.restart_interval);
    }
    return JPEGERR_SUCCESS;
}

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
int jpege_enqueue_output_buffer(
    jpege_obj_t     obj,
    jpeg_buffer_t   *p_enqueue_buf,
    uint32_t        enqueue_buf_cnt)
{
    int  rc;
    uint8_t i;
    jpeg_encoder_t *p_encoder = (jpeg_encoder_t *)obj;

    // Check input parameter
    if ((!p_encoder) ||
        (!p_enqueue_buf))
    {
        JPEG_DBG_ERROR("jpege_enqueue_output_buffer: failed with null input parameter\n");
        return JPEGERR_ENULLPTR;
    }

    // Check enqueue output buffer pointer, size and offset
    for (i = 0; i < enqueue_buf_cnt; i++)
    {
        jpeg_buf_t *p_buf = (jpeg_buf_t *)*(p_enqueue_buf + i);
        if (!p_buf || !p_buf->ptr || !p_buf->size || p_buf->offset >= p_buf->size)
        {
            JPEG_DBG_ERROR("jpege_enqueue_output_buffer: invalid outpt buffer\n");
            return JPEGERR_EBADPARM;
        }
    }
    // Enqueue the output buffer(s) to queue
    rc = jpeg_queue_enqueue(p_encoder->output_queue, (void **)(p_enqueue_buf), enqueue_buf_cnt);
    return rc;
}

/******************************************************************************
* Function: jpege_get_output_buffer
* Description: Get a output buffer from the buffer(s) queue.
*              It is called by jpeg encoder engine.
*              It returns the pointer to the thumbnail buffer of jpeg writer if
*              encoding thumbnail.
*              It reuse one output buffer if encoding
*              during file size control procedure.
*              It returns the pointer to a new output buffer if
*              encoding main image.
* Input parameters:
*   p_obj              - The pointer to Jpeg Encoder object.
*   pp_dequeue_buffer  - The pointer to dequeued buffer.
* Return values:
*   - JPEGERR_SUCCESS
*     JPEGERR_EFAILED
*     JPEGERR_ENULLPTR
*     JPEGERR_ETIMEDOUT
*     JPEGERR_THUMBNAIL_DROPPED
* Notes: none
*****************************************************************************/
static int  jpege_get_output_buffer(jpege_engine_obj_t  *p_obj,
                                    jpeg_buf_t          **pp_dequeue_buffer )
{
    jpeg_encoder_t  *p_encoder = (jpeg_encoder_t *)p_obj->encoder;
    int  rc;

    // Check encoder pointer
    if (!p_encoder)
    {
        JPEG_DBG_ERROR("jpege_get_output_buffer: failed with null encoder\n");
        return JPEGERR_ENULLPTR;
    }

    // Returns the pointer to the thumbnail buffer of jpeg writer
    // if encoding thumbnail.
    if (p_encoder->state == JPEGE_ENCODING_THUMBNAIL)
    {
        if (!p_encoder->p_thumbnail_buf)
        {
            JPEG_DBG_ERROR("jpege_get_output_buffer: failed with null thumb buffer\n");
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
        // Check if encode for file size control
        if (p_encoder->config.target_filesize)
        {
            // Wait until size control to update current output file size
            // because encode thread has to wait output thread to complete output buffer
            jpeg_buffer_wait_until_empty(p_encoder->p_output_buffer_ref);
        }
        else
        {
            // Pass the output pointer to engine,
            // it continues the encoding of main image
            p_encoder->p_output_buffer_ref = NULL;
        }
        return  JPEGERR_SUCCESS;
    }

    // Dequeue a buffer from the output buffer queue
    os_mutex_lock(&(p_encoder->mutex));
    if (p_encoder->state == JPEGE_ABORTING)
    {
        JPEG_DBG_ERROR("jpege_get_output_buffere: encoder aborting.\n");
        os_mutex_unlock(&(p_encoder->mutex));
        return JPEGERR_EFAILED;
    }
    os_mutex_unlock(&(p_encoder->mutex));

    rc = jpeg_queue_dequeue(p_encoder->output_queue, (void **)pp_dequeue_buffer, TIME_OUT_MS);
    if JPEG_FAILED(rc)
    {
        JPEG_DBG_ERROR("jpege_get_output_buffere: failed with dequeue buffer\n");
        return rc;
    }
    if (p_encoder->config.target_filesize)
    {
        // Keep a reference to the output buffer pointer
        // and will reuse it during file size control process
        p_encoder->p_output_buffer_ref = *pp_dequeue_buffer;
    }
    return  JPEGERR_SUCCESS;
}

/******************************************************************************
* Function: jpegw_get_output_buffer
* Description: Get a output buffer from the buffer(s) queue
*              when encoding header of main image.
*              It is called by jpeg writer,
*              and it returns the pointer to a new output buffer.
*              And reuse if that buffer is NOT full.
* Input parameters:
*   p_obj               - The pointer to Jpeg Encoder object.
*   pp_dequeue_buffer   - The pointer to dequeued buffer.
* Return values:
*   - JPEGERR_SUCCESS
*     JPEGERR_EFAILED
*     JPEGERR_ENULLPTR
*     JPEGERR_ETIMEDOUT
* Notes: none
*****************************************************************************/
static int  jpegw_get_output_buffer(jpeg_encoder_t  *p_encoder,
                                    jpeg_buf_t      **pp_dequeue_buffer)
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
    if (p_encoder->state == JPEGE_ABORTING)
    {
        JPEG_DBG_ERROR("jpegw_get_output_buffere: encoder aborted.\n");
        os_mutex_unlock(&(p_encoder->mutex));
        return JPEGERR_EFAILED;
    }
    os_mutex_unlock(&(p_encoder->mutex));

    rc = jpeg_queue_dequeue(p_encoder->output_queue, (void **)pp_dequeue_buffer, TIME_OUT_MS);
    if JPEG_FAILED(rc)
    {
        JPEG_DBG_ERROR("jpege_get_output_buffere: failed with dequeue buffer\n");
        return rc;
    }
    p_encoder->p_output_buffer_ref = *pp_dequeue_buffer;
    return  JPEGERR_SUCCESS;
}

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
)
{
    int rc = JPEGERR_SUCCESS;
    jpeg_encoder_t *p_encoder = (jpeg_encoder_t *)obj;

    // Check input parameter
    if (!p_encoder)
    {
        JPEG_DBG_ERROR("jpege_deliver_image_fragment: failed with null encoder pointer\n");
        return JPEGERR_ENULLPTR;
    }

    // deliver image fragment will not be interruptable
    os_mutex_lock(&(p_encoder->mutex));

    rc = jpege_validate_image_fragment(p_encoder, fragment, fragment_id);

    if (JPEG_SUCCEEDED(rc))
    {
         p_encoder->source.p_main->p_fragments[fragment_id] = fragment;
         JPEG_DBG_LOW("jpege_deliver_image_fragment: fragment delivered.\n");
    }

    os_mutex_unlock(&(p_encoder->mutex));

    return rc;
}
