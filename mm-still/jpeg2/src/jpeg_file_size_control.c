/*========================================================================


*//** @file jpeg_file_size_control.cpp

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (c) 2009-2011 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary.  Export of this technology or software is regulated
by the U.S. Government, Diversion contrary to U.S. law prohibited.
========================================================================*/

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
02/16/10   sw      Changed file size control state checking to on whether
                   it is succeed and whether it is in output state.
11/02/09   sw      Added file size control on AC bits per pixels for a
                   target file size in Kbytes, and allowed multiple passes
                   encoding with scaled quant tables.
10/01/09   sw      Created file.
======================================================================= */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

======================================================================= */
#include "jpeg_common_private.h"
#include "jpeg_common.h"
#include "jpege.h"
#include "jpegerr.h"
#include "jpeglog.h"
#include "jpeg_buffer_private.h"
#include "jpeg_file_size_control.h"

/* -----------------------------------------------------------------------
** Type Declarations
** -------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Global Object Definitions
** -------------------------------------------------------------------- */

/* -----------------------------------------------------------------------
** Local Object Definitions
** -------------------------------------------------------------------- */

/*------------------------------------------------------------------------
The (actual file size from current pass)/(target file size) ratio is
compared against the corresponding threshold below to determine whether
the undershoot from current pass is acceptable.
Note: At lower quality factors, undershoot is less tolerated.
------------------------------------------------------------------------*/
uint32_t file_size_undershoot_thresholds[] = {
    80,   /* for 0-10  percent quality */
    80,   /* for 11-20 percent quality */
    70,   /* for 21-30 percent quality */
    60,   /* for 31-40 percent quality */
    50,   /* for 41-50 percent quality */
    40,   /* for 51-60 percent quality */
    30,   /* for 61-70 percent quality */
    30,   /* for 71-80 percent quality */
    30,   /* for 81-90 percent quality */
    30    /* for 91-100 percent quality */
};

/* =======================================================================
**                          Macro Definitions
** =====================================================================*/

// CLAMP macro
#define _CLAMP(d,low,high) \
             {if ((d) < low) (d) = low; if ((d) > high) (d) = high;}

// Calculate scaling facotr based on equation
//  = 1.5825 * acBitsPerPixel + 0.6869 * (acBitsPerPixel ^ 2)
#define SCALE_FACTOR(a) \
             (((SCALE_LINEAR_COEFF_Q8 * (a)) >> 8) + \
             ((SCALE_SQUARE_COEFF_Q8 * (a) * (a)) >> 16))


/* =======================================================================
**                          Function Definitions
** ==================================================================== */

/*========================================================================
FUNCTION        jpegfsc_init

DESCRIPTION     jpegfsc initialization

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
========================================================================*/
int jpegfsc_init(jpege_file_size_control_t *p_file_size_control,
                 jpege_cfg_t  encode_cfg)
{
    int rc = JPEGERR_SUCCESS;

    // Validate inputs
    if (!p_file_size_control)
        return JPEGERR_ENULLPTR;

    // Zero all fields in the structure
    STD_MEMSET(p_file_size_control, 0, sizeof(jpege_file_size_control_t));

    // Convert the specified target file size Kbytes to bytes
    p_file_size_control->target_filesize = 1024 * encode_cfg.target_filesize ;
    // Scale down the target file size to accomodate error
    p_file_size_control->scaled_target_filesize =
        (uint32_t) (p_file_size_control->target_filesize
                 * TARGET_FILE_SIZE_SCALE_PERCENT / 100);
    // Initialize file size counter
    p_file_size_control->curr_filesize = 0;
    p_file_size_control->thumbnail_size = 0;
    // Initialize iteration count to 0
    p_file_size_control->iteration_count = 0;
    // Initiatlize state
    p_file_size_control->state = PASS_ENCODING;
    // Initialize overflow flag
    p_file_size_control->overflow_flag = false;
    // Initialize Buffer
    p_file_size_control->output_buffer = NULL;
    return rc;
}

/*========================================================================
FUNCTION        jpegfsc_set_max_quality

DESCRIPTION     Set Max Quality

DEPENDENCIES    None

RETURN VALUE    AEEResult

SIDE EFFECTS    None
========================================================================*/
int jpegfsc_set_max_quality (jpege_file_size_control_t *p_file_size_control,
                             uint32_t mainimg_quality)
{
    // Obtain main image quality factor from ImProperties
    if (mainimg_quality < 1 || mainimg_quality > 100)
        return JPEGERR_EBADPARM;
    CLAMP(mainimg_quality, 1, 98);

    // Set Max Quality
    p_file_size_control->max_quality = mainimg_quality;
    return JPEGERR_SUCCESS;
}

/*========================================================================
FUNCTION        jpegfsc_set_output_buffer

DESCRIPTION     Set Output Buffer

DEPENDENCIES    None

RETURN VALUE    None

SIDE EFFECTS    None
========================================================================*/
void jpegfsc_set_output_buffer (jpege_file_size_control_t   *p_file_size_control,
                                jpeg_buf_t   *p_buf)
{
    p_file_size_control->output_buffer = p_buf;
}

/*========================================================================
FUNCTION        jpegfsc_is_state_output

DESCRIPTION     Check State Is Output

DEPENDENCIES    None

RETURN VALUE    BOOLEAN

SIDE EFFECTS    None
========================================================================*/
int jpegfsc_is_state_output (jpege_file_size_control_t  file_size_control)
{
    return ((file_size_control.state == PASS_SUCCEEDED_TO_OUTPUT) ||
            (file_size_control.state == PASS_1_SUCCEEDED) );
}

/*========================================================================
FUNCTION        jpegfsc_is_succeed

DESCRIPTION     Check Pass is fail or not

DEPENDENCIES    None

RETURN VALUE    BOOLEAN

SIDE EFFECTS    None
========================================================================*/
int jpegfsc_is_succeed (jpege_file_size_control_t  file_size_control)
{
    return (file_size_control.state != FILE_SIZE_CONTROL_FAILURE);
}

/*========================================================================
FUNCTION        jpegfsc_update_output_size

DESCRIPTION     Update Curr FileSize

DEPENDENCIES    None

RETURN VALUE    AEEResult

SIDE EFFECTS    None
========================================================================*/
void jpegfsc_update_output_size(jpege_file_size_control_t *p_file_size_control,
                                uint32_t curr_buffer_size)
{
     p_file_size_control->curr_filesize += curr_buffer_size;
     if (p_file_size_control->curr_filesize > curr_buffer_size)
     {
         p_file_size_control->overflow_flag = true;
     }
}

/*========================================================================
FUNCTION        jpegfsc_estimate_target_quality

DESCRIPTION     Estimate target quality with a given image resolution and
                target file size

DEPENDENCIES    None

RETURN VALUE    AEEResult

SIDE EFFECTS    None
========================================================================*/
int jpegfsc_estimate_target_quality (jpege_file_size_control_t *p_file_size_control,
                                     uint32_t width,
                                     uint32_t height,
                                     uint32_t thumbnail_size,
                                     jpeg_subsampling_t subsampling)
{
    uint32_t total_pixels;
    pass_info_t *p_curr_pass;

    /*------------------------------------------------------------
      First, convert max quality percent to scale factor:

      Apply this formula:
      if quality is > 50, scale factor = 50 / (100 - quality%)
      else ,              scale factor = quality% / 50
    ------------------------------------------------------------*/
    if (p_file_size_control->max_quality > 50)
    {
        p_file_size_control->max_scale_factor_Q8 =
            (uint32_t) (256.0 * (50.0 / (float) (100 - p_file_size_control->max_quality)));
                                                    // Q8 format
    }
    else
    {
        p_file_size_control->max_scale_factor_Q8 =
            (uint32_t) (256.0 * ((float) p_file_size_control->max_quality / 50.0));
                                                    // Q8 format
    }

    /*------------------------------------------------------------
      Calculate the total pixel count of Luma
    ------------------------------------------------------------*/
    total_pixels = width * height;
    total_pixels += thumbnail_size;

    /*------------------------------------------------------------
      Calculate number of 8x8 blocks = total_pixels / 64
    ------------------------------------------------------------*/
    p_file_size_control->num_blocks = ((total_pixels + 63) >> 6);

    /*------------------------------------------------------------
      Estimate the number of EOB bytes for the entire image
       EOB is 4 bits
       EOB_BITS_PER_H2V2_BLOCK 6
       EOB_BITS_PER_H2V1_OR_H1V2_BLOCK 8
       EOB_BITS_PER_H1V1_BLOCK 12
      right shift 3 is to divide 8 for bits to bytes conversion
    ------------------------------------------------------------*/
    if (subsampling == JPEG_H2V2)
    {
      p_file_size_control->estnum_bytes_EOB = (uint32_t)
          ((EOB_BITS_PER_H2V2_BLOCK * p_file_size_control->num_blocks) >> 3);
    }
    else if (subsampling == JPEG_H1V1)
    {
      p_file_size_control->estnum_bytes_EOB = (uint32_t)
          ((EOB_BITS_PER_H1V1_BLOCK * p_file_size_control->num_blocks) >> 3);
    }
    else // H1V2 or H2V1
    {
      p_file_size_control->estnum_bytes_EOB = (uint32_t)
          ((EOB_BITS_PER_H2V1_OR_H1V2_BLOCK * p_file_size_control->num_blocks) >> 3);
    }

    p_curr_pass = &(p_file_size_control->pass_info[0]);

    /*------------------------------------------------------------
      Estimate number of bytes for DC based on 50% quality factor
      SCALE_FACTOR_50_PERCENT_Q8 = 256
    ------------------------------------------------------------*/
    p_file_size_control->estnum_bytes_DC
        = jpegfsc_estimate_dcsize (p_file_size_control->num_blocks,
                                   subsampling);

    // Calculate the estimated AC bytes.
    p_curr_pass->estnum_bytes_AC = p_file_size_control->scaled_target_filesize
                               - p_file_size_control->estnum_bytes_EOB
                               - p_file_size_control->estnum_bytes_DC
                               - DEFAULT_HEADER_SIZE;

    // If the AC Byte Budget is less or equal to zero
    // return error.
    if (p_curr_pass->estnum_bytes_AC <= 0)
    {
        JPEG_DBG_MED("File Size Control Failed: Requested file size too small\n");
        return JPEGERR_EFAILED;
    }

    // Calculate the scale factor & quality based on above AC Byte Budget
    p_file_size_control->target_scale_factor_Q8 = jpegfsc_calculate_quant_scale_factor (p_curr_pass->estnum_bytes_AC,
                               p_file_size_control->num_blocks,
                               &(p_curr_pass->scale_factor_Q8),
                               &(p_curr_pass->quality_percent),
                               &(p_curr_pass->est_ACbits_per_pix_Q8),
                               p_file_size_control->max_scale_factor_Q8);

    // If the calculated quality percent is 0, return error.
    if (p_curr_pass->quality_percent == 0)
    {
        JPEG_DBG_MED("File Size Control Failed: Estimated Quality Percent is 0\n");
        return JPEGERR_EFAILED;
    }

    return JPEGERR_SUCCESS;
}

/*========================================================================
FUNCTION        jpegfsc_estimate_dcsize

DESCRIPTION     Estimate bitstream size for DC component based on Quant Scale
                factor and the size of image

DEPENDENCIES    None

RETURN VALUE    Estimated number of DC bytes in image

SIDE EFFECTS    None
========================================================================*/
uint32_t jpegfsc_estimate_dcsize (uint32_t blocks,
                                  jpeg_subsampling_t subsampling)
{
    uint32_t DC_bits_luma_count_Q8, DC_bits_block_count_Q8;
    uint32_t total_DC_bytes;

    /*------------------------------------------------------------
      Calculate number of DC bits per 8x8 Luma Block
      With the estimation of 4 bits
    ------------------------------------------------------------*/
    DC_bits_luma_count_Q8 = 256 * 4 ;

    // Calculate number of DC bits for 8x8 luma
    // and associated chroma
    if (subsampling == JPEG_H2V2)
    {
        // * 1.5
        DC_bits_block_count_Q8 = (uint32_t)
            (DC_bits_luma_count_Q8 + (DC_bits_luma_count_Q8 >> 1));
    }
    else if (subsampling == JPEG_H1V1)
    {
        DC_bits_block_count_Q8 = (uint32_t) (DC_bits_luma_count_Q8 * 3);
    }
    else
    {
        DC_bits_block_count_Q8 = (uint32_t) (DC_bits_luma_count_Q8 * 2);
    }

    /*------------------------------------------------------------
      Calculate total DC bytes in image.
      right shift >> 3 for bits to bytes
      right shift >> 8 for Q8 to Q0 conversion.
    ------------------------------------------------------------*/
    total_DC_bytes = (DC_bits_block_count_Q8 * blocks) >> 11;

    return (total_DC_bytes);
}

/*========================================================================
FUNCTION        jpegfsc_calculate_quant_scale_factor

DESCRIPTION     Estimate the Image Quality and the corresponding Scale factor
                required to achieve the target AC size

DEPENDENCIES    None

RETURN VALUES   Target Image Quality
                Target Scale Factor
                Target AC bits per pixel

SIDE EFFECTS    None
========================================================================*/
uint32_t jpegfsc_calculate_quant_scale_factor (uint32_t fileSize_AC,
                                               uint32_t blocks,
                                               uint32_t *scale_factor_Q8,
                                               uint32_t *quality_percent,
                                               uint32_t *ac_bits_per_pixel_Q8,
                                               uint32_t max_scale_factor_Q8)
{
    // Output a debug message if number of bytes allocated for AC too small
    if (fileSize_AC < MIN_NUM_BYTES_FOR_AC)
    {
        JPEG_DBG_MED("File Size Control: Number of bytes allocated for AC too small\n");
    }

    /*------------------------------------------------------------
      AC Bits per pixel = 2^8 * (8 * fileSizeAC/ (nBlocks * 64));
                            ^     ^                         ^
                            |     |                         |
                          for Q8  bytes2bits           64 pix/blk
    ------------------------------------------------------------*/
    *ac_bits_per_pixel_Q8 = 32 * fileSize_AC / blocks;

    /*------------------------------------------------------------
     Estimate Scaling factor with equation of
     Scale factor
      = 1.5825 * acBitsPerPixel + 0.6869 * (acBitsPerPixel ^ 2)
    ------------------------------------------------------------*/
    *scale_factor_Q8 = SCALE_FACTOR ((*ac_bits_per_pixel_Q8));

    /*------------------------------------------------------------
     Not encode at more than max quality.
     Limit scale factor.
    ------------------------------------------------------------*/
    if (*scale_factor_Q8 > max_scale_factor_Q8)
    {
        *scale_factor_Q8 = max_scale_factor_Q8;
    }

    // Calculate quality percent from scale factor
    if (*scale_factor_Q8 > SCALE_FACTOR_50_PERCENT_Q8)
    {
        *quality_percent = 100 - ((uint32_t) ((50 * 256) / (*scale_factor_Q8)));
    }
    else
    {
        // corresponds to less than 50% quality factor
        *quality_percent = (uint32_t) ((50 * (*scale_factor_Q8)) >> 8);
    }
    return (*scale_factor_Q8) ;
}

/*========================================================================
FUNCTION        jpegfsc_encode_done_handler

DESCRIPTION     File Size Control - End of pass N handling.

DEPENDENCIES    None

RETURN VALUES   AEEResult

SIDE EFFECTS    None
========================================================================*/
int jpegfsc_encode_done_handler (jpege_file_size_control_t  *p_file_size_control,
                                 jpege_cfg_t  *p_cfg,
                                 uint16_t  **dst_main_luma_quant_tbl,
                                 uint16_t  **dst_main_chroma_quant_tbl)
{
    pass_info_t *p_curr_pass = &(p_file_size_control->pass_info[p_file_size_control->iteration_count]);
    pass_info_t *p_next_pass;

    uint32_t actual_filesize, actual_by_target_ratio;
    uint32_t new_AC_bits_per_pix_Q8, new_scale_Q8 = 1;
    int rc;

    // Set actual size of this encoding
    actual_filesize = p_file_size_control->curr_filesize;

    /*------------------------------------------------------------
      Calculate the size ratio
      if < 100 means undershoot
      if > 100 means overshoot
    ------------------------------------------------------------*/
    actual_by_target_ratio = (uint32_t) (100.0
                                        * (float) actual_filesize
                                        / p_file_size_control->target_filesize + 0.9);

    // Reset bytes were really consumed by AC
    p_curr_pass->actual_num_bytes_AC = actual_filesize
                                    - p_file_size_control->thumbnail_size
                                    - p_file_size_control->estnum_bytes_DC
                                    - p_file_size_control->estnum_bytes_EOB
                                    - DEFAULT_HEADER_SIZE;

    /*------------------------------------------------------------
      AC Bits per pixel = 2^8 * (8 * fileSizeAC/ (nBlocks * 64));
                            ^     ^                         ^
                            |     |                         |
                          for Q8  bytes2bits           64 pix/blk
    ------------------------------------------------------------*/
    p_curr_pass->actual_ACbits_per_pix_Q8 = 32 * p_curr_pass->actual_num_bytes_AC
                                         / p_file_size_control->num_blocks ;

    // Check if we passed on this run
    if (actual_by_target_ratio <= 100)
    {
        // Undershoot. Determine if it's acceptable.
        // First, look up threshold for the encoded quality factor.
        uint32_t index = (uint32_t)
                           (p_curr_pass->quality_percent / 10);

        // If actual By Target Ratio exceeds threshold, declare success.
        // Also, if max number of passes exceeded, declare success.
        if ((actual_by_target_ratio >=
            file_size_undershoot_thresholds[index])
            || (p_file_size_control->iteration_count >= FILE_SIZE_MAX_NUM_PASSES))
        {
            p_file_size_control->state = PASS_SUCCEEDED_TO_OUTPUT;

            if ((p_file_size_control->iteration_count == 0)
                 && (!p_file_size_control->overflow_flag))
            {
                // one pass success with given output buffer
                p_file_size_control->state = PASS_1_SUCCEEDED;
            }
        }
    }

    // Update iteration counter
    p_file_size_control->iteration_count += 1;

    if ((p_file_size_control->state == PASS_SUCCEEDED_TO_OUTPUT) || (p_file_size_control->state == PASS_1_SUCCEEDED))
    {
        JPEG_DBG_MED("File Size Control Successed\n");
        return JPEGERR_SUCCESS;     // Success
    }
    else if (p_file_size_control->iteration_count >= FILE_SIZE_MAX_NUM_PASSES)
    {
        // Overshoot & max number of passes exceeded
        p_file_size_control->state = FILE_SIZE_CONTROL_FAILURE;
        JPEG_DBG_MED("File Size Control Failed with Maximum Passes\n");
        return JPEGERR_SUCCESS;
    }
    else
    {
        // Compute paremeters for next pass
        p_next_pass = &(p_file_size_control->pass_info[p_file_size_control->iteration_count]);

        // Actual file size is greater than the target size
        if (p_curr_pass->est_ACbits_per_pix_Q8 < p_curr_pass->actual_ACbits_per_pix_Q8)
        {
            // First make sure there is room to scale down
            if (p_curr_pass->scale_factor_Q8 <= SCALE_FACTOR_1_PER_Q8)
            {
                // Scale factor for previous pass is already lowest
                // so do NOT  attempt another pass, just return here
                p_file_size_control->state = FILE_SIZE_CONTROL_FAILURE;
                JPEG_DBG_MED("File Size Control Failed with Lowest Scale Factor\n");
                return JPEGERR_SUCCESS;
            }

            /*------------------------------------------------------------
            update bits per pixel
            --------------------------------------------------------------*/
            new_AC_bits_per_pix_Q8 = p_curr_pass->actual_ACbits_per_pix_Q8 + (uint32_t)
            (( p_curr_pass->actual_ACbits_per_pix_Q8 - p_curr_pass->est_ACbits_per_pix_Q8) >> 3 );

            /*------------------------------------------------------------
            Now apply new BPP from above to get a mult factor (K) for the
            next scale factor
            Scale factor
            = 1.5825 * acBitsPerPixel + 0.6869 * (acBitsPerPixel ^ 2)
            --------------------------------------------------------------*/
            new_scale_Q8 = SCALE_FACTOR(new_AC_bits_per_pix_Q8 );

        }// end of overshoot
        else
        {

            // If undershot by too much, adjust quality up and encode again

            // First make sure there is room to scale up
            if (p_curr_pass->scale_factor_Q8 >= p_file_size_control->max_scale_factor_Q8)
            {
                /*------------------------------------------------------------
                 Scale factor for current pass is already highest, so do NOT
                 attempt another pass, just return here
                 This time we are successful
                --------------------------------------------------------------*/
                p_file_size_control->state = PASS_SUCCEEDED_TO_OUTPUT;

                return JPEGERR_SUCCESS;
            }
            else
            {
                /*------------------------------------------------------------
                 Compute new scale factor
                 Scale factor
                 = 1.5825 * acBitsPerPixel + 0.6869 * (acBitsPerPixel ^ 2)
                --------------------------------------------------------------*/
                new_scale_Q8 = SCALE_FACTOR(p_curr_pass->actual_ACbits_per_pix_Q8);

                // Limit scale
                new_scale_Q8 = p_curr_pass->scale_factor_Q8
                             + (uint32_t) ((new_scale_Q8
                             - p_curr_pass->scale_factor_Q8) >> 2);
            }
        }// end of undershoot

        /*------------------------------------------------------------
          Based on the new scale factor, adjust the old one here
          Scale the estimated scale factor in the current pass by K
          to form the new scale factor for the next pass
        --------------------------------------------------------------*/
        p_next_pass->scale_factor_Q8 = (uint32_t) (p_curr_pass->scale_factor_Q8
                                             * p_file_size_control->target_scale_factor_Q8
                                             / new_scale_Q8);

        /*------------------------------------------------------------
          MAX_K_FACTOR_VALUE = 2
          2 is the upper limit on the change in the scale factor
          from pass to pass.
        --------------------------------------------------------------*/
        if (p_next_pass->scale_factor_Q8 > p_curr_pass->scale_factor_Q8 * MAX_K_FACTOR_VALUE)
        {
            JPEG_DBG_MED("File Size Control Failed with Capping K Factor to max acceptable\n");
            p_next_pass->scale_factor_Q8 = p_curr_pass->scale_factor_Q8 * MAX_K_FACTOR_VALUE;
        }

        // Convert from Q8 scale factor to quality percent
        if (p_next_pass->scale_factor_Q8 > SCALE_FACTOR_50_PERCENT_Q8)
        {
            // for > 50% quality
            p_next_pass->quality_percent = 100 -
                        ((uint32_t) ((50 * 256) / p_next_pass->scale_factor_Q8 ));
        }
        else
        {
            // for <= 50% quality
            p_next_pass->quality_percent = (uint32_t) ((50 * p_next_pass->scale_factor_Q8 ) >>8);
        }

        // Reset of the bytes were really consumed by AC for next pass
        p_next_pass->estnum_bytes_AC = (uint32_t) (p_file_size_control->scaled_target_filesize
                                              - p_file_size_control->thumbnail_size
                                              - p_file_size_control->estnum_bytes_EOB
                                              - p_file_size_control->estnum_bytes_DC
                                              - DEFAULT_HEADER_SIZE);

        p_next_pass->est_ACbits_per_pix_Q8 = (32 * p_next_pass->estnum_bytes_AC) / p_file_size_control->num_blocks;

        // Update state
        p_file_size_control->state = PASS_ENCODING;

        // Scale quantization table according to the quality factors
        if (p_next_pass->scale_factor_Q8)
        {
            // Main luma
            rc = jpegfsc_scale_quant_table (p_next_pass->scale_factor_Q8,
                                            p_curr_pass->scale_factor_Q8,
                                            *dst_main_luma_quant_tbl,
                                            dst_main_luma_quant_tbl);
            p_cfg->main_cfg.luma_quant_tbl = *dst_main_luma_quant_tbl;
            // Main chroma
            rc = jpegfsc_scale_quant_table (p_next_pass->scale_factor_Q8,
                                            p_curr_pass->scale_factor_Q8,
                                            *dst_main_chroma_quant_tbl,
                                            dst_main_chroma_quant_tbl);
            p_cfg->main_cfg.chroma_quant_tbl = *dst_main_chroma_quant_tbl;
            if (JPEG_FAILED(rc))return rc;
        }
        else
        {
            p_file_size_control->state = FILE_SIZE_CONTROL_FAILURE;
            JPEG_DBG_MED("File Size Control Failed with Scale Factor\n");
            return JPEGERR_SUCCESS;
        }

        p_file_size_control->curr_filesize = 0;
        return JPEGERR_SUCCESS;
    }// end of for another pass
}

/*===========================================================================
FUNCTION        jpegfsc_scale_quant_table

DESCRIPTION     Scale the quantization table

DEPENDENCIES    None

RETURN VALUE    AEEResult

SIDE EFFECTS    None
===========================================================================*/
int jpegfsc_scale_quant_table (uint32_t scale_factor,
                               uint32_t old_scale_factor,
                               uint16_t *src_quant_tbl,
                               uint16_t **dst_quant_tbl)
{
    int i;
    uint16_t *dst_qtable = *dst_quant_tbl;

    // Allocate destination table if not yet allocated
    if (!dst_qtable)
    {
        dst_qtable = (uint16_t *)JPEG_MALLOC(64 * sizeof(uint16_t));
        if (!dst_qtable) return JPEGERR_EFAILED;
    }

    // Scale quant entries
    for (i = 0; i < 64; i++)
    {
        /*------------------------------------------------------------
          Compute new value based on input percent and on the 50% table (low)
          Add 0.5 after the divide to round up fractional divides to be
          more conservative.
        --------------------------------------------------------------*/
        dst_qtable[i] = (uint16_t) (((double)src_quant_tbl[i] * old_scale_factor
                                             / scale_factor) + 0.5);
        // Clamp
        CLAMP (dst_qtable[i], 1, 255);
    }
    *dst_quant_tbl = dst_qtable;
    return JPEGERR_SUCCESS;
}

/*===========================================================================
FUNCTION        jpegfsc_init_quant_tables

DESCRIPTION     Init the quantization tables

DEPENDENCIES    None

RETURN VALUE    AEEResult

SIDE EFFECTS    None
===========================================================================*/
int jpegfsc_init_quant_tables (jpege_file_size_control_t  file_size_control,
                               jpege_img_cfg_t *p_cfg,
                               uint16_t **dst_luma_quant_tbl,
                               uint16_t *src_luma_quant_tbl,
                               uint16_t **dst_chroma_quant_tbl,
                               uint16_t *src_chroma_quant_tbl)
{
    int rc;
    uint32_t old_quality_factor_Q8;
    uint32_t quality_factor_Q8 = file_size_control.pass_info[file_size_control.iteration_count].scale_factor_Q8;
    // Default Quant Table with Quality FactorQ8 = 256
    old_quality_factor_Q8 = 256;
    // Main Luma
    rc = jpegfsc_scale_quant_table (quality_factor_Q8, old_quality_factor_Q8, src_luma_quant_tbl, dst_luma_quant_tbl);
    if (JPEG_FAILED(rc))  return rc;
    // Main chroma
    rc = jpegfsc_scale_quant_table (quality_factor_Q8, old_quality_factor_Q8, src_chroma_quant_tbl, dst_chroma_quant_tbl);
    if (JPEG_FAILED(rc))  return rc;

    p_cfg->luma_quant_tbl = *dst_luma_quant_tbl;
    p_cfg->chroma_quant_tbl = *dst_chroma_quant_tbl;
    return JPEGERR_SUCCESS;
}

