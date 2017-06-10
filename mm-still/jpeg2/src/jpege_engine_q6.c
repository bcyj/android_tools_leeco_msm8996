/*****************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential. *
*****************************************************************************/

/*========================================================================
                             Edit History

$Header: //source/qcom/qct/multimedia2/EmbeddedA/Camera/srckit/rel/6.0/still/jpege/core/src/jpege_engine_q6.c#3 $

when       who     what, where, why
--------   ---     -------------------------------------------------------
07/09/13   staceyw  Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

#include "jpeg_buffer_private.h"
#include "jpege_engine_q6.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include "jpeg_common.h"
#include "jpeg_common_private.h"
#include "os_timer_sp.h"
#include "apr_pmem.h"

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
static void jpege_engine_q6_create(
    jpege_engine_hybrid_obj_t *p_obj,
    jpege_engine_hybrid_obj    hybrid);
static int jpege_engine_q6_init(
    jpege_engine_hybrid_obj_t    *p_obj,
    jpege_engine_hybrid_event_handler_t   p_event_handler,
    jpege_engine_hybrid_output_handler_t  p_output_handler);
static int jpege_engine_q6_start(
    jpege_engine_hybrid_obj_t   *p_obj,
    jpege_img_cfg_t             *p_config,
    jpege_img_data_t            *p_source,
    jpege_engine_hybrid_cfg_t   *p_hybrid_cfg);
static int  jpege_engine_q6_abort   (
    jpege_engine_hybrid_obj_t*);
static void jpege_engine_q6_destroy (
    jpege_engine_hybrid_obj_t*);
static void jpege_engine_q6_thread_destroy (
    jpege_engine_hybrid_obj_t*);
static int  jpege_engine_q6_check_start_param (
    jpege_img_cfg_t*,
    jpege_img_data_t*,
    jpege_engine_hybrid_cfg_t*);

static int jpege_engine_q6_configure(
   jpege_engine_q6_t  *p_engine,
   jpege_img_cfg_t    *p_config,
   jpege_img_data_t   *p_source,
   jpege_engine_hybrid_cfg_t *p_hybrid_cfg);

static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
   jpege_engine_q6_encode (OS_THREAD_FUNC_ARG_T);
static const char jpeg_engine_q6_encode_thread_name[] = "jpege_engine_q6_encode";
/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
#define OS_Q6_PAGE  4096
/* =======================================================================
**                      Global Object Definitions
** ======================================================================= */

static const char jpege_engine_q6_name[] = "Jpeg DSP Encode Engine";
jpege_engine_hybrid_profile_t jpege_engine_q6_profile = {jpege_engine_q6_create, jpege_engine_q6_name};

/* =======================================================================
**                          Function Definitions
** ======================================================================= */
static void jpege_engine_q6_create(
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
        p_obj->create            = &jpege_engine_q6_create;
        p_obj->init              = &jpege_engine_q6_init;
        p_obj->check_start_param = &jpege_engine_q6_check_start_param;
        p_obj->start             = &jpege_engine_q6_start;
        p_obj->abort             = &jpege_engine_q6_abort;
        p_obj->destroy           = &jpege_engine_q6_destroy;
        p_obj->thread_destroy    = &jpege_engine_q6_thread_destroy;
        p_obj->p_engine          = NULL;
        p_obj->hybrid            = hybrid;
        p_obj->is_initialized    = false;
    }
}

static int jpege_engine_q6_init(
    jpege_engine_hybrid_obj_t            *p_obj,
    jpege_engine_hybrid_event_handler_t   p_event_handler,
    jpege_engine_hybrid_output_handler_t  p_output_handler)
{
    jpege_engine_q6_t *p_engine;
    int rc;
    // Validate input arguments
    if (!p_obj || !p_event_handler || !p_output_handler)
    {
        JPEG_DBG_ERROR("jpege_engine_q6_init : NULL input pointers");
        return JPEGERR_ENULLPTR;
    }

    rc = adsp_jpege_fastrpc_start();

    if (rc)
    {
        JPEG_DBG_ERROR("jpege_engine_q6_init: adsp_jpege_fastrpc_start() failed: %d\n", rc);
        return JPEGERR_EFAILED;
    }


    rc = adsp_jpege_init();
    // If there is a failure in initing adsp jpege, exit
    if (rc)
    {
        JPEG_DBG_ERROR("jpege_engine_q6_init: adsp_jpege_init() failed: %d\n", rc);
        return JPEGERR_EFAILED;
    }

    // Allocate memory for the engine structure
    p_engine = (jpege_engine_q6_t *)JPEG_MALLOC(sizeof(jpege_engine_q6_t));

    if (!p_engine)
    {
        JPEG_DBG_ERROR("jpege_engine_q6_init : Allocating memeory for the engine structure failed\n");
        return JPEGERR_EMALLOC;
    }

    // Initialize the fields inside the engine structure below
    STD_MEMSET(p_engine, 0, sizeof(jpege_engine_q6_t));  // Zero out the entire structure
    p_engine->p_wrapper = p_obj;                         // Initialize the pointer to the wrapper
    p_engine->p_event_handler = p_event_handler;         // Initialize the event handler
    p_engine->p_output_handler = p_output_handler;       // Initialize the output handler
    os_mutex_init(&(p_engine->mutex));                   // Initialize the mutex
    os_cond_init(&(p_engine->cond));                     // Initialize the condition variables

    // Assign allocated engine structure to p_obj
    p_obj->p_engine = (void *)p_engine;
    p_obj->is_initialized = true;

   return JPEGERR_SUCCESS;
}


static int jpege_engine_q6_check_start_param(
    jpege_img_cfg_t            *p_config,
    jpege_img_data_t           *p_source,
    jpege_engine_hybrid_cfg_t  *p_hybrid_cfg)
{
    /*------------------------------------------------------------
      Input validation
      ------------------------------------------------------------*/
    if (!p_config || !p_source || !p_hybrid_cfg)
        return JPEGERR_ENULLPTR;

    // Validate color format
    if (p_source->color_format > YCBCRLP_H1V1)
    {
        // Supports only YUV color format
        JPEG_DBG_MED("jpege_engine_q6_check_start_param: unsupported color format %d\n", p_source->color_format);
        return JPEGERR_EUNSUPPORTED;
    }

    // Validate rotation
    if (p_config->rotation_degree_clk % 90)
    {
        JPEG_DBG_ERROR("jpege_engine_q6_check_start_param : Unsupported rotation : %d\n",p_config->rotation_degree_clk);
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
        JPEG_DBG_MED("jpege_engine_q6_check_start_param: invalid scale config: \
          (h offset, v offset)(%d, %d)  (input width, input height)(%d, %d) (output width,output height)(%d, %d)\n",
           p_config->scale_cfg.h_offset, p_config->scale_cfg.v_offset,
           p_config->scale_cfg.input_width, p_config->scale_cfg.input_height,
           p_config->scale_cfg.output_width, p_config->scale_cfg.output_height);
        return JPEGERR_EBADPARM;
    }
    // Validate fragment
    if (p_source->fragment_cnt > 1)
    {
        JPEG_DBG_ERROR("jpege_engine_q6_check_start_param: unsupported fragment: %d\n", p_source->fragment_cnt);
        return JPEGERR_EBADPARM;
    }
    // Validate source pointer
    if((p_source->p_fragments[0].color.yuv.luma_buf) && p_source->p_fragments[0].color.yuv.chroma_buf)
    {
        if (((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->pmem_fd < 1 ||
            ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->pmem_fd < 1)
        {
            JPEG_DBG_ERROR("jpege_engine_q6_check_start_param: not valid source pmem fd\n");
            return JPEGERR_EBADPARM;
        }
    }
    else
    {
        JPEG_DBG_ERROR("jpege_engine_q6_check_start_param: luma or chroma source buffer is NULL: luma_buf = 0x%x chroma_buf = 0x%x \n",
            p_source->p_fragments[0].color.yuv.luma_buf,p_source->p_fragments[0].color.yuv.chroma_buf);
        return JPEGERR_ENULLPTR;
    }

    return JPEGERR_SUCCESS;
}

static int jpege_engine_q6_start(
    jpege_engine_hybrid_obj_t   *p_obj,
    jpege_img_cfg_t             *p_config,
    jpege_img_data_t            *p_source,
    jpege_engine_hybrid_cfg_t   *p_hybrid_cfg)
{
    int                    rc;
    jpege_engine_q6_t     *p_engine;

    // Validate input arguments
    if (!p_obj || !p_obj->p_engine ||!p_source || !p_config || !p_hybrid_cfg)
    {
        JPEG_DBG_ERROR("jpege_engine_q6_start : NULL input pointer\n");
        return JPEGERR_ENULLPTR;
    }
    // Cast p_obj->obj as jpege_engine_q6_t object
    p_engine = (jpege_engine_q6_t *)p_obj->p_engine;
    // Make sure no threads are running
    os_mutex_lock(&(p_engine->mutex));
    if (p_engine->is_active)
    {
        os_mutex_unlock(&(p_engine->mutex));
        JPEG_DBG_ERROR("jpege_engine_q6_start: previous encoding still active\n");
        return JPEGERR_EBADSTATE;
    }

    // Configure the engine based on the input configuration
    rc = jpege_engine_q6_configure(p_engine, p_config, p_source, p_hybrid_cfg);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_engine_q6_start : Configuring Engine failed\n");
        os_mutex_unlock(&(p_engine->mutex));
        return rc;
    }

    // Reset abort flag
    p_engine->abort_flag = false;
    // Reset error flag
    p_engine->error_flag = false;

    // Start Jpeg Encoding as a new thread
    rc = os_thread_create(&p_engine->thread,
                          jpege_engine_q6_encode,
                          (void *)p_engine);
    // If there is a failure in creating the thread, clean up and exit
    if (rc)
    {
        os_mutex_unlock(&p_engine->mutex);
        JPEG_DBG_ERROR("jpege_engine_q6_start: os_thread_create() failed: %d\n", rc);
        return JPEGERR_EFAILED;
    }
    // Initialize the state
    p_engine->is_active = true;
    os_mutex_unlock(&(p_engine->mutex));

    // Join the encode thread
    //os_thread_detach(&p_engine->thread);
    return JPEGERR_SUCCESS;
}

static int jpege_engine_q6_configure(
   jpege_engine_q6_t  *p_engine,
   jpege_img_cfg_t    *p_config,
   jpege_img_data_t   *p_source,
   jpege_engine_hybrid_cfg_t *p_hybrid_cfg)
{
    uint32_t                    luma_width, luma_height, chroma_width, chroma_height;
    uint32_t                    rotated_hori_MCU_Count, index;
    jpeg_subsampling_t          subsampling;
    jpege_q6_enc_cfg_target_t   q6_enc_target_cfg;

    for(index = 0; index < 17; index++)
    {
        q6_enc_target_cfg.luma_dc_huff_tbl.bits[index] = p_config->luma_dc_huff_tbl.bits[index];
        q6_enc_target_cfg.chroma_dc_huff_tbl.bits[index] = p_config->chroma_dc_huff_tbl.bits[index];
        q6_enc_target_cfg.luma_ac_huff_tbl.bits[index]= p_config->luma_ac_huff_tbl.bits[index];
        q6_enc_target_cfg.chroma_ac_huff_tbl.bits[index] = p_config->chroma_ac_huff_tbl.bits[index];
    }
    for(index = 0; index < 256; index++)
    {
        q6_enc_target_cfg.luma_dc_huff_tbl.values[index] = p_config->luma_dc_huff_tbl.values[index];
        q6_enc_target_cfg.chroma_dc_huff_tbl.values[index] = p_config->chroma_dc_huff_tbl.values[index];
        q6_enc_target_cfg.luma_ac_huff_tbl.values[index] = p_config->luma_ac_huff_tbl.values[index];
        q6_enc_target_cfg.chroma_ac_huff_tbl.values[index] = p_config->chroma_ac_huff_tbl.values[index];
    }

    q6_enc_target_cfg.rotation = p_config->rotation_degree_clk;
    q6_enc_target_cfg.restart_interval = p_config->restart_interval;
    q6_enc_target_cfg.base_restart_marker = p_hybrid_cfg->base_restart_marker;
    // Program Quant Tables
    for(index = 0; index < 64; index++)
    {
        q6_enc_target_cfg.qtbl_0[index] = p_config->luma_quant_tbl[index];
        q6_enc_target_cfg.qtbl_1[index] = p_config->chroma_quant_tbl[index];
    }

    q6_enc_target_cfg.input_width = p_source->width;
    q6_enc_target_cfg.input_height = p_source->height;
    q6_enc_target_cfg.input_stride = p_source->width;
    q6_enc_target_cfg.output_MCUs = p_hybrid_cfg->output_MCUs;
    q6_enc_target_cfg.color_format = (jpege_q6_color_format_t)p_source->color_format;

    q6_enc_target_cfg.output_buffer_length = p_hybrid_cfg->output_buffer_length;

    q6_enc_target_cfg.scale_cfg.crop_width = p_config->scale_cfg.input_width;
    q6_enc_target_cfg.scale_cfg.crop_height = p_config->scale_cfg.input_height;
    q6_enc_target_cfg.scale_cfg.h_offset = p_config->scale_cfg.h_offset;
    q6_enc_target_cfg.scale_cfg.v_offset = p_config->scale_cfg.v_offset;
    q6_enc_target_cfg.scale_cfg.output_width = p_config->scale_cfg.output_width;
    q6_enc_target_cfg.scale_cfg.output_height = p_config->scale_cfg.output_height;
    q6_enc_target_cfg.scale_cfg.enable = p_config->scale_cfg.enable;

    if (p_config->scale_cfg.enable)
    {
        luma_width  = p_config->scale_cfg.output_width;
        luma_height  = p_config->scale_cfg.output_height;
    }
    else
    {
        luma_width = p_source->width;
        luma_height = p_source->height;
    }

    subsampling = (jpeg_subsampling_t)((uint8_t)p_source->color_format / 2);
    switch (subsampling)
    {
        case JPEG_H1V1:
        {
            chroma_width = luma_width;
            chroma_height = luma_height;
            break;
        }
        case JPEG_H1V2:
        {
            chroma_width = luma_width;
            chroma_height = (luma_height + 1) >> 1;           // odd luma support
            break;
        }
        case JPEG_H2V1:
        {
            chroma_width = (luma_width + 1) >> 1;             // odd luma support
            chroma_height = luma_height;
            break;
        }
        case JPEG_H2V2:
        {
            chroma_width = (luma_width + 1) >> 1;             // odd luma support
            chroma_height = (luma_height + 1) >> 1;           // odd luma support
            break;
        }
        default:
            JPEG_DBG_ERROR("jpege_engine_q6_configure: invalid jpeg subsampling: %d\n", subsampling);
            return JPEGERR_EBADPARM;
    }

    switch (p_config->rotation_degree_clk)
    {
        case 0:
        case 180:
        {
            rotated_hori_MCU_Count  = (chroma_width+ 7) >> 3;  // divide by 8
            break;
        }
        case 90:
        case 270:
        {
            rotated_hori_MCU_Count  = (chroma_height + 7) >> 3; // divide by 8
            break;
        }
        default:
            JPEG_DBG_ERROR("jpege_engine_q6_configure: unrecognized rotation: %d\n", p_config->rotation_degree_clk);
            return JPEGERR_EBADPARM;
    }

    if (!p_config->scale_cfg.enable)
    {
        q6_enc_target_cfg.output_start_mcu_index = rotated_hori_MCU_Count * p_hybrid_cfg->start_vert_MCU_index;
    }
    else
    {
        q6_enc_target_cfg.output_start_mcu_index = (rotated_hori_MCU_Count * p_hybrid_cfg->scale_cfg.vert_chroma_row_index) >> 3;
    }

    p_engine->q6_enc_target_cfg = q6_enc_target_cfg;
    p_engine->output_buffer_ptr = p_hybrid_cfg->output_buffer_ptr;

    /* Initialize input Y and CbCr/CrCb pointers */
    if((p_source->p_fragments[0].color.yuv.luma_buf) && (p_source->p_fragments[0].color.yuv.chroma_buf))
    {
        p_engine->plane_ptr[0] = ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->ptr + ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->start_offset +
                                ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->offset;
        p_engine->plane_ptr[1] = ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->ptr + ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->start_offset +
                                ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->offset;
        p_engine->plane_ptr_size[0] = ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->size -((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->start_offset -
                                    ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->offset;
        p_engine->plane_ptr_size[1] = ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->size - ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->start_offset -
                                    ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->offset;

    p_engine->plane_ptr[0] = ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->ptr;
    p_engine->plane_ptr[1] = ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->ptr;
    p_engine->plane_ptr_size[0] = ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->size ;
    p_engine->plane_ptr_size[1] = ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->size;

    }
    else
    {
        JPEG_DBG_ERROR("jpege_engine_q6_configure : NULL input luma buffer or chroma buffer\n");
        return JPEGERR_ENULLPTR;
    }

    return JPEGERR_SUCCESS;
}

static OS_THREAD_FUNC_RET_T OS_THREAD_FUNC_MODIFIER
    jpege_engine_q6_encode(OS_THREAD_FUNC_ARG_T arg)
{
    uint32_t                 jpeg_output_size = 0 ;
    int                      rc =0;
    uint32_t                 thread_flag = 1;
    uint32_t                 p_output_buffer_size;
    uint8_t                 *input_buffer_chroma, *input_buffer_luma;
    struct   mmap_info_ion   mapion_output;
    uint32_t                 ion_heapid;
    uint8_t                 *p_output_buffer;

    // Cast input argument as the engine object
    jpege_engine_q6_t *p_engine = (jpege_engine_q6_t *)arg;

    p_output_buffer_size = p_engine->q6_enc_target_cfg.output_buffer_length;

    input_buffer_luma = p_engine->plane_ptr[0];
    input_buffer_chroma = p_engine->plane_ptr[1];


    JPEG_DBG_HIGH("jpege_engine_q6_encode: input configuration\n");
    JPEG_DBG_HIGH("base_restart_marker = %d \n", p_engine->q6_enc_target_cfg.base_restart_marker);
    JPEG_DBG_HIGH("restart_interval = %d \n", p_engine->q6_enc_target_cfg.restart_interval);
    JPEG_DBG_HIGH("rotation = %d \n",p_engine->q6_enc_target_cfg.rotation);
    JPEG_DBG_HIGH("scale_cfg = %d \n", p_engine->q6_enc_target_cfg.scale_cfg);
    JPEG_DBG_HIGH("input_width = %d \n", p_engine->q6_enc_target_cfg.input_width);
    JPEG_DBG_HIGH("input_height = %d \n", p_engine->q6_enc_target_cfg.input_height);
    JPEG_DBG_HIGH("input_stride = %d \n", p_engine->q6_enc_target_cfg.input_stride);
    JPEG_DBG_HIGH("output_MCUs = %d \n", p_engine->q6_enc_target_cfg.output_MCUs);
    JPEG_DBG_HIGH("output_start_mcu_index = %d \n", p_engine->q6_enc_target_cfg.output_start_mcu_index);
    JPEG_DBG_HIGH("Colour Format 0x%x \n", p_engine->q6_enc_target_cfg.color_format);
    JPEG_DBG_HIGH("output ptr 0x%x \n", p_engine->output_buffer_ptr);
    JPEG_DBG_HIGH("p_output_buffer_size = %d \n", p_output_buffer_size);
    JPEG_DBG_HIGH("input_buffer_luma size = %d \n", p_engine->plane_ptr_size[0]);
    JPEG_DBG_HIGH("input_buffer_chroma size = %d \n", p_engine->plane_ptr_size[1]);
    JPEG_DBG_HIGH("input luma ptr 0x%x \n", input_buffer_luma);
    JPEG_DBG_HIGH("input chroma ptr0x%x \n", input_buffer_chroma);

    mapion_output.bufsize = p_engine->q6_enc_target_cfg.output_buffer_length + OS_Q6_PAGE;
    ion_heapid = (1 << ION_IOMMU_HEAP_ID);
    (void) apr_pmem_alloc_ion_cached(&mapion_output, ion_heapid);
    p_output_buffer = mapion_output.pVirtualAddr;

    rc = adsp_jpege_q6_process(&(p_engine->q6_enc_target_cfg),
        input_buffer_luma,
        (int)p_engine->plane_ptr_size[0],
        input_buffer_chroma,
        (int)p_engine->plane_ptr_size[1],
        NULL,
        0,
        NULL,
        0,
        (uint32*)&(jpeg_output_size),
        p_output_buffer,
        (int)p_output_buffer_size,
        thread_flag);

    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_ERROR("jpege_engine_q6_encode: adsp_jpege_q6_process failed %d\n", rc);
    }
    if (JPEG_SUCCEEDED(rc))
    {
        JPEG_DBG_ERROR("jpege_engine_q6_encode: adsp_jpege_q6_process succeeded %d size %d \n", rc, (int)p_output_buffer_size);
        (void)STD_MEMMOVE(p_engine->output_buffer_ptr, p_output_buffer, jpeg_output_size);

        rc = p_engine->p_output_handler(p_engine->p_wrapper,
                                        p_engine->output_buffer_ptr,
                                        jpeg_output_size);
        if (JPEG_FAILED(rc))
        {
            JPEG_DBG_ERROR("jpege_engine_q6_encode: failed to flush output buffer\n");
        }
    }

    apr_pmem_free_ion(mapion_output);

    if (JPEG_FAILED(rc))
    {
        os_mutex_lock(&(p_engine->mutex));
        p_engine->error_flag = 1;
        os_mutex_unlock(&(p_engine->mutex));
        JPEG_DBG_ERROR("jpege_engine_q6_encode: set error flag.\n");
    }

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
    return OS_THREAD_FUNC_RET_SUCCEEDED;

}

static int jpege_engine_q6_abort(
    jpege_engine_hybrid_obj_t *p_obj)
{
    jpege_engine_q6_t *p_engine;
    int rc;

    // Validate input arguments
    if (!p_obj || !p_obj->p_engine)
    {
        JPEG_DBG_ERROR("jpege_engine_q6_abort : NULL input pointer\n");
        return JPEGERR_ENULLPTR;
    }

    rc = adsp_jpege_deinit();
    // If there is a failure in deiniting adsp jpege
    if (rc)
    {
        JPEG_DBG_ERROR("jpege_engine_q6_abort: adsp_jpege_deinit() failed: %d\n", rc);
    }

    // Cast p_obj->obj as jpege_engine_q6_t object
    p_engine = (jpege_engine_q6_t *)p_obj->p_engine;

    // Abort if engine is actively encoding
    os_mutex_lock(&(p_engine->mutex));

    if (p_engine->is_active)
    {
        JPEG_DBG_LOW("jpege_engine_q6_abort: engine active.\n");
        p_engine->abort_flag = true;
        JPEG_DBG_HIGH("jpege_engine_q6_abort: waiting to finish.\n");
        while (p_engine->abort_flag)
        {
            os_cond_wait(&(p_engine->cond), &(p_engine->mutex));
        }
        JPEG_DBG_LOW("jpege_engine_q6_abort: aborted.\n");
    }
    os_mutex_unlock(&(p_engine->mutex));
    return JPEGERR_SUCCESS;
}

static void jpege_engine_q6_thread_destroy(
    jpege_engine_hybrid_obj_t *p_obj)
{
    if (p_obj)
    {
        jpege_engine_q6_t *p_engine = (jpege_engine_q6_t *)(p_obj->p_engine);

        if (p_engine->thread)
        {
            os_thread_join(&p_engine->thread, NULL);
        }
    }
}

static void jpege_engine_q6_destroy(
    jpege_engine_hybrid_obj_t *p_obj)
{
    if (p_obj)
    {
        jpege_engine_q6_t *p_engine = (jpege_engine_q6_t *)(p_obj->p_engine);
        // Abort and wait until engine is done with current encoding
        (void)jpege_engine_q6_abort(p_obj);
        jpege_engine_q6_thread_destroy(p_obj);
        // Release allocated memory
        if (p_engine)
        {
            // Signal the output thread to exit
            os_mutex_destroy(&(p_engine->mutex));
            os_cond_destroy(&(p_engine->cond));
            JPEG_FREE(p_obj->p_engine);
            p_obj->p_engine = NULL;
        }
    }
}
