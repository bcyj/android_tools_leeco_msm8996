#ifndef _JPS_WRITER_H
#define _JPS_WRITER_H
/*========================================================================

                 C o m m o n   D e f i n i t i o n s

*//** @file jps_writer.h

Copyright (c) 1991-1998, Thomas G. Lane
See the IJG_README.txt file for more details.
Copyright (C) 2010-11 Qualcomm Technologies, Inc.
All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                             Edit History

$Header:

when       who     what, where, why
--------   ---     -------------------------------------------------------
11/15/10   staceyw Created file.

========================================================================== */

/* =======================================================================

                     INCLUDE FILES FOR MODULE

========================================================================== */
#include "jpse.h"
#include "jpeg_buffer_private.h"
#include "exif_private.h"

/* =======================================================================

                        DATA DECLARATIONS

========================================================================== */
/* -----------------------------------------------------------------------
** Constant / Define Declarations
** ----------------------------------------------------------------------- */
typedef uint8_t jps_marker_t;

// Maximum APP1 length
#define EXIF_MAX_APP1_LENGTH     0xFFFF

typedef int (*jpsw_output_handler_t)(jpse_obj_t encoder, jpeg_buf_t *p_buf);

typedef struct
{
    // mobicat data length
    uint32_t  length;

    // mobicat data
    uint8_t *data;


}jpse_mbc_t;

/* -----------------------------------------------------------------------
** Type Declarations
** ----------------------------------------------------------------------- */
typedef struct
{
    // Output handler
    jpsw_output_handler_t  p_handler;

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

    // Flag indicating if App3 header is present
    uint8_t fApp3Present;

    // The Jpeg Encoder object
    jpse_obj_t  encoder;

    // The image source object
    jpse_src_t *p_source;

    // The destination object
    jpse_dst_t *p_dest;

    // The encode configuration
    jpse_cfg_t *p_encode_cfg;

    // The encode 3d configuration
    jps_cfg_3d_t *p_encode_cfg_3d;

    // The Exif Info object
    exif_info_t *p_exif_info;

    // The Mobicata data object
    jpse_mbc_t *p_mobicat_data;

    // Flag indicating whether the exif info object is owned by the writer
    uint8_t is_exif_info_owned;

    // Flag indicating whether any overflow has occurred
    uint8_t overflow_flag;

} jps_writer_t;

/* =======================================================================
**                          Macro Definitions
** ======================================================================= */
#define JPEG_EXIF_HEADER_SIZE 2000   /* Enough to hold GPS and PIM data  */
#define JPEG_JFIF_HEADER_SIZE  600
/* =======================================================================
**                          Function Declarations
** ======================================================================= */

int  jpsw_init                (jps_writer_t            *p_writer,
                               jpse_obj_t                encoder,
                               jpsw_output_handler_t    p_handler);
void jpsw_configure           (jps_writer_t            *p_writer,
                               jpse_src_t               *p_source,
                               jpse_dst_t               *p_dest,
                               jpse_cfg_t               *p_encode_cfg,
                               jps_cfg_3d_t             *p_encode_cfg_3d,
                               jpse_mbc_t               *p_mobicat_data,
                               exif_info_obj_t          *p_exif_info_obj);
void jpsw_reset               (jps_writer_t            *p_writer);
int  jpsw_emit_header         (jps_writer_t            *p_writer,
                               jpeg_buf_t               *p_thumbnail_buf);
void jpsw_emit_frame_header   (jps_writer_t            *p_writer,
                               jpse_img_cfg_t          *p_config,
                               jpse_img_data_t         *p_src);
void jpsw_emit_scan_header    (jps_writer_t            *p_writer,
                               const jpse_img_cfg_t    *p_config);
void jpsw_destroy             (jps_writer_t            *p_writer);

#endif /* _JPS_WRITER_H */
