/*========================================================================

 @file gemini_app_calc_param.h

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
#include "gemini_lib_common.h"

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef struct
{
	uint32_t buffer_size;	/*  in Bytes */
	uint32_t quality;
	uint32_t image_width;
	uint32_t image_height;
	uint32_t h_sampling;
	uint32_t v_sampling;

} gemini_app_img_src;

/* -----------------------------------------------------------------------
** Forward Declarations
** ----------------------------------------------------------------------- */
int gemini_app_calc_param (gemini_cmd_jpeg_encode_cfg * p_cfg, gemini_app_img_src src);
