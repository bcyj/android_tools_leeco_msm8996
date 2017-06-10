/*******************************************************************************
* Copyright (c) 2008-2012,2014 Qualcomm Technologies, Inc.  All Rights Reserved.
* Qualcomm Technologies Proprietary and Confidential.
*******************************************************************************/

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
10/16/14   vgaur   Added multi-threaded fastCV flag.
03/25/14  revathys Added stride support.
03/08/11   mingy   Updated included header file list.
05/26/10   staceyw Move scale type jpege_scale_t to jpege_engine_sw_scale.h
                   and scale related constant define to jpege_engine_sw_scale.c
05/18/10   vma     Rely on os_thread_detach for thread detachment rather
                   than joining
04/28/10   sigu    Adding a flag is_ethread_need_join to ensure that thread join is
                   called to avoid memleaks
02/16/10   staceyw Add two flag and cond for last buffer output to fix
                   syncronization with encode and output two different threads.
10/22/09   vma     Export the engine 'profile' for easier engine picking.
10/14/09   staceyw Remove IPL, added in-line cropping and upscaling support,
                   and split out jpege_engine_sw_configure_internal_buffer(),
                   jpege_engine_sw_configure_parameter() and
                   jpege_engine_sw_configure_huff_tables().
09/08/09   zhiminl Combined fetch_block with fdct_block.
08/03/09   vma     Switched to use the os abstraction layer (os_*)
06/17/09   zhiminl Added cropping and upscaling support (phase 1 - IPL).
04/14/09   zhiminl Added internal bitstream pack buffer.
04/08/09   zhiminl Replaced zigzag table with zigzag offset table.
10/14/08   vma     Optimizations.
07/29/08   vma     Created file.

========================================================================== */

#ifndef _JPEGE_ENGINE_SW_H
#define _JPEGE_ENGINE_SW_H

#include "os_thread.h"
#include "jpeg_common.h"
#include "jpeg_common_private.h"
#include "jpege_engine_hybrid.h"
#include "jpege_engine_sw_scale.h"

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
#define PENDING_OUTPUT_Q_SIZE 2
#define H2V2  0
#define H2V1  1
#define H1V2  2
#define H1V1  3
#define NUM_CHROMA_BLOCKS 2

#define JBLOCK_SIZE 64
#define JBLOCK_SIZE_MINUS_1 63

#define BLOCK_WIDTH 8
#define BLOCK_HEIGHT 8

#define DC 0
#define AC 1

#define FIX_POINT_FACTOR 18
#define FIX_POINT_ROUNDING_NUM 0x20000

#define NUM_DC_CATEGORIES 12
#define NUM_AC_CATEGORIES 11

#define EOB 0x00
#define ZRL 0xF0

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef int16_t DCTELEM;    // 16 or 32 bits is fine

typedef enum
{
    LUMA,
    CB,
    CR

} yCbCrType_t;

typedef enum
{
    YCbCr,
    YCrCb

} input_format_t;

typedef struct
{
    uint32_t bitAssemblyBufferUL32;   // 32-bit buffer for bitstream_t assembly
    uint32_t bitsFree;                // # of bits free in the bitAssemblyBuff
    uint8_t *nextBytePtr;             // Next output byte goes to this location
    uint8_t *buffer;                  // Pointer to the allocated bitstream buffer
    uint32_t buffersize;              // Size of the bitstream buffer

} bitstream_t;

typedef struct
{
    jpeg_buf_t  *p_buffer;
    uint8_t      is_valid;
    // flag for last buffer
    uint8_t      is_last_buffer;

} jpege_engine_output_t;

typedef struct
{
    uint16_t codeLength;
    uint16_t codeWord;

} huff_lookup_table_t;

typedef struct
{
    huff_lookup_table_t lumaDCHuffLookupTable[12];
    huff_lookup_table_t chromaDCHuffLookupTable[12];
    huff_lookup_table_t lumaACHuffLookupTable[256];
    huff_lookup_table_t chromaACHuffLookupTable[256];

} huff_tables_t;

typedef struct
{
    uint8_t  dcBits[16];         // Number of DC codewords of each length
    uint8_t  dcHuffVal[12];      // List of DC Values assigned to codewords
    uint8_t  acBits[16];         // Number of AC codewords of each length
    uint8_t  acHuffVal[176];     // List of AC Values assigned to codewords

} huff_bits_val_tables_t;

typedef struct jpege_engine_sw_t jpege_engine_sw_t;
typedef void  (*jpege_engine_sw_fetch_dct_mcu_t)(jpege_engine_sw_t *p_engine,
                                                 int16_t *curr_mcu_luma_dct_output,
                                                 int16_t *curr_mcu_chroma_dct_output);
struct jpege_engine_sw_t
{
    jpege_engine_hybrid_obj_t  *p_wrapper;   // The wrapper engine object
    os_mutex_t           mutex;              // os mutex object
    os_cond_t            cond;               // os condition variable
    os_cond_t            output_cond;        // os condition variable (for signaling output thread)
    os_cond_t            consume_cond;       // os condition variable (for output thread consume signaling)
    os_cond_t            final_output_cond;  // os condition variable (for output last buffer flush signaling)
    os_thread_t          thread;             // The encode thread
    os_thread_t          output_thread;      // The output thread
    jpege_engine_dst_t   dest;               // the destination object

    jpege_engine_hybrid_event_handler_t   p_event_handler;   // the event handler
    jpege_engine_hybrid_output_handler_t  p_output_handler;  // the output handler

    uint8_t is_active;                       // Flag indicating whether the engine is
                                             // actively encoding or not
    uint8_t thread_exit_flag;                // Flag indicating whether the pending
                                             // output thread should exit

    jpege_engine_output_t pending_outputs[PENDING_OUTPUT_Q_SIZE]; // Pending output queue

    uint16_t pending_output_q_head;          // Pending output queue head
    uint16_t pending_output_q_tail;          // Pending output queue tail

    uint32_t restartInterval;                // restart interval in number of MCUs
    uint32_t rotation;                       // clockwise rotation angle
    jpeg_subsampling_t subsampling;          // Subsampling format
    uint32_t lumaWidth;                      // Input Luma Width
    uint32_t lumaHeight;                     // Input Luma Height
    uint32_t chromaWidth;                    // Input Chroma Width
    uint32_t chromaHeight;                   // Input Chroma Height
    uint16_t quantQualityFactor;             // Quantization Quality Factor
    input_format_t InputFormat;              // Input File Format (YCbCr or YCrCb)

    uint8_t * inputYPtr;                     // Input Y buffer ptr
    uint8_t * inputCbCrPtr;                  // Input CbCr buffer ptr

    huff_bits_val_tables_t jpegeLumaBitsValTables;   // Luma Huffman BITS & VALUE tables

    huff_bits_val_tables_t jpegeChromaBitsValTables; // Chroma Huffman BITS & VALUE tables

    huff_tables_t jpegeHuffTables;           // structure containing Huffman Look-up Tables

    int16_t lumaQuantTableQ16[JBLOCK_SIZE];
    int16_t chromaQuantTableQ16[JBLOCK_SIZE];

    int16_t zigzagOffsetTable[JBLOCK_SIZE];

    uint32_t MCUWidth;                 // MCU Width
    uint32_t MCUHeight;                // MCU Height

    uint32_t horiMCUCount;             // Horizontal MCU Count
    uint32_t vertMCUCount;             // Vertical MCU Count
    uint32_t horiMCUIndex;             // Horizontal MCU Index
    uint32_t vertMCUIndex;             // Vertical MCU Index

    uint32_t rotatedlumaWidth;         // rotated luma matrix width
    uint32_t rotatedchromaWidth;       // rotated chroma matrix width
    uint32_t rotatedlumaHeight;        // rotated luma matrix height
    uint32_t rotatedchromaHeight;      // rotated chroma matrix height

    uint32_t rotatedMCUWidth;          // rotated MCU Width
    uint32_t rotatedMCUHeight;         // rotated MCU Height

    uint32_t rotatedhoriMCUCount;      // rotated Horizontal MCU Count
    uint32_t rotatedvertMCUCount;      // rotated Vertical MCU Count
    uint32_t rotatedhoriMCUIndex;      // rotated Horizontal MCU Index
    uint32_t rotatedvertMCUIndex;      // rotated Vertical MCU Index

    uint32_t currOrigin_luma;          // Address of current origin in orginal luma matrix
    uint32_t currOrigin_chroma;        // Address of current origin in orginal chroma matrix
    int32_t horiIncrement_luma;        // Horizontal step for luma matrix
    int32_t vertIncrement_luma;        // Vertical step for luma matrix
    int32_t horiIncrement_chroma;      // Horizontal step for chroma matrix
    int32_t vertIncrement_chroma;      // Vertical step for chroma matrix

    int16_t prevLumaDC;                // previous Luma DC value
    int16_t prevCbDC;                  // previous Cb DC value
    int16_t prevCrDC;                  // previous Cr DC value
    uint16_t restartMCUModCount;       // MCU Mod Count to determine if restart interval has occured.
    uint16_t restartMarkerModCount;    // marker mod count(range = 0 to 7)

    bitstream_t jpegeBitStreamState;   // structure containing BitStream parameters

    int16_t  currMCULumaDctOutput[JBLOCK_SIZE * H2V2_NUM_LUMA_BLOCKS];
    int16_t  currMCUChromaDctOutput[JBLOCK_SIZE * NUM_CHROMA_BLOCKS];
    uint32_t numLumaBlocks;

    jpeg_buf_t     *p_dest_buffer;     // destination buffer
    jpeg_buf_t     *p_internal_buf;    // internal buffer (needed as destination buffer if
                                       // none is supplied)
    uint8_t        *output_buffer_ptr;   // output buffer pointer
    uint32_t        output_buffer_length;// output buffer length
    uint32_t        output_buffer_offset;// output buffer offset

    uint8_t abort_flag;                // abort flag
    uint8_t error_flag;                // error flag
    uint8_t final_output_flag;         // last buffer flag

    jpege_scale_t jpege_scale;         // in-line crop and scale
    jpege_stride_cfg_t stride_cfg;     // stride configuration
    uint32_t       output_MCUs;        // Piece output MCU
    uint32_t       base_restart_marker;// Piece base restart marker
    uint32_t       src_index;          // Piece source index

    uint8_t        fastCV_flag;

    // fetch dct mcu function pointer
    jpege_engine_sw_fetch_dct_mcu_t    jpege_engine_sw_fetch_dct_mcu_func;
};

#endif /* _JPEGE_ENGINE_SW_H */
