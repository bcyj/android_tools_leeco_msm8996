#ifndef _JPEG_HEADER_H
#define _JPEG_HEADER_H
/*========================================================================

                 C o m m o n   D e f i n i t i o n s

*//** @file jpeg_header.h

Copyright (C) 2008-2011 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */
/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
11/18/10   mingy   Added JPS structure member variable.
07/07/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "exif_private.h"
#include "jpeg_common_private.h"

#include "jps.h"

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef enum
{
    JPEG_PROCESS_BASELINE,                 // Baseline
    JPEG_PROCESS_EXTENDED_HUFFMAN,         // Extended Huffman
    JPEG_PROCESS_PROGRESSIVE_HUFFMAN,      // Extended Progressive Huffman
    JPEG_PROCESS_LOSSLESS_HUFFMAN,         // Lossless Huffman
    JPEG_PROCESS_DIFF_SEQ_HUFFMAN,         // Sequential Huffman
    JPEG_PROCESS_DIFF_PROG_HUFFMAN,        // Differential Progressive Huffman
    JPEG_PROCESS_DIFF_LOSSLESS_HUFFMAN,    // Differential Lossless Huffman
    JPEG_PROCESS_SEQ_ARITHMETIC,           // Sequential Arithmetic
    JPEG_PROCESS_PROGRESSIVE_ARITHMETIC,   // Progressive Arithmetic
    JPEG_PROCESS_LOSSLESS_ARITHMETIC,      // Lossless Arithmetic
    JPEG_PROCESS_DIFF_SEQ_ARITHMETIC,      // Differential Sequential Arithmetic
    JPEG_PROCESS_DIFF_PROG_ARITHMETIC,     // Differential Progressive Arithmetic
    JPEG_PROCESS_DIFF_LOSSLESS_ARITHMETIC, // Differential Lossless Arithmetic

} jpeg_encode_process_t;

typedef struct
{
    uint8_t comp_id;
    uint8_t sampling_h;
    uint8_t sampling_v;
    uint8_t qtable_sel;

} jpeg_comp_info_t;

typedef struct
{
    uint8_t comp_id;
    uint8_t dc_selector;
    uint8_t ac_selector;

} jpeg_comp_entropy_sel_t;

typedef struct
{
    uint32_t                  offset;
    uint8_t                   num_selectors;
    uint8_t                   spec_start;
    uint8_t                   spec_end;
    uint8_t                   succ_approx_h;
    uint8_t                   succ_approx_l;
    jpeg_comp_entropy_sel_t  *p_selectors;

} jpeg_scan_info_t;

typedef struct
{
    uint32_t                width;
    uint32_t                height;
    jpeg_subsampling_t      subsampling; // derived information
    uint32_t                restart_interval;
    jpeg_encode_process_t   process;
    uint8_t                 precision;
    uint8_t                 quant_precision;
    uint8_t                 num_comps;
    uint8_t                 num_scans;
    uint8_t                 qtable_present_flag;
    uint8_t                 htable_present_flag;
    jpeg_quant_table_t      p_qtables[4];
    jpeg_huff_table_t       p_htables[8];
    jpeg_comp_info_t       *p_comp_infos;
    jpeg_scan_info_t      **pp_scan_infos;

} jpeg_frame_info_t;

typedef struct
{
    jpeg_frame_info_t    *p_tn_frame_info;
    jpeg_frame_info_t    *p_main_frame_info;
    exif_info_t          *p_exif_info;
    jps_cfg_3d_t          jps_info;

} jpeg_header_t;

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
/* =======================================================================
**                          Function Declarations
** ======================================================================= */
jpeg_scan_info_t *jpeg_add_scan_info(jpeg_frame_info_t *p_frame_info);
void jpeg_header_destroy(jpeg_header_t *p_header);
void jpeg_frame_info_destroy(jpeg_frame_info_t *p_info);
#ifdef _DEBUG
void jpeg_dump_header(jpeg_header_t *p_header);
#else
#define jpeg_dump_header(p)
#endif

#endif /* _JPEG_HEADER_H */
