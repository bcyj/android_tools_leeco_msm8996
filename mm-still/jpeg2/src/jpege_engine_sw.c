/*******************************************************************************
* Copyright (c) 2008-2012,2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
10/16/14   vgaur   Added support for multi threaded fastCV
03/25/14  revathys Added stride support.
06/04/12   apant   Resolving KW errors
03/28/12   csriniva Fixed Compiler warnings on QNX
03/01/12   csriniva Added support for YUY2 and UYVY inputs
10/29/10   gbc     Fixed lint errors
07/21/10   siguj   Fixing compiler warnings
06/18/10   staceyw Extend upscale module to scale module with sw engine
                   config, setting check and buffer destroy for both upscale
                   and downscale.
06/15/10   leim    Fixed Jpeg engine abort issue.
05/18/10   vma     Rely on os_thread_detach for thread detachment rather
                   than joining
05/11/10   sigu    Commenting out os_thread_join, as it is trying to join on itself
04/28/10   sigu    Adding a flag is_ethread_need_join to ensure that thread join is
                   called to avoid memleaks
04/26/10    sv     Added thread name and priority in thread create API of jpeg OSAL.
04/22/10   sigu      Changes required to incorporate os_perf API's.
04/16/10   staceyw Added flag to indicate output buffer whether is last
                   output buffer during output.
02/16/10   staceyw Provide support of get next output buffer from buffer(s)
                   queue. No more internal output buffers, and delete internal
                   get next buffer function. Add handling to fix syncronization
                   with last buffer for encode and output two different threads.
10/29/09   zhiminl Fixed quant tables for 90/270 rotations.
10/22/09   vma     Export the engine 'profile' for easier engine picking.
10/15/09   vma     Deliver output only when destination buffer is full.
10/14/09   staceyw Remove IPL, added in-line cropping and upscaling support,
                   and split out jpege_engine_sw_configure_internal_buffer(),
                   jpege_engine_sw_configure_parameter() and
                   jpege_engine_sw_configure_huff_tables().
09/08/09   zhiminl Combined fetch_block with fdct_block to remove
                   STD_MEMMOVE() as much as possible.
08/03/09   vma     Switched to use the os abstraction layer (os_*)
07/06/09   zhiminl Fixed wait until all the dest buffers are empty before
                   sending DONE event.
06/17/09   zhiminl Added cropping and upscaling support (phase 1 - IPL).
06/16/09   zhiminl Fixed memory leak caused by internal bitstream pack
                   buffer in jpege_engine_sw_configure().
04/14/09   zhiminl Added internal bitstream pack buffer.
04/08/09   zhiminl Replaced zigzag table with zigzag offset table.
04/06/09   zhiminl Removed jpege_engine_sw_dct_wrapper() and split out
                   jpege_engine_sw_quant_zigzag().
04/04/09   zhiminl Fixed type conversion in fetch_mcu() which caused
                   segmentation fault for rotation cases.
10/14/08   vma     Optimizations.
09/15/08   vma     Fixed abort handling.
07/07/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "jpeg_buffer_private.h"
#include "jpege_engine_sw.h"
#include "jpege.h"
#include "jpegerr.h"
#include "jpeglog.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
/* Static constants */
static const int16_t zigzagTable[JBLOCK_SIZE] =
{
    0,   1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};
static const int16_t zigzagTableTranspose[JBLOCK_SIZE] =
{
    0,  8,  1,  2,  9,  16, 24, 17,
    10, 3,  4,  11, 18, 25, 32, 40,
    33, 26, 19, 12, 5,  6,  13, 20,
    27, 34, 41, 48, 56, 49, 42, 35,
    28, 21, 14, 7,  15, 22, 29, 36,
    43, 50, 57, 58, 51, 44, 37, 30,
    23, 31, 38, 45, 52, 59, 60, 53,
    46, 39, 47, 54, 61, 62, 55, 63
};

static const char jpeg_engine_sw_encode_thread_name[] = "jpege_engine_sw_encode";
static const char jpeg_engine_sw_scale_thread_name[] = "jpege_engine_scale_fastCV";

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* Function prototypes of jpege_engine_obj_t interface functions */
static void jpege_engine_sw_create(
    jpege_engine_hybrid_obj_t *p_obj,
    jpege_engine_hybrid_obj    hybrid);
static int jpege_engine_sw_init(
    jpege_engine_hybrid_obj_t    *p_obj,
    jpege_engine_hybrid_event_handler_t   p_event_handler,
    jpege_engine_hybrid_output_handler_t  p_output_handler);
static int jpege_engine_sw_start(
    jpege_engine_hybrid_obj_t   *p_obj,
    jpege_img_cfg_t             *p_config,
    jpege_img_data_t            *p_source,
    jpege_engine_hybrid_cfg_t   *p_hybrid_cfg);
static int  jpege_engine_sw_abort   (
    jpege_engine_hybrid_obj_t*);
static void jpege_engine_sw_destroy (
    jpege_engine_hybrid_obj_t*);
static void jpege_engine_sw_thread_destroy (
    jpege_engine_hybrid_obj_t*);
static int  jpege_engine_sw_check_start_param (
    jpege_img_cfg_t*,
                                               jpege_img_data_t*,
    jpege_engine_hybrid_cfg_t*);

/* Function prototypes of helper functions */
static int jpege_engine_sw_configure(jpege_engine_sw_t  *p_engine,
                                     jpege_img_cfg_t    *p_config,
                                     jpege_img_data_t   *p_source,
                                     jpege_engine_hybrid_cfg_t *p_hybrid_cfg);
static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
             jpege_engine_sw_encode         (OS_THREAD_FUNC_ARG_T);
static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
             jpege_engine_sw_scale_fastCV         (OS_THREAD_FUNC_ARG_T);
static void  jpege_engine_sw_copy_bytes(jpege_engine_sw_t *p_engine, uint8_t force_flush);
static void  jpege_engine_sw_fetch_dct_block_partial_luma(jpege_engine_sw_t *,
                                                          const uint8_t *,
                                                          uint32_t, uint32_t, int16_t *);
static void  jpege_engine_sw_fetch_dct_block_partial_chroma(jpege_engine_sw_t *,
                                                            const uint8_t *,
                                                            uint32_t, uint32_t, int16_t *);
static void  jpege_engine_sw_fetch_dct_block_partial_luma_packed(jpege_engine_sw_t *,
                                                                 const uint8_t *,
                                                                 uint32_t, uint32_t, int16_t *);
static void  jpege_engine_sw_fetch_dct_block_partial_chroma_packed(jpege_engine_sw_t *,
                                                                   const uint8_t *,
                                                                   uint32_t, uint32_t, int16_t *);
static void  jpege_engine_sw_fetch_dct_mcu(jpege_engine_sw_t *, int16_t *, int16_t *);
static void  jpege_engine_sw_fetch_dct_mcu_packed(jpege_engine_sw_t *, int16_t *, int16_t *);
static int   jpege_engine_sw_configure_internal_buffer(jpege_engine_sw_t  *p_engine,
                 jpege_engine_hybrid_cfg_t *p_cfg);
static int   jpege_engine_sw_configure_parameter(jpege_engine_sw_t  *p_engine);
static int   jpege_engine_sw_configure_rotation(jpege_engine_sw_t *,
                                                jpege_img_cfg_t *);
static int   jpege_engine_sw_configure_rotation_packed(jpege_engine_sw_t *,
                                                       jpege_img_cfg_t *);
static void  jpege_engine_sw_configure_huff_tables(jpege_engine_sw_t  *p_engine,
                                                   jpege_img_cfg_t    *p_config);
/* Extern functions from other files within the same engine */
extern void jpege_engine_sw_huff_init_tables (jpege_engine_sw_t *p_engine);
extern void jpege_engine_sw_huff_encode(jpege_engine_sw_t *p_engine,
                                        int16_t *zigzagOutput,
                                        yCbCrType_t yORcbcr);
extern void jpege_engine_sw_fdct_block (const uint8_t *pixelDomainBlock,
                                        DCTELEM       *frequencyDomainBlock);
extern void jpege_engine_sw_fetch_dct_block_luma(const uint8_t *, int16_t *, uint32_t);
extern void jpege_engine_sw_fetch_dct_block_luma_packed(const uint8_t *, int16_t *, uint32_t);
extern void jpege_engine_sw_fetch_dct_block_chroma(const uint8_t  *,
                                                   int16_t *, uint32_t,
                                                   input_format_t);
extern void jpege_engine_sw_fetch_dct_block_chroma_packed(const uint8_t  *,
                                                   int16_t *, uint32_t,
                                                   input_format_t);
extern void jpege_engine_sw_quant_zigzag (const int16_t  *quantInput,
                                                int16_t  *zigzagOutput,
                                          const int16_t  *zigzagOffsetTable,
                                          const int16_t  *quantTable);
extern void jpege_engine_sw_pack_bs (uint32_t, uint32_t, bitstream_t *);

/* configue function of in-line scale */
extern int  jpege_engine_sw_scale_configure(jpege_engine_sw_t  *p_engine,
                                            jpege_img_cfg_t    *p_config,
                                            jpege_img_data_t   *p_source);

extern int  jpege_engine_sw_scale_configure_packed(jpege_engine_sw_t  *p_engine,
                                            jpege_img_cfg_t    *p_config,
                                            jpege_img_data_t   *p_source);
extern void    jpege_engine_sw_downscale_mcu_lines_fastCV(
    jpege_scale_t  *p_scale_info,
    uint32_t rotated_mcu_height);

/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
#define INTERNAL_BUFFER_SIZE   8192 /* in bytes */
#define MAX_MCU_SIZE           384  /* in bytes */
static const char jpege_engine_sw_name[] = "Jpeg Software Encode Engine";

jpege_engine_hybrid_profile_t jpege_engine_sw_profile =
  { jpege_engine_sw_create, jpege_engine_sw_name };

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
**                          Function Definitions
** ======================================================================= */

static void jpege_engine_sw_create(
    jpege_engine_hybrid_obj_t *p_obj,
    jpege_engine_hybrid_obj    hybrid)
{
    if (p_obj)
    {
        // Destroy previous engine if it exists
        if (p_obj->destroy)
        {
            p_obj->destroy(p_obj);
        }
        p_obj->create            = &jpege_engine_sw_create;
        p_obj->init              = &jpege_engine_sw_init;
        p_obj->check_start_param = &jpege_engine_sw_check_start_param;
        p_obj->start             = &jpege_engine_sw_start;
        p_obj->abort             = &jpege_engine_sw_abort;
        p_obj->destroy           = &jpege_engine_sw_destroy;
        p_obj->thread_destroy    = &jpege_engine_sw_thread_destroy;
        p_obj->p_engine          = NULL;
        p_obj->hybrid            = hybrid;
        p_obj->is_initialized    = false;
    }
}

static int jpege_engine_sw_init(
    jpege_engine_hybrid_obj_t            *p_obj,
    jpege_engine_hybrid_event_handler_t   p_event_handler,
    jpege_engine_hybrid_output_handler_t  p_output_handler)
{
    jpege_engine_sw_t *p_engine;

    // Validate input arguments
    if (!p_obj || !p_event_handler || !p_output_handler)
        return JPEGERR_ENULLPTR;

    // Allocate memory for the engine structure
    p_engine = (jpege_engine_sw_t *)JPEG_MALLOC(sizeof(jpege_engine_sw_t));
    if (!p_engine)
        return JPEGERR_EMALLOC;

    // Initialize the fields inside the engine structure below
    STD_ZEROAT(p_engine);  // Zero out the entire structure
    p_engine->p_wrapper = p_obj;                         // Initialize the pointer to the wrapper
    p_engine->p_event_handler = p_event_handler;         // Initialize the event handler
    p_engine->p_output_handler = p_output_handler;       // Initialize the output handler
    os_mutex_init(&(p_engine->mutex));                   // Initialize the mutex
    os_cond_init(&(p_engine->cond));                     // Initialize the condition variables
    os_cond_init(&(p_engine->output_cond));
    // Initialize the condition used for output thread consume
    os_cond_init(&(p_engine->consume_cond));
    // Initialize the condition used for last buffer output
    os_cond_init(&(p_engine->final_output_cond));

    // Assign allocated engine structure to p_obj
    p_obj->p_engine = (void *)p_engine;
    p_obj->is_initialized = true;
    return JPEGERR_SUCCESS;
}

static int jpege_engine_sw_check_start_param(
    jpege_img_cfg_t      *p_config,
    jpege_img_data_t     *p_source,
    jpege_engine_hybrid_cfg_t  *p_hybrid_cfg)
{
    /*------------------------------------------------------------
      Input validation
      ------------------------------------------------------------*/
    if (!p_config || !p_source || !p_hybrid_cfg)
        return JPEGERR_ENULLPTR;

    // Validate color format
    if (p_source->color_format > YCBCRLP_H1V1 && p_source->color_format <= JPEG_BITSTREAM_H1V1)
    {
        // Supports only YUV color format
        JPEG_DBG_MED("jpege_engine_sw_check_start_param: unsupported color format\n");
        return JPEGERR_EUNSUPPORTED;
    }

    // Validate rotation
    if (p_config->rotation_degree_clk % 90)
    {
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
        JPEG_DBG_MED("jpege_engine_sw_check_start_param: invalid scale config: \
          (h offset, v offset)(%d, %d)  (input width, input height)(%d, %d) (output width,output height)(%d, %d)"
          " (source width, source height) (%d, %d)\n",
          p_config->scale_cfg.h_offset, p_config->scale_cfg.v_offset,
          p_config->scale_cfg.input_width, p_config->scale_cfg.input_height,
          p_config->scale_cfg.output_width, p_config->scale_cfg.output_height,
          p_source->width, p_source->height);
        return JPEGERR_EBADPARM;
    }

    JPEG_DBG_LOW("Stride enable = %d\n",p_config->stride_cfg.enable);
    JPEG_DBG_LOW("input_luma_stride = %d width = %d\n", p_config->stride_cfg.input_luma_stride,p_source->width);
    JPEG_DBG_LOW("input_chroma_stride = %d ", p_config->stride_cfg.input_chroma_stride);
    if (p_source->color_format == YCBCRLP_H2V2 || p_source->color_format == YCRCBLP_H2V2 || p_source->color_format == YCRCBLP_H2V1 || p_source->color_format == YCBCRLP_H2V1)
    {
        if ((p_config->stride_cfg.enable) && (p_config->stride_cfg.input_luma_stride < p_source->width || p_config->stride_cfg.input_chroma_stride < p_source->width))
        {
            JPEG_DBG_ERROR(" jpege_engine_sw_check_start_param:Input stride not configured properly \n");
            return JPEGERR_EBADPARM;
        }
        if ((p_config->stride_cfg.enable) && (p_config->rotation_degree_clk == 0 || p_config->rotation_degree_clk == 180) && (p_config->stride_cfg.output_luma_stride < p_source->width || p_config->stride_cfg.output_chroma_stride < p_source->width))
        {
            JPEG_DBG_ERROR(" jpege_engine_sw_check_start_param:Output stride not configured properly \n");
            return JPEGERR_EBADPARM;
        }
        else if ((p_config->stride_cfg.enable) && (p_config->rotation_degree_clk == 90 || p_config->rotation_degree_clk == 270) && (p_config->stride_cfg.output_luma_stride < p_source->height || p_config->stride_cfg.output_chroma_stride < p_source->height))
        {
            JPEG_DBG_ERROR(" jpege_engine_sw_check_start_param:Output stride not configured properly \n");
            return JPEGERR_EBADPARM;
        }
    }
    else if (p_source->color_format == YCRCBLP_H1V2 || p_source->color_format == YCBCRLP_H1V2 || p_source->color_format == YCBCRLP_H1V1 || p_source->color_format == YCRCBLP_H1V1)
    {
        if ((p_config->stride_cfg.enable) && (p_config->stride_cfg.input_luma_stride < p_source->width || p_config->stride_cfg.input_chroma_stride < (2 * p_source->width)))
        {
            JPEG_DBG_ERROR(" jpege_engine_sw_check_start_param:Input stride not configured properly \n");
            return JPEGERR_EBADPARM;
        }
        if ((p_config->stride_cfg.enable) && (p_config->rotation_degree_clk == 0 || p_config->rotation_degree_clk == 180) && (p_config->stride_cfg.output_luma_stride < p_source->width || p_config->stride_cfg.output_chroma_stride < (2 * p_source->width)))
        {
            JPEG_DBG_ERROR("jpege_engine_sw_check_start_param: Output stride not configured properly \n");
            return JPEGERR_EBADPARM;
        }
        else if ((p_config->stride_cfg.enable) && (p_config->rotation_degree_clk == 90 || p_config->rotation_degree_clk == 270) && (p_config->stride_cfg.output_luma_stride < p_source->height || p_config->stride_cfg.output_chroma_stride < (2 * p_source->height)))
        {
            JPEG_DBG_ERROR("jpege_engine_sw_check_start_param: Output stride not configured properly \n");
            return JPEGERR_EBADPARM;
        }
    }
    // Validate fragment
    if (p_source->fragment_cnt > 1)
    {
        JPEG_DBG_ERROR("jpege_engine_sw_check_start_param: unsupported fragment: %d\n", p_source->fragment_cnt);
        return JPEGERR_EBADPARM;
    }
    return JPEGERR_SUCCESS;
}

static int jpege_engine_sw_start(
    jpege_engine_hybrid_obj_t   *p_obj,
    jpege_img_cfg_t      *p_config,
    jpege_img_data_t     *p_source,
    jpege_engine_hybrid_cfg_t   *p_hybrid_cfg)
{
    int rc;
    jpege_engine_sw_t *p_engine;
    // Validate input arguments
    if (!p_obj || !p_obj->p_engine)
        return JPEGERR_ENULLPTR;

    // Cast p_obj->obj as jpege_engine_sw_t object
    p_engine = (jpege_engine_sw_t *)p_obj->p_engine;
    p_engine->fastCV_flag = p_config->fastCV_flag;

   // Make sure no threads are running
    os_mutex_lock(&(p_engine->mutex));
    if (p_engine->is_active)
    {
        os_mutex_unlock(&(p_engine->mutex));
        JPEG_DBG_ERROR("jpege_engine_sw_start: previous encoding still active\n");
        return JPEGERR_EBADSTATE;
    }

    // Configure the engine based on the input configuration
    rc = jpege_engine_sw_configure(p_engine, p_config, p_source, p_hybrid_cfg);
    if (JPEG_FAILED(rc))
    {
        os_mutex_unlock(&(p_engine->mutex));
        return rc;
    }

    // Reset abort flag
    p_engine->abort_flag = false;
    // Reset error flag
    p_engine->error_flag = false;
    // Reset final ouput flag
    p_engine->final_output_flag = false;

    if(p_engine->fastCV_flag)
        rc = os_thread_create_ex(&p_engine->thread,
                             jpege_engine_sw_scale_fastCV,
                             (void *)p_engine,
                             OS_THREAD_DEFAULT_PRIORITY,
                             (uint8_t*)jpeg_engine_sw_scale_thread_name);
    else
        rc = os_thread_create_ex(&p_engine->thread,
                             jpege_engine_sw_encode,
                             (void *)p_engine,
                             OS_THREAD_DEFAULT_PRIORITY,
                             (uint8_t*)jpeg_engine_sw_encode_thread_name);
    // If there is a failure in creating the thread, clean up and exit
    if (rc)
    {
        os_mutex_unlock(&p_engine->mutex);
        JPEG_DBG_ERROR("jpege_engine_sw_start: os_thread_create() failed: %d\n", rc);
        return JPEGERR_EFAILED;
    }
    p_engine->is_active = true;
    os_mutex_unlock(&(p_engine->mutex));

    // Detach the encode thread
    //os_thread_detach(&p_engine->thread);

    return JPEGERR_SUCCESS;
}

static int jpege_engine_sw_abort(
    jpege_engine_hybrid_obj_t *p_obj)
{
    int32_t i;
    jpege_engine_sw_t *p_engine;

    // Validate input arguments
    if (!p_obj || !p_obj->p_engine)
        return JPEGERR_ENULLPTR;

    // Cast p_obj->obj as jpege_engine_sw_t object
    p_engine = (jpege_engine_sw_t *)p_obj->p_engine;

    // Abort if engine is actively encoding
    os_mutex_lock(&(p_engine->mutex));

    // clear queue on output thread first
    JPEG_DBG_LOW("jpege_engine_sw_abort: clear output queue.\n");
    for (i=0; i<PENDING_OUTPUT_Q_SIZE; i++)
    {
        p_engine->pending_outputs[i].is_valid = false;
    }

    if (p_engine->is_active)
    {
        JPEG_DBG_LOW("jpege_engine_sw_abort: engine active.\n");
        p_engine->abort_flag = true;

        // broadcast signals to un block engine to run to a exit point
        JPEG_DBG_LOW("jpege_engine_sw_abort: un-block signals.\n");
        os_cond_signal(&(p_engine->output_cond));
        os_cond_signal(&(p_engine->consume_cond));
        os_cond_signal(&(p_engine->final_output_cond));

        JPEG_DBG_HIGH("jpege_engine_sw_abort: waiting to finish.\n");
        while (p_engine->abort_flag)
        {
            os_cond_wait(&(p_engine->cond), &(p_engine->mutex));
        }
        JPEG_DBG_LOW("jpege_engine_sw_abort: aborted.\n");
    }
    os_mutex_unlock(&(p_engine->mutex));
    return JPEGERR_SUCCESS;
}

static void jpege_engine_sw_thread_destroy(
    jpege_engine_hybrid_obj_t *p_obj)
{
    if (p_obj)
    {
        jpege_engine_sw_t *p_engine = (jpege_engine_sw_t *)(p_obj->p_engine);

        if (p_engine->thread)
        {
            os_thread_join(&p_engine->thread, NULL);
        }
    }
}

static void jpege_engine_sw_destroy(
    jpege_engine_hybrid_obj_t *p_obj)
{
    if (p_obj)
    {
        jpege_engine_sw_t *p_engine = (jpege_engine_sw_t *)(p_obj->p_engine);
        // Abort and wait until engine is done with current encoding
        // todo
        (void)jpege_engine_sw_abort(p_obj);
        jpege_engine_sw_thread_destroy(p_obj);

        // Release allocated memory
        if (p_engine)
        {
            if (p_engine->jpegeBitStreamState.buffer)
            {
                JPEG_FREE(p_engine->jpegeBitStreamState.buffer);
                STD_ZEROAT(&(p_engine->jpegeBitStreamState));
            }

            // Destroy the scale internal buffers
            jpege_engine_sw_scale_destroy(&(p_engine->jpege_scale));

            // Signal the output thread to exit
            os_mutex_lock(&(p_engine->mutex));
            p_engine->thread_exit_flag = true;
            os_cond_signal(&(p_engine->output_cond));
            os_cond_signal(&(p_engine->consume_cond));
            os_cond_signal(&(p_engine->final_output_cond));
            os_mutex_unlock(&(p_engine->mutex));

            // Join the output thread
            os_thread_join(&p_engine->output_thread, NULL);
            // Join the encode thread
            JPEG_DBG_MED("jpege_engine_sw_destroy: join %d %d",
              p_engine->thread, os_thread_self());

            os_mutex_destroy(&(p_engine->mutex));
            os_cond_destroy(&(p_engine->final_output_cond));
            os_cond_destroy(&(p_engine->cond));
            os_cond_destroy(&(p_engine->consume_cond));
            os_cond_destroy(&(p_engine->output_cond));
            JPEG_FREE(p_obj->p_engine);
            p_obj->p_engine = NULL;
        }
    }
}

/******************************************************************************
 * Helper functions below
 *****************************************************************************/
static int jpege_engine_sw_configure(jpege_engine_sw_t  *p_engine,
                                     jpege_img_cfg_t    *p_config,
                                     jpege_img_data_t   *p_source,
                                     jpege_engine_hybrid_cfg_t *p_hybrid_cfg)
{
    int rc;
    // Pass current JPEG encoding information
    p_engine->output_MCUs = p_hybrid_cfg->output_MCUs;
    p_engine->restartMCUModCount = (uint16_t)p_hybrid_cfg->restart_mcu_count;
    p_engine->restartMarkerModCount = (uint16_t)p_hybrid_cfg->base_restart_marker;
    p_engine->src_index = p_hybrid_cfg->src_index;
    p_engine->rotatedhoriMCUIndex = p_hybrid_cfg->start_hori_MCU_index;
    p_engine->rotatedvertMCUIndex = p_hybrid_cfg->start_vert_MCU_index;

    /*------------------------------------------------------------
      Initialize the fields in the p_engine structure
    --------------------------------------------------------------*/
    p_engine->restartInterval = p_config->restart_interval;
    p_engine->rotation = p_config->rotation_degree_clk;

    if (p_config->scale_cfg.enable)
    {
        p_engine->lumaWidth  = p_config->scale_cfg.output_width;
        p_engine->lumaHeight = p_config->scale_cfg.output_height;
    }
    else
    {
        p_engine->lumaWidth = p_source->width;
        p_engine->lumaHeight = p_source->height;
    }
    p_engine->InputFormat = ((uint8_t)p_source->color_format % 2) ? YCbCr : YCrCb;
    p_engine->subsampling = (jpeg_subsampling_t)((uint8_t)p_source->color_format / 2);


    /* Initialize stride configurations */
    if (p_config->stride_cfg.enable)
    {
        //If stride is enabled, then set it to the stride cfg provided.
        p_engine->stride_cfg = p_config->stride_cfg;
    }
    else
    {
        //If stride is disabled, then luma_stride is set to lumawidth and chroma stride is set to twice the chromawidth.
        p_engine->stride_cfg.enable = p_config->stride_cfg.enable;
        p_engine->stride_cfg.input_luma_stride = p_source->width;
        p_engine->stride_cfg.output_luma_stride = p_source->width;
        switch (p_engine->subsampling)
        {
        case JPEG_H1V1:
        case JPEG_H1V2:
        {
            p_engine->stride_cfg.input_chroma_stride = p_source->width;
            p_engine->stride_cfg.output_chroma_stride = p_source->width;
            break;
        }
        case JPEG_H2V1:
        case JPEG_H2V2:
        {
            p_engine->stride_cfg.input_chroma_stride = 2 * ((p_source->width + 1) >> 1); // odd luma support
            p_engine->stride_cfg.output_chroma_stride = 2 * ((p_source->width + 1) >> 1); // odd luma support
            break;
        }
        default:
            JPEG_DBG_ERROR("jpege_engine_sw_configure: invalid jpeg subsampling: %d\n", p_engine->subsampling);
            return JPEGERR_EBADPARM;
        }
    }

    /* Initialize input Y and CbCr/CrCb pointers */
    p_engine->inputYPtr    = ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->ptr +
                             ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->start_offset;
    p_engine->inputCbCrPtr = ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->ptr +
                             ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->start_offset;

    if(p_source->color_format == YUY2)
    {
        p_engine->inputYPtr    = ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->ptr;
        p_engine->inputCbCrPtr = ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->ptr + 1;
        p_engine->jpege_scale.isPacked = 1;
        p_engine->InputFormat = YCbCr;
    }

    if(p_source->color_format == UYVY)
    {
        p_engine->inputYPtr    = ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->ptr + 1;
        p_engine->inputCbCrPtr = ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->ptr;
        p_engine->jpege_scale.isPacked = 1;
        p_engine->InputFormat = YCbCr;
    }

    JPEG_DBG_MED("jpege_engine_sw_configure luma_start_offset %d chroma_start_offset %d",
                 ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->start_offset,
                 ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->start_offset);

    /* Initialize dest buffers and internal bitstream buffer */
    rc = jpege_engine_sw_configure_internal_buffer(p_engine, p_hybrid_cfg);
    if (JPEG_FAILED(rc))
    {
        return rc;
    }
    /* Configure parameters - chroma sizes and MCU sizes determined by
     * subsample format, and other initializations.
     */
    rc = jpege_engine_sw_configure_parameter(p_engine);
    if (JPEG_FAILED(rc))
    {
        return rc;
    }
    /* Configure rotation related parameters - rotated luma & chroma sizes,
     * rotated MCU sizes, rotated luma & chroma fetch origins and steps,
     * zigzag table and quant tables for DCT domain rotation.
     */
    if(p_engine->jpege_scale.isPacked == 0)
    {
        rc = jpege_engine_sw_configure_rotation(p_engine, p_config);
    }
    else
    {
        rc = jpege_engine_sw_configure_rotation_packed(p_engine, p_config);
    }

    if (JPEG_FAILED(rc))
    {
        return rc;
    }
    /* Configure huffman tables */
    jpege_engine_sw_configure_huff_tables(p_engine, p_config);

    /* Configure default fetch mcu function */
    if(p_engine->jpege_scale.isPacked == 0)
    {
        p_engine->jpege_engine_sw_fetch_dct_mcu_func = &jpege_engine_sw_fetch_dct_mcu;
    }
    else
    {
        p_engine->jpege_engine_sw_fetch_dct_mcu_func = &jpege_engine_sw_fetch_dct_mcu_packed;
    }

    /* Check if scale is enabled */
    if (p_config->scale_cfg.enable)
    {
        /* Configue for crop and scale */
        p_engine->jpege_scale.vert_luma_row_index = p_hybrid_cfg->scale_cfg.vert_luma_row_index;
        p_engine->jpege_scale.vert_luma_output_height = p_hybrid_cfg->scale_cfg.vert_luma_output_height;
        p_engine->jpege_scale.vert_chroma_row_index = p_hybrid_cfg->scale_cfg.vert_chroma_row_index;
        p_engine->jpege_scale.vert_chroma_output_height = p_hybrid_cfg->scale_cfg.vert_chroma_output_height;
        p_engine->jpege_scale.piece_height = p_hybrid_cfg->scale_cfg.piece_height;
        p_engine->jpege_scale.fastCV_enable = p_hybrid_cfg->scale_cfg.fastCV_enable;
        p_engine->jpege_scale.fastCV_buffer = p_hybrid_cfg->scale_cfg.fastCV_buffer;
        rc = jpege_engine_sw_scale_configure(p_engine, p_config, p_source);
        return rc;
    }

    return JPEGERR_SUCCESS;
}

static void jpege_engine_sw_stuff_bs(bitstream_t *p_bitstream)
{
    if (p_bitstream->bitsFree != 32)
    {
        /*-----------------------------------------------------------------------
        At least one bit in the 32-bit assembly buffer. Append 7 1s starting
        from the bitsFree. There is at least one 1 already there.
        -----------------------------------------------------------------------*/
        jpege_engine_sw_pack_bs(0x7F, 7, p_bitstream);

        //Clear bitAssemblybuffer and reset empty bit location.
        p_bitstream->bitAssemblyBufferUL32 = 0;
        p_bitstream->bitsFree = 32;
    }
}

static void jpege_engine_sw_insert_restart_marker(jpege_engine_sw_t *p_engine)
{
    bitstream_t *p_bitstream = &(p_engine->jpegeBitStreamState);
    uint16_t currentMarker;

    /*--------------------------------------------------------------------------
    1. 1-bit stuffing at the end of remaining bits in the 64-bit word buffer.
    2. Flush out the remaining byte in the 64-bit word buffer to the bit-stream.
       Reset the bitCount
    3. Insert the appropriate the restart marker to the bit-stream
      ------------------------------------------------------------------------*/

    jpege_engine_sw_stuff_bs(p_bitstream);

    // Insert Restart Marker and update Marker Mod Count.
    currentMarker = BASE_RESTART_MARKER + p_engine->restartMarkerModCount;
    p_engine->restartMarkerModCount =
        (p_engine->restartMarkerModCount + 1) % MAX_RESTART_MARKER_COUNT;

    // Copy the restart marker to the bit-stream buffer. Increment byte count.
    *(p_bitstream->nextBytePtr++) = 0xFF;
    *(p_bitstream->nextBytePtr++) = (uint8_t)(currentMarker & 0xFF);

    // Reset Y, Cb and Cr DC values.
    p_engine->prevLumaDC = 0;
    p_engine->prevCbDC = 0;
    p_engine->prevCrDC = 0;
}

static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
    jpege_engine_sw_scale_fastCV(OS_THREAD_FUNC_ARG_T arg)
{
    jpege_engine_sw_t *p_engine = (jpege_engine_sw_t *)arg;
    os_mutex_lock(&(p_engine->mutex));
    jpege_engine_sw_downscale_mcu_lines_fastCV(&(p_engine->jpege_scale),p_engine->rotatedMCUHeight);
    p_engine->is_active = false;
    os_mutex_unlock(&(p_engine->mutex));
    p_engine->p_event_handler(p_engine->p_wrapper, JPEG_EVENT_DONE, NULL);
    return OS_THREAD_FUNC_RET_SUCCEEDED;
}

static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
    jpege_engine_sw_encode(OS_THREAD_FUNC_ARG_T arg)
{
    // Cast input argument as the engine object
    jpege_engine_sw_t *p_engine = (jpege_engine_sw_t *)arg;

    uint32_t     currMCUIndex, totalMCUCount;
    int16_t     *currDctOutputPtr;
    int16_t      zigzagOutput[JBLOCK_SIZE];
    uint32_t     numLumaBlocks = p_engine->numLumaBlocks;
    int16_t     *currMCULumaDctOutputPtr = p_engine->currMCULumaDctOutput;
    int16_t     *currMCUChromaDctOutputPtr = p_engine->currMCUChromaDctOutput;
    bitstream_t *p_bitstream = &(p_engine->jpegeBitStreamState);
    uint8_t     *p_bound = p_bitstream->buffer +
                           (p_bitstream->buffersize - MAX_MCU_SIZE);
    uint32_t     i;

    /*Turn on VeNum*/
    //os_perf_toggle(JPEG_VENUM_ENC, JPEG_PERFORMANCE_REQD);

    /* Initialize Huffman lookup tables */
    jpege_engine_sw_huff_init_tables(p_engine);

    /* Set total MCU to be encoded*/
    totalMCUCount = p_engine->output_MCUs;

    /*------------------------------------------------------------
      MCU processing loop. Process 1 MCU per iteration.
    ------------------------------------------------------------*/
    for (currMCUIndex = 0; currMCUIndex < totalMCUCount; currMCUIndex++)
    {
        /* Check abort or error flag */
        os_mutex_lock(&(p_engine->mutex));

        if (p_engine->abort_flag)
        {
            p_engine->is_active = false;
            p_engine->abort_flag = false;
            p_engine->error_flag = false;
            os_cond_signal(&p_engine->cond);
            os_mutex_unlock(&(p_engine->mutex));
            //os_perf_toggle(JPEG_VENUM_ENC, JPEG_PERFORMANCE_NOT_REQD);
            return OS_THREAD_FUNC_RET_FAILED;
        }
        else
            {
            if (p_engine->error_flag)
            {
                p_engine->is_active = false;
                p_engine->error_flag = false;
                os_mutex_unlock(&(p_engine->mutex));
                // Throw encode error event
                p_engine->p_event_handler(p_engine->p_wrapper, JPEG_EVENT_ERROR, NULL);
                //os_perf_toggle(JPEG_VENUM_ENC, JPEG_PERFORMANCE_NOT_REQD);
                return OS_THREAD_FUNC_RET_FAILED;
            }
        }
        os_mutex_unlock(&(p_engine->mutex));

        /* Restart Interval Processing, if necessary */
        if ((p_engine->restartMCUModCount == p_engine->restartInterval) &&
            (p_engine->restartInterval != 0))
        {
            p_engine->restartMCUModCount = 0;
            jpege_engine_sw_insert_restart_marker(p_engine);
        }
        p_engine->restartMCUModCount++;

        /* Fetch MCU from the input image. Do DCT */
        (*(p_engine->jpege_engine_sw_fetch_dct_mcu_func))(p_engine,
                                                          currMCULumaDctOutputPtr,
                                                          currMCUChromaDctOutputPtr);

        /*--------------------------------------------------------
          Loop to process Luma Blocks. Do Quant, Zigzag and
          Entropy Encode
        --------------------------------------------------------*/
        currDctOutputPtr = currMCULumaDctOutputPtr;
        for (i = 0; i < numLumaBlocks; i++)
        {
            /* Do Quant and Zigzag */
            jpege_engine_sw_quant_zigzag (
                currDctOutputPtr,
                zigzagOutput,
                p_engine->zigzagOffsetTable,
                p_engine->lumaQuantTableQ16);

            /* Do Baseline Huffman Encode */
            jpege_engine_sw_huff_encode (p_engine, zigzagOutput, LUMA);
            currDctOutputPtr += JBLOCK_SIZE;
        }

        /*--------------------------------------------------------
          Cb: Do Quant, Zigzag and Entropy Encode
        --------------------------------------------------------*/
        /* Do Quant and Zigzag */
        jpege_engine_sw_quant_zigzag (
            currMCUChromaDctOutputPtr,
            zigzagOutput,
            p_engine->zigzagOffsetTable,
            p_engine->chromaQuantTableQ16);

        /* Do Baseline Huffman Encode */
        jpege_engine_sw_huff_encode (p_engine, zigzagOutput, CB);

        /*--------------------------------------------------------
         Cr: Do Quant, Zigzag and Entropy Encode
        --------------------------------------------------------*/
        /* Do Quant and Zigzag */
        jpege_engine_sw_quant_zigzag (
            (currMCUChromaDctOutputPtr + JBLOCK_SIZE),
            zigzagOutput,
            p_engine->zigzagOffsetTable,
            p_engine->chromaQuantTableQ16);

        /* Do Baseline Huffman Encode */
        jpege_engine_sw_huff_encode (p_engine, zigzagOutput, CR);

        /*--------------------------------------------------------
          If this is the last MCU block of the image,
         last residue bits should be stuffed by 1s to form a byte.
        --------------------------------------------------------*/
        if (currMCUIndex == (totalMCUCount - 1))
        {
            jpege_engine_sw_stuff_bs(p_bitstream);
        }

        /* Update horizontal and vertical indices for the next MCU */
        p_engine->rotatedhoriMCUIndex ++;
        if (p_engine->rotatedhoriMCUIndex == p_engine->rotatedhoriMCUCount)
        {
            p_engine->rotatedhoriMCUIndex = 0;
            p_engine->rotatedvertMCUIndex++;
        }
        /* Check if the upper bound of internal bitstream buffer is reached */
        if (p_bitstream->nextBytePtr > p_bound)
        {
            /* Copy the bitstream to output buffer */
            jpege_engine_sw_copy_bytes(p_engine, 0); // Do not force flushing
        }
    }   // End MCU processing loop

    /* Check abort or error flag */
    os_mutex_lock(&(p_engine->mutex));

    if (p_engine->abort_flag)
    {
        p_engine->is_active = false;
        p_engine->abort_flag = false;
        p_engine->error_flag = false;
        os_cond_signal(&p_engine->cond);
        os_mutex_unlock(&(p_engine->mutex));
        //os_perf_toggle(JPEG_VENUM_ENC, JPEG_PERFORMANCE_NOT_REQD);
        return OS_THREAD_FUNC_RET_FAILED;
    }
    else
    {
        if (p_engine->error_flag)
        {
            p_engine->is_active = false;
            p_engine->error_flag = false;
            os_mutex_unlock(&(p_engine->mutex));
            // Throw encode error event
            p_engine->p_event_handler(p_engine->p_wrapper, JPEG_EVENT_ERROR, NULL);
            //os_perf_toggle(JPEG_VENUM_ENC, JPEG_PERFORMANCE_NOT_REQD);
            return OS_THREAD_FUNC_RET_FAILED;
        }
    }
    os_mutex_unlock(&(p_engine->mutex));

    // finish reset of encoding if not error or abort yet
    // Do Not Emit EOI, Hybrid Engine Will Emit.

    // Flush any remaining output that is not outputted
    jpege_engine_sw_copy_bytes(p_engine, 1); // Force flushing even if desination is not full

    os_mutex_lock(&(p_engine->mutex));
    // wait for final output
    if (p_engine->final_output_flag && p_engine->abort_flag != 1)
    {
        // Wait output thread to signal with completion final output buffer,
        // Then it will continue to finish encoding
        os_cond_wait(&(p_engine->final_output_cond), &(p_engine->mutex));
    }
    os_mutex_unlock(&(p_engine->mutex));

    // Set the active flag to inactive
    os_mutex_lock(&(p_engine->mutex));
    if (p_engine->abort_flag)
    {
        p_engine->is_active = false;
        p_engine->abort_flag = false;
        p_engine->error_flag = false;
        os_cond_signal(&p_engine->cond);
        os_mutex_unlock(&(p_engine->mutex));
    }
    else
    {
        if (p_engine->error_flag)
        {
            p_engine->is_active = false;
            p_engine->error_flag = false;
            os_mutex_unlock(&(p_engine->mutex));
            // Throw encode error event
            p_engine->p_event_handler(p_engine->p_wrapper, JPEG_EVENT_ERROR, NULL);
        }
        else
        {
            // Notify event done
            p_engine->is_active = false;
            os_mutex_unlock(&(p_engine->mutex));
            p_engine->p_event_handler(p_engine->p_wrapper, JPEG_EVENT_DONE, NULL);
        }
    }
    //os_perf_toggle(JPEG_VENUM_ENC, JPEG_PERFORMANCE_NOT_REQD);
    return OS_THREAD_FUNC_RET_SUCCEEDED;
}

static void jpege_engine_sw_fetch_dct_mcu(jpege_engine_sw_t *p_engine,
                                          int16_t *currMCULumaDctOutput,
                                          int16_t *currMCUChromaDctOutput)
{
    uint8_t *currMCUYStartAddr, *currMCUCbCrStartAddr;
    uint8_t *currBlockStartAddr;
    int16_t *currDctOutputPtr;
    uint32_t horiPixelIndex, vertPixelIndex;
    uint32_t vertMCUOffset, horiMCUOffset, vertBound, horiBound;
    int32_t  vertCross, horiCross;
    uint32_t i, j;

    /* fetch luma data */
    currDctOutputPtr = currMCULumaDctOutput;

    /* Calculate the modified origin for the luma data */
    horiPixelIndex = p_engine->rotatedhoriMCUIndex * p_engine->rotatedMCUWidth;
    vertPixelIndex = p_engine->rotatedvertMCUIndex * p_engine->rotatedMCUHeight;

    /* Use the MCU x and y coordinates from above to calculate the address of
     * the first pixel in the current MCU.
     */
    currMCUYStartAddr = p_engine->inputYPtr + (int32_t)p_engine->currOrigin_luma +
                        //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
                        (int32_t)horiPixelIndex * p_engine->horiIncrement_luma +
                        //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
                        (int32_t)vertPixelIndex * p_engine->vertIncrement_luma;

    /* Two layers of loops for the luma blocks within a MCU */
    vertMCUOffset = 0;

    for (i=0; i < p_engine->rotatedMCUHeight/BLOCK_HEIGHT; i++)
    {
        /* Calculate how many pixels are needed to pad vertically */
        vertCross = (int32_t)(p_engine->rotatedlumaHeight -
            (vertPixelIndex + vertMCUOffset + 1));
        if (vertCross >= BLOCK_HEIGHT) // No pad
        {
            vertBound = BLOCK_HEIGHT;
            currBlockStartAddr = currMCUYStartAddr +
                //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
                //int32_t*unit32 is not a problem.
                ((int32_t)vertMCUOffset * p_engine->vertIncrement_luma);
        }
        else if (vertCross >= 0) // Partially pad
        {
            //lint -e{632} Assignment to strong type 'uint32_t' in context: assignment
            //assignment to uint32_t is not a problem, since vertCross >= 0 here.
            vertBound = vertCross + 1;
            //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
            //int32_t*unit32 is not a problem.
            currBlockStartAddr = currMCUYStartAddr + (int32_t)vertMCUOffset *
                                 p_engine->vertIncrement_luma;
        }
        else // Fully pad
        {
            vertBound = 1;
            currBlockStartAddr = currMCUYStartAddr +
                                 ((int32_t)(vertMCUOffset - p_engine->rotatedMCUHeight) +
            //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
                                   (BLOCK_HEIGHT + vertCross)) * p_engine->vertIncrement_luma;
        }
        horiMCUOffset = 0;
        for (j=0; j < p_engine->rotatedMCUWidth/BLOCK_WIDTH; j++)
        {
            /* Calculate how many pixels are needed to pad horizontally */
            //lint -e{632} Assignment to strong type 'int32_t' in context: assignment
            horiCross = p_engine->rotatedlumaWidth - (horiPixelIndex + horiMCUOffset + 1);
            if (horiCross >= BLOCK_WIDTH) // No pad
            {
                horiBound = BLOCK_WIDTH;
                //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
                currBlockStartAddr += (int32_t)horiMCUOffset * p_engine->horiIncrement_luma;
            }
            else if (horiCross >= 0) // Partially pad
            {
                //lint -e{632} Assignment to strong type 'uint32_t' in context: assignment
                //assignment to uint32_t is not a problem, since horiCross >= 0 here.
                horiBound = horiCross + 1;
                //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
                currBlockStartAddr += (int32_t)horiMCUOffset * p_engine->horiIncrement_luma;
            }
            else // Fully pad
            {
                horiBound = 1;
                currBlockStartAddr += ((int32_t)(horiMCUOffset - p_engine->rotatedMCUWidth) +
                //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
                                        (BLOCK_WIDTH + horiCross)) * p_engine->horiIncrement_luma;
            }

            if ((vertBound == BLOCK_HEIGHT) && (horiBound == BLOCK_WIDTH))
            {
                jpege_engine_sw_fetch_dct_block_luma(
                        currBlockStartAddr, currDctOutputPtr, p_engine->stride_cfg.input_luma_stride);
            }
            else
            {
                jpege_engine_sw_fetch_dct_block_partial_luma(
                    p_engine, currBlockStartAddr, horiBound, vertBound, currDctOutputPtr);
            }

            currDctOutputPtr += JBLOCK_SIZE;
            horiMCUOffset += BLOCK_WIDTH;
        }
        vertMCUOffset += BLOCK_HEIGHT;
    }

    /* fetch chroma data */
    currDctOutputPtr = currMCUChromaDctOutput;

    /* Calculate the modified origin for the chroma data */
    horiPixelIndex = p_engine->rotatedhoriMCUIndex * BLOCK_WIDTH;
    vertPixelIndex = p_engine->rotatedvertMCUIndex * BLOCK_HEIGHT;
    currMCUCbCrStartAddr = p_engine->inputCbCrPtr + (int32_t)p_engine->currOrigin_chroma +
                //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
                           (int32_t)horiPixelIndex * p_engine->horiIncrement_chroma +
                //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
                           (int32_t)vertPixelIndex * p_engine->vertIncrement_chroma;

    /* Calculate how many pixels are needed to pad vertically */
    //lint -e{632} Assignment to strong type 'int32_t' in context: assignment
    vertCross = p_engine->rotatedchromaHeight - (vertPixelIndex + 1);
    if (vertCross >= BLOCK_HEIGHT)
    {
        vertBound = BLOCK_HEIGHT; // No pad
    }
    else
    {
        //lint -e{632} Assignment to strong type 'uint32_t' in context: assignment
        vertBound = vertCross + 1; // Partially Pad
    }
    /* Calculate how many pixels are needed to pad horizontally */
    //lint -e{632} Assignment to strong type 'int32_t' in context: assignment
    horiCross = p_engine->rotatedchromaWidth - (horiPixelIndex + 1);
    if (horiCross >= BLOCK_WIDTH)
    {
        horiBound = BLOCK_WIDTH; // No pad
    }
    else
    {
        //lint -e{632} Assignment to strong type 'uint32_t' in context: assignment
        horiBound = horiCross + 1; // Partially pad
    }

    if ((vertBound == BLOCK_HEIGHT) && (horiBound == BLOCK_WIDTH))
    {
        jpege_engine_sw_fetch_dct_block_chroma(
            currMCUCbCrStartAddr, currDctOutputPtr, (p_engine->stride_cfg.input_chroma_stride) / 2,
            p_engine->InputFormat);
    }
    else
    {
        jpege_engine_sw_fetch_dct_block_partial_chroma(
            p_engine, currMCUCbCrStartAddr, horiBound, vertBound, currDctOutputPtr);
    }
}
static void jpege_engine_sw_fetch_dct_mcu_packed(jpege_engine_sw_t *p_engine,
                                          int16_t *currMCULumaDctOutput,
                                          int16_t *currMCUChromaDctOutput)
{
    uint8_t *currMCUYStartAddr, *currMCUCbCrStartAddr;
    uint8_t *currBlockStartAddr;
    int16_t *currDctOutputPtr;
    uint32_t horiPixelIndex, vertPixelIndex;
    uint32_t vertMCUOffset, horiMCUOffset, vertBound, horiBound;
    int32_t  vertCross, horiCross;
    uint32_t i, j;

    // fetch luma data
    currDctOutputPtr = currMCULumaDctOutput;

    // Calculate the modified origin for the luma data
    horiPixelIndex = p_engine->rotatedhoriMCUIndex * p_engine->rotatedMCUWidth;
    vertPixelIndex = p_engine->rotatedvertMCUIndex * p_engine->rotatedMCUHeight;

    // Use the MCU x and y coordinates from above to calculate the address of
    // the first pixel in the current MCU.
    //
    currMCUYStartAddr = p_engine->inputYPtr + (int32_t)p_engine->currOrigin_luma +
                        //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
                        (int32_t)horiPixelIndex * p_engine->horiIncrement_luma +
                        //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
                        (int32_t)vertPixelIndex * p_engine->vertIncrement_luma;

    // Two layers of loops for the luma blocks within a MCU
    vertMCUOffset = 0;

    for (i=0; i < p_engine->rotatedMCUHeight/BLOCK_HEIGHT; i++)
    {
        // Calculate how many pixels are needed to pad vertically
        vertCross = (int32_t)(p_engine->rotatedlumaHeight -
            (vertPixelIndex + vertMCUOffset + 1));
        if (vertCross >= BLOCK_HEIGHT) // No pad
        {
            vertBound = BLOCK_HEIGHT;
            currBlockStartAddr = currMCUYStartAddr +
                //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
                //int32_t*unit32 is not a problem.
                ((int32_t)vertMCUOffset * p_engine->vertIncrement_luma);
        }
        else if (vertCross >= 0) // Partially pad
        {
            //lint -e{632} Assignment to strong type 'uint32_t' in context: assignment
            //assignment to uint32_t is not a problem, since vertCross >= 0 here.
            vertBound = vertCross + 1;
            //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
            //int32_t*unit32 is not a problem.
            currBlockStartAddr = currMCUYStartAddr + (int32_t)vertMCUOffset *
                                 p_engine->vertIncrement_luma;
        }
        else // Fully pad
        {
            vertBound = 1;
            currBlockStartAddr = currMCUYStartAddr +
                                 ((int32_t)(vertMCUOffset - p_engine->rotatedMCUHeight) +
            //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
                                   (BLOCK_HEIGHT + vertCross)) * p_engine->vertIncrement_luma;
        }
        horiMCUOffset = 0;
        for (j=0; j < p_engine->rotatedMCUWidth/BLOCK_WIDTH; j++)
        {
            // Calculate how many pixels are needed to pad horizontally
            //lint -e{632} Assignment to strong type 'int32_t' in context: assignment
            horiCross = p_engine->rotatedlumaWidth - (horiPixelIndex + horiMCUOffset + 1);
            if (horiCross >= BLOCK_WIDTH) // No pad
            {
                horiBound = BLOCK_WIDTH;
                //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
                currBlockStartAddr += (int32_t)horiMCUOffset * p_engine->horiIncrement_luma;
            }
            else if (horiCross >= 0) // Partially pad
            {
                //lint -e{632} Assignment to strong type 'uint32_t' in context: assignment
                //assignment to uint32_t is not a problem, since horiCross >= 0 here.
                horiBound = (horiCross + 1); // what should this be?
                //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
                currBlockStartAddr += (int32_t)horiMCUOffset * p_engine->horiIncrement_luma;
            }
            else // Fully pad
            {
                horiBound = 1;
                currBlockStartAddr += ((int32_t)(horiMCUOffset - p_engine->rotatedMCUWidth) +
                //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
                                        (BLOCK_WIDTH + horiCross)) * p_engine->horiIncrement_luma;
            }

            if ((vertBound == BLOCK_HEIGHT) && (horiBound == BLOCK_WIDTH))
            {
                jpege_engine_sw_fetch_dct_block_luma_packed(
                        currBlockStartAddr, currDctOutputPtr, p_engine->stride_cfg.input_luma_stride);
            }
            else
            {
                jpege_engine_sw_fetch_dct_block_partial_luma_packed(
                    p_engine, currBlockStartAddr, horiBound, vertBound, currDctOutputPtr);
            }

            currDctOutputPtr += JBLOCK_SIZE;
            horiMCUOffset += BLOCK_WIDTH;

       }
        vertMCUOffset += BLOCK_HEIGHT;
    }

    // fetch chroma data
    currDctOutputPtr = currMCUChromaDctOutput;

    // Calculate the modified origin for the chroma data
    horiPixelIndex = p_engine->rotatedhoriMCUIndex * BLOCK_WIDTH;
    vertPixelIndex = p_engine->rotatedvertMCUIndex * BLOCK_HEIGHT;
    currMCUCbCrStartAddr = p_engine->inputCbCrPtr + (int32_t)p_engine->currOrigin_chroma +
                //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
                           (int32_t)horiPixelIndex * p_engine->horiIncrement_chroma +
                //lint -e{639} Strong type mismatch for type 'uint32_t' in binary operation
                           (int32_t)vertPixelIndex * p_engine->vertIncrement_chroma;

    // Calculate how many pixels are needed to pad vertically
    //lint -e{632} Assignment to strong type 'int32_t' in context: assignment
    vertCross = p_engine->rotatedchromaHeight - (vertPixelIndex + 1);
    if (vertCross >= BLOCK_HEIGHT)
    {
        vertBound = BLOCK_HEIGHT; // No pad
    }
    else
    {
        //lint -e{632} Assignment to strong type 'uint32_t' in context: assignment
        vertBound = vertCross + 1; // Partially Pad
    }
    // Calculate how many pixels are needed to pad horizontally
    //lint -e{632} Assignment to strong type 'int32_t' in context: assignment
    horiCross = p_engine->rotatedchromaWidth - (horiPixelIndex + 1);
    if (horiCross >= BLOCK_WIDTH)
    {
        horiBound = BLOCK_WIDTH; // No pad
    }
    else
    {
        //lint -e{632} Assignment to strong type 'uint32_t' in context: assignment
        horiBound = horiCross + 1; // Partially pad
    }

    if ((vertBound == BLOCK_HEIGHT) && (horiBound == BLOCK_WIDTH))
    {
        jpege_engine_sw_fetch_dct_block_chroma_packed(
            currMCUCbCrStartAddr, currDctOutputPtr, (p_engine->stride_cfg.input_chroma_stride) / 2,
            p_engine->InputFormat);
    }
    else
    {
        jpege_engine_sw_fetch_dct_block_partial_chroma_packed(
            p_engine, currMCUCbCrStartAddr, horiBound, vertBound, currDctOutputPtr);
    }
}

static void jpege_engine_sw_fetch_dct_block_partial_luma(jpege_engine_sw_t *p_engine,
                                                         const uint8_t *p_luma_block_origin,
                                                         uint32_t       hori_bound,
                                                         uint32_t       vert_bound,
                                                         int16_t       *p_dct_output)
{
    const uint8_t *p_luma_block;
    const uint8_t *p_padding_line;
    uint32_t       luma_width = p_engine->lumaWidth;
    uint8_t        fetched_block[JBLOCK_SIZE];
    uint8_t       *p_fetched_block = fetched_block;
    uint32_t       temp_bound;
    uint32_t       i, j;

    luma_width = p_engine->stride_cfg.input_luma_stride;

    switch (p_engine->rotation)
    {
    case 90:
        p_luma_block = p_luma_block_origin + ((BLOCK_HEIGHT - hori_bound) * luma_width);
        // swap hori_bound and vert_bound
        temp_bound = hori_bound;
        hori_bound = vert_bound;
        vert_bound = temp_bound;
        break;

    case 180:
        p_luma_block = p_luma_block_origin + ((BLOCK_HEIGHT - vert_bound) * luma_width) +
                       (BLOCK_WIDTH  - hori_bound);
        break;

    case 270:
        p_luma_block = p_luma_block_origin + (BLOCK_WIDTH  - vert_bound);
        // swap horiBound and vertBound
        temp_bound = hori_bound;
        hori_bound = vert_bound;
        vert_bound = temp_bound;
        break;

    default:
        p_luma_block = p_luma_block_origin;
        break;
    }
    // Vertically unpadded part
    for (i = 0; i < vert_bound; i++)
    {
        // Horizontally unpadded part
        STD_MEMMOVE(p_fetched_block, p_luma_block, (hori_bound * sizeof(uint8_t)));
        p_fetched_block += hori_bound;

        // Horizontally padded part
        for (j = hori_bound; j < BLOCK_WIDTH; j++)
        {
            *p_fetched_block = *(p_fetched_block - 1);
            p_fetched_block++;
        }
        p_luma_block += luma_width;
    }

    // Vertically padded part
    p_padding_line = p_fetched_block - BLOCK_WIDTH;
    for (i = vert_bound; i < BLOCK_HEIGHT; i++)
    {
        STD_MEMMOVE(p_fetched_block, p_padding_line, (BLOCK_WIDTH * sizeof(uint8_t)));
        p_fetched_block += BLOCK_WIDTH;
        p_luma_block    += luma_width;
    }

    // Do DCT
    jpege_engine_sw_fdct_block(fetched_block, p_dct_output);
}

static void jpege_engine_sw_fetch_dct_block_partial_luma_packed(jpege_engine_sw_t *p_engine,
                                                         const uint8_t *p_luma_block_origin,
                                                         uint32_t       hori_bound,
                                                         uint32_t       vert_bound,
                                                         int16_t       *p_dct_output)
{
    const uint8_t *p_luma_block;
    const uint8_t *p_padding_line;
    uint32_t       luma_width;
    uint8_t        fetched_block[JBLOCK_SIZE];
    uint8_t       *p_fetched_block = fetched_block;
    uint32_t       temp_bound;
    uint32_t       i, j;

    luma_width = p_engine->stride_cfg.input_luma_stride;

    switch (p_engine->rotation)
    {
    case 90:
        p_luma_block = p_luma_block_origin + ((BLOCK_HEIGHT - hori_bound) * 2 * luma_width);
        // swap hori_bound and vert_bound
        temp_bound = hori_bound;
        hori_bound = vert_bound;
        vert_bound = temp_bound;
        break;

    case 180:
        p_luma_block = p_luma_block_origin + ((BLOCK_HEIGHT - vert_bound) * luma_width * 2) +
                       (BLOCK_WIDTH  - hori_bound) * 2;
        break;

    case 270:
        p_luma_block = p_luma_block_origin + (BLOCK_WIDTH  - vert_bound) * 2;
        // swap horiBound and vertBound
        temp_bound = hori_bound;
        hori_bound = vert_bound;
        vert_bound = temp_bound;
        break;

    default:
        p_luma_block = p_luma_block_origin;
        break;
    }
    // Vertically unpadded part
    for (i = 0; i < vert_bound; i++)
    {
        // Horizontally unpadded part
        for(j=0; j < hori_bound * 2; j+=2)
        {
            *(p_fetched_block + j/2) = *(p_luma_block + j);
        }
        p_fetched_block += hori_bound;

        // Horizontally padded part
        for (j = hori_bound; j < BLOCK_WIDTH; j++)
        {
            *p_fetched_block = *(p_fetched_block - 1);
            p_fetched_block++;
        }
        p_luma_block += 2 * luma_width;
    }

    // Vertically padded part
    p_padding_line = p_fetched_block - BLOCK_WIDTH;
    for (i = vert_bound; i < BLOCK_HEIGHT; i++)
    {

        STD_MEMMOVE(p_fetched_block, p_padding_line, (BLOCK_WIDTH * sizeof(uint8_t)));
        p_fetched_block += BLOCK_WIDTH;
        p_luma_block    += 2 * luma_width;
    }

    // Do DCT
    jpege_engine_sw_fdct_block(fetched_block, p_dct_output);
}

static void jpege_engine_sw_fetch_dct_block_partial_chroma(jpege_engine_sw_t *p_engine,
                                                           const uint8_t     *p_chroma_block_origin,
                                                           uint32_t           hori_bound,
                                                           uint32_t           vert_bound,
                                                           int16_t           *p_dct_output)
{
    const uint8_t *p_chroma_block;
    const uint8_t *p_padding_line;
    uint32_t       chroma_width;
    uint8_t        fetched_block[JBLOCK_SIZE * NUM_CHROMA_BLOCKS]; // chroma is interleaved
    uint8_t        deinterleaved_block[JBLOCK_SIZE * NUM_CHROMA_BLOCKS];
    uint8_t       *p_fetched_block = fetched_block;
    uint8_t       *p_deinterleaved_block1 = deinterleaved_block;
    uint8_t       *p_deinterleaved_block2 = p_deinterleaved_block1 + JBLOCK_SIZE;
    uint8_t       *p_cb_block, *p_cr_block;
    uint32_t       temp_bound;
    uint32_t       i, j;

    chroma_width = (p_engine->stride_cfg.input_chroma_stride) / 2;

    switch (p_engine->rotation)
    {
    case 90:
        p_chroma_block = p_chroma_block_origin +
                         ((BLOCK_HEIGHT - hori_bound) * (chroma_width * 2));
        // swap hori_bound and vert_bound
        temp_bound = hori_bound;
        hori_bound = vert_bound;
        vert_bound = temp_bound;
        break;

    case 180:
        p_chroma_block = p_chroma_block_origin +
                         ((BLOCK_HEIGHT - vert_bound) * (chroma_width * 2)) +
                         ((BLOCK_WIDTH  - hori_bound) * 2);
        break;

    case 270:
        p_chroma_block = p_chroma_block_origin + ((BLOCK_WIDTH  - vert_bound) * 2);
        // swap hori_bound and vert_bound
        temp_bound = hori_bound;
        hori_bound = vert_bound;
        vert_bound = temp_bound;
        break;

    default:
        p_chroma_block = p_chroma_block_origin;
        break;
    }

    // Vertically unpadded part
    for (i = 0; i < vert_bound; i++)
    {
        // Horizontally unpadded part
        STD_MEMMOVE(p_fetched_block, p_chroma_block, (hori_bound * sizeof(uint8_t) * 2));
        p_fetched_block += (hori_bound * 2);

        // Horizontally padded part
        for (j = hori_bound; j < BLOCK_WIDTH; j++)
        {
            *(p_fetched_block)     = *(p_fetched_block - 2);
            *(p_fetched_block + 1) = *(p_fetched_block - 1);
            p_fetched_block += 2;
        }
        p_chroma_block += (chroma_width * 2);
    }

    // Vertically padded part
    p_padding_line = p_fetched_block - (BLOCK_WIDTH * 2);
    for (i = vert_bound; i < BLOCK_HEIGHT; i++)
    {
        STD_MEMMOVE(p_fetched_block, p_padding_line, (BLOCK_WIDTH * sizeof(uint8_t) * 2));
        p_fetched_block += (BLOCK_WIDTH * 2);
        p_chroma_block  += (chroma_width * 2);
    }

    // Deinterleave chroma block
    for (i = 0; i < BLOCK_HEIGHT; i++)
    {
        for (j = 0; j < BLOCK_WIDTH; j++)
        {
            *p_deinterleaved_block1++ = fetched_block[i*BLOCK_WIDTH*2+j*2];
            *p_deinterleaved_block2++ = fetched_block[i*BLOCK_WIDTH*2+j*2+1];
        }
    }

    // Dual Format Selection
    if (p_engine->InputFormat == YCrCb)
    {
        p_cb_block = deinterleaved_block + JBLOCK_SIZE;
        p_cr_block = deinterleaved_block;
    }
    else
    {
        p_cb_block = deinterleaved_block;
        p_cr_block = deinterleaved_block + JBLOCK_SIZE;
    }

    // Do DCT - always in the order of CbCr
    jpege_engine_sw_fdct_block(p_cb_block, p_dct_output);
    jpege_engine_sw_fdct_block(p_cr_block, (p_dct_output + JBLOCK_SIZE));
}

static void jpege_engine_sw_fetch_dct_block_partial_chroma_packed(jpege_engine_sw_t *p_engine,
                                                           const uint8_t     *p_chroma_block_origin,
                                                           uint32_t           hori_bound,
                                                           uint32_t           vert_bound,
                                                           int16_t           *p_dct_output)
{
    const uint8_t *p_chroma_block;
    const uint8_t *p_padding_line;
    uint32_t       chroma_width;
    uint8_t        fetched_block[JBLOCK_SIZE * NUM_CHROMA_BLOCKS] = {0}; // chroma is interleaved
    uint8_t        deinterleaved_block[JBLOCK_SIZE * NUM_CHROMA_BLOCKS];
    uint8_t       *p_fetched_block = fetched_block;
    uint8_t       *p_deinterleaved_block1 = deinterleaved_block;
    uint8_t       *p_deinterleaved_block2 = p_deinterleaved_block1 + JBLOCK_SIZE;
    uint8_t       *p_cb_block, *p_cr_block;
    uint32_t       temp_bound;
    uint32_t       i, j;

    chroma_width = (p_engine->stride_cfg.input_chroma_stride) / 2;

    switch (p_engine->rotation)
    {
    case 90:
        p_chroma_block = p_chroma_block_origin +
                         ((BLOCK_HEIGHT - hori_bound) * (chroma_width * 4));
        // swap hori_bound and vert_bound
        temp_bound = hori_bound;
        hori_bound = vert_bound;
        vert_bound = temp_bound;
        break;

    case 180:
        p_chroma_block = p_chroma_block_origin +
                         ((BLOCK_HEIGHT - vert_bound) * (chroma_width * 4)) +
                         ((BLOCK_WIDTH  - hori_bound) * 4);
        break;

    case 270:
        p_chroma_block = p_chroma_block_origin + ((BLOCK_WIDTH  - vert_bound) * 4);
        // swap hori_bound and vert_bound
        temp_bound = hori_bound;
        hori_bound = vert_bound;
        vert_bound = temp_bound;
        break;

    default:
        p_chroma_block = p_chroma_block_origin;
        break;
    }

    // Vertically unpadded part
    for (i = 0; i < vert_bound; i++)
    {
        // Horizontally unpadded part
        for(j=0; j < hori_bound * 4; j+=2)
        {
            *(p_fetched_block + j/2) = *(p_chroma_block + j);
        }
        p_fetched_block += (hori_bound * 2);

        // Horizontally padded part
        for (j = hori_bound; j < BLOCK_WIDTH; j++)
        {
            *(p_fetched_block)     = *(p_fetched_block - 2);
            *(p_fetched_block + 1) = *(p_fetched_block - 1);
            p_fetched_block += 2;
        }
        p_chroma_block += (chroma_width * 4);
    }

    // Vertically padded part
    p_padding_line = p_fetched_block - (BLOCK_WIDTH * 2);
    for (i = vert_bound; i < BLOCK_HEIGHT; i++)
    {
        STD_MEMMOVE(p_fetched_block, p_padding_line, (BLOCK_WIDTH * sizeof(uint8_t) * 2));
        p_fetched_block += (BLOCK_WIDTH * 2);
        p_chroma_block  += (chroma_width * 4);
    }

    // Deinterleave chroma block
    for (i = 0; i < BLOCK_HEIGHT; i++)
    {
        for (j = 0; j < BLOCK_WIDTH; j++)
        {
            *p_deinterleaved_block1++ = fetched_block[i*BLOCK_WIDTH*2+j*2];
            *p_deinterleaved_block2++ = fetched_block[i*BLOCK_WIDTH*2+j*2+1];
        }
    }

    p_cb_block = deinterleaved_block;
    p_cr_block = deinterleaved_block + JBLOCK_SIZE;

    // Do DCT - always in the order of CbCr
    jpege_engine_sw_fdct_block(p_cb_block, p_dct_output);
    jpege_engine_sw_fdct_block(p_cr_block, (p_dct_output + JBLOCK_SIZE));
}

static void jpege_engine_sw_copy_bytes(jpege_engine_sw_t *p_engine, uint8_t force_flush)
{
    bitstream_t           *p_bitstream = &(p_engine->jpegeBitStreamState);
    uint32_t               bytes_to_copy, bytes_remaining, bytes_written = 0; //buffer sizes limited to 32 bit
    int                    rc;

         // Get the next output buffer from buffer(s) queue if necessary
    bytes_remaining = (uint32_t)(p_bitstream->nextBytePtr - p_bitstream->buffer);

        // Calculate how much can be written to the destination buffer
    bytes_to_copy = STD_MIN(bytes_remaining, (p_engine->output_buffer_length - p_engine->output_buffer_offset));

        // Copy the data from bitstream buffer to destination buffer starting from the offset
    STD_MEMMOVE(p_engine->output_buffer_ptr + p_engine->output_buffer_offset, p_bitstream->buffer + bytes_written, bytes_to_copy);

        // Update destination buffer offset and other state variables
    p_engine->output_buffer_offset += bytes_to_copy;
        bytes_remaining -= bytes_to_copy;
        bytes_written += bytes_to_copy;

        // After copying, if flush is forced or the destination buffer is full, deliver it
    if (force_flush)
    {
                // Set the final output flag:
                // So this output can set is_last_buffer flag in flush output function,
                // and output thread will signal encode thread if is_last_buffer flag is set.
                // while encode thread waits the output thread to output it
                // if final_output_flag is set
                os_mutex_lock(&(p_engine->mutex));
                p_engine->final_output_flag = true;
                os_mutex_unlock(&(p_engine->mutex));

        // Flush the current destination buffer to upper layer
        rc = p_engine->p_output_handler(p_engine->p_wrapper,
                                        p_engine->output_buffer_ptr,
                                        p_engine->output_buffer_offset);

            if (JPEG_FAILED(rc))
            {
                JPEG_DBG_ERROR("jpege_engine_sw_copy_bytes: failed to flush output buffer\n");

                os_mutex_lock(&(p_engine->mutex));
                p_engine->error_flag = 1;
                os_mutex_unlock(&(p_engine->mutex));
                JPEG_DBG_ERROR("jpege_engine_sw_copy_bytes: set error flag, return.\n");
                return ;
            }
            // Clean up the local output buffer ref
        os_cond_signal(&(p_engine->final_output_cond));
        p_engine->final_output_flag = false;
    }

    // Reset internal bitstream buffer
    p_bitstream->nextBytePtr = p_bitstream->buffer;

} /* jpege_engine_sw_copy_bytes() */

static int jpege_engine_sw_configure_internal_buffer(
    jpege_engine_sw_t         *p_engine,
    jpege_engine_hybrid_cfg_t *p_cfg)
{
    bitstream_t *p_bitstream = &(p_engine->jpegeBitStreamState);

    // Output buffers not allocated for FastCV scaling so do not check
    if(!(p_engine->fastCV_flag)) {
        if (!p_cfg->output_buffer_ptr)
        {
            JPEG_DBG_ERROR("jpege_engine_sw_configure_internal_buffer: output buffer NULL)\n");
            return JPEGERR_ENULLPTR;
        }
    }
    p_engine->output_buffer_ptr = p_cfg->output_buffer_ptr;
    p_engine->output_buffer_length = p_cfg->output_buffer_length;
    p_engine->output_buffer_offset = 0;

    /* Free previously allocated internal bitstream buffer first */
    if (p_bitstream->buffer)
    {
        JPEG_FREE(p_bitstream->buffer);
        STD_ZEROAT(p_bitstream);
    }
    /* Allocate the internal bitstream buffer with size INTERNAL_BUFFER_SIZE */
    p_bitstream->buffer = (uint8_t *)JPEG_MALLOC(INTERNAL_BUFFER_SIZE * sizeof (uint8_t));
    if (!p_bitstream->buffer) return JPEGERR_EMALLOC;
    p_bitstream->buffersize = INTERNAL_BUFFER_SIZE;
    p_bitstream->nextBytePtr = p_bitstream->buffer;
    /* clear bit-stream assembly buffer */
    p_bitstream->bitAssemblyBufferUL32 = 0;
    /* init emptyBitLocation to 32 */
    p_bitstream->bitsFree = 32;
    return JPEGERR_SUCCESS;
}

static int jpege_engine_sw_configure_parameter(jpege_engine_sw_t  *p_engine)
{
    /*-----------------------------------------------------------------------
      Calculate Chroma Width and Height
    -----------------------------------------------------------------------*/
    switch (p_engine->subsampling)
    {
    case JPEG_H1V1:
    {
        p_engine->chromaWidth = p_engine->lumaWidth;
        p_engine->chromaHeight = p_engine->lumaHeight;
        p_engine->MCUWidth = 8;
        p_engine->MCUHeight = 8;
        p_engine->numLumaBlocks = H1V1_NUM_LUMA_BLOCKS;
        break;
    }
    case JPEG_H1V2:
    {
        p_engine->chromaWidth = p_engine->lumaWidth;
        p_engine->chromaHeight = (p_engine->lumaHeight + 1) >> 1; // odd luma support
        p_engine->MCUWidth = 8;
        p_engine->MCUHeight = 16;
        p_engine->numLumaBlocks = H1V2_NUM_LUMA_BLOCKS;
        break;
    }
    case JPEG_H2V1:
    case JPEG_YUY2:
    case JPEG_UYVY:
    {
        p_engine->chromaWidth = (p_engine->lumaWidth + 1) >> 1; // odd luma support
        p_engine->chromaHeight = p_engine->lumaHeight;
        p_engine->MCUWidth = 16;
        p_engine->MCUHeight = 8;
        p_engine->numLumaBlocks = H2V1_NUM_LUMA_BLOCKS;
        break;
    }
    case JPEG_H2V2:
    {
        p_engine->chromaWidth = (p_engine->lumaWidth + 1) >> 1; // odd luma support
        p_engine->chromaHeight = (p_engine->lumaHeight + 1) >> 1; // odd luma support
        p_engine->MCUWidth = 16;
        p_engine->MCUHeight = 16;
        p_engine->numLumaBlocks = H2V2_NUM_LUMA_BLOCKS;
        break;
    }
    default:
        JPEG_DBG_ERROR("jpege_engine_sw_configure: invalid jpeg subsampling: %d\n", p_engine->subsampling);
        return JPEGERR_EBADPARM;
    }

    /*-----------------------------------------------------------------------
     Calculate the horizontal and vertical MCU counts
    -----------------------------------------------------------------------*/
    // divide by 8 calculate the amounts of MCUs
    p_engine->horiMCUCount = (p_engine->chromaWidth + 7) >> 3;
    p_engine->vertMCUCount = (p_engine->chromaHeight + 7) >> 3; // with padding support

    /*-----------------------------------------------------------------------
      Initialize horizontal and vertical MCU indices to 0
    -----------------------------------------------------------------------*/
    p_engine->horiMCUIndex = 0;
    p_engine->vertMCUIndex = 0;

    /*-----------------------------------------------------------------------
     Reset DC prediction values
    -----------------------------------------------------------------------*/
    p_engine->prevLumaDC = 0;
    p_engine->prevCbDC = 0;
    p_engine->prevCrDC = 0;

    /*-----------------------------------------------------------------------
      Restart MCU Count and Restart Marker Mod Count Initialized by Hybrid Layer
    -----------------------------------------------------------------------*/
    return JPEGERR_SUCCESS;
}

static int jpege_engine_sw_configure_rotation(jpege_engine_sw_t *p_engine,
                                              jpege_img_cfg_t   *p_config)
{
    uint32_t  lumaWidth       = p_engine->lumaWidth;
    uint32_t  lumaHeight      = p_engine->lumaHeight;
    uint32_t  chromaWidth     = p_engine->chromaWidth;
    uint32_t  chromaHeight    = p_engine->chromaHeight;
    uint16_t *lumaQTablePtr   = (uint16_t *)p_config->luma_quant_tbl;
    uint16_t *chromaQTablePtr = (uint16_t *)p_config->chroma_quant_tbl;
    const int16_t *zigzagTablePtr;
    uint16_t  qFactorInvQ15;
    uint32_t  i, j;

    switch (p_engine->rotation)
    {
    case 0:
    {
        /* setup real geometry parameters */
        p_engine->rotatedMCUWidth      = p_engine->MCUWidth;
        p_engine->rotatedMCUHeight     = p_engine->MCUHeight;
        p_engine->rotatedhoriMCUCount  = (chromaWidth + 7) >> 3;  // divide by 8
        p_engine->rotatedvertMCUCount  = (chromaHeight + 7) >> 3; // divide by 8
        p_engine->rotatedlumaWidth     = lumaWidth;
        p_engine->rotatedlumaHeight    = lumaHeight;
        p_engine->rotatedchromaWidth   = chromaWidth;
        p_engine->rotatedchromaHeight  = chromaHeight;
        /* setup Origin */
        p_engine->currOrigin_luma      = 0;
        p_engine->currOrigin_chroma    = 0;
        /* setup relative step */
        p_engine->horiIncrement_luma   = 1;
            /*In case of stride enabled, increment is configured to stride set by user. In case of stride disabled, it is set to the width*/
        p_engine->vertIncrement_luma = p_engine->stride_cfg.input_luma_stride;
        p_engine->horiIncrement_chroma = 2;
        p_engine->vertIncrement_chroma = p_engine->stride_cfg.input_chroma_stride;
        /* Calculate zigzag table and quant table for rotation */
        /* for 0 degree rotation, use original zigzag table */
        zigzagTablePtr = zigzagTable;
        /* for 0 degree rotation, use original quant tables */
        for (i = 0; i < 8; i++)
        {
            for (j = 0; j < 8; j++)
            {
                qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[i*8+j] * ((1<<(FIX_POINT_FACTOR-3))-1) + 0.5);
                p_engine->lumaQuantTableQ16[i*8+j] = (int16_t)qFactorInvQ15;
                qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[i*8+j] * ((1<<(FIX_POINT_FACTOR-3))-1) + 0.5);
                p_engine->chromaQuantTableQ16[i*8+j] = (int16_t)qFactorInvQ15;
            }
        }
        break;
    }
    case 90:
    {
        /* setup real geometry parameters */
        p_engine->rotatedMCUWidth      = p_engine->MCUHeight;
        p_engine->rotatedMCUHeight     = p_engine->MCUWidth;
        p_engine->rotatedhoriMCUCount  = (chromaHeight + 7) >> 3; // divide by 8
        p_engine->rotatedvertMCUCount  = (chromaWidth + 7) >> 3;  // divide by 8
        p_engine->rotatedlumaWidth     = lumaHeight;
        p_engine->rotatedlumaHeight    = lumaWidth;
        p_engine->rotatedchromaWidth   = chromaHeight;
        p_engine->rotatedchromaHeight  = chromaWidth;
        /* setup Origin */
        p_engine->currOrigin_luma = (lumaHeight - BLOCK_HEIGHT) * p_engine->stride_cfg.input_luma_stride;
        p_engine->currOrigin_chroma = (chromaHeight - BLOCK_HEIGHT) * p_engine->stride_cfg.input_chroma_stride;
        /* setup relative step */
            /*In case of stride enabled, increment is configured to stride set by user. In case of stride disabled, it is set to the width*/
        p_engine->horiIncrement_luma   = (int32_t)(0 - p_engine->stride_cfg.input_luma_stride);
        p_engine->vertIncrement_luma   = 1;
        p_engine->horiIncrement_chroma = (-1 * (int32_t)(p_engine->stride_cfg.input_chroma_stride));
        p_engine->vertIncrement_chroma = 2;
        /* calculate zigzag table and quant table for rotation */
        /* for 90 degree rotation, transpose zigzag table */
        zigzagTablePtr = zigzagTableTranspose;
        /* for 90 degree rotation, transpose quant tables */
        for (i = 0; i < 8; i++)
        {
            for (j = 0; j < 8; j++)
            {
                qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[i*8+j] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->lumaQuantTableQ16[j*8+i] = (int16_t)qFactorInvQ15;
                qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[i*8+j] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->chromaQuantTableQ16[j*8+i] = (int16_t)qFactorInvQ15;
            }
        }
        /* then flip the sign of odd quant row */
        for (i = 1; i < 8; i+=2)
        {
            for (j = 0; j < 8; j++)
            {
                p_engine->lumaQuantTableQ16[i*8+j] = -p_engine->lumaQuantTableQ16[i*8+j];
                p_engine->chromaQuantTableQ16[i*8+j] = -p_engine->chromaQuantTableQ16[i*8+j];
            }
        }
        break;
    }
    case 180:
    {
        /* setup real geometry parameters */
        p_engine->rotatedMCUWidth      = p_engine->MCUWidth;
        p_engine->rotatedMCUHeight     = p_engine->MCUHeight;
        p_engine->rotatedhoriMCUCount  = (chromaWidth + 7) >> 3;  // divide by 8
        p_engine->rotatedvertMCUCount  = (chromaHeight + 7) >> 3; // divide by 8
        p_engine->rotatedlumaWidth     = lumaWidth;
        p_engine->rotatedlumaHeight    = lumaHeight;
        p_engine->rotatedchromaWidth   = chromaWidth;
        p_engine->rotatedchromaHeight  = chromaHeight;
        /* setup Origin */
        p_engine->currOrigin_luma = (lumaHeight - BLOCK_HEIGHT) * p_engine->stride_cfg.input_luma_stride +
                                         (lumaWidth - BLOCK_WIDTH);
        p_engine->currOrigin_chroma = (chromaHeight - BLOCK_HEIGHT) * p_engine->stride_cfg.input_chroma_stride +
                                         (chromaWidth - BLOCK_WIDTH) * 2;
        /* setup relative step */
            /*In case of stride enabled, increment is configured to stride set by user. In case of stride disabled, it is set to the width*/
        p_engine->horiIncrement_luma   = -1;
        p_engine->vertIncrement_luma   = (int32_t)(0 - p_engine->stride_cfg.input_luma_stride);
        p_engine->horiIncrement_chroma = -2;
        p_engine->vertIncrement_chroma = (-2 * (int32_t)(p_engine->stride_cfg.input_chroma_stride / 2));
        /* calculate zigzag table and quant table for rotation */
        /* for 180 degree rotation, use original zigzag table */
        zigzagTablePtr = zigzagTable;
        /* for 180 degree rotation, flip odd entries for quant tables */
        for (i = 0; i < 7; i+=2)
        {
            for (j = 0; j < 7; j+=2)
            {
                qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[i*8+j] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->lumaQuantTableQ16[i*8+j] = (int16_t)qFactorInvQ15;
                qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[i*8+j+1] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->lumaQuantTableQ16[i*8+j+1] = -(int16_t)qFactorInvQ15;

                qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[i*8+j] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->chromaQuantTableQ16[i*8+j] = (int16_t)qFactorInvQ15;
                qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[i*8+j+1] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->chromaQuantTableQ16[i*8+j+1] = -(int16_t)qFactorInvQ15;
            }
            for (j = 0; j < 7; j+=2)
            {
                qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[(i+1)*8+j] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->lumaQuantTableQ16[(i+1)*8+j] = -(int16_t)qFactorInvQ15;
                qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[(i+1)*8+j+1] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->lumaQuantTableQ16[(i+1)*8+j+1] = (int16_t)qFactorInvQ15;

                qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[(i+1)*8+j] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->chromaQuantTableQ16[(i+1)*8+j] = -(int16_t)qFactorInvQ15;
                qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[(i+1)*8+j+1] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->chromaQuantTableQ16[(i+1)*8+j+1] = (int16_t)qFactorInvQ15;
            }
        }
        break;
    }
    case 270:
    {
        /* setup real geometry parameters */
        p_engine->rotatedMCUWidth      = p_engine->MCUHeight;
        p_engine->rotatedMCUHeight     = p_engine->MCUWidth;
        p_engine->rotatedhoriMCUCount  = (chromaHeight + 7) >> 3; // divide by 8
        p_engine->rotatedvertMCUCount  = (chromaWidth + 7) >> 3;  // divide by 8
        p_engine->rotatedlumaWidth     = lumaHeight;
        p_engine->rotatedlumaHeight    = lumaWidth;
        p_engine->rotatedchromaWidth   = chromaHeight;
        p_engine->rotatedchromaHeight  = chromaWidth;
        /* setup Origin */
        p_engine->currOrigin_luma      = (lumaWidth - BLOCK_WIDTH);
        p_engine->currOrigin_chroma    = (chromaWidth - BLOCK_WIDTH) * 2;
        /* setup relative step */
            /*In case of stride enabled, increment is configured to stride set by user. In case of stride disabled, it is set to the width*/
        p_engine->horiIncrement_luma   = p_engine->stride_cfg.input_luma_stride;
        p_engine->vertIncrement_luma   = -1;
        p_engine->horiIncrement_chroma = (p_engine->stride_cfg.input_chroma_stride);
        p_engine->vertIncrement_chroma = -2;
        /* calculate zigzag table and quant table for rotation */
        /* for 270 degree rotation, transpose zigzag table */
        zigzagTablePtr = zigzagTableTranspose;
        /* for 270 degree rotation, transpose quant tables */
        for (i = 0; i < 8; i++)
        {
            for (j = 0; j < 8; j++)
            {
                qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[i*8+j] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->lumaQuantTableQ16[j*8+i] = (int16_t)qFactorInvQ15;
                qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[i*8+j] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->chromaQuantTableQ16[j*8+i] = (int16_t)qFactorInvQ15;
            }
        }
        /* then flip sign of odd quant col */
        for (i = 0; i < 8; i++)
        {
            for (j = 1; j < 8; j+=2)
            {
                p_engine->lumaQuantTableQ16[i*8+j] = -p_engine->lumaQuantTableQ16[i*8+j];
                p_engine->chromaQuantTableQ16[i*8+j] = -p_engine->chromaQuantTableQ16[i*8+j];
            }
        }
        break;
    }
    default:
        JPEG_DBG_ERROR("jpege_engine_sw_configure: unrecognized rotation: %d\n", p_engine->rotation);
        return JPEGERR_EBADPARM;
    }
    /* Calculate the zigzagOffsetTable, the offset is the relative BYTES !!!
     * between DCTOutput[i] and DCTOutput[i+1] (which is int16_t), this is due to
     * the limitation of ARM LDRSH instruction which does not allow <opsh>, like:
     *
     * LDRSH Rd, [Rn], Rm, LSL #1
     *
     * and this zigzagOffsetTable is scanned decrementally, that is from
     * 63 down to 0.
     */
    for (i = JBLOCK_SIZE_MINUS_1; i != 0; i--)
    {
        p_engine->zigzagOffsetTable[i] =
            (zigzagTablePtr[i - 1] - zigzagTablePtr[i]) * sizeof(int16_t);
    }
    p_engine->zigzagOffsetTable[0] = 0;

    return JPEGERR_SUCCESS;
}

static int jpege_engine_sw_configure_rotation_packed(jpege_engine_sw_t *p_engine,
                                              jpege_img_cfg_t   *p_config)
{
    uint32_t  lumaWidth       = p_engine->lumaWidth;
    uint32_t  lumaHeight      = p_engine->lumaHeight;
    uint32_t  chromaWidth     = p_engine->chromaWidth;
    uint32_t  chromaHeight    = p_engine->chromaHeight;
    uint16_t *lumaQTablePtr   = (uint16_t *)p_config->luma_quant_tbl;
    uint16_t *chromaQTablePtr = (uint16_t *)p_config->chroma_quant_tbl;
    const int16_t *zigzagTablePtr;
    uint16_t  qFactorInvQ15;
    uint32_t  i, j;

    switch (p_engine->rotation)
    {
    case 0:
    {
        /* setup real geometry parameters */
        p_engine->rotatedMCUWidth      = p_engine->MCUWidth;
        p_engine->rotatedMCUHeight     = p_engine->MCUHeight;
        p_engine->rotatedhoriMCUCount  = (chromaWidth+ 7) >> 3;  // divide by 8
        p_engine->rotatedvertMCUCount  = (chromaHeight + 7) >> 3; // divide by 8
        p_engine->rotatedlumaWidth     = lumaWidth;
        p_engine->rotatedlumaHeight    = lumaHeight;
        p_engine->rotatedchromaWidth   = chromaWidth;
        p_engine->rotatedchromaHeight  = chromaHeight;
        /* setup Origin */
        p_engine->currOrigin_luma      = 0;
        p_engine->currOrigin_chroma    = 0;
        /* setup relative step */
            /*In case of stride enabled, increment is configured to stride set by user. In case of stride disabled, it is set to the width*/
        p_engine->horiIncrement_luma   = 2;
        p_engine->vertIncrement_luma   = 2 * (p_engine->stride_cfg.input_luma_stride);
        p_engine->horiIncrement_chroma = 4;
        p_engine->vertIncrement_chroma = 4 * (p_engine->stride_cfg.input_chroma_stride / 2);
        /* Calculate zigzag table and quant table for rotation */
        /* for 0 degree rotation, use original zigzag table */
        zigzagTablePtr = zigzagTable;
        /* for 0 degree rotation, use original quant tables */
        for (i = 0; i < 8; i++)
        {
            for (j = 0; j < 8; j++)
            {
                qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[i*8+j] * ((1<<(FIX_POINT_FACTOR-3))-1) + 0.5);
                p_engine->lumaQuantTableQ16[i*8+j] = (int16_t)qFactorInvQ15;
                qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[i*8+j] * ((1<<(FIX_POINT_FACTOR-3))-1) + 0.5);
                p_engine->chromaQuantTableQ16[i*8+j] = (int16_t)qFactorInvQ15;
            }
        }
        break;
    }
    case 90:
    {
        /* setup real geometry parameters */
        p_engine->rotatedMCUWidth      = p_engine->MCUHeight;
        p_engine->rotatedMCUHeight     = p_engine->MCUWidth;
        p_engine->rotatedhoriMCUCount  = (chromaHeight + 7) >> 3; // divide by 8
        p_engine->rotatedvertMCUCount  = (chromaWidth + 7) >> 3;  // divide by 8
        p_engine->rotatedlumaWidth     = lumaHeight;
        p_engine->rotatedlumaHeight    = lumaWidth;
        p_engine->rotatedchromaWidth   = chromaHeight;
        p_engine->rotatedchromaHeight  = chromaWidth;
        /* setup Origin */
        p_engine->currOrigin_luma   = (lumaHeight - BLOCK_HEIGHT) * (p_engine->stride_cfg.input_luma_stride) * 2;
        p_engine->currOrigin_chroma = (lumaHeight - BLOCK_HEIGHT) * (p_engine->stride_cfg.input_chroma_stride) * 2;
        /* setup relative step */
            /*In case of stride enabled, increment is configured to stride set by user. In case of stride disabled, it is set to the width*/
        p_engine->horiIncrement_luma   = (int32_t)((0 - (p_engine->stride_cfg.input_luma_stride)) * 2);
        p_engine->vertIncrement_luma   = 2;
        p_engine->horiIncrement_chroma = (-4 * (int32_t)(p_engine->stride_cfg.input_chroma_stride / 2));
        p_engine->vertIncrement_chroma = 4;
        /* calculate zigzag table and quant table for rotation */
        /* for 90 degree rotation, transpose zigzag table */
        zigzagTablePtr = zigzagTableTranspose;
        /* for 90 degree rotation, transpose quant tables */
        for (i = 0; i < 8; i++)
        {
            for (j = 0; j < 8; j++)
            {
                qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[i*8+j] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->lumaQuantTableQ16[j*8+i] = (int16_t)qFactorInvQ15;
                qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[i*8+j] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->chromaQuantTableQ16[j*8+i] = (int16_t)qFactorInvQ15;
            }
        }
        /* then flip the sign of odd quant row */
        for (i = 1; i < 8; i+=2)
        {
            for (j = 0; j < 8; j++)
            {
                p_engine->lumaQuantTableQ16[i*8+j] = -p_engine->lumaQuantTableQ16[i*8+j];
                p_engine->chromaQuantTableQ16[i*8+j] = -p_engine->chromaQuantTableQ16[i*8+j];
            }
        }
        break;
    }
    case 180:
    {
        /* setup real geometry parameters */
        p_engine->rotatedMCUWidth      = p_engine->MCUWidth;
        p_engine->rotatedMCUHeight     = p_engine->MCUHeight;
        p_engine->rotatedhoriMCUCount  = (chromaWidth + 7) >> 3;  // divide by 8
        p_engine->rotatedvertMCUCount  = (chromaHeight + 7) >> 3; // divide by 8
        p_engine->rotatedlumaWidth     = lumaWidth;
        p_engine->rotatedlumaHeight    = lumaHeight;
        p_engine->rotatedchromaWidth   = chromaWidth;
        p_engine->rotatedchromaHeight  = chromaHeight;
        /* setup Origin */
        p_engine->currOrigin_luma   = (lumaHeight - BLOCK_HEIGHT) * (p_engine->stride_cfg.input_luma_stride) * 2 +
                                         (lumaWidth - BLOCK_WIDTH) * 2;
        p_engine->currOrigin_chroma = (chromaHeight - BLOCK_HEIGHT) * (p_engine->stride_cfg.input_chroma_stride / 2) * 4 +
                                         (chromaWidth - BLOCK_WIDTH) * 4;
        /* setup relative step */
            /*In case of stride enabled, increment is configured to stride set by user. In case of stride disabled, it is set to the width*/
        p_engine->horiIncrement_luma   = -2;
            p_engine->vertIncrement_luma = (int32_t)((0 - (p_engine->stride_cfg.input_luma_stride)) * 2);
        p_engine->horiIncrement_chroma = -4;
            p_engine->vertIncrement_chroma = (-4 * (int32_t)(p_engine->stride_cfg.input_chroma_stride / 2));
        /* calculate zigzag table and quant table for rotation */
        /* for 180 degree rotation, use original zigzag table */
        zigzagTablePtr = zigzagTable;
        /* for 180 degree rotation, flip odd entries for quant tables */
        for (i = 0; i < 7; i+=2)
        {
            for (j = 0; j < 7; j+=2)
            {
                qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[i*8+j] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->lumaQuantTableQ16[i*8+j] = (int16_t)qFactorInvQ15;
                qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[i*8+j+1] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->lumaQuantTableQ16[i*8+j+1] = -(int16_t)qFactorInvQ15;

                qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[i*8+j] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->chromaQuantTableQ16[i*8+j] = (int16_t)qFactorInvQ15;
                qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[i*8+j+1] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->chromaQuantTableQ16[i*8+j+1] = -(int16_t)qFactorInvQ15;
            }
            for (j = 0; j < 7; j+=2)
            {
                qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[(i+1)*8+j] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->lumaQuantTableQ16[(i+1)*8+j] = -(int16_t)qFactorInvQ15;
                qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[(i+1)*8+j+1] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->lumaQuantTableQ16[(i+1)*8+j+1] = (int16_t)qFactorInvQ15;

                qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[(i+1)*8+j] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->chromaQuantTableQ16[(i+1)*8+j] = -(int16_t)qFactorInvQ15;
                qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[(i+1)*8+j+1] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->chromaQuantTableQ16[(i+1)*8+j+1] = (int16_t)qFactorInvQ15;
            }
        }
        break;
    }
    case 270:
    {
        /* setup real geometry parameters */
        p_engine->rotatedMCUWidth      = p_engine->MCUHeight;
        p_engine->rotatedMCUHeight     = p_engine->MCUWidth;
        p_engine->rotatedhoriMCUCount  = (chromaHeight + 7) >> 3; // divide by 8
        p_engine->rotatedvertMCUCount  = (chromaWidth + 7) >> 3;  // divide by 8
        p_engine->rotatedlumaWidth     = lumaHeight;
        p_engine->rotatedlumaHeight    = lumaWidth;
        p_engine->rotatedchromaWidth   = chromaHeight;
        p_engine->rotatedchromaHeight  = chromaWidth;
        /* setup Origin */
        p_engine->currOrigin_luma      = (lumaWidth - BLOCK_WIDTH) * 2;
        p_engine->currOrigin_chroma    = (chromaWidth - BLOCK_WIDTH) * 4;
        /* setup relative step */
            /*In case of stride enabled, increment is configured to stride set by user. In case of stride disabled, it is set to the width*/
        p_engine->horiIncrement_luma   = 2 * (p_engine->stride_cfg.input_luma_stride);
        p_engine->vertIncrement_luma   = -2;
        p_engine->horiIncrement_chroma = 4 * (p_engine->stride_cfg.input_chroma_stride / 2);
        p_engine->vertIncrement_chroma = -4;
        /* calculate zigzag table and quant table for rotation */
        /* for 270 degree rotation, transpose zigzag table */
        zigzagTablePtr = zigzagTableTranspose;
        /* for 270 degree rotation, transpose quant tables */
        for (i = 0; i < 8; i++)
        {
            for (j = 0; j < 8; j++)
            {
                qFactorInvQ15 = (uint16_t)(1.0/lumaQTablePtr[i*8+j] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->lumaQuantTableQ16[j*8+i] = (int16_t)qFactorInvQ15;
                qFactorInvQ15 = (uint16_t)(1.0/chromaQTablePtr[i*8+j] * ((1<<(FIX_POINT_FACTOR-3)) - 1) + 0.5);
                p_engine->chromaQuantTableQ16[j*8+i] = (int16_t)qFactorInvQ15;
            }
        }
        /* then flip sign of odd quant col */
        for (i = 0; i < 8; i++)
        {
            for (j = 1; j < 8; j+=2)
            {
                p_engine->lumaQuantTableQ16[i*8+j] = -p_engine->lumaQuantTableQ16[i*8+j];
                p_engine->chromaQuantTableQ16[i*8+j] = -p_engine->chromaQuantTableQ16[i*8+j];
            }
        }
        break;
    }
    default:
        JPEG_DBG_ERROR("jpege_engine_sw_configure: unrecognized rotation: %d\n", p_engine->rotation);
        return JPEGERR_EBADPARM;
    }

    /* Calculate the zigzagOffsetTable, the offset is the relative BYTES !!!
     * between DCTOutput[i] and DCTOutput[i+1] (which is int16_t), this is due to
     * the limitation of ARM LDRSH instruction which does not allow <opsh>, like:
     *
     * LDRSH Rd, [Rn], Rm, LSL #1
     *
     * and this zigzagOffsetTable is scanned decrementally, that is from
     * 63 down to 0.
     */
    for (i = JBLOCK_SIZE_MINUS_1; i != 0; i--)
    {
        p_engine->zigzagOffsetTable[i] =
            (zigzagTablePtr[i - 1] - zigzagTablePtr[i]) * sizeof(int16_t);
    }
    p_engine->zigzagOffsetTable[0] = 0;

    return JPEGERR_SUCCESS;
}

static void jpege_engine_sw_configure_huff_tables(jpege_engine_sw_t  *p_engine,
                                                  jpege_img_cfg_t    *p_config)
{
    jpeg_huff_table_t *tempHuffTablePtr;
    uint8_t   *tempSrcPtr, *tempDestPtr;
    uint16_t  j;
    /*-----------------------------------------------------------------------
      Copy the Huffman Tables
    -----------------------------------------------------------------------*/
    /* Luma DC */
    tempHuffTablePtr = &(p_config->luma_dc_huff_tbl);
    tempSrcPtr = tempHuffTablePtr->bits;
    tempDestPtr = p_engine->jpegeLumaBitsValTables.dcBits;
    for (j=1; j<=16; j++)
        tempDestPtr[j-1] = tempSrcPtr[j];

    tempDestPtr = p_engine->jpegeLumaBitsValTables.dcHuffVal;
    tempSrcPtr = tempHuffTablePtr->values;
    for (j=0; j<12; j++)
        tempDestPtr[j] = tempSrcPtr[j];

    /* Luma AC */
    tempHuffTablePtr = &(p_config->luma_ac_huff_tbl);
    tempSrcPtr = tempHuffTablePtr->bits;
    tempDestPtr = p_engine->jpegeLumaBitsValTables.acBits;
    for (j=1; j<=16; j++)
        tempDestPtr[j-1] = tempSrcPtr[j];

    tempDestPtr = p_engine->jpegeLumaBitsValTables.acHuffVal;
    tempSrcPtr = tempHuffTablePtr->values;
    for (j=0; j<162; j++)
        tempDestPtr[j] = tempSrcPtr[j];

    /* Chroma DC */
    tempHuffTablePtr = &(p_config->chroma_dc_huff_tbl);
    tempSrcPtr = tempHuffTablePtr->bits;
    tempDestPtr = p_engine->jpegeChromaBitsValTables.dcBits;
    for (j=1; j<=16; j++)
        tempDestPtr[j-1] = tempSrcPtr[j];

    tempSrcPtr = tempHuffTablePtr->values;
    tempDestPtr = p_engine->jpegeChromaBitsValTables.dcHuffVal;
    for (j=0; j<12; j++)
        tempDestPtr[j] = tempSrcPtr[j];

    /* Chroma AC */
    tempHuffTablePtr = &(p_config->chroma_ac_huff_tbl);
    tempSrcPtr = tempHuffTablePtr->bits;
    tempDestPtr = p_engine->jpegeChromaBitsValTables.acBits;
    for (j=1; j<=16; j++)
        tempDestPtr[j-1] = tempSrcPtr[j];

    tempSrcPtr = tempHuffTablePtr->values;
    tempDestPtr = p_engine->jpegeChromaBitsValTables.acHuffVal;
    for (j=0; j<162; j++)
        tempDestPtr[j] = tempSrcPtr[j];
}
