#ifndef _JPEG_WRITER_H
#define _JPEG_WRITER_H
/*========================================================================

                 C o m m o n   D e f i n i t i o n s

*//** @file jpeg_writer.h

Copyright (c) 1991-1998, Thomas G. Lane
See the IJG_README.txt file for more details.
Copyright (C) 2008-11 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
4/20/10    leim    Added mobicat data support implementation
02/25/10   leim    Add jpege_mbc_t for mobicat data structure.
02/16/10   sw      Added the pass of thumbnail buffer in jpegw_emit_header
                   from encoder intead of allocating one in jpeg writer.
10/13/09   vma     Changed to pass output to upper layer (encoder) instead
                   of outputting through destination callback directly.
07/07/08   vma     Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpege.h"
#include "jpeg_buffer_private.h"
#include "exif_private.h"

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */

// Maximum APP1 length
#define EXIF_MAX_APP1_LENGTH     0xFFFF


// Maximum Thumbnail length
#define EXIF_MAX_THUMBNAIL_LENGTH 0x7FFFF

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef uint8_t jpeg_marker_t;

typedef int (*jpegw_output_handler_t)(jpege_obj_t encoder, jpeg_buf_t *p_buf);
typedef struct
{
    uint32_t  length;
    uint8_t *data;
}jpege_mbc_t;

typedef struct
{
    // Output handler
    jpegw_output_handler_t  p_handler;

    // Scratch buffer
    jpeg_buf_t *scratchBuf;

    // Write ahead buffer
    jpeg_buf_t *aheadBuf;

    // Thumbnail buffer
    jpeg_buf_t *thumbnailBuf;

    // Save the Tiff header location
    uint32_t nTiffHeaderOffset;

    // Save the App1 length location
    uint32_t nApp1LengthOffset;

    // Save the thumbnail starting location
    uint32_t nThumbnailOffset;

    // Save the thumbnail stream starting location
    uint32_t nThumbnailStreamOffset;

    // Save the Gps Ifd pointer location
    uint32_t nGpsIfdPointerOffset;

    // Save the Thumbnail Ifd pointer location
    uint32_t nThumbnailIfdPointerOffset;

    // Keep count of how many tags are written
    uint32_t nFieldCount;

    // Save the location to write the IFD count
    uint32_t nFieldCountOffset;

    // Save the offset in ThumbnailIfd specifying the end of thumbnail offset
    uint32_t nJpegInterchangeLOffset;

    // Flag indicating whether header is written
    uint8_t fHeaderWritten;

    // Flag indicating if App1 header is present
    uint8_t fApp1Present;

    // The Jpeg Encoder object
    uint8_t app2_present;

    // The App2 start offset
    uint32_t app2_start_offset;

    // The App2 data length
    uint32_t app2_header_length;

    // The Jpeg Encoder object
    jpege_obj_t  encoder;

    // The image source object
    jpege_src_t *p_source;

    // The destination object
    jpege_dst_t *p_dest;

    // The encode configuration
    jpege_cfg_t *p_encode_cfg;

    // The Exif Info object
    exif_info_t *p_exif_info;
    jpege_mbc_t *p_mobicat_data;

    // Flag indicating whether the exif info object is owned by the writer
    uint8_t is_exif_info_owned;

    // Flag indicating whether any overflow has occurred
    uint8_t overflow_flag;

} jpeg_writer_t;

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
#define JPEG_EXIF_HEADER_SIZE 2000   /* Enough to hold GPS and PIM data  */
#define JPEG_JFIF_HEADER_SIZE  600
/* =======================================================================
**                          Function Declarations
** ======================================================================= */

int  jpegw_init               (jpeg_writer_t            *p_writer,
                               jpege_obj_t               encoder,
                               jpegw_output_handler_t    p_handler);
void jpegw_configure          (jpeg_writer_t            *p_writer,
                               jpege_src_t              *p_source,
                               jpege_dst_t              *p_dest,
                               jpege_cfg_t              *p_encode_cfg,
                               jpege_mbc_t              *p_mobicat_data,
                               exif_info_obj_t          *p_exif_info_obj);
void jpegw_reset              (jpeg_writer_t            *p_writer);
int  jpegw_emit_header        (jpeg_writer_t            *p_writer,
                               jpeg_buf_t               *p_thumbnail_buf);
void jpegw_emit_frame_header  (jpeg_writer_t            *p_writer,
                               jpege_img_cfg_t          *p_config,
                               jpege_img_data_t         *p_src);
void jpegw_emit_scan_header   (jpeg_writer_t            *p_writer,
                               const jpege_img_cfg_t    *p_config);
void jpegw_destroy            (jpeg_writer_t            *p_writer);

#endif /* _JPEG_WRITER_H */
