/*========================================================================


*//** @file mpoe.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (c) 2011-2013 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when         who     what, where, why
--------     ---     -------------------------------------------------------
01/10/11   staceyw     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpeg_common_private.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include "jpeg_buffer_private.h"
#include "mpo_private.h"
#include "mpoe.h"
#include "mpo_writer.h"
#include "jpege.h"

#include "os_thread.h"
#include "os_timer.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef enum
{
    MPOE_IDLE = 0,
    MPOE_ENCODING_THUMBNAIL,
    MPOE_ENCODING_MAIN,
    MPOE_ABORTING,

} mpoe_state_t;

typedef struct mpo_encoder_t
{
    // Exif info array
    exif_info_obj_t        exif_info[MAX_IMAGES];
    // The Mpo Writer
    mpo_writer_t           mpo_writer;
    // The Jpeg Encode
    jpege_obj_t            encoder;
    // The Mpo event handler
    mpoe_event_handler_t   p_event_handler;
    // The Mpo encode source array
    mpoe_src_t             source[MAX_IMAGES];
    // The Mpo encode source count
    uint32_t               source_cnt;
    // The Mpo encode destination
    mpoe_dst_t             dest;
    // The Mpo encode configuration array
    mpoe_cfg_t             config[MAX_IMAGES];
    // The MPO encode info
    mpo_info_obj_t         mpo_info;
    // Pointer to user data
    void                  *p_user_data;
    // Frame encoded count
    uint32_t               frame_encoded_count;
    // The Mpo encode engine destination
    jpege_dst_t            jpege_dst;
    // The state of the encoder
    mpoe_state_t           state;
    // os mutex
    os_mutex_t             mutex;
    // os condition variable
    os_cond_t              cond;
#ifdef _DEBUG
    os_timer_t             timer;
#endif

} mpo_encoder_t;


/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations of helper functions
** ----------------------------------------------------------------------- */
static int mpoe_configure(mpo_encoder_t *p_encoder, mpoe_cfg_t *p_cfg);
static int mpoe_validate_image_data(mpoe_img_data_t *p_img);
static void mpoe_get_default_img_cfg(mpoe_img_cfg_t *p_img_cfg);

// mpo encoder event handler
static void mpoe_event_handler(
    void         *p_user_data,
    jpeg_event_t  event,
    void         *p_arg);

// mpo encoder output handler
static int jpege_output_handler(
    mpoe_obj_t  *p_obj,    // The encode engine object calling it
    void        *p_arg,
    jpeg_buf_t  *p_buf,    // The buffer containing the output bitstream_t
    uint8_t      last_buf_flag); // The flag to indicate last output buffer

/* -----------------------------------------------------------------------
** Extern variables
** ----------------------------------------------------------------------- */

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
    void                  *p_user_data)
{
    int rc;
    mpo_encoder_t *p_encoder;
    jpege_obj_t    jpeg_encoder;

    // Input validation
    if (!p_handler)
        return JPEGERR_ENULLPTR;

    // Allocate for the Mpo Encoder structure
    p_encoder = (mpo_encoder_t *)JPEG_MALLOC(sizeof(mpo_encoder_t));
    if (!p_encoder)
        return JPEGERR_EMALLOC;

    // Initialize all fields in the encoder structure
    STD_MEMSET(p_encoder, 0, sizeof(mpo_encoder_t));

    // Store the event handler and user data
    p_encoder->p_event_handler = p_handler;
    p_encoder->p_user_data = p_user_data;

	// Initialize mpo writer
    rc = mpow_init(&(p_encoder->mpo_writer));
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("mpoe_init: mpow_init failed\n");
        JPEG_FREE(p_encoder);
        return rc;
    }

    // Initialize jpeg encoder
    rc = jpege_init(&jpeg_encoder, &mpoe_event_handler, (void *)p_encoder);
    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "mpoe_init: jpege_init failed\n");
        return OS_THREAD_FUNC_RET_FAILED;
    }
    p_encoder->encoder = jpeg_encoder;

    // Initialize thread condition variable and mutex
    (void)os_mutex_init(&(p_encoder->mutex));
    (void)os_cond_init(&(p_encoder->cond));

    // Cast the created encoder into the opaque object pointer
    *p_obj = (mpoe_obj_t)p_encoder;
    return JPEGERR_SUCCESS;
}

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
    mpoe_cfg_t  *p_config)
{
    jpege_cfg_t  jpege_config;

    // Call jpeg encoder to get default confg:
    // like default huff tables and quant tables.
    int rc = jpege_get_default_config(&jpege_config);

    if (JPEG_FAILED(rc))
    {
        fprintf(stderr, "mpoe_get_default_config: jpege_get_default_config failed\n");
        return rc;
    }

    if (!p_config)
        return JPEGERR_ENULLPTR;

    // Zero out all fields in the structure
    STD_MEMSET(p_config, 0, sizeof(mpoe_cfg_t));

    // Set Default config of mpo encoder
    STD_MEMMOVE(p_config, &jpege_config, sizeof(jpege_cfg_t));

    return JPEGERR_SUCCESS;
}

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
    uint32_t       src_id)
{
    mpo_encoder_t *p_encoder = (mpo_encoder_t *)obj;

    if (!p_encoder)
        return JPEGERR_EUNINITIALIZED;

    if (!p_config)
        return JPEGERR_ENULLPTR;

    if (src_id >= MAX_IMAGES ||
        src_id >= p_encoder->source_cnt)
   {
        JPEG_DBG_ERROR("mpoe_get_actual_config: invalid sources id %d\n", src_id);
        return JPEGERR_EBADPARM;
   }

    *p_config = (*p_encoder).config[src_id];

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: mpoe_set_source
 * Description: Assigns the image source to the Mpo Encoder object.
 * Input parameters:
 *   obj       - The Mpo Encoder object
 *   p_source  - The pointer to the source array to be assigned to the Mpo
 *               Encoder.
 *   source_cnt- The source count.
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
    uint32_t      source_cnt)
{
    uint32_t i;
    int rc;
    mpo_encoder_t *p_encoder = (mpo_encoder_t *)obj;

    if (!p_encoder)
        return JPEGERR_EUNINITIALIZED;

    // Validate source count
    if (source_cnt > MAX_IMAGES)
    {
        JPEG_DBG_ERROR("mpoe_set_source: invalid source count %d\n", source_cnt);
        return JPEGERR_EBADPARM;
    }

    // Input validation
    for (i = 0; i < source_cnt; i++)
    {
        if (!(p_source + i))
        {
            return JPEGERR_ENULLPTR;
        }
        p_encoder->source[i] = *(p_source + i);

        // Validate main image data
        rc = mpoe_validate_image_data(p_encoder->source[i].p_main);
        if (JPEG_FAILED(rc)) return rc;

        // Validate thumbnail image if present
        if (p_encoder->source[i].p_thumbnail)
        {
           rc = mpoe_validate_image_data(p_encoder->source[i].p_thumbnail);
           if (JPEG_FAILED(rc)) return rc;
        }

        // Validate mpo type
        if (p_encoder->source[i].image_attribute.type != MULTI_VIEW_DISPARITY ||
            p_encoder->source[i].image_attribute.format != JPEG)
        {
            JPEG_DBG_ERROR("mpoe_set_source: MPO attribute not supported: %d %d \n",
                           p_encoder->source[i].image_attribute.type, p_encoder->source[i].image_attribute.format);
            return JPEGERR_EUNSUPPORTED;
        }
    }

    p_encoder->source_cnt = source_cnt;

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: mpoe_set_destination
 * Description: Assigns the destination to the Mpo Encoder object.
 * Input parameters:
 *   obj       - The Mpo Encoder object
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
    mpoe_obj_t   obj,
    mpoe_dst_t  *p_dest)
{
    uint8_t  i;
    mpo_encoder_t *p_encoder = (mpo_encoder_t *)obj;
	jpege_dst_t   jpege_dst;

    if (!p_encoder)
        return JPEGERR_EUNINITIALIZED;

    // Input validations
    if (!p_dest || !p_dest->p_output_handler)
        return JPEGERR_ENULLPTR;

    if (p_dest->buffer_cnt < 1)
    {
        JPEG_DBG_ERROR("mpoe_set_destination: invalid buffer cnt\n");
        return JPEGERR_EBADPARM;
    }

    // Check enqueue output buffer pointer, size and offset
    for (i = 0; i < p_dest->buffer_cnt; i++)
    {
        jpeg_buf_t *p_buf = (jpeg_buf_t *)*(p_dest->p_buffer + i);
        if (!p_buf || !p_buf->ptr || !p_buf->size || p_buf->offset >= p_buf->size)
        {
            JPEG_DBG_ERROR("mpoe_set_destination: invalid outpt buffer\n");
            return JPEGERR_EBADPARM;
        }
    }

    // Pass the dest pointer to jpeg encoder
    STD_MEMMOVE(&jpege_dst, p_dest, sizeof(mpoe_dst_t));
    jpege_dst.p_output_handler = &jpege_output_handler;
    jpege_set_destination(p_encoder->encoder, &jpege_dst);

    // Store the dest pointer
    p_encoder->dest = *p_dest;

    // Pass output handler
    p_encoder->jpege_dst.p_output_handler = p_dest->p_output_handler;

    return JPEGERR_SUCCESS;
}

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
 *   p_mpo_info_obj  - The pointer to the mpo info.
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
    mpo_info_obj_t   *p_mpo_info_obj,
    uint32_t          src_cnt)
{
    uint32_t  i;
    int rc;
    mpo_encoder_t *p_encoder = (mpo_encoder_t *)obj;
    jpege_cfg_t   jpege_cfg;
    jpege_src_t   jpege_src;

    if (!p_encoder ||
        !p_cfg     ||
        !p_mpo_info_obj)
    {
        return JPEGERR_EUNINITIALIZED;
    }

    // Check whether the Mpo Encoder is IDLE before proceeding
    if (p_encoder->state != MPOE_IDLE)
    {
        return JPEGERR_EBADSTATE;
    }

    if (src_cnt != p_encoder->source_cnt)
    {
        JPEG_DBG_ERROR("mpoe_start: invalid source count\n");
        return JPEGERR_EBADPARM;
    }

    // Set mpo info
    p_encoder->mpo_info = *p_mpo_info_obj;

    // Configure the Mpo Writer for first image
    rc = mpow_configure_first(&(p_encoder->mpo_writer),
                                p_encoder->source,
                              &(p_encoder->dest),
                              &(p_encoder->mpo_info),
                                p_encoder->source_cnt);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("mpoe_start: failed to configure mpo writer for first image\n");
        return rc;
    }

    // Configure the encoder
    rc = mpoe_configure(p_encoder, p_cfg);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("mpoe_start: failed to configure encoder\n");
        return rc;
    }

    // Set exif info array
    for (i = 0; i < src_cnt; i++)
    {
        if (p_exif_info + i)
        {
            p_encoder->exif_info[i] = *(p_exif_info + i);
        }
    }

    // Try to create and initialize available underlying engines
    os_mutex_lock(&(p_encoder->mutex));

    if (p_encoder->state == MPOE_ABORTING)
    {
        p_encoder->state = MPOE_IDLE;
        os_mutex_unlock(&(p_encoder->mutex));
        return JPEGERR_EFAILED;
    }
    // Set the jpeg encoder source
    STD_MEMMOVE(&jpege_src, &p_encoder->source[0], sizeof(jpege_src_t));
    jpege_set_source(p_encoder->encoder, &jpege_src);

    // Set the jpeg encoder configuration
    STD_MEMMOVE(&jpege_cfg, p_cfg, sizeof(jpege_cfg_t));
    jpege_cfg.app2_header_length = p_encoder->mpo_writer.app2_header_length;

    // Start encoding for first image
    rc = jpege_start(p_encoder->encoder, &jpege_cfg, p_exif_info);

    os_mutex_unlock(&(p_encoder->mutex));

    if (JPEG_FAILED(rc))
    {
        os_mutex_lock(&(p_encoder->mutex));
        p_encoder->state = MPOE_IDLE;
        os_mutex_unlock(&(p_encoder->mutex));
    }

    return rc;
}

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
int mpoe_abort(mpoe_obj_t obj)
{
    int rc = JPEGERR_SUCCESS;
    mpo_encoder_t *p_encoder = (mpo_encoder_t *)obj;
    if (!p_encoder)
        return JPEGERR_EUNINITIALIZED;

    os_mutex_lock(&(p_encoder->mutex));
    if (p_encoder->state != MPOE_IDLE)
    {
        JPEG_DBG_LOW("mpoe_abort: abort.\n");
        p_encoder->state = MPOE_ABORTING;
        os_mutex_unlock(&(p_encoder->mutex));
        JPEG_DBG_LOW("mpoe_abort: aborting output req queue.\n");
        if (JPEG_SUCCEEDED(rc))
        {
            JPEG_DBG_LOW("mpoe_abort: aborting engine.\n");
            rc = jpege_abort(p_encoder->encoder);
            JPEG_DBG_LOW("mpoe_abort: engine aborted.\n");
        }
    }
    else
        os_mutex_unlock(&(p_encoder->mutex));

    return rc;
}

/******************************************************************************
 * Function: mpoe_destroy
 * Description: Destroys the Mpo Encoder object, releasing all memory
 *              previously allocated.
 * Input parameters:
 *   p_obj       - The pointer to the Mpo Encoder object
 * Return values: None
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int mpoe_destroy(mpoe_obj_t *p_obj)
{
	int rc = JPEGERR_SUCCESS;

    if (p_obj)
    {
        mpo_encoder_t *p_encoder = (mpo_encoder_t *)(*p_obj);
        if (p_encoder)
        {
            JPEG_DBG_LOW("mpoe_destroy\n");

            // Destroy the mpo writer
            rc =  mpow_destroy(&(p_encoder->mpo_writer));

			if (JPEG_FAILED(rc))
            {
               JPEG_DBG_ERROR("mpoe_destroy: failed to destroy MPO writer\n");
               return rc;
            }

            // Destroy the jpeg encoder
            jpege_destroy(&(p_encoder->encoder));

            // Destroy thread condition variable and mutex
            (void)os_mutex_destroy(&(p_encoder->mutex));
            (void)os_cond_destroy(&(p_encoder->cond));

            // Release memory of encoder
            JPEG_FREE(p_encoder);
            *p_obj = NULL;
        }
        jpeg_show_leak();
    }

	return rc;
}

static void mpoe_event_handler(
    void               *p_obj,  // The encode engine object calling it
    jpeg_event_t        event,  // The event code
    void               *p_arg)  // An optional argument for that event.
{
    int rc = JPEGERR_SUCCESS;
    mpo_encoder_t *p_encoder;
    jpege_cfg_t   jpege_cfg;
    jpege_src_t   jpege_src;
    uint32_t      src_id;
    if (!p_obj)
    {
        JPEG_DBG_ERROR("mpoe_event_handler: p_obj NULL!\n");
        return;
    }
    p_encoder = (mpo_encoder_t *)p_obj;

    // event handling will not be interruptable
    os_mutex_lock(&(p_encoder->mutex));
    if (p_encoder->state == MPOE_ABORTING)
    {
        p_encoder->state = MPOE_IDLE;
        os_mutex_unlock(&(p_encoder->mutex));
        return;
    }

    switch (event)
    {
    case JPEG_EVENT_DONE:

        p_encoder->frame_encoded_count ++;
        src_id = p_encoder->frame_encoded_count;

        // Take action based on the frame encoded
        if  (p_encoder->frame_encoded_count < p_encoder->source_cnt)
        {
            // Set individual image source to jpeg encoder
            STD_MEMMOVE(&jpege_src, &(p_encoder->source[src_id]), sizeof(jpege_src_t));
            rc = jpege_set_source(p_encoder->encoder, &jpege_src);
            if (JPEG_FAILED(rc))
            {
                // Change state to IDLE
                p_encoder->state = MPOE_IDLE;
                p_encoder->p_event_handler(p_encoder->p_user_data, JPEG_EVENT_ERROR, NULL);
            }

            // Configure the Mpo Writer for individual image
            rc = mpow_configure(&(p_encoder->mpo_writer), src_id);
            if (JPEG_FAILED(rc))
            {
                JPEG_DBG_ERROR("mpoe_event_handler: failed to configure mpo writer\n");
                 // Change state to IDLE
                p_encoder->state = MPOE_IDLE;
                p_encoder->p_event_handler(p_encoder->p_user_data, JPEG_EVENT_ERROR, NULL);
            }

            // Configure jpeg encoder for individual image
            STD_MEMMOVE(&jpege_cfg, &(p_encoder->config[src_id]), sizeof(jpege_cfg_t));
            jpege_cfg.app2_header_length = p_encoder->mpo_writer.app2_header_length;

            // Start jpeg encoding for individual image
            rc = jpege_start(p_encoder->encoder, &jpege_cfg, &p_encoder->exif_info[src_id]);
            if (JPEG_FAILED(rc))
            {
                // Change state to IDLE
                p_encoder->state = MPOE_IDLE;
                p_encoder->p_event_handler(p_encoder->p_user_data, JPEG_EVENT_ERROR, NULL);
            }
        }
        // Done with all images
        else if (p_encoder->frame_encoded_count == p_encoder->source_cnt)
        {
            // Change state to IDLE
            p_encoder->state = MPOE_IDLE;
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

/******************************************************************************
* Function: jpege_output_handler
* Return values:
*   - JPEGERR_SUCCESS
*     JPEGERR_EFAILED
*     JPEGERR_ENULLPTR
*     JPEGERR_ETIMEDOUT
* Notes: none
*****************************************************************************/
static int jpege_output_handler(
    mpoe_obj_t  *p_obj,    // The encode engine object calling it
    void        *p_arg,
    jpeg_buf_t  *p_buf,    // The buffer containing the output bitstream_t
    uint8_t      last_buf_flag) // The flag to indicate last output buffer
{
    int rc = JPEGERR_SUCCESS;
    mpo_encoder_t *p_encoder;
    uint8_t    last_frame_encoded_flag;
    if (!p_obj)
    {
        JPEG_DBG_ERROR("jpege_output_handler: p_obj NULL!\n");
        return JPEGERR_EFAILED;
    }
    p_encoder = (mpo_encoder_t *)p_obj;

    uint32_t app2_start_offset = 0;
    if(p_arg != NULL)
        app2_start_offset = *(uint32_t *)p_arg;

    // If piece-wise output with App2 start offset
    // Set App2 start offset
    mpow_set_app2_info(&(p_encoder->mpo_writer), p_buf->offset,
            app2_start_offset, p_encoder->frame_encoded_count);

    // If one image encoding is done
    if (last_buf_flag)
    {
        // First individual image
        if (p_encoder->frame_encoded_count == 0)
        {
            mpow_fill_app2_header_first(&(p_encoder->mpo_writer), p_buf->offset);
        }
        else // All other individual images
        {
            mpow_fill_app2_header(&(p_encoder->mpo_writer), p_encoder->frame_encoded_count);
        }
    }

    last_frame_encoded_flag = (p_encoder->frame_encoded_count == (p_encoder->source_cnt - 1));

    // Forward to uppder layer if buffer is full for piece-wise or all images encoding is done for one giant buffer
    if ((p_buf->offset == p_buf->size) ||
        (last_buf_flag && last_frame_encoded_flag))
    {
        // Forward to upper layer
        rc = p_encoder->jpege_dst.p_output_handler(p_encoder->p_user_data,
                                                   p_encoder->dest.p_arg,
                                                  (jpeg_buffer_t)p_buf,
                                                   last_buf_flag && last_frame_encoded_flag);
    }

    return rc;
}


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
    uint32_t           count)
{
    int  rc = JPEGERR_SUCCESS;
    uint8_t i;
    mpo_encoder_t *p_encoder = (mpo_encoder_t *)obj;

    if (!p_encoder)
    {
        return JPEGERR_EUNINITIALIZED;
    }

    // Valid user provided app2_data and buffer
    if (!p_app2_data ||
        !p_app2_data->data_buf ||
        !p_app2_data->data_buf->ptr)
    {
        return JPEGERR_ENULLPTR;
    }

    // Valid count
    if (count != p_encoder->frame_encoded_count)
    {
        return JPEGERR_EBADPARM;
    }

    // Get App2 header data for each individual image
    for (i = 0; i < p_encoder->frame_encoded_count; i++)
    {
        if (JPEG_SUCCEEDED(rc))
        {
            rc = mpow_get_app2_header(&(p_encoder->mpo_writer), (p_app2_data + i), i);
        }
    }

    return rc;
}

static int mpoe_validate_image_data(mpoe_img_data_t *p_img)
{
    uint32_t i;

    // Validate input pointer
    if (!p_img)
        return JPEGERR_ENULLPTR;

    // Validate color format
    if ((p_img->color_format > YCBCRLP_H1V1) && (p_img->color_format < JPEG_BITSTREAM_H2V2))
    {
        JPEG_DBG_ERROR("mpoe_validate_image_data: RGB input not supported: %d\n",
                     p_img->color_format);
        return JPEGERR_EUNSUPPORTED;
    }

    // Validate fragments
    if (p_img->fragment_cnt < 1 || p_img->fragment_cnt > MAX_FRAGMENTS)
    {
        return JPEGERR_EBADPARM;
    }
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
                JPEG_DBG_ERROR("mpoe_validate_image_data: broken fragment (%d)\n", i);
                return JPEGERR_EBADPARM;
            }
        }
    }

    // Validate dimension
    if (!p_img->height || !p_img->width)
    {
        JPEG_DBG_ERROR("mpoe_validate_image_data: bad image dimension: (%dx%d)\n",
                     p_img->width, p_img->height);
        return JPEGERR_EBADPARM;
    }

    return JPEGERR_SUCCESS;
}

int mpoe_validate_image_config(mpoe_img_cfg_t *p_cfg)
{

    if (p_cfg->quality < 1 || p_cfg->quality > 100 ||         // Validate quality factor
        !p_cfg->luma_quant_tbl || !p_cfg->chroma_quant_tbl)   // Validate quant tables
    {
        return JPEGERR_EBADPARM;
    }
    return JPEGERR_SUCCESS;
}

static int mpoe_configure(mpo_encoder_t *p_encoder, mpoe_cfg_t cfg[])
{
    int rc;
    uint32_t i;

    if (!cfg)
        return JPEGERR_ENULLPTR;



    // Make a copy of the config
    for (i = 0; i < p_encoder->source_cnt; i++)
    {
        // Check header type
        if (cfg[i].header_type != OUTPUT_MPO_JFIF && cfg[i].header_type != OUTPUT_MPO_EXIF)
           return JPEGERR_EBADPARM;

        // Check preference
        if (cfg[i].preference >= MPO_ENCODER_PREF_MAX)
           return JPEGERR_EBADPARM;


        // Check image specific configuration
        rc = mpoe_validate_image_config(&(cfg[i].main_cfg)); // main
        if (JPEG_FAILED(rc)) return rc;

        p_encoder->config[i] = cfg[i];
    }

    return JPEGERR_SUCCESS;
}

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
int mpoe_enqueue_output_buffer(
    mpoe_obj_t       obj,
    jpeg_buffer_t   *p_enqueue_buf,
    uint32_t         enqueue_buf_cnt)
{
    int  rc;
    uint8_t i;
    mpo_encoder_t *p_encoder = (mpo_encoder_t *)obj;

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
    rc = jpege_enqueue_output_buffer(p_encoder->encoder, p_enqueue_buf, enqueue_buf_cnt);
    return rc;
}

