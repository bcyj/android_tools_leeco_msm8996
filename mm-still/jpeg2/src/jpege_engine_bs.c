/*========================================================================


*//** @file jpege_engine_bs.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2009-2011,2014 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/still/jpeg2/v2/latest/src/jpege_engine_bs.c#1 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
05/31/11   staceyw Fixing compiling warnings.
08/10/10   zhiminl Fixed quant tables in case of odd rotation only with
                   padding in BS engine.
07/13/10   zhiminl Integrated with PPF to support rotation and scaling.
06/18/10   staceyw Update upsampler_cfg to scale_cfg while bitstream engine
                   does not support upscale or downscale.
05/18/10   vma     Rely on os_thread_detach for thread detachment rather
                   than joining
04/26/10   sv      Added thread name and priority in thread create API of jpeg OSAL.
04/16/10   staceyw Added flag to indicate output buffer whether is last
                   output buffer during output.
02/16/10   staceyw Provide support of get next output buffer from buffer(s)
                   queue. No more internal output buffers, and delete internal
                   get next buffer function. Add handling to fix syncronization
                   with last buffer for encode and output two different threads.
02/08/10   staceyw Added bitstream engine to support bitstream input and
11/11/09   staceyw Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "jpeg_buffer_private.h"
#include "jpege_engine_bs.h"
#include "jpege.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include "jpeg_common_private.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* Function prototypes of jpege_engine_obj_t interface functions */
static void jpege_engine_bs_create  (jpege_engine_obj_t *, jpege_obj_t);
static int  jpege_engine_bs_init    (jpege_engine_obj_t *,
                                     jpege_engine_event_handler_t);
static int  jpege_engine_bs_start   (jpege_engine_obj_t *, jpege_img_cfg_t *,
                                     jpege_img_data_t *,
                                     jpege_engine_dst_t *);
static int  jpege_engine_bs_configure(jpege_engine_bs_t *,
                                      jpege_img_cfg_t *,
                                      jpege_img_data_t *,
                                      jpege_engine_dst_t *);
static int  jpege_engine_bs_abort   (jpege_engine_obj_t *);
static void jpege_engine_bs_destroy (jpege_engine_obj_t *);
static int  jpege_engine_bs_check_start_param (jpege_img_cfg_t *,
                                               jpege_img_data_t *,
                                               jpege_engine_dst_t *);

/* Function prototypes of helper functions */
static jpeg_event_t jpege_engine_bs_copy(jpege_engine_bs_t *);
static int  jpege_engine_bs_validate_padding(jpege_img_data_t *);
static int  jpege_engine_bs_flush_output(jpege_engine_bs_t *);
#if 0 //No PPF
static jpeg_event_t jpege_engine_bs_ppf (jpege_engine_bs_t *);

static int  jpege_engine_bs_ppf_get_output_buffer(void *,
                                                  jpeg_buffer_t *);
static int  jpege_engine_bs_ppf_output_handler(void *, jpeg_buffer_t,
                                               void *, uint8_t);
#endif
/* Function prototypes of thread functions */
static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
            jpege_engine_bs_thread(OS_THREAD_FUNC_ARG_T);
static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
            jpege_engine_bs_output_thread(OS_THREAD_FUNC_ARG_T);

/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
static const char jpege_engine_bs_name[] = "Jpeg Bitstream Encode Engine";
jpege_engine_profile_t jpege_engine_bs_profile = {jpege_engine_bs_create, jpege_engine_bs_name};


static const char jpeg_engine_bs_output_thread_name[] = "jpege_engine_bs_output_thread";
static const char jpeg_engine_bs_encode_thread_name[] = "jpege_engine_bs_encode";

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
**                          Function Definitions
** ======================================================================= */
static void jpege_engine_bs_create(jpege_engine_obj_t *p_obj, jpege_obj_t encoder)
{
    if (p_obj)
    {
        // Destroy previous engine if it exists
        if (p_obj->destroy)
        {
            p_obj->destroy(p_obj);
        }
        p_obj->create            = &jpege_engine_bs_create;
        p_obj->init              = &jpege_engine_bs_init;
        p_obj->check_start_param = &jpege_engine_bs_check_start_param;
        p_obj->start             = &jpege_engine_bs_start;
        p_obj->abort             = &jpege_engine_bs_abort;
        p_obj->destroy           = &jpege_engine_bs_destroy;
        p_obj->p_engine          = NULL;
        p_obj->encoder           = encoder;
        p_obj->is_initialized    = false;
    }
}

static int jpege_engine_bs_init(
    jpege_engine_obj_t           *p_obj,
    jpege_engine_event_handler_t  p_handler)
{
    jpege_engine_bs_t *p_engine;
    int                rc;

    // Validate input arguments
    if (!p_obj || !p_handler)
        return JPEGERR_ENULLPTR;

    // Allocate memory for the engine structure
    p_engine = (jpege_engine_bs_t *)JPEG_MALLOC(sizeof(jpege_engine_bs_t));
    if (!p_engine)
        return JPEGERR_EMALLOC;

    // Initialize the fields inside the engine structure below
    STD_ZEROAT(p_engine);// Zero out the entire structure
    p_engine->p_wrapper = p_obj;                         // Initialize the pointer to the wrapper
    p_engine->p_handler = p_handler;                     // Initialize the event handler
    (void)os_mutex_init(&(p_engine->mutex));             // Initialize the mutex
    (void)os_cond_init(&(p_engine->cond));               // Initialize the condition variables
    (void)os_cond_init(&(p_engine->output_cond));
    // Initialize the condition used for output thread consume
    (void)os_cond_init(&(p_engine->consume_cond));
    // Initialize the condition used for last buffer output
    (void)os_cond_init(&(p_engine->final_output_cond));

#if 0 // No PPF
    // Create PPF
    rc = ppf_create(&(p_engine->ppf), NULL,
                    &jpege_engine_bs_ppf_output_handler,
                    &jpege_engine_bs_ppf_get_output_buffer,
                    (const void *)p_engine);
#endif
    // Create output thread
    if (os_thread_create_ex(&(p_engine->output_thread),
                            jpege_engine_bs_output_thread,
                            (void *)p_engine,
                            OS_THREAD_DEFAULT_PRIORITY,
                            (uint8_t*)jpeg_engine_bs_output_thread_name))
    {
        // Failed to create PPF or output thread, clean up and exit
        (void)os_mutex_destroy(&(p_engine->mutex));
        (void)os_cond_destroy(&(p_engine->final_output_cond));
        (void)os_cond_destroy(&(p_engine->cond));
        (void)os_cond_destroy(&(p_engine->consume_cond));
        (void)os_cond_destroy(&(p_engine->output_cond));
#if 0 // No PPF
        ppf_destroy(&(p_engine->ppf));
#endif
        JPEG_FREE(p_obj->p_engine);
        p_obj->p_engine = NULL;
        return JPEGERR_EFAILED;
    }

    // Assign allocated engine structure to p_obj
    p_obj->p_engine = (void *)p_engine;
    p_obj->is_initialized = true;
    return JPEGERR_SUCCESS;
}

static int jpege_engine_bs_check_start_param(
    jpege_img_cfg_t      *p_config,
    jpege_img_data_t     *p_source,
    jpege_engine_dst_t   *p_dest)
{
    // Validate input pointers
    if (!p_config || !p_source || !p_dest)
        return JPEGERR_ENULLPTR;

    // Temporary workaround for BS engine to notify service whether quant tables
    // need to be transposed or not
    p_dest->transposed = false;

    // Validate color format
    if ((p_source->color_format < JPEG_BITSTREAM_H2V2) ||
        (p_source->color_format > JPEG_BITSTREAM_H1V1))
    {
        JPEG_DBG_MED("jpege_engine_bs_check_start_param: unsupported color format\n");
        return JPEGERR_EUNSUPPORTED;
    }

    // Validate rotation degree
    if (p_config->rotation_degree_clk % 90)
    {
        JPEG_DBG_ERROR("jpege_engine_bs_check_start_param: invalid rotation degree: %d\n",
                       p_config->rotation_degree_clk);
        return JPEGERR_EBADPARM;
    }

    // Make sure scaling related settings are within range,
    // padding is not required from the caller
    if (p_config->scale_cfg.enable &&
        (!p_config->scale_cfg.input_width ||
         !p_config->scale_cfg.input_height ||
         !p_config->scale_cfg.output_width ||
         !p_config->scale_cfg.output_height ||
         (p_config->scale_cfg.input_width  > 0xfff) ||
         (p_config->scale_cfg.input_height > 0xfff) ||
         (p_config->scale_cfg.h_offset > 0xfff) ||
         (p_config->scale_cfg.v_offset > 0xfff) ||
         ((p_config->scale_cfg.h_offset + p_config->scale_cfg.input_width) > p_source->width) ||
         ((p_config->scale_cfg.v_offset + p_config->scale_cfg.input_height) > p_source->height) ||
         ((p_config->scale_cfg.input_width > p_config->scale_cfg.output_width) &&
          (p_config->scale_cfg.input_height < p_config->scale_cfg.output_height)) ||
         ((p_config->scale_cfg.input_width < p_config->scale_cfg.output_width) &&
          (p_config->scale_cfg.input_height > p_config->scale_cfg.output_height))))
    {
        JPEG_DBG_ERROR("jpege_engine_bs_check_start_param: invalid scaling config\n");
        return JPEGERR_EBADPARM;
    }
    // Validate fragment
    if (p_source->fragment_cnt > 1)
    {
        JPEG_DBG_ERROR("jpege_engine_bs_check_start_param: unsupported fragment: %d\n", p_source->fragment_cnt);
        return JPEGERR_EBADPARM;
    }
    // Temporary workaround for BS engine to notify service whether quant tables
    // need to be transposed or not
    if (((p_config->rotation_degree_clk == 90) || (p_config->rotation_degree_clk == 270)) &&
        !(p_config->scale_cfg.enable) && (JPEG_SUCCEEDED(jpege_engine_bs_validate_padding(p_source))))
    {
        // Odd rotations ONLY with source dimension of multiple of MCU
        p_dest->transposed = true;
    }

    return JPEGERR_SUCCESS;
}

static int jpege_engine_bs_abort(
    jpege_engine_obj_t *p_obj)
{
    jpege_engine_bs_t *p_engine;

    // Validate input arguments
    if (!p_obj || !p_obj->p_engine)
        return JPEGERR_ENULLPTR;

    // Cast p_obj->obj as jpege_engine_bs_t object
    p_engine = p_obj->p_engine;

    // Abort if engine is actively encoding
    (void)os_mutex_lock(&(p_engine->mutex));
    if (p_engine->is_active)
    {
        p_engine->abort_flag = true;
        while (p_engine->abort_flag)
        {
            (void)os_cond_wait(&(p_engine->cond), &(p_engine->mutex));
        }
    }
    (void)os_mutex_unlock(&(p_engine->mutex));
    return JPEGERR_SUCCESS;
}

static void jpege_engine_bs_destroy(
    jpege_engine_obj_t *p_obj)
{
    if (p_obj)
    {
        jpege_engine_bs_t *p_engine = (jpege_engine_bs_t *)(p_obj->p_engine);
        (void)jpege_engine_bs_abort(p_obj);

        // Release allocated memory
        if (p_engine)
        {
            // Signal the output thread to exit
            (void)os_mutex_lock(&(p_engine->mutex));
            p_engine->thread_exit_flag = true;
            (void)os_cond_signal(&(p_engine->output_cond));
            (void)os_cond_signal(&(p_engine->consume_cond));
            (void)os_cond_signal(&(p_engine->final_output_cond));
            (void)os_mutex_unlock(&(p_engine->mutex));

            // Join the output thread
            (void)os_thread_join(&p_engine->output_thread, NULL);

            (void)os_mutex_destroy(&(p_engine->mutex));
            (void)os_cond_destroy(&(p_engine->final_output_cond));
            (void)os_cond_destroy(&(p_engine->cond));
            (void)os_cond_destroy(&(p_engine->consume_cond));
            (void)os_cond_destroy(&(p_engine->output_cond));

            // Destroy PPF
#if 0 // No PPF
            ppf_destroy(&(p_engine->ppf));
#endif
            JPEG_FREE(p_obj->p_engine);
            p_obj->p_engine = NULL;
        }
    }
}

static int jpege_engine_bs_start(
    jpege_engine_obj_t   *p_obj,
    jpege_img_cfg_t      *p_config,
    jpege_img_data_t     *p_source,
    jpege_engine_dst_t   *p_dest)
{
    int rc;
    jpege_engine_bs_t *p_engine;

    // Validate input arguments
    if (!p_obj || !p_obj->p_engine || !p_config)
        return JPEGERR_ENULLPTR;

    // Cast p_obj->obj as jpege_engine_bs_t object
    p_engine = p_obj->p_engine;

    // Make sure no threads are running
    (void)os_mutex_lock(&(p_engine->mutex));
    if (p_engine->is_active)
    {
        (void)os_mutex_unlock(&(p_engine->mutex));
        JPEG_DBG_HIGH("jpege_engine_bs_start: previous encoding still active\n");
        return JPEGERR_EBADSTATE;
    }

    // Configure the engine based on the input configuration
    rc = jpege_engine_bs_configure(p_engine, p_config, p_source, p_dest);
    if (JPEG_FAILED(rc))
    {
        (void)os_mutex_unlock(&(p_engine->mutex));
        return rc;
    }

    // Reset abort flag
    p_engine->abort_flag = false;
    // Reset error flag
    p_engine->error_flag = false;
    // Reset final ouput flag
    p_engine->final_output_flag = false;

    // Start Jpeg Encoding as a new thread
    rc = os_thread_create_ex(&(p_engine->thread),
                             jpege_engine_bs_thread,
                             (void *)p_engine,
                             OS_THREAD_DEFAULT_PRIORITY,
                             (uint8_t*)jpeg_engine_bs_encode_thread_name);
    // If there is a failure in creating the thread, clean up and exit
    if (rc)
    {
        (void)os_mutex_unlock(&p_engine->mutex);
        JPEG_DBG_ERROR("jpege_engine_bs_start: os_thread_create() failed: %d\n", rc);
        return JPEGERR_EFAILED;
    }
    p_engine->is_active = true;
    (void)os_mutex_unlock(&(p_engine->mutex));

    // Detach the bs thread
    os_thread_detach(&p_engine->thread);

    return JPEGERR_SUCCESS;
}

static int jpege_engine_bs_configure(jpege_engine_bs_t  *p_engine,
                                     jpege_img_cfg_t    *p_config,
                                     jpege_img_data_t   *p_source,
                                     jpege_engine_dst_t *p_dest)
{
    int rc;

    // Make a copy of engine destination
    p_engine->dest = *p_dest;

    // Get input bitstream pointer and size
    p_engine->p_input_bitstream =
        ((jpeg_buf_t *)p_source->p_fragments[0].color.bitstream.bitstream_buf)->ptr;
    p_engine->bitstream_size =
        ((jpeg_buf_t *)p_source->p_fragments[0].color.bitstream.bitstream_buf)->offset;

    // Check if PPF is needed for post-processing input bitstream
#if 0 //No PPF
    p_engine->use_ppf = false;
    if ((p_config->rotation_degree_clk > 0) || (p_config->scale_cfg.enable))
    {
        ppf_image_t         source;
        uint32_t            source_handle;
        ppf_scaling_cfg_t   scaling_cfg;

        // PPF is needed
        p_engine->use_ppf = true;

        // Set PPF source information
        STD_ZEROAT(&source);
        source.format = PPF_IMAGE_FORMAT_JPEG_BS;
        source.data.jpeg_bs.property.width = p_source->width;
        source.data.jpeg_bs.property.height = p_source->height;
        source.data.jpeg_bs.property.subsample =
            (jpeg_subsampling_t)(p_source->color_format >> 1);
        source.data.jpeg_bs.property.restart = p_config->restart_interval;
        source.data.jpeg_bs.property.quality = p_config->quality;
        source.data.jpeg_bs.property.p_luma_quant_tbl =
            p_config->luma_quant_tbl;
        source.data.jpeg_bs.property.p_chroma_quant_tbl =
            p_config->chroma_quant_tbl;
        source.data.jpeg_bs.property.p_luma_dc_huff_tbl =
            &(p_config->luma_dc_huff_tbl);
        source.data.jpeg_bs.property.p_chroma_dc_huff_tbl =
            &(p_config->chroma_dc_huff_tbl);
        source.data.jpeg_bs.property.p_luma_ac_huff_tbl =
            &(p_config->luma_ac_huff_tbl);
        source.data.jpeg_bs.property.p_chroma_ac_huff_tbl =
            &(p_config->chroma_ac_huff_tbl);
        source.data.jpeg_bs.bitstream_buf =
            p_source->p_fragments[0].color.bitstream.bitstream_buf;

        // Set PPF destination information
        p_engine->ppf_dest = source;
        p_engine->ppf_dest.data.jpeg_bs.bitstream_buf = NULL;
        if (p_config->scale_cfg.enable)
        {
            p_engine->ppf_dest.data.jpeg_bs.property.width =
                p_config->scale_cfg.output_width;
            p_engine->ppf_dest.data.jpeg_bs.property.height =
                p_config->scale_cfg.output_height;
        }

        // Set PPF source
        rc = ppf_fill_sources(p_engine->ppf, &source, 1, &source_handle);
        if (JPEG_FAILED(rc)) return rc;

        // Configure PPF with scaling
        if (p_config->scale_cfg.enable)
        {
            scaling_cfg.crop.x = p_config->scale_cfg.h_offset;
            scaling_cfg.crop.y = p_config->scale_cfg.v_offset;
            scaling_cfg.crop.dx = p_config->scale_cfg.input_width;
            scaling_cfg.crop.dy = p_config->scale_cfg.input_height;
            scaling_cfg.output_width = p_config->scale_cfg.output_width;
            scaling_cfg.output_height = p_config->scale_cfg.output_height;
            scaling_cfg.enable = p_config->scale_cfg.enable;
            rc = ppf_configure_scaling(p_engine->ppf, source_handle,
                                       &(scaling_cfg));
            if (JPEG_FAILED(rc)) return rc;
        }

        // Configure PPF with rotation
        rc = ppf_configure_rotation(p_engine->ppf, source_handle, p_config->rotation_degree_clk);
        if (JPEG_FAILED(rc)) return rc;
    }
#endif
    return JPEGERR_SUCCESS;
}

static jpeg_event_t jpege_engine_bs_copy(jpege_engine_bs_t *p_engine)
{
    jpeg_buf_t         *p_dest_buf;
    uint32_t            bytes_to_copy, bytes_remaining;
    uint32_t            bytes_written = 0;
    int                 rc;

    bytes_remaining = p_engine->bitstream_size;
    while (bytes_remaining)
    {
        // Check abort flag
        (void)os_mutex_lock(&(p_engine->mutex));
        if (p_engine->abort_flag)
        {
            p_engine->is_active  = false;
            p_engine->abort_flag = false;
            (void)os_cond_signal(&(p_engine->cond));
            (void)os_mutex_unlock(&(p_engine->mutex));
            return JPEG_EVENT_ABORTED;
        }
        (void)os_mutex_unlock(&(p_engine->mutex));

        // Get the next output buffer from buffer(s) queue if necessary
        if (!p_engine->p_dest_buffer)
        {
            rc = p_engine->dest.p_get_output_buffer_handler(
                p_engine->p_wrapper, &(p_engine->p_dest_buffer));
            if (JPEG_FAILED(rc) || !(p_engine->p_dest_buffer) ||
                !(p_engine->p_dest_buffer->ptr) ||
                (p_engine->p_dest_buffer->offset == p_engine->p_dest_buffer->size))
            {
                JPEG_DBG_ERROR("jpege_engine_bs_copy: failed to get next output buffer\n");
                (void)os_mutex_lock(&(p_engine->mutex));
                p_engine->error_flag = true;
                (void)os_mutex_unlock(&(p_engine->mutex));
                return JPEG_EVENT_ERROR;
            }
        }
        p_dest_buf = p_engine->p_dest_buffer;

        // Calculate how much can be written to the destination buffer
        bytes_to_copy = STD_MIN(bytes_remaining, (p_dest_buf->size - p_dest_buf->offset));

        // Copy the data from bitstream buffer to destination buffer starting from the offset
        (void)STD_MEMMOVE((p_dest_buf->ptr + p_dest_buf->offset),
                          (p_engine->p_input_bitstream + bytes_written), bytes_to_copy);

        // Update destination buffer offset and other state variables
        p_dest_buf->offset += bytes_to_copy;
        bytes_written += bytes_to_copy;
        bytes_remaining -= bytes_to_copy;

        // Check to see whether it is the final output
        if (!bytes_remaining)
        {
            // Set the final output flag to notify jpege_engine_bs_flush_output()
            // to set "is_last_buffer" flag when sending output request to
            // output thread
            (void)os_mutex_lock(&(p_engine->mutex));
            p_engine->final_output_flag = true;
            (void)os_mutex_unlock(&(p_engine->mutex));
        }

        // Flush the current destination buffer to upper layer
        if (JPEG_FAILED(jpege_engine_bs_flush_output(p_engine)))
        {
            (void)os_mutex_lock(&(p_engine->mutex));
            p_engine->error_flag = true;
            (void)os_mutex_unlock(&(p_engine->mutex));
            return JPEG_EVENT_ERROR;
        }
        // Do not hold the output buffer any more
        p_engine->p_dest_buffer = NULL;
    }

    return JPEG_EVENT_DONE;
}

#if 0
static jpeg_event_t jpege_engine_bs_ppf(jpege_engine_bs_t *p_engine)
{
    int                 rc;

    // Start post-processing
    rc = ppf_process(p_engine->ppf, &(p_engine->ppf_dest));
    if (JPEG_FAILED(rc)) return JPEG_EVENT_ERROR;

    return JPEG_EVENT_DONE;
}
#endif

static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
    jpege_engine_bs_thread(OS_THREAD_FUNC_ARG_T arg)
{
    // Cast input argument as the engine object
    jpege_engine_bs_t  *p_engine = arg;
    jpeg_event_t        event;
#if 0
    if (!p_engine->use_ppf)
    {
#endif
        event = jpege_engine_bs_copy(p_engine);
#if 0
    }
    else
    {
      event = jpege_engine_bs_ppf(p_engine);
    }
#endif
    // Wait until the final output is consumed
    (void)os_mutex_lock(&(p_engine->mutex));
    while (p_engine->final_output_flag)
    {
        // Wait output thread to signal with completion of final output buffer
        (void)os_cond_wait(&(p_engine->final_output_cond), &(p_engine->mutex));
    }

    // Set the active flag to inactive
    p_engine->is_active = false;
    if (p_engine->abort_flag)
    {
        p_engine->abort_flag = false;
        (void)os_cond_signal(&p_engine->cond);
    }
    if (p_engine->error_flag)
    {
        p_engine->error_flag = false;
    }
    (void)os_mutex_unlock(&(p_engine->mutex));

    // Notify encode event
    p_engine->p_handler(p_engine->p_wrapper, event, NULL);
    return OS_THREAD_FUNC_RET_SUCCEEDED;
}

static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
    jpege_engine_bs_output_thread(OS_THREAD_FUNC_ARG_T p_arg)
{
    jpege_engine_bs_t *p_engine = (jpege_engine_bs_t *)p_arg;

    for (;;)
    {
        jpege_engine_bs_output_t *p_output;

        (void)os_mutex_lock(&(p_engine->mutex));

        // Block if no output request is pending
        while (!p_engine->pending_outputs[p_engine->pending_output_q_head].is_valid &&
               !p_engine->thread_exit_flag)
        {
            (void)os_cond_wait(&(p_engine->output_cond), &(p_engine->mutex));
        }

        // It is signaled that the thread should exit
        if (p_engine->thread_exit_flag)
        {
            (void)os_mutex_unlock(&(p_engine->mutex));
            break;
        }

        // Dequeue output request and deliver
        p_output = p_engine->pending_outputs + p_engine->pending_output_q_head;
        p_engine->dest.p_output_handler(p_engine->p_wrapper,
                                        p_output->p_buffer,
                                        p_output->is_last_buffer);

        // Check to see if it is last output buffer
        if (p_output->is_last_buffer)
        {
            // Signal encode thread that last buffer is consumed
            // So encode thread can go to finish afterwards
            (void)os_cond_signal(&(p_engine->final_output_cond));
            // Clear the final output flag
            p_engine->final_output_flag = false;
            // Clear the last buffer flag
            p_output->is_last_buffer = false;
        }
        // Output request is done
        p_output->is_valid = false;
        p_engine->pending_output_q_head = (p_engine->pending_output_q_head + 1) %
            PENDING_OUTPUT_Q_SIZE;

        // Signal encode thread that output request is done
        (void)os_cond_signal(&(p_engine->consume_cond));

        (void)os_mutex_unlock(&(p_engine->mutex));
    }

    return OS_THREAD_FUNC_RET_SUCCEEDED;
}

static int jpege_engine_bs_flush_output(jpege_engine_bs_t *p_engine)
{
    jpege_engine_bs_output_t *p_output;

    os_mutex_lock(&(p_engine->mutex));
    while (p_engine->pending_outputs[p_engine->pending_output_q_tail].is_valid)
    {
        // If output request queue is full, wait output thread to signal
        (void)os_cond_wait(&(p_engine->consume_cond), &(p_engine->mutex));
        JPEG_DBG_HIGH("jpege_engine_bs_flush_output: queue is full\n");
    }

    // Set up output request
    p_output = p_engine->pending_outputs + p_engine->pending_output_q_tail;
    p_output->p_buffer = p_engine->p_dest_buffer;
    p_output->is_valid = true;
    // Check to see whether it is the final output
    if (p_engine->final_output_flag)
    {
        // Set output request with "is_last_buffer" flag, output thread
        // will signal encode thread and clear "is_last_buffer" flag once
        // the final buffer is consumed
        p_output->is_last_buffer = true;
    }

    // Enqueue the output request
    p_engine->pending_output_q_tail = (p_engine->pending_output_q_tail + 1) % PENDING_OUTPUT_Q_SIZE;

    // Mark the buffer as filled
    jpeg_buffer_mark_filled(p_engine->p_dest_buffer);

    // Signal output thread to handle the output request
    (void)os_cond_signal(&(p_engine->output_cond));
    (void)os_mutex_unlock(&(p_engine->mutex));

    return JPEGERR_SUCCESS;
}

#if 0
static int jpege_engine_bs_ppf_get_output_buffer
(
    void                 *p_user_data,
    jpeg_buffer_t        *p_buffer
)
{
    jpege_engine_bs_t    *p_engine;
    int                   rc;

    // Validate input pointers
    if (!p_user_data || !p_buffer)
        return JPEGERR_ENULLPTR;

    // Cast p_user_data as jpege_engine_bs_t object
    p_engine = (jpege_engine_bs_t *)p_user_data;

    // Get an output buffer if necessary
    rc = p_engine->dest.p_get_output_buffer_handler(
        p_engine->p_wrapper, &(p_engine->p_dest_buffer));
    if (JPEG_FAILED(rc) || !(p_engine->p_dest_buffer) ||
        !(p_engine->p_dest_buffer->ptr) ||
        (p_engine->p_dest_buffer->offset == p_engine->p_dest_buffer->size))
    {
        JPEG_DBG_ERROR("jpege_engine_bs_ppf_get_output_buffer: "
                       "failed to get next output buffer\n");
        (void)os_mutex_lock(&(p_engine->mutex));
        p_engine->error_flag = true;
        (void)os_mutex_unlock(&(p_engine->mutex));
        return JPEGERR_EFAILED;
    }

    // Copy output buffer pointer
    *p_buffer = p_engine->p_dest_buffer;

    // Do not hold the output buffer any more
    p_engine->p_dest_buffer = NULL;
    return JPEGERR_SUCCESS;
}

static int jpege_engine_bs_ppf_output_handler
(
    void                 *p_user_data,
    jpeg_buffer_t         buffer,
    void                 *p_arg,
    uint8_t               is_last_buffer
)
{
    jpege_engine_bs_t    *p_engine;

    (void)p_arg;

    // Validate input pointers
    if (!p_user_data) return JPEGERR_ENULLPTR;

    // Cast p_user_data as jpege_engine_bs_t object
    p_engine = (jpege_engine_bs_t *)p_user_data;

    // Make a copy of output buffer
    p_engine->p_dest_buffer = (jpeg_buf_t *)buffer;
    // Make a copy of is_last_buffer
    (void)os_mutex_lock(&(p_engine->mutex));
    p_engine->final_output_flag = is_last_buffer;
    (void)os_mutex_unlock(&(p_engine->mutex));

    // Flush the current destination buffer to upper layer
    if (JPEG_FAILED(jpege_engine_bs_flush_output(p_engine)))
    {
        (void)os_mutex_lock(&(p_engine->mutex));
        p_engine->error_flag = true;
        (void)os_mutex_unlock(&(p_engine->mutex));
        return JPEGERR_EFAILED;
    }
    // Do not hold the output buffer any more
    p_engine->p_dest_buffer = NULL;

    return JPEGERR_SUCCESS;
}

#endif

static int jpege_engine_bs_validate_padding
(
    jpege_img_data_t     *p_source
)
{
    uint32_t              width  = p_source->width;
    uint32_t              height = p_source->height;

    // Validate if padding is needed
    switch (p_source->color_format)
    {
    case JPEG_BITSTREAM_H2V2:
        if ((width != ROUND_TO_16(width)) || (height != ROUND_TO_16(height)))
        {
            JPEG_DBG_MED("jpege_engine_bs_validate_padding: H2V2 dimension: "
                         "(%dx%d)\n", width, height);
            return JPEGERR_EUNSUPPORTED;
        }
        break;

    case JPEG_BITSTREAM_H2V1:
        if ((width != ROUND_TO_16(width)) || (height != ROUND_TO_8(height)))
        {
            JPEG_DBG_MED("jpege_engine_bs_validate_padding: H2V1 dimension: "
                         "(%dx%d)\n", width, height);
            return JPEGERR_EUNSUPPORTED;
        }
        break;

    case JPEG_BITSTREAM_H1V2:
        if ((width != ROUND_TO_8(width)) || (height != ROUND_TO_16(height)))
        {
            JPEG_DBG_MED("jpege_engine_bs_validate_padding: H1V2 dimension: "
                         "(%dx%d)\n", width, height);
            return JPEGERR_EUNSUPPORTED;
        }
        break;

    case JPEG_BITSTREAM_H1V1:
        if ((width != ROUND_TO_8(width)) || (height != ROUND_TO_8(height)))
        {
            JPEG_DBG_MED("jpege_engine_bs_validate_padding: H1V1 dimension: "
                         "(%dx%d)\n", width, height);
            return JPEGERR_EUNSUPPORTED;
        }
        break;

    default:
        {
            JPEG_DBG_ERROR("jpege_engine_bs_validate_padding: defaulting \n");
            return JPEGERR_EUNSUPPORTED;
        }
    }

    return JPEGERR_SUCCESS;
}

