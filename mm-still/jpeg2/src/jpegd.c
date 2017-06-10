/*******************************************************************************
* Copyright (c) 2008-2012 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/


/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
11/18/09   mingy   Added tiling output support.
10/22/09   vma     Changed the way engines are picked (using lists)
10/01/09   mingy   Added H/W region decoding unsupport checking.
09/21/09   mingy   Added region based decoding support.
08/03/09   vma     Switched to use the os abstraction layer (os_*)
04/28/09   mingy   Swapped p_decoder->engine_dst width/height for 90/270
                   rotations to obtain correct resizeFacoter by the engine.
                   Added rotation parameter to be passed to the functions.
10/08/08   vma     Fixed abort deadlock
09/25/08   vma     Fixed bug where decode_from is computed wrong
09/15/08   vma     Added Exif reading and aborting
07/07/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpeg_common_private.h"
#include "jpegd.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include "jpeg_reader.h"
#include "jpeg_buffer_private.h"
#include "jpegd_engine.h"
#include "jpeg_postprocessor.h"

#include "os_thread.h"

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define INPUT_REQ_Q_SIZE       2

// This is to check if b is 1/1, 1/2, 1/4, 1/8 of a
#define RATIO_IS_1248(a,b) (((a) == (b) || (a)>>1 == (b) || (a)>>2 == (b) || (a)>>3 == (b)))

/******************************************************************************
* Rule out the cases where the scalable output factor is different in
* width and height.
* eg: output width is 1/2 of input width, but output height is 1/4 of
* input height.
*
* The following implementation is indeed
*     in_width/out_width == in_height/out_height
******************************************************************************/
#define EQUAL_RATIO(in_width, in_height, out_width, out_height) \
                   (in_width * out_height == in_height * out_width)

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef enum
{
    JPEGD_IDLE = 0,
    JPEGD_DECODING = 1,
    JPEGD_ABORTING = 2,

} jpegd_state_t;

typedef struct
{
    jpeg_buf_t  *p_buffer;
    uint32_t     start_offset;
    uint32_t     length;
    uint8_t      is_valid;

} jpegd_input_request_t;

typedef struct jpeg_decoder_t
{
    // The Jpeg Reader
    jpeg_reader_t          reader;
    // The Jpeg Postprocessor
    jpeg_postprocessor_t   processor;
    // The Jpeg Decode Engine
    jpegd_engine_obj_t     engine;
    // The Jpeg event handler
    jpegd_event_handler_t  p_event_handler;
    // The input request handler
    jpegd_input_req_handler_t p_input_req_handler;
    // The pointer to user supplied data (store untouched)
    void                  *p_user_data;
    // The Jpeg decoder source
    jpegd_src_t            source;
    // The Jpeg decoder destination
    jpegd_dst_t            dest;
    // The Jpeg decode engine source
    jpegd_engine_src_t     engine_src;
    // The Jpeg decode engine destination
    jpegd_engine_dst_t     engine_dst;
    // The state of the decoder
    jpegd_state_t          state;
    // os thread to handle input requests
    os_thread_t            thread;
    // os mutex
    os_mutex_t             mutex;
    // os condition variable
    os_cond_t              cond;
    // Flag to signal the input request thread to exit
    uint8_t                thread_exit_flag;
    // Full Jpeg header
    jpeg_header_t         *p_full_hdr;
    // Index of next ping-pong input buffer to be used to receive input
    uint8_t                next_input_idx;
    // Input request queue
    jpegd_input_request_t  input_requests[INPUT_REQ_Q_SIZE];
    // Input request queue head
    uint16_t               input_req_q_head;
    // Input request queue tail
    uint16_t               input_req_q_tail;
} jpeg_decoder_t;

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations of helper functions
** ----------------------------------------------------------------------- */
static void jpegd_engine_event_handler(
    jpegd_engine_obj_t *p_obj,  // The decode engine object calling it
    jpeg_event_t        event,  // The event code
    void               *p_arg); // An optional argument for that event.

static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
    jpegd_handle_input_req_thread(OS_THREAD_FUNC_ARG_T p_arg);

static int jpegd_handle_input_request(jpegd_obj_t  obj,
                                      jpeg_buf_t  *p_buffer,
                                      uint32_t     start_offset,
                                      uint32_t     length);

static int jpegd_engine_output_handler(
    jpegd_engine_obj_t *p_obj,          // The decode engine object calling it
    jpeg_buf_t         *p_luma_buf,     // The buffer containing the luma output
    jpeg_buf_t         *p_chroma_buf);  // The buffer containing the chroma output

static int jpegd_try_engines(jpeg_decoder_t *p_engine,
                             jpegd_cfg_t    *p_cfg,
                             jpegd_dst_t    *p_dest);
/* -----------------------------------------------------------------------
** Extern variables
** ----------------------------------------------------------------------- */

#ifndef CODEC_V1
/* use extenal list*/
extern jpegd_engine_lists_t jpegd_engine_try_lists;

#else
/* use software only */
static jpegd_engine_profile_t* sw_only_list[2] = {&jpegd_engine_sw_profile, 0};

/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
jpegd_engine_lists_t jpegd_engine_try_lists =
{
    sw_only_list,   // JPEG_DECODER_PREF_SOFTWARE_ONLY
    sw_only_list, // JPEG_DECODER_PREF_SOFTWARE_ONLY
    sw_only_list,   // JPEG_DECODER_PREF_SOFTWARE_ONLY
    sw_only_list, // JPEG_DECODER_PREF_SOFTWARE_ONLY
    sw_only_list,   // JPEG_DECODER_PREF_SOFTWARE_ONLY
};

#endif

/* =======================================================================
**                          Function Definitions
** ======================================================================= */

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
 * Return values:
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
    void                   *p_user_data)
{
    jpeg_decoder_t *p_decoder;

    // Input validation
    if (!p_event_handler)
        return JPEGERR_ENULLPTR;

    // Allocate for the Jpeg Decoder structure
    p_decoder = (jpeg_decoder_t *)JPEG_MALLOC(sizeof(jpeg_decoder_t));
    if (!p_decoder)
        return JPEGERR_EMALLOC;

    // Initialize all fields in the decoder structure
    STD_MEMSET(p_decoder, 0, sizeof(jpeg_decoder_t));

    // Store the event handler
    p_decoder->p_event_handler = p_event_handler;

    // Store the user data
    p_decoder->p_user_data = p_user_data;

    // Initialize thread condition variable and mutex
    (void)os_mutex_init(&(p_decoder->mutex));
    (void)os_cond_init(&(p_decoder->cond));

    // Initialize postprocessor
    (void)jpeg_postprocessor_init(&(p_decoder->processor),
                                  p_output_handler,
                                  p_user_data);

    // Create thread to handle input requests
    if (os_thread_create(&p_decoder->thread,
                         jpegd_handle_input_req_thread,
                         (void *)(p_decoder)))
    {
        jpeg_postprocessor_destroy(&(p_decoder->processor));

        os_mutex_destroy(&(p_decoder->mutex));
        os_cond_destroy(&(p_decoder->cond));
        JPEG_FREE(p_decoder);
        return JPEGERR_EFAILED;
    }

    // Set up engine source handlers
    p_decoder->engine_src.p_input_req_handler = &jpegd_handle_input_request;

    // Cast the created decoder into the opaque object pointer
    *p_obj = (jpegd_obj_t)p_decoder;

    // Set up engine output handler
    p_decoder->engine_dst.p_output_handler = &jpegd_engine_output_handler;

    // Set up engine destination buffers
    if (JPEG_FAILED(jpeg_buffer_init((jpeg_buffer_t *)&(p_decoder->engine_dst.p_luma_buffers[0]))) ||
        JPEG_FAILED(jpeg_buffer_init((jpeg_buffer_t *)&(p_decoder->engine_dst.p_luma_buffers[1]))) ||
        JPEG_FAILED(jpeg_buffer_init((jpeg_buffer_t *)&(p_decoder->engine_dst.p_chroma_buffers[0]))) ||
        JPEG_FAILED(jpeg_buffer_init((jpeg_buffer_t *)&(p_decoder->engine_dst.p_chroma_buffers[1]))))
    {
        jpegd_destroy(p_obj);
        return JPEGERR_EFAILED;

    }
    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: jpegd_set_source
 * Description: Assigns the image source to the Jpeg Decoder object.
 * Input parameters:
 *   obj       - The Jpeg Decoder object
 *   p_source  - The pointer to the source object to be assigned to the Jpeg
 *               Decoder.
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_ENULLPTR
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpegd_set_source(
    jpegd_obj_t  obj,
    jpegd_src_t *p_source)
{
    int rc;
    jpeg_decoder_t *p_decoder = (jpeg_decoder_t *)obj;
    if (!p_decoder)
        return JPEGERR_EUNINITIALIZED;

    // Input validation
    if (!p_source || !p_source->p_input_req_handler)
       return JPEGERR_ENULLPTR;

    if (!p_source->buffers[0] || !p_source->buffers[1])
    {
        JPEG_DBG_LOW("jpegd_set_source: source buffers uninitialized\n");
        return JPEGERR_EBADPARM;
    }

    if (!p_source->total_length)
    {
        JPEG_DBG_LOW("jpegd_set_source: bad total length\n");
        return JPEGERR_EBADPARM;
    }

    // If all validation passed, store the source pointer
    p_decoder->p_input_req_handler = p_source->p_input_req_handler;
    p_decoder->source = *p_source;

    // Initialize jpeg reader
    rc = jpegr_init(&(p_decoder->reader), (jpegd_obj_t)p_decoder,
                    &(p_decoder->source), &jpegd_handle_input_request);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_MED("jpegd_set_source: jpegr_init failed\n");
        return rc;
    }

    // Derive engine source object
    p_decoder->engine_src.p_buffers[0] = (jpeg_buf_t *)p_source->buffers[0];
    p_decoder->engine_src.p_buffers[1] = (jpeg_buf_t *)p_source->buffers[1];
    p_decoder->engine_src.total_length = p_source->total_length;

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: jpegd_read_header
 * Description: todo
 * Input parameters: todo
 * Return value: todo
 * Notes: none
 *****************************************************************************/
int jpegd_read_header(
    jpegd_obj_t  obj,
    jpeg_hdr_t  *p_header)
{
    jpeg_decoder_t *p_decoder = (jpeg_decoder_t *)obj;
    if (!p_decoder)
        return JPEGERR_EUNINITIALIZED;

    // Reject request if decoder is actively decoding or is cleaning up
    if (p_decoder->state != JPEGD_IDLE)
        return JPEGERR_EBADSTATE;

    // Parse the Jpeg header
    p_decoder->p_full_hdr = jpegr_read_header(&(p_decoder->reader));

    // If full header is present, derive short header from it
    if (p_decoder->p_full_hdr)
    {
        jpegd_convert_frame_info(p_decoder->p_full_hdr->p_main_frame_info,
                                 &(p_header->main));
        jpegd_convert_frame_info(p_decoder->p_full_hdr->p_tn_frame_info,
                                 &(p_header->thumbnail));
        p_header->exif_info = (exif_info_obj_t)p_decoder->p_full_hdr->p_exif_info;
    }
    else
    {
        return JPEGERR_EFAILED;
    }

    return JPEGERR_SUCCESS;
}

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
    uint32_t            list_count)
{
    jpeg_decoder_t *p_decoder = (jpeg_decoder_t *)obj;

    uint32_t i, q_index;

    // Reject if the number of buffers being enqueued is greater than the queue size
    if (list_count > OUTPUT_BUF_Q_SIZE)
    {
        JPEG_DBG_ERROR("jpegd_enqueue_output_buf: fatal error: number of buffers is greater than queue size\n");
        return JPEGERR_EFAILED;
    }

    os_mutex_lock(&(p_decoder->processor.mutex));

    for (i = 0; i < list_count; i++)
    {
        q_index = (p_decoder->processor.output_buf_q_tail + i) % OUTPUT_BUF_Q_SIZE;

        if (p_decoder->processor.output_buffers[q_index].is_in_q)
        {
            JPEG_DBG_ERROR("jpegd_enqueue_output_buf: fatal error: output buffer queue is full\n");
            os_mutex_unlock(&(p_decoder->processor.mutex));
            return JPEGERR_EFAILED;
        }

        // Adding the output buffers to the Q one by one
        p_decoder->processor.output_buffers[q_index].data        = (p_output_buf_array + i)->data;
        p_decoder->processor.output_buffers[q_index].tile_width  = (p_output_buf_array + i)->tile_width;
        p_decoder->processor.output_buffers[q_index].tile_height = (p_output_buf_array + i)->tile_height;
        p_decoder->processor.output_buffers[q_index].is_in_q     = true;
    }

    // Enqueue the output buffers
    p_decoder->processor.output_buf_q_tail = (p_decoder->processor.output_buf_q_tail + list_count) % OUTPUT_BUF_Q_SIZE;

    // Signal that outpuf buffer enqueue is done
    os_cond_signal(&(p_decoder->processor.cond));
    os_mutex_unlock(&(p_decoder->processor.mutex));

    return JPEGERR_SUCCESS;
}

/******************************************************************************
 * Function: jpegd_start
 * Description:
 * Input parameters:
 * Return values:
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
    uint32_t            list_count)
{
    int rc;
    jpeg_decoder_t *p_decoder = (jpeg_decoder_t *)obj;

    if (!p_decoder || !p_cfg || !p_dest)
        return JPEGERR_ENULLPTR;

    // Validate rotation paramter
    if (  0 != p_cfg->rotation &&  90 != p_cfg->rotation &&
        180 != p_cfg->rotation && 270 != p_cfg->rotation)
    {
        return JPEGERR_EBADPARM;
    }

    /**************************************************************************
    *   There are some restrictions when tiling is enabled:
    *
    * 1. Arbitrary down scaling is not supported.
    *    Only scalable down scaling is supported, meaning decoding to 1/2, 1/4,
    *    1/8 of the original dimensions.
    *
    * 2. If the scalable down scaling output size is less than the thumbnail
    *    image size, and the decoding preference is set to AUTO, then the
    *    decoder will decode from thumbnail and then perform arbitrary down
    *    scaling. This is not allowed in the tiling case since arbitrary
    *    down scaling is not supported if tiling is enabled.
    *
    **************************************************************************/

    // todo need to reset this for back-to-back decoding
    //p_output_buf_array->tile_width = 0;
    //p_output_buf_array->tile_height = 0;

    JPEG_DBG_LOW("--(%d)%s() tile width = %d, tile height = %d\n", __LINE__, __func__,
           p_output_buf_array->tile_width, p_output_buf_array->tile_height);

    // Tiling is not requested if the tile dimension is
    // the same to the output dimension.
    if (p_output_buf_array->tile_width  == p_dest->stride &&
        p_output_buf_array->tile_height == p_dest->height)
    {
        p_decoder->processor.tiling_enabled  = false;
    }

    // Tiling is not requested if the tiling dimension is 0.
    // set the tile dimension to be the output dimension
    else if (p_output_buf_array->tile_width  == 0 ||
             p_output_buf_array->tile_height == 0)
    {
        p_output_buf_array->tile_width       = p_dest->width;
        p_output_buf_array->tile_height      = p_dest->height;
        p_decoder->processor.tiling_enabled  = false;
    }
    else
    {
        p_decoder->processor.tiling_enabled = true;

        if (p_cfg->decode_from == JPEGD_DECODE_FROM_AUTO)
        {
            if (p_decoder->p_full_hdr->p_tn_frame_info &&   // thumbnail is present
                RATIO_IS_1248(p_decoder->p_full_hdr->p_tn_frame_info->width, p_dest->width) &&
                RATIO_IS_1248(p_decoder->p_full_hdr->p_tn_frame_info->height, p_dest->height))
            {
                p_cfg->decode_from = JPEGD_DECODE_FROM_THUMB;
            }
            else
            {
                p_cfg->decode_from = JPEGD_DECODE_FROM_MAIN;
            }
        }

        if (p_cfg->decode_from == JPEGD_DECODE_FROM_THUMB)
        {
            if (!RATIO_IS_1248(p_decoder->p_full_hdr->p_tn_frame_info->width, p_dest->width)  ||
                !RATIO_IS_1248(p_decoder->p_full_hdr->p_tn_frame_info->height, p_dest->height)||
                !EQUAL_RATIO(p_decoder->p_full_hdr->p_tn_frame_info->width, p_decoder->p_full_hdr->p_tn_frame_info->height,
                p_dest->width, p_dest->height))
            {
                return JPEGERR_EUNSUPPORTED;
            }
        }
        else // it must be JPEGD_DECODE_FROM_MAIN
        {
            if (!RATIO_IS_1248(p_decoder->p_full_hdr->p_main_frame_info->width, p_dest->width)  ||
                !RATIO_IS_1248(p_decoder->p_full_hdr->p_main_frame_info->height, p_dest->height)||
                !EQUAL_RATIO(p_decoder->p_full_hdr->p_main_frame_info->width, p_decoder->p_full_hdr->p_main_frame_info->height,
                p_dest->width, p_dest->height))
            {
                return JPEGERR_EUNSUPPORTED;
            }
        }

        // tiling supports only RGB output
        if (p_dest->output_format != RGB565 &&
            p_dest->output_format != RGB888 &&
            p_dest->output_format != RGBa)
        {
            return JPEGERR_EUNSUPPORTED;
        }

        // tiling supports only 0 rotation
        if (p_cfg->rotation != 0)
        {
            return JPEGERR_EUNSUPPORTED;
        }
    }
    JPEG_DBG_LOW("%s,%d\n",__func__,__LINE__);
    // if region is defined, re-assign the output width/height
    if (p_dest->region.right || p_dest->region.bottom)
    {
        if (0 == p_cfg->rotation || 180 == p_cfg->rotation)
        {
            p_dest->width  = STD_MIN((p_dest->width),  (uint32_t)(p_dest->region.right  - p_dest->region.left + 1));
            p_dest->height = STD_MIN((p_dest->height), (uint32_t)(p_dest->region.bottom - p_dest->region.top  + 1));
        }
        // Swap output width/height for 90/270 rotation cases
        else if (90 == p_cfg->rotation || 270 == p_cfg->rotation)
        {
            p_dest->height  = STD_MIN((p_dest->height), (uint32_t)(p_dest->region.right  - p_dest->region.left + 1));
            p_dest->width   = STD_MIN((p_dest->width),  (uint32_t)(p_dest->region.bottom - p_dest->region.top  + 1));
        }
        // Unsupported rotation cases
        else
        {
            return JPEGERR_EBADPARM;
        }
    }
    JPEG_DBG_LOW("%s,%d\n",__func__,__LINE__);
    // Check whether the Jpeg Decoder is IDLE before proceeding
    if (p_decoder->state != JPEGD_IDLE)
        return JPEGERR_EBADSTATE;

    if (p_cfg->preference >= JPEG_DECODER_PREF_MAX ||
        p_dest->output_format >= JPEG_COLOR_FORMAT_MAX)
    {
        return JPEGERR_EBADPARM;
    }
    JPEG_DBG_LOW("%s,%d\n",__func__,__LINE__);
    // If header is not parsed yet, parse now
    if (!p_decoder->p_full_hdr)
    {
        p_decoder->p_full_hdr = jpegr_read_header(&(p_decoder->reader));
        if (!p_decoder->p_full_hdr)
        {
            JPEG_DBG_MED("jpegd_start: jpegr_read_header failed\n");
            return JPEGERR_EFAILED;
        }
    }
    // Determine which frame info to use based on 'decode_from'
    p_decoder->engine_src.p_frame_info = p_decoder->p_full_hdr->p_tn_frame_info;
    JPEG_DBG_LOW("%s,%d\n",__func__,__LINE__);
    switch (p_cfg->decode_from)
    {
    case JPEGD_DECODE_FROM_AUTO:
        {
            // Decode from thumbnail only if it exists and it is large enough
            if (!(p_decoder->engine_src.p_frame_info &&
                  p_decoder->engine_src.p_frame_info->width >= p_dest->width &&
                  p_decoder->engine_src.p_frame_info->height >= p_dest->height))
            {
                p_decoder->engine_src.p_frame_info = p_decoder->p_full_hdr->p_main_frame_info;
            }
        };
        break;
    case JPEGD_DECODE_FROM_THUMB:
        {
            if (!p_decoder->engine_src.p_frame_info)
            {
                JPEG_DBG_LOW("jpegd_start: forcing thumbnail decoding while it is absent\n");
                return JPEGERR_EFAILED;
            }
        }
        break;
    case JPEGD_DECODE_FROM_MAIN:
        p_decoder->engine_src.p_frame_info = p_decoder->p_full_hdr->p_main_frame_info;
        break;
    default:
        return JPEGERR_EBADPARM;
    }

    // Enqueue the first output buffer, reject if the tiling buffer is not
    // a single line when tiling is requsted.
    if (p_decoder->processor.tiling_enabled &&
        (p_output_buf_array->tile_height != 1))
    {
        return JPEGERR_EUNSUPPORTED;
    }
    else
    {
        JPEG_DBG_LOW("%s,%d\n",__func__,__LINE__);
        jpegd_enqueue_output_buf(obj, p_output_buf_array, list_count);
    }

    // Store the destination object
    p_decoder->dest = *p_dest;

    /*************************************************************************
    * The Resize Factor variable (in the decoder engine) is computed based on
    * the un-rotated output dimensions.
    * For 90/270 rotation cases, the output width and height need to be
    * swapped in order to compute a correct Resize Factor value.
    *
    * p_dest->height/width is the final output dimensions,
    * p_decoder->engine_dst.width/height is the decoder engine output
    * dimensions (before post processor)
    *
    * The following statements ensure then engine works on the un-rotated case.
    *************************************************************************/
    if (90 == p_cfg->rotation || 270 == p_cfg->rotation)
    {
        p_decoder->engine_dst.width  = p_dest->height;
        p_decoder->engine_dst.height = p_dest->width;
    }
    else
    {
        p_decoder->engine_dst.width  = p_dest->width;
        p_decoder->engine_dst.height = p_dest->height;
    }

    // Assigne region to engine destination
    p_decoder->engine_dst.region = p_dest->region;
    JPEG_DBG_LOW("%s,%d, rc = %d\n",__func__,__LINE__,rc);
    p_decoder->engine_dst.p_output_buf_array = p_output_buf_array;

    rc = jpegd_try_engines(p_decoder, p_cfg, p_dest);

    if (JPEG_SUCCEEDED(rc))
    {
        JPEG_DBG_LOW("%s,%d\n",__func__,__LINE__);
        p_decoder->state = JPEGD_DECODING;
    }
    JPEG_DBG_LOW("%s,%d\n",__func__,__LINE__);
    return rc;
}

/******************************************************************************
 * Function: jpegd_abort
 * Description: Aborts the decoding session in progress. If there is no decoding
 *              session in progress, a JPEGERR_SUCCESS will be returned
 *              immediately. Otherwise, the call will block until the decoding
 *              is completely aborted, then return a JPEGERR_SUCCESS.
 * Input parameters:
 *   obj       - The Jpeg Decoder object
 * Return values:
 *     JPEGERR_SUCCESS
 *     JPEGERR_EFAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
int jpegd_abort(jpegd_obj_t obj)
{
    int rc = JPEGERR_SUCCESS;
    jpeg_decoder_t *p_decoder = (jpeg_decoder_t *)obj;
    if (!p_decoder)
        return JPEGERR_EUNINITIALIZED;

    os_mutex_lock(&p_decoder->mutex);
    if (p_decoder->state != JPEGD_IDLE)
    {
        p_decoder->state = JPEGD_ABORTING;
        os_mutex_unlock(&p_decoder->mutex);
        rc = p_decoder->engine.abort(&(p_decoder->engine));

        os_mutex_lock(&(p_decoder->processor.mutex));   // Lock the post processor
        p_decoder->processor.abort_flag = true;         // Set post processor abort flag
        os_cond_signal(&(p_decoder->processor.cond));   // Signal the post processor (abort)
        os_mutex_unlock(&(p_decoder->processor.mutex)); // Unlock post processor
    }
    else
        os_mutex_unlock(&p_decoder->mutex);

    return rc;
}

/******************************************************************************
 * Function: jpegd_destroy
 * Description: Destroys the Jpeg Decoder object, releasing all memory
 *              previously allocated.
 * Input parameters:
 *   p_obj       - The pointer to the Jpeg Decoder object
 * Return values: None
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
void jpegd_destroy(jpegd_obj_t *p_obj)
{
    if (p_obj)
    {
        jpeg_decoder_t *p_decoder = (jpeg_decoder_t *)(*p_obj);
        if (p_decoder)
        {
            // Signal the input request thread to exit
            os_mutex_lock(&(p_decoder->mutex));
            p_decoder->thread_exit_flag = true;
            os_cond_signal(&(p_decoder->cond));
            os_mutex_unlock(&(p_decoder->mutex));

            // Join the input request thread
            os_thread_join(&p_decoder->thread, NULL);

            // Destroy destination buffers
            jpeg_buffer_destroy((jpeg_buffer_t *)&(p_decoder->engine_dst.p_luma_buffers[0]));
            jpeg_buffer_destroy((jpeg_buffer_t *)&(p_decoder->engine_dst.p_luma_buffers[1]));
            jpeg_buffer_destroy((jpeg_buffer_t *)&(p_decoder->engine_dst.p_chroma_buffers[0]));
            jpeg_buffer_destroy((jpeg_buffer_t *)&(p_decoder->engine_dst.p_chroma_buffers[1]));
            jpeg_postprocessor_destroy(&(p_decoder->processor));
            // Destory the reader
            jpegr_destroy(&p_decoder->reader);
            // Destroy the engine
            if (p_decoder->engine.destroy)
            {
                p_decoder->engine.destroy(&(p_decoder->engine));
            }
            os_mutex_destroy(&(p_decoder->mutex));
            os_cond_destroy(&(p_decoder->cond));
            JPEG_FREE(p_decoder);
            *p_obj = NULL;
        }
        jpeg_show_leak();
    }
}

static void jpegd_engine_event_handler(
    jpegd_engine_obj_t *p_obj,  // The decode engine object calling it
    jpeg_event_t        event,  // The event code
    void               *p_arg)  // An optional argument for that event.
{
    jpeg_decoder_t *p_decoder = (jpeg_decoder_t *)p_obj->decoder;
    switch (event)
    {
    case JPEG_EVENT_DONE:
        JPEG_DBG_HIGH("jpegd_engine_event_handler: JPEG_EVENT_DONE\n");
        p_decoder->state = JPEGD_IDLE;
        // Forward to the upper layer
        p_decoder->p_event_handler(p_decoder->p_user_data, event, p_arg);
        break;
    case JPEG_EVENT_WARNING:
    case JPEG_EVENT_ERROR:
        if (event == JPEG_EVENT_ERROR)
        {
            p_decoder->state = JPEGD_IDLE;
        }
        // Forward to the upper layer
        p_decoder->p_event_handler(p_decoder->p_user_data, event, p_arg);
        break;
    }
}

static int jpegd_engine_output_handler(
    jpegd_engine_obj_t  *p_obj,         // The decode engine object calling it
    jpeg_buf_t          *p_luma_buf,    // The buffer containing the luma output
    jpeg_buf_t          *p_chroma_buf)  // The buffer containing the chroma output
{
    jpeg_decoder_t *p_decoder = (jpeg_decoder_t *)p_obj->decoder;
    return jpeg_postprocessor_process(&p_decoder->processor, p_luma_buf, p_chroma_buf);
}

void jpegd_convert_frame_info(jpeg_frame_info_t *p_full_info,
                              jpeg_frm_info_t *p_short_info)
{
    if (!p_short_info) return;

    if (!p_full_info)
    {
        STD_MEMSET(p_short_info, 0, sizeof(jpeg_frm_info_t));
    }
    else
    {
        p_short_info->width  = p_full_info->width;
        p_short_info->height = p_full_info->height;
        p_short_info->subsampling = p_full_info->subsampling;
    }
}

static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
    jpegd_handle_input_req_thread(OS_THREAD_FUNC_ARG_T p_arg)
{
    jpeg_decoder_t *p_decoder = (jpeg_decoder_t *)p_arg;

    for (;;)
    {
        jpegd_input_request_t *p_request;

        os_mutex_lock(&(p_decoder->mutex));

        // Block if no request is pending
        while (!p_decoder->input_requests[p_decoder->input_req_q_head].is_valid &&
               !p_decoder->thread_exit_flag)
        {
            os_cond_wait(&(p_decoder->cond), &(p_decoder->mutex));
        }

        // It is signaled that the thread should exit
        if (p_decoder->thread_exit_flag)
        {
            os_mutex_unlock(&(p_decoder->mutex));
            break;
        }

        // Dequeue the input request queue and service it
        p_request = p_decoder->input_requests + p_decoder->input_req_q_head;
        p_request->p_buffer->offset =
            p_decoder->p_input_req_handler(p_decoder->p_user_data,
                                           p_request->p_buffer,
                                           p_request->start_offset,
                                           p_request->length);
        p_request->is_valid = false;
        p_decoder->input_req_q_head = (p_decoder->input_req_q_head + 1) %
            INPUT_REQ_Q_SIZE;

        // Mark the buffer as filled
        jpeg_buffer_mark_filled(p_request->p_buffer);
        os_mutex_unlock(&(p_decoder->mutex));
    }
    return 0;
}

static int jpegd_handle_input_request(jpegd_obj_t     obj,
                                      jpeg_buf_t     *p_buffer,
                                      uint32_t        start_offset,
                                      uint32_t        length)
{
    jpegd_input_request_t *p_request;
    jpeg_decoder_t *p_decoder = (jpeg_decoder_t *)obj;

    os_mutex_lock(&(p_decoder->mutex));
    if (p_decoder->input_requests[p_decoder->input_req_q_tail].is_valid)
    {
        JPEG_DBG_ERROR("jpegd_handle_input_request: fatal error: input request queue is full\n");
        return JPEGERR_EFAILED;
    }

    // Set up input request
    p_request = p_decoder->input_requests + p_decoder->input_req_q_tail;
    p_request->p_buffer      = p_buffer;
    p_request->start_offset  = start_offset;
    p_request->length        = length;
    p_request->is_valid      = true;

    // Enqueue the request
    p_decoder->input_req_q_tail = (p_decoder->input_req_q_tail + 1) % INPUT_REQ_Q_SIZE;

    // Signal thread to handle the request
    os_cond_signal(&(p_decoder->cond));
    os_mutex_unlock(&(p_decoder->mutex));

    return JPEGERR_SUCCESS;
}

static int jpegd_try_engines(jpeg_decoder_t    *p_decoder,
                             jpegd_cfg_t       *p_cfg,
                             jpegd_dst_t       *p_dest)
{
    uint32_t chunk_width, chunk_height;
    jpegd_engine_profile_t** profile_list = jpegd_engine_try_lists[p_cfg->preference];

    uint8_t *p_luma_buf;
    uint8_t *p_chroma_buf;
    // Go through the engine try list according to the pref
    JPEG_DBG_LOW("%s,%d\n",__func__,__LINE__);
    while (*profile_list)
    {
        int rc = JPEGERR_SUCCESS;
        jpegd_engine_profile_t *engine_profile = *profile_list;

        // If same engine is previously created and initialized,
        // skip creation and intialization
        if (p_decoder->engine.create != engine_profile->create_func ||
            !p_decoder->engine.is_intialized)
        {
            // Create the engine
            JPEG_DBG_LOW("%s,%d\n",__func__,__LINE__);
            engine_profile->create_func(&(p_decoder->engine), (jpegd_obj_t )p_decoder);

            // Initialize engine
            rc = p_decoder->engine.init(&(p_decoder->engine),
                                        &jpegd_engine_event_handler,
                                        &jpegd_handle_input_request);
        }

        if (JPEG_SUCCEEDED(rc))
        {
            JPEG_DBG_ERROR("%s:%d] rotation %d scale_factor %d num_planes %d\n",
              __func__, __LINE__, p_cfg->hw_rotation, p_cfg->scale_factor,
              p_cfg->num_planes);
            // Configure the engine
            p_decoder->engine_src.hw_rotation = p_cfg->hw_rotation;
            p_decoder->engine_src.hw_scale_factor = p_cfg->scale_factor;
            p_decoder->engine_src.num_planes = p_cfg->num_planes;
            rc = p_decoder->engine.configure(&(p_decoder->engine),
                                             &(p_decoder->engine_src),
                                             &(p_decoder->engine_dst),
                                             p_dest,
                                             &chunk_width,
                                             &chunk_height);
            JPEG_DBG_LOW("jpegd_try_engines: configure engine (%s): %d\n",
                         engine_profile->engine_name, rc);
        }

        if (JPEG_SUCCEEDED(rc) && !p_decoder->engine.skip_pp)
        {
            // Configure the post processor
            rc = jpeg_postprocessor_configure(&(p_decoder->processor),
                                              &(p_decoder->engine_dst),
                                              p_dest,
                                              chunk_width,
                                              chunk_height,
                                              engine_profile->need_pmem,
                                              p_cfg->rotation);
            JPEG_DBG_LOW("jpegd_try_engines: configure postprocessor (%s): %d\n",
                         engine_profile->engine_name, rc);
        }

        if (JPEG_SUCCEEDED(rc))
        {
            // Start the decode engine
            rc = p_decoder->engine.start(&(p_decoder->engine));
            JPEG_DBG_LOW("jpegd_try_engines: start engine (%s): %d\n",
                         engine_profile->engine_name, rc);
        }

        // Print debug message
        if (JPEG_SUCCEEDED(rc))
        {
            JPEG_DBG_MED("jpegd_try_engines: Use %s\n", engine_profile->engine_name);
            return rc;
        }

        // Move on to the next one
        profile_list++;
    }

    // Exhausted all engines without success
    return JPEGERR_EFAILED;
}

