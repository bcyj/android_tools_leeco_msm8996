/*========================================================================

*//** @file jpeg_common.h

@par EXTERNALIZED FUNCTIONS
  (none)

@par INITIALIZATION AND SEQUENCING REQUIREMENTS
  (none)

Copyright (C) 2008-10, 2014 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
05/20/10   stacyew Added bitstream formats in jpeg color format and subsampling.
09/21/09   mingy   Defined jpeg_rectangle_t structure.
08/03/09   vma     Switched to use the os abstraction layer (os_*)
07/13/09   zhiminl Added new event for aborting.
04/23/09   vma     Added new event where encoded thumbnail is
                   too big and is dropped by the encoder.
03/13/09   leim    Added RGBa color format.
07/07/08   vma     Created file.

========================================================================== */

#ifndef _JPEG_COMMON_H
#define _JPEG_COMMON_H

#include <string.h>
#include "jpegerr.h"
#include "exif.h"

#include <stdlib.h>

// LUMA BLOCKS
#define H1V1_NUM_LUMA_BLOCKS      1
#define H2V1_NUM_LUMA_BLOCKS      2
#define H1V2_NUM_LUMA_BLOCKS      2
#define H2V2_NUM_LUMA_BLOCKS      4
// Restart Marker
#define BASE_RESTART_MARKER       0xFFD0
#define MAX_RESTART_MARKER_COUNT  8

typedef uint32_t  jpeg_event_t;
// Events common to encoder and decoder
#define JPEG_EVENT_DONE      0
#define JPEG_EVENT_WARNING   1
#define JPEG_EVENT_ERROR     2
#define JPEG_EVENT_ABORTED   3
// Events specific to encoder
#define JPEG_EVENT_THUMBNAIL_DROPPED 4

typedef uint16_t* jpeg_quant_table_t;
typedef struct
{
    uint8_t bits[17];
    uint8_t values[256];

} jpeg_huff_table_t;

#ifndef true
#define true  1
#endif
#ifndef false
#define false 0
#endif

// Color subsampling formats
// Ordering handcrafted for efficient coding, alter with care!
typedef enum
{
    JPEG_H2V2      = 0,
    JPEG_H2V1      = 1,
    JPEG_H1V2      = 2,
    JPEG_H1V1      = 3,
    JPEG_GRAYSCALE = 4,

    JPEG_BS_H2V2      = 6,
    JPEG_BS_H2V1      = 7,
    JPEG_BS_H1V2      = 8,
    JPEG_BS_H1V1      = 9,
    JPEG_YUY2         = 10,
    JPEG_UYVY         = 11,

} jpeg_subsampling_t;

// Possibly supported color formats
// Ordering handcrafted for efficient coding, alter with care!
typedef enum
{
    YCRCBLP_H2V2 = 0,
    YCBCRLP_H2V2 = 1,

    YCRCBLP_H2V1 = 2,
    YCBCRLP_H2V1 = 3,

    YCRCBLP_H1V2 = 4,
    YCBCRLP_H1V2 = 5,

    YCRCBLP_H1V1 = 6,
    YCBCRLP_H1V1 = 7,

    MONOCHROME = 8,

    RGB565 = 9,
    RGB888 = 10,
    RGBa   = 11,

    JPEG_BITSTREAM_H2V2 = 12,
    JPEG_BITSTREAM_H2V1 = 14,
    JPEG_BITSTREAM_H1V2 = 16,
    JPEG_BITSTREAM_H1V1 = 18,

    YUY2 = 20,
    UYVY = 22,

    JPEG_COLOR_FORMAT_MAX,

} jpeg_color_format_t;

typedef enum
{
    NV12,
    NV21,
    IYUV,
    YUV12
} jpeg_memory_format_t;

typedef struct
{
    uint32_t           width;
    uint32_t           height;
    jpeg_subsampling_t subsampling;

} jpeg_frm_info_t;

typedef struct
{
    jpeg_frm_info_t    thumbnail;
    jpeg_frm_info_t    main;
    exif_info_obj_t    exif_info;

} jpeg_hdr_t;

typedef struct
{
    int32_t left;
    int32_t top;
    int32_t right;
    int32_t bottom;

} jpeg_rectangle_t;

#endif // #ifndef _JPEG_COMMON_H
