/*========================================================================


*//** @file jpegd_engine_q5.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-09 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
12/02/09   mingy   Added checking engine output handler return value.
10/22/09   vma     Export the engine 'profile' for easier engine picking.
08/03/09   vma     Switched to use the os abstraction layer (os_*)
06/23/09   zhiminl Fixed pmem_fd < 0 when invalid, not <= 0.
04/16/09   vma     PMEM unregistration.
02/26/09   vma     Took away destination buffer lookup.
02/04/09   vma     Pass virtual address down to DSP (require kernel support)
10/09/08   vma     Do not call abort during destruction if engine was never
                   properly initialized;
                   Send IDLE_CMD to DSP when decoding is done.
10/08/08   vma     Took out turning on of VDC clock (taken care of by kernel
                   when JPEGTASK is opened)
07/29/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "jpegd_engine_q5.h"
#include "jpegd.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include "jpeg_buffer_private.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* Function prototypes of jpegd_engine_obj_t interface functions */
static void jpegd_engine_q5_create  (jpegd_engine_obj_t *p_obj,
                                     jpegd_obj_t decoder);
static int  jpegd_engine_q5_init    (jpegd_engine_obj_t*,
                                     jpegd_engine_event_handler_t,
                                     jpegd_engine_input_req_handler_t);
static int  jpegd_engine_q5_start   (jpegd_engine_obj_t*);
static int  jpegd_engine_q5_abort   (jpegd_engine_obj_t*);
static void  jpegd_engine_q5_destroy (jpegd_engine_obj_t*);
static int   jpegd_engine_q5_configure (jpegd_engine_obj_t*,
                                        jpegd_engine_src_t*,
                                        jpegd_engine_dst_t*,
                                        uint32_t*,
                                        uint32_t*);
static void jpegd_engine_q5_enqueue_event (jpegd_engine_q5_t*,
                                           jpeg_event_t event,
                                           void *payload);
static void jpegd_engine_q5_event_handler (void * p_client, uint16_t event_id,
                                           uint8_t * p_buf, uint32_t len);
static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
             jpegd_engine_q5_notify_event_thread(OS_THREAD_FUNC_ARG_T arg);
/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */
extern const int32_t zag[64];
static const char jpegd_engine_q5_name[] = "Jpeg DSP Decode Engine";
jpegd_engine_profile_t jpegd_engine_q5_profile = {jpegd_engine_q5_create, // create_func
                                                         jpegd_engine_q5_name,   // engine_name
                                                         1};                     // need_pmem

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
#define DEFAULT_OUTBUF_SIZE    4096
#define ROUND_TO_8(x)          ((((x) + 7) >> 3) << 3)
#define ROUND_TO_16(x)         ((((x) + 15) >> 4) << 4)
/* =======================================================================
**                          Function Definitions
** ======================================================================= */

static void jpegd_engine_q5_create(jpegd_engine_obj_t *p_obj, jpegd_obj_t decoder)
{
    if (p_obj)
    {
        // Destroy previous engine if it exists
        if (p_obj->destroy)
        {
            p_obj->destroy(p_obj);
        }
        p_obj->create            = &jpegd_engine_q5_create;
        p_obj->init              = &jpegd_engine_q5_init;
        p_obj->configure         = &jpegd_engine_q5_configure;
        p_obj->start             = &jpegd_engine_q5_start;
        p_obj->abort             = &jpegd_engine_q5_abort;
        p_obj->destroy           = &jpegd_engine_q5_destroy;
        p_obj->p_engine          = NULL;
        p_obj->decoder           = decoder;
        p_obj->is_intialized     = false;
    }
}

static int jpegd_engine_q5_init(
    jpegd_engine_obj_t              *p_obj,
    jpegd_engine_event_handler_t     p_event_handler,
    jpegd_engine_input_req_handler_t p_input_req_handler)
{
    int rc;
    jpegd_engine_q5_t *p_engine;

    // Validate input arguments
    if (!p_obj || !p_event_handler || !p_input_req_handler)
        return JPEGERR_ENULLPTR;

    // Allocate memory for the engine structure
    p_engine = (jpegd_engine_q5_t *)JPEG_MALLOC(sizeof(jpegd_engine_q5_t));
    if (!p_engine)
    {
        return JPEGERR_EMALLOC;
    }

    // Initialize the fields inside the engine structure below
    STD_MEMSET(p_engine, 0, sizeof(jpegd_engine_q5_t));    // Zero out the entire structure
    p_engine->p_wrapper               = p_obj;             // Initialize the pointer to the wrapper
    p_engine->input_fetcher.p_wrapper = p_obj;             // Initialize the pionter to the wrapper (for fetcher)
    p_engine->p_event_handler         = p_event_handler;   // Initialize the event handler
    p_engine->input_fetcher.p_source  = &p_engine->source; // Initialize source pointer of fetcher
    p_engine->input_fetcher.p_input_req_handler =
        p_input_req_handler;                               // Initialize the input request handler
    os_mutex_init(&(p_engine->mutex));                     // Initialize the mutex
    os_cond_init(&(p_engine->cond));                       // Initialize the condition variable

    // Assign allocated engine structure to p_obj
    p_obj->p_engine = (void *)p_engine;

    // Create thread for event notification
    if (os_thread_create(&p_engine->thread, jpegd_engine_q5_notify_event_thread, (void *)p_engine))
    {
        JPEG_DBG_HIGH("jpegd_engine_q5_init: failed to create event notification thread\n");
        jpegd_engine_q5_destroy(p_obj);
        return JPEGERR_EFAILED;
    }
    p_engine->thread_running = true;

    rc = jpeg_q5_helper_init(&(p_engine->q5_helper),  // Initizlize the q5 helper
                             (void *)p_engine,
                             &jpegd_engine_q5_event_handler);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_HIGH("jpegd_engine_q5_init: jpeg_q5_helper_init failed\n");
        jpegd_engine_q5_destroy(p_obj);
        return rc;
    }
    // Initialize the state
    p_engine->state = JPEGD_Q5_IDLE;
    p_obj->is_intialized = true;
    return JPEGERR_SUCCESS;
}

static int jpegd_engine_q5_configure(jpegd_engine_obj_t *p_obj,
                                     jpegd_engine_src_t *p_source,
                                     jpegd_engine_dst_t *p_dest,
                                     uint32_t           *p_output_chunk_width,
                                     uint32_t           *p_output_chunk_height)
{
    jpegd_engine_q5_t *p_engine = p_obj->p_engine;

    // Check state
    if (p_engine->state != JPEGD_Q5_IDLE)
    {
        return JPEGERR_EBADSTATE;
    }

    // Input validation
    if (!p_source || !p_dest || !p_source->p_frame_info || !p_source->p_input_req_handler ||
        !p_source->p_buffers[0] || !p_source->p_buffers[1] ||
        p_source->p_buffers[0]->pmem_fd < 0 || p_source->p_buffers[1]->pmem_fd < 0 ||
        !p_dest->p_luma_buffers[0] || !p_dest->p_chroma_buffers[0] ||
        !p_dest->p_luma_buffers[1] || !p_dest->p_chroma_buffers[1] ||
        !p_dest->width || !p_dest->height)
    {
        return JPEGERR_EBADPARM;
    }

    // Check for support
    if ((p_source->p_frame_info->subsampling != JPEG_H2V2 &&  // support only H2V2 and H2V1
         p_source->p_frame_info->subsampling != JPEG_H2V1) ||
        p_dest->width  > p_source->p_frame_info->width || // no upscaling support
        p_dest->height > p_source->p_frame_info->height ||
        p_dest->region.right || p_dest->region.bottom || // no region decoding support
        p_source->p_frame_info->quant_precision) // only support 8-bit quantization table precision
    {
        return JPEGERR_EUNSUPPORTED;
    }

    // Determine resize scaling factor
    p_engine->scaling_factor = STD_MIN(p_source->p_frame_info->width / p_dest->width,
                                       p_source->p_frame_info->height / p_dest->height);

    // Limit scaling to 0, 1, 2 or 3 (0 - no scaling, 1 - scale by 2, 2 - scale by 4, 3 - scale by 8)
    if (p_engine->scaling_factor >= 8)
    {
        p_engine->scaling_factor = 3;
    }
    else if (p_engine->scaling_factor >= 4)
    {
        p_engine->scaling_factor = 2;
    }
    else
    {
        p_engine->scaling_factor = (p_engine->scaling_factor >= 2) ? 1 : 0;
    }

    JPEG_DBG_LOW("p_engine->scaling_factor = %d\n", p_engine->scaling_factor);
    // Update the destination dimension
    p_dest->width  = p_source->p_frame_info->width  >> p_engine->scaling_factor;
    p_dest->height = p_source->p_frame_info->height >> p_engine->scaling_factor;
    p_dest->subsampling = p_source->p_frame_info->subsampling;

    p_engine->source = *p_source;
    p_engine->dest = *p_dest;

    // update chunk width and height
    *p_output_chunk_width  = ROUND_TO_16(p_source->p_frame_info->width) >> p_engine->scaling_factor;
    *p_output_chunk_height = ((p_source->p_frame_info->subsampling == JPEG_H2V1) ? 8 : 16) >>
        p_engine->scaling_factor;

    jpeg_buffer_mark_empty(p_engine->source.p_buffers[0]);
    jpeg_buffer_mark_empty(p_engine->source.p_buffers[1]);
    //p_engine->buffer_in_use = 0;
    return JPEGERR_SUCCESS;
}

static int jpegd_engine_q5_start(jpegd_engine_obj_t *p_obj)
{
    int rc, i, j, k;
    jpeg_huff_table_t *p_dc_luma_htbl;
    jpeg_huff_table_t *p_ac_luma_htbl;
    jpeg_huff_table_t *p_dc_chroma_htbl;
    jpeg_huff_table_t *p_ac_chroma_htbl;
    uint8_t temp_qtbl[64];
    jpegd_engine_q5_t     *p_engine;
    jpeg_q5_dec_cfg_cmd_t  cfg_cmd;
    jpeg_q5_dec_cmd_t      dec_cmd;
    jpegd_engine_input_fetcher_t *p_fetcher;

    // Validate input arguments
    if (!p_obj || !p_obj->p_engine)
        return JPEGERR_ENULLPTR;

    // Cast p_obj->obj as jpegd_engine_q5_t object
    p_engine = p_obj->p_engine;

    // Make sure engine is idle
    os_mutex_lock(&(p_engine->mutex));
    if (p_engine->state != JPEGD_Q5_IDLE)
    {
        os_mutex_unlock(&(p_engine->mutex));
        JPEG_DBG_HIGH("jpegd_engine_q5_start: engine not idle\n");
        return JPEGERR_EBADSTATE;
    }

    // Clear decode config command structure
    STD_MEMSET(&cfg_cmd, 0, sizeof(jpeg_q5_dec_cfg_cmd_t));

    // Set up header command
    cfg_cmd.header.command = JPEG_Q5_CONFIG_CMD_JPEGD;

    if (p_engine->source.p_frame_info->subsampling == JPEG_H2V2)
    {
        cfg_cmd.height = ROUND_TO_16(p_engine->source.p_frame_info->height);
        cfg_cmd.subsample = JPEG_FORMAT_H2V2;
    }
    else
    {
        cfg_cmd.height = ROUND_TO_8(p_engine->source.p_frame_info->height);
        cfg_cmd.subsample = JPEG_FORMAT_H2V1;
    }
    cfg_cmd.width = ROUND_TO_16(p_engine->source.p_frame_info->width);
    cfg_cmd.scaling_factor = p_engine->scaling_factor;
    cfg_cmd.restart = p_engine->source.p_frame_info->restart_interval;
    cfg_cmd.rtos_partition_num = 0;

    /* Jpeg Decode input buffer configuration */
    /* Request input to fill up the buffer first */
    p_fetcher = &p_engine->input_fetcher;
    p_fetcher->buffer_in_use = 0;
    // Set next byte to fetch
    p_fetcher->next_byte_to_fetch_offset = p_engine->source.p_frame_info->pp_scan_infos[0]->offset;
    rc = jpegd_engine_input_fetcher_fetch(p_fetcher);
    if (JPEG_FAILED(rc))
    {
        return rc;
    }

    // Unregister any possible residual registered PMEM
    (void) jpeg_q5_helper_unregister_pmem(&(p_engine->q5_helper));

    jpeg_q5_helper_register_pmem(&(p_engine->q5_helper), p_fetcher->p_source->p_buffers[0]);
    jpeg_q5_helper_register_pmem(&(p_engine->q5_helper), p_fetcher->p_source->p_buffers[1]);
    cfg_cmd.input_buf_cfg.buffer_size  = p_fetcher->num_bytes_left;
    cfg_cmd.input_buf_cfg.end_of_input = p_fetcher->last_byte_fetched;
    cfg_cmd.input_buf_cfg.p_input_buf  = p_fetcher->p_source->p_buffers[p_fetcher->buffer_in_use]->ptr;

    jpeg_q5_helper_register_pmem(&(p_engine->q5_helper), p_engine->dest.p_luma_buffers[0]);
    jpeg_q5_helper_register_pmem(&(p_engine->q5_helper), p_engine->dest.p_luma_buffers[1]);
    jpeg_q5_helper_register_pmem(&(p_engine->q5_helper), p_engine->dest.p_chroma_buffers[0]);
    jpeg_q5_helper_register_pmem(&(p_engine->q5_helper), p_engine->dest.p_chroma_buffers[1]);

    /* Jpeg Decode output buffer configurations */
    cfg_cmd.output_buf_cfg[0].luma_buf_size = p_engine->dest.p_luma_buffers[0]->size;
    cfg_cmd.output_buf_cfg[0].p_luma_buf = p_engine->dest.p_luma_buffers[0]->ptr;
    cfg_cmd.output_buf_cfg[0].p_chroma_buf = p_engine->dest.p_chroma_buffers[0]->ptr;
    cfg_cmd.output_buf_cfg[1].luma_buf_size = p_engine->dest.p_luma_buffers[1]->size;
    cfg_cmd.output_buf_cfg[1].p_luma_buf = p_engine->dest.p_luma_buffers[1]->ptr;
    cfg_cmd.output_buf_cfg[1].p_chroma_buf = p_engine->dest.p_chroma_buffers[1]->ptr;

    /* Jpeg Decode luma & chroma dequantization table */
    /* Pack 64 8-bit entries into 16 32-bit entries (4th data byte is MSB) */
    for (i = 0; i < 2; i++)
    {
        uint8_t q_table_index = p_engine->source.p_frame_info->p_comp_infos[i].qtable_sel;
        uint16_t *p_src_qtbl = p_engine->source.p_frame_info->p_qtables[q_table_index];

        /* Rearrange zig zag order */
        for (j = 0; j < 64; j++)
        {
            temp_qtbl[zag[j]] = (uint8_t)p_src_qtbl[j];
        }
        for (j = 0; j < 16; j++)
        {
            uint8_t *ptr = temp_qtbl + 3 + j * 4;
            for (k = 0; k < 4; k++)
            {
                cfg_cmd.dequant_tables[i][j] = cfg_cmd.dequant_tables[i][j] << 8;
                cfg_cmd.dequant_tables[i][j] |= (uint32_t)*ptr;
                ptr--;
            }
        }
    }

    /* Jpeg Decode Huffman count tables */
    /* Pack 8-bit entries into 32-bit entries */
    i = p_engine->source.p_frame_info->pp_scan_infos[0]->p_selectors[0].dc_selector;
    p_dc_luma_htbl = &p_engine->source.p_frame_info->p_htables[i];
    i = p_engine->source.p_frame_info->pp_scan_infos[0]->p_selectors[0].ac_selector;
    p_ac_luma_htbl = &p_engine->source.p_frame_info->p_htables[i + 4];
    i = p_engine->source.p_frame_info->pp_scan_infos[0]->p_selectors[1].dc_selector;
    p_dc_chroma_htbl = &p_engine->source.p_frame_info->p_htables[i];
    i = p_engine->source.p_frame_info->pp_scan_infos[0]->p_selectors[1].ac_selector;
    p_ac_chroma_htbl = &p_engine->source.p_frame_info->p_htables[i + 4];

    // Code counts
    for (j = 0; j < 4; j++)
    {
        uint8_t *dc_luma_bits   = p_dc_luma_htbl->bits   + 4 + j * 4;
        uint8_t *ac_luma_bits   = p_ac_luma_htbl->bits   + 4 + j * 4;
        uint8_t *dc_chroma_bits = p_dc_chroma_htbl->bits + 4 + j * 4;
        uint8_t *ac_chroma_bits = p_ac_chroma_htbl->bits + 4 + j * 4;

        for (k = 0; k < 4; k++)
        {
            cfg_cmd.huff_dc_luma_code_count[j] = cfg_cmd.huff_dc_luma_code_count[j] << 8;
            cfg_cmd.huff_dc_luma_code_count[j] |= (uint32_t)*dc_luma_bits;
            dc_luma_bits--;

            cfg_cmd.huff_ac_luma_code_count[j] = cfg_cmd.huff_ac_luma_code_count[j] << 8;
            cfg_cmd.huff_ac_luma_code_count[j] |= (uint32_t)*ac_luma_bits;
            ac_luma_bits--;

            cfg_cmd.huff_dc_chroma_code_count[j] = cfg_cmd.huff_dc_chroma_code_count[j] << 8;
            cfg_cmd.huff_dc_chroma_code_count[j] |= (uint32_t)*dc_chroma_bits;
            dc_chroma_bits--;

            cfg_cmd.huff_ac_chroma_code_count[j] = cfg_cmd.huff_ac_chroma_code_count[j] << 8;
            cfg_cmd.huff_ac_chroma_code_count[j] |= (uint32_t)*ac_chroma_bits;
            ac_chroma_bits--;
        }
    }

    // Code values (dc)
    for (j = 0; j < 3; j++)
    {
        uint8_t *dc_luma_values   = p_dc_luma_htbl->values   + 3 + j * 4;
        uint8_t *dc_chroma_values = p_dc_chroma_htbl->values + 3 + j * 4;

        for (k = 0; k < 4; k++)
        {
            cfg_cmd.huff_dc_luma_code_value[j] = cfg_cmd.huff_dc_luma_code_value[j] << 8;
            cfg_cmd.huff_dc_luma_code_value[j] |= (uint32_t)*dc_luma_values;
            dc_luma_values--;

            cfg_cmd.huff_dc_chroma_code_value[j] = cfg_cmd.huff_dc_chroma_code_value[j] << 8;
            cfg_cmd.huff_dc_chroma_code_value[j] |= (uint32_t)*dc_chroma_values;
            dc_chroma_values--;
        }
    }
    // Code values (ac)
    for (j = 0; j < 41; j++)
    {
        uint8_t *ac_luma_values   = p_ac_luma_htbl->values   + 3 + j * 4;
        uint8_t *ac_chroma_values = p_ac_chroma_htbl->values + 3 + j * 4;

        for (k = 0; k < 4; k++)
        {
            cfg_cmd.huff_ac_luma_code_value[j] = cfg_cmd.huff_ac_luma_code_value[j] << 8;
            cfg_cmd.huff_ac_luma_code_value[j] |= (uint32_t)*ac_luma_values;
            ac_luma_values--;

            cfg_cmd.huff_ac_chroma_code_value[j] = cfg_cmd.huff_ac_chroma_code_value[j] << 8;
            cfg_cmd.huff_ac_chroma_code_value[j] |= (uint32_t)*ac_chroma_values;
            ac_chroma_values--;
        }
    }

    // Send decode config command
    rc = jpeg_q5_helper_send_cfg_command(&(p_engine->q5_helper), (uint8_t *)&cfg_cmd, sizeof(cfg_cmd));
    if (JPEG_FAILED(rc))
    {
        os_mutex_unlock(&(p_engine->mutex));
        return rc;
    }

    // Rest destination buffer index
    p_engine->dst_buf_index = 0;

    p_engine->state = JPEGD_Q5_DECODE_WAIT;

    // Send decode command
    dec_cmd.header = JPEG_Q5_ACTION_CMD_JPEGD_DECODE;
    rc = jpeg_q5_helper_send_action_command(&(p_engine->q5_helper),
                                            (uint8_t *)&dec_cmd,
                                            sizeof(dec_cmd));
    if (JPEG_FAILED(rc))
    {
        p_engine->state = JPEGD_Q5_IDLE;
        os_mutex_unlock(&(p_engine->mutex));
        return rc;
    }

    // Wait for decode acknowledgement from DSP
    JPEG_DBG_LOW("jpegd_engine_q5_start: decode wait\n");
    rc = 0;
    while (p_engine->state == JPEGD_Q5_DECODE_WAIT)
    {
       // Wait for decode ack
        rc = os_cond_timedwait(&(p_engine->cond), &(p_engine->mutex), 2000);
        if (rc < 0) break;
    }
    if (rc < 0)
    {
        p_engine->state = JPEGD_Q5_IDLE;
    }
    os_mutex_unlock(&(p_engine->mutex));
    return (rc < 0) ? JPEGERR_EFAILED : JPEGERR_SUCCESS;
}

static int jpegd_engine_q5_abort(
    jpegd_engine_obj_t *p_obj)
{
    jpegd_engine_q5_t *p_engine;

    // Validate input arguments
    if (!p_obj || !p_obj->p_engine)
        return JPEGERR_ENULLPTR;

    // Cast p_obj->obj as jpegd_engine_q5_t object
    p_engine = p_obj->p_engine;

    // Send IDLE command to DSP if it is actively decoding
    os_mutex_lock(&(p_engine->mutex));
    if (p_engine->state == JPEGD_Q5_DECODING ||
        p_engine->state == JPEGD_Q5_DECODE_WAIT)
    {
        jpeg_q5_dec_idle_cmd_t  idle_cmd;

        p_engine->state = JPEGD_Q5_ABORT_WAIT;
        idle_cmd.header = JPEG_Q5_ACTION_CMD_JPEGD_IDLE;
        if (JPEG_FAILED(jpeg_q5_helper_send_action_command(&(p_engine->q5_helper),
                                                           (uint8_t *)&idle_cmd,
                                                           sizeof(idle_cmd))))
        {
            os_mutex_unlock(&(p_engine->mutex));
            return JPEGERR_EFAILED;
        }
    }
    while (p_engine->state != JPEGD_Q5_IDLE)
    {
        os_cond_wait(&(p_engine->cond), &(p_engine->mutex));
    }
    os_mutex_unlock(&(p_engine->mutex));
    return JPEGERR_SUCCESS;
}

static void jpegd_engine_q5_destroy(
    jpegd_engine_obj_t *p_obj)
{
    if (p_obj)
    {
        jpegd_engine_q5_t *p_engine = (jpegd_engine_q5_t *)(p_obj->p_engine);

        JPEG_DBG_LOW("jpegd_engine_q5_destroy\n");
        // Release allocated memory
        if (p_engine)
        {
            if (p_engine->state != JPEGD_Q5_UNINITIALIZED)
                (void)jpegd_engine_q5_abort(p_obj);

            if (p_engine->thread_running)
            {
                // Signal the event notifier thread to exit
                os_mutex_lock(&(p_engine->mutex));
                p_engine->thread_exit_flag = true;
                os_cond_broadcast(&(p_engine->cond));
                os_mutex_unlock(&(p_engine->mutex));

                // Join the event notifier thread
                os_thread_join(&p_engine->thread, NULL);
            }

            os_mutex_destroy(&(p_engine->mutex));
            os_cond_destroy(&(p_engine->cond));
            jpeg_q5_helper_destroy(&(p_engine->q5_helper));
            JPEG_FREE(p_obj->p_engine);
            p_obj->p_engine = NULL;
        }
    }
}

static void jpegd_engine_q5_event_handler(void     *p_client,
                                          uint16_t  event_id,
                                          uint8_t  *p_buf,
                                          uint32_t  len)
{
    jpegd_engine_q5_t *p_engine = (jpegd_engine_q5_t *)p_client;

    int               rc;

    switch (event_id)
    {
    case JPEG_Q5_MSG_JPEGD_DECODE_ACK:
        {
            JPEG_DBG_LOW("jpegd_engine_q5_event_handler: DECODE ACK\n");
            os_mutex_lock(&(p_engine->mutex));
            if (p_engine->state == JPEGD_Q5_DECODE_WAIT)
            {
                p_engine->state = JPEGD_Q5_DECODING;
            }
            os_cond_broadcast(&(p_engine->cond));
            os_mutex_unlock(&(p_engine->mutex));
        }
        break;
    case JPEG_Q5_MSG_JPEGD_INPUT_REQUEST:
        {
            int rc;
            jpeg_q5_dec_input_request_cmd_t cmd;
            JPEG_DBG_LOW("jpegd_engine_q5_event_handler: INPUT REQUEST\n");
            rc = jpegd_engine_input_fetcher_fetch(&p_engine->input_fetcher);
            if (JPEG_FAILED(rc))
            {
                jpeg_q5_dec_idle_cmd_t  idle_cmd;

                os_mutex_lock(&(p_engine->mutex));
                p_engine->state = JPEGD_Q5_ERROR_WAIT;
                idle_cmd.header = JPEG_Q5_ACTION_CMD_JPEGD_IDLE;
                (void)jpeg_q5_helper_send_action_command(&(p_engine->q5_helper),
                                                         (uint8_t *)&idle_cmd,
                                                         sizeof(idle_cmd));
                os_mutex_unlock(&(p_engine->mutex));
                JPEG_DBG_HIGH("jpegd_engine_q5_event_handler: jpegd_engine_input_fetcher_fetch failed\n");
                break;
            }
            cmd.header = JPEG_Q5_ACTION_CMD_JPEGD_INPUT;
            cmd.input_request.buffer_size  = p_engine->input_fetcher.num_bytes_left;
            // work around for DSP bug before it's fixed and released
            jpeg_buffer_get_max_size(p_engine->source.p_buffers[p_engine->input_fetcher.buffer_in_use],
                                     &cmd.input_request.buffer_size);
            // work around ends
            cmd.input_request.end_of_input = p_engine->input_fetcher.last_byte_fetched;
            cmd.input_request.p_input_buf  = p_engine->input_fetcher.p_source->p_buffers[
                p_engine->input_fetcher.buffer_in_use]->ptr;
            (void)jpeg_q5_helper_send_action_command(&(p_engine->q5_helper),
                                                     (uint8_t *)&cmd,
                                                     sizeof(cmd));
        }
        break;
    case JPEG_Q5_MSG_JPEGD_IDLE_ACK:
        {
            uint8_t done_wait = false;
            uint8_t error_wait = false;
            JPEG_DBG_LOW("jpegd_engine_q5_event_handler: IDLE ACK\n");
            os_mutex_lock(&(p_engine->mutex));
            if (p_engine->state == JPEGD_Q5_DONE_WAIT)
            {
                done_wait = true;
            }
            else if (p_engine->state == JPEGD_Q5_ERROR_WAIT)
            {
                error_wait = true;
            }
            p_engine->state = JPEGD_Q5_IDLE;
            os_cond_broadcast(&(p_engine->cond));
            os_mutex_unlock(&(p_engine->mutex));

            if (done_wait)
            {
                jpegd_engine_q5_enqueue_event(p_engine, JPEG_EVENT_DONE, NULL);
            }
            else if (error_wait)
            {
                jpegd_engine_q5_enqueue_event(p_engine, JPEG_EVENT_ERROR, NULL);
            }
        }
        break;
    case JPEG_Q5_MSG_JPEGD_ILLEGAL_COMMAND:
        {
            os_mutex_lock(&(p_engine->mutex));
            JPEG_DBG_LOW("jpegd_engine_q5_event_handler: ILLEGAL COMMAND\n");
            if (p_engine->state == JPEGD_Q5_DECODE_WAIT)
            {
                p_engine->state = JPEGD_Q5_IDLE;
            }
            os_cond_broadcast(&(p_engine->cond));
            os_mutex_unlock(&(p_engine->mutex));
        }
        break;

    case JPEG_Q5_MSG_JPEGD_OUTPUT_PRODUCED:
        {
            jpeg_q5_dec_output_produced_msg_t msg = *((jpeg_q5_dec_output_produced_msg_t *)p_buf);
            JPEG_DBG_LOW("jpegd_engine_q5_event_handler: OUTPUT PRODUCED\n");
            // Check error code reported by DSP
            if (msg.error_msg)
            {
                jpeg_q5_dec_idle_cmd_t  idle_cmd;
                switch (msg.error_msg)
                {
                case 0x01:
                    jpegd_engine_q5_enqueue_event(p_engine, JPEG_EVENT_ERROR, (void*)"EOI Marker missing");
                    break;
                case 0x02:
                    jpegd_engine_q5_enqueue_event(p_engine, JPEG_EVENT_ERROR, (void*)"Restart marker error");
                    break;
                case 0x03:
                    jpegd_engine_q5_enqueue_event(p_engine, JPEG_EVENT_ERROR, (void*)"Incorrect marker");
                    break;
                case 0x04:
                    jpegd_engine_q5_enqueue_event(p_engine, JPEG_EVENT_ERROR, (void*)"Huffman search exceeded 16 bits");
                    break;
                case 0x05:
                case 0x06:
                    jpegd_engine_q5_enqueue_event(p_engine, JPEG_EVENT_ERROR, (void*)"Huffman exceeded 64 entries");
                    break;
                default:
                    jpegd_engine_q5_enqueue_event(p_engine, JPEG_EVENT_ERROR, (void*)"Invalid error");
                    break;
                }
                // Send IDLE command
                os_mutex_lock(&(p_engine->mutex));
                p_engine->state = JPEGD_Q5_ABORT_WAIT;
                idle_cmd.header = JPEG_Q5_ACTION_CMD_JPEGD_IDLE;
                (void)jpeg_q5_helper_send_action_command(&(p_engine->q5_helper),
                                                         (uint8_t *)&idle_cmd,
                                                         sizeof(idle_cmd));
                os_mutex_unlock(&(p_engine->mutex));
                break;
            }
            // Deliver the output
            rc = p_engine->dest.p_output_handler(p_engine->p_wrapper,
                                            p_engine->dest.p_luma_buffers[p_engine->dst_buf_index],
                                            p_engine->dest.p_chroma_buffers[p_engine->dst_buf_index]);

            // if something goes wrong when sending the output, bail out from the loop and return event error.
            if (JPEG_FAILED(rc))
            {
                jpegd_engine_q5_enqueue_event(p_engine, JPEG_EVENT_ERROR, (void*)"Deliver output error");
                break;
            }

            // Wait for the buffers to be marked as consumed
            jpeg_buffer_wait_until_empty(p_engine->dest.p_luma_buffers[p_engine->dst_buf_index]);
            jpeg_buffer_wait_until_empty(p_engine->dest.p_chroma_buffers[p_engine->dst_buf_index]);

            // Decode is done
            if (msg.decode_state)
            {
                // Send IDLE_CMD to DSP
                jpeg_q5_dec_idle_cmd_t  idle_cmd;
                os_mutex_lock(&(p_engine->mutex));
                p_engine->state = JPEGD_Q5_DONE_WAIT;
                idle_cmd.header = JPEG_Q5_ACTION_CMD_JPEGD_IDLE;
                if (JPEG_FAILED(jpeg_q5_helper_send_action_command(&(p_engine->q5_helper),
                                                                   (uint8_t *)&idle_cmd,
                                                                   sizeof(idle_cmd))))
                {
                    p_engine->state = JPEGD_Q5_IDLE;
                    os_mutex_unlock(&(p_engine->mutex));
                    jpegd_engine_q5_enqueue_event(p_engine, JPEG_EVENT_ERROR,
                                                  "error sending idle command to dsp");
                }
                else
                {
                    os_mutex_unlock(&(p_engine->mutex));
                }
            }
            else
            {
                // Notify DSP with an output consumed command
                jpeg_q5_dec_output_consumed_cmd_t cmd;

                cmd.header = JPEG_Q5_ACTION_CMD_JPEGD_OUTPUT_CONSUMED;
                cmd.output_buffer.p_luma_buf = msg.p_luma_buf;
                cmd.output_buffer.p_chroma_buf = msg.p_chroma_buf;
                cmd.output_buffer.luma_buf_size = p_engine->dest.p_luma_buffers[p_engine->dst_buf_index]->size;
                (void)jpeg_q5_helper_send_action_command(&(p_engine->q5_helper),
                                                         (uint8_t *)&cmd,
                                                         sizeof(cmd));
            }
            // update destination buffer index
            p_engine->dst_buf_index = 1 - p_engine->dst_buf_index;
        }
        break;

    case JPEG_Q5_IOCTL_ERROR:
        os_mutex_lock(&(p_engine->mutex));
        p_engine->state = JPEGD_Q5_IDLE;
        os_mutex_unlock(&(p_engine->mutex));
        jpegd_engine_q5_enqueue_event(p_engine, JPEG_EVENT_ERROR,
                                      (void *)"error communicating with dsp");
        break;
    default:
        break;
    }
}

static void jpegd_engine_q5_enqueue_event(jpegd_engine_q5_t *p_engine,
                                          jpeg_event_t event,
                                          void *payload)
{
    jpegd_engine_event_q_entry *p_entry;

    os_mutex_lock(&(p_engine->mutex));
    if (p_engine->event_q[p_engine->event_q_tail].is_valid)
    {
        JPEG_DBG_ERROR("jpegd_engine_q5_enqueue_event: fatal error: queue is full\n");
        return;
    }

    // Set up event entry
    p_entry = p_engine->event_q + p_engine->event_q_tail;
    p_entry->event = event;
    p_entry->payload = payload;
    p_entry->is_valid = true;

    // Enqueue the entry
    p_engine->event_q_tail = (p_engine->event_q_tail + 1) % EVENT_Q_SIZE;

    // Signal thread to process the event
    os_cond_broadcast(&(p_engine->cond));
    os_mutex_unlock(&(p_engine->mutex));
}

static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
             jpegd_engine_q5_notify_event_thread(OS_THREAD_FUNC_ARG_T arg)
{
    jpegd_engine_q5_t *p_engine = (jpegd_engine_q5_t *)arg;

    for (;;)
    {
        jpegd_engine_event_q_entry *p_entry;

        os_mutex_lock(&(p_engine->mutex));

        // Block if no event is pending
        while (!p_engine->event_q[p_engine->event_q_head].is_valid &&
               !p_engine->thread_exit_flag)
        {
            os_cond_wait(&(p_engine->cond), &(p_engine->mutex));
        }

        // It is signaled that the thread should exit
        if (p_engine->thread_exit_flag)
            break;

        p_entry = p_engine->event_q + p_engine->event_q_head;
        p_engine->p_event_handler(p_engine->p_wrapper,
                                  p_entry->event,
                                  p_entry->payload);
        p_entry->is_valid = false;
        p_engine->event_q_head = (p_engine->event_q_head + 1) % EVENT_Q_SIZE;

        os_mutex_unlock(&(p_engine->mutex));
    }
    return OS_THREAD_FUNC_RET_SUCCEEDED;
}

