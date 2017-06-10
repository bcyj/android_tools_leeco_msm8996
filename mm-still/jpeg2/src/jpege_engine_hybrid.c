/*****************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential. *
*****************************************************************************/

/*========================================================================
                             Edit History


when       who     what, where, why
10/16/14   vgaur   Added support for multi threaded fastCV
12/17/13   staceyw Fixed a coverity issue.
07/09/13   staceyw Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "jpeg_buffer_private.h"
#include "jpege_engine_hybrid.h"
#include "jpeg_queue.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include "jpege_hybrid_englist.h"
#include "jpeg_common_private.h"
#ifdef _DEBUG
#include "os_timer.h"
#endif
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
typedef struct jpege_engine_hybrid_t jpege_engine_hybrid_t;

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* Function prototypes of jpege_engine_obj_t interface functions */
static void jpege_engine_hybrid_create  (
    jpege_engine_obj_t *p_obj,
    jpege_obj_t encoder);
static int  jpege_engine_hybrid_init    (
    jpege_engine_obj_t*,
    jpege_engine_event_handler_t);
static int  jpege_engine_hybrid_start   (
    jpege_engine_obj_t*,
    jpege_img_cfg_t*,
    jpege_img_data_t*,
    jpege_engine_dst_t*);
static int  jpege_engine_hybrid_abort   (jpege_engine_obj_t*);
static void jpege_engine_hybrid_destroy (jpege_engine_obj_t*);
static int  jpege_engine_hybrid_check_start_param (
    jpege_img_cfg_t*,
    jpege_img_data_t*,
    jpege_engine_dst_t*);
static int   jpege_engine_hybrid_configure(
    jpege_engine_hybrid_t*, jpege_img_cfg_t*,
    jpege_img_data_t*, jpege_engine_dst_t*);
static int   jpege_engine_hybrid_configure_scale(
    jpege_engine_hybrid_t*, jpege_img_cfg_t*,
    jpege_img_data_t*, jpege_engine_dst_t*);
/******************************************************************************
 * Function: jpege_engine_hybrid_event_handler
 * Description: It is called whenever JPEG events like JPEG_EVENT_DONE,
 * JPEG_EVENT_WARNING,JPEG_EVENT_ERROR,JPEG_EVENT_ABORTED occur. In case of
 * which is sent to upper layer. In case of JPEG_EVENT_DONE, it check s for any
 * pending jobs and if there are any then the job is dequeued and performed.
 * Input parameters:
 *  p_obj   The encode engine object calling it
 *  event   The event code
 *  p_arg   An optional argument for that event
 * Return values:
 *   JPEGERR_SUCCESS
 *   JPEGERR_FAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
static int jpege_engine_hybrid_event_handler(
    jpege_engine_hybrid_obj_t *p_obj,        // The encode engine object calling it
    jpeg_event_t               event,        // The event code
    void                      *p_arg);       // An optional argument for that event.
/******************************************************************************
 * Function: jpege_engine_hybrid_output_handler
 * Description: Copies buffer containing output bitstream to another buffer
 * which is sent to upper layer.
 * Input parameters:
 *   p_obj       The encode engine object calling it
 *  buf_ptr      The buffer containing the output bitstream_t
 *  buf_offset   The size of output bitstream to be copied
 * Return values:
 *   JPEGERR_SUCCESS
 *   JPEGERR_FAILED
 * (See jpegerr.h for description of error values.)
 * Notes: none
 *****************************************************************************/
static int jpege_engine_hybrid_output_handler(
    jpege_engine_hybrid_obj_t *p_obj,
    uint8_t                   *buf_ptr,
    uint32_t                   buf_offset);
static int  jpege_hybrid_get_output_buffer(
    jpege_engine_hybrid_obj_t  *p_obj,
    jpeg_buf_t        **pp_dequeue_buffer,
    uint8_t             src_index);

void JPEG_SWAP(uint32_t *a, uint32_t *b)
{
    uint32_t  c;
    c = *b;
    *b = *a;
    *a = c;
}
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** -----------------------------------------------------------------------*/
// the maximum number of output buffers
#define MAX_OUTPUT_BUF 30

/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
static const char jpege_engine_hybrid_name[] = "Jpeg Hybrid Encode Engine";
jpege_engine_profile_t jpege_engine_hybrid_profile = {jpege_engine_hybrid_create, jpege_engine_hybrid_name};

struct jpege_engine_hybrid_t
{
    jpege_engine_obj_t                *p_wrapper;          // The wrapper engine object
    jpege_engine_create_t              create;
    jpege_engine_init_t                init;
    jpege_engine_check_start_param_t   check_start_param;
    jpege_engine_start_t               start;
    jpege_engine_abort_t               abort;
    jpege_engine_destroy_t             destroy;
    void                              *p_engine;
    jpege_obj_t                        encoder;
    jpege_engine_dst_t                 dest;               // the destination object
    uint8_t                            is_initialized;
    os_cond_t                          final_output_cond;

    jpege_engine_hybrid_obj_t          engines[MAX_HYBRID_ENGINE];
    uint8_t                            engine_count;
    uint8_t                            job_count;
    uint8_t                            job_scheduled;
    uint8_t                            job_done;
    jpeg_queue_t                       job_queue;
    jpeg_buf_t                        *p_output_buf[MAX_OUTPUT_BUF];
    uint8_t                            output_buf_received[MAX_OUTPUT_BUF];
    uint8_t                            output_buf_received_cnt;
    uint32_t                           output_buf_size;
    uint8_t                            output_buf_cnt;
    uint8_t                            abort_flag;          // abort flag
    uint8_t                            error_flag;          // error flag
    uint8_t                            final_output_flag;   // last buffer flag
    os_mutex_t                         mutex;               // os mutex
    os_cond_t                          cond;                // os condition variable
    jpege_engine_event_handler_t       p_handler;           // the event handler
    jpeg_buf_t                        *p_dest_buf;          // destination buffer

};

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
typedef struct jpege_engine_job_t jpege_engine_job_t;
struct jpege_engine_job_t
{
    jpege_engine_hybrid_cfg_t          engine_cfg;
    jpege_img_cfg_t                   *p_config;           // the image configure
    jpege_img_data_t                  *p_source;           // the source object

};
/* =======================================================================
**                          Function Definitions
** ======================================================================= */

static void jpege_engine_hybrid_create(jpege_engine_obj_t *p_obj, jpege_obj_t encoder)
{
    if (p_obj)
    {
        // Destroy previous engine if it exists
        if (p_obj->destroy)
        {
            p_obj->destroy(p_obj);
        }
        p_obj->create            = &jpege_engine_hybrid_create;
        p_obj->init              = &jpege_engine_hybrid_init;
        p_obj->check_start_param = &jpege_engine_hybrid_check_start_param;
        p_obj->start             = &jpege_engine_hybrid_start;
        p_obj->abort             = &jpege_engine_hybrid_abort;
        p_obj->destroy           = &jpege_engine_hybrid_destroy;
        p_obj->p_engine          = NULL;
        p_obj->encoder           = encoder;
        p_obj->is_initialized    = false;

    }
}

static int jpege_engine_hybrid_init(
    jpege_engine_obj_t           *p_obj,
    jpege_engine_event_handler_t  p_handler)
{
    int rc = JPEGERR_SUCCESS;
    jpege_engine_hybrid_t   *p_hybrid;
    jpege_engine_hybrid_profile_t  **profile_list = hybrid_list;
    uint8_t  count = 0;

    // Validate input arguments
    if (!p_obj || !p_handler)
    {
        JPEG_DBG_ERROR("jpege_engine_hybrid_init :  NULL input pointer\n");
        return JPEGERR_ENULLPTR;
    }

    // Allocate memory for the engine structure
    p_hybrid = (jpege_engine_hybrid_t *)JPEG_MALLOC(sizeof(jpege_engine_hybrid_t));
    if (!p_hybrid)
    {
        JPEG_DBG_ERROR("jpege_engine_hybrid_init :  Allocating memory for the engine structure failed\n");
        return JPEGERR_EMALLOC;
    }

    // Initialize the fields inside the engine structure b elow
    STD_MEMSET(p_hybrid, 0, sizeof(jpege_engine_hybrid_t));  // Zero out the entire structure

    (void)os_mutex_init(&(p_hybrid->mutex));
    (void)os_cond_init(&(p_hybrid->cond));
    // Initialize the condition used for last buffer output
    os_cond_init(&(p_hybrid->final_output_cond));

    p_hybrid->p_wrapper = p_obj;                         // Initialize the pointer to the wrapper
    p_hybrid->p_handler = p_handler;

    // Initialize jpeg output queue
    rc = jpeg_queue_init(&(p_hybrid->job_queue));
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_engine_hybrid_init: job_queue init failed\n");
        JPEG_FREE(p_hybrid);
        return rc;
    }

    // Go through the engine try list according to the pref
    while (*profile_list)
    {
        int rc = JPEGERR_SUCCESS;
        jpege_engine_hybrid_profile_t *engine_profile = *profile_list;

        // Create the engine
        engine_profile->create_func(&(p_hybrid->engines[count]), p_hybrid);

        // Initialize engine
        rc = p_hybrid->engines[count].init((jpege_engine_hybrid_obj_t *)&(p_hybrid->engines[count]),
                                           (jpege_engine_hybrid_event_handler_t)jpege_engine_hybrid_event_handler,
                                           (jpege_engine_hybrid_output_handler_t)jpege_engine_hybrid_output_handler);
        if (JPEG_SUCCEEDED(rc))
        {
            JPEG_DBG_MED("jpege_engine_hybrid_init: use engine %d %s\n", count, engine_profile->engine_name);
            count++;
            p_hybrid->engine_count = count;
        }
        else
        {
            JPEG_DBG_MED("jpege_engine_hybrid_init: not to use engine %d %s\n", count, engine_profile->engine_name);
            // Failed to use the current engine, destroy it first
            if (p_hybrid->engines[count].destroy)
            {
                p_hybrid->engines[count].destroy(&(p_hybrid->engines[count]));
            }
        }
        // Move on to the next one
        profile_list++;
    }

    // Assign allocated engine structure to p_obj
    p_obj->p_engine = (void *)p_hybrid;
    p_obj->is_initialized = true;
    return JPEGERR_SUCCESS;
}

static int jpege_engine_hybrid_start(
    jpege_engine_obj_t   *p_obj,
    jpege_img_cfg_t      *p_config,
    jpege_img_data_t     *p_source,
    jpege_engine_dst_t   *p_dest)
{
    int rc = JPEGERR_SUCCESS;
    jpege_engine_hybrid_t  *p_hybrid;
    jpege_engine_job_t      *p_job;
    uint32_t                count;

    // Validate input arguments
    if (!p_obj || !p_config ||!p_source||!p_dest)
    {
        JPEG_DBG_ERROR("jpege_engine_hybrid_start : NULL input pointer\n");
        return JPEGERR_ENULLPTR;
    }

    p_config->fastCV_flag = p_obj->fastCV_flag;
    p_config->fastCV_buffer = p_obj->fastCV_buffer;

    // Allocate memory for the engine structure
    p_hybrid = (jpege_engine_hybrid_t *)p_obj->p_engine;
    if (!p_hybrid)
    {
        JPEG_DBG_ERROR("jpege_engine_hybrid_start : Allocating memory for engine structure \n");
        return JPEGERR_EMALLOC;
    }

    // prepare jobs
    // Reset job queue
    rc = jpeg_queue_reset(p_hybrid->job_queue);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_engine_hybrid_start: failed to reset job queue\n");
        return rc;
    }

    p_hybrid->job_done = 0;
    p_hybrid->job_scheduled = 0;
    p_hybrid->output_buf_received_cnt = 0;
    JPEG_DBG_LOW("jpege_engine_hybrid_start:\n");

    if (p_config->scale_cfg.enable)
    {
        // Configure the engine hybrid based on the input configuration
        rc = jpege_engine_hybrid_configure_scale(p_hybrid, p_config, p_source, p_dest);
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jpege_engine_hybrid_start: failed to configure\n");
            return rc;
        }
    }
    else
    {
        // Configure the engine hybrid based on the input configuration
        rc = jpege_engine_hybrid_configure(p_hybrid, p_config, p_source, p_dest);
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jpege_engine_hybrid_start: failed to configure\n");
            return rc;
        }
    }
    count = 0;
    os_mutex_lock(&(p_hybrid->mutex));
    while (count < p_hybrid->engine_count &&
           count < p_hybrid->job_count)
    {
        // dequeue job from job queue
        // send start encoding to engines
        rc = jpeg_queue_dequeue(p_hybrid->job_queue, (void **)&p_job, 1);
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jpege_engine_hybrid_start: failed to dequeue \n");
            os_mutex_unlock(&(p_hybrid->mutex));
            return rc;
        }

        rc = p_hybrid->engines[count].start(&(p_hybrid->engines[count]),
                                             p_config,
                                             p_source,
                                             &(p_job->engine_cfg));
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jpege_engine_hybrid_start: failed to JPEG encoding with engine %d\n", count);
            os_mutex_unlock(&(p_hybrid->mutex));
            return rc;
        }
        JPEG_FREE(p_job);
        p_job = NULL;
        count ++;
        p_hybrid->job_scheduled ++;
    }
    os_mutex_unlock(&(p_hybrid->mutex));

    p_hybrid->dest = *p_dest;
    return rc;
}

static int jpege_engine_hybrid_abort(
    jpege_engine_obj_t *p_obj)
{
    int rc = JPEGERR_SUCCESS;
    jpege_engine_hybrid_t *p_hybrid;
    uint8_t count = 0;

    // Validate input arguments
    if (!p_obj || !p_obj->p_engine)
    {
        JPEG_DBG_ERROR("jpege_engine_hybrid_abort : NULL input pointer\n");
        return JPEGERR_ENULLPTR;
    }

    // Cast p_obj->obj as jpege_engine_hybrid_t object
    p_hybrid = (jpege_engine_hybrid_t *)p_obj->p_engine;
    rc = jpeg_queue_abort(&(p_hybrid->job_queue));

    os_cond_signal(&(p_hybrid->final_output_cond));
    while (count < p_hybrid->engine_count)
    {
       p_hybrid->engines[count].abort(&(p_hybrid->engines[count]));
       count++;
    }
    return JPEGERR_SUCCESS;
}

static void jpege_engine_hybrid_destroy(
    jpege_engine_obj_t *p_obj)
{
    uint8_t count = 0;
    uint8_t i;
    if (p_obj)
    {
        jpege_engine_hybrid_t *p_hybrid = (jpege_engine_hybrid_t *)(p_obj->p_engine);


        // Release allocated memory
        if (p_hybrid)
        {
            os_mutex_lock(&(p_hybrid->mutex));
            while (count < p_hybrid->engine_count)
            {
               p_hybrid->engines[count].destroy(&(p_hybrid->engines[count]));
               count++;
            }

            // Destroy the job queue
            jpeg_queue_destroy(&(p_hybrid->job_queue));
            os_mutex_unlock(&(p_hybrid->mutex));
            // Destroy condition variable and mutex
            (void)os_mutex_destroy(&(p_hybrid->mutex));
            (void)os_cond_destroy(&(p_hybrid->cond));
            (void)os_cond_destroy(&(p_hybrid->final_output_cond));
            // Release memory of encoder
            JPEG_FREE(p_obj->p_engine);
            p_obj->p_engine = NULL;
        }
    }
}

static int jpege_engine_hybrid_check_start_param(
    jpege_img_cfg_t      *p_config,
    jpege_img_data_t     *p_source,
    jpege_engine_dst_t   *p_dest)
{
    /*------------------------------------------------------------
      Input validation
      ------------------------------------------------------------*/
    if (!p_config || !p_source || !p_dest)
    {
        JPEG_DBG_ERROR("jpege_engine_hybrid_check_start_param : NULL input pointer\n");
        return JPEGERR_ENULLPTR;
    }

    // Validate color format
    if (p_source->color_format > YCBCRLP_H1V1)
    {
        // Supports only YUV color format
        JPEG_DBG_MED("jpege_engine_hybrid_check_start_param: unsupported color format\n");
        return JPEGERR_EUNSUPPORTED;
    }

    // Validate rotation
    if (p_config->rotation_degree_clk % 90)
    {
        JPEG_DBG_ERROR("jpege_engine_hybrid_check_start_param : unsupported rotation : %d\n",p_config->rotation_degree_clk);
        return JPEGERR_EBADPARM;
    }
    // Make sure scale related settings are within range.
    // Not support upscale in one direction but downscale in the other direction.
    // padding is not required from the caller.
    if (p_config->scale_cfg.enable &&
        (!p_config->scale_cfg.input_width ||
         !p_config->scale_cfg.input_height ||
         !p_config->scale_cfg.output_width ||
         !p_config->scale_cfg.output_height ||
         (p_config->scale_cfg.input_width  > 0x1fff) ||
         (p_config->scale_cfg.input_height > 0x1fff) ||
         (p_config->scale_cfg.h_offset > 0x1fff) ||
         (p_config->scale_cfg.v_offset > 0x1fff) ||
         ((p_config->scale_cfg.h_offset + p_config->scale_cfg.input_width) > p_source->width) ||
         ((p_config->scale_cfg.v_offset + p_config->scale_cfg.input_height) > p_source->height) ||
         ((p_config->scale_cfg.input_width > p_config->scale_cfg.output_width) &&
          (p_config->scale_cfg.input_height < p_config->scale_cfg.output_height)) ||
         ((p_config->scale_cfg.input_width < p_config->scale_cfg.output_width) &&
          (p_config->scale_cfg.input_height > p_config->scale_cfg.output_height))))
    {
        JPEG_DBG_MED("jpege_engine_hybrid_check_start_param: invalid scale config: \
          (h offset, v offset)(%d, %d)  (input width, input height)(%d, %d) (output width,output height)(%d, %d)\n",
           p_config->scale_cfg.h_offset, p_config->scale_cfg.v_offset,
           p_config->scale_cfg.input_width, p_config->scale_cfg.input_height,
           p_config->scale_cfg.output_width, p_config->scale_cfg.output_height);
        return JPEGERR_EBADPARM;
    }

    return JPEGERR_SUCCESS;
}

static int jpege_engine_hybrid_configure_scale(
   jpege_engine_hybrid_t  *p_hybrid,
   jpege_img_cfg_t    *p_config,
   jpege_img_data_t   *p_source,
   jpege_engine_dst_t *p_dest)
{
    int  rc;
    jpege_engine_job_t     *p_job;

    jpeg_subsampling_t  subsampling;          // Subsampling format
    uint32_t  luma_width, luma_height, chroma_width, chroma_height, default_piece_height;
    uint32_t  sampling_h_shift, sampling_v_shift;
    uint32_t  MCU_width, MCU_height, luma_blocks;
    uint32_t  hori_MCU_count, vert_MCU_count, total_MCU_count;

    uint32_t  piece_height, remaining_height, current_height;
    uint32_t  src_index;

     // Configure rotations
    luma_width = p_config->scale_cfg.output_width;
    luma_height = p_config->scale_cfg.output_height;
    subsampling = (jpeg_subsampling_t)((uint8_t)p_source->color_format / 2);

    /*-----------------------------------------------------------------------
      Calculate Chroma Width and Height
    -----------------------------------------------------------------------*/
    switch (subsampling)
    {
    case JPEG_H1V1:
    {
        chroma_width = luma_width;
        chroma_height = luma_height;
        MCU_width = 8;
        MCU_height = 8;
        luma_blocks = H1V1_NUM_LUMA_BLOCKS;
        sampling_h_shift = 0;
        sampling_v_shift = 0;
        break;
    }
    case JPEG_H1V2:
    {
        chroma_width = luma_width;
        chroma_height = (luma_height + 1) >> 1; // odd luma support
        MCU_width = 8;
        MCU_height = 16;
        luma_blocks = H1V2_NUM_LUMA_BLOCKS;
        sampling_h_shift = 0;
        sampling_v_shift = 1;
        break;
    }
    case JPEG_H2V1:
    case JPEG_YUY2:
    case JPEG_UYVY:
    {
        chroma_width = (luma_width + 1) >> 1; // odd luma support
        chroma_height = luma_height;
        MCU_width = 16;
        MCU_height = 8;
        luma_blocks = H2V1_NUM_LUMA_BLOCKS;
        sampling_h_shift = 1;
        sampling_v_shift = 0;
        break;
    }
    case JPEG_H2V2:
    {
        chroma_width = (luma_width + 1) >> 1; // odd luma support
        chroma_height = (luma_height + 1) >> 1; // odd luma support
        MCU_width = 16;
        MCU_height = 16;
        luma_blocks = H2V2_NUM_LUMA_BLOCKS;
        sampling_h_shift = 1;
        sampling_v_shift = 1;
        break;
    }
    default:
        JPEG_DBG_ERROR("jpege_engine_hybrid_configure: invalid jpeg subsampling: %d\n", subsampling);
        return JPEGERR_EBADPARM;
    }

    if(!(p_config->fastCV_flag))
    {
        // Configure rotations
        switch (p_config->rotation_degree_clk)
        {
        case 0:
        case 180:
            break;
        case 90:
        case 270:
        {
            JPEG_SWAP(&luma_width, &luma_height);
            JPEG_SWAP(&chroma_width, &chroma_height);
            JPEG_SWAP(&MCU_width, &MCU_height);
            JPEG_SWAP(&sampling_h_shift, &sampling_v_shift);
            break;
        }
        default:
            JPEG_DBG_ERROR("jpege_engine_hybrid_configure: unrecognized rotation: %d\n", p_config->rotation_degree_clk);
            return JPEGERR_EBADPARM;
        }
    }

    p_hybrid->dest = *p_dest;

    p_hybrid->job_count = 0;
    p_hybrid->output_buf_cnt = 0;

    /*-----------------------------------------------------------------------
     1. Calculate the horizontal and vertical MCU counts
     2. Calculate restart intervals
    -----------------------------------------------------------------------*/
    // divide by 8 calculate the amounts of MCUs
    hori_MCU_count = (chroma_width + 7) >> 3;  // with padding support
    p_config->restart_interval = hori_MCU_count;

    /*-----------------------------------------------------------------------
     Configure job
     1. Pass the source information to engines
     2. Calculate the source start offset
     3. Pass the piece index
     4. Calculate output MCUs
     5. Calculate restart markers
    -----------------------------------------------------------------------*/
    src_index = 0;
    piece_height = 0;
    current_height = 0;
    remaining_height = luma_height;

    // Piece height has to be a multiple of 48 for scaling cases.
    if(luma_height/48 <= 8)
    {
      default_piece_height = 48*2;
    }
    else if (luma_height/48 <= 12)
    {
      default_piece_height = 48*3;
    }
    else if (luma_height/48 <= 16)
    {
      default_piece_height = 48*4;
    }
    else if (luma_height/48 <= 24)
    {
      default_piece_height = 48*6;
    }
    else if (luma_height/48 <= 32)
    {
      default_piece_height = 48*8;
    }
    else if (luma_height/48 <= 48)
    {
      default_piece_height = 48*12;
    }
    else
    {
      // Max number of elements in the queue = 16. Hence max height supported = 16*48*16 = 12288
      default_piece_height = 48*16;
    }

    while (remaining_height > 0)
    {
        p_job = (jpege_engine_job_t *)JPEG_MALLOC(sizeof(jpege_engine_job_t));
        if (!p_job)
        {
            JPEG_DBG_ERROR("jpege_engine_hybrid_configure_scale : Memory allocation for an engine job failed\n");
            return JPEGERR_EMALLOC;
        }

        // Job configurations
        p_job->engine_cfg.color_format = p_source->color_format;
        p_job->engine_cfg.input_width = p_source->width;
        p_job->engine_cfg.input_height = p_source->height;
        p_job->engine_cfg.luma_ptr = p_source->p_fragments[0].color.yuv.luma_buf->ptr;
        p_job->engine_cfg.chroma_ptr = p_source->p_fragments[0].color.yuv.chroma_buf->ptr;

        if((p_config->fastCV_flag))
        {
            p_job->engine_cfg.scale_cfg.fastCV_enable = 1;
            p_job->engine_cfg.scale_cfg.fastCV_buffer = p_config->fastCV_buffer;
        }
        else
        {
            p_job->engine_cfg.scale_cfg.fastCV_enable = 0;
        }

        p_job->engine_cfg.rotation = p_config->rotation_degree_clk;
        p_job->engine_cfg.restart_interval = p_config->restart_interval;
        p_job->engine_cfg.scale_cfg.enable = p_config->scale_cfg.enable;
        p_job->engine_cfg.scale_cfg.input_width = p_config->scale_cfg.input_width;
        p_job->engine_cfg.scale_cfg.input_height = p_config->scale_cfg.input_height;
        p_job->engine_cfg.scale_cfg.output_width = p_config->scale_cfg.output_width;
        p_job->engine_cfg.scale_cfg.output_height = p_config->scale_cfg.output_height;
        p_job->engine_cfg.scale_cfg.h_offset = p_config->scale_cfg.h_offset;
        p_job->engine_cfg.scale_cfg.v_offset = p_config->scale_cfg.v_offset;

        p_job->engine_cfg.luma_dc_huff_tbl = p_config->luma_dc_huff_tbl;
        p_job->engine_cfg.luma_ac_huff_tbl = p_config->luma_ac_huff_tbl;
        p_job->engine_cfg.chroma_dc_huff_tbl = p_config->chroma_dc_huff_tbl;
        p_job->engine_cfg.chroma_ac_huff_tbl = p_config->chroma_ac_huff_tbl;
        p_job->engine_cfg.luma_qtbl = (uint16_t *)p_config->luma_quant_tbl;
        p_job->engine_cfg.chroma_qtbl = (uint16_t *)p_config->chroma_quant_tbl;
        p_job->engine_cfg.src_index = src_index;

        // Configure start offset with rotations
        p_job->engine_cfg.start_hori_MCU_index = 0;
        p_job->engine_cfg.start_vert_MCU_index = (uint32_t)((current_height + MCU_height - 1)/ MCU_height);

        piece_height = STD_MIN(remaining_height, default_piece_height);
        remaining_height -= piece_height;

        p_job->engine_cfg.output_MCUs = ((uint32_t)(piece_height + MCU_height - 1)/ MCU_height) * hori_MCU_count;
        // todo: add output estimation
        p_hybrid->output_buf_size = 1.5 * p_job->engine_cfg.output_MCUs * MCU_height * MCU_width;

        // calculate total MCUs
        vert_MCU_count = (current_height + MCU_height - 1) / MCU_height; // with padding support
        total_MCU_count = hori_MCU_count * vert_MCU_count;

        if (total_MCU_count == 0)
        {
            p_job->engine_cfg.base_restart_marker = 0;
            p_job->engine_cfg.restart_mcu_count = 0;
        }
        else
        {
            p_job->engine_cfg.base_restart_marker = ((total_MCU_count / p_config->restart_interval - 1) % MAX_RESTART_MARKER_COUNT);
            p_job->engine_cfg.restart_mcu_count = hori_MCU_count;
        }

        // Initialize the output of piece-wise image if fastCV not enabled
        if(!(p_config->fastCV_flag)) {
            if (jpeg_buffer_init(&p_hybrid->p_output_buf[src_index]))
            {
                JPEG_FREE(p_job);
                JPEG_DBG_ERROR("jpege_hybrid_get_output_buffer: Init buffer failed\n");
                return JPEGERR_EFAILED;
            }

            // Allocate buffers to hold the output of piece-wise image
            if (jpeg_buffer_allocate(p_hybrid->p_output_buf[src_index], p_hybrid->output_buf_size, 0))
            {
                JPEG_DBG_ERROR("jpege_hybrid_get_output_buffer: Allocation buffer failed\n");
                jpeg_buffer_destroy(&p_hybrid->p_output_buf[src_index]);
                JPEG_FREE(p_job);
                return JPEGERR_EFAILED;
            }

            p_job->engine_cfg.output_buffer_ptr = p_hybrid->p_output_buf[src_index]->ptr +
                p_hybrid->p_output_buf[src_index]->start_offset + p_hybrid->p_output_buf[src_index]->offset;
            p_job->engine_cfg.output_buffer_length = p_hybrid->p_output_buf[src_index]->size -
                p_hybrid->p_output_buf[src_index]->start_offset - p_hybrid->p_output_buf[src_index]->offset;
        }

        p_job->engine_cfg.scale_cfg.vert_luma_row_index = current_height;
        p_job->engine_cfg.scale_cfg.vert_luma_output_height = current_height + piece_height;
        p_job->engine_cfg.scale_cfg.vert_chroma_row_index = current_height >> sampling_v_shift;
        p_job->engine_cfg.scale_cfg.vert_chroma_output_height = (current_height + piece_height) >> sampling_v_shift;
        p_job->engine_cfg.scale_cfg.piece_height = piece_height;

        current_height += piece_height;
        src_index ++;

        p_job->p_source = p_source;
        p_job->p_config = p_config;

        rc = jpeg_queue_enqueue(p_hybrid->job_queue, (void **)&p_job, 1);
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jpege_engine_hybrid_configure_scale: failed to enqueue \n");
            JPEG_FREE(p_job);
            return rc;
        }
        p_hybrid->job_count ++;
        p_hybrid->output_buf_cnt ++;
    }
    return JPEGERR_SUCCESS;
}

static int jpege_engine_hybrid_configure(
   jpege_engine_hybrid_t  *p_hybrid,
   jpege_img_cfg_t    *p_config,
   jpege_img_data_t   *p_source,
   jpege_engine_dst_t *p_dest)
{
    jpeg_subsampling_t  subsampling;          // Subsampling format
    uint32_t  luma_width, luma_height, chroma_width, chroma_height, default_piece_height;
    uint32_t  sampling_h_shift, sampling_v_shift;
    uint32_t  MCU_width, MCU_height, luma_blocks;
    uint32_t  hori_MCU_count, vert_MCU_count, total_MCU_count;

    uint32_t  piece_height, remaining_height, current_height;
    uint32_t  src_index;
    int       rc;
    jpege_engine_job_t     *p_job;

    luma_width = p_source->width;
    luma_height = p_source->height;
    subsampling = (jpeg_subsampling_t)((uint8_t)p_source->color_format / 2);

    /*-----------------------------------------------------------------------
      Calculate Chroma Width and Height
    -----------------------------------------------------------------------*/
    switch (subsampling)
    {
    case JPEG_H1V1:
    {
        chroma_width = luma_width;
        chroma_height = luma_height;
        MCU_width = 8;
        MCU_height = 8;
        luma_blocks = H1V1_NUM_LUMA_BLOCKS;
        sampling_h_shift = 0;
        sampling_v_shift = 0;
        break;
    }
    case JPEG_H1V2:
    {
        chroma_width = luma_width;
        chroma_height = (luma_height + 1) >> 1; // odd luma support
        MCU_width = 8;
        MCU_height = 16;
        luma_blocks = H1V2_NUM_LUMA_BLOCKS;
        sampling_h_shift = 0;
        sampling_v_shift = 1;
        break;
    }
    case JPEG_H2V1:
    case JPEG_YUY2:
    case JPEG_UYVY:
    {
        chroma_width = (luma_width + 1) >> 1; // odd luma support
        chroma_height = luma_height;
        MCU_width = 16;
        MCU_height = 8;
        luma_blocks = H2V1_NUM_LUMA_BLOCKS;
        sampling_h_shift = 1;
        sampling_v_shift = 0;
        break;
    }
    case JPEG_H2V2:
    {
        chroma_width = (luma_width + 1) >> 1; // odd luma support
        chroma_height = (luma_height + 1) >> 1; // odd luma support
        MCU_width = 16;
        MCU_height = 16;
        luma_blocks = H2V2_NUM_LUMA_BLOCKS;
        sampling_h_shift = 1;
        sampling_v_shift = 1;
        break;
    }
    default:
        JPEG_DBG_ERROR("jpege_engine_hybrid_configure: invalid jpeg subsampling: %d\n", subsampling);
        return JPEGERR_EBADPARM;
    }

    // Configure rotations
    switch (p_config->rotation_degree_clk)
    {
    case 0:
    case 180:
        break;
    case 90:
    case 270:
    {
        JPEG_SWAP(&luma_width, &luma_height);
        JPEG_SWAP(&chroma_width, &chroma_height);
        JPEG_SWAP(&MCU_width, &MCU_height);
        JPEG_SWAP(&sampling_h_shift, &sampling_v_shift);
        break;
    }
    default:
        JPEG_DBG_ERROR("jpege_engine_hybrid_configure: unrecognized rotation: %d\n", p_config->rotation_degree_clk);
        return JPEGERR_EBADPARM;
    }

    p_hybrid->dest = *p_dest;
    /*-----------------------------------------------------------------------
     1. Calculate the horizontal and vertical MCU counts
     2. Calculate restart intervals
    -----------------------------------------------------------------------*/
    // divide by 8 calculate the amounts of MCUs
    hori_MCU_count = (chroma_width + 7) >> 3;  // with padding support
    p_config->restart_interval = hori_MCU_count;

    /*-----------------------------------------------------------------------
     Configure job
     1. Pass the source information to engines
     2. Calculate the source start offset
     3. Pass the piece index
     4. Calculate output MCUs
     5. Calculate restart markers
    -----------------------------------------------------------------------*/
    src_index = 0;
    piece_height = 0;
    current_height = 0;
    remaining_height = luma_height;
    p_hybrid->job_count = 0;
    p_hybrid->output_buf_cnt = 0;

    // Piece height has to be a multiple of 48 for scaling cases.
    if (luma_height/16 <= 12)
    {
      default_piece_height = 16*3;
    }
    else if (luma_height/16 <= 24)
    {
      default_piece_height = 16*6;
    }
    else if (luma_height/16 <= 32)
    {
      default_piece_height = 16*8;
    }
    else if (luma_height/16 <= 48)
    {
      default_piece_height = 16*12;
    }
    else if (luma_height/16 <= 72)
    {
      default_piece_height = 16*16;
    }
    else if (luma_height/16 <= 96)
    {
      default_piece_height = 16*24;
    }
    else if (luma_height/16 <= 144)
    {
      default_piece_height = 16*36;
    }
    else if (luma_height/16 <= 144)
    {
      default_piece_height = 16*36;
    }
    else if (luma_height/16 <= 192)
    {
      default_piece_height = 16*48;
    }
    else
    {
      // Max number of elements in the queue = 16. Hence max height supported = 16*16*72 = 18432
      default_piece_height = 16*72;
    }

    while (remaining_height > 0)
    {
        p_job = (jpege_engine_job_t *)JPEG_MALLOC(sizeof(jpege_engine_job_t));
        if (!p_job)
        {
            JPEG_DBG_ERROR("jpege_engine_hybrid_configure : Memory allocation for an engine job failed\n");
            return JPEGERR_EMALLOC;
        }

       // Job configurations
        p_job->engine_cfg.color_format = p_source->color_format;
        p_job->engine_cfg.input_width = p_source->width;
        p_job->engine_cfg.input_height = p_source->height;
        p_job->engine_cfg.luma_ptr = p_source->p_fragments[0].color.yuv.luma_buf->ptr;
        p_job->engine_cfg.chroma_ptr = p_source->p_fragments[0].color.yuv.chroma_buf->ptr;

        p_job->engine_cfg.rotation = p_config->rotation_degree_clk;
        p_job->engine_cfg.restart_interval = p_config->restart_interval;
        p_job->engine_cfg.scale_cfg.enable = false;
        p_job->engine_cfg.luma_dc_huff_tbl = p_config->luma_dc_huff_tbl;
        p_job->engine_cfg.luma_ac_huff_tbl = p_config->luma_ac_huff_tbl;
        p_job->engine_cfg.chroma_dc_huff_tbl = p_config->chroma_dc_huff_tbl;
        p_job->engine_cfg.chroma_ac_huff_tbl = p_config->chroma_ac_huff_tbl;
        p_job->engine_cfg.luma_qtbl = (uint16_t *)p_config->luma_quant_tbl;
        p_job->engine_cfg.chroma_qtbl = (uint16_t *)p_config->chroma_quant_tbl;

        p_job->engine_cfg.src_index = src_index;

        piece_height = STD_MIN(remaining_height, default_piece_height);
        remaining_height -= piece_height;
        p_job->engine_cfg.quality = p_config->quality;

        // Configure start offset with rotations
        p_job->engine_cfg.start_hori_MCU_index = 0;
        p_job->engine_cfg.start_vert_MCU_index = (uint32_t)((current_height + MCU_height - 1)/ MCU_height);

        p_job->engine_cfg.output_MCUs = ((uint32_t)(piece_height + MCU_height - 1)/ MCU_height) * hori_MCU_count;

        // todo: outupt buffer estimation
        p_hybrid->output_buf_size = 1.5 * p_job->engine_cfg.output_MCUs * MCU_height * MCU_width;

        // calculate total MCUs
        vert_MCU_count = (current_height + MCU_height - 1) / MCU_height; // with padding support
        total_MCU_count = hori_MCU_count * vert_MCU_count;

        if (total_MCU_count == 0)
        {
            p_job->engine_cfg.base_restart_marker = 0;
            p_job->engine_cfg.restart_mcu_count = 0;
        }
        else
        {
            p_job->engine_cfg.base_restart_marker = ((total_MCU_count / p_config->restart_interval - 1) % MAX_RESTART_MARKER_COUNT);
            p_job->engine_cfg.restart_mcu_count = hori_MCU_count;
        }

        // Initialize the output of piece-wise image
        if (jpeg_buffer_init(&p_hybrid->p_output_buf[src_index]))
        {
            JPEG_DBG_ERROR("jpege_hybrid_get_output_buffer: Init buffer failed\n");
            JPEG_FREE(p_job);
        return JPEGERR_EFAILED;
        }

        // Allocate new buffer to hold the output of piece-wise image
        if (jpeg_buffer_allocate(p_hybrid->p_output_buf[src_index], p_hybrid->output_buf_size, 0))
        {
            JPEG_DBG_ERROR("jpege_hybrid_get_output_buffer: Allocation buffer failed\n");
            jpeg_buffer_destroy(&p_hybrid->p_output_buf[src_index]);
            JPEG_FREE(p_job);
            return JPEGERR_EFAILED;
        }

        p_job->engine_cfg.output_buffer_ptr = p_hybrid->p_output_buf[src_index]->ptr + p_hybrid->p_output_buf[src_index]->start_offset+p_hybrid->p_output_buf[src_index]->offset;
        p_job->engine_cfg.output_buffer_length = p_hybrid->p_output_buf[src_index]->size  - p_hybrid->p_output_buf[src_index]->offset;
        p_job->p_source = p_source;
        p_job->p_config = p_config;

        current_height += piece_height;
        src_index ++;
        rc = jpeg_queue_enqueue(p_hybrid->job_queue, (void **)&p_job, 1);
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jpege_engine_hybrid_configure: failed to enqueue \n");
            JPEG_FREE(p_job);
            return rc;
        }

        p_hybrid->job_count ++;
        p_hybrid->output_buf_cnt ++;
    }
    return JPEGERR_SUCCESS;
}

static int jpege_engine_hybrid_event_handler(
    jpege_engine_hybrid_obj_t *p_obj,  // The encode engine object calling it
    jpeg_event_t               event,  // The event code
    void                      *p_arg)  // An optional argument for that event.
{
    int rc = JPEGERR_SUCCESS;
    jpege_engine_hybrid_t *p_hybrid;
    jpege_engine_job_t      *p_job;
    (void *)p_arg;
    (jpeg_event_t)event;

    if (!p_obj)
    {
        JPEG_DBG_ERROR("jpege_engine_hybrid_event_handler: p_obj NULL!\n");
        return JPEGERR_ENULLPTR;
    }

    // Cast object
    p_hybrid = p_obj->hybrid;

    os_mutex_lock(&(p_hybrid->mutex));

    // Check the abort status
    if (p_hybrid->abort_flag)
    {
        JPEG_DBG_ERROR("jpege_engine_hybrid_event_handler: abort start\n");
        os_mutex_unlock(&(p_hybrid->mutex));
        return JPEGERR_EFAILED;
    }

    switch (event)
    {
    case JPEG_EVENT_DONE:
        p_hybrid->job_done ++;
        if (p_hybrid->job_done == p_hybrid->job_count)
        {
            JPEG_DBG_ERROR("jpege_engine_hybrid_event_handler: job done\n");
            p_hybrid->p_handler(p_hybrid->p_wrapper, JPEG_EVENT_DONE, NULL);
            os_mutex_unlock(&(p_hybrid->mutex));
            return rc;
        }
        else if (p_hybrid->job_scheduled < p_hybrid->job_count)
        {
            // dequeue job from job queue
            // send start encoding to engines
            rc = jpeg_queue_dequeue(p_hybrid->job_queue, (void **)&p_job, 1);
            if (JPEG_FAILED(rc))
            {
                JPEG_DBG_ERROR("jpege_engine_hybrid_event_handler: failed to dequeue \n");
                os_mutex_unlock(&(p_hybrid->mutex));
                return rc;
            }

            // Destroy thread before creating another in start()
            p_obj->thread_destroy(p_obj);

            rc = p_obj->start(p_obj, p_job->p_config, p_job->p_source, &(p_job->engine_cfg));
            if (JPEG_FAILED(rc))
            {
                JPEG_DBG_ERROR("jpege_engine_hybrid_event_handler: failed to start \n");
                os_mutex_unlock(&(p_hybrid->mutex));
                return rc;
            }
            JPEG_FREE(p_job);
            p_job = NULL;
            p_hybrid->job_scheduled ++;
            break;
        }
        break;

    case JPEG_EVENT_WARNING:
    case JPEG_EVENT_ERROR:
    case JPEG_EVENT_ABORTED:
        JPEG_DBG_ERROR("jpege_engine_hybrid_event_handler: error event!\n");
        p_hybrid->p_handler(p_hybrid->p_wrapper, event, NULL);
        break;

    default:
        JPEG_DBG_ERROR("jpege_engine_hybrid_event_handler: invalid event\n");
        os_mutex_unlock(&(p_hybrid->mutex));
        return JPEGERR_EBADPARM;
    }

    os_mutex_unlock(&(p_hybrid->mutex));

    return rc;
}

static int jpege_engine_hybrid_output_handler(
    jpege_engine_hybrid_obj_t  *p_obj,    // The encode engine object calling it
    uint8_t             *buf_ptr,    // The buffer containing the output bitstream_t
    uint32_t             buf_offset) // The flag to indicate last output buffer
{
    jpege_engine_hybrid_t *p_hybrid;
    int                    rc = JPEGERR_SUCCESS;
    uint8_t                i = 0, index = 0;
    jpeg_buf_t            *p_dest_buf;
    uint32_t               bytes_to_copy, bytes_remaining, bytes_written;

    if (!p_obj)
    {
        JPEG_DBG_ERROR("jpege_engine_hybrid_output_handler: p_obj NULL!\n");
        return JPEGERR_EFAILED;
    }

    // Cast p_obj->obj as jpege_engine_hybrid_tobject
    p_hybrid = (jpege_engine_hybrid_t *)p_obj->hybrid;

    os_mutex_lock(&(p_hybrid->mutex));

    // Check the abort status
    if (p_hybrid->abort_flag)
    {
        JPEG_DBG_ERROR("jpege_engine_hybrid_output_handler: abort start\n");
        os_mutex_unlock(&(p_hybrid->mutex));
        return JPEGERR_EFAILED;
    }

    p_dest_buf = p_hybrid->p_dest_buf;

    // Verify the received output buffer
    while (i < p_hybrid->output_buf_cnt)
    {
        if (!p_hybrid->output_buf_received[i])
        {
            if (buf_ptr == p_hybrid->p_output_buf[i]->ptr + p_hybrid->p_output_buf[i]->start_offset)
            {
               p_hybrid->output_buf_received[i] = true;
               p_hybrid->p_output_buf[i]->offset = buf_offset;
               p_hybrid->output_buf_received_cnt ++;
               break;
            }
        }
        i++;
    }

    // If received all the output from engines under
    // Copy the internal output and forward to upper layer
    if (p_hybrid->output_buf_received_cnt == p_hybrid->output_buf_cnt)
    {
        i = 0;
        while (i < p_hybrid->output_buf_cnt)
        {
            bytes_remaining = p_hybrid->p_output_buf[i]->offset;
            bytes_written = 0;
            while (bytes_remaining)
            {
                if (!p_dest_buf)
                {
                    rc = p_hybrid->dest.p_get_output_buffer_handler(p_hybrid->p_wrapper, &p_dest_buf);
                    if (JPEG_FAILED(rc) || !p_dest_buf)
                    {
                        JPEG_DBG_ERROR("jpege_engine_hybrid_output_handler: failed to get next output buffer\n");
                        p_hybrid->error_flag = 1;
                        os_mutex_unlock(&(p_hybrid->mutex));
                        JPEG_DBG_LOW("jpege_engine_hybrid_output_handler: set error flag, return.\n");
                        return rc;
                    }
                }
                // Calculate how much can be written to the destination buffer
                bytes_to_copy = STD_MIN(bytes_remaining, (p_dest_buf->size - p_dest_buf->offset));
                // Copy the data from bitstream buffer to destination buffer starting from the offset
                (void)STD_MEMMOVE(p_dest_buf->ptr + p_dest_buf->start_offset + p_dest_buf->offset,p_hybrid->p_output_buf[i]->ptr + p_hybrid->p_output_buf[i]->start_offset + bytes_written, bytes_to_copy);

                // Update destination buffer offset and other state variables
                p_dest_buf->offset += bytes_to_copy;
                bytes_remaining -= bytes_to_copy;
                bytes_written += bytes_to_copy;

                // After copying, if last output or the destination buffer is full, deliver it
                if (i == (p_hybrid->output_buf_cnt-1) || p_dest_buf->offset == p_dest_buf->size)
                {
                    // Check to see whether it is the final output
                    if (i == (p_hybrid->output_buf_cnt-1) && !bytes_remaining)
                    {
                        // Emit EOI
                        *(p_dest_buf->ptr + p_dest_buf->offset)     = 0xFF;
                        *(p_dest_buf->ptr + p_dest_buf->offset + 1) = 0xD9;
                        p_dest_buf->offset += 2;

                        // Set the final output flag:
                        // So this output can set is_last_buffer flag in flush output function,
                        // if final_output_flag is set
                        p_hybrid->final_output_flag = true;
                    }

                   // Flush the current destination buffer to upper layer
                    rc = p_hybrid->dest.p_output_handler(p_hybrid->p_wrapper,
                                        p_dest_buf,
                                        p_hybrid->final_output_flag);
                    if (JPEG_FAILED(rc))
                    {
                        JPEG_DBG_ERROR("jpege_engine_hybrid_output_handlers: failed to flush output buffer\n");
                        p_hybrid->error_flag = 1;
                        os_mutex_unlock(&(p_hybrid->mutex));
                        JPEG_DBG_ERROR("jpege_engine_hybrid_output_handler: set error flag, return.\n");
                        return rc;
                    }
                    // Clean up the local output buffer ref
                    p_hybrid->p_dest_buf = NULL;
                    p_dest_buf = NULL;
                }
            }
            i++;
        } // end of buffer copy

        index = 0;
        // Clean up output buffer
        while (index < p_hybrid->output_buf_cnt)
        {
            jpeg_buffer_destroy(&p_hybrid->p_output_buf[index]);
            index++;
        }

    } // end of buffer flush

    os_mutex_unlock(&(p_hybrid->mutex));
    return rc;
}
