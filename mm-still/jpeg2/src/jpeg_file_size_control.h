#ifndef _JPEG_FILESIZECONTROL_H
#define _JPEG_FILESIZECONTROL_H
/*========================================================================

                 C o m m o n   D e f i n i t i o n s

*//** @file jpeg_file_size_control.h

Copyright (c) 2009-2011 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary.  Export of this technology or software is regulated
by the U.S. Government, Diversion contrary to U.S. law prohibited.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
02/16/10   sw      Changed file size control state checking to on whether
                   it is succeed and whether it is in output state.
11/02/09   sw      Created file size control for a target file size.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations / Macro Definitions
** ----------------------------------------------------------------------- */
// Factor by which target file size is scaled in order to accomodate
// algorithmic error in prediction.
#define TARGET_FILE_SIZE_SCALE_PERCENT   90

// Default Header Size
#define DEFAULT_HEADER_SIZE 2000

// Typical number of EOB bits per 8x8 block
// EOB is 4 bits
#define EOB_BITS_PER_H2V2_BLOCK 6
#define EOB_BITS_PER_H2V1_OR_H1V2_BLOCK 8
#define EOB_BITS_PER_H1V1_BLOCK 12

// Minimum number of bytes required to encode AC
#define MIN_NUM_BYTES_FOR_AC 500

// Square and Linear term coefficients for bit-rate vs scale factor equation
#define SCALE_SQUARE_COEFF_Q8 (uint32_t)(256 * 0.6869)
#define SCALE_LINEAR_COEFF_Q8 (uint32_t)(256 * 1.5825)

// Scale factor corresponding to quality 50.
#define SCALE_FACTOR_50_PERCENT_Q8 (uint32_t)(1 * 256)
#define SCALE_FACTOR_1_PER_Q8 (uint32_t)(0.02 * 256)

/* Max number of extra passes allowed for file size control on main image.
   Before first encoding, the quant values are estimated for both
   main and thumbnail. The number of extra passes with file size control
   is defined by FILE_SIZE_MAX_NUM_PASSES, which quant tables are scaled
   only for main images. Afer encoding with number of FILE_SIZE_MAX_NUM_PASSES,
   if succeed, there is a final encoding to output.
*/
#define FILE_SIZE_MAX_NUM_PASSES 5

/* Place an upper limit on the change in the scale factor from pass to
   pass by caping the K multiplication factor.  This is needed
   because of corner cases with very simple image subjects that can
   result in huge changes in K
   This 2 allows for (at most) a 25% increase in JPEG the quality
   percent */
#define MAX_K_FACTOR_VALUE       2

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef enum
{
    PASS_ENCODING,              // Encoding
    PASS_1_SUCCEEDED,           // Succeed after 1st pass
    PASS_SUCCEEDED_TO_OUTPUT,   // Succeed of last pass, and this
                                //    pass is to output
    FILE_SIZE_CONTROL_FAILURE   // File size control fail
} file_size_control_state_t;

typedef struct {
    int32_t  estnum_bytes_AC;        // Estimated number of AC bytes
    uint32_t est_ACbits_per_pix_Q8;  // Derived from estnum_bytes_DC

    uint32_t actual_num_bytes_AC;    // Number of AC bytes
    uint32_t actual_ACbits_per_pix_Q8;// Number of AC bits per pixel Q8 format

    uint32_t scale_factor_Q8;        // Quant Table scale factor for curr pass.
    uint32_t quality_percent;        // Quality for curr pass
} pass_info_t;

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/**
 * This is the implementation of File Size Control
*/
typedef struct
{
    file_size_control_state_t state; // State of the file size rate control

    uint32_t  target_filesize;       // Target file size in bytes (not scaled down)

    uint32_t  scaled_target_filesize;
                                     // Scaled down target file size

    uint32_t  curr_filesize;         // Current file size in bytes

    uint16_t  iteration_count;       // Iteration count (starts at 0)

    uint32_t  max_quality;           // Max quality to use (if applicable)

    uint32_t  max_scale_factor_Q8;   // Max scale factor (derived from max quality)

    uint32_t  target_scale_factor_Q8;// Origianl scale factor calculated

    uint32_t  num_blocks;            // Number of 8x8 blocks in the main + thumbnail
                                     // images

    uint32_t  estnum_bytes_EOB;      // Estimated number of EOB bytes (for luma and
                                     // chroma)

    uint32_t  estnum_bytes_DC;       // Estimated number of DC bytes (for luma and
                                     // chroma)

    uint32_t  thumbnail_size;        // Thumbnail size

    uint8_t   overflow_flag;         // Overflow flag

    jpeg_buf_t  * output_buffer;     // Output buffer pointer

    pass_info_t pass_info[FILE_SIZE_MAX_NUM_PASSES];

} jpege_file_size_control_t;

/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Function Declarations
** ======================================================================= */

int jpegfsc_init(jpege_file_size_control_t   *p_file_size_control,
                 jpege_cfg_t   encode_cfg);
int jpegfsc_set_max_quality (jpege_file_size_control_t   *p_file_size_control,
                             uint32_t   mainimg_quality);
void jpegfsc_set_output_buffer (jpege_file_size_control_t  *p_file_size_control,
                                jpeg_buf_t   *p_buf);
int jpegfsc_is_state_output (jpege_file_size_control_t   file_size_control);
int jpegfsc_is_succeed (jpege_file_size_control_t   file_size_control);
void jpegfsc_update_output_size(jpege_file_size_control_t   *p_file_size_control,
                                uint32_t   curr_buffer_size);
/**
 * Estimate the target image quality based on image resolution and
 * target file size.
 */
int jpegfsc_estimate_target_quality (jpege_file_size_control_t *p_file_size_control,
                                     uint32_t width,
                                     uint32_t height,
                                     uint32_t thumbnail_size,
                                     jpeg_subsampling_t subsampling);
/**
 * Estimate DC Size.
 */
uint32_t jpegfsc_estimate_dcsize (uint32_t blocks,
                                  jpeg_subsampling_t subsampling);
/**
 * Calculate Quality Scale Facotr.
 */
uint32_t jpegfsc_calculate_quant_scale_factor(uint32_t  filesize_AC,
                                              uint32_t  num_blocks,
                                              uint32_t  * scale_factor_Q8,
                                              uint32_t  * quality_percent,
                                              uint32_t  * acbits_per_pixel_Q8,
                                              uint32_t   max_scale_factor_Q8);
/**
 * Handle when encode is done for file size control.
 */
int jpegfsc_encode_done_handler (jpege_file_size_control_t  *p_file_size_control,
                                 jpege_cfg_t *p_cfg,
                                 uint16_t **dst_main_luma_quant_tbl,
                                 uint16_t **dst_main_chroma_quant_tbl);
/**
 * Scale Quant Table.
 */
int jpegfsc_scale_quant_table (uint32_t nScaleFactor,
                               uint32_t nOldScaleFactor,
                               uint16_t *src_quant_tbl,
                               uint16_t **dst_quant_tbl);
/**
 * Init Main or Thumbnail Luma and Chroma Quant Tables
 */
int jpegfsc_init_quant_tables (jpege_file_size_control_t  file_size_control,
                               jpege_img_cfg_t *p_cfg,
                               uint16_t **dst_luma_quant_tbl,
                               uint16_t *src_luma_quant_tbl,
                               uint16_t **dst_chroma_quant_tbl,
                               uint16_t *src_chroma_quant_tbl);
#endif /* _JPEG_FILESIZECONTROL_H */
