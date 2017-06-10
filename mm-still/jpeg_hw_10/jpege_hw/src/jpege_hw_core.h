/*============================================================================

   Copyright (c) 2012 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.

============================================================================*/

#ifndef JPEGE_HW_CORE_H
#define JPEGE_HW_CORE_H

#include <jpege_lib_common.h>

/*  according to Gemini HDD */
/*  Gemini core clock can run up to 160 MHz for Halcyon */
/*  to meet the requirement of 12MP@10fps */
/*  For BB/PT, it is 200MHz for 12MP@15fps */
#define GEMINI_JPEG_FREQUENCY_MAX           160

/*  queue length is set to 16 */
#define GEMINI_NUM_QUEUELENGTH              16

/*  max number of MCUs for frame width and height */
/*  ref SWI section 2.1.1.5 */
#define GEMINI_FRAME_WIDTH_MCUS_MAX         512
#define GEMINI_FRAME_HEIGHT_MCUS_MAX        512
#define GEMINI_FRAME_WIDTH_MCUS_MIN         1
#define GEMINI_FRAME_HEIGHT_MCUS_MIN        1

/*  #define GEMINI_FE_BURST_LENGTH4             1 */
/*  #define GEMINI_FE_BURST_LENGTH8             2 */

#define GEMINI_FE_CRCB_ORDER                0
#define GEMINI_FE_CBCR_ORDER                1

/*  #define GEMINI_WE_BURST_LENGTH4             1 */
/*  #define GEMINI_WE_BURST_LENGTH8             2 */

#define GEMINI_BYTEORDERING_MIN             0
#define GEMINI_BYTEORDERING_MAX             7

#define GEMINI_DC_COMPONENT_INDEX           0
#define GEMINI_AC_COMPONENT_INDEX           1

#define GEMINI_NUM_QUANTIZATION_ENTRIES     64
#define GEMINI_NUM_HUFFMANDC_ENTRIES        12
#define GEMINI_NUM_HUFFMANAC_ENTRIES        176


typedef struct
{
  JPEGE_HW_INPUT_FORMAT eInputFormat;
  uint32_t nReadBurstLength;
  uint32_t nFrameWidthMCUs;
  uint32_t nFrameHeightMCUs;
} gemini_fe_operation_cfg_type;

#endif /*  JPEGE_HW_CORE_H */
