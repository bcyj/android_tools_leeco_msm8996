/*========================================================================


*//** @file jpege_engine_q5.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (c) 2008-10 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary.  Export of this technology or software is regulated
by the U.S. Government, Diversion contrary to U.S. law prohibited.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
05/20/10   staceyw Update upsampler_cfg to scale_cfg for upscale or downscale.
04/16/10   staceyw Added flag to indicate output buffer whether is last
                   output buffer during output.
01/29/10   staceyw Provide support of mutiple buffers for q5 output,
                   and get next output buffer from buffers queue,
                   and no more internal output buffers.
10/30/09   vma     When internal output buffers are used, data originally
                   in the destination buffers are lost because they
                   were not flushed. Proper fix should be to copy data
                   in internal buffers to the destination buffer before
                   outputting. Currently putting a quick fix by flushing
                   the data in the destination buffer while directly
                   outputting internal buffers.
10/22/09   vma     Export the engine 'profile' for easier engine picking.
10/13/09   vma     Allow partially used desintation buffer (start dumping
                   output from offset instead of start of buffer.
08/03/09   vma     Switched to use the os abstraction layer (os_*)
07/21/09   vma     Work around for HW problem with upsampling for
                   certain upsampling parameters with small images.
06/23/09   zhiminl Fixed pmem_fd < 0 when invalid, not <= 0.
05/27/09   mingy   Detach the thread created for notifying the done event
                   so it can be cleaned up by the system when terminated.
04/16/09   vma     Added PMEM unregistration.
01/20/09   vma     Added PMEM registration and changed to pass virtual
                   buffer addresses to DSP (need kernel change to do the
                   address fixup.)
11/04/08   vma     Stricter pre-padding requirements. Do not allow
                   'pseudo' pre-padding.
10/27/08   vma     Fixed checking for pre-padding.
10/10/08   vma     Fixed rotation direction.
10/09/08   vma     Round up output dimensions properly to multiple of MCUs
10/08/08   vma     Took out turning on of VDC clock (taken care of by kernel
                   when JPEGTASK is opened)
09/29/08   vma     Added software workaround for HW bug where single
                   fragment cannot exceed certain size.
09/29/08   vma     Added support for H1V2 with odd rotations.
09/12/08   vma     Added upsampler support.
07/29/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "jpege_engine_q5.h"
#include "jpege.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include "jpeg_buffer_private.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
static const polyphase_coeff_t polyphase_coeff[64] =
{
    { 0x00000000, 0x0007FC00 },
    { 0x000000FF, 0x80D7F3F9 },
    { 0x000000FE, 0xC1C7DBF3 },
    { 0x000000FE, 0x42B7BFED },
    { 0x000000FD, 0x83C79BE8 },
    { 0x000000FC, 0xC4D76FE4 },
    { 0x000000FC, 0x45F743E0 },
    { 0x000000FB, 0x87270FDE },
    { 0x000000FA, 0xC856D7DB },
    { 0x000000FA, 0x09869BD9 },
    { 0x000000F9, 0x8AC65BD8 },
    { 0x000000F8, 0xCC1617D7 },
    { 0x000000F8, 0x4D55CFD7 },
    { 0x000000F7, 0xCE9587D7 },
    { 0x000000F7, 0x4FE53BD8 },
    { 0x000000F6, 0xD124EBD8 },
    { 0x000000F6, 0x92649BDA },
    { 0x000000F6, 0x13A44BDB },
    { 0x000000F6, 0x14E3FBDD },
    { 0x000000F5, 0xD613A7DF },
    { 0x000000F5, 0xD73357E1 },
    { 0x000000F5, 0xD85307E3 },
    { 0x000000F6, 0x1962B3E6 },
    { 0x000000F6, 0x5A6263E8 },
    { 0x000000F6, 0xDB5217EB },
    { 0x000000F7, 0x9C31CBEE },
    { 0x000000F8, 0x1D017FF1 },
    { 0x000000F9, 0x1DB137F3 },
    { 0x000000FA, 0x1E60F3F6 },
    { 0x000000FB, 0x5EF0AFF9 },
    { 0x000000FC, 0xDF6073FB },
    { 0x000000FE, 0x5FC037FE },
};
/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* Function prototypes of jpege_engine_obj_t interface functions */
static void jpege_engine_q5_create  (jpege_engine_obj_t *p_obj, jpege_obj_t encoder);
static int  jpege_engine_q5_init    (jpege_engine_obj_t*,
                                     jpege_engine_event_handler_t);
static int  jpege_engine_q5_start   (jpege_engine_obj_t*, jpege_img_cfg_t*,
                                     jpege_img_data_t*,
                                     jpege_engine_dst_t*);
static int  jpege_engine_q5_abort   (jpege_engine_obj_t*);
static void jpege_engine_q5_destroy (jpege_engine_obj_t*);
static int  jpege_engine_q5_check_start_param (jpege_img_cfg_t*,
                                               jpege_img_data_t*,
                                               jpege_engine_dst_t*);
static void jpege_engine_q5_event_handler (void * p_client, uint16_t event_id,
                                           uint8_t * p_buf, uint32_t len);
static jpeg_buf_t *jpege_engine_q5_locate_dst_buf(jpege_engine_q5_t *p_engine,
                                                  void *ptr);
static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
             jpege_engine_q5_notify_done(OS_THREAD_FUNC_ARG_T arg);
/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
#define DEFAULT_OUTBUF_SIZE  4096

/* =======================================================================
**                      Global Object Definitions
** ======================================================================= */
static const char jpege_engine_q5_name[] = "Jpeg DSP Encode Engine";
jpege_engine_profile_t jpege_engine_q5_profile = {jpege_engine_q5_create, jpege_engine_q5_name};

/* =======================================================================
**                          Function Definitions
** ======================================================================= */

static void jpege_engine_q5_create(jpege_engine_obj_t *p_obj, jpege_obj_t encoder)
{
    if (p_obj)
    {
        // Destroy previous engine if it exists
        if (p_obj->destroy)
        {
            p_obj->destroy(p_obj);
        }
        p_obj->create            = &jpege_engine_q5_create;
        p_obj->init              = &jpege_engine_q5_init;
        p_obj->check_start_param = &jpege_engine_q5_check_start_param;
        p_obj->start             = &jpege_engine_q5_start;
        p_obj->abort             = &jpege_engine_q5_abort;
        p_obj->destroy           = &jpege_engine_q5_destroy;
        p_obj->p_engine          = NULL;
        p_obj->encoder           = encoder;
        p_obj->is_initialized    = false;
    }
}

static int jpege_engine_q5_init(
    jpege_engine_obj_t           *p_obj,
    jpege_engine_event_handler_t  p_handler)
{
    int rc;
    jpege_engine_q5_t *p_engine;

    // Validate input arguments
    if (!p_obj || !p_handler)
        return JPEGERR_ENULLPTR;

    // Allocate memory for the engine structure
    p_engine = (jpege_engine_q5_t *)JPEG_MALLOC(sizeof(jpege_engine_q5_t));
    if (!p_engine)
    {
        return JPEGERR_EMALLOC;
    }

    // Initialize the fields inside the engine structure below
    STD_MEMSET(p_engine, 0, sizeof(jpege_engine_q5_t));   // Zero out the entire structure
    p_engine->p_wrapper = p_obj;                          // Initialize the pointer to the wrapper
    p_engine->p_handler = p_handler;                      // Initialize the event handler
    os_mutex_init(&(p_engine->mutex));                    // Initialize the mutex
    os_cond_init(&(p_engine->cond));                      // Initialize the condition variable


    rc = jpeg_q5_helper_init(&(p_engine->q5_helper),  // Initizlize the q5 helper
                                 (void *)p_engine,
                                 &jpege_engine_q5_event_handler);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_HIGH("jpege_engine_q5_init: jpeg_q5_helper_init failed\n");
        JPEG_FREE(p_engine);
        return rc;
    }

    // Initialize the state
    p_engine->state = JPEGE_Q5_IDLE;

    // Assign allocated engine structure to p_obj
    p_obj->p_engine = (void *)p_engine;
    p_obj->is_initialized = true;
    return JPEGERR_SUCCESS;
}

static int jpege_engine_q5_check_start_param (
    jpege_img_cfg_t      *p_config,
    jpege_img_data_t     *p_source,
    jpege_engine_dst_t   *p_dest)
{
    uint32_t i;
    // Pointer validation
    if (!p_config || !p_source || !p_dest)
    {
        return JPEGERR_ENULLPTR;
    }

    // Support H2V2, H2V1, H1V2 YCrCb only
    if (p_source->color_format != YCRCBLP_H2V2 &&
        p_source->color_format != YCRCBLP_H2V1 &&
        p_source->color_format != YCRCBLP_H1V2)
    {
        JPEG_DBG_MED("jpege_engine_q5_check_start_param: only support YCRCB (H2V2, H2V1 and H1V2)\n");
        return JPEGERR_EUNSUPPORTED;
    }
    // Validate rotation
    if (p_config->rotation_degree_clk % 90)
    {
        return JPEGERR_EBADPARM;
    }

    // Reject H2V1 and odd rotation,
    // and H1V2 with even rotation
    if ((p_source->color_format == YCRCBLP_H2V1 && p_config->rotation_degree_clk % 180) ||
        (p_source->color_format == YCRCBLP_H1V2 && !(p_config->rotation_degree_clk % 180)))
    {
        JPEG_DBG_MED("jpege_engine_q5_check_start_param: "
                     "H2V1 with even rotation/H1V2 with odd rotation not supported\n");
        return JPEGERR_EUNSUPPORTED;
    }

    // Validate fragments
    if (!p_source->fragment_cnt)
    {
        JPEG_DBG_MED("jpege_engine_q5_check_start_param: no valid fragment\n");
        return JPEGERR_EBADPARM;
    }
    for (i = 0; i < p_source->fragment_cnt; i++)
    {
        if (((jpeg_buf_t *)p_source->p_fragments[i].color.yuv.luma_buf)->pmem_fd < 0 ||
            ((jpeg_buf_t *)p_source->p_fragments[i].color.yuv.chroma_buf)->pmem_fd < 0)
        {
            JPEG_DBG_MED("jpege_engine_q5_check_start_param: input buffer needs to be physically mapped\n");
            return JPEGERR_EUNSUPPORTED;
        }
    }
    // Fragments need to be padded
    if ((p_source->color_format != YCRCBLP_H1V2 && p_source->p_fragments[0].width != ROUND_TO_16(p_source->width)) ||
        (p_source->color_format != YCRCBLP_H2V1 && p_source->p_fragments[0].height != ROUND_TO_16(p_source->height)))
    {
        JPEG_DBG_MED("jpege_engine_q5_check_start_param: input fragment not pre-padded\n");
        return JPEGERR_EUNSUPPORTED;
    }

    // Give warnings if the supplied destination buffer(s) are not useful
    if (p_dest->buffer_cnt < 2)
    {
        JPEG_DBG_MED("jpege_engine_q5_check_start_param: warning: 2 destination buffers needed; "
                     "use internal ones\n");
    }
    if (((jpeg_buf_t *)p_dest->p_buffers[0])->pmem_fd < 0 ||
        ((jpeg_buf_t *)p_dest->p_buffers[1])->pmem_fd < 0)
    {
        JPEG_DBG_MED("jpege_engine_q5_check_start_param: warning: destination buffer not physically mapped; "
                     "use internal ones\n");
    }

    // Do not support upsampling with H1V2 input
    if (p_config->scale_cfg.enable && p_source->color_format == YCRCBLP_H1V2)
    {
        JPEG_DBG_MED("jpege_engine_q5_check_start_param: Do not support H1V2 with upsampling\n");
        return JPEGERR_EUNSUPPORTED;
    }

    if ( p_config->scale_cfg.enable &&
         ((90 == p_config->rotation_degree_clk) ||
         (180 == p_config->rotation_degree_clk)) )
    {
        JPEG_DBG_HIGH("jpege_engine_q5_check_start_param: Do not support scaling with 90 and 180 degree "
                      "rotation because of padding on the top\n");
        return JPEGERR_EUNSUPPORTED;
    }

    // Make sure upsampling related settings are within range (12-bits),
    // and there is enough padding (4 pixels on all sides)
    // and if the upsampling ratio is acceptable (1x - 4x only)
    if (p_config->scale_cfg.enable &&
        (p_config->scale_cfg.input_width  > 0xfff ||
         p_config->scale_cfg.input_height > 0xfff ||
         !p_config->scale_cfg.input_width ||
         !p_config->scale_cfg.input_height ||
         p_config->scale_cfg.output_width > (p_config->scale_cfg.input_width << 2) ||
         p_config->scale_cfg.output_height > (p_config->scale_cfg.input_height << 2) ||
         p_config->scale_cfg.output_width < p_config->scale_cfg.input_width ||
         p_config->scale_cfg.output_height < p_config->scale_cfg.input_height ||
         p_config->scale_cfg.h_offset > 0xfff ||
         p_config->scale_cfg.v_offset > 0xfff ||
         p_config->scale_cfg.h_offset < 4 ||
         p_config->scale_cfg.v_offset < 4 ||
         p_config->scale_cfg.h_offset + p_config->scale_cfg.input_width > p_source->width - 4 ||
         p_config->scale_cfg.v_offset + p_config->scale_cfg.input_height > p_source->height - 4))
    {
        JPEG_DBG_MED("jpege_engine_q5_check_start_param: invalid upsampling config\n");
        return JPEGERR_EBADPARM;
    }

    return JPEGERR_SUCCESS;
}

static int jpege_engine_q5_start(
    jpege_engine_obj_t   *p_obj,
    jpege_img_cfg_t      *p_config,
    jpege_img_data_t     *p_source,
    jpege_engine_dst_t   *p_dest)
{
    int i, rc;
    jpege_engine_q5_t     *p_engine;
    jpeg_q5_enc_cfg_cmd_t  cfg_cmd;
    jpeg_q5_enc_cmd_t      enc_cmd;

    // Validate input arguments
    if (!p_obj || !p_obj->p_engine)
        return JPEGERR_ENULLPTR;

    // Cast p_obj->obj as jpege_engine_q5_t object
    p_engine = p_obj->p_engine;

    // Make sure engine is idle
    os_mutex_lock(&(p_engine->mutex));
    if (p_engine->state != JPEGE_Q5_IDLE)
    {
        os_mutex_unlock(&(p_engine->mutex));
        JPEG_DBG_HIGH("jpege_engine_q5_start: engine not idle\n");
        return JPEGERR_EBADSTATE;
    }

    // Store p_dest
    p_engine->dest = *p_dest;

    // Determine internal output buffers need to be used
    if (p_dest->buffer_cnt < 2 ||
        p_dest->p_buffers[0]->pmem_fd < 0 ||
        p_dest->p_buffers[1]->pmem_fd < 0)
    {
        JPEG_DBG_LOW("jpege_engine_q5_start: about to create internal buffers\n");
        // Allocate internal output buffers if not already
        if (!p_engine->p_out_bufs[0])
        {
            rc = jpeg_buffer_init((jpeg_buffer_t *)&p_engine->p_out_bufs[0]);
            if (JPEG_SUCCEEDED(rc))
            {
                rc = jpeg_buffer_allocate((jpeg_buffer_t)p_engine->p_out_bufs[0], DEFAULT_OUTBUF_SIZE, true);
            }
            if (JPEG_SUCCEEDED(rc))
            {
                rc = jpeg_buffer_init((jpeg_buffer_t *)&p_engine->p_out_bufs[1]);
            }
            if (JPEG_SUCCEEDED(rc))
            {
                rc = jpeg_buffer_allocate((jpeg_buffer_t)p_engine->p_out_bufs[1], DEFAULT_OUTBUF_SIZE, true);
            }
            if (JPEG_FAILED(rc))
            {
                jpeg_buffer_destroy((jpeg_buffer_t *)p_engine->p_out_bufs[0]);
                jpeg_buffer_destroy((jpeg_buffer_t *)p_engine->p_out_bufs[1]);
                return rc;
            }
        }
        // Flush what is already in the destination buffer (temporary solution)
        if (p_engine->dest.p_buffers[0]->offset)
        {
            jpeg_buffer_mark_filled(p_engine->dest.p_buffers[0]);
            p_engine->dest.p_output_handler(p_engine->p_wrapper,
                                            p_engine->dest.p_buffers[0]);
        }

        p_engine->dest.buffer_cnt = 2;
        p_engine->dest.p_buffers[0] = p_engine->p_out_bufs[0];
        p_engine->dest.p_buffers[1] = p_engine->p_out_bufs[1];
    }

    // Clear encode config command structure
    STD_MEMSET(&cfg_cmd, 0, sizeof(jpeg_q5_enc_cfg_cmd_t));

    // Set up header command
    cfg_cmd.header.command = JPEG_Q5_CONFIG_CMD_JPEGE;

    // Unregister any possible residual PMEM registration
    (void) jpeg_q5_helper_unregister_pmem(&(p_engine->q5_helper));

    jpeg_q5_helper_register_pmem(&(p_engine->q5_helper), p_engine->dest.p_buffers[0]);
    jpeg_q5_helper_register_pmem(&(p_engine->q5_helper), p_engine->dest.p_buffers[1]);

    // Set up output buffer configuration
    cfg_cmd.output_buffer_cfg[0].p_buffer = p_engine->dest.p_buffers[0]->ptr + p_engine->dest.p_buffers[0]->offset;
    cfg_cmd.output_buffer_cfg[1].p_buffer = p_engine->dest.p_buffers[1]->ptr + p_engine->dest.p_buffers[1]->offset;
    cfg_cmd.output_buffer_cfg[0].size     = p_engine->dest.p_buffers[0]->size - p_engine->dest.p_buffers[0]->offset;
    cfg_cmd.output_buffer_cfg[1].size     = p_engine->dest.p_buffers[1]->size - p_engine->dest.p_buffers[1]->offset;

    // Verify that the addresses are valid
    if (!cfg_cmd.output_buffer_cfg[0].p_buffer ||
        !cfg_cmd.output_buffer_cfg[1].p_buffer)
    {
        JPEG_DBG_HIGH("jpege_engine_q5_start: failed to obtain physical address\n");
        return JPEGERR_EFAILED;
    }

    // Set up subsampling
    cfg_cmd.subsample = (jpeg_q5_format_t)(p_source->color_format / 2);

    // Set up quantization tables
    for (i = 0; i < 64; i++)
    {
        cfg_cmd.luma_qtbl[i] = (uint16_t) ((double)0xFFFF/(double)p_config->luma_quant_tbl[i]);
        cfg_cmd.chroma_qtbl[i] = (uint16_t) ((double)0xFFFF/(double)p_config->chroma_quant_tbl[i]);
    }

    // Set up rotation
    cfg_cmd.rotation = (jpeg_q5_rotation_t)((p_config->rotation_degree_clk / 90) & 0x03);

    // Set up fragments
    cfg_cmd.fragment_height = p_source->p_fragments[0].height;
    cfg_cmd.fragments = p_source->fragment_cnt;
    // SW workaround for HW bug where one single fragment cannot be beyond certain size
    if (cfg_cmd.fragments == 1)
    {
        uint16_t luma_incr = cfg_cmd.fragment_height >> 2;
        uint16_t chroma_incr = luma_incr;
        uint32_t orig_frag_height = cfg_cmd.fragment_height;

        if (cfg_cmd.subsample == JPEG_FORMAT_H2V2)
        {
            chroma_incr /= 2;
        }
        jpeg_q5_helper_register_pmem(&(p_engine->q5_helper),
                                     (jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf);
        jpeg_q5_helper_register_pmem(&(p_engine->q5_helper),
                                     (jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf);
        cfg_cmd.fragments = 4;
        cfg_cmd.fragment_height = luma_incr;
        cfg_cmd.fragment_config[0].p_luma =
            ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->ptr;
        cfg_cmd.fragment_config[0].p_chroma =
            ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->ptr;

        // SW workaround for HW upsampling issue with certain upsampling parameters -
        // Repeat fragment addresses to other unused fragment address holders
        // while keeping the fragment height as the entire image height
        if (orig_frag_height <= 144)
        {
            luma_incr = 0;
            chroma_incr = 0;
            cfg_cmd.fragment_height = orig_frag_height;
        }
        for (i = 1; i < 4; i++)
        {
            cfg_cmd.fragment_config[i].p_luma = cfg_cmd.fragment_config[0].p_luma +
                p_source->p_fragments[0].width * luma_incr * i;
            cfg_cmd.fragment_config[i].p_chroma = cfg_cmd.fragment_config[0].p_chroma +
                p_source->p_fragments[0].width * chroma_incr * i;
        }
    }
    else
    {
        for (i = 0; i < 8; i++)
        {
            if (i < cfg_cmd.fragments)
            {
                jpeg_q5_helper_register_pmem(&(p_engine->q5_helper),
                                             (jpeg_buf_t *)p_source->p_fragments[i].color.yuv.luma_buf);
                jpeg_q5_helper_register_pmem(&(p_engine->q5_helper),
                                             (jpeg_buf_t *)p_source->p_fragments[i].color.yuv.chroma_buf);
                cfg_cmd.fragment_config[i].p_luma =
                    ((jpeg_buf_t *)p_source->p_fragments[i].color.yuv.luma_buf)->ptr;
                cfg_cmd.fragment_config[i].p_chroma =
                    ((jpeg_buf_t *)p_source->p_fragments[i].color.yuv.chroma_buf)->ptr;
            }
            else
            {
                cfg_cmd.fragment_config[i].p_luma = NULL;
                cfg_cmd.fragment_config[i].p_chroma = NULL;
            }
        }
    }
    // Set up restart marker interval
    cfg_cmd.restart = p_config->restart_interval;
    // Set up input dimension
    cfg_cmd.input_width   = p_source->p_fragments[0].width;
    cfg_cmd.input_height  = p_source->p_fragments[0].height;
    // Set up output dimension
    if (p_config->scale_cfg.enable)
    {
        cfg_cmd.output_width = p_config->scale_cfg.output_width;
        cfg_cmd.output_height = p_config->scale_cfg.output_height;
    }
    else
    {
        cfg_cmd.output_width  = p_source->width;
        cfg_cmd.output_height = p_source->height;
    }
    // Make sure the output is rounded up correctly
    if (cfg_cmd.subsample == JPEG_FORMAT_H2V2)
    {
        cfg_cmd.output_width = ROUND_TO_16(cfg_cmd.output_width);
        cfg_cmd.output_height = ROUND_TO_16(cfg_cmd.output_height);
    }
    else if (cfg_cmd.subsample == JPEG_FORMAT_H2V1)
    {
        cfg_cmd.output_width = ROUND_TO_16(cfg_cmd.output_width);
        cfg_cmd.output_height = ROUND_TO_8(cfg_cmd.output_height);
    }
    else // It must be H1V2
    {
        cfg_cmd.output_width = ROUND_TO_8(cfg_cmd.output_width);
        cfg_cmd.output_height = ROUND_TO_16(cfg_cmd.output_height);
    }

    // No upsampling for now
    // todo: enable it
    cfg_cmd.upsampler_enable       = p_config->scale_cfg.enable;
    cfg_cmd.upsampler_input_width  = p_config->scale_cfg.input_width;
    cfg_cmd.upsampler_input_height = p_config->scale_cfg.input_height;
    cfg_cmd.upsampler_hoffset      = p_config->scale_cfg.h_offset;
    cfg_cmd.upsampler_voffset      = p_config->scale_cfg.v_offset;

    // Set up RTOS_Partition number
    cfg_cmd.rtos_partition_num = 0;

    // Set up polyphase filter coefficients
    STD_MEMMOVE(cfg_cmd.coeff, polyphase_coeff, sizeof(polyphase_coeff_t) * 32);

    // Send encode config command
    rc = jpeg_q5_helper_send_cfg_command(&(p_engine->q5_helper), (uint8_t *)&cfg_cmd, sizeof(cfg_cmd));
    if (JPEG_FAILED(rc))
    {
        return rc;
    }

    p_engine->state = JPEGE_Q5_ENCODE_WAIT;

    // Send encode command
    enc_cmd.header = JPEG_Q5_ACTION_CMD_JPEGE_ENCODE;
    rc = jpeg_q5_helper_send_action_command(&(p_engine->q5_helper),
                                            (uint8_t *)&enc_cmd,
                                            sizeof(enc_cmd));
    if (JPEG_FAILED(rc))
    {
        p_engine->state = JPEGE_Q5_IDLE;
        os_mutex_unlock(&(p_engine->mutex));
        return rc;
    }

    // Wait for encode acknowledgement from DSP
    JPEG_DBG_LOW("jpege_engine_q5_start: encode wait\n");
    rc = 0;
    while (p_engine->state == JPEGE_Q5_ENCODE_WAIT)
    {
       // Wait for encode ack
        rc = os_cond_timedwait(&(p_engine->cond), &(p_engine->mutex), 2000);
        if (rc < 0) break;
    }
    if (rc < 0)
    {
        p_engine->state = JPEGE_Q5_IDLE;
    }
    os_mutex_unlock(&(p_engine->mutex));
    return (rc < 0) ? JPEGERR_EFAILED : JPEGERR_SUCCESS;
}

static int jpege_engine_q5_abort(
    jpege_engine_obj_t *p_obj)
{
    jpege_engine_q5_t *p_engine;

    // Validate input arguments
    if (!p_obj || !p_obj->p_engine)
        return JPEGERR_ENULLPTR;

    // Cast p_obj->obj as jpege_engine_q5_t object
    p_engine = p_obj->p_engine;

    // Send IDLE command to DSP if it is actively encoding
    os_mutex_lock(&(p_engine->mutex));
    if (p_engine->state == JPEGE_Q5_ENCODING ||
        p_engine->state == JPEGE_Q5_ENCODE_WAIT)
    {
        jpeg_q5_enc_idle_cmd_t  idle_cmd;

        p_engine->state = JPEGE_Q5_ABORT_WAIT;
        idle_cmd.header = JPEG_Q5_ACTION_CMD_JPEGE_IDLE;
        if (JPEG_FAILED(jpeg_q5_helper_send_action_command(&(p_engine->q5_helper),
                                                           (uint8_t *)&idle_cmd,
                                                           sizeof(idle_cmd))))
        {
            os_mutex_unlock(&(p_engine->mutex));
            return JPEGERR_EFAILED;
        }
    }
    while (p_engine->state != JPEGE_Q5_IDLE)
    {
        os_cond_wait(&(p_engine->cond), &(p_engine->mutex));
    }
    os_cond_broadcast(&(p_engine->cond)); /*Signal the start routine if its waiting*/
    os_mutex_unlock(&(p_engine->mutex));
    return JPEGERR_SUCCESS;
}

static void jpege_engine_q5_destroy(
    jpege_engine_obj_t *p_obj)
{
    if (p_obj)
    {
        jpege_engine_q5_t *p_engine = (jpege_engine_q5_t *)(p_obj->p_engine);

        JPEG_DBG_LOW("jpege_engine_q5_destroy\n");
        // Release allocated memory
        if (p_engine)
        {
            (void)jpege_engine_q5_abort(p_obj);

            if (p_engine->p_out_bufs[0])
            {
                jpeg_buffer_destroy((jpeg_buffer_t *)&p_engine->p_out_bufs[0]);
                jpeg_buffer_destroy((jpeg_buffer_t *)&p_engine->p_out_bufs[1]);
            }
            os_mutex_destroy(&(p_engine->mutex));
            os_cond_destroy(&(p_engine->cond));
            jpeg_q5_helper_destroy(&(p_engine->q5_helper));
            JPEG_FREE(p_obj->p_engine);
            p_obj->p_engine = NULL;
        }
    }
}

static void jpege_engine_q5_event_handler(void     *p_client,
                                          uint16_t  event_id,
                                          uint8_t  *p_buf,
                                          uint32_t  len)
{
    jpege_engine_q5_t *p_engine = (jpege_engine_q5_t *)p_client;

    switch (event_id)
    {
    case JPEG_Q5_MSG_JPEGE_ENCODE_ACK:
        {
            JPEG_DBG_LOW("jpege_engine_q5_event_handler: ENCODE ACK\n");
            os_mutex_lock(&(p_engine->mutex));
            if (p_engine->state == JPEGE_Q5_ENCODE_WAIT)
            {
                p_engine->state = JPEGE_Q5_ENCODING;
            }
            os_cond_broadcast(&(p_engine->cond));
            os_mutex_unlock(&(p_engine->mutex));
        }
        break;
    case JPEG_Q5_MSG_JPEGE_IDLE_ACK:
        {
            uint8_t done_wait = false;
            JPEG_DBG_LOW("jpege_engine_q5_event_handler: IDLE ACK\n");
            os_mutex_lock(&(p_engine->mutex));
            if (p_engine->state == JPEGE_Q5_DONE_WAIT)
            {
                done_wait = true;
            }
            p_engine->state = JPEGE_Q5_IDLE;
            os_cond_broadcast(&(p_engine->cond));
            os_mutex_unlock(&(p_engine->mutex));

            if (done_wait)
            {
                // spawn a new thread to notify the done event
                os_thread_t thread;
                if (os_thread_create(&thread, jpege_engine_q5_notify_done,
                                   (void *)p_engine))
                {
                    JPEG_DBG_ERROR("jpege_engine_q5_event_handler: pthread_create failed\n");
                    p_engine->p_handler(p_engine->p_wrapper, JPEG_EVENT_ERROR, NULL);
                }
                else
                {
                    os_thread_detach(&thread);
                }
            }
        }
        break;
    case JPEG_Q5_MSG_JPEGE_ILLEGAL_COMMAND:
        {
            os_mutex_lock(&(p_engine->mutex));
            JPEG_DBG_LOW("jpege_engine_q5_event_handler: ILLEGAL COMMAND\n");
            if (p_engine->state == JPEGE_Q5_ENCODE_WAIT)
            {
                p_engine->state = JPEGE_Q5_IDLE;
            }
            os_cond_broadcast(&(p_engine->cond));
            os_mutex_unlock(&(p_engine->mutex));
        }
        break;

    case JPEG_Q5_MSG_JPEGE_OUTPUT_PRODUCED:
        {
            jpeg_q5_enc_output_produced_msg_t msg =
                *((jpeg_q5_enc_output_produced_msg_t *)p_buf);

            jpeg_buf_t *p_dest_buf = jpege_engine_q5_locate_dst_buf(p_engine, msg.p_buf);

            JPEG_DBG_LOW("jpege_engine_q5_event_handler: output produced ptr=0x%x size=0x%x\n", (int)msg.p_buf, (int)msg.size);

            if (!p_dest_buf)
            {
                JPEG_DBG_ERROR("jpege_engine_q5_event_handler: invalid output buffer detected\n");
                p_engine->p_handler(p_engine->p_wrapper, JPEG_EVENT_ERROR,
                                    "invalid output buffer from dsp");
                break;
            }
            p_dest_buf->offset += msg.size;
            if (p_dest_buf->offset)
            {
                jpeg_buffer_mark_filled(p_dest_buf);
                p_engine->dest.p_output_handler(p_engine->p_wrapper,
                                                p_dest_buf);
            }
            if (msg.status.done_flag)
            {
                jpeg_q5_enc_idle_cmd_t  idle_cmd;
                os_mutex_lock(&(p_engine->mutex));
                p_engine->state = JPEGE_Q5_DONE_WAIT;
                idle_cmd.header = JPEG_Q5_ACTION_CMD_JPEGE_IDLE;
                if (JPEG_FAILED(jpeg_q5_helper_send_action_command(&(p_engine->q5_helper),
                                                                   (uint8_t *)&idle_cmd,
                                                                   sizeof(idle_cmd))))
                {
                    p_engine->state = JPEGE_Q5_IDLE;
                    os_mutex_unlock(&(p_engine->mutex));
                    p_engine->p_handler(p_engine->p_wrapper, JPEG_EVENT_ERROR,
                                        "error sending idle command to dsp");

                }
                else
                {
                    os_mutex_unlock(&(p_engine->mutex));
                }
            }
            else
            {
                jpeg_q5_enc_output_consumed_cmd_t cmd;
                // Block until the buffer is empty
                jpeg_buffer_wait_until_empty(p_dest_buf);

                cmd.header = JPEG_Q5_ACTION_CMD_JPEGE_OUTPUT_CONSUMED;
                cmd.output_buffer.p_buffer = p_dest_buf->ptr + p_dest_buf->offset;
                cmd.output_buffer.size     = p_dest_buf->size - p_dest_buf->offset;
                if (JPEG_FAILED(jpeg_q5_helper_send_action_command(&(p_engine->q5_helper),
                                                                   (uint8_t *)&cmd, sizeof(cmd))))
                {
                    os_mutex_lock(&(p_engine->mutex));
                    p_engine->state = JPEGE_Q5_IDLE;
                    os_mutex_unlock(&(p_engine->mutex));
                    p_engine->p_handler(p_engine->p_wrapper, JPEG_EVENT_ERROR,
                                        "error sending output consumed command to dsp");
                }
            }
        }
        break;

    case JPEG_Q5_IOCTL_ERROR:
        os_mutex_lock(&(p_engine->mutex));
        p_engine->state = JPEGE_Q5_IDLE;
        os_mutex_unlock(&(p_engine->mutex));
        p_engine->p_handler(p_engine->p_wrapper, JPEG_EVENT_ERROR,
                            "error communicating with dsp");
        break;
    default:
        break;
    }
}

static jpeg_buf_t *jpege_engine_q5_locate_dst_buf(jpege_engine_q5_t *p_engine,
                                                  void *ptr)
{
    void *tmp1 = p_engine->dest.p_buffers[1]->ptr + p_engine->dest.p_buffers[1]->size;
    void *tmp2 = p_engine->dest.p_buffers[0]->ptr + p_engine->dest.p_buffers[0]->size;
    if (ptr >= (void *)p_engine->dest.p_buffers[1]->ptr &&
        ptr  < (void *)tmp1)
    {
        return p_engine->dest.p_buffers[1];
    }
    else if (ptr >= (void *)p_engine->dest.p_buffers[0]->ptr &&
             ptr  < (void *)tmp2)
    {
        return p_engine->dest.p_buffers[0];
    }
    return NULL;
}

static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
             jpege_engine_q5_notify_done(OS_THREAD_FUNC_ARG_T arg)
{
    jpege_engine_q5_t *p_engine = (jpege_engine_q5_t *)arg;
    if (p_engine && p_engine->p_handler)
    {
        p_engine->p_handler(p_engine->p_wrapper, JPEG_EVENT_DONE, NULL);
    }
    return OS_THREAD_FUNC_RET_SUCCEEDED;
}

