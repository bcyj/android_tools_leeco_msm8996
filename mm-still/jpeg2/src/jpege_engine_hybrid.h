/*****************************************************************************
* Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential. *
*****************************************************************************/

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
10/16/14   vgaur   Added support for multi threaded fastCV.
07/09/13   staceyw Created file.

========================================================================== */

#ifndef _JPEGE_ENGINE_HYBRID_H
#define _JPEGE_ENGINE_HYBRID_H

#include "jpeg_common.h"
#include "jpege_engine.h"

typedef struct jpege_engine_hybrid_obj_struct jpege_engine_hybrid_obj_t;

struct jpege_engine_hybrid_t;
typedef struct jpege_engine_hybrid_t *jpege_engine_hybrid_obj;

/* The Jpeg Encode Engine event handler definition */
typedef void(*jpege_engine_hybrid_event_handler_t)(
    jpege_engine_hybrid_obj_t  *p_obj,      // The encode engine object calling it
    jpeg_event_t                event,      // The event code
    void                       *p_arg);     // An optional argument for that event.
                                // JPEG_EVENT_DONE:    unused
                                // JPEG_EVENT_WARNING: const char * (warning message)
                                // JPEG_EVENT_ERROR:   const char * (error message)

/* The handler for dealing with output produced by the Jpeg Encode Engine */
typedef int (*jpege_engine_hybrid_output_handler_t)(
    jpege_engine_hybrid_obj_t  *p_obj,       // The encode engine object calling it
    uint8_t                    *buf_ptr,     // The buffer containing the output bitstream_t
    uint32_t                    buf_offset); // The flag to indicate last output buffer

/* Function to create a Jpeg Encode Engine */
typedef void(*jpege_engine_hybrid_create_t)(
    jpege_engine_hybrid_obj_t  *p_obj,
    jpege_engine_hybrid_obj     hybrid);

/* Profile of a Jpeg Encode Engine */
typedef struct
{
    jpege_engine_hybrid_create_t create_func;
    const char*                  engine_name;

} jpege_engine_hybrid_profile_t;

typedef void(*jpege_engine_hybrid_destroy_t)(
    jpege_engine_hybrid_obj_t   *p_obj);

typedef void(*jpege_engine_hybrid_thread_destroy_t)(
    jpege_engine_hybrid_obj_t   *p_obj);

typedef struct
{
    uint32_t                     input_width;   // input width
    uint32_t                     input_height;  // input height
    uint32_t                     h_offset;      // h_offset
    uint32_t                     v_offset;      // v_offset
    uint32_t                     output_width;  // output width
    uint32_t                     output_height; // output height
    uint8_t                      enable;        // enable flag
    uint8_t                      fastCV_enable; // Fast CV flag

    // Row Index for next upscale
    uint32_t                     vert_luma_row_index;
    uint32_t                     vert_luma_output_height;

    // Row Index for next chroma upscale
    uint32_t                     vert_chroma_row_index;
    uint32_t                     vert_chroma_output_height;

    uint32_t                     piece_height;
    uint8_t                     *fastCV_buffer;

} jpege_engine_hybrid_scale_cfg_t;

typedef struct jpege_engine_hybrid_cfg_t
{
    // image rotation
    uint32_t                     rotation;
    // restart interval
    uint32_t                     restart_interval;
    // restart marker base for current piece
    uint32_t                     base_restart_marker;
    // restart MCU count
    uint32_t                     restart_mcu_count;
    // quant tables
    uint16_t                    *luma_qtbl;
    uint16_t                    *chroma_qtbl;

    // The luma Huffman table for DC components.
    jpeg_huff_table_t            luma_dc_huff_tbl;
    // The chroma Huffman table for DC components.
    jpeg_huff_table_t            chroma_dc_huff_tbl;
    // The luma Huffman table for AC components.
    jpeg_huff_table_t            luma_ac_huff_tbl;
    // The chroma Huffman table for AC components.
    jpeg_huff_table_t            chroma_ac_huff_tbl;

    // input width/height/stride
    uint32_t                     input_width;
    uint32_t                     input_height;
    uint32_t                     input_stride;
    // input address of whole image
    uint8_t                     *luma_ptr;
    uint8_t                     *chroma_ptr;
    // offset address for current piece
    uint32_t                     luma_offset;
    uint32_t                     chroma_offset;
    // output MCUs and output buffer address with length
    uint32_t                     output_MCUs;
    uint8_t                     *output_buffer_ptr;
    uint32_t                     output_buffer_length;

    // Color format of the image data (e.g. YCRCBLP_H2V2)
    jpeg_color_format_t          color_format;
    uint32_t                     src_index;
    uint32_t                     start_hori_MCU_index;
    uint32_t                     start_vert_MCU_index;
    uint32_t                     fragment_cnt;

    uint32_t                     quality;
    jpege_engine_hybrid_scale_cfg_t  scale_cfg;

} jpege_engine_hybrid_cfg_t;

typedef int(*jpege_engine_hybrid_init_t)(
    jpege_engine_hybrid_obj_t            *p_obj,
    jpege_engine_hybrid_event_handler_t   p_event_handler,
    jpege_engine_hybrid_output_handler_t  p_output_handler);

typedef int(*jpege_engine_hybrid_check_start_param_t)(
    jpege_img_cfg_t             *p_config,
    jpege_img_data_t            *p_source,
    jpege_engine_hybrid_cfg_t   *p_hybrid_cfg);

typedef int(*jpege_engine_hybrid_start_t)(
    jpege_engine_hybrid_obj_t   *p_obj,
    jpege_img_cfg_t             *p_config,
    jpege_img_data_t            *p_source,
    jpege_engine_hybrid_cfg_t   *p_hybrid_cfg);

typedef int(*jpege_engine_hybrid_abort_t)(
    jpege_engine_hybrid_obj_t   *p_obj);

/* Definition of Jpeg Encode Engine object */
struct jpege_engine_hybrid_obj_struct
{
    jpege_engine_hybrid_create_t              create;
    jpege_engine_hybrid_init_t                init;
    jpege_engine_hybrid_check_start_param_t   check_start_param;
    jpege_engine_hybrid_start_t               start;
    jpege_engine_hybrid_abort_t               abort;
    jpege_engine_hybrid_destroy_t             destroy;
    jpege_engine_hybrid_thread_destroy_t      thread_destroy;
    void                                     *p_engine;
    jpege_engine_hybrid_obj                   hybrid;
    uint8_t                                   is_initialized;
    jpege_engine_hybrid_obj_t                *p_wrapper;          // The wrapper engine object

};

#endif /* _JPEGE_ENGINE_HYBRID_H */
