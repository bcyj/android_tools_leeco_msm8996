/*========================================================================

*//** @file jpeg_postprocessor.h

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
11/18/09   mingy   Added tiling output support and dequeue declaration.
11/03/09   mingy   Added CbCr output support.
09/21/09   mingy   Added new variables for region based decoding.
08/03/09   vma     Switched to use the os abstraction layer (os_*)
05/12/09   mingy   Added Data Move, Color Convert and Down Scale operation
                   structures.
                   Added post process function pointer.
                   Added rotation and stride support.
03/05/09   leim    a) Added H1V1, H1V2 sampling format support;
                   b) Added aRGB output format
07/07/08   vma     Created file.
========================================================================== */

#ifndef _JPEG_POSTPROCESSOR_H
#define _JPEG_POSTPROCESSOR_H

#include "os_thread.h"
#include "jpeg_common.h"
#include "jpegd_engine.h"
#include "jpegd.h"
#include "jpeg_queue.h"

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define OUTPUT_BUF_Q_SIZE       8
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */

typedef enum
{
    H2V2_TO_H2V2,
    H2V2_TO_H2V1,
    H2V1_TO_H2V2,
    H2V1_TO_H2V1,
    H1V2_TO_H2V2,
    H1V2_TO_H2V1,
    H1V1_TO_H2V2,
    H1V1_TO_H2V1,

    H2V2_TO_H2V2_R90,
    H2V2_TO_H2V1_R90,
    H2V1_TO_H2V2_R90,
    H2V1_TO_H2V1_R90,
    H1V2_TO_H2V2_R90,
    H1V2_TO_H2V1_R90,
    H1V1_TO_H2V2_R90,
    H1V1_TO_H2V1_R90,

    H2V2_TO_H2V2_R180,
    H2V2_TO_H2V1_R180,
    H2V1_TO_H2V2_R180,
    H2V1_TO_H2V1_R180,
    H1V2_TO_H2V2_R180,
    H1V2_TO_H2V1_R180,
    H1V1_TO_H2V2_R180,
    H1V1_TO_H2V1_R180,

    H2V2_TO_H2V2_R270,
    H2V2_TO_H2V1_R270,
    H2V1_TO_H2V2_R270,
    H2V1_TO_H2V1_R270,
    H1V2_TO_H2V2_R270,
    H1V2_TO_H2V1_R270,
    H1V1_TO_H2V2_R270,
    H1V1_TO_H2V1_R270,

    H2V2_TO_H2V2_CbCr,
    H2V2_TO_H2V1_CbCr,
    H2V1_TO_H2V2_CbCr,
    H2V1_TO_H2V1_CbCr,
    H1V2_TO_H2V2_CbCr,
    H1V2_TO_H2V1_CbCr,
    H1V1_TO_H2V2_CbCr,
    H1V1_TO_H2V1_CbCr,

    H2V2_TO_H2V2_R90_CbCr,
    H2V2_TO_H2V1_R90_CbCr,
    H2V1_TO_H2V2_R90_CbCr,
    H2V1_TO_H2V1_R90_CbCr,
    H1V2_TO_H2V2_R90_CbCr,
    H1V2_TO_H2V1_R90_CbCr,
    H1V1_TO_H2V2_R90_CbCr,
    H1V1_TO_H2V1_R90_CbCr,

    H2V2_TO_H2V2_R180_CbCr,
    H2V2_TO_H2V1_R180_CbCr,
    H2V1_TO_H2V2_R180_CbCr,
    H2V1_TO_H2V1_R180_CbCr,
    H1V2_TO_H2V2_R180_CbCr,
    H1V2_TO_H2V1_R180_CbCr,
    H1V1_TO_H2V2_R180_CbCr,
    H1V1_TO_H2V1_R180_CbCr,

    H2V2_TO_H2V2_R270_CbCr,
    H2V2_TO_H2V1_R270_CbCr,
    H2V1_TO_H2V2_R270_CbCr,
    H2V1_TO_H2V1_R270_CbCr,
    H1V2_TO_H2V2_R270_CbCr,
    H1V2_TO_H2V1_R270_CbCr,
    H1V1_TO_H2V2_R270_CbCr,
    H1V1_TO_H2V1_R270_CbCr,

    DATA_MOVE_MAX

} jpeg_data_move_t;

typedef enum
{
    H2V2_TO_RGB565,
    H2V2_TO_RGB888,
    H2V2_TO_RGBa,
    H2V1_TO_RGB565,
    H2V1_TO_RGB888,
    H2V1_TO_RGBa,
    H1V2_TO_RGB565,
    H1V2_TO_RGB888,
    H1V2_TO_RGBa,
    H1V1_TO_RGB565,
    H1V1_TO_RGB888,
    H1V1_TO_RGBa,

    H2V2_TO_RGB565_R90,
    H2V2_TO_RGB888_R90,
    H2V2_TO_RGBa_R90,
    H2V1_TO_RGB565_R90,
    H2V1_TO_RGB888_R90,
    H2V1_TO_RGBa_R90,
    H1V2_TO_RGB565_R90,
    H1V2_TO_RGB888_R90,
    H1V2_TO_RGBa_R90,
    H1V1_TO_RGB565_R90,
    H1V1_TO_RGB888_R90,
    H1V1_TO_RGBa_R90,

    H2V2_TO_RGB565_R180,
    H2V2_TO_RGB888_R180,
    H2V2_TO_RGBa_R180,
    H2V1_TO_RGB565_R180,
    H2V1_TO_RGB888_R180,
    H2V1_TO_RGBa_R180,
    H1V2_TO_RGB565_R180,
    H1V2_TO_RGB888_R180,
    H1V2_TO_RGBa_R180,
    H1V1_TO_RGB565_R180,
    H1V1_TO_RGB888_R180,
    H1V1_TO_RGBa_R180,

    H2V2_TO_RGB565_R270,
    H2V2_TO_RGB888_R270,
    H2V2_TO_RGBa_R270,
    H2V1_TO_RGB565_R270,
    H2V1_TO_RGB888_R270,
    H2V1_TO_RGBa_R270,
    H1V2_TO_RGB565_R270,
    H1V2_TO_RGB888_R270,
    H1V2_TO_RGBa_R270,
    H1V1_TO_RGB565_R270,
    H1V1_TO_RGB888_R270,
    H1V1_TO_RGBa_R270,

    COLOR_CONVERT_MAX

}jpeg_color_convert_t;

typedef enum
{
    DS_H2V2,
    DS_H2V1,
    DS_RGB888,
    DS_RGB565,
    DS_RGBa,

    DS_H2V2_R90,
    DS_H2V1_R90,
    DS_RGB888_R90,
    DS_RGB565_R90,
    DS_RGBa_R90,

    DS_H2V2_R180,
    DS_H2V1_R180,
    DS_RGB888_R180,
    DS_RGB565_R180,
    DS_RGBa_R180,

    DS_H2V2_R270,
    DS_H2V1_R270,
    DS_RGB888_R270,
    DS_RGB565_R270,
    DS_RGBa_R270,

    DS_H2V2_CbCr,
    DS_H2V1_CbCr,

    DS_H2V2_R90_CbCr,
    DS_H2V1_R90_CbCr,

    DS_H2V2_R180_CbCr,
    DS_H2V1_R180_CbCr,

    DS_H2V2_R270_CbCr,
    DS_H2V1_R270_CbCr,

    DOWN_SCALE_MAX

}jpeg_down_scale_t;

/** ==========================================================================
 * define jpegd post process function parameter structure
 * =========================================================================*/
typedef struct jpeg_postprocessor_struct_t jpeg_postprocessor_t;

typedef struct jpegd_postprocess_func_param_t
{
    jpeg_postprocessor_t *p_processor;
    uint32_t nRowsToProcess;
    uint32_t nColsToProcess;
    uint8_t *pSrcLuma;
    uint8_t *pSrcChroma;
    uint8_t *pDstLuma;
    uint8_t *pDstChroma;
    uint8_t *pDstRGB;

} jpegd_postprocess_func_param_t;

// post process function type
typedef void (*jpegd_postprocess_t)(jpegd_postprocess_func_param_t *post_process);

// post process structure definition
struct jpeg_postprocessor_struct_t
{
    // os mutex object
    os_mutex_t           mutex;
    // os condition variable
    os_cond_t            cond;
    // jpegd destination object
    jpegd_dst_t         *p_dest;

    // post process function
    jpegd_postprocess_t  jpegd_postprocess_func;

    // postprocess operations
    jpeg_data_move_t     data_move;
    jpeg_color_convert_t color_convert;
    jpeg_down_scale_t    down_scale;

     // Input subsampling
    jpeg_subsampling_t   inputSubsampling;

    // Input dimension
    uint32_t             nInputWidth;
    uint32_t             nInputHeight;

    // Output dimension
    uint32_t             nOutputWidth;
    uint32_t             nOutputHeight;

    // Luma/Chroma/RGB destination buffer
    uint8_t             *pDestinationLuma;
    uint8_t             *pDestinationChroma;
    uint8_t             *pDestinationRGB;

    // number of rows or columns processed
    uint32_t             nRowsProcessed;
    uint32_t             nColsProcessed;

    // status flags
    uint8_t              fVerticalChroma;

    // buffer dimensions
    uint32_t             nInputBufferWidth;
    uint32_t             nInputBufferHeight;
    uint32_t             nInputBufferLumaRowIncr;

    // Upsampled chroma values
    uint8_t              *pChromaSmpOutput;

    // intermediate values
    uint32_t            *pLumaAccum;
    uint32_t            *pChromaAccum;

    uint8_t             *pTempLumaLine;
    uint8_t             *pTempChromaLine;

    uint32_t             nVStep;
    uint32_t             nVCount;

    // Engine output buffer (MCU Row) dimension
    uint32_t             chunk_width;
    uint32_t             chunk_height;

    /**************************************************************************
    **  Save the final output buffer dimensions.
    **  for 90/270 rotation cases, the final output height is saved in nHeight,
    **  the intermediate computations will use the swapped output width/height.
    **  the Down scalar algorithm is based on non-rotated dimensions,
    **  output width/height is swapped before fed to down scalar,
    **  nHeight will save the final output height.
    *************************************************************************/
    uint32_t             nStride;
    uint32_t             nHeight;

    // Address offset for rotation cases
    uint32_t  nLumaAddressOffset;
    uint32_t  nChromaAddressOffset;
    uint32_t  nRGBAddressOffset;

    /// Flag to indicate if the current MCU Row is the first Row
    uint8_t              is_first_row;

    // Region offset within the MCU Row
    uint32_t             region_top_offset_in_mcu_row_y;
    uint32_t             region_top_offset_in_mcu_row_crcb;
    uint32_t             region_left_offset_in_mcu_row_y;
    uint32_t             region_left_offset_in_mcu_row_crcb;
    uint32_t             lines_to_skip;

    // Output buffer queue
    jpegd_output_buf_t   output_buffers[OUTPUT_BUF_Q_SIZE];
    // Output buffer queue head
    uint16_t             output_buf_q_head;
    // Output buffer queue tail
    uint16_t             output_buf_q_tail;

	//Output buffer queue Used by mpod
	jpeg_queue_t             output_queue;

    // The pointer to user supplied data (store untouched)
    void                 *p_user_data;

    uint32_t             first_row_id;
    uint8_t              is_last_buffer;

    // The Jpeg output handler
    jpegd_output_handler_t  p_output_handler;

    /***********************************************************************
    * Internal MCU Row buffer used by color convert to write converted
    * rgb data.
    * Then postprosessor copies the data from this internal MCU Row buffer
    * to the tiling buffer and send it out.
    ***********************************************************************/
    uint8_t              *p_internal_buf;

    // flag indicating if tiling is enabled
    uint8_t              tiling_enabled;

    // Abort flag
    uint8_t              abort_flag;

} ;


/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
#define _CLAMP(x) ( ((x) & 0xFFFFFF00) ? ((~(x)>>31) & 0xFF) : (x))

/* =======================================================================
**                          Function Declarations
** ======================================================================= */

/* Entry function to create a software-based Jpeg Decode Engine */
int  jpeg_postprocessor_init     (jpeg_postprocessor_t   *p_processor,
                                  jpegd_output_handler_t  p_output_handler,
                                  void                   *p_user_data);
int  jpeg_postprocessor_configure(jpeg_postprocessor_t *p_processor,
                                  jpegd_engine_dst_t   *p_engine_dst,
                                  jpegd_dst_t          *p_dest,
                                  uint32_t              chunk_width,
                                  uint32_t              chunk_height,
                                  uint8_t               use_pmem,
                                  int32_t               rotation);
int  jpeg_postprocessor_process  (jpeg_postprocessor_t   *p_processor,
                                  jpeg_buf_t           *p_luma_buf,
                                  jpeg_buf_t           *p_chroma_buf);
void jpeg_postprocessor_destroy  (jpeg_postprocessor_t *p_processor);
int  jpeg_postprocessor_dequeue_output_buf(jpeg_postprocessor_t *p_processor,
                                           jpegd_output_buf_t  **p_output_buf);

/**************************************************************************
** Data Move functions.
*************************************************************************/
void data_move(jpegd_postprocess_func_param_t *postprocess);
void data_move_rot90(jpegd_postprocess_func_param_t *postprocess);
void data_move_rot180(jpegd_postprocess_func_param_t *postprocess);
void data_move_rot270(jpegd_postprocess_func_param_t *postprocess);

/**************************************************************************
** Color Convert functions.
*************************************************************************/
void color_convert(jpegd_postprocess_func_param_t *postprocess);
void color_convert_rot90(jpegd_postprocess_func_param_t *postprocess);
void color_convert_rot180(jpegd_postprocess_func_param_t *postprocess);
void color_convert_rot270(jpegd_postprocess_func_param_t *postprocess);

/**************************************************************************
** Down Scale functions.
*************************************************************************/
void down_scale(jpegd_postprocess_func_param_t *post_process);

/**************************************************************************
** Color converting functions, equivalent to VaNum Assembly code.
** These functions are in a seperate file in accordance with the
** corresponding VaNum file.
*************************************************************************/
void y1vu2rgb565line(uint8_t *, uint8_t *, uint8_t *, uint32_t);
void y1vu2rgb888line(uint8_t *, uint8_t *, uint8_t *, uint32_t);
void y1vu2rgbaline  (uint8_t *, uint8_t *, uint8_t *, uint32_t);
void y2vu2rgb565line(uint8_t *, uint8_t *, uint8_t *, uint32_t);
void y2vu2rgb888line(uint8_t *, uint8_t *, uint8_t *, uint32_t);
void y2vu2rgbaline  (uint8_t *, uint8_t *, uint8_t *, uint32_t);

/**************************************************************************
** Color converting functions for 90/180/270 rotation cases.
** No equivalent VaNum Assembly code.
** These functions are currently at the rear part of this file.
*************************************************************************/
void y1vu2rgb565line_rot(uint8_t *, uint8_t *, uint8_t *, uint32_t, int32_t);
void y1vu2rgb888line_rot(uint8_t *, uint8_t *, uint8_t *, uint32_t, int32_t);
void y1vu2rgbaline_rot  (uint8_t *, uint8_t *, uint8_t *, uint32_t, int32_t);
void y2vu2rgb565line_rot(uint8_t *, uint8_t *, uint8_t *, uint32_t, int32_t);
void y2vu2rgb888line_rot(uint8_t *, uint8_t *, uint8_t *, uint32_t, int32_t);
void y2vu2rgbaline_rot  (uint8_t *, uint8_t *, uint8_t *, uint32_t, int32_t);

#endif /* _JPEG_POSTPROCESSOR_H */
