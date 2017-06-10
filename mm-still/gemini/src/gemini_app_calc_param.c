/*========================================================================

   @file gemini_app_calc_param.c

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2009-2010 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
01/11/10   sw      change to linux type definition AND return value
11/30/09   sw      configure gemini parameter:
                   1) restart interval
                   2) disable customized huffman tables
                   3) luma and chroma quant tables
                   4) gemini file size control
                      - calculate regionSize
                      - calculate each region budgets
11/23/09   sw      Created file

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "gemini_app_calc_param.h"
#include "../src/gemini_dbg.h"

/* -----------------------------------------------------------------------
** Local Object Definitions
** ----------------------------------------------------------------------- */
/* Static constants */
/* Standard Quantization Tables */
static const uint8_t standard_luma_q_tbl[] = {
	16, 11, 10, 16, 24, 40, 51, 61,
	12, 12, 14, 19, 26, 58, 60, 55,
	14, 13, 16, 24, 40, 57, 69, 56,
	14, 17, 22, 29, 51, 87, 80, 62,
	18, 22, 37, 56, 68, 109, 103, 77,
	24, 35, 55, 64, 81, 104, 113, 92,
	49, 64, 78, 87, 103, 121, 120, 101,
	72, 92, 95, 98, 112, 100, 103, 99
};

static const uint8_t standard_chroma_q_tbl[] = {
	17, 18, 24, 47, 99, 99, 99, 99,
	18, 21, 26, 66, 99, 99, 99, 99,
	24, 26, 56, 99, 99, 99, 99, 99,
	47, 66, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99
};

/* -----------------------------------------------------------------------
** Quantity bits target rate, minimum rate and total budget
** ----------------------------------------------------------------------- */
#define FSC_Q_BITS 12

/* -----------------------------------------------------------------------
** Region numbers
** ----------------------------------------------------------------------- */
#define REGION_NUM 16
#define REGION_NUM_IN_BITS 4

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
/*  return camera error codes */
static int scale_quant_table (gemini_quantTable * dst_quant_tbl,
			      const uint8_t * src_quant_tbl,
			      uint32_t quality_factor);
/* -----------------------------------------------------------------------
** Global Object Definitions
** ----------------------------------------------------------------------- */

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
/*  #define CLAMP_(x,min,max)           {if ((x) < (min)) (x) = (min); */
/*                                       if ((x) > (max)) (x) = (max);} */

#define CLAMP_(x,min,max)       ( ( ((int)x)<(min) ? (min) : ( ((int)x)>(max)?(max):(x) ) ) )
#define X_CLAMP_(x,min,max)       ( (x) = CLAMP_(x,min,max) )
/* =======================================================================
**                          Function Definitions
** ======================================================================= */
/*===========================================================================
FUNCTION        gemini_app_calc_param

DESCRIPTION     configure gemini parameter:
                1) restart interval
                2) disable customized huffman tables
                3) luma and chroma quant tables
                4) gemini file size control
                   - calculate regionSize
                   - calculate each region budgets

DEPENDENCIES    None

RETURN VALUE    0 - SUCCESS
                1 - FAILURE

SIDE EFFECTS    None
===========================================================================*/
int gemini_app_calc_param (gemini_cmd_jpeg_encode_cfg * p_cfg, gemini_app_img_src src)
{
	int32_t rc, i;

	/*
	 * bit budget for each mcu regions
	 */
	uint8_t mcu_budget[REGION_NUM];

	/*
	 * target rate in FSC_Q_BITS
	 */
	uint32_t target_rate;

	/*
	 * minimum rate in FSC_Q_BITS
	 */
	uint32_t minimum_rate = 0;

	uint32_t num_of_mcu_rows;

	/*
	 * rate in FSC_Q_BITS
	 */
	uint32_t current_rate;

	/*
	 * total bits budget in FSC_Q_BITS
	 */
	uint32_t total_budget;

	uint32_t region_size;

	uint32_t total_pixel;

    /*-----------------------------------------------------------------------
      1. Configure number of MCU between restart marker insertion
    -----------------------------------------------------------------------*/
	p_cfg->restartInterval =
		(uint16_t) ((src.image_width + (1 << (src.h_sampling + 2)) - 1)
			    >> (src.h_sampling + 2));

    /*-----------------------------------------------------------------------
      2. Configure NOT use customized Huffman table for JPEG processing (= 0x0) 
    -----------------------------------------------------------------------*/
	p_cfg->bCustomHuffmanTbl = 0;

	/*
	 * NOT valid when customized huffman table is NOT selected
	 */
	p_cfg->huffmanTblYDcPtr = NULL;
	p_cfg->huffmanTblYAcPtr = NULL;
	p_cfg->huffmanTblCbcrDcPtr = NULL;
	p_cfg->huffmanTblCbcrAcPtr = NULL;

    /*-----------------------------------------------------------------------
      3. Configure Quant tables for Luma and Chroma: 64 entries.
         Gemini Driver to convert to 1/Q, then program the hardware.
    -----------------------------------------------------------------------*/
		/*
		 * Scale quant tables
		 */
		rc = scale_quant_table (p_cfg->quantTblY,
					standard_luma_q_tbl, src.quality);
	rc = scale_quant_table (p_cfg->quantTblChroma,
				standard_chroma_q_tbl, src.quality);
	if (rc) {
		/*
		 * Failed with configuration
		 */
		return 1;
	}

    /*-----------------------------------------------------------------------
      4. Configure file size control
    -----------------------------------------------------------------------*/
	total_pixel = src.image_width * src.image_height;

	target_rate = (uint32_t) ((src.buffer_size << (FSC_Q_BITS + 3))
				  / (total_pixel * 1.1417));

	/*
	 * number of mcu rows is equal to
	 */
	/*
	 * (image height + 15) / 16 for H2V2/H1V2
	 */
	/*
	 * or    (image height + 7)  / 8  for H2V1/H1V1
	 */
	num_of_mcu_rows =
		(uint32_t) ((src.image_height + (1 << (src.v_sampling + 2)) - 1)
			    >> (src.v_sampling + 2));

	total_budget = target_rate * num_of_mcu_rows;

    /*-----------------------------------------------------------------------
      image is divided into vertical regions
        with number of REGION_NUM = 16      
      region size is equal to total number of mcu rows
        divide by vertical region numbers.
      Rifht shift of REGION_NUM_IN_BITS is used instead of division.
    -----------------------------------------------------------------------*/
	region_size = (num_of_mcu_rows + REGION_NUM - 1) >> REGION_NUM_IN_BITS;

    /*-----------------------------------------------------------------------
       the number of MCU rows for each region.
       If there are N MCU rows per region,
       this register should be programmed as N-1.
       This implies that there is a minimum setting on 1 MCU row per region.
    -----------------------------------------------------------------------*/
	p_cfg->sFileSizeControl.regionSize = region_size - 1;

    /*-----------------------------------------------------------------------
       Calculate bit budget for regions.
    -----------------------------------------------------------------------*/
	/*
	 * Initialization zero
	 */
	for (i = 0; i < (REGION_NUM - 1); i++) {
		mcu_budget[i] = 0;
	}

	for (i = 0;
	     i < (REGION_NUM - 1) && num_of_mcu_rows > region_size &&
	     total_budget; i++) {
	/*-----------------------------------------------------------------------
          * (h_samp_factor * v_samp_factor)
          = * (4 = 2^2 / 2 = 2^1 / 1 = 2^0)
          = << (h_samp_factor + v_samp_factor - 2)
        -----------------------------------------------------------------------*/
		mcu_budget[i] =
			(uint8_t) CLAMP_ ((int) ((2 * target_rate - minimum_rate)
					   >> (FSC_Q_BITS - src.h_sampling -
					       src.v_sampling)), 0, 255);

		/*
		 * current rate in FSC_Q_BITS
		 */
		current_rate =
			(mcu_budget[i] <<
			 (FSC_Q_BITS - src.h_sampling - src.v_sampling));

		num_of_mcu_rows -= region_size;

		/*
		 * total budget in FSC_Q_BITS
		 */
		if (total_budget > current_rate * region_size) {
			total_budget -= current_rate * region_size;
		} else {
			total_budget = 0;
		}

		/*
		 * target rate in FSC_Q_BITS
		 */
		target_rate = (uint32_t) (total_budget / num_of_mcu_rows);
	}

	if ((int) target_rate >= 0) {
		mcu_budget[i] =
			(uint8_t) CLAMP_ ((int) ((2 * target_rate - minimum_rate)
					   >> (FSC_Q_BITS - src.h_sampling -
					       src.v_sampling)), 0, 255);
	}
	/*
	 * return SUCCESS
	 */
	return 0;
}

/*===========================================================================
FUNCTION        scale_quant_table

DEPENDENCIES    None

RETURN VALUE    0 - SUCCESS
                1 - FAILURE

SIDE EFFECTS    None
===========================================================================*/
static int scale_quant_table (gemini_quantTable * dst_quant_tbl,
			      const uint8_t * src_quant_tbl,
			      uint32_t quality_factor)
{
	int i;
	double scale_factor;
        int tmp;

	/*
	 * return FAILURE if not yet allocated
	 */
	if (!dst_quant_tbl->qtbl) {
		return 1;
	}

	/*
	 * 50% is equalivalent to no scaling
	 */
	if (quality_factor == 50) {
		/*
		 * copy and quant tables;
		 */
		for (i = 0; i < 64; i++) {
			dst_quant_tbl->qtbl[i] = (uint8_t) (src_quant_tbl[i]);

			/*
			 * Clamp
			 */
                        tmp = 0;
                        tmp = (int)dst_quant_tbl->qtbl[i];
			X_CLAMP_ (tmp, 1, 255);
		}
		/*
		 * return SUCCESS
		 */
		return 0;
	}

	X_CLAMP_ (quality_factor, 1, 98);

	/*
	 * Turn quality percent into scale factor
	 */
	if (quality_factor > 50) {
		scale_factor = 50.0 / (float) (100 - quality_factor);
	} else {
		scale_factor = (float) quality_factor / 50.0;
	}

	/*
	 * Scale quant entries
	 */
	uint32_t temp_q = 0;
	for (i = 0; i < 64; i++) {
		/*
		 * Compute new value based on input percent
		 */
		/*
		 * and on the 50% table (low)
		 */
		/*
		 * Add 0.5 after the divide to round up fractional divides to be
		 */
		/*
		 * more conservative.
		 */
		temp_q = (uint32_t) (((double) src_quant_tbl[i] / scale_factor) +
				   0.5);
		if (temp_q > 255)
		  GMN_PR_ERR("Qtable[%d] = %d", i, temp_q);
		dst_quant_tbl->qtbl[i] = X_CLAMP_(temp_q, 1, 255);

		/*
		 * Clamp
		 */
	}
	/*
	 * return SUCCESS
	 */
	return 0;
}
