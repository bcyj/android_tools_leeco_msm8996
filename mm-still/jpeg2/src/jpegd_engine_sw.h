/*========================================================================


*//** @file jpegd_engine_sw.h

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
12/10/09   vma     Removed the dependency on os_thread_detach by joining
                   the decode thread properly.
10/22/09   vma     Export the engine 'profile' for easier engine picking.
10/15/09   mingy   Modified iDCT function signature.
09/21/09   mingy   Added new variables and Macro for region decoding.
08/03/09   vma     Switched to use the os abstraction layer (os_*)
07/20/09   zhiminl Added support for progressive decoding.
07/07/09   zhiminl Decoupled bit buffer from jpegd_engine_input_fetcher_t.
05/12/09   mingy   Modified the Huffman table structure.
07/07/08   vma     Created file.

========================================================================== */

#ifndef _JPEGD_ENGINE_SW_H
#define _JPEGD_ENGINE_SW_H

#include "os_thread.h"
#include "jpeg_common.h"
#include "jpegd_engine.h"
#include "jpegd.h"
#include "jpegd_engine_utils.h"

/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
/** ==============================================================
 * JPEG defined maximum numbe of blocks per MCU
 * =============================================================*/
#define JPEGD_MAXBLOCKSPERMCU   10
/** ==============================================================
 * JPEG defined maximum number of huffman and quant table
 * and components
 * =============================================================*/
#define JPEGD_MAXHUFFTABLES     8
#define JPEGD_MAXQUANTTABLES    4
#define JPEGD_MAXCOMPONENTS     4
#define JPEGD_MAXCOMPSINSCAN    4

/** ==============================================================
 * Limitations that specific to this decode implementation and can
 * be changed at compile time.
 * =============================================================*/
#define JPEGD_MAXBLOCKSPERROW        6144
#define JPEGD_MAXBLOCKS              100
#define JPEGD_MAX_HEIGHT             8192
#define JPEGD_MAX_WIDTH              8192
#define JPEGD_DCTSIZE                8
#define JPEGD_DCTSIZE2               64
#define JPEGD_MAX_BITSTREABUF_SIZE 1024
#define JPEGD_BIT_BUFFER_LENGTH      32
#define JPEGD_BIT_BUFFER_LENGTH_HALF 16

/** ==============================================================
 * Define Roundings
 * =============================================================*/
#define ROUND_TO_16(x)         ((((x) + 15) >> 4) << 4)
#define ROUND_TO_8(x)          ((((x) + 7)  >> 3) << 3)

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef struct
{
    uint16_t len;
    uint16_t cat;
}jpegd_derived_htable_entry_t;

typedef struct
{
    uint32_t nCategoryCodeSize[256];
    jpegd_derived_htable_entry_t symbol[512];

} jpegd_derived_htable_t;

typedef struct
{
  // Bit accumulator
  uint32_t  bit_accumulator;
  // Bits left
  int32_t   bits_left;
} jpegd_bitbuffer_t;

// progressive scan info for current scan
typedef struct
{
    // Start of spectrum selection
    uint32_t ss;
    // End of spectral selection
    uint32_t se;
    // Successive approximation bit position low
    uint32_t al;
    // Successive approximation bit position high
    uint32_t ah;
} jpegd_progressive_info_t;

typedef int16_t   jpegd_coeff_t;
typedef int16_t   jpegd_sample_t;
typedef uint8_t   jpegd_pixel_t;

typedef struct jpegd_engine_sw_struct jpegd_engine_sw_t;

// IDCT function type
typedef void (*jpegd_engine_sw_idct_t)(jpegd_coeff_t *input,
                                       jpegd_sample_t *output,
                                       int32_t);
// MCU output function type
typedef void (*jpegd_engine_sw_output_mcu_t)(jpegd_engine_sw_t *p_engine);

struct jpegd_engine_sw_struct
{
    // The wrapper engine object
    jpegd_engine_obj_t  *p_wrapper;
    // os mutex object
    os_mutex_t           mutex;
    // os condition variable
    os_cond_t            cond;
    // os decode thread object
    os_thread_t          decode_thread;
    // The source object
    jpegd_engine_src_t   source;
    // The destination object
    jpegd_engine_dst_t   dest;
    // The event handler
    jpegd_engine_event_handler_t  p_event_handler;
    // The input fetch helper
    jpegd_engine_input_fetcher_t  input_fetcher;
    // Index of the destination buffer in use
    uint32_t             curr_dest_buf_index;
    // IDCT function pointer
    jpegd_engine_sw_idct_t  idct_func;
    // The MCU output function
    jpegd_engine_sw_output_mcu_t  mcu_output_func;

    int32_t              numOfValidCoeff;

    // Input buffer flag
    int8_t               fPaddingFlag;
    int8_t               fIsPadded;
    int8_t               fInputError;

    // Input bit buffer
    jpegd_bitbuffer_t    bitbuffer;

    // Flag indicating whether the engine is actively decoding or not
    uint8_t              is_active;
    // Abort flag
    uint8_t              abort_flag;
    // Flag indicating whether the decode thread needs to be joined
    uint8_t              is_dthread_need_join;
    // Image format and dimensions
    jpeg_subsampling_t   jpegdFormat;
    uint32_t             nImageWidth;
    uint32_t             nImageHeight;

    // MCU per row
    uint32_t nMCUPerRow;
    // MCU per column
    uint32_t nMCUPerCol;

    // Restart marker information
    int32_t nRestartLeft;
    int32_t nRestartInterval;
    uint8_t nNextRestartNum;
    // Image dimensions in MCU after padding.
    uint32_t nBlocksPerMCU;

    // Number of components in current scan
    uint8_t nNumberOfComponentsInScan;

    // Order of MCU blocks in each scan and components
    uint8_t MCUBlockCompList[JPEGD_MAXBLOCKSPERMCU];
    uint8_t compListInScan[JPEGD_MAXCOMPSINSCAN];

    // Index for components id, huff table, quant table and dc val
    uint8_t compId[JPEGD_MAXCOMPONENTS];
    uint8_t quantId[JPEGD_MAXCOMPONENTS];
    uint8_t dcHuffTableId[JPEGD_MAXCOMPSINSCAN];
    uint8_t acHuffTableId[JPEGD_MAXCOMPSINSCAN];
    int32_t lastDcVal[JPEGD_MAXBLOCKSPERMCU];

    // Output related
    uint32_t nOutputRowIncrY;
    uint32_t nOutputRowIncrCbCr;

    uint32_t nOutputBufferSizeY;
    uint32_t nOutputBufferSizeCbCr;

    // Region decoding MCU number count
    uint32_t left_mcu_num;
    uint32_t top_mcu_num;
    uint32_t right_mcu_num;
    uint32_t bottom_mcu_num;
    uint32_t region_start_mcu_id;
    uint32_t region_end_mcu_id;

    // Huff table and quant table local storage
    jpegd_derived_htable_t * pDerivedHuffTable[JPEGD_MAXHUFFTABLES];
    jpeg_quant_table_t       pDeQuantTable[JPEGD_MAXQUANTTABLES];

    // Internal buffers for coefficient block and sample block and lines
    jpegd_coeff_t    pCoeffBlockBuf[JPEGD_DCTSIZE2];
    jpegd_sample_t * pSampleMCUBuf[JPEGD_MAXBLOCKSPERMCU];

    jpegd_pixel_t *  pMCULineBufPtrY;
    jpegd_pixel_t *  pMCULineBufPtrCbCr;

    jpegd_pixel_t *  pOutputBufferY;
    jpegd_pixel_t *  pOutputBufferCbCr;

    // Output parameter for scalable engine test
    uint32_t nOutputWidth;
    uint32_t nOutputHeight;

    uint32_t nYLinesPerMCU;
    uint32_t nCbCrLinesPerMCU;

    // Resize factor for IDCT
    int8_t   nResizeFactor;

    // Number of components in frame
    uint8_t  nNumberOfComponentsInFrame;

    // Sampling info of components in frame
    uint8_t  hSample[JPEGD_MAXCOMPONENTS];
    uint8_t  vSample[JPEGD_MAXCOMPONENTS];
    uint8_t  maxHSample;
    uint8_t  maxVSample;

    // Progressive current scan id
    uint8_t  currentScanId;

    // Progressive current scan info
    jpegd_progressive_info_t currentScanProgressiveInfo;

    // Progressive current scan component entropy selectors
    jpeg_comp_entropy_sel_t  compEntropySelectors[JPEGD_MAXCOMPONENTS];

    // Progressive current scan EOBRUN
    uint32_t EOBRUN;

    // Progressive coefficient buffer per component for the full image
    jpegd_coeff_t *pCompCoeffBuf[JPEGD_MAXCOMPONENTS];

    // Progressive number of horizontal coefficient blocks per component
    uint32_t compCoeffHBlocks[JPEGD_MAXCOMPONENTS];
    // Progressive number of vertical coefficient blocks per component
    uint32_t compCoeffVBlocks[JPEGD_MAXCOMPONENTS];

    // Progressive coefficient block buffer pointers per MCU, which is
    // the one to one mapping to MCUBlockCompList[]
    jpegd_coeff_t *pMCUCoeffBlockBuf[JPEGD_MAXBLOCKSPERMCU];

    // Flag indicating whether the image is progressive or not
    uint8_t  is_progressive;

    // Configured flag
    uint8_t  is_configured;
};

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */

/* =======================================================================
**                          Function Declarations
** ======================================================================= */
extern jpegd_engine_profile_t jpegd_engine_sw_profile;

#endif /* _JPEGD_ENGINE_SW_H */
