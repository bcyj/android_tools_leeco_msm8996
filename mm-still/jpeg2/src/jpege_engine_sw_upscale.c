/*========================================================================


*//** @file jpege_engine_sw_upscale.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2009 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
08/09/10   sunidw  Update luma & chroma ptr based on start offset
04/21/10   staceyw Fixed the bottom line extension during fetch luma/chroma
                   from input buffer to lines buffer.
10/14/09   staceyw Added in-line cropping and upscaling support, using 4-tap
                   32 polyphase FIR Filter.
09/01/09   staceyw Created file

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include <stdlib.h>
#include "jpege_engine_sw.h"
#include "jpeglog.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
// 4-tap 32 polyphase coefficents
// number of total coefficents = 32*4
// POLYPHASE_FILTER_COEFFS = 32*4
static const int16_t m_polyphaseCoeff4taps[POLYPHASE_FILTER_COEFFS]={
0, 512, 0, 0,
-7, 511, 8, 0,
-14, 507, 19, 0,
-19, 501, 32, -2,
-24, 493, 46, -3,
-28, 483, 62, -5,
-31, 472, 78, -7,
-34, 459, 96, -9,
-36, 444, 116, -12,
-37, 428, 135, -14,
-37, 410, 156, -17,
-37, 391, 177, -19,
-37, 372, 199, -22,
-36, 352, 221, -25,
-35, 331, 243, -27,
-33, 309, 265, -29,
-32, 288, 288, -32,
-29, 265, 309, -33,
-27, 243, 331, -35,
-25, 221, 352, -36,
-22, 199, 372, -37,
-19, 177, 391, -37,
-17, 156, 410, -37,
-14, 135, 428, -37,
-12, 116, 444, -36,
-9, 96, 459, -34,
-7, 78, 472, -31,
-5, 62, 483, -28,
-3, 46, 493, -24,
-2, 32, 501, -19,
0, 19, 507, -14,
0, 8, 511, -7};

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
/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/* functions related with in-line up-scale */
int jpege_engine_sw_upscale_configure(jpege_engine_sw_t  *p_engine,
                                      jpege_img_cfg_t    *p_config,
                                      jpege_img_data_t   *p_source,
                                      jpege_engine_dst_t *p_dest);
static int jpege_engine_sw_upscale_crop_setup(jpege_engine_sw_t  *p_engine,
                                              jpege_img_cfg_t    *p_config,
                                              jpege_img_data_t   *p_source);
static void jpege_engine_sw_upscale_configure_tables(jpege_engine_sw_t  *p_engine,
                                                     jpege_img_cfg_t    *p_config);
static int jpege_engine_sw_upscale_configure_filter(jpege_engine_sw_t  *p_engine);
static void jpege_engine_sw_fetch_dct_scaled_mcu (jpege_engine_sw_t *p_engine,
                                                  int16_t * currMCULumaDctOutput,
                                                  int16_t * currMCUChromaDctOutput);
static void jpege_engine_sw_fetch_dct_scaled_block_partial_luma(jpege_up_scale_t *p_upscale,
                                                                const uint8_t *p_luma_block_origin,
                                                                uint32_t       hori_bound,
                                                                uint32_t       vert_bound,
                                                                int16_t       *p_dct_output);
static void jpege_engine_sw_fetch_dct_scaled_block_partial_chroma(jpege_engine_sw_t *p_engine,
                                                                  const uint8_t     *p_chroma_block_origin,
                                                                  uint32_t           hori_bound,
                                                                  uint32_t           vert_bound,
                                                                  int16_t           *p_dct_output);
static void jpege_engine_sw_scale_mcu_lines(jpege_engine_sw_t *p_engine);
static void jpege_engine_sw_copy_rotated_luma_lines(uint8_t *srcPtr,
                                                    uint8_t *outputPtr,
                                                    uint32_t inputWidth,
                                                    int32_t  fetchIncrement,
                                                    uint32_t pixelExtendRight);
static void jpege_engine_sw_copy_rotated_chroma_lines(uint8_t *srcPtr,
                                                      uint8_t *outputPtr,
                                                      uint32_t inputWidth,
                                                      int32_t  fetchIncrement,
                                                      uint32_t pixelExtendRight);
static void jpege_engine_sw_fetch_luma_to_lines_buffer(jpege_engine_sw_t *p_engine,
                                                       uint32_t lineIndex);
static void jpege_engine_sw_fetch_chroma_to_lines_buffer(jpege_engine_sw_t *p_engine,
                                                         uint32_t lineIndex);
static void jpege_engine_sw_vertical_scale_luma_line(uint8_t *pInputLine,
                                                     uint8_t *pOutputLine,
                                                     jpege_up_scale_t *pScaleInfo);
static void jpege_engine_sw_horizontal_scale_luma_line(uint8_t *pInputLine,
                                                       uint8_t *pOutputLine,
                                                       jpege_up_scale_t *pScaleInfo);
static void jpege_engine_sw_vertical_scale_chroma_line(uint8_t *pInputLine,
                                                       uint8_t *pOutputLine,
                                                       jpege_up_scale_t *pScaleInfo);
static void jpege_engine_sw_horizontal_scale_chroma_line(uint8_t *pInputLine,
                                                         uint8_t *pOutputLine,
                                                         jpege_up_scale_t *pScaleInfo);
/* Extern functions from other files within the same engine */
extern void jpege_engine_sw_fetch_dct_block_luma(const uint8_t *, int16_t *, uint32_t);
extern void jpege_engine_sw_fetch_dct_block_chroma(const uint8_t  *,
                                                   int16_t *,
                                                   uint32_t,
                                                   input_format_t);
extern void jpege_engine_sw_fdct_block (const uint8_t *pixelDomainBlock,
                                        DCTELEM       *frequencyDomainBlock);
// jpeg fetch mcu funtion ptr
extern void (* jpege_fetch_dct_mcu)(jpege_engine_sw_t *p_engine,
                                    int16_t * currMCULumaDctOutput,
                                    int16_t * currMCUChromaDctOutput);

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
#define CLAMP(x) ( ((x) & 0xFFFFFF00) ? ((~(x) >> 31) & 0xFF) : (x))

#define MOD4(a) ((a) & 0x3)

/* =======================================================================
**                          Function Definitions
** ======================================================================= */

/*===========================================================================

FUNCTION        jpege_engine_sw_up_scale_configure

DESCRIPTION     JpegEncode Configure Function for crop and upscale

DEPENDENCIES    Called for crop and upscale cases

RETURN VALUE    INT

SIDE EFFECTS    Set fetch mcu function for crop and upscale cases
===========================================================================*/
int jpege_engine_sw_upscale_configure(jpege_engine_sw_t  *p_engine,
                                      jpege_img_cfg_t    *p_config,
                                      jpege_img_data_t   *p_source,
                                      jpege_engine_dst_t *p_dest)
{
    int       rc;

    /*-----------------------------------------------------------------------
      1. Initialize upscale information
    -----------------------------------------------------------------------*/
    //init scale information
    p_engine->jpege_up_scale.fetchedLumaBlocks = 0;
    p_engine->jpege_up_scale.verticalRowIndex = 0;
    p_engine->jpege_up_scale.verticalChromaRowIndex = 0;
    p_engine->jpege_up_scale.verticalLastIndex = 0;
    p_engine->jpege_up_scale.verticalChromaLastIndex = 0;
    p_engine->jpege_up_scale.cropFlag = false;
    p_engine->jpege_up_scale.upScaleFlag = false;
    p_engine->jpege_up_scale.verticalScaleFlag = false;
    p_engine->jpege_up_scale.horizontalScaleFlag = false;

    /* in-line crop and upscale set up */
    rc = jpege_engine_sw_upscale_crop_setup(p_engine,
                                            p_config,
                                            p_source);
    if (JPEG_FAILED(rc))
    {
        return rc;
    }

    /* in-line crop and upscale configue huffman and quant tables*/
    jpege_engine_sw_upscale_configure_tables(p_engine,
                                             p_config);

    /* Configue 4-tap 32 polyphase Filter for in-line crop and upscale */
    rc = jpege_engine_sw_upscale_configure_filter(p_engine);
    if (JPEG_FAILED(rc))
    {
        return rc;
    }

    // fetchdct mcu function for crop and scale cases
    jpege_fetch_dct_mcu = &jpege_engine_sw_fetch_dct_scaled_mcu;
    return JPEGERR_SUCCESS;
}

/*===========================================================================

FUNCTION        jpege_engine_sw_upscale_crop_setup

DESCRIPTION     in-line upscalar set up for crop and upscale

DEPENDENCIES    None

RETURN VALUE    INT

SIDE EFFECTS    Set fetch mcu function for crop and upscale cases
===========================================================================*/
static int jpege_engine_sw_upscale_crop_setup(jpege_engine_sw_t  *p_engine,
                                              jpege_img_cfg_t    *p_config,
                                              jpege_img_data_t   *p_source)
{
    uint32_t  nWidth, nHeight, nChromaWidth, nChromaHeight;
    uint32_t  nCropWidth, nCropHeight, nChromaCropWidth, nChromaCropHeight;
    uint32_t  nRotatedCropWidth, nRotatedCropChromaWidth;
    uint32_t  nOutputChromaWidth, nOutputChromaHeight, nBufferRatio;
    uint32_t  nCropTop, nCropBottom, nCropLeft, nCropRight;
    uint32_t  nChromaCropTop, nChromaCropBottom, nChromaCropLeft, nChromaCropRight;
    int32_t   nRotatedRatio, nBufferSize;
    int       rc;

    //set image sizes
    nWidth = p_source->width;
    nHeight = p_source->height;

    //set corp sizes
    nCropWidth = p_config->upsampler_cfg.input_width;
    nCropHeight = p_config->upsampler_cfg.input_height;
    nCropLeft = p_config->upsampler_cfg.h_offset;
    nCropRight = p_config->upsampler_cfg.h_offset
                 + p_config->upsampler_cfg.input_width - 1;
    nCropTop = p_config->upsampler_cfg.v_offset;
    nCropBottom = p_config->upsampler_cfg.v_offset
                  + p_config->upsampler_cfg.input_height - 1;

    /*-----------------------------------------------------------------------
      Initialize crop and scale flag
    -----------------------------------------------------------------------*/
    if ((p_config->upsampler_cfg.h_offset != 0 ||
         p_config->upsampler_cfg.v_offset != 0 ||
        ((p_config->upsampler_cfg.h_offset + p_config->upsampler_cfg.input_width) < p_source->width) ||
        ((p_config->upsampler_cfg.v_offset + p_config->upsampler_cfg.input_height) < p_source->height)) &&
       (nCropWidth != p_source->width ||
        nCropHeight != p_source->height))
    {
        p_engine->jpege_up_scale.cropFlag = true;
    }

    if  ((p_config->upsampler_cfg.input_width < p_config->upsampler_cfg.output_width) ||
         (p_config->upsampler_cfg.input_height < p_config->upsampler_cfg.output_height))
    {
        p_engine->jpege_up_scale.upScaleFlag = true;
    }

    // Set Vertical and Horizontal Scale Flag
    if (nCropHeight < p_config->upsampler_cfg.output_height)
    {
         p_engine->jpege_up_scale.verticalScaleFlag = true;
    }
    if (nCropWidth < p_config->upsampler_cfg.output_width)
    {
         p_engine->jpege_up_scale.horizontalScaleFlag = true;
    }

    /*-----------------------------------------------------------------------
      Calculate Chroma Width and Height
      and crop infomation for chroma
      and prepare to calculate the scale buffer size
    -----------------------------------------------------------------------*/
    switch (p_engine->subsampling)
    {
        case JPEG_H1V1:
        {
            nChromaWidth = nWidth;
            nChromaHeight = nHeight;
            nChromaCropWidth = nCropWidth;
            nChromaCropHeight = nCropHeight;
            nOutputChromaWidth = p_config->upsampler_cfg.output_width;
            nOutputChromaHeight = p_config->upsampler_cfg.output_height;

            nChromaCropLeft = nCropLeft;
            nChromaCropRight = nCropRight;
            nChromaCropTop = nCropTop;
            nChromaCropBottom = nCropBottom;
            nBufferRatio = 3;
            nRotatedRatio = 1;
            break;
        }
        case JPEG_H1V2:
        {
            nChromaWidth = nWidth;
            nChromaHeight = nHeight >> 1;
            nChromaCropWidth = nCropWidth;
            nChromaCropHeight = nCropHeight >> 1;
            nOutputChromaWidth = p_config->upsampler_cfg.output_width;
            nOutputChromaHeight = (p_config->upsampler_cfg.output_height + 1) >> 1;

            nChromaCropLeft = nCropLeft;
            nChromaCropRight = nCropRight;
            nChromaCropTop = nCropTop >> 1; // one half
            nChromaCropBottom = nCropBottom >> 1; // one half
            nBufferRatio = 2;
            nRotatedRatio = 1;
            break;
        }
        case JPEG_H2V1:
        {
            nChromaWidth = nWidth >> 1;
            nChromaHeight = nHeight;
            nChromaCropWidth = nCropWidth >> 1;
            nChromaCropHeight = nCropHeight;

            nOutputChromaWidth = (p_config->upsampler_cfg.output_width + 1) >> 1;
            nOutputChromaHeight = p_config->upsampler_cfg.output_height;
            nChromaCropLeft = nCropLeft >> 1;  // one half
            nChromaCropRight = nCropRight >> 1; // one half
            nChromaCropTop = nCropTop;
            nChromaCropBottom = nCropBottom;
            nBufferRatio = 2;
            nRotatedRatio = 2;
            break;
        }
        case JPEG_H2V2:
        {
            nChromaWidth = nWidth >> 1;
            nChromaHeight = nHeight >> 1;
            nChromaCropWidth = nCropWidth >> 1;
            nChromaCropHeight = nCropHeight >> 1;
            nOutputChromaWidth = (p_config->upsampler_cfg.output_width + 1) >> 1;
            nOutputChromaHeight = (p_config->upsampler_cfg.output_height + 1) >> 1;

            nChromaCropLeft = nCropLeft >> 1;  // one half
            nChromaCropRight = nCropRight >> 1; // one half
            nChromaCropTop = nCropTop >> 1;
            nChromaCropBottom = nCropBottom >> 1;
            nBufferRatio = 2;
            nRotatedRatio = 2;
            break;
        }
    default:
        JPEG_DBG_HIGH("jpege_engine_sw_up_scale_configure: invalid jpeg subsampling: %d\n", p_engine->subsampling);
        return JPEGERR_EBADPARM;
    }

    switch (p_engine->rotation)
    {
        case 0:
        {
            // setup relative step
            p_engine->horiIncrement_luma = 1;
            p_engine->vertIncrement_luma = p_engine->lumaWidth;
            p_engine->horiIncrement_chroma = 2;
            p_engine->vertIncrement_chroma = 2 * p_engine->chromaWidth;

            // crop width, output width and input width
            nRotatedCropWidth = nCropWidth;
            p_engine->jpege_up_scale.cropHeight = nCropHeight;
            p_engine->jpege_up_scale.outputWidth = p_config->upsampler_cfg.output_width;
            p_engine->jpege_up_scale.outputHeight = p_config->upsampler_cfg.output_height;
            // chroma crop width, output width
            nRotatedCropChromaWidth = nChromaCropWidth;
            p_engine->jpege_up_scale.cropChromaHeight = nChromaCropHeight;
            p_engine->jpege_up_scale.outputChromaWidth =  nOutputChromaWidth;
            p_engine->jpege_up_scale.outputChromaHeight = nOutputChromaHeight;

            // rotated fetch increment from input to line buffer
            p_engine->jpege_up_scale.rotatedLumaFetchIncrement = p_source->width;
            p_engine->jpege_up_scale.rotatedLumaSrcIncrement = p_source->width;
            p_engine->jpege_up_scale.rotatedChromaFetchIncrement
                = p_engine->jpege_up_scale.rotatedLumaFetchIncrement / nRotatedRatio;
            p_engine->jpege_up_scale.rotatedChromaSrcIncrement
                = p_engine->jpege_up_scale.rotatedLumaSrcIncrement / nRotatedRatio;
            break;
        }
        case 90:
        {
            // setup relative step
            p_engine->horiIncrement_luma = 1;
            p_engine->vertIncrement_luma = p_engine->lumaHeight;
            p_engine->horiIncrement_chroma = 2;
            p_engine->vertIncrement_chroma = 2 * p_engine->chromaHeight;

            // crop width, output width and input width
            nRotatedCropWidth = nCropHeight;
            p_engine->jpege_up_scale.cropHeight = nCropWidth;
            p_engine->jpege_up_scale.outputWidth = p_config->upsampler_cfg.output_height;
            p_engine->jpege_up_scale.outputHeight = p_config->upsampler_cfg.output_width;
            // chroma crop width, output width
            nRotatedCropChromaWidth = nChromaCropHeight;
            p_engine->jpege_up_scale.cropChromaHeight = nChromaCropWidth;
            p_engine->jpege_up_scale.outputChromaWidth = nOutputChromaHeight;
            p_engine->jpege_up_scale.outputChromaHeight = nOutputChromaWidth;

            // rotated fetch increment from input to line buffer
            p_engine->jpege_up_scale.rotatedLumaFetchIncrement
                = (int32_t)(0 - p_source->width);
            p_engine->jpege_up_scale.rotatedLumaSrcIncrement = 1;
            p_engine->jpege_up_scale.rotatedChromaFetchIncrement
                = (int32_t)p_engine->jpege_up_scale.rotatedLumaFetchIncrement / nRotatedRatio;
            p_engine->jpege_up_scale.rotatedChromaSrcIncrement = 1;
            break;
        }
        case 180:
        {
            // setup relative step
            p_engine->horiIncrement_luma = 1;
            p_engine->vertIncrement_luma = p_engine->lumaWidth;
            p_engine->horiIncrement_chroma = 2;
            p_engine->vertIncrement_chroma = 2 * p_engine->chromaWidth;

            // crop width, output width and input width
            nRotatedCropWidth = nCropWidth;
            p_engine->jpege_up_scale.cropHeight = nCropHeight;
            p_engine->jpege_up_scale.outputWidth = p_config->upsampler_cfg.output_width;
            p_engine->jpege_up_scale.outputHeight = p_config->upsampler_cfg.output_height;
            // chroma crop width, output width
            nRotatedCropChromaWidth = nChromaCropWidth;
            p_engine->jpege_up_scale.cropChromaHeight = nChromaCropHeight;
            p_engine->jpege_up_scale.outputChromaWidth = nOutputChromaWidth ;
            p_engine->jpege_up_scale.outputChromaHeight = nOutputChromaHeight ;

            // rotated fetch increment from input to line buffer
            p_engine->jpege_up_scale.rotatedLumaFetchIncrement = -1;
            p_engine->jpege_up_scale.rotatedLumaSrcIncrement
                = (int32_t) (0 - p_source->width);
            p_engine->jpege_up_scale.rotatedChromaFetchIncrement = -1;
            p_engine->jpege_up_scale.rotatedChromaSrcIncrement
                = (int32_t)p_engine->jpege_up_scale.rotatedLumaSrcIncrement / nRotatedRatio;
            break;
        }
        case 270:
        {
            // setup relative step
            p_engine->horiIncrement_luma = 1;
            p_engine->vertIncrement_luma = p_engine->lumaHeight;
            p_engine->horiIncrement_chroma = 2;
            p_engine->vertIncrement_chroma = 2 * p_engine->chromaHeight;

            // crop width, output width and input width
            nRotatedCropWidth = nCropHeight;
            p_engine->jpege_up_scale.cropHeight = nCropWidth;
            p_engine->jpege_up_scale.outputWidth = p_config->upsampler_cfg.output_height;
            p_engine->jpege_up_scale.outputHeight = p_config->upsampler_cfg.output_width;
            // chroma crop width, output width
            nRotatedCropChromaWidth = nChromaCropHeight;
            p_engine->jpege_up_scale.cropChromaHeight = nChromaCropWidth;
            p_engine->jpege_up_scale.outputChromaWidth = nOutputChromaHeight;
            p_engine->jpege_up_scale.outputChromaHeight = nOutputChromaWidth;

            // rotated fetch increment from input to line buffer
            p_engine->jpege_up_scale.rotatedLumaFetchIncrement = p_source->width;
            p_engine->jpege_up_scale.rotatedLumaSrcIncrement = -1;
            p_engine->jpege_up_scale.rotatedChromaFetchIncrement
                = p_engine->jpege_up_scale.rotatedLumaFetchIncrement / nRotatedRatio;
            p_engine->jpege_up_scale.rotatedChromaSrcIncrement = -1;
            break;
        }
        default:
            JPEG_DBG_HIGH("jpege_engine_sw_up_scale_configure: unrecognized rotation: %d\n", p_engine->rotation);
            return JPEGERR_EBADPARM;
    }

    /*-----------------------------------------------------------------------
      Output Buffer for up-scaling cases
    -----------------------------------------------------------------------*/
    nBufferSize = p_engine->jpege_up_scale.outputWidth * p_engine->rotatedMCUHeight;
    nBufferSize *= nBufferRatio;
    // Free previously allocated inline output first
    if (p_engine->p_inline_scaled_buffer)
    {
        jpeg_buffer_destroy((jpeg_buffer_t *)&(p_engine->p_inline_scaled_buffer));
    }
    // Allocate inline upscale output buffer
    rc = jpeg_buffer_init((jpeg_buffer_t *)&(p_engine->p_inline_scaled_buffer));
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_MED("jpege_engine_sw_up_scale_configure: p_inline_scaled_buffer init failed\n");
        return rc;
    }
    rc = jpeg_buffer_allocate((jpeg_buffer_t)p_engine->p_inline_scaled_buffer, nBufferSize * sizeof(uint8_t), false);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_MED("jpege_engine_sw_up_scale_configure: p_inline_scaled_buffer %u bytes allocation failed\n",
                     nBufferSize);
        jpeg_buffer_destroy((jpeg_buffer_t *)&(p_engine->p_inline_scaled_buffer));
        return rc;
    }

    p_engine->jpege_up_scale.verticalLumaFetchWidth = nRotatedCropWidth;
    p_engine->jpege_up_scale.verticalChromaFetchWidth = nRotatedCropChromaWidth;

    // Initialize input Y and CbCr/CrCb pointers
    p_engine->jpege_up_scale.lumaInputPtr = (uint8_t *)((p_source->p_fragments[0].color.yuv.luma_buf)->ptr)+
                             ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.luma_buf)->start_offset;
    p_engine->jpege_up_scale.chromaInputPtr = (uint8_t *)((p_source->p_fragments[0].color.yuv.chroma_buf)->ptr) +
                             ((jpeg_buf_t *)p_source->p_fragments[0].color.yuv.chroma_buf)->start_offset;

    // Set up input pointer to luma and chroma
    if (p_engine->jpege_up_scale.cropFlag == true)
    {
        // crop cases
        switch (p_engine->rotation)
        {
            case 0:
            {
                p_engine->jpege_up_scale.lumaInputPtr += p_source->width * nCropTop + nCropLeft;
                p_engine->jpege_up_scale.chromaInputPtr += 2 * nChromaWidth * nChromaCropTop
                                                           + 2 * nChromaCropLeft;
                break;
            }
            case 90:
            {
                p_engine->jpege_up_scale.lumaInputPtr += p_source->width * nCropBottom + nCropLeft;
                p_engine->jpege_up_scale.chromaInputPtr += 2 * nChromaWidth * nChromaCropBottom
                                                           + 2 * nChromaCropLeft;
                break;
            }
            case 180:
            {
                p_engine->jpege_up_scale.lumaInputPtr += p_source->width * nCropBottom + nCropRight;
                p_engine->jpege_up_scale.chromaInputPtr += 2 * nChromaWidth * nChromaCropBottom
                                                           + 2 * nChromaCropRight;
                break;
            }
            case 270:
            {
                p_engine->jpege_up_scale.lumaInputPtr += p_source->width * nCropTop + nCropRight;
                p_engine->jpege_up_scale.chromaInputPtr += 2 * nChromaWidth * nChromaCropTop
                                                           + 2 * nChromaCropRight;
                break;
            }
            default:
                JPEG_DBG_HIGH("jpege_engine_sw_up_scale_configure: unrecognized rotation: %d\n", p_engine->rotation);
                return JPEGERR_EBADPARM;
        }
    }
    else
    {
        // no crop cases
        switch ( p_engine->rotation)
        {
            case 0:
                break;
            case 90:
            {
                p_engine->jpege_up_scale.lumaInputPtr += p_source->width * (p_source->height - 1);
                p_engine->jpege_up_scale.chromaInputPtr += nChromaWidth * 2 * (nChromaHeight - 1);
                break;
            }
            case 180:
            {
                p_engine->jpege_up_scale.lumaInputPtr += p_source->width * p_source->height - 1;
                p_engine->jpege_up_scale.chromaInputPtr += (nChromaWidth * nChromaHeight - 1) * 2;
                break;
            }
            case 270:
            {
                p_engine->jpege_up_scale.lumaInputPtr += p_source->width - 1;
                p_engine->jpege_up_scale.chromaInputPtr += (nChromaWidth - 1) * 2;
                break;
            }
            default:
                JPEG_DBG_HIGH("jpege_engine_sw_up_scale_configure: unrecognized rotation: %d\n", p_engine->rotation);
                return JPEGERR_EBADPARM;
        }
    }
    return JPEGERR_SUCCESS;
}

/*===========================================================================
FUNCTION        jpege_engine_sw_upscale_configure_tables

DESCRIPTION     upscalar configure quant and zigzag tables

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_upscale_configure_tables(jpege_engine_sw_t  *p_engine,
                                                     jpege_img_cfg_t    *p_config)
{
    uint16_t  *tempLumaQTablePtr, *tempChromaQTablePtr;
    uint16_t  qFactorInvQ15;
    const int16_t *zigzagTablePtr;
    uint16_t  i, j;

    /*-----------------------------------------------------------------------
      Copy Quant Tables
    -----------------------------------------------------------------------*/
    /* Initialize Luma and Chroma Quantization tables */
    tempLumaQTablePtr = (uint16_t *)p_config->luma_quant_tbl;
    tempChromaQTablePtr = (uint16_t *)p_config->chroma_quant_tbl;
    for (i = 0; i < 8; i++)
    {
        for (j = 0; j < 8; j++)
        {
            qFactorInvQ15 = (uint16_t)(1.0/tempLumaQTablePtr[i*8+j] * ((1<<(FIX_POINT_FACTOR-3))-1) + 0.5);
            p_engine->lumaQuantTableQ16[i*8+j] = (int16_t)qFactorInvQ15;

            qFactorInvQ15 = (uint16_t)(1.0/tempChromaQTablePtr[i*8+j] * ((1<<(FIX_POINT_FACTOR-3))-1) + 0.5);
            p_engine->chromaQuantTableQ16[i*8+j] = (int16_t)qFactorInvQ15;
        }
    }
    // zigzag table
    zigzagTablePtr = zigzagTable;

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
}

/*===========================================================================
FUNCTION        jpege_engine_sw_upscale_configure_filter

DESCRIPTION     upscalar configure 4-tap 32 polyphase filter

DEPENDENCIES    None

RETURN VALUE    INT

SIDE EFFECTS    None
===========================================================================*/
static int jpege_engine_sw_upscale_configure_filter(jpege_engine_sw_t  *p_engine)
{
    int32_t   nBufferSize;
    int       rc;

    /*------------------------------------------------------------
     32 polyphase 4-tap Filter:
     1. set default POLYPHASE_ACCUM_Q_BITS = 17 bits
        for quantization
     2. polyphase is represented at the highest 5 bits
     3. initial phase = 0
     4. phase step = (cropWidth<<POLYPHASE_ACCUM_Q_BITS)/outputWidth
     5. calculate number of pixels to be extended
     6. calculate luma and chroma extended width
    ------------------------------------------------------------*/
    /*------------------------------------------------------------
      Vertical Polyphase init
    ------------------------------------------------------------*/
    // vertical init
    p_engine->jpege_up_scale.verticalInit = 0;

    // vertical phase step
    p_engine->jpege_up_scale.verticalStep
         = ((uint32_t)p_engine->jpege_up_scale.cropHeight << POLYPHASE_ACCUM_Q_BITS)
                     / p_engine->jpege_up_scale.outputHeight;

    // Compute number of pixels to be extended
    p_engine->jpege_up_scale.pixelExtendTop = 1;
    p_engine->jpege_up_scale.pixelExtendBottom
         = ((p_engine->jpege_up_scale.verticalStep * (p_engine->jpege_up_scale.outputHeight - 1))
             >> POLYPHASE_ACCUM_Q_BITS) + 3 - p_engine->jpege_up_scale.cropHeight ;

    // Init vertical accumulator
    p_engine->jpege_up_scale.verticalLumaAccumulator = 0;
    p_engine->jpege_up_scale.verticalChromaAccumulator = 0;

    /*------------------------------------------------------------
      Horizontal Polyphase init
    ------------------------------------------------------------*/
    // vertical init
    p_engine->jpege_up_scale.horizontalInit = 0;

    // vertical phase step
    // p_engine->jpege_up_scale.verticalLumaFetchWidth = nRotatedCropWidth
    p_engine->jpege_up_scale.horizontalStep
         = ((uint32_t)p_engine->jpege_up_scale.verticalLumaFetchWidth << POLYPHASE_ACCUM_Q_BITS)
                     / p_engine->jpege_up_scale.outputWidth;

    // Compute number of pixels to be extended
    p_engine->jpege_up_scale.pixelExtendLeft = 1;
    p_engine->jpege_up_scale.pixelExtendRight
         = ((p_engine->jpege_up_scale.horizontalStep * (p_engine->jpege_up_scale.outputWidth - 1))
             >> POLYPHASE_ACCUM_Q_BITS ) + 3 - p_engine->jpege_up_scale.verticalLumaFetchWidth ;

    // Compute extended width
    p_engine->jpege_up_scale.cropExtendedWidth
         = (p_engine->jpege_up_scale.verticalLumaFetchWidth + p_engine->jpege_up_scale.pixelExtendLeft
                              + p_engine->jpege_up_scale.pixelExtendRight);
    // p_engine->jpege_up_scale.verticalChromaFetchWidth = nRotatedCropChromaWidth
    p_engine->jpege_up_scale.cropChromaExtendedWidth
         = (p_engine->jpege_up_scale.verticalChromaFetchWidth + p_engine->jpege_up_scale.pixelExtendLeft
                                    + p_engine->jpege_up_scale.pixelExtendRight);

    // Allocate memory for the luma Lines
    // POLYPHASE_FILTER_TAP = 4 is the number of luma lines buffer
    // Free previously allocated luma line buffer first
    if (p_engine->p_input_luma_line_buffer)
    {
        jpeg_buffer_destroy((jpeg_buffer_t *)&(p_engine->p_input_luma_line_buffer));
    }
    // Allocate luma line buffer
    rc = jpeg_buffer_init((jpeg_buffer_t *)&(p_engine->p_input_luma_line_buffer));
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_MED("jpege_engine_sw_upscale_configure: p_input_luma_line_buffer init failed\n");
        return rc;
    }
    nBufferSize = POLYPHASE_FILTER_TAP
                  * p_engine->jpege_up_scale.cropExtendedWidth * sizeof(uint8_t);
    rc = jpeg_buffer_allocate((jpeg_buffer_t)p_engine->p_input_luma_line_buffer, nBufferSize, false);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_MED("jpege_engine_sw_upscale_configure: p_input_luma_line_buffer %u bytes allocation failed\n",
                     nBufferSize);
        jpeg_buffer_destroy((jpeg_buffer_t *)&(p_engine->p_input_luma_line_buffer));
        return rc;
    }

    // Allocate memory for the chroma Lines
    // POLYPHASE_FILTER_TAP = 4 is the number of chroma lines buffer
    // Free previously allocated chroma line buffer first
    if (p_engine->p_input_chroma_line_buffer)
    {
        jpeg_buffer_destroy((jpeg_buffer_t *)&(p_engine->p_input_chroma_line_buffer));
    }
    // Allocate chroma line buffer
    rc = jpeg_buffer_init((jpeg_buffer_t *)&(p_engine->p_input_chroma_line_buffer));
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_MED("jpege_engine_sw_upscale_configure: p_input_chroma_line_buffer init failed\n");
        return rc;
    }
    nBufferSize = POLYPHASE_FILTER_TAP
                  * p_engine->jpege_up_scale.cropChromaExtendedWidth * 2 * sizeof(uint8_t);
    rc = jpeg_buffer_allocate((jpeg_buffer_t)p_engine->p_input_chroma_line_buffer, nBufferSize, false);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_MED("jpege_engine_sw_upscale_configure: p_input_chroma_line_buffer %u bytes allocation failed\n",
                     nBufferSize);
        jpeg_buffer_destroy((jpeg_buffer_t *)&(p_engine->p_input_chroma_line_buffer));
        return rc;
    }

    // Allocate memory for internal Vertical Scale Buffer
    // Free previously allocated vertical scaled buffer first
    if (p_engine->p_vertical_scaled_buffer)
    {
        jpeg_buffer_destroy((jpeg_buffer_t *)&(p_engine->p_vertical_scaled_buffer));
    }
    // Allocate vertical scaled buffer
    rc = jpeg_buffer_init((jpeg_buffer_t *)&(p_engine->p_vertical_scaled_buffer));
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_MED("jpege_engine_sw_upscale_configure: p_vertical_scaled_buffer init failed\n");
        return rc;
    }
    nBufferSize = p_engine->jpege_up_scale.cropChromaExtendedWidth * 2 * sizeof(uint8_t);
    rc = jpeg_buffer_allocate((jpeg_buffer_t)p_engine->p_vertical_scaled_buffer, nBufferSize, false);
    if (JPEG_FAILED(rc))
    {
        JPEG_DBG_MED("jpege_engine_sw_upscale_configure: p_vertical_scaled_buffer %u bytes allocation failed\n",
                     nBufferSize);
        jpeg_buffer_destroy((jpeg_buffer_t *)&(p_engine->p_vertical_scaled_buffer));
        return rc;
    }
    return JPEGERR_SUCCESS;
}

/*===========================================================================
FUNCTION        jpege_engine_sw_fetch_scaled_mcu

DESCRIPTION     Fetch Scale MCU from the input buffers
                Do DCT

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_fetch_dct_scaled_mcu (jpege_engine_sw_t *p_engine,
                                                  int16_t * currMCULumaDctOutput,
                                                  int16_t * currMCUChromaDctOutput)
{
    uint8_t * currMCUYStartAddr, * currMCUCbCrStartAddr;
    uint8_t * currBlockStartAddr, *currPtr;
    uint32_t horiPixelIndex, vertPixelIndex;
    uint32_t vertMCUOffset, horiMCUOffset, vertBound, horiBound;
    int32_t vertCross, horiCross;
    uint32_t i, j;
    int16_t *currDctOutputPtr;

    JPEG_DBG_LOW("jpege_engine_sw_fetch_dct_scaled_mcu: horiMCUIndex=%d, vertMCUIndex=%d\n",
                 p_engine->rotatedhoriMCUIndex, p_engine->rotatedvertMCUIndex);

    /* fetch luma data */
    currDctOutputPtr = currMCULumaDctOutput;

    /*------------------------------------------------------------
      If the luma blocks in the line buffer is empty
      need to in-line Upscale cropped Luma and Chroma
      and output p_engine->rotatedMCUHeight Lines
      Luma and Chroma
    ------------------------------------------------------------*/
    if (p_engine->jpege_up_scale.fetchedLumaBlocks <= 0)
    {
        jpege_engine_sw_scale_mcu_lines(p_engine);
    }

    // Calculate the modified origin for the luma data
    horiPixelIndex = p_engine->rotatedhoriMCUIndex * p_engine->rotatedMCUWidth;
    vertPixelIndex = p_engine->rotatedvertMCUIndex * p_engine->rotatedMCUHeight;

    /*------------------------------------------------------------
      Use the MCU x and y coordinates from above to calculate the address of
      the first pixel in the current MCU
      in-line fetch from p_engine->rotatedMCUHeight Lines Luma
    ------------------------------------------------------------*/
    currMCUYStartAddr = (unsigned char *)(p_engine->p_inline_scaled_buffer->ptr)
        + (int32_t)horiPixelIndex * p_engine->horiIncrement_luma
        + (int32_t)(vertPixelIndex % (p_engine->rotatedMCUHeight))
                                   * p_engine->vertIncrement_luma;
    JPEG_DBG_LOW("jpege_engine_sw_fetch_scaled_mcu: inputYPtr=0x%x, currMCUYStartAddr=0x%x\n",
                 (uint32_t)p_engine->inputYPtr, (uint32_t)currMCUYStartAddr);

    // Two layers of loops for the luma blocks within a MCU
    vertMCUOffset = 0;

    for (i = 0; i < p_engine->rotatedMCUHeight / BLOCK_HEIGHT; i++)
    {
        // Calculate how many pixels are needed to pad vertically
        vertCross = (int32_t)(p_engine->rotatedlumaHeight -
            (vertPixelIndex + vertMCUOffset + 1));
        if (vertCross >= BLOCK_HEIGHT) // No pad
        {
            vertBound = BLOCK_HEIGHT;
            currBlockStartAddr = currMCUYStartAddr +
                ((int32_t)vertMCUOffset * p_engine->vertIncrement_luma);
        }
        else if (vertCross >= 0) // Partially pad
        {
            vertBound = vertCross + 1;
            currBlockStartAddr = currMCUYStartAddr + (int32_t)vertMCUOffset *
                                 p_engine->vertIncrement_luma;
        }
        else // Fully pad
        {
            vertBound = 1;
            currBlockStartAddr = currMCUYStartAddr +
                                 ((int32_t)(vertMCUOffset - p_engine->rotatedMCUHeight) +
                                 (BLOCK_HEIGHT + vertCross)) * p_engine->vertIncrement_luma;
            currBlockStartAddr = currBlockStartAddr < (unsigned char *)p_engine->p_inline_scaled_buffer->ptr
                                 ?  (unsigned char *)p_engine->p_inline_scaled_buffer->ptr : currBlockStartAddr;
        }
        horiMCUOffset = 0;
        for (j=0; j < p_engine->rotatedMCUWidth/BLOCK_WIDTH; j++)
        {
            // Calculate how many pixels are needed to pad horizontally
            horiCross = p_engine->rotatedlumaWidth - (horiPixelIndex + horiMCUOffset + 1);
            if (horiCross >= BLOCK_WIDTH) // No pad
            {
                horiBound = BLOCK_WIDTH;
                currBlockStartAddr += (int32_t)horiMCUOffset * p_engine->horiIncrement_luma;
            }
            else if (horiCross >= 0) // Partially pad
            {
                horiBound = horiCross + 1;
                currBlockStartAddr += (int32_t)horiMCUOffset * p_engine->horiIncrement_luma;
            }
            else // Fully pad
            {
                horiBound = 1;
                currBlockStartAddr += ((int32_t)(horiMCUOffset - p_engine->rotatedMCUWidth) +
                                        (BLOCK_WIDTH + horiCross)) * p_engine->horiIncrement_luma;
            }
            JPEG_DBG_LOW("jpege_engine_sw_fetch_scaled_mcu: currYBlockStartAddr=0x%x\n",
                         (uint32_t)currBlockStartAddr);

            // fetch block from scaled luma
            if ((vertBound == BLOCK_HEIGHT) && (horiBound == BLOCK_WIDTH))
            {
                jpege_engine_sw_fetch_dct_block_luma (currBlockStartAddr,
                                                      currDctOutputPtr,
                                                      p_engine->jpege_up_scale.outputWidth);
            }
            else
            {
                jpege_engine_sw_fetch_dct_scaled_block_partial_luma (&p_engine->jpege_up_scale,
                                                                     currBlockStartAddr,
                                                                     horiBound, vertBound, currDctOutputPtr);
            }
            // Fetching one Luma block
            p_engine->jpege_up_scale.fetchedLumaBlocks -= 1;

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
    /*------------------------------------------------------------
      Use the MCU x and y coordinates from above to calculate the address of
      the first pixel in the current MCU
      in-line fetch from BLOCK_HEIGHT Lines chroma
    ------------------------------------------------------------*/
    currMCUCbCrStartAddr = (unsigned char *)p_engine->p_inline_scaled_buffer->ptr
        + p_engine->jpege_up_scale.outputWidth * p_engine->rotatedMCUHeight
        + (int32_t)horiPixelIndex * p_engine->horiIncrement_chroma
        + (int32_t)(vertPixelIndex % (BLOCK_HEIGHT)) * p_engine->vertIncrement_chroma;
    JPEG_DBG_LOW("jpege_engine_sw_fetch_scaled_mcu: inputCbCrPtr=0x%x, currMCUCbCrStartAddr=0x%x\n",
                 (uint32_t)p_engine->inputCbCrPtr, (uint32_t)currMCUCbCrStartAddr);

    // Calculate how many pixels are needed to pad vertically
    vertCross = p_engine->rotatedchromaHeight - (vertPixelIndex + 1);
    if (vertCross >= BLOCK_HEIGHT)
    {
        vertBound = BLOCK_HEIGHT; // No pad
    }
    else
    {
        vertBound = vertCross + 1; // Partially Pad
    }
    // Calculate how many pixels are needed to pad horizontally
    horiCross = p_engine->rotatedchromaWidth - (horiPixelIndex + 1);
    if (horiCross >= BLOCK_WIDTH)
    {
        horiBound = BLOCK_WIDTH; // No pad
    }
    else
    {
        horiBound = horiCross + 1; // Partially pad
    }

    // Dual Format Selection
    // fetch block from scaled chroma
    if ((vertBound == BLOCK_HEIGHT) && (horiBound == BLOCK_WIDTH))
    {
        jpege_engine_sw_fetch_dct_block_chroma(currMCUCbCrStartAddr,
                                               currDctOutputPtr,
                                               p_engine->jpege_up_scale.outputChromaWidth,
                                               p_engine->InputFormat);
    }
    else
    {
        jpege_engine_sw_fetch_dct_scaled_block_partial_chroma (p_engine,
                                                               currMCUCbCrStartAddr,
                                                               horiBound, vertBound, currDctOutputPtr);
    }
}

/*===========================================================================
FUNCTION        jpege_engine_sw_fetch_dct_scaled_block_partial_luma

DESCRIPTION     Fetch partial Luma block from the scaled input
                buffers and Do Dct

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_fetch_dct_scaled_block_partial_luma(jpege_up_scale_t *p_upscale,
                                                                const uint8_t *p_luma_block_origin,
                                                                uint32_t       hori_bound,
                                                                uint32_t       vert_bound,
                                                                int16_t       *p_dct_output)
{
    const uint8_t *p_luma_block;
    const uint8_t *p_padding_line;
    uint32_t       luma_width = p_upscale->outputWidth;
    uint8_t        fetched_block[JBLOCK_SIZE];
    uint8_t       *p_fetched_block = fetched_block;
    uint32_t       temp_bound;
    uint32_t       i, j;

    p_luma_block = p_luma_block_origin;

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

/*===========================================================================
FUNCTION        jpege_engine_sw_fetch_dct_scaled_block_partial_chroma

DESCRIPTION     Fetch partial Chroma block from the scaled input
                buffers and Do Dct

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_fetch_dct_scaled_block_partial_chroma(jpege_engine_sw_t *p_engine,
                                                                  const uint8_t     *p_chroma_block_origin,
                                                                  uint32_t           hori_bound,
                                                                  uint32_t           vert_bound,
                                                                  int16_t           *p_dct_output)
{
    const uint8_t *p_chroma_block;
    const uint8_t *p_padding_line;
    uint32_t       chroma_width = p_engine->jpege_up_scale.outputChromaWidth;
    uint8_t        fetched_block[JBLOCK_SIZE * NUM_CHROMA_BLOCKS]; // chroma is interleaved
    uint8_t        deinterleaved_block[JBLOCK_SIZE * NUM_CHROMA_BLOCKS];
    uint8_t       *p_fetched_block = fetched_block;
    uint8_t       *p_deinterleaved_block1 = deinterleaved_block;
    uint8_t       *p_deinterleaved_block2 = p_deinterleaved_block1 + JBLOCK_SIZE;
    uint8_t       *p_cb_block, *p_cr_block;
    uint32_t       temp_bound;
    uint32_t       i, j;

    p_chroma_block = p_chroma_block_origin;

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

/*===========================================================================
FUNCTION        jpege_engine_sw_copy_rotated_luma_lines

DESCRIPTION     copy rotated luma lines

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_copy_rotated_luma_lines(uint8_t *srcPtr,
                                                    uint8_t *outputPtr,
                                                    uint32_t inputWidth,
                                                    int32_t  fetchIncrement,
                                                    uint32_t pixelExtendRight)
{
    uint32_t i, k;
    // Extend the first line
    // Extend the left
    *outputPtr = *srcPtr;
    // Fetch pixel by pixel
    for (i = 0; i < inputWidth; i++)
    {
        *(outputPtr + i + 1) = *(srcPtr + fetchIncrement * i);
    }
    // Extend the right
    for (k = 1; k <= pixelExtendRight; k++)
    {
        *(outputPtr + inputWidth + k)
            = *(srcPtr + fetchIncrement * (inputWidth - 1) );
    }
}

/*===========================================================================
FUNCTION        jpege_engine_sw_fetch_luma_to_lines_buffer

DESCRIPTION     Fetch Luma from the input buffer to lines buffer

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_fetch_luma_to_lines_buffer(jpege_engine_sw_t *p_engine,
                                                       uint32_t lineIndex)
{
    /*------------------------------------------------------------
     For rotation =0, it can fetch one line luma to buffer
    ------------------------------------------------------------*/
    uint32_t inputWidth = p_engine->jpege_up_scale.verticalLumaFetchWidth;
    uint32_t extendedWidth = p_engine->jpege_up_scale.cropExtendedWidth;
    int32_t fetchIncrement = p_engine->jpege_up_scale.rotatedLumaFetchIncrement;
    /*---------------------------------------------------------------
     For one output line, it needs line A-1, A, A+1, up to line A+2,
     And this is a 4 line buffer,
     plus there is 1 extended line at the top of the beginning
    ----------------------------------------------------------------*/
    uint32_t pixelIndex = MOD4(p_engine->jpege_up_scale.verticalPixelIndex + 3);
    uint32_t pixelExtendRight = p_engine->jpege_up_scale.pixelExtendRight;
    uint32_t fetchPixelIndex;
    uint8_t  *srcPtr, *outputPtr;
    uint32_t i, k;

    if (lineIndex == 0)
    {
        /*------------------------------------------------------------
         Fetch from the beginning
        ------------------------------------------------------------*/
        srcPtr = p_engine->jpege_up_scale.lumaInputPtr;
        outputPtr = p_engine->p_input_luma_line_buffer->ptr;
    }
    else if ((p_engine->jpege_up_scale.verticalPixelIndex + 2) >= p_engine->jpege_up_scale.cropHeight)
    {
        /*------------------------------------------------------------
         Fetch from the last row
        ------------------------------------------------------------*/
        fetchPixelIndex = p_engine->jpege_up_scale.cropHeight - 1;
        srcPtr = p_engine->jpege_up_scale.lumaInputPtr
                 + (int32_t)fetchPixelIndex * p_engine->jpege_up_scale.rotatedLumaSrcIncrement;
        outputPtr = p_engine->p_input_luma_line_buffer->ptr + pixelIndex * extendedWidth;
    }
    else
    {
        fetchPixelIndex = p_engine->jpege_up_scale.verticalPixelIndex + 2 ;
        srcPtr = p_engine->jpege_up_scale.lumaInputPtr
                 + (int32_t)fetchPixelIndex * p_engine->jpege_up_scale.rotatedLumaSrcIncrement;
        outputPtr = p_engine->p_input_luma_line_buffer->ptr + pixelIndex * extendedWidth;
    }

    switch (p_engine->rotation)
    {
    case 90:
    case 180:
    case 270:
    {
        /*------------------------------------------------------------
          For rotated cases, it fetch pixel by pixel
        ------------------------------------------------------------*/
        jpege_engine_sw_copy_rotated_luma_lines(srcPtr,
                                                outputPtr,
                                                inputWidth,
                                                fetchIncrement,
                                                pixelExtendRight);
        if (lineIndex == 0)
        {
            outputPtr += extendedWidth;
            for (i = 1; i < 4; i++)
            {
                jpege_engine_sw_copy_rotated_luma_lines(srcPtr,
                                                        outputPtr,
                                                        inputWidth,
                                                        fetchIncrement,
                                                        pixelExtendRight);
                srcPtr += p_engine->jpege_up_scale.rotatedLumaSrcIncrement;
                outputPtr += extendedWidth;
            }
        }
        break;
    }
    case 0:
    default:
    {
        /*------------------------------------------------------------
          For non rotated cases, it fetch line by line
        ------------------------------------------------------------*/
        // Extend the first line
        // Extend the left
        *outputPtr = *srcPtr;
        // Fetch a line
        (void)STD_MEMMOVE(outputPtr + 1, srcPtr, (inputWidth * sizeof(uint8_t)));
        // Extend the right
        for (k = 1; k <= pixelExtendRight; k++)
        {
            *(outputPtr + inputWidth + k) = *(srcPtr + inputWidth - 1);
        }
        if (lineIndex == 0)
        {
            outputPtr += extendedWidth;
            for (i = 1; i < 4; i++)
            {
                // Extend the first line
                // Extend the left
                *outputPtr = *srcPtr;
                // Fetch a line
                (void)STD_MEMMOVE(outputPtr + 1, srcPtr, (inputWidth * sizeof(uint8_t)));
                // Extend the right
                for (k = 1; k <= pixelExtendRight; k++)
                {
                    *(outputPtr + inputWidth + k) = *(srcPtr + inputWidth - 1);
                }
                srcPtr += fetchIncrement;
                outputPtr += extendedWidth;
            }
        }
    }
    } // end of switch
}

/*===========================================================================
FUNCTION        jpege_engine_sw_copy_rotated_chroma_lines

DESCRIPTION     Copy rotated chroma lines

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_copy_rotated_chroma_lines(uint8_t *srcPtr,
                                                      uint8_t *outputPtr,
                                                      uint32_t inputWidth,
                                                      int32_t  fetchIncrement,
                                                      uint32_t pixelExtendRight)
{
    uint32_t k;
    // Extend the first line
    // Extend the left
    *outputPtr = *srcPtr;
    *(outputPtr + 1) = *(srcPtr + 1);
    for (k = 0; k < inputWidth; k += 2)
    {
        *(outputPtr + k + 2)
            = *(srcPtr + fetchIncrement * k);
        *(outputPtr + k + 3)
            = *(srcPtr + fetchIncrement * k + 1);
    }
    // Extend the right
    for (k = 1; k <= pixelExtendRight; k++)
    {
        *(outputPtr + inputWidth + 2 * k)
            = *(srcPtr + fetchIncrement * (inputWidth - 2) );
        *(outputPtr + inputWidth + 2 * k + 1)
            = *(srcPtr + fetchIncrement * (inputWidth - 2) + 1);
    }
}

/*===========================================================================
FUNCTION        jpege_engine_sw_fetch_chroma_to_lines_buffer

DESCRIPTION     Fetch Chroma from the input buffer to lines buffer

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_fetch_chroma_to_lines_buffer(jpege_engine_sw_t *p_engine,
                                                         uint32_t lineIndex)
{
    /*------------------------------------------------------------
     For rotation =0, it can fetch one line
    ------------------------------------------------------------*/
    uint32_t inputWidth = p_engine->jpege_up_scale.verticalChromaFetchWidth * 2;
    uint32_t extendedWidth = p_engine->jpege_up_scale.cropChromaExtendedWidth * 2;
    int32_t fetchIncrement = p_engine->jpege_up_scale.rotatedChromaFetchIncrement;
    /*---------------------------------------------------------------
     For one output line, it needs line A-1, A, A+1, up to line A+2,
     And this is a 4 line buffer,
     plus there is 1 extended line at the top of the beginning
    ----------------------------------------------------------------*/
    uint32_t pixelIndex = MOD4(p_engine->jpege_up_scale.verticalPixelIndex + 3);
    uint32_t pixelExtendRight = p_engine->jpege_up_scale.pixelExtendRight;
    uint32_t fetchPixelIndex;
    uint8_t  *srcPtr, *outputPtr;
    uint32_t i, k;

    if (lineIndex == 0)
    {
        /*------------------------------------------------------------
         Fetch from the beginning
        ------------------------------------------------------------*/
        srcPtr = p_engine->jpege_up_scale.chromaInputPtr;
        outputPtr = p_engine->p_input_chroma_line_buffer->ptr;
    }
    else if ((p_engine->jpege_up_scale.verticalPixelIndex + 2) >= p_engine->jpege_up_scale.cropChromaHeight)
    {
        /*------------------------------------------------------------
         Fetch from the last row
        ------------------------------------------------------------*/
        fetchPixelIndex = p_engine->jpege_up_scale.cropChromaHeight - 1;
        srcPtr = p_engine->jpege_up_scale.chromaInputPtr
                  + (int32_t)fetchPixelIndex * 2 * p_engine->jpege_up_scale.rotatedChromaSrcIncrement;
        outputPtr = p_engine->p_input_chroma_line_buffer->ptr + pixelIndex * (extendedWidth);
    }
    else
    {
        fetchPixelIndex = p_engine->jpege_up_scale.verticalPixelIndex + 2;
        srcPtr = p_engine->jpege_up_scale.chromaInputPtr
                  + (int32_t)fetchPixelIndex * 2 * p_engine->jpege_up_scale.rotatedChromaSrcIncrement;
        outputPtr = p_engine->p_input_chroma_line_buffer->ptr + pixelIndex * (extendedWidth);
    }

    switch (p_engine->rotation)
    {
    case 90:
    case 180:
    case 270:
    {
        /*------------------------------------------------------------
          For rotated cases, it fetch pixel by pixel
        ------------------------------------------------------------*/
        jpege_engine_sw_copy_rotated_chroma_lines(srcPtr,
                                                  outputPtr,
                                                  inputWidth,
                                                  fetchIncrement,
                                                  pixelExtendRight);
        if (lineIndex == 0)
        {
            outputPtr += extendedWidth;
            for (i = 1; i < 4; i++)
            {
                jpege_engine_sw_copy_rotated_chroma_lines(srcPtr,
                                                          outputPtr,
                                                          inputWidth,
                                                          fetchIncrement,
                                                          pixelExtendRight);
                srcPtr += 2 * p_engine->jpege_up_scale.rotatedChromaSrcIncrement;
                outputPtr += extendedWidth;
            }
        }
        break;
    }
    case 0:
    default:
    {
        /*------------------------------------------------------------
          For non rotated cases, it fetch line by line
        ------------------------------------------------------------*/
        // Extend the left
        *outputPtr = *srcPtr;
        *(outputPtr + 1) = *(srcPtr + 1);
        (void)STD_MEMMOVE(outputPtr + 2, srcPtr, (inputWidth * sizeof(uint8_t)));
        // Extend the right
        for (k = 1; k <= pixelExtendRight; k++)
        {
            *(outputPtr + inputWidth + 2 * k) = *(srcPtr + inputWidth - 2);
            *(outputPtr + inputWidth + 2 * k + 1) = *(srcPtr + inputWidth - 1);
        }
        if (lineIndex == 0)
        {
            outputPtr += extendedWidth;
            for (i = 1; i < 4; i++)
            {
                // Extend the left
                *outputPtr = *srcPtr;
                *(outputPtr + 1) = *(srcPtr + 1);
                (void)STD_MEMMOVE(outputPtr + 2, srcPtr, (inputWidth * sizeof(uint8_t)));
                // Extend the right
                for (k = 1; k <= pixelExtendRight; k++)
                {
                    *(outputPtr + inputWidth + 2 * k) = *(srcPtr + inputWidth - 2);
                    *(outputPtr + inputWidth + 2 * k + 1) = *(srcPtr + inputWidth - 1);
                }
                srcPtr += fetchIncrement * 2;
                outputPtr += extendedWidth;
            }
        }
      }
    } // end of switch
}

/*===========================================================================
FUNCTION        jpege_engine_sw_upsample_scale_mcu_lines

DESCRIPTION     Scale MCU Lines from the input buffers
                1) fetch the luma/chroma lines to line buffer
                2) vertical scale luma/chroma lines
                3) horizontal scale luma/chroma lines
                4) output to m_pScaleBuffer

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_scale_mcu_lines(jpege_engine_sw_t *p_engine)
{
    uint32_t j,k;
    uint32_t vertAccumulator = 0;
    uint32_t vertChromaAccumulator = 0;
    uint8_t  *pVertScaleLine;
    jpege_up_scale_t *pScaleInfo = &p_engine->jpege_up_scale;

    /*------------------------------------------------------------
      Luma upscaling
    ------------------------------------------------------------*/
    uint8_t *pInputLine= p_engine->p_input_luma_line_buffer->ptr;

    uint8_t *pScaleLine = p_engine->p_inline_scaled_buffer->ptr;

    for (j = pScaleInfo->verticalRowIndex;
         j < pScaleInfo->verticalRowIndex + p_engine->rotatedMCUHeight && j < pScaleInfo->outputHeight;
         j++)
    {
        pVertScaleLine = p_engine->p_vertical_scaled_buffer->ptr;

        if (j == 0)
        {
            pScaleInfo->verticalLumaAccumulator = pScaleInfo->verticalInit;
        }
	    else
        {
            pScaleInfo->verticalLumaAccumulator += pScaleInfo->verticalStep;
        }

        vertAccumulator = pScaleInfo->verticalLumaAccumulator;

        /*------------------------------------------------------------
          POLYPHASE_ACCUM_Q_BITS used for quantity
        ------------------------------------------------------------*/
	    pScaleInfo->verticalPixelIndex = (vertAccumulator >> POLYPHASE_ACCUM_Q_BITS);

        if (j == 0)
        {
            pScaleInfo->verticalPixelIndex = 0;
        }

        /*------------------------------------------------------------
          phase Index gets the most significant 5 fractional bits
          of Accumulator
        ------------------------------------------------------------*/
	    pScaleInfo->verticalPhaseIndex = (uint32_t)((vertAccumulator >> (POLYPHASE_ACCUM_Q_BITS - 5))
                                                   - (pScaleInfo->verticalPixelIndex << 5));

        // phaseIndex is between 0 and 31
        if (pScaleInfo->verticalPhaseIndex > 31)
	    {
            pScaleInfo->verticalPhaseIndex = 0;
        }
        // fetch lines if not in the luma lines buffer
        if ((j == 0) || (pScaleInfo->verticalPixelIndex > pScaleInfo->verticalLastIndex))
        {
            jpege_engine_sw_fetch_luma_to_lines_buffer(p_engine,j);
        }

        /*------------------------------------------------------------
          Vertical scale a luma line
          There are one line extended in line buffer at top
        ------------------------------------------------------------*/
        if (pScaleInfo->verticalScaleFlag)
        {
            jpege_engine_sw_vertical_scale_luma_line(pInputLine, pVertScaleLine, pScaleInfo);
        }
        else
        {
            (void)STD_MEMMOVE(pVertScaleLine,
                              p_engine->p_input_luma_line_buffer->ptr + (MOD4(j + 1)) * (pScaleInfo->cropExtendedWidth),
                             (pScaleInfo->cropExtendedWidth * sizeof(uint8_t)));
        }

        pScaleInfo->verticalLastIndex = pScaleInfo->verticalPixelIndex;

        /*------------------------------------------------------------
          Horizontal scale a luma line
        ------------------------------------------------------------*/
        if (pScaleInfo->horizontalScaleFlag)
        {
            jpege_engine_sw_horizontal_scale_luma_line( pVertScaleLine, pScaleLine, pScaleInfo );
        }
        else
        {
            (void)STD_MEMMOVE(pScaleLine, pVertScaleLine + 1, (pScaleInfo->outputWidth * sizeof(uint8_t)));
        }

        pVertScaleLine += pScaleInfo->cropExtendedWidth;

        pScaleLine += pScaleInfo->outputWidth;
    }

    /*------------------------------------------------------------
      Compute feteched luma blocks
    ------------------------------------------------------------*/
    pScaleInfo->fetchedLumaBlocks = ((j - pScaleInfo->verticalRowIndex) * pScaleInfo->outputWidth
                                      + JBLOCK_SIZE - 1) / JBLOCK_SIZE ;

    // in-line upscale
    // need to remember the continuing line
    pScaleInfo->verticalRowIndex = j;

    /*------------------------------------------------------------
      Chroma vertical upscaling
    ------------------------------------------------------------*/
    pInputLine= p_engine->p_input_chroma_line_buffer->ptr;

    pScaleLine = p_engine->p_inline_scaled_buffer->ptr + pScaleInfo->outputWidth * p_engine->rotatedMCUHeight;

    for (k = pScaleInfo->verticalChromaRowIndex;
         k < pScaleInfo->verticalChromaRowIndex + BLOCK_HEIGHT && k < pScaleInfo->outputChromaHeight;
         k++)
    {
        pVertScaleLine = p_engine->p_vertical_scaled_buffer->ptr;

        if (k == 0)
        {
            pScaleInfo->verticalChromaAccumulator = pScaleInfo->verticalInit;
        }
        else
        {
            pScaleInfo->verticalChromaAccumulator += pScaleInfo->verticalStep;
        }

        vertChromaAccumulator = pScaleInfo->verticalChromaAccumulator;

        /*------------------------------------------------------------
          POLYPHASE_ACCUM_Q_BITS used for quantity
        ------------------------------------------------------------*/
	    pScaleInfo->verticalPixelIndex = (vertChromaAccumulator >> POLYPHASE_ACCUM_Q_BITS);

        if (k == 0)
        {
            pScaleInfo->verticalPixelIndex = 0;
        }

        /*------------------------------------------------------------
          phase Index gets the most significant 5 fractional bits
          of Accumulator
        ------------------------------------------------------------*/
        pScaleInfo->verticalPhaseIndex = (uint32_t)((vertChromaAccumulator >> (POLYPHASE_ACCUM_Q_BITS - 5))
                                                   - (pScaleInfo->verticalPixelIndex << 5));

        // phaseIndex is between 0 and 31
        if (pScaleInfo->verticalPhaseIndex > 31)
	    {
            pScaleInfo->verticalPhaseIndex = 0;
        }
        // fetch lines if not in the chroma lines buffer
        if ((k == 0) || (pScaleInfo->verticalPixelIndex > pScaleInfo->verticalChromaLastIndex))
        {
            jpege_engine_sw_fetch_chroma_to_lines_buffer(p_engine, k);
        }
        /*------------------------------------------------------------
          Vertical scale
        ------------------------------------------------------------*/
        if (pScaleInfo->verticalScaleFlag)
        {
            jpege_engine_sw_vertical_scale_chroma_line(pInputLine, pVertScaleLine, pScaleInfo);
        }
        else
        {
            (void)STD_MEMMOVE(pVertScaleLine,
                             p_engine->p_input_chroma_line_buffer->ptr + (MOD4(k+1)) * pScaleInfo->cropChromaExtendedWidth * 2,
                             (pScaleInfo->cropChromaExtendedWidth * 2 * sizeof(uint8_t)));
        }

        pScaleInfo->verticalChromaLastIndex = pScaleInfo->verticalPixelIndex;

        /*------------------------------------------------------------
          Horizontal scale
        ------------------------------------------------------------*/
        if (pScaleInfo->horizontalScaleFlag)
        {
            jpege_engine_sw_horizontal_scale_chroma_line(pVertScaleLine, pScaleLine, pScaleInfo);
        }
        else
        {
            (void)STD_MEMMOVE(pScaleLine, pVertScaleLine + 2,
                             (pScaleInfo->outputChromaWidth * 2 * sizeof(uint8_t)));
        }

        pVertScaleLine += pScaleInfo->cropChromaExtendedWidth * 2;

        pScaleLine += pScaleInfo->outputChromaWidth * 2;
    }

    // in-line upscale
    // need to remember the continuing chroma line
    pScaleInfo->verticalChromaRowIndex = k;
}

/*===========================================================================
FUNCTION        jpege_engine_sw_vertical_scale_luma_line

DESCRIPTION     Verticallly Scale Luma Line

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_vertical_scale_luma_line(uint8_t * pInputLine,
                                                     uint8_t * pOutputLine,
                                                     jpege_up_scale_t *pScaleInfo )
{
    uint32_t i, dwStepWidth;
    int32_t  nWeightSum;
    const int16_t  *pFilter;
    uint32_t coeffIndex ,pixelIndex;
    uint8_t  *pLastLine, *pCurrLine, *pNextLine, *pAdvcLine;

    /*------------------------------------------------------------
      Scale Luma Line
    ------------------------------------------------------------*/
    dwStepWidth = pScaleInfo->cropExtendedWidth;

    // current index
    pixelIndex = 1 + pScaleInfo->verticalPixelIndex;

    // 32 polyphase with 4 tap each
    // calculate polyphase coefficient index
    coeffIndex = pScaleInfo->verticalPhaseIndex << 2;

    pFilter = &(m_polyphaseCoeff4taps[coeffIndex]);

    // vertical scale for one line
    // it need pixelIndex-1, pixelIndex, pixelIndex+1, pixelIndex+2 lines
    pLastLine = pInputLine + MOD4((pixelIndex - 1)) * dwStepWidth;
    pCurrLine = pInputLine + MOD4((pixelIndex + 0)) * dwStepWidth;
    pNextLine = pInputLine + MOD4((pixelIndex + 1)) * dwStepWidth;
    pAdvcLine = pInputLine + MOD4((pixelIndex + 2)) * dwStepWidth;

    for (i = 0; i < pScaleInfo->cropExtendedWidth; i++ )
    {
        // 4-tap filter
        nWeightSum =  pFilter[0] * (*(pLastLine + i)) + pFilter[1] * (*(pCurrLine + i))
                    + pFilter[2] * (*(pNextLine + i)) + pFilter[3] * (*(pAdvcLine + i)) ;

        // coefficents sum equal to 512
        // make rounding
        nWeightSum = (1 + (nWeightSum >> 8)) >> 1;

        // CLAMP to between 0 and 255
        // write output pixel
        *(pOutputLine + i) = (uint8_t) (CLAMP(nWeightSum));
    } //end for loop

} // end of jpege_engine_sw_vertical_scale_luma_line

/*===========================================================================
FUNCTION        jpege_engine_sw_vertical_scale_chroma_line

DESCRIPTION     Verticallly Scale Chroma Lines

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_vertical_scale_chroma_line(uint8_t * pInputLine,
                                                       uint8_t * pOutputLine,
                                                       jpege_up_scale_t *pScaleInfo  )
{
    uint32_t i, dwStepWidth;
    int32_t  nWeightSum;
    const int16_t  *pFilter;
    uint32_t coeffIndex, pixelIndex;
    uint8_t  *pLastLine, *pCurrLine, *pNextLine, *pAdvcLine;

    /*------------------------------------------------------------
      Scale for both Cb and Cr
    ------------------------------------------------------------*/
    dwStepWidth = pScaleInfo->cropChromaExtendedWidth * 2;

    pixelIndex = 1 + pScaleInfo->verticalPixelIndex;

    // 32 polyphase with 4 tap each
    // calculate polyphase coefficient index
    coeffIndex = pScaleInfo->verticalPhaseIndex << 2;

    pFilter = &(m_polyphaseCoeff4taps[coeffIndex]);

    // vertical scale for one line
    // it need pixelIndex-1, pixelIndex, pixelIndex+1, pixelIndex+2 lines
    pLastLine = pInputLine + MOD4((pixelIndex - 1)) * dwStepWidth ;
    pCurrLine = pInputLine + MOD4((pixelIndex + 0)) * dwStepWidth ;
    pNextLine = pInputLine + MOD4((pixelIndex + 1)) * dwStepWidth ;
    pAdvcLine = pInputLine + MOD4((pixelIndex + 2)) * dwStepWidth ;

    for (i = 0; i< pScaleInfo->cropChromaExtendedWidth * 2; i += 2)
    {
        // 4-tap filter
        nWeightSum =   pFilter[0] * (*(pLastLine + i)) + pFilter[1] * (*(pCurrLine + i))
                     + pFilter[2] * (*(pNextLine + i)) + pFilter[3] * (*(pAdvcLine + i)) ;

        // coefficents sum equal to 512
        // make rounding
        nWeightSum = (1 + (nWeightSum >> 8)) >> 1;

        // CLAMP to between 0 and 255
        // write output pixel
        *(pOutputLine + i) = (uint8_t)(CLAMP(nWeightSum));

        // 4-tap filter
        nWeightSum =   pFilter[0] * (*(pLastLine + i + 1)) + pFilter[1] * (*(pCurrLine + i + 1))
                     + pFilter[2] * (*(pNextLine + i + 1)) + pFilter[3] * (*(pAdvcLine + i + 1)) ;

        // coefficents sum equal to 512
        // make rounding
        nWeightSum = (1 + (nWeightSum >>8)) >> 1;

        // CLAMP to between 0 and 255
        // write output pixel
        *(pOutputLine + i + 1) = (uint8_t)(CLAMP(nWeightSum));
    } // end for loop

} // end of  jpege_engine_sw_vertical_scale_chroma_line

/*===========================================================================
FUNCTION        jpege_engine_sw_horizontal_scale_luma_line

DESCRIPTION     Horizontally Scale Luma Lines
                1) caculate accumulator
                2) caculate pixel index
                3) caculate phase index
                3) output luma line afer scale

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_horizontal_scale_luma_line(uint8_t * pInputLine,
                                                       uint8_t * pOutputLine,
                                                       jpege_up_scale_t *pScaleInfo )
{
    uint32_t i;
    const int16_t  *pFilter;
    int32_t  nWeightSum;
    uint32_t horiAccumulator = 0, phaseIndex, coeffIndex, pixelIndex;

    /*------------------------------------------------------------
      Scale Luma Line
    ------------------------------------------------------------*/
    for (i = 0; i < pScaleInfo->outputWidth; i++)
    {
        if (i == 0)
        {
            horiAccumulator = pScaleInfo->horizontalInit;
        }
        else
        {
            horiAccumulator += pScaleInfo->horizontalStep;
        }

        // Compute pixel index
        pixelIndex = 1 + (horiAccumulator >> POLYPHASE_ACCUM_Q_BITS);
        if (i == 0)
        {
            pixelIndex = 1;
        }

        // Compute phase
        phaseIndex = (uint32_t)((horiAccumulator>>(POLYPHASE_ACCUM_Q_BITS - 5))
                              - ((pixelIndex-1) << 5));

        // phaseIndex is between 0 and 31
        if (phaseIndex > 31)
        {
            phaseIndex = 0;
        }
        // 32 polyphase with 4 tap each
        // calculate polyphase coefficient index
        coeffIndex = phaseIndex << 2;

        pFilter = &(m_polyphaseCoeff4taps[coeffIndex]);

        // 4-tap filter
        nWeightSum =   pFilter[0] * (*(pInputLine + pixelIndex - 1))
                     + pFilter[1] * (*(pInputLine + pixelIndex   ))
                     + pFilter[2] * (*(pInputLine + pixelIndex + 1))
                     + pFilter[3] * (*(pInputLine + pixelIndex + 2)) ;

        // coefficents sum equal to 512
        // make rounding
        nWeightSum = (1 + (nWeightSum >>8)) >> 1;

        // CLAMP to between 0 and 255
        // write output pixel
        *(pOutputLine + i) = (uint8_t)(CLAMP(nWeightSum));
    }
}

/*===========================================================================
FUNCTION        jpege_engine_sw_horizontal_scale_chroma_line

DESCRIPTION     Horizontally Scale Chroma Lines
                1) caculate accumulator
                2) caculate pixel index
                3) caculate phase index
                3) output chroma line afer scale

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
===========================================================================*/
static void jpege_engine_sw_horizontal_scale_chroma_line(uint8_t * pInputLine,
                                                         uint8_t * pOutputLine,
                                                         jpege_up_scale_t *pScaleInfo )
{
    uint32_t i;
    const int16_t  *pFilter;
    int32_t  nWeightSum;
    uint32_t horiAccumulator = 0, phaseIndex, coeffIndex, pixelIndex;

    /*------------------------------------------------------------
      Scale Lines for both Cb Cr
    ------------------------------------------------------------*/
    for (i = 0; i < pScaleInfo->outputChromaWidth * 2; i += 2)
    {
        if (i == 0)
        {
            horiAccumulator = pScaleInfo->horizontalInit;
        }
        else
        {
            horiAccumulator += pScaleInfo->horizontalStep;
        }

        // Compute pixel index
        pixelIndex = 1 + (horiAccumulator>>POLYPHASE_ACCUM_Q_BITS);
        if (i == 0)
        {
            pixelIndex = 1;
        }

        // Compute phase
        phaseIndex = (uint32_t)((horiAccumulator>>(POLYPHASE_ACCUM_Q_BITS - 5))
                              - ((pixelIndex-1) << 5));

        // phaseIndex is between 0 and 31
        if (phaseIndex > 31)
        {
            phaseIndex = 0;
        }
        // 32 polyphase with 4 tap each
        // calculate polyphase coefficient index
        coeffIndex = phaseIndex << 2;

        pFilter = &(m_polyphaseCoeff4taps[coeffIndex]);

        // 4-tap filter
        nWeightSum =   pFilter[0] * (*(pInputLine + 2 * (pixelIndex - 1) ))
                     + pFilter[1] * (*(pInputLine + 2 * pixelIndex       ))
                     + pFilter[2] * (*(pInputLine + 2 * (pixelIndex + 1) ))
                     + pFilter[3] * (*(pInputLine + 2 * (pixelIndex + 2) )) ;

        // sum of coefficients equals to 516=2^9
        nWeightSum = (1 + (nWeightSum >>8)) >> 1;

        // CLAMP to between 0 and 255
        // write output pixel
        *(pOutputLine + i) = (uint8_t)(CLAMP(nWeightSum));

        nWeightSum =   pFilter[0] * (*(pInputLine + 2 * (pixelIndex - 1) + 1))
                     + pFilter[1] * (*(pInputLine + 2 *  pixelIndex      + 1))
                     + pFilter[2] * (*(pInputLine + 2 * (pixelIndex + 1) + 1))
                     + pFilter[3] * (*(pInputLine + 2 * (pixelIndex + 2) + 1)) ;

        // sum of coefficients equals to 516=2^9
        nWeightSum = (1 + (nWeightSum >>8)) >> 1;

        // CLAMP to between 0 and 255
        // write output pixel
        *(pOutputLine + i + 1) = (uint8_t)(CLAMP(nWeightSum));
    }
 }
